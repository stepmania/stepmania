#include "stdafx.h"

//	from VirtualDub
//	Copyright (C) 1998-2001 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

// DO NOT USE stdio.h!  printf() calls malloc()!
//#include <stdio.h>
#include <stdarg.h>
#include <crtdbg.h>

#include <windows.h>
#include <tlhelp32.h>

#include "resource.h"
#include "crash.h"
#include "disasm.h"
#include "CrashList.h"

#include "archutils/Win32/GotoURL.h"

static void DoSave(const EXCEPTION_POINTERS *pExc);

///////////////////////////////////////////////////////////////////////////

#define CODE_WINDOW (256)

///////////////////////////////////////////////////////////////////////////

extern HINSTANCE g_hInstance;
extern unsigned long version_num;

static CodeDisassemblyWindow *g_pcdw;

struct VDDebugInfoContext {
	void *pRawBlock;

	int nBuildNumber;

	const unsigned char *pRVAHeap;
	unsigned	nFirstRVA;

	const char *pClassNameHeap;
	const char *pFuncNameHeap;
	const unsigned long (*pSegments)[2];
	int		nSegments;
};


static VDDebugInfoContext g_debugInfo;

///////////////////////////////////////////////////////////////////////////

BOOL APIENTRY CrashDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void ReportCrashLog(HWND hwnd, HANDLE hFile);
static void ReportStaticLog(HWND hwnd, HANDLE hFile);

///////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
/*
void checkfpustack(const char *file, const int line) throw() {
	static const char szFPUProblemCaption[]="FPU/MMX internal problem";
	static const char szFPUProblemMessage[]="The FPU stack wasn't empty!  Tagword = %04x\nFile: %s, line %d";
	static bool seenmsg=false;

	char	buf[128];
	unsigned short tagword;

	if (seenmsg)
		return;

	__asm fnstenv buf

	tagword = *(unsigned short *)(buf + 8);

	if (tagword != 0xffff) {
		wsprintf(buf, szFPUProblemMessage, tagword, file, line);
		MessageBox(NULL, buf, szFPUProblemCaption, MB_OK);
		seenmsg=true;
	}

}

static void __declspec(naked) *operator new(size_t bytes) {
	static const char fname[]="stack trace";

	__asm {
		push	ebp
		mov		ebp,esp

		push	[ebp+4]				;return address
		push	offset fname		;'filename'
		push	_NORMAL_BLOCK		;block type
		push	[ebp+8]				;allocation size

		call	_malloc_dbg
		add		esp,16

		pop		ebp
		ret
	}
}
*/
#endif

#if 0
void __declspec(naked) stackcheck(void *&sp) {
	static const char g_szStackHemorrhage[]="WARNING: Thread is hemorrhaging stack space!\n";

	__asm {
		mov		eax,[esp+4]
		mov		ecx,[eax]
		or		ecx,ecx
		jnz		started
		mov		[eax],esp
		ret
started:
		sub		ecx,esp
		mov		eax,ecx
		sar		ecx,31
		xor		eax,ecx
		sub		eax,ecx
		cmp		eax,128
		jb		ok
		push	offset g_szStackHemorrhage
		call	dword ptr [OutputDebugString]
		int		3
ok:
		ret
	}
}
#endif

///////////////////////////////////////////////////////////////////////////
//
//	Nina's per-thread debug logs are really handy, so I back-ported
//	them to 1.x.  These are lightweight in comparison, however.
//

VirtualDubThreadState __declspec(thread) g_PerThreadState;
__declspec(thread)
struct {
	ListNode node;
	VirtualDubThreadState *pState;
} g_PerThreadStateNode;

class VirtualDubThreadStateNode : public ListNode2<VirtualDubThreadStateNode> {
public:
	VirtualDubThreadState *pState;
};

static CRITICAL_SECTION g_csPerThreadState;
static List2<VirtualDubThreadStateNode> g_listPerThreadState;
static LONG g_nThreadsTrackedMinusOne = -1;

void VirtualDubInitializeThread(const char *pszName) {
	DWORD dwThreadId = GetCurrentThreadId();

	if (!InterlockedIncrement(&g_nThreadsTrackedMinusOne)) {
		InitializeCriticalSection(&g_csPerThreadState);
	}

	EnterCriticalSection(&g_csPerThreadState);

	if (DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), (HANDLE *)&g_PerThreadState.hThread, NULL, FALSE, DUPLICATE_SAME_ACCESS)) {

		g_PerThreadState.pszThreadName = pszName;
		g_PerThreadState.dwThreadId = dwThreadId;

		g_PerThreadStateNode.pState = &g_PerThreadState;
		g_listPerThreadState.AddTail((ListNode2<VirtualDubThreadStateNode> *)&g_PerThreadStateNode);
	}

	LeaveCriticalSection(&g_csPerThreadState);
}

void VirtualDubDeinitializeThread() {
	EnterCriticalSection(&g_csPerThreadState);

	((ListNode2<VirtualDubThreadStateNode> *)&g_PerThreadStateNode)->Remove();

	LeaveCriticalSection(&g_csPerThreadState);

	if (g_PerThreadState.pszThreadName)
		CloseHandle((HANDLE)g_PerThreadState.hThread);

	InterlockedDecrement(&g_nThreadsTrackedMinusOne);
}

///////////////////////////////////////////////////////////////////////////

static const struct ExceptionLookup {
	DWORD	code;
	const char *name;
} exceptions[]={
	{	EXCEPTION_ACCESS_VIOLATION,			"Access Violation"		},
	{	EXCEPTION_BREAKPOINT,				"Breakpoint"			},
	{	EXCEPTION_FLT_DENORMAL_OPERAND,		"FP Denormal Operand"	},
	{	EXCEPTION_FLT_DIVIDE_BY_ZERO,		"FP Divide-by-Zero"		},
	{	EXCEPTION_FLT_INEXACT_RESULT,		"FP Inexact Result"		},
	{	EXCEPTION_FLT_INVALID_OPERATION,	"FP Invalid Operation"	},
	{	EXCEPTION_FLT_OVERFLOW,				"FP Overflow",			},
	{	EXCEPTION_FLT_STACK_CHECK,			"FP Stack Check",		},
	{	EXCEPTION_FLT_UNDERFLOW,			"FP Underflow",			},
	{	EXCEPTION_INT_DIVIDE_BY_ZERO,		"Integer Divide-by-Zero",	},
	{	EXCEPTION_INT_OVERFLOW,				"Integer Overflow",		},
	{	EXCEPTION_PRIV_INSTRUCTION,			"Privileged Instruction",	},
	{	EXCEPTION_ILLEGAL_INSTRUCTION,		"Illegal instruction"	},
	{	EXCEPTION_INVALID_HANDLE,			"Invalid handle"		},
	{	0xe06d7363,							"Unhandled Microsoft C++ Exception",	},
			// hmm... '_msc'... gee, who would have thought?
	{	NULL	},
};

long VDDebugInfoLookupRVA(VDDebugInfoContext *pctx, unsigned rva, char *buf, int buflen);
bool VDDebugInfoInitFromMemory(VDDebugInfoContext *pctx, const void *_src);
bool VDDebugInfoInitFromFile(VDDebugInfoContext *pctx, const char *pszFilename);
void VDDebugInfoDeinit(VDDebugInfoContext *pctx);

static void SpliceProgramPath(char *buf, int bufsiz, const char *fn) {
	char tbuf[MAX_PATH];
	char *pszFile;

	GetModuleFileName(NULL, tbuf, sizeof tbuf);
	GetFullPathName(tbuf, bufsiz, buf, &pszFile);
	strcpy(pszFile, fn);
}

long CrashSymLookup(VDDisassemblyContext *pctx, unsigned long virtAddr, char *buf, int buf_len) {
	if (!g_debugInfo.pRVAHeap)
		return -1;

	return VDDebugInfoLookupRVA(&g_debugInfo, virtAddr, buf, buf_len);
}

static char szEmergencyDumpName[MAX_PATH];
static void DoSaveEmergencyDump(const char *buf, int siz) {
	HANDLE hFile;

	SpliceProgramPath(szEmergencyDumpName, sizeof szEmergencyDumpName, "crashdump.dat");
	hFile = CreateFile(szEmergencyDumpName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return;

	DWORD dwActual;
	WriteFile(hFile, buf, siz, &dwActual, NULL);
	FlushFileBuffers(hFile);
	
	CloseHandle(hFile);
}

static void DoEraseEmergencyDump()
{
	DeleteFile(szEmergencyDumpName);
}

extern HWND g_hWndMain;
long __stdcall CrashHandler(EXCEPTION_POINTERS *pExc) {
	/* If we're fullscreen, the fullscreen d3d window will obscure
	 * the crash dialog.  Do this before we suspend threads, or it'll
	 * deadlock if the thread controlling this window has been stopped. */
	/* XXX problem: If the sound thread crashes at the same time the main
	 * thread tells it to shut down, the main thread will be blocking waiting
	 * for the sound thread to finish; we'll come here and deadlock waiting for
	 * it to hide.  Hmmmmm. */
	ShowWindow( g_hWndMain, SW_HIDE );

	/////////////////////////
	//
	// QUICKLY: SUSPEND ALL THREADS THAT AREN'T US.

	EnterCriticalSection(&g_csPerThreadState);

	try {
		DWORD dwCurrentThread = GetCurrentThreadId();

		for(List2<VirtualDubThreadStateNode>::fwit it = g_listPerThreadState.begin(); it; ++it) {
			const VirtualDubThreadState *pState = it->pState;

			if (pState->dwThreadId && pState->dwThreadId != dwCurrentThread) {
				SuspendThread((HANDLE)pState->hThread);
			}
		}
	} catch(...) {
	}

	LeaveCriticalSection(&g_csPerThreadState);

	static bool InHere = false;
	if(InHere)
	{
		/* If we get here, then we've been called recursively, which means
		 * we (the disassembler or related code) crashed.  If this happens,
		 * we're not in very good shape.  We've left the emergency dump;
		 * that's all we have to go by.  Tell this to the user, and then die.
		 * 
		 * We've already blown up twice, and this is our last line of defense.
		 * If this dies, we have nothing, so keep it simple. */
		SetUnhandledExceptionFilter(NULL);
		MessageBox( NULL,
			"The error reporting interface has crashed.\n"
			"Please report a bug and attach the file \"crashdump.dat\"\n"
			"to the report.", "Fatal Error", MB_OK );
#ifdef _DEBUG
		DebugBreak();
#endif

		return EXCEPTION_EXECUTE_HANDLER;
	}
	InHere=true;
	/////////////////////////

	static char buf[CODE_WINDOW+16];
	HANDLE hprMe = GetCurrentProcess();
	void *lpBaseAddress = pExc->ExceptionRecord->ExceptionAddress;
	char *lpAddr = (char *)((long)lpBaseAddress & -32);

	memset(buf, 0, sizeof buf);

	if ((unsigned long)lpAddr > CODE_WINDOW/2)
		lpAddr -= CODE_WINDOW/2;
	else
		lpAddr = NULL;

	if (!ReadProcessMemory(hprMe, lpAddr, buf, CODE_WINDOW, NULL)) {
		int i;

		for(i=0; i<CODE_WINDOW; i+=32)
			if (!ReadProcessMemory(hprMe, lpAddr+i, buf+i, 32, NULL))
				memset(buf+i, 0, 32);
	}

	DoSaveEmergencyDump(buf, CODE_WINDOW);
		 
	CodeDisassemblyWindow cdw(buf, CODE_WINDOW, (char *)(buf-lpAddr), lpAddr);

	g_pcdw = &cdw;

	cdw.setFaultAddress(lpBaseAddress);

	// Attempt to read debug file.

	bool bSuccess=0;

	if (cdw.vdc.pExtraData) {
		bSuccess = VDDebugInfoInitFromMemory(&g_debugInfo, cdw.vdc.pExtraData);
	} else {
		SpliceProgramPath(buf, sizeof buf, "StepMania.vdi");
		bSuccess = VDDebugInfoInitFromFile(&g_debugInfo, buf);
		if (!bSuccess) {
			SpliceProgramPath(buf, sizeof buf, "StepMani.vdi");
			bSuccess = VDDebugInfoInitFromFile(&g_debugInfo, buf);
		}
	}

	cdw.vdc.pSymLookup = CrashSymLookup;

	cdw.parse();

	/* In case something goes amiss before the user can view the crash
	 * dump, save it now. */
	DoSave(pExc);

	/* Little trick to get an HINSTANCE of ourself without having access to the hwnd ... */
	int ret;
	{
		TCHAR szFullAppPath[MAX_PATH];
		GetModuleFileName(NULL, szFullAppPath, MAX_PATH);
		HINSTANCE handle = LoadLibrary(szFullAppPath);

		/* Returns TRUE if we want to pop up the standard crash box;
		 * FALSE if we should just die. */
		ret = DialogBoxParam(handle, MAKEINTRESOURCE(IDD_DISASM_CRASH), NULL,
			CrashDlgProc, (LPARAM)pExc);
	}

	VDDebugInfoDeinit(&g_debugInfo);

	/* We've made it.  Delete the emergency dump. */
	DoEraseEmergencyDump();
	InHere = false;

	SetUnhandledExceptionFilter(NULL);
	if(ret)
		UnhandledExceptionFilter(pExc);

	return EXCEPTION_EXECUTE_HANDLER;
}

static void Report(HWND hwndList, HANDLE hFile, const char *format, ...) {
	char buf[10240];
	va_list val;
	int ch;

	va_start(val, format);
	ch = wvsprintf(buf, format, val);
	va_end(val);

	if (hwndList)
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)buf);

	if (hFile) {
		DWORD dwActual;

		buf[ch] = '\r';
		buf[ch+1] = '\n';
		WriteFile(hFile, buf, ch+2, &dwActual, NULL);
	}
}

static void SetWindowTitlef(HWND hwnd, const char *format, ...) {
	char buf[256];
	va_list val;

	va_start(val, format);
	wvsprintf(buf, format, val);
	va_end(val);
	SetWindowText(hwnd, buf);
}

static void ReportReason(HWND hwnd, HWND hwndReason, HANDLE hFile, const EXCEPTION_POINTERS *const pExc)
{
	const EXCEPTION_RECORD *const pRecord = (const EXCEPTION_RECORD *)pExc->ExceptionRecord;

	// fill out bomb reason
	const struct ExceptionLookup *pel = exceptions;

	while(pel->code) {
		if (pel->code == pRecord->ExceptionCode)
			break;

		++pel;
	}

	// Unfortunately, EXCEPTION_ACCESS_VIOLATION doesn't seem to provide
	// us with the read/write flag and virtual address as the docs say...
	// *sigh*

	if (!pel->code) {
		if (hwndReason)
			SetWindowTitlef(hwndReason, "Crash reason: unknown exception 0x%08lx", pRecord->ExceptionCode);

		if (hFile)
			Report(hwnd, hFile, "Crash reason: unknown exception 0x%08lx", pRecord->ExceptionCode);
	} else {
		if (hwndReason)
			SetWindowTitlef(hwndReason, "Crash reason: %s", pel->name);

		if (hFile)
			Report(hwnd, hFile, "Crash reason: %s", pel->name);
	}
}

static void ReportThreadStacks(HWND hwnd, HANDLE hFile, const EXCEPTION_POINTERS *const pExc)
{
	EnterCriticalSection(&g_csPerThreadState);

	try {
		for(List2<VirtualDubThreadStateNode>::fwit it = g_listPerThreadState.begin(); it; ++it) {
			const VirtualDubThreadState *pState = it->pState;

			Report(hwnd, hFile, "Thread %08lx (%s)", pState->dwThreadId, pState->pszThreadName?pState->pszThreadName:"unknown");

			for(int i=0; i<CHECKPOINT_COUNT; ++i) {
				const VirtualDubCheckpoint& cp = pState->cp[(pState->nNextCP+i) & (CHECKPOINT_COUNT-1)];

				if (cp.file) {
					Report(hwnd, hFile, "\t%s:%d %s", cp.file, cp.line, cp.message? cp.message:"");
				}
			}
		}
	} catch(...) {
	}

	LeaveCriticalSection(&g_csPerThreadState);
}

static void ReportCrashData(HWND hwnd, HANDLE hFile, const EXCEPTION_POINTERS *const pExc) {
//	const EXCEPTION_RECORD *const pRecord = (const EXCEPTION_RECORD *)pExc->ExceptionRecord;
	const CONTEXT *const pContext = (const CONTEXT *)pExc->ContextRecord;
	int i, tos;

	Report(hwnd, hFile, "EAX = %08lx", pContext->Eax);
	Report(hwnd, hFile, "EBX = %08lx", pContext->Ebx);
	Report(hwnd, hFile, "ECX = %08lx", pContext->Ecx);
	Report(hwnd, hFile, "EDX = %08lx", pContext->Edx);
	Report(hwnd, hFile, "EBP = %08lx", pContext->Ebp);
	Report(hwnd, hFile, "DS:ESI = %04x:%08lx", pContext->SegDs, pContext->Esi);
	Report(hwnd, hFile, "ES:EDI = %04x:%08lx", pContext->SegEs, pContext->Edi);
	Report(hwnd, hFile, "SS:ESP = %04x:%08lx", pContext->SegSs, pContext->Esp);
	Report(hwnd, hFile, "CS:EIP = %04x:%08lx", pContext->SegCs, pContext->Eip);
	Report(hwnd, hFile, "FS = %04x", pContext->SegFs);
	Report(hwnd, hFile, "GS = %04x", pContext->SegGs);
	Report(hwnd, hFile, "EFLAGS = %08lx", pContext->EFlags);
	Report(hwnd, hFile, "");

	// extract out MMX registers

	tos = (pContext->FloatSave.StatusWord & 0x3800)>>11;

	for(i=0; i<8; i++) {
		long *pReg = (long *)(pContext->FloatSave.RegisterArea + 10*((i-tos) & 7));

		Report(hwnd, hFile, "MM%c = %08lx%08lx", i+'0', pReg[1], pReg[0]);
	}
}

static const char *GetNameFromHeap(const char *heap, int idx) {
	while(idx--)
		while(*heap++);

	return heap;
}

//////////////////////////////////////////////////////////////////////////////

static bool IsValidCall(char *buf, int len) {
	// Permissible CALL sequences that we care about:
	//
	//	E8 xx xx xx xx			CALL near relative
	//	FF (group 2)			CALL near absolute indirect
	//
	// Minimum sequence is 2 bytes (call eax).
	// Maximum sequence is 7 bytes (call dword ptr [eax+disp32]).

	if (len >= 5 && buf[-5] == (char)0xE8)
		return true;

	// FF 14 xx					CALL [reg32+reg32*scale]

	if (len >= 3 && buf[-3] == (char)0xFF && buf[-2]==0x14)
		return true;

	// FF 15 xx xx xx xx		CALL disp32

	if (len >= 6 && buf[-6] == (char)0xFF && buf[-5]==0x15)
		return true;

	// FF 00-3F(!14/15)			CALL [reg32]

	if (len >= 2 && buf[-2] == (char)0xFF && (unsigned char)buf[-1] < 0x40)
		return true;

	// FF D0-D7					CALL reg32

	if (len >= 2 && buf[-2] == (char)0xFF && (buf[-1]&0xF8) == 0xD0)
		return true;

	// FF 50-57 xx				CALL [reg32+reg32*scale+disp8]

	if (len >= 3 && buf[-3] == (char)0xFF && (buf[-2]&0xF8) == 0x50)
		return true;

	// FF 90-97 xx xx xx xx xx	CALL [reg32+reg32*scale+disp32]

	if (len >= 7 && buf[-7] == (char)0xFF && (buf[-6]&0xF8) == 0x90)
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////////

struct ModuleInfo {
	const char *name;
	unsigned long base, size;
};

// ARRGH.  Where's psapi.h?!?

struct Win32ModuleInfo {
	DWORD base, size, entry;
};

typedef BOOL (__stdcall *PENUMPROCESSMODULES)(HANDLE, HMODULE *, DWORD, LPDWORD);
typedef DWORD (__stdcall *PGETMODULEBASENAME)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef BOOL (__stdcall *PGETMODULEINFORMATION)(HANDLE, HMODULE, Win32ModuleInfo *, DWORD);

typedef HANDLE (__stdcall *PCREATETOOLHELP32SNAPSHOT)(DWORD, DWORD);
typedef BOOL (WINAPI *PMODULE32FIRST)(HANDLE, LPMODULEENTRY32);
typedef BOOL (WINAPI *PMODULE32NEXT)(HANDLE, LPMODULEENTRY32);

static ModuleInfo *CrashGetModules(void *&ptr) {
	void *pMem = VirtualAlloc(NULL, 65536, MEM_COMMIT, PAGE_READWRITE);

	if (!pMem) {
		ptr = NULL;
		return NULL;
	}

	// This sucks.  If we're running under Windows 9x, we must use
	// TOOLHELP.DLL to get the module list.  Under Windows NT, we must
	// use PSAPI.DLL.  With Windows 2000, we can use both (but prefer
	// PSAPI.DLL).

	HMODULE hmodPSAPI = LoadLibrary("psapi.dll");

	if (hmodPSAPI) {
		// Using PSAPI.DLL.  Call EnumProcessModules(), then GetModuleFileNameEx()
		// and GetModuleInformation().

		PENUMPROCESSMODULES pEnumProcessModules = (PENUMPROCESSMODULES)GetProcAddress(hmodPSAPI, "EnumProcessModules");
		PGETMODULEBASENAME pGetModuleBaseName = (PGETMODULEBASENAME)GetProcAddress(hmodPSAPI, "GetModuleBaseNameA");
		PGETMODULEINFORMATION pGetModuleInformation = (PGETMODULEINFORMATION)GetProcAddress(hmodPSAPI, "GetModuleInformation");
		HMODULE *pModules, *pModules0 = (HMODULE *)((char *)pMem + 0xF000);
		DWORD cbNeeded;

		if (pEnumProcessModules && pGetModuleBaseName && pGetModuleInformation
			&& pEnumProcessModules(GetCurrentProcess(), pModules0, 0x1000, &cbNeeded)) {

			ModuleInfo *pMod, *pMod0;
			char *pszHeap = (char *)pMem, *pszHeapLimit;

			if (cbNeeded > 0x1000) cbNeeded = 0x1000;

			pModules = (HMODULE *)((char *)pMem + 0x10000 - cbNeeded);
			memmove(pModules, pModules0, cbNeeded);

			pMod = pMod0 = (ModuleInfo *)((char *)pMem + 0x10000 - sizeof(ModuleInfo) * (cbNeeded / sizeof(HMODULE) + 1));
			pszHeapLimit = (char *)pMod;

			do {
				HMODULE hCurMod = *pModules++;
				Win32ModuleInfo mi;

				if (pGetModuleBaseName(GetCurrentProcess(), hCurMod, pszHeap, pszHeapLimit - pszHeap)
					&& pGetModuleInformation(GetCurrentProcess(), hCurMod, &mi, sizeof mi)) {

					char *period = NULL;

					pMod->name = pszHeap;

					while(*pszHeap++)
						if (pszHeap[-1] == '.')
							period = pszHeap-1;

					if (period) {
						*period = 0;
						pszHeap = period+1;
					}

					pMod->base = mi.base;
					pMod->size = mi.size;
					++pMod;
				}
			} while((cbNeeded -= sizeof(HMODULE *)) > 0);

			pMod->name = NULL;

			FreeLibrary(hmodPSAPI);
			ptr = pMem;
			return pMod0;
		}

		FreeLibrary(hmodPSAPI);
	} else {
		// No PSAPI.  Use the ToolHelp functions in KERNEL.

		HMODULE hmodKERNEL32 = LoadLibrary("kernel32.dll");

		PCREATETOOLHELP32SNAPSHOT pCreateToolhelp32Snapshot = (PCREATETOOLHELP32SNAPSHOT)GetProcAddress(hmodKERNEL32, "CreateToolhelp32Snapshot");
		PMODULE32FIRST pModule32First = (PMODULE32FIRST)GetProcAddress(hmodKERNEL32, "Module32First");
		PMODULE32NEXT pModule32Next = (PMODULE32NEXT)GetProcAddress(hmodKERNEL32, "Module32Next");
		HANDLE hSnap;

		if (pCreateToolhelp32Snapshot && pModule32First && pModule32Next) {
			if ((HANDLE)-1 != (hSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0))) {
				ModuleInfo *pModInfo = (ModuleInfo *)((char *)pMem + 65536);
				char *pszHeap = (char *)pMem;
				MODULEENTRY32 me;

				--pModInfo;
				pModInfo->name = NULL;

				me.dwSize = sizeof(MODULEENTRY32);

				if (pModule32First(hSnap, &me))
					do {
						if (pszHeap+strlen(me.szModule) >= (char *)(pModInfo - 1))
							break;

						strcpy(pszHeap, me.szModule);

						--pModInfo;
						pModInfo->name = pszHeap;

						char *period = NULL;

						while(*pszHeap++);
							if (pszHeap[-1]=='.')
								period = pszHeap-1;

						if (period) {
							*period = 0;
							pszHeap = period+1;
						}

						pModInfo->base = (unsigned long)me.modBaseAddr;
						pModInfo->size = me.modBaseSize;

					} while(pModule32Next(hSnap, &me));

				CloseHandle(hSnap);

				FreeLibrary(hmodKERNEL32);

				ptr = pMem;
				return pModInfo;
			}
		}

		FreeLibrary(hmodKERNEL32);
	}

	VirtualFree(pMem, 0, MEM_RELEASE);

	ptr = NULL;
	return NULL;
}

///////////////////////////////////////////////////////////////////////////
//
//	info from Portable Executable/Common Object File Format (PE/COFF) spec

typedef unsigned short ushort;
typedef unsigned long ulong;

struct PEHeader {
	ulong		signature;
	ushort		machine;
	ushort		sections;
	ulong		timestamp;
	ulong		symbol_table;
	ulong		symbols;
	ushort		opthdr_size;
	ushort		characteristics;
};

struct PESectionHeader {
	char		name[8];
	ulong		virtsize;
	ulong		virtaddr;
	ulong		rawsize;
	ulong		rawptr;
	ulong		relocptr;
	ulong		linenoptr;
	ushort		reloc_cnt;
	ushort		lineno_cnt;
	ulong		characteristics;
};

struct PEExportDirectory {
	ulong		flags;
	ulong		timestamp;
	ushort		major;
	ushort		minor;
	ulong		nameptr;
	ulong		ordbase;
	ulong		addrtbl_cnt;
	ulong		nametbl_cnt;
	ulong		addrtbl_ptr;
	ulong		nametbl_ptr;
	ulong		ordtbl_ptr;
};

struct PE32OptionalHeader {
	ushort		magic;					// 0
	char		major_linker_ver;		// 2
	char		minor_linker_ver;		// 3
	ulong		codesize;				// 4
	ulong		idatasize;				// 8
	ulong		udatasize;				// 12
	ulong		entrypoint;				// 16
	ulong		codebase;				// 20
	ulong		database;				// 24
	ulong		imagebase;				// 28
	ulong		section_align;			// 32
	ulong		file_align;				// 36
	ushort		majoros;				// 40
	ushort		minoros;				// 42
	ushort		majorimage;				// 44
	ushort		minorimage;				// 46
	ushort		majorsubsys;			// 48
	ushort		minorsubsys;			// 50
	ulong		reserved;				// 52
	ulong		imagesize;				// 56
	ulong		hdrsize;				// 60
	ulong		checksum;				// 64
	ushort		subsystem;				// 68
	ushort		characteristics;		// 70
	ulong		stackreserve;			// 72
	ulong		stackcommit;			// 76
	ulong		heapreserve;			// 80
	ulong		heapcommit;				// 84
	ulong		loaderflags;			// 88
	ulong		dictentries;			// 92

	// Not part of header, but it's convienent here

	ulong		export_RVA;				// 96
	ulong		export_size;			// 100
};

struct PE32PlusOptionalHeader {
	ushort		magic;					// 0
	char		major_linker_ver;		// 2
	char		minor_linker_ver;		// 3
	ulong		codesize;				// 4
	ulong		idatasize;				// 8
	ulong		udatasize;				// 12
	ulong		entrypoint;				// 16
	ulong		codebase;				// 20
	__int64		imagebase;				// 24
	ulong		section_align;			// 32
	ulong		file_align;				// 36
	ushort		majoros;				// 40
	ushort		minoros;				// 42
	ushort		majorimage;				// 44
	ushort		minorimage;				// 46
	ushort		majorsubsys;			// 48
	ushort		minorsubsys;			// 50
	ulong		reserved;				// 52
	ulong		imagesize;				// 56
	ulong		hdrsize;				// 60
	ulong		checksum;				// 64
	ushort		subsystem;				// 68
	ushort		characteristics;		// 70
	__int64		stackreserve;			// 72
	__int64		stackcommit;			// 80
	__int64		heapreserve;			// 88
	__int64		heapcommit;				// 96
	ulong		loaderflags;			// 104
	ulong		dictentries;			// 108

	// Not part of header, but it's convienent here

	ulong		export_RVA;				// 112
	ulong		export_size;			// 116
};

static const char *CrashLookupExport(HMODULE hmod, unsigned long addr, unsigned long &fnbase) {
	char *pBase = (char *)hmod;

	// The PEheader offset is at hmod+0x3c.  Add the size of the optional header
	// to step to the section headers.

	PEHeader *pHeader = (PEHeader *)(pBase + ((long *)hmod)[15]);

	if (pHeader->signature != 'EP')
		return NULL;

#if 0
	PESectionHeader *pSHdrs = (PESectionHeader *)((char *)pHeader + sizeof(PEHeader) + pHeader->opthdr_size);

	// Scan down the section headers and look for ".edata"

	int i;

	for(i=0; i<pHeader->sections; i++) {
		MessageBox(NULL, pSHdrs[i].name, "section", MB_OK);
		if (!memcmp(pSHdrs[i].name, ".edata", 6))
			break;
	}

	if (i >= pHeader->sections)
		return NULL;
#endif

	// Verify the optional structure.

	PEExportDirectory *pExportDir;

	if (pHeader->opthdr_size < 104)
		return NULL;

	switch(*(short *)((char *)pHeader + sizeof(PEHeader))) {
	case 0x10b:		// PE32
		{
			PE32OptionalHeader *pOpt = (PE32OptionalHeader *)((char *)pHeader + sizeof(PEHeader));

			if (pOpt->dictentries < 1)
				return NULL;

			pExportDir = (PEExportDirectory *)(pBase + pOpt->export_RVA);
		}
		break;
	case 0x20b:		// PE32+
		{
			PE32PlusOptionalHeader *pOpt = (PE32PlusOptionalHeader *)((char *)pHeader + sizeof(PEHeader));

			if (pOpt->dictentries < 1)
				return NULL;

			pExportDir = (PEExportDirectory *)(pBase + pOpt->export_RVA);
		}
		break;

	default:
		return NULL;
	}

	// Hmmm... no exports?

	if ((char *)pExportDir == pBase)
		return NULL;

	// Find the location of the export information.

	ulong *pNameTbl = (ulong *)(pBase + pExportDir->nametbl_ptr);
	ulong *pAddrTbl = (ulong *)(pBase + pExportDir->addrtbl_ptr);
	ushort *pOrdTbl = (ushort *)(pBase + pExportDir->ordtbl_ptr);

	// Scan exports.

	const char *pszName = NULL;
	ulong bestdelta = 0xFFFFFFFF;
	unsigned i;

	addr -= (ulong)pBase;

	for(i=0; i<pExportDir->nametbl_cnt; i++) {
		ulong fnaddr;
		int idx;

		idx = pOrdTbl[i];
		fnaddr = pAddrTbl[idx];

		if (addr >= fnaddr) {
			ulong delta = addr - fnaddr;

			if (delta < bestdelta) {
				bestdelta = delta;
				fnbase = fnaddr;

				if (pNameTbl[i])
					pszName = pBase + pNameTbl[i];
				else {
					static char buf[8];

					wsprintf(buf, "ord%d", pOrdTbl[i]);
					pszName = buf;
				}

			}
		}
	}

	return pszName;
}

///////////////////////////////////////////////////////////////////////////

static bool IsExecutableProtection(DWORD dwProtect) {
	MEMORY_BASIC_INFORMATION meminfo;

	// Windows NT/2000 allows Execute permissions, but Win9x seems to
	// rip it off.  So we query the permissions on our own code block,
	// and use it to determine if READONLY/READWRITE should be
	// considered 'executable.'

	VirtualQuery(IsExecutableProtection, &meminfo, sizeof meminfo);

	switch((unsigned char)dwProtect) {
	case PAGE_READONLY:				// *sigh* Win9x...
	case PAGE_READWRITE:			// *sigh*
		return meminfo.Protect==PAGE_READONLY || meminfo.Protect==PAGE_READWRITE;

	case PAGE_EXECUTE:
	case PAGE_EXECUTE_READ:
	case PAGE_EXECUTE_READWRITE:
	case PAGE_EXECUTE_WRITECOPY:
		return true;
	}
	return false;
}

static const char *CrashGetModuleBaseName(HMODULE hmod, char *pszBaseName) {
	char szPath1[MAX_PATH];
	char szPath2[MAX_PATH];

	__try {
		DWORD dw;
		char *pszFile, *period = NULL;

		if (!GetModuleFileName(hmod, szPath1, sizeof szPath1))
			return NULL;

		dw = GetFullPathName(szPath1, sizeof szPath2, szPath2, &pszFile);

		if (!dw || dw>sizeof szPath2)
			return NULL;

		strcpy(pszBaseName, pszFile);

		pszFile = pszBaseName;

		while(*pszFile++)
			if (pszFile[-1]=='.')
				period = pszFile-1;

		if (period)
			*period = 0;
	} __except(1) {
		return NULL;
	}

	return pszBaseName;
}


///////////////////////////////////////////////////////////////////////////

bool VDDebugInfoInitFromMemory(VDDebugInfoContext *pctx, const void *_src) {
	const unsigned char *src = (const unsigned char *)_src;

	pctx->pRVAHeap = NULL;

	// Check type string

	if (!memcmp((char *)src + 6, "] StepMania disasm", 18))
		src += *(long *)(src + 64) + 72;

	if (src[0] != '[' || src[3] != '|')
		return false;

	if (memcmp((char *)src + 6, "] StepMania symbolic debug information", 38))
		return false;

	// Check version number

//	int write_version = (src[1]-'0')*10 + (src[2] - '0');
	int compat_version = (src[4]-'0')*10 + (src[5] - '0');

	if (compat_version > 1)
		return false;	// resource is too new for us to load

	// Extract fields

	src += 64;

	pctx->nBuildNumber		= *(int *)src;
	pctx->pRVAHeap			= (const unsigned char *)(src + 24);
	pctx->nFirstRVA			= *(const long *)(src + 20);
	pctx->pClassNameHeap	= (const char *)pctx->pRVAHeap - 4 + *(const long *)(src + 4);
	pctx->pFuncNameHeap		= pctx->pClassNameHeap + *(const long *)(src + 8);
	pctx->pSegments			= (unsigned long (*)[2])(pctx->pFuncNameHeap + *(const long *)(src + 12));
	pctx->nSegments			= *(const long *)(src + 16);

	return true;
}

void VDDebugInfoDeinit(VDDebugInfoContext *pctx) {
	if (pctx->pRawBlock) {
		VirtualFree(pctx->pRawBlock, 0, MEM_RELEASE);
		pctx->pRawBlock = NULL;
	}
}

bool VDDebugInfoInitFromFile(VDDebugInfoContext *pctx, const char *pszFilename) {
	pctx->pRawBlock = NULL;
	pctx->pRVAHeap = NULL;

	HANDLE h = CreateFile(pszFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == h) {
		return false;
	}

	do {
		DWORD dwFileSize = GetFileSize(h, NULL);

		if (dwFileSize == 0xFFFFFFFF)
			break;

		pctx->pRawBlock = VirtualAlloc(NULL, dwFileSize, MEM_COMMIT, PAGE_READWRITE);
		if (!pctx->pRawBlock)
			break;

		DWORD dwActual;
		if (!ReadFile(h, pctx->pRawBlock, dwFileSize, &dwActual, NULL) || dwActual != dwFileSize)
			break;

		if (VDDebugInfoInitFromMemory(pctx, pctx->pRawBlock)) {
			CloseHandle(h);
			return true;
		}
	} while(false);

	VDDebugInfoDeinit(pctx);
	CloseHandle(h);
	return false;
}

long VDDebugInfoLookupRVA(VDDebugInfoContext *pctx, unsigned rva, char *buf, int buflen) {
	int i;

	for(i=0; i<pctx->nSegments; ++i) {
		if (rva >= pctx->pSegments[i][0] && rva < pctx->pSegments[i][0] + pctx->pSegments[i][1])
			break;
	}

	if (i >= pctx->nSegments)
		return -1;

	const unsigned char *pr = pctx->pRVAHeap;
	const unsigned char *pr_limit = (const unsigned char *)pctx->pClassNameHeap;
	int idx = 0;

	// Linearly unpack RVA deltas and find lower_bound

	rva -= pctx->nFirstRVA;

	if ((signed)rva < 0)
		return -1;

	while(pr < pr_limit) {
		unsigned char c;
		unsigned diff = 0;

		do {
			c = *pr++;

			diff = (diff << 7) | (c & 0x7f);
		} while(c & 0x80);

		rva -= diff;

		if ((signed)rva < 0) {
			rva += diff;
			break;
		}

		++idx;
	}

	// Decompress name for RVA

	if (pr < pr_limit) {
		const char *fn_name = GetNameFromHeap(pctx->pFuncNameHeap, idx);
		const char *class_name = NULL;
		const char *prefix = "";

		if (*fn_name == 0)
			return -1;

		if (*fn_name < 32) {
			int class_idx;

			class_idx = ((unsigned)fn_name[0] - 1)*128 + ((unsigned)fn_name[1] - 1);
			class_name = GetNameFromHeap(pctx->pClassNameHeap, class_idx);

			fn_name += 2;

			if (*fn_name == 1) {
				fn_name = class_name;
			} else if (*fn_name == 2) {
				fn_name = class_name;
				prefix = "~";
			} else if (*fn_name < 32)
				fn_name = "(special)";
		}

		// ehh... where's my wsnprintf?  _snprintf() might allocate memory or locks....
		return wsprintf(buf, "%s%s%s%s", class_name?class_name:"", class_name?"::":"", prefix, fn_name) >= 0
				? rva
				: -1;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////

static bool ReportCrashCallStack(HWND hwnd, HANDLE hFile, const EXCEPTION_POINTERS *const pExc, const void *pDebugSrc) {
	const CONTEXT *const pContext = (const CONTEXT *)pExc->ContextRecord;
	HANDLE hprMe = GetCurrentProcess();
	char *lpAddr = (char *)pContext->Esp;
	int limit = 100;
	unsigned long data;
	char buf[512];

	if (!g_debugInfo.pRVAHeap) {
		Report(hwnd, hFile, "Could not open debug resource file (StepMania.vdi).");
		return false;
	}

	if (g_debugInfo.nBuildNumber != int(version_num)) {
		Report(hwnd, hFile, "Incorrect StepMania.vdi file (build %d, expected %d) for this version of StepMania -- call stack unavailable.", g_debugInfo.nBuildNumber, int(version_num));
		return false;
	}

	// Get some module names.

	void *pModuleMem;
	ModuleInfo *pModules = CrashGetModules(pModuleMem);

	// Retrieve stack pointers.
	// Not sure if NtCurrentTeb() is available on Win95....

	NT_TIB *pTib;

	__asm {
		mov	eax, fs:[0]_NT_TIB.Self
		mov pTib, eax
	}

	char *pStackBase = (char *)pTib->StackBase;

	// Walk up the stack.  Hopefully it wasn't fscked.

	data = pContext->Eip;
	do {
		bool fValid = true;
		int len;
		MEMORY_BASIC_INFORMATION meminfo;

		VirtualQuery((void *)data, &meminfo, sizeof meminfo);
		
		if (!IsExecutableProtection(meminfo.Protect) || meminfo.State!=MEM_COMMIT) {
//				Report(hwnd, hFile, "Rejected: %08lx (%08lx)", data, meminfo.Protect);
			fValid = false;
		}

		if (data != pContext->Eip) {
			len = 7;

			*(long *)(buf + 0) = *(long *)(buf + 4) = 0;

			while(len > 0 && !ReadProcessMemory(GetCurrentProcess(), (void *)(data-len), buf+7-len, len, NULL))
				--len;

			fValid &= IsValidCall(buf+7, len);
		}
		
		if (fValid) {
			if (VDDebugInfoLookupRVA(&g_debugInfo, data, buf, sizeof buf) >= 0) {
				Report(hwnd, hFile, "%08lx: %s()", data, buf);
				--limit;
			} else {
				ModuleInfo *pMods = pModules;
				ModuleInfo mi;
				char szName[MAX_PATH];

				mi.name = NULL;

				if (pMods) {
					while(pMods->name) {
						if (data >= pMods->base && (data - pMods->base) < pMods->size)
							break;

						++pMods;
					}

					mi = *pMods;
				} else {

					// Well, something failed, or we didn't have either PSAPI.DLL or ToolHelp
					// to play with.  So we'll use a nastier method instead.

					mi.base = (unsigned long)meminfo.AllocationBase;
					mi.name = CrashGetModuleBaseName((HMODULE)mi.base, szName);
				}

				if (mi.name) {
					unsigned long fnbase;
					const char *pExportName = CrashLookupExport((HMODULE)mi.base, data, fnbase);

					if (pExportName)
						Report(hwnd, hFile, "%08lx: %s!%s [%08lx+%lx+%lx]", data, mi.name, pExportName, mi.base, fnbase, (data-mi.base-fnbase));
					else
						Report(hwnd, hFile, "%08lx: %s!%08lx", data, mi.name, data - mi.base);
				} else
					Report(hwnd, hFile, "%08lx: %08lx", data, data);

				--limit;
			}
		}

		if (lpAddr >= pStackBase)
			break;

		lpAddr += 4;
	} while(limit > 0 && ReadProcessMemory(hprMe, lpAddr-4, &data, 4, NULL));

	// All done, close up shop and exit.

	if (pModuleMem)
		VirtualFree(pModuleMem, 0, MEM_RELEASE);

	return true;
}

static void ReportDisasm(HWND hwnd, HANDLE hFile, const EXCEPTION_POINTERS *const pExc)
{
	char tbuf[2048];
	long idx = 0;

	Report(hwnd, hFile, "Disassembly:");

	while(idx = g_pcdw->getInstruction(tbuf, idx)) {
		Report(hwnd, hFile, "%s", tbuf);
	}
	FlushFileBuffers(hFile);
}

static void DoSave(const EXCEPTION_POINTERS *pExc) {
	HANDLE hFile;
	char szModName2[MAX_PATH];

	SpliceProgramPath(szModName2, sizeof szModName2, "crashinfo.txt");

	hFile = CreateFile(szModName2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == hFile)
		return;

	Report(NULL, hFile,
			"StepMania crash report -- build %d\r\n"
			"--------------------------------------"
			"\r\n", version_num);

	// Detect operating system.

	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	/* XXX: rewrite this mess */
	if (GetVersionEx(&ovi)) {
		Report(NULL, hFile, "Windows %d.%d (Win%s build %d) [%s]",
			ovi.dwMajorVersion,
			ovi.dwMinorVersion,
			ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS?
				(ovi.dwMinorVersion>0? "98" : "95"):
			ovi.dwPlatformId == VER_PLATFORM_WIN32_NT?
				(ovi.dwMajorVersion >= 5? 
				    (ovi.dwMinorVersion>0? "XP":"2000"): "NT")
					: "?",
			ovi.dwBuildNumber & 0xffff,
			ovi.szCSDVersion);
	}
	Report(NULL, hFile, "");

	ReportReason(NULL, NULL, hFile, pExc);
	Report(NULL, hFile, "");

	// Dump thread stacks
	ReportThreadStacks(NULL, hFile, pExc);
	Report(NULL, hFile, "");

	ReportCrashCallStack(NULL, hFile, pExc, g_pcdw->vdc.pExtraData);
	Report(NULL, hFile, "");

	ReportStaticLog(NULL, hFile);
	Report(NULL, hFile, "");

	ReportCrashLog(NULL, hFile);
	Report(NULL, hFile, "");

	/* We can do without these for now.  People keep saying "here's a bug,
	 * tell me if you want the crash dump" which wastes time (of course we
	 * do!).  Let's make the dump smaller to encourage just sending it, which
	 * is what people should be doing.  (If we can use the SF BTS more
	 * consistently, instead of forums, this will be less of an issue.) */
/*	ReportDisasm(NULL, hFile, pExc);
	Report(NULL, hFile, "");

	ReportCrashData(NULL, hFile, pExc);
	Report(NULL, hFile, "");
*/
	Report(NULL, hFile, "-- End of report");
	CloseHandle(hFile);
}

void ViewWithNotepad(const char *str)
{
	char buf[256] = "";
	strcat(buf, "notepad.exe  ");
	strcat(buf, str);

	char cwd[MAX_PATH];
	SpliceProgramPath(cwd, MAX_PATH, "");

	PROCESS_INFORMATION pi;
	STARTUPINFO	si;
	ZeroMemory( &si, sizeof(si) );

	CreateProcess(
		NULL,		// pointer to name of executable module
		buf,		// pointer to command line string
		NULL,  // process security attributes
		NULL,   // thread security attributes
		false,  // handle inheritance flag
		0, // creation flags
		NULL,  // pointer to new environment block
		cwd,   // pointer to current directory name
		&si,  // pointer to STARTUPINFO
		&pi  // pointer to PROCESS_INFORMATION
	);
}

BOOL APIENTRY CrashDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static const EXCEPTION_POINTERS *s_pExc;
	static bool s_bHaveCallstack;

	switch(msg) {

		case WM_INITDIALOG:
			{
				HWND hwndList1 = GetDlgItem(hDlg, IDC_ASMBOX);
				HWND hwndList2 = GetDlgItem(hDlg, IDC_REGDUMP);
				HWND hwndList3 = GetDlgItem(hDlg, IDC_CALL_STACK);
				HWND hwndReason = GetDlgItem(hDlg, IDC_STATIC_BOMBREASON);
				const EXCEPTION_POINTERS *const pExc = (const EXCEPTION_POINTERS *)lParam;

				s_pExc = pExc;

				g_pcdw->DoInitListbox(hwndList1);

				SendMessage(hwndList2, WM_SETFONT, SendMessage(hwndList1, WM_GETFONT, 0, 0), MAKELPARAM(TRUE, 0));
				SendMessage(hwndList3, WM_SETFONT, SendMessage(hwndList1, WM_GETFONT, 0, 0), MAKELPARAM(TRUE, 0));

				ReportReason(NULL, hwndReason, NULL, pExc);
				ReportCrashData(hwndList2, NULL, pExc);
				s_bHaveCallstack = ReportCrashCallStack(hwndList3, NULL, pExc, g_pcdw->vdc.pExtraData);
			}
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDC_BUTTON_CLOSE:
				EndDialog(hDlg, FALSE);
				return TRUE;
			case IDOK:
				EndDialog(hDlg, TRUE);
				return TRUE;
			case IDC_VIEW_LOG:
				ViewWithNotepad("log.txt");
				break;
			case IDC_CRASH_SAVE:
				if (!s_bHaveCallstack)
					if (IDOK != MessageBox(hDlg,
						"StepMania cannot load its crash resource file, and thus the crash dump will be "
						"missing the most important part, the call stack. Crash dumps are much less useful "
						"without the call stack.",
						"StepMania warning", MB_OK|MB_ICONEXCLAMATION))
						return TRUE;

				ViewWithNotepad("crashinfo.txt");
				return TRUE;
			case IDC_BUTTON_RESTART:
				{
					char cwd[MAX_PATH];
					SpliceProgramPath(cwd, MAX_PATH, "");

					TCHAR szFullAppPath[MAX_PATH];
					GetModuleFileName(NULL, szFullAppPath, MAX_PATH);

					// Launch StepMania
					PROCESS_INFORMATION pi;
					STARTUPINFO	si;
					ZeroMemory( &si, sizeof(si) );

					CreateProcess(
						NULL,		// pointer to name of executable module
						szFullAppPath,		// pointer to command line string
						NULL,  // process security attributes
						NULL,   // thread security attributes
						false,  // handle inheritance flag
						0, // creation flags
						NULL,  // pointer to new environment block
						cwd,   // pointer to current directory name
						&si,  // pointer to STARTUPINFO
						&pi  // pointer to PROCESS_INFORMATION
					);
				}
				EndDialog( hDlg, FALSE );
				break;
			case IDC_BUTTON_REPORT:
				GotoURL( "http://sourceforge.net/tracker/?func=add&group_id=37892&atid=421366" );
				break;
			}
			break;

		case WM_MEASUREITEM:
			return g_pcdw->DoMeasureItem(lParam);

		case WM_DRAWITEM:
			return g_pcdw->DoDrawItem(lParam);
	}

	return FALSE;
}

void crash() {
	__try {
		__asm xor ebx,ebx
		__asm mov eax,dword ptr [ebx]
//		__asm mov dword ptr [ebx],eax
//		__asm lock add dword ptr cs:[00000000h], 12345678h
	} __except(CrashHandler((EXCEPTION_POINTERS*)_exception_info())) {
	}
}

/* RageLog feeds us logs; keep track of recent logs in a circular
 * buffer.  This lets us include recent logs in the crash report.
 * We don't want to include RageLog, since we don't want to pull
 * in MFC.
 *
 * Since this is a static, preallocated region, even if it gets
 * trampled we'll just print garbage, not crash due to an invalid
 * pointer. */
static const int BACKLOG_LINES = 10;
static char backlog[BACKLOG_LINES][1024];
static int backlog_start=0, backlog_cnt=0;

void CrashLog(const char *str)
{
	int len = strlen(str);
	if(len > sizeof(backlog[backlog_start])-1)
		len = sizeof(backlog[backlog_start])-1;

	strncpy(backlog[backlog_start], str, len);
	backlog[backlog_start] [ len ] = 0;

	backlog_start++;
	if(backlog_start > backlog_cnt)
		backlog_cnt=backlog_start;
	backlog_start %= BACKLOG_LINES;
}

static void ReportCrashLog(HWND hwnd, HANDLE hFile)
{
	Report(NULL, hFile, "Partial log:");

	int i;
	for(i = backlog_start; i < BACKLOG_LINES && i < backlog_cnt; ++i)
		Report(hwnd, hFile, "%s", backlog[i]);
	for(i = 0; i < backlog_start && i < backlog_cnt; ++i)
		Report(hwnd, hFile, "%s", backlog[i]);
}

/* Same idea, except this is for data that we *always* want to print
 * in the crash log, even if it was printed when we started. */
static const int STATICLOG_SIZE = 1024*32;
static char staticlog[STATICLOG_SIZE];
static char *staticlog_ptr=staticlog, *staticlog_end=staticlog+STATICLOG_SIZE;
void StaticLog(const char *str)
{
	if(!staticlog_ptr)
		return;

	int len = strlen(str)+2; /* +newline +null */
	if(staticlog_ptr+len >= staticlog_end)
	{
		const char *txt = "\nStaticlog limit reached\n";
		char *max_ptr = staticlog_end-strlen(txt)-1;
		if(staticlog_ptr > max_ptr)
			staticlog_ptr = max_ptr;

		strcpy(staticlog_ptr, txt);
		staticlog_ptr=NULL; /* stop */
		return;
	}

	strcpy(staticlog_ptr, str);

	staticlog_ptr[len-2] = '\n';
	staticlog_ptr[len-1] = 0;
	/* Advance to sit on the NULL, so the terminator will be overwritten
	 * on the next log. */
	staticlog_ptr += len-1;
}

static void ReportStaticLog(HWND hwnd, HANDLE hFile)
{
	Report(NULL, hFile, "Static log:");
	Report(hwnd, hFile, "%s", staticlog);
}

