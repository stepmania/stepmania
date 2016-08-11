#include "global.h"
#include "Backtrace.hpp"
#include "RageUtil.h"

#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cerrno>
#include <fcntl.h>

#include <Cocoa/Cocoa.h>
#include <mach/mach.h>

static vm_address_t g_StackPointer = 0;
struct Frame
{
  const Frame *link;
  const void *return_address;
};
#define PROT_RW (VM_PROT_READ|VM_PROT_WRITE)
#define PROT_EXE (VM_PROT_READ|VM_PROT_EXECUTE)

/* Returns the starting address and the protection. Pass in mach_task_self() and the starting address. */
static bool GetRegionInfo( mach_port_t machPort, const void *address, vm_address_t &startOut, vm_prot_t &protectionOut )
{
  struct vm_region_basic_info_64 info;
  mach_msg_type_number_t infoCnt = VM_REGION_BASIC_INFO_COUNT_64;
  mach_port_t unused;
  vm_size_t size = 0;
  vm_address_t start = vm_address_t( address );
  kern_return_t ret = vm_region_64 (machPort, &start, &size, VM_REGION_BASIC_INFO_64,
                                (vm_region_info_t)&info, &infoCnt, &unused );
  
  if( ret != KERN_SUCCESS ||
     start >= (vm_address_t)address ||
     (vm_address_t)address >= start + size )
  {
    return false;
  }
  startOut = start;
  protectionOut = info.protection;
  return true;
}

void InitializeBacktrace()
{
  static bool bInitialized = false;
  
  if( bInitialized )
    return;
  vm_prot_t protection;
  if( !GetRegionInfo(mach_task_self(), __builtin_frame_address(0), g_StackPointer, protection) ||
	    protection != PROT_RW )
  {
    g_StackPointer = 0;
  }
  bInitialized = true;
}

void GetSignalBacktraceContext( BacktraceContext *ctx, const ucontext_t *uc )
{
#if !defined(MACOSX)
  ctx->ip = (void *) uc->uc_mcontext->ss.eip;
  ctx->bp = (void *) uc->uc_mcontext->ss.ebp;
  ctx->sp = (void *) uc->uc_mcontext->ss.esp;
#elif defined(__i386__)
  ctx->ip = (void *) uc->uc_mcontext->__ss.__eip;
  ctx->bp = (void *) uc->uc_mcontext->__ss.__ebp;
  ctx->sp = (void *) uc->uc_mcontext->__ss.__esp;
#elif defined(__x86_64__)
  ctx->ip = (void *) uc->uc_mcontext->__ss.__rip;
  ctx->bp = (void *) uc->uc_mcontext->__ss.__rbp;
  ctx->sp = (void *) uc->uc_mcontext->__ss.__rsp;
#endif
}

/* The following from VirtualDub: */
/* ptr points to a return address, and does not have to be word-aligned. */
static bool PointsToValidCall( vm_address_t start, const void *ptr )
{
  using std::min;
  const char *buf = reinterpret_cast<char const *>(ptr);
  
  /* We're reading buf backwards, between buf[-7] and buf[-1].  Find out how
   * far we can read. */
  auto check = intptr_t(ptr) - start;
  const int len = min( check, 7ul );
  
  // Permissible CALL sequences that we care about:
  //
  //	E8 xx xx xx xx			CALL near relative
  //	FF (group 2)			CALL near absolute indirect
  //
  // Minimum sequence is 2 bytes (call eax).
  // Maximum sequence is 7 bytes (call dword ptr [eax+disp32]).
  
  if (len >= 5 && buf[-5] == '\xe8')
    return true;
  
  // FF 14 xx					CALL [reg32+reg32*scale]
  if (len >= 3 && buf[-3] == '\xff' && buf[-2]=='\x14')
    return true;
  
  // FF 15 xx xx xx xx		CALL disp32
  if (len >= 6 && buf[-6] == '\xff' && buf[-5]=='\x15')
    return true;
  
  // FF 00-3F(!14/15)			CALL [reg32]
  if (len >= 2 && buf[-2] == '\xff' && (unsigned char)buf[-1] < '\x40')
    return true;
  
  // FF D0-D7					CALL reg32
  if (len >= 2 && buf[-2] == '\xff' && char(buf[-1]&0xF8) == '\xd0')
    return true;
  
  // FF 50-57 xx				CALL [reg32+reg32*scale+disp8]
  if (len >= 3 && buf[-3] == '\xff' && char(buf[-2]&0xF8) == '\x50')
    return true;
  
  // FF 90-97 xx xx xx xx xx	CALL [reg32+reg32*scale+disp32]
  if (len >= 7 && buf[-7] == '\xff' && char(buf[-6]&0xF8) == '\x90')
    return true;
  
  return false;
}


void GetBacktrace( const void **buf, size_t size, const BacktraceContext *ctx )
{
  InitializeBacktrace();
  
  if( g_StackPointer == 0 )
  {
    buf[0] = BACKTRACE_METHOD_NOT_AVAILABLE;
    buf[1] = nullptr;
    return;
  }
  
  BacktraceContext CurrentCtx;
  if( ctx == nullptr )
  {
    ctx = &CurrentCtx;
    
    CurrentCtx.ip = nullptr;
    CurrentCtx.bp = __builtin_frame_address(0);
    CurrentCtx.sp = __builtin_frame_address(0);
  }
  
  mach_port_t self = mach_task_self();
  vm_address_t stackPointer = 0;
  vm_prot_t protection = 0;
  vm_address_t start = 0;
  
  size_t i = 0;
  if( i < size-1 && ctx->ip )
    buf[i++] = ctx->ip;
  
  if( GetRegionInfo(self, ctx->sp, stackPointer, protection) && protection == (VM_PROT_READ|VM_PROT_WRITE) )
  {
    const void *p = *(const void **)ctx->sp;
    if( GetRegionInfo(self, p, start, protection) &&
       (protection & (VM_PROT_READ|VM_PROT_EXECUTE)) == (VM_PROT_READ|VM_PROT_EXECUTE) &&
       PointsToValidCall(start, p) && i < size-1 )
    {
      buf[i++] = p;
    }
  }
  
  GetRegionInfo( self, ctx->sp, stackPointer, protection );
  if( protection != PROT_RW )
  {
    /* There isn't much we can do if this is the case. The stack should be read/write
     * and since it isn't, give up. */
    buf[i] = nullptr;
    return;
  }
  const Frame *frame = (Frame *)ctx->sp;
  
  while( i < size-1 )
  {
    // Make sure this is on the stack
    if( !GetRegionInfo(self, frame, start, protection) || protection != PROT_RW )
      break;
    if( (start != g_StackPointer && start != stackPointer) || uintptr_t(frame)-uintptr_t(start) < sizeof(Frame) )
      break;
    
    /* The stack pointer is always 16 byte aligned _before_ the call. Thus a valid frame
     * should look like the follwoing.
     * |                  |
     * |  Caller's frame  |
     * -------------------- 16 byte boundary
     * |     Linkage      | This is return_address
     * - - - - - - - - - -
     * |   Saved %ebp     | This is link
     * - - - - - - - - - -
     * |    Rest of       |
     * |  Callee's frame  |
     *
     * Therefore, frame + 8 should be on a 16 byte boundary, the frame link should be
     * at a higher address, the link should be on the stack and it should be RW. The
     * return address should be EXE and point to a valid call (well, just after). */
    if( (((uintptr_t)frame+8) & 0xF) != 0 ||// boundary
       frame->link <= frame || // the frame link goes up
       !GetRegionInfo(self, frame->link, start, protection) ||
       (start != g_StackPointer && start != stackPointer) || // the link is on the stack
       protection != PROT_RW || // RW
       !GetRegionInfo(self, frame->return_address, start, protection) ||
       protection != PROT_EXE || // EXE
       !PointsToValidCall(start, frame->return_address) )// follows a CALL
    {
      /* This is not a valid frame but we might be in code compiled with
       * -fomit-frame-pointer so look at each address on the stack that is
       * 4 bytes below a 16 byte boundary. */
      if( (((uintptr_t)frame+4) & 0xF) == 0 )
      {
        void *p = *(void **)frame;
        if( GetRegionInfo(self, p, start, protection) &&
           protection == PROT_EXE &&
           PointsToValidCall(start, p) )
        {
          buf[i++] = p;
        }
      }
      frame = (Frame *)(intptr_t(frame)+4);
      continue;
    }
    // Valid.
    buf[i++] = frame->return_address;
    frame = frame->link;
  }
  
  buf[i] = nullptr;
}
#undef PROT_RW
#undef PROT_EXE
