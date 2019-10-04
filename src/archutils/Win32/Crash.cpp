#include "global.h"

// DO NOT USE stdio.h!  printf() calls malloc()!
//#include <stdio.h>

#include <windows.h>

#include "global.h"
#include "arch/Threads/Threads_Win32.h"
#include "crash.h"
#include "CrashHandlerInternal.h"
#include "RageLog.h" // for RageLog::GetAdditionalLog and Flush
#include "RageThreads.h" // for GetCheckpointLogs
#include "PrefsManager.h" // for g_bAutoRestart
#include "RestartProgram.h"

// WARNING: This is called from crash-time conditions!  No malloc() or new!!!

#define malloc not_allowed_here
#define new not_allowed_here

static void SpliceProgramPath(char *buf, int bufsiz, const char *fn) {
	char tbuf[MAX_PATH];
	char *pszFile;

	GetModuleFileName(nullptr, tbuf, sizeof tbuf);
	GetFullPathName(tbuf, bufsiz, buf, &pszFile);
	strcpy(pszFile, fn);
}

///////////////////////////////////////////////////////////////////////////

static const struct ExceptionLookup {
	DWORD	code;
	const char *name;
} exceptions[]={
	{ EXCEPTION_ACCESS_VIOLATION,		"Access Violation"		},
	{ EXCEPTION_BREAKPOINT,			"Breakpoint"			},
	{ EXCEPTION_FLT_DENORMAL_OPERAND,	"FP Denormal Operand"		},
	{ EXCEPTION_FLT_DIVIDE_BY_ZERO,		"FP Divide-by-Zero"		},
	{ EXCEPTION_FLT_INEXACT_RESULT,		"FP Inexact Result"		},
	{ EXCEPTION_FLT_INVALID_OPERATION,	"FP Invalid Operation"		},
	{ EXCEPTION_FLT_OVERFLOW,		"FP Overflow",			},
	{ EXCEPTION_FLT_STACK_CHECK,		"FP Stack Check",		},
	{ EXCEPTION_FLT_UNDERFLOW,		"FP Underflow",			},
	{ EXCEPTION_INT_DIVIDE_BY_ZERO,		"Integer Divide-by-Zero",	},
	{ EXCEPTION_INT_OVERFLOW,		"Integer Overflow",		},
	{ EXCEPTION_PRIV_INSTRUCTION,		"Privileged Instruction",	},
	{ EXCEPTION_ILLEGAL_INSTRUCTION,	"Illegal instruction"		},
	{ EXCEPTION_INVALID_HANDLE,		"Invalid handle"		},
	{ EXCEPTION_STACK_OVERFLOW,		"Stack overflow"		},
	{ 0xe06d7363,				"Unhandled Microsoft C++ Exception",	},
	{ 0 },
};

static const char *LookupException( DWORD code )
{
	for( int i = 0; exceptions[i].code; ++i )
		if( exceptions[i].code == code )
			return exceptions[i].name;

	return nullptr;
}

static CrashInfo g_CrashInfo;
static void GetReason( const EXCEPTION_RECORD *pRecord, CrashInfo *crash )
{
	// fill out bomb reason
	const char *reason = LookupException( pRecord->ExceptionCode );

	if( reason == nullptr )
		wsprintf( crash->m_CrashReason, "unknown exception 0x%08lx", pRecord->ExceptionCode );
	else
		strcpy( crash->m_CrashReason, reason );
}

static HWND g_hForegroundWnd = nullptr;
void CrashHandler::SetForegroundWindow( HWND hWnd )
{
	g_hForegroundWnd = hWnd;
}

void WriteToChild( HANDLE hPipe, const void *pData, size_t iSize )
{
	while( iSize )
	{
		DWORD iActual;
		if( !WriteFile(hPipe, pData, static_cast<DWORD>(iSize), &iActual, nullptr) )
			return;
		iSize -= iActual;
	}
}

/* Execute the child process. Return a handle to the process, a writable handle
 * to its stdin, and a readable handle to its stdout. */
bool StartChild( HANDLE &hProcess, HANDLE &hToStdin, HANDLE &hFromStdout )
{
	char cwd[MAX_PATH];
	SpliceProgramPath( cwd, MAX_PATH, "" );

	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.dwFlags |= STARTF_USESTDHANDLES;

	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = true;
		sa.lpSecurityDescriptor = nullptr;

		CreatePipe( &si.hStdInput, &hToStdin, &sa, 0 );
		CreatePipe( &hFromStdout, &si.hStdOutput, &sa, 0 );
		SetHandleInformation( hToStdin, HANDLE_FLAG_INHERIT, 0 );
		SetHandleInformation( hFromStdout, HANDLE_FLAG_INHERIT, 0 );
	}

	char szBuf[MAX_PATH] = "";
	GetModuleFileName( nullptr, szBuf, MAX_PATH );
	strcat( szBuf, " " );
	strcat( szBuf, CHILD_MAGIC_PARAMETER );

	PROCESS_INFORMATION pi;
	int iRet = CreateProcess(
		nullptr,		// pointer to name of executable module
		szBuf,		// pointer to command line string
		nullptr,		// process security attributes
		nullptr,		// thread security attributes
		true,		// handle inheritance flag
		0,		// creation flags
		nullptr,		// pointer to new environment block
		cwd,		// pointer to current directory name
		&si,		// pointer to STARTUPINFO
		&pi		// pointer to PROCESS_INFORMATION
	);

	CloseHandle( si.hStdInput );
	CloseHandle( si.hStdOutput );

	if( !iRet )
	{
		CloseHandle( hToStdin );
		CloseHandle( hFromStdout );
		return false;
	}

	hProcess = pi.hProcess;
	CloseHandle(pi.hThread);

	return true;
}

static const char *CrashGetModuleBaseName(HMODULE hmod, char *pszBaseName)
{
	char szPath1[MAX_PATH];
	char szPath2[MAX_PATH];

// XXX: It looks like nothing in here COULD throw an exception. Need to verify that.
//	__try {
		if( !GetModuleFileName(hmod, szPath1, sizeof(szPath1)) )
			return nullptr;

		char *pszFile;
		DWORD dw = GetFullPathName( szPath1, sizeof(szPath2), szPath2, &pszFile );

		if( !dw || dw > sizeof(szPath2) )
			return nullptr;

		strcpy( pszBaseName, pszFile );

		pszFile = pszBaseName;

		char *period = nullptr;
		while( *pszFile++ )
			if( pszFile[-1]=='.' )
				period = pszFile-1;

		if( period )
			*period = 0;
//	} __except(1) {
//		return nullptr;
//	}

	return pszBaseName;
}

void RunChild()
{
	HANDLE hProcess, hToStdin, hFromStdout;
	StartChild( hProcess, hToStdin, hFromStdout );

	// 0. Send a handle of this process to the crash handling process, which it
	// can use to handle symbol lookups.
	{
		HANDLE hTargetHandle;
		DuplicateHandle(
			GetCurrentProcess(),
			GetCurrentProcess(),
			hProcess,
			&hTargetHandle,
			0,
			false,
			DUPLICATE_SAME_ACCESS
		);

		WriteToChild( hToStdin, &hTargetHandle, sizeof(hTargetHandle) );
	}

	// 1. Write the CrashData.
	WriteToChild( hToStdin, &g_CrashInfo, sizeof(g_CrashInfo) );

		// 2. Write info.
		const TCHAR *p = RageLog::GetInfo();
		int iSize = static_cast<int>(strlen( p ));
		WriteToChild( hToStdin, &iSize, sizeof(iSize) );
		WriteToChild( hToStdin, p, iSize );

		// 3. Write AdditionalLog.
		p = RageLog::GetAdditionalLog();
		iSize = static_cast<int>(strlen( p ));
		WriteToChild( hToStdin, &iSize, sizeof(iSize) );
		WriteToChild( hToStdin, p, iSize );

		// 4. Write RecentLogs.
		int cnt = 0;
		const TCHAR *ps[1024];
		while( cnt < 1024 && (ps[cnt] = RageLog::GetRecentLog( cnt )) != nullptr )
				++cnt;

		WriteToChild(hToStdin, &cnt, sizeof(cnt));
		for( int i = 0; i < cnt; ++i )
		{
				iSize = static_cast<int>(strlen(ps[i])) + 1;
				WriteToChild( hToStdin, &iSize, sizeof(iSize) );
				WriteToChild( hToStdin, ps[i], iSize );
		}

		// 5. Write CHECKPOINTs.
		static TCHAR buf[1024*32];
		Checkpoints::GetLogs( buf, sizeof(buf), "$$" );
		iSize = static_cast<int>(strlen( buf )) + 1;
		WriteToChild( hToStdin, &iSize, sizeof(iSize) );
		WriteToChild( hToStdin, buf, iSize );

		// 6. Write the crashed thread's name.
		p = RageThread::GetCurrentThreadName();
		iSize = static_cast<int>(strlen( p )) + 1;
		WriteToChild( hToStdin, &iSize, sizeof(iSize) );
		WriteToChild( hToStdin, p, iSize );

	/* The parent process needs to access this process briefly. When it's done,
	 * it'll close the handle. Wait until we see that before exiting. */
	while(1)
	{
		/* Ugly: the new process can't execute GetModuleFileName on this process,
		 * since GetModuleFileNameEx might not be available. Run the requests here. */
		HMODULE hMod;
		DWORD iActual;
		if( !ReadFile( hFromStdout, &hMod, sizeof(hMod), &iActual, nullptr) )
			break;

		TCHAR szName[MAX_PATH];
		if( !CrashGetModuleBaseName(hMod, szName) )
			strcpy( szName, "???" );
		iSize = static_cast<int>(strlen( szName ));
		WriteToChild( hToStdin, &iSize, sizeof(iSize) );
		WriteToChild( hToStdin, szName, iSize );
	}
}

static DWORD WINAPI MainExceptionHandler( LPVOID lpParameter )
{
	// Flush the log so it isn't cut off at the end.
	/* 1. We can't do regular file access in the crash handler.
	 * 2. We can't access LOG itself at all, since it may not be set up or the
	 * pointer might be munged. We must only ever use the RageLog:: methods that
	 * access static data, that we're being very careful to null-terminate as needed.
	 * Logs are rarely important, anyway. Only info.txt and crashinfo.txt are
	 * needed 99% of the time. */
//	LOG->Flush();

	/* We aren't supposed to receive these exceptions. For example, if you do
	 * a floating point divide by zero, you should receive a result of #INF.
	 * Only if the floating point exception for _EM_ZERODIVIDE is unmasked
	 * does this exception occur, and we never unmask it.
	 * However, once in a while some driver or library turns evil and unmasks an
	 * exception flag on us. If this happens, re-mask it and continue execution. */
	PEXCEPTION_POINTERS pExc = reinterpret_cast<PEXCEPTION_POINTERS>(lpParameter);
	switch( pExc->ExceptionRecord->ExceptionCode )
	{
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_FLT_INEXACT_RESULT:
#if _WIN64
		pExc->ContextRecord->FltSave.ControlWord |= 0x3F;
#else
		pExc->ContextRecord->FloatSave.ControlWord |= 0x3F;
#endif
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	static int InHere = 0;
	if( InHere > 0 )
	{
		/* If we get here, then we've been called recursively, which means we
		 * crashed. If InHere is greater than 1, then we crashed after writing
		 * the crash dump; say so. */
		SetUnhandledExceptionFilter(nullptr);
		MessageBox( nullptr,
			InHere == 1?
			"The error reporting interface has crashed.\n":
			"The error reporting interface has crashed. However, crashinfo.txt was"
			"written successfully to the program directory.\n",
			"Fatal Error", MB_OK );
#ifdef DEBUG
		DebugBreak();
#endif

		return EXCEPTION_EXECUTE_HANDLER;
	}
	++InHere;
	/////////////////////////

	RageThread::HaltAllThreads( false );

	if( !g_CrashInfo.m_CrashReason[0] )
		GetReason( pExc->ExceptionRecord, &g_CrashInfo );
	CrashHandler::do_backtrace( g_CrashInfo.m_BacktracePointers, BACKTRACE_MAX_SIZE, GetCurrentProcess(),  GetCurrentThread(), pExc->ContextRecord );

	RunChild();

	++InHere;

	if( g_bAutoRestart )
		Win32RestartProgram();

	/* Now things get more risky. If we're fullscreen, the window will obscure
	 * the crash dialog. Try to hide the window. Things might blow up here; do
	 * this after DoSave, so we always write a crash dump. */
	if( GetWindowThreadProcessId( g_hForegroundWnd, nullptr ) == GetCurrentThreadId() )
	{
		/* The thread that crashed was the thread that created the main window.
		 * Hide the window. This will also restore the video mode, if necessary. */
		ShowWindow( g_hForegroundWnd, SW_HIDE );
	} else {
		/* A different thread crashed. Simply kill all other windows. We can't
		 * safely call ShowWindow; the main thread might be deadlocked. */
		RageThread::HaltAllThreads( true );
		ChangeDisplaySettings( nullptr, 0 );
	}

	InHere = false;

	SetUnhandledExceptionFilter(nullptr);

	/* Forcibly terminate; if we keep going, we'll try to shut down threads and
	 * do other things that may deadlock, which is confusing for users. */
	TerminateProcess( GetCurrentProcess(), 0 );

	return EXCEPTION_EXECUTE_HANDLER;
}

long __stdcall CrashHandler::ExceptionHandler( EXCEPTION_POINTERS *pExc )
{
	/* If the stack overflowed, we have a very limited amount of stack space.
	 * Allocate a new stack, and run the exception handler in it, to increase
	 * the chances of success. */
	HANDLE hExceptionHandler = CreateThread(nullptr, 1024 * 32, MainExceptionHandler, reinterpret_cast<LPVOID>(pExc), 0, nullptr);
	if (hExceptionHandler == NULL)
	{
		TerminateProcess(GetCurrentProcess(), 0);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	WaitForSingleObject(hExceptionHandler, INFINITE);

	DWORD ret;
	GetExitCodeThread(hExceptionHandler, &ret);
	CloseHandle(hExceptionHandler);

	return static_cast<long>(ret);
}

//////////////////////////////////////////////////////////////////////////////

static bool IsValidCall(char *buf, int len)
{
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

	if (len >= 2 && buf[-2] == '\xff' && (buf[-1]&0xF8) == '\xd0')
		return true;

	// FF 50-57 xx				CALL [reg32+reg32*scale+disp8]

	if (len >= 3 && buf[-3] == '\xff' && (buf[-2]&0xF8) == '\x50')
		return true;

	// FF 90-97 xx xx xx xx xx	CALL [reg32+reg32*scale+disp32]

	if (len >= 7 && buf[-7] == '\xff' && (buf[-6]&0xF8) == '\x90')
		return true;

	return false;
}

static bool IsExecutableProtection(DWORD dwProtect) {
	MEMORY_BASIC_INFORMATION meminfo;

	// Windows NT/2000 allows Execute permissions, but Win9x seems to rip it
	// off. So we query the permissions on our own code block, and use it to
	// determine if READONLY/READWRITE should be considered 'executable.'
	// XXX: Special logic for Win98? Really? This should be cut.

	VirtualQuery( (LPCVOID) IsExecutableProtection, &meminfo, sizeof meminfo);

	switch((unsigned char)dwProtect) {
	case PAGE_READONLY:			// *sigh* Win9x...
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

static bool PointsToValidCall( ULONG_PTR ptr )
{
	char buf[7];
	int len = 7;

	memset( buf, 0, sizeof(buf) );

	while(len > 0 && !ReadProcessMemory(GetCurrentProcess(), (void *)(ptr-len), buf+7-len, len, nullptr))
		--len;

	return IsValidCall(buf+7, len);
}

void CrashHandler::do_backtrace( const void **buf, size_t size, 
						 HANDLE hProcess, HANDLE hThread, const CONTEXT *pContext )
{
	const void **pLast = buf + size - 1;
	bool bFirst = true;

	/* The EIP of the position that crashed is normally on the stack, since the
	 * exception handler was called on the same stack. However, once in a while,
	 * due to stack corruption, we might not be able to get any frames from the
	 * stack. Pull it out of pContext->Eip, which is always valid, and then
	 * discard the first stack frame if it's the same. */
#if _WIN64
	if( buf+1 != pLast && pContext->Rip != 0 )
	{
		*buf = (void *) pContext->Rip;
#else
	if( buf+1 != pLast && pContext->Eip != 0 )
	{
		*buf = (void *) pContext->Eip;
#endif
		++buf;
	}

	// Retrieve stack pointers.
	const char *pStackBase;
	{
		LDT_ENTRY sel;
		if( !GetThreadSelectorEntry( hThread, pContext->SegFs, &sel ) )
		{
			*buf = nullptr;
			return;
		}

		const NT_TIB *tib = reinterpret_cast<NT_TIB *>(((static_cast<DWORD_PTR>(sel.HighWord.Bits.BaseHi) << 24) + (static_cast<DWORD_PTR>(sel.HighWord.Bits.BaseMid) << 16) + sel.BaseLow));
		const NT_TIB *pTib = tib->Self;
		pStackBase = (char *)pTib->StackBase;
	}

	// Walk up the stack.
#if _WIN64
	const char *lpAddr = (const char *)pContext->Rsp;

	const void *data = (void *) pContext->Rip;
#else
	const char *lpAddr = (const char *)pContext->Esp;

	const void *data = (void *) pContext->Eip;
#endif
	do {
		if( buf == pLast )
			break;

		bool fValid = true;

		/* The first entry is usually EIP.  We already logged it; skip it, so we don't always
		 * show the first frame twice. */
#if _WIN64
		if( bFirst && data == (void *) pContext->Rip )
#else
		if( bFirst && data == (void *) pContext->Eip )
#endif
			fValid = false;
		bFirst = false;

		{
			MEMORY_BASIC_INFORMATION meminfo;

			VirtualQuery((void *)data, &meminfo, sizeof meminfo);
			
			if (!IsExecutableProtection(meminfo.Protect) || meminfo.State!=MEM_COMMIT)
				fValid = false;

#if _WIN64
			if ( data != (void *) pContext->Rip && !PointsToValidCall(reinterpret_cast<ULONG_PTR>(data)) )
#else
			if ( data != (void *) pContext->Eip && !PointsToValidCall(reinterpret_cast<ULONG_PTR>(data)) )
#endif
				fValid = false;
		}

		if( fValid )
		{
			*buf = data;
			++buf;
		}

		if (lpAddr >= pStackBase)
			break;

		lpAddr += 4;
	} while( ReadProcessMemory(hProcess, lpAddr-4, &data, 4, nullptr));

	*buf = nullptr;
}

// Trigger the crash handler. This works even in the debugger.
static void NORETURN debug_crash()
{
//	__try {
#if defined(__MSC_VER)
		__asm xor ebx,ebx
		__asm mov eax,dword ptr [ebx]
//		__asm mov dword ptr [ebx],eax
//		__asm lock add dword ptr cs:[00000000h], 12345678h
#elif defined(__GNUC__)
		// HACK: I don't know a lick of ASM and don't know how to port this to
		// the AT&T syntax gas uses. So we temporarily change it.
		asm(".intel_syntax noprefix\n\t"
			"xor ebx,ebx\n\t"
			"mov eax,dword ptr [ebx]\n\t"
//			"mov dword ptr [ebx],eax"
//			"lock add dword ptr cs:[00000000h], 12345678h"
			".att_syntax noprefix\n\t");
#endif
//	} __except( CrashHandler::ExceptionHandler((EXCEPTION_POINTERS*)_exception_info()) ) {
//	}
}

/* Get a stack trace of the current thread and the specified thread.
 * If iID == GetInvalidThreadId(), then output a stack trace for every thread. */
void CrashHandler::ForceDeadlock( RString reason, uint64_t iID )
{
	strncpy( g_CrashInfo.m_CrashReason, reason, sizeof(g_CrashInfo.m_CrashReason) );
	g_CrashInfo.m_CrashReason[ sizeof(g_CrashInfo.m_CrashReason)-1 ] = 0;

	/* Suspend the other thread we're going to backtrace. (We need to at least
	 * suspend hThread, for GetThreadContext to work.) */
	RageThread::HaltAllThreads( false );

	if( iID == GetInvalidThreadId() )
	{
		// Backtrace all threads.
		int iCnt = 0;
		for( int i = 0; RageThread::EnumThreadIDs(i, iID); ++i )
		{
			if( iID == GetInvalidThreadId() )
				continue;

			if( iID == GetCurrentThreadId() )
				continue;

			const HANDLE hThread = Win32ThreadIdToHandle( iID );

			CONTEXT context;
			context.ContextFlags = CONTEXT_FULL;
			if( !GetThreadContext( hThread, &context ) )
				wsprintf( g_CrashInfo.m_CrashReason + strlen(g_CrashInfo.m_CrashReason),
					"; GetThreadContext(%Ix) failed", reinterpret_cast<uintptr_t>(hThread) );
			else
			{
				static const void *BacktracePointers[BACKTRACE_MAX_SIZE];
				do_backtrace( g_CrashInfo.m_AlternateThreadBacktrace[iCnt], BACKTRACE_MAX_SIZE, GetCurrentProcess(), hThread, &context );

				const char *pName = RageThread::GetThreadNameByID( iID );
				strncpy( g_CrashInfo.m_AlternateThreadName[iCnt], pName? pName:"???", sizeof(g_CrashInfo.m_AlternateThreadName[iCnt])-1 );

				++iCnt;
			}

			if( iCnt == CrashInfo::MAX_BACKTRACE_THREADS )
				break;
		}
	} else {
		const HANDLE hThread = Win32ThreadIdToHandle( iID );

		CONTEXT context;
		context.ContextFlags = CONTEXT_FULL;
		if( !GetThreadContext( hThread, &context ) )
			strcat( g_CrashInfo.m_CrashReason, "(GetThreadContext failed)" );
		else
		{
			static const void *BacktracePointers[BACKTRACE_MAX_SIZE];
			do_backtrace( g_CrashInfo.m_AlternateThreadBacktrace[0], BACKTRACE_MAX_SIZE, GetCurrentProcess(), hThread, &context );

			const char *pName = RageThread::GetThreadNameByID( iID );
			strncpy( g_CrashInfo.m_AlternateThreadName[0], pName? pName:"???", sizeof(g_CrashInfo.m_AlternateThreadName[0])-1 );
		}
	}

	debug_crash();
}

void CrashHandler::ForceCrash( const char *reason )
{
	strncpy( g_CrashInfo.m_CrashReason, reason, sizeof(g_CrashInfo.m_CrashReason) );
	g_CrashInfo.m_CrashReason[ sizeof(g_CrashInfo.m_CrashReason)-1 ] = 0;

	debug_crash();
}

/*
 * (c) 1998-2001 Avery Lee
 * (c) 2003-2004 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

