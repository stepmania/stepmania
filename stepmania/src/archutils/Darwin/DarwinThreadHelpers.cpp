#include <mach/mach_types.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#include "Backtrace.h"

bool SuspendThread(uint64_t threadHandle)
{
	return !thread_suspend(thread_act_t(threadHandle));
}

bool ResumeThread(uint64_t threadHandle)
{
	return !thread_resume(thread_act_t(threadHandle));
}

uint64_t GetCurrentThreadId()
{
	return mach_thread_self();
}

bool GetThreadBacktraceContext(int iCrashHandle, BacktraceContext *ctx)
{
	thread_act_t thread = thread_act_t(iCrashHandle);
	ppc_thread_state state;
	mach_msg_type_number_t count = PPC_THREAD_STATE_COUNT;
	
	if (thread_get_state(thread, PPC_THREAD_STATE, thread_state_t(&state),
						 &count))
		return false;
	ctx->FramePtr = (void *)state.r1;
	ctx->PC = (void *)state.srr0;
	return true;
}
	
