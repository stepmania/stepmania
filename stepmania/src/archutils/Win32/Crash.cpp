#include "global.h"

// DO NOT USE stdio.h!  printf() calls malloc()!
//#include <stdio.h>
#include <stdarg.h>
#include <crtdbg.h>

#include <windows.h>
#include <tlhelp32.h>

#include "arch/Threads/Threads_Win32.h"
#include "archutils/win32/WindowsResources.h"
#include "crash.h"
#include "ProductInfo.h"

#include "RageLog.h" /* for RageLog::GetAdditionalLog and Flush */
#include "RageThreads.h" /* for GetCheckpointLogs */
#include "PrefsManager.h" /* for g_bAutoRestart */
#include "RestartProgram.h"

#include "GotoURL.h"

static HFONT hFontMono = NULL;

static void DoSave();

///////////////////////////////////////////////////////////////////////////

#define CODE_WINDOW (256)

///////////////////////////////////////////////////////////////////////////

extern HINSTANCE g_hInstance;
extern unsigned long version_num;



extern HINSTANCE g_hInstance;
#define BACKTRACE_MAX_SIZE 100

// WARNING: This is called from crash-time conditions!  No malloc() or new!!!

#define malloc not_allowed_here
#define new not_allowed_here


static void GetVDIPath( char *buf, int bufsiz )
{
	GetModuleFileName( NULL, buf, bufsiz );
	buf[bufsiz-5] = 0;
	char *p = strrchr( buf, '.' );
	if( p )
		strcpy( p, ".vdi" );
	else
		strcat( buf, ".vdi" );
}

static void SpliceProgramPath(char *buf, int bufsiz, const char *fn) {
	char tbuf[MAX_PATH];
	char *pszFile;

	GetModuleFileName(NULL, tbuf, sizeof tbuf);
	GetFullPathName(tbuf, bufsiz, buf, &pszFile);
	strcpy(pszFile, fn);
}

struct VDDebugInfoContext
{
	VDDebugInfoContext() { pRVAHeap=NULL; }
	bool Loaded() const { return pRVAHeap != NULL; }
	void *pRawBlock;

	int nBuildNumber;

	const unsigned char *pRVAHeap;
	unsigned	nFirstRVA;

	const char *pFuncNameHeap;
	const unsigned long (*pSegments)[2];
	int		nSegments;
	char	sFilename[1024];
};


static VDDebugInfoContext g_debugInfo;

BOOL APIENTRY CrashDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);


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

static const char *LookupException( DWORD code )
{
	for( int i = 0; exceptions[i].code; ++i )
		if( exceptions[i].code == code )
			return exceptions[i].name;

	return NULL;
}

struct CrashInfo
{
	char m_CrashReason[1024*8];

	const void *m_BacktracePointers[BACKTRACE_MAX_SIZE];

	enum { MAX_BACKTRACE_THREADS = 32 };
	const void *m_AlternateThreadBacktrace[MAX_BACKTRACE_THREADS][BACKTRACE_MAX_SIZE];
	char m_AlternateThreadName[MAX_BACKTRACE_THREADS][128];

	CrashInfo()
	{
		m_CrashReason[0] = 0;
		memset( m_AlternateThreadBacktrace, 0, sizeof(m_AlternateThreadBacktrace) );
		memset( m_AlternateThreadName, 0, sizeof(m_AlternateThreadName) );
		m_BacktracePointers[0] = NULL;
	}
};

static CrashInfo g_CrashInfo;
static void GetReason( const EXCEPTION_RECORD *pRecord, CrashInfo *crash )
{
	// fill out bomb reason
	const char *reason = LookupException( pRecord->ExceptionCode );

	if( reason == NULL )
		wsprintf( crash->m_CrashReason, "Crash reason: unknown exception 0x%08lx", pRecord->ExceptionCode );
	else
	{
		strcpy( crash->m_CrashReason, "Crash reason: " );
		strcat( crash->m_CrashReason, reason );
	}
}

long VDDebugInfoLookupRVA(VDDebugInfoContext *pctx, unsigned rva, char *buf, int buflen);
bool VDDebugInfoInitFromMemory(VDDebugInfoContext *pctx, const void *_src);
bool VDDebugInfoInitFromFile( VDDebugInfoContext *pctx );
void VDDebugInfoDeinit(VDDebugInfoContext *pctx);


extern HWND g_hWndMain;
long __stdcall CrashHandler(EXCEPTION_POINTERS *pExc)
{
	/* Flush the log it isn't cut off at the end. */
	/* 1. We can't do regular file access in the crash handler.
	 * 2. We can't access LOG itself at all, since it may not be set up or the pointer might
	 * be munged.  We must only ever use the RageLog:: methods that access static data, that
	 * we're being very careful to null-terminate as needed.
	 *
	 * Logs are rarely important, anyway.  Only info.txt and crashinfo.txt are needed 99%
	 * of the time. */
//	LOG->Flush();

	/* We aren't supposed to receive these exceptions.  For example, if you do
	 * a floating point divide by zero, you should receive a result of #INF.  Only
	 * if the floating point exception for _EM_ZERODIVIDE is unmasked does this
	 * exception occur, and we never unmask it.
	 *
	 * However, once in a while some driver or library turns evil and unmasks an
	 * exception flag on us.  If this happens, re-mask it and continue execution. */
	switch( pExc->ExceptionRecord->ExceptionCode )
	{
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_FLT_INEXACT_RESULT:
		pExc->ContextRecord->FloatSave.ControlWord |= 0x3F;
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	static int InHere = 0;
	if( InHere > 0 )
	{
		/* If we get here, then we've been called recursively, which means we crashed.
		 * If InHere is greater than 1, then we crashed after writing the crash dump;
		 * say so. */
		SetUnhandledExceptionFilter(NULL);
		MessageBox( NULL,
			InHere == 1?
			"The error reporting interface has crashed.\n":
			"The error reporting interface has crashed.  However, crashinfo.txt was"
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

	hFontMono = CreateFont(
			10,				// nHeight
			0,				// nWidth
			0,				// nEscapement
			0,				// nOrientation
			FW_DONTCARE,	// fnWeight
			FALSE,			// fdwItalic
			FALSE,			// fdwUnderline
			FALSE,			// fdwStrikeOut
			ANSI_CHARSET,	// fdwCharSet
			OUT_DEFAULT_PRECIS,		// fdwOutputPrecision
			CLIP_DEFAULT_PRECIS,	// fdwClipPrecision
			DEFAULT_QUALITY,		// fdwQuality
			DEFAULT_PITCH | FF_DONTCARE,	// fdwPitchAndFamily
			"Lucida Console"
			);

	if (!hFontMono)
		hFontMono = (HFONT)GetStockObject(ANSI_FIXED_FONT);

	// Attempt to read debug file.

	VDDebugInfoInitFromFile( &g_debugInfo );

	/* In case something goes amiss before the user can view the crash
	 * dump, save it now. */
	if( !g_CrashInfo.m_CrashReason[0] )
		GetReason( pExc->ExceptionRecord, &g_CrashInfo );
	do_backtrace( g_CrashInfo.m_BacktracePointers, BACKTRACE_MAX_SIZE, GetCurrentProcess(),  GetCurrentThread(), pExc->ContextRecord );
	DoSave();

	++InHere;

	/* Now things get more risky.  If we're fullscreen, the window will obscure the
	 * crash dialog.  Try to hide the window.  Things might blow up here; do this
	 * after DoSave, so we always write a crash dump. */
	if( GetWindowThreadProcessId( g_hWndMain, NULL ) == GetCurrentThreadId() )
	{
		/* The thread that crashed was the thread that created the main window.  Hide
		 * the window.  This will also restore the video mode, if necessary. */
		ShowWindow( g_hWndMain, SW_HIDE );
	} else {
		/* A different thread crashed.  Simply kill all other windows.  We can't safely
		 * call ShowWindow; the main thread might be deadlocked. */
		RageThread::HaltAllThreads( true );
		ChangeDisplaySettings(NULL, 0);
	}

	if( g_bAutoRestart )
		Win32RestartProgram();

	/* Little trick to get an HINSTANCE of ourself without having access to the hwnd ... */
	{
		TCHAR szFullAppPath[MAX_PATH];
		GetModuleFileName(NULL, szFullAppPath, MAX_PATH);
		HINSTANCE handle = LoadLibrary(szFullAppPath);

		DialogBoxParam(handle, MAKEINTRESOURCE(IDD_DISASM_CRASH), NULL, CrashDlgProc, (LPARAM)pExc);
	}

	VDDebugInfoDeinit(&g_debugInfo);

	InHere = false;

	SetUnhandledExceptionFilter(NULL);

	/* Forcibly terminate; if we keep going, we'll try to shut down threads and do other
	 * things that may deadlock, which is confusing for users. */
	TerminateProcess( GetCurrentProcess(), 0 );

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



static void ReportReason( HWND hwndReason, HANDLE hFile, const CrashInfo *pCrash )
{
	if( hwndReason )
		SetWindowText( hwndReason, pCrash->m_CrashReason );

	if( hFile )
		Report( NULL, hFile, pCrash->m_CrashReason );
}

static const char *GetNameFromHeap(const char *heap, int idx)
{
	while(idx--)
		while(*heap++);

	return heap;
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

	if (memcmp((char *)src, "StepMania symbolic debug information", 36))
		return false;

	// Extract fields

	src += 64;

	pctx->nBuildNumber		= *(int *)src;
	pctx->pRVAHeap			= (const unsigned char *)(src + 20);
	pctx->nFirstRVA			= *(const long *)(src + 16);
	pctx->pFuncNameHeap		= (const char *)pctx->pRVAHeap - 4 + *(const long *)(src + 4);
	pctx->pSegments			= (unsigned long (*)[2])(pctx->pFuncNameHeap + *(const long *)(src + 8));
	pctx->nSegments			= *(const long *)(src + 12);

	return true;
}

void VDDebugInfoDeinit(VDDebugInfoContext *pctx) {
	if (pctx->pRawBlock) {
		VirtualFree(pctx->pRawBlock, 0, MEM_RELEASE);
		pctx->pRawBlock = NULL;
	}
}

bool VDDebugInfoInitFromFile( VDDebugInfoContext *pctx )
{
	if( pctx->Loaded() )
		return true;

	GetVDIPath( pctx->sFilename, sizeof(pctx->sFilename) );

	pctx->pRawBlock = NULL;
	pctx->pRVAHeap = NULL;

	HANDLE h = CreateFile(pctx->sFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == h)
		return false;

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

		VirtualFree(pctx->pRawBlock, 0, MEM_RELEASE);

	} while(false);

	VDDebugInfoDeinit(pctx);
	CloseHandle(h);
	return false;
}

static bool PointerIsInAnySegment( const VDDebugInfoContext *pctx, unsigned rva )
{
	for( int i=0; i<pctx->nSegments; ++i )
	{
		if (rva >= pctx->pSegments[i][0] && rva < pctx->pSegments[i][0] + pctx->pSegments[i][1])
			return true;
	}

	return false;
}

long VDDebugInfoLookupRVA(VDDebugInfoContext *pctx, unsigned rva, char *buf, int buflen)
{
	if ( !PointerIsInAnySegment(pctx, rva) )
		return -1;

	const unsigned char *pr = pctx->pRVAHeap;
	const unsigned char *pr_limit = (const unsigned char *)pctx->pFuncNameHeap;
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
	if (pr >= pr_limit)
		return -1;

	// Decompress name for RVA
	const char *fn_name = GetNameFromHeap(pctx->pFuncNameHeap, idx);

	if (!*fn_name)
		fn_name = "(special)";

	strncpy( buf, fn_name, buflen );
	buf[buflen-1]=0;

	return rva;
}

///////////////////////////////////////////////////////////////////////////
#include "archutils/Win32/ddk/dbghelp.h"
#pragma comment(lib, "archutils/Win32/ddk/dbghelp.lib")

static bool InitDbghelp()
{
	static bool initted = false;
	if( !initted )
	{
		SymSetOptions( SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS );

		if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
			return false;

		initted = true;
	}

	return true;
}

static SYMBOL_INFO *GetSym( unsigned long ptr, DWORD64 &disp )
{
	InitDbghelp();

	static BYTE buffer[1024];
	SYMBOL_INFO *pSymbol = (PSYMBOL_INFO)buffer;

	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = sizeof(buffer) - sizeof(SYMBOL_INFO) + 1;

	if (!SymFromAddr(GetCurrentProcess(), ptr, &disp, pSymbol))
		return NULL;

	return pSymbol;
}

static const char *Demangle( const char *buf )
{
	if( !InitDbghelp() )
		return buf;

	static char obuf[1024];
	if( !UnDecorateSymbolName(buf, obuf, sizeof(obuf),
		UNDNAME_COMPLETE
		| UNDNAME_NO_CV_THISTYPE
		| UNDNAME_NO_ALLOCATION_MODEL
		| UNDNAME_NO_ACCESS_SPECIFIERS // no public:
		| UNDNAME_NO_MS_KEYWORDS // no __cdecl 
		) )
	{
		return buf;
	}

	if( obuf[0] == '_' )
	{
		strcat( obuf, "()" ); /* _main -> _main() */
		return obuf+1; /* _main -> main */
	}

	return obuf;
}

static bool PointsToValidCall( unsigned long ptr )
{
	char buf[7];
	int len = 7;

	memset( buf, 0, sizeof(buf) );

	while(len > 0 && !ReadProcessMemory(GetCurrentProcess(), (void *)(ptr-len), buf+7-len, len, NULL))
		--len;

	return IsValidCall(buf+7, len);
}

void do_backtrace( const void **buf, size_t size, 
						 HANDLE hProcess, HANDLE hThread, const CONTEXT *pContext )
{
	// Retrieve stack pointers.
	const char *pStackBase;
	{
		LDT_ENTRY sel;
		if( !GetThreadSelectorEntry( hThread, pContext->SegFs, &sel ) )
		{
			buf[0] = NULL;
			return;
		}

		const NT_TIB *tib = (NT_TIB *) ((sel.HighWord.Bits.BaseHi<<24)+(sel.HighWord.Bits.BaseMid<<16)+sel.BaseLow);
		const NT_TIB *pTib = tib->Self;
		pStackBase = (char *)pTib->StackBase;
	}

	// Walk up the stack.
	const char *lpAddr = (const char *)pContext->Esp;

	const void *data = (void *) pContext->Eip;
	size_t i = 0;
	do {
		if( i+1 >= size )
			break;

		bool fValid = true;

		/* The first entry is EIP, which is always interesting, even if it's not valid. */
		if( i != 0 )
		{
			MEMORY_BASIC_INFORMATION meminfo;

			VirtualQuery((void *)data, &meminfo, sizeof meminfo);
			
			if (!IsExecutableProtection(meminfo.Protect) || meminfo.State!=MEM_COMMIT)
				fValid = false;

			if ( data != (void *) pContext->Eip && !PointsToValidCall((unsigned long)data) )
				fValid = false;
		}

		if( fValid )
			buf[i++] = data;

		if (lpAddr >= pStackBase)
			break;

		lpAddr += 4;
	} while( ReadProcessMemory(hProcess, lpAddr-4, &data, 4, NULL));

	buf[i++] = NULL;
}

void SymLookup( const void *ptr, char *buf )
{
	VDDebugInfoInitFromFile( &g_debugInfo );
	if( !g_debugInfo.Loaded() )
	{
		strcpy( buf, "error" );
		return;
	}

	MEMORY_BASIC_INFORMATION meminfo;
	VirtualQuery( ptr, &meminfo, sizeof meminfo );

	char tmp[512];
	if( VDDebugInfoLookupRVA(&g_debugInfo, (unsigned int)ptr, tmp, sizeof(tmp)) >= 0 )
	{
		wsprintf( buf, "%08x: %s", ptr, Demangle(tmp) );
		return;
	}

	char szName[MAX_PATH];
	if( !CrashGetModuleBaseName((HMODULE)meminfo.AllocationBase, szName) )
		strcpy( szName, "???" );

	DWORD64 disp;
	SYMBOL_INFO *pSymbol = GetSym( (unsigned int)ptr, disp );

	if( pSymbol )
	{
		wsprintf( buf, "%08lx: %s!%s [%08lx+%lx+%lx]",
			(unsigned long) ptr, szName, pSymbol->Name,
			(unsigned long) meminfo.AllocationBase,
			(unsigned long) (pSymbol->Address) - (unsigned long) (meminfo.AllocationBase),
			(unsigned long) disp);
		return;
	}

	wsprintf( buf, "%08lx: %s!%08lx",
		(unsigned long) ptr, szName, 
		(unsigned long) meminfo.AllocationBase );
}

static bool ReportCallStack( HWND hwnd, HANDLE hFile, const void **Backtrace )
{
	VDDebugInfoInitFromFile( &g_debugInfo );
	if( !g_debugInfo.Loaded() )
	{
		Report( hwnd, hFile, "Could not open debug resource file (%s).", g_debugInfo.sFilename );
		return false;
	}

	if( g_debugInfo.nBuildNumber != int(version_num) )
	{
		Report(hwnd, hFile, "Incorrect %s file (build %d, expected %d) for this version of " PRODUCT_NAME " -- call stack unavailable.",
			g_debugInfo.sFilename, g_debugInfo.nBuildNumber, int(version_num));
		return false;
	}

	for( int i = 0; Backtrace[i]; ++i )
	{
		char buf[10240];
		SymLookup( Backtrace[i], buf );
		Report( hwnd, hFile, "%s", buf );
	}

	return true;
}

void WriteBuf( HANDLE hFile, const char *buf )
{
	DWORD dwActual;
	WriteFile(hFile, buf, strlen(buf), &dwActual, NULL);
}

static void DoSave()
{
	char szModName2[MAX_PATH];

	SpliceProgramPath(szModName2, sizeof szModName2, "../crashinfo.txt");

	HANDLE hFile = CreateFile(szModName2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return;

	Report(NULL, hFile,
			"%s crash report (build %d)\r\n"
			"--------------------------------------"
			"\r\n", PRODUCT_NAME_VER, version_num);

	ReportReason( NULL, hFile, &g_CrashInfo );
	Report(NULL, hFile, "");

	// Dump thread stacks
	WriteBuf( hFile, Checkpoints::GetLogs("\r\n") );
	Report(NULL, hFile, "");

	ReportCallStack( NULL, hFile, g_CrashInfo.m_BacktracePointers );
	Report(NULL, hFile, "");

	if( g_CrashInfo.m_AlternateThreadBacktrace[0] )
	{
		for( int i = 0; i < CrashInfo::MAX_BACKTRACE_THREADS; ++i )
		{
			if( !g_CrashInfo.m_AlternateThreadBacktrace[i][0] )
				continue;

			Report( NULL, hFile, "Thread %s:", g_CrashInfo.m_AlternateThreadName[i] );
			Report( NULL, hFile, "" );
			ReportCallStack( NULL, hFile, g_CrashInfo.m_AlternateThreadBacktrace[i] );
			Report(NULL, hFile, "");
		}
	}

	Report(NULL, hFile, "Static log:");
	WriteBuf( hFile, RageLog::GetInfo() );
	WriteBuf( hFile, RageLog::GetAdditionalLog() );
	Report(NULL, hFile, "");

	Report(NULL, hFile, "Partial log:");
	int i = 0;
	while( const char *p = RageLog::GetRecentLog( i++ ) )
		Report(NULL, hFile, "%s", p);
	Report(NULL, hFile, "");

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

BOOL APIENTRY CrashDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool s_bHaveCallstack;

	switch(msg)
	{
	case WM_INITDIALOG:
		{
			HWND hwndList = GetDlgItem(hDlg, IDC_CALL_STACK);
			HWND hwndReason = GetDlgItem(hDlg, IDC_STATIC_BOMBREASON);

			if (hFontMono)
				SendMessage(hwndList, WM_SETFONT, (WPARAM)hFontMono, MAKELPARAM(TRUE, 0));

			ReportReason( hwndReason, NULL, &g_CrashInfo );
			s_bHaveCallstack = ReportCallStack( hwndList, NULL, g_CrashInfo.m_BacktracePointers );
		}
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BUTTON_CLOSE:
			EndDialog(hDlg, FALSE);
			return TRUE;
		case IDOK:
			// EndDialog(hDlg, TRUE); /* don't always exit on ENTER */
			return TRUE;
		case IDC_VIEW_LOG:
			ViewWithNotepad("../log.txt");
			break;
		case IDC_CRASH_SAVE:
			if (!s_bHaveCallstack)
				if (IDOK != MessageBox(hDlg,
					PRODUCT_NAME " cannot load its crash resource file, and thus the crash dump will be "
					"missing the most important part, the call stack. Crash dumps are much less useful "
					"without the call stack.",
					PRODUCT_NAME " warning", MB_OK|MB_ICONEXCLAMATION))
					return TRUE;

			ViewWithNotepad("../crashinfo.txt");
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
	}

	return FALSE;
}

void NORETURN debug_crash()
{
	__try {
		__asm xor ebx,ebx
		__asm mov eax,dword ptr [ebx]
//		__asm mov dword ptr [ebx],eax
//		__asm lock add dword ptr cs:[00000000h], 12345678h
	} __except(CrashHandler((EXCEPTION_POINTERS*)_exception_info())) {
	}
}

/* Get a stack trace of the current thread and the specified thread.  If
 * iID == GetInvalidThreadId(), then output a stack trace for every thread. */
void ForceCrashHandlerDeadlock( CString reason, uint64_t iID )
{
	strncpy( g_CrashInfo.m_CrashReason, reason, sizeof(g_CrashInfo.m_CrashReason) );
	g_CrashInfo.m_CrashReason[ sizeof(g_CrashInfo.m_CrashReason)-1 ] = 0;

	/* Suspend the other thread we're going to backtrace.  (We need to at least suspend
	 * hThread, for GetThreadContext to work.) */
	RageThread::HaltAllThreads( false );

	if( iID == GetInvalidThreadId() )
	{
		/* Backtrace all threads. */
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
					"; GetThreadContext(%x) failed", (int) hThread );
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

void ForceCrashHandler( const char *reason )
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

