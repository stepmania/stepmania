#include "global.h"
#include "CrashHandlerInternal.h"
#include "Crash.h"
#include <errno.h>

#include <windows.h>
#include <commctrl.h>
#include "archutils/Win32/ddk/dbghelp.h"
#include <io.h>
#include <fcntl.h>

#include "arch/ArchHooks/ArchHooks.h"
#include "archutils/Win32/WindowsResources.h"
#include "archutils/Win32/DialogUtil.h"
#include "archutils/Win32/ErrorStrings.h"
#include "archutils/Win32/GotoURL.h"
#include "archutils/Win32/RestartProgram.h"
#include "archutils/Win32/CrashHandlerNetworking.h"
#include "archutils/Win32/WindowsDialogBox.h"
#include "ProductInfo.h"
#include "RageUtil.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "LocalizedString.h"
#include "RageFileDriverDeflate.h"

#if defined(_MSC_VER)
#pragma comment(lib, "archutils/Win32/ddk/dbghelp.lib")
#endif

extern unsigned long version_num;
extern const char *const version_time;

// VDI symbol lookup:
namespace VDDebugInfo
{
	struct Context
	{
		Context() { pRVAHeap=NULL; }
		bool Loaded() const { return pRVAHeap != NULL; }
		RString sRawBlock;

		int nBuildNumber;

		const unsigned char *pRVAHeap;
		unsigned nFirstRVA;

		const char *pFuncNameHeap;
		const unsigned long (*pSegments)[2];
		int nSegments;
		char sFilename[1024];
		RString sError;
	};

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

	bool VDDebugInfoInitFromMemory( Context *pctx )
	{
		if( pctx->sRawBlock[0] == '\x1f' &&
			pctx->sRawBlock[1] == '\x8b' )
		{
			RString sBufOut;
			RString sError;
			if( !GunzipString(pctx->sRawBlock, sBufOut, sError) )
			{
				pctx->sError = werr_ssprintf( GetLastError(), "VDI error: %s", sError.c_str() );
				return false;
			}

			pctx->sRawBlock = sBufOut;
		}


		const unsigned char *src = (const unsigned char *) pctx->sRawBlock.data();

		pctx->pRVAHeap = NULL;

		static const char *header = "symbolic debug information";
		if( memcmp(src, header, strlen(header)) )
		{
			pctx->sError = "header doesn't match";
			return false;
		}

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

	void VDDebugInfoDeinit( Context *pctx )
	{
		if( !pctx->sRawBlock.empty() )
			pctx->sRawBlock = RString();
	}

	bool VDDebugInfoInitFromFile( Context *pctx )
	{
		if( pctx->Loaded() )
			return true;

		pctx->sRawBlock = RString();
		pctx->pRVAHeap = NULL;
		GetVDIPath( pctx->sFilename, ARRAYLEN(pctx->sFilename) );
		pctx->sError = RString();

		HANDLE h = CreateFile( pctx->sFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( h == INVALID_HANDLE_VALUE )
		{
			pctx->sError = werr_ssprintf( GetLastError(), "CreateFile failed" );
			return false;
		}

		do {
			DWORD dwFileSize = GetFileSize( h, NULL );
			if( dwFileSize == INVALID_FILE_SIZE )
				break;

			char *pBuf = pctx->sRawBlock.GetBuffer( dwFileSize );
			if( pBuf == NULL )
				break;

			DWORD dwActual;
			int iRet = ReadFile(h, pBuf, dwFileSize, &dwActual, NULL);
			CloseHandle(h);
			pctx->sRawBlock.ReleaseBuffer( dwActual );

			if( !iRet || dwActual != dwFileSize )
				break;

			if( VDDebugInfoInitFromMemory(pctx) )
				return true;
		} while(0);

		VDDebugInfoDeinit(pctx);
		return false;
	}

	static bool PointerIsInAnySegment( const Context *pctx, unsigned rva )
	{
		for( int i=0; i<pctx->nSegments; ++i )
		{
			if (rva >= pctx->pSegments[i][0] && rva < pctx->pSegments[i][0] + pctx->pSegments[i][1])
				return true;
		}

		return false;
	}

	static const char *GetNameFromHeap(const char *heap, int idx)
	{
		while(idx--)
			while(*heap++);

		return heap;
	}

	long VDDebugInfoLookupRVA( const Context *pctx, unsigned rva, char *buf, int buflen )
	{
		if( !PointerIsInAnySegment(pctx, rva) )
			return -1;

		const unsigned char *pr = pctx->pRVAHeap;
		const unsigned char *pr_limit = (const unsigned char *)pctx->pFuncNameHeap;
		int idx = 0;

		// Linearly unpack RVA deltas and find lower_bound
		rva -= pctx->nFirstRVA;

		if( (signed)rva < 0 )
			return -1;

		while( pr < pr_limit )
		{
			unsigned char c;
			unsigned diff = 0;

			do
			{
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
		if( pr >= pr_limit )
			return -1;

		// Decompress name for RVA
		const char *fn_name = GetNameFromHeap(pctx->pFuncNameHeap, idx);

		if( !*fn_name )
			fn_name = "(special)";

		strncpy( buf, fn_name, buflen );
		buf[buflen-1] = 0;

		return rva;
	}
}

bool ReadFromParent( int fd, void *p, int size )
{
	char *buf = (char *) p;
	int got = 0;
	while( got < size )
	{
		int ret = read( fd, buf+got, size-got );
		if( ret == -1 )
		{
			if( errno == EINTR )
				continue;
			fprintf( stderr, "Crash handler: error communicating with parent: %s\n", strerror(errno) );
			return false;
		}

		if( ret == 0 )
		{
			fprintf( stderr, "Crash handler: EOF communicating with parent.\n" );
			return false;
		}

		got += ret;
	}

	return true;
}


// General symbol lookup; uses VDDebugInfo for detailed information within the
// process, and DbgHelp for simpler information about loaded DLLs.
namespace SymbolLookup
{
	HANDLE g_hParent;

	bool InitDbghelp()
	{
		static bool bInitted = false;
		if( !bInitted )
		{
			SymSetOptions( SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS );

			if( !SymInitialize(g_hParent, NULL, TRUE) )
				return false;

			bInitted = true;
		}

		return true;
	}

	SYMBOL_INFO *GetSym( unsigned long ptr, DWORD64 &disp )
	{
		InitDbghelp();

		static BYTE buffer[1024];
		SYMBOL_INFO *pSymbol = (PSYMBOL_INFO)buffer;

		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = sizeof(buffer) - sizeof(SYMBOL_INFO) + 1;

		if( !SymFromAddr(g_hParent, ptr, &disp, pSymbol) )
			return NULL;

		return pSymbol;
	}

	const char *Demangle( const char *buf )
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
			| UNDNAME_NO_MEMBER_TYPE // no virtual, static
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

	RString CrashChildGetModuleBaseName( HMODULE hMod )
	{
		write( _fileno(stdout), &hMod,  sizeof(hMod) );

		int iFD = fileno(stdin);
		int iSize;
		if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
			return "???";
		RString sName;
		char *pBuf = sName.GetBuffer( iSize );
		if( !ReadFromParent(iFD, pBuf, iSize) )
			return "???";
		sName.ReleaseBuffer( iSize );
		return sName;
	}

	void SymLookup( VDDebugInfo::Context *pctx, const void *ptr, char *buf )
	{
		if( !pctx->Loaded() )
		{
			strcpy( buf, "error" );
			return;
		}

		MEMORY_BASIC_INFORMATION meminfo;
		VirtualQueryEx( g_hParent, ptr, &meminfo, sizeof meminfo );

		char tmp[512];
		long iAddress = VDDebugInfo::VDDebugInfoLookupRVA(pctx, (unsigned int)ptr, tmp, sizeof(tmp));
		if( iAddress >= 0 )
		{
			wsprintf( buf, "%08x: %s [%08lx+%lx+%lx]", ptr, Demangle(tmp),
				pctx->nFirstRVA,
				((unsigned int) ptr) - pctx->nFirstRVA - iAddress,
				iAddress );
			return;
		}

		
		RString sName = CrashChildGetModuleBaseName( (HMODULE)meminfo.AllocationBase );

		DWORD64 disp;
		SYMBOL_INFO *pSymbol = GetSym( (unsigned int)ptr, disp );

		if( pSymbol )
		{
			wsprintf( buf, "%08lx: %s!%s [%08lx+%lx+%lx]",
				(unsigned long) ptr, sName.c_str(), pSymbol->Name,
				(unsigned long) meminfo.AllocationBase,
				(unsigned long) (pSymbol->Address) - (unsigned long) (meminfo.AllocationBase),
				(unsigned long) disp);
			return;
		}

		wsprintf( buf, "%08lx: %s!%08lx",
			(unsigned long) ptr, sName.c_str(), 
			(unsigned long) meminfo.AllocationBase );
	}
}

namespace
{

RString SpliceProgramPath( RString fn )
{
	char szBuf[MAX_PATH];
	GetModuleFileName( NULL, szBuf, sizeof(szBuf) );

	char szModName[MAX_PATH];
	char *pszFile;
	GetFullPathName( szBuf, sizeof(szModName), szModName, &pszFile );
	strcpy( pszFile, fn );

	return szModName;
}

namespace
{
	VDDebugInfo::Context g_debugInfo;

	RString ReportCallStack( const void * const *Backtrace )
	{
		if( !g_debugInfo.Loaded() )
			return ssprintf( "debug resource file '%s': %s.\n", g_debugInfo.sFilename, g_debugInfo.sError.c_str() );

		if( g_debugInfo.nBuildNumber != int(version_num) )
		{
			return ssprintf( "Incorrect %s file (build %d, expected %d) for this version of " PRODUCT_FAMILY " -- call stack unavailable.\n",
				g_debugInfo.sFilename, g_debugInfo.nBuildNumber, int(version_num) );
		}

		RString sRet;
		for( int i = 0; Backtrace[i]; ++i )
		{
			char buf[10240];
			SymbolLookup::SymLookup( &g_debugInfo, Backtrace[i], buf );
			sRet += ssprintf( "%s\n", buf );
		}

		return sRet;
	}
}

struct CompleteCrashData
{
	CrashInfo m_CrashInfo;
	RString m_sInfo;
	RString m_sAdditionalLog;
	RString m_sCrashedThread;
	vector<RString> m_asRecent;
	vector<RString> m_asCheckpoints;
};

static void MakeCrashReport( const CompleteCrashData &Data, RString &sOut )
{
	sOut += ssprintf(
			"%s crash report (build %d, %s)\n"
			"--------------------------------------\n\n",
			PRODUCT_ID_VER, version_num, version_time );

	sOut += ssprintf( "Crash reason: %s\n", Data.m_CrashInfo.m_CrashReason );
	sOut += ssprintf( "\n" );

	// Dump thread stacks
	static char buf[1024*32];
	sOut += ssprintf( "%s\n", join("\n", Data.m_asCheckpoints).c_str() );
	
	sOut += ReportCallStack( Data.m_CrashInfo.m_BacktracePointers );
	sOut += ssprintf( "\n" );

	if( Data.m_CrashInfo.m_AlternateThreadBacktrace[0] )
	{
		for( int i = 0; i < CrashInfo::MAX_BACKTRACE_THREADS; ++i )
		{
			if( !Data.m_CrashInfo.m_AlternateThreadBacktrace[i][0] )
				continue;

			sOut += ssprintf( "Thread %s:\n", Data.m_CrashInfo.m_AlternateThreadName[i] );
			sOut += ssprintf( "\n" );
			sOut += ReportCallStack( Data.m_CrashInfo.m_AlternateThreadBacktrace[i] );
			sOut += ssprintf( "" );
		}
	}

	sOut += ssprintf( "Static log:\n" );
	sOut += ssprintf( "%s", Data.m_sInfo.c_str() );
	sOut += ssprintf( "%s", Data.m_sAdditionalLog.c_str() );
	sOut += ssprintf( "\n" );

	sOut += ssprintf( "Partial log:\n" );
	for( size_t  i = 0; i < Data.m_asRecent.size(); ++i )
		sOut += ssprintf( "%s\n", Data.m_asRecent[i].c_str() );
	sOut += ssprintf( "\n" );

	sOut += ssprintf( "-- End of report\n" );
}

static void DoSave( const RString &sReport )
{
	RString sName = SpliceProgramPath( "../crashinfo.txt" );

	SetFileAttributes( sName, FILE_ATTRIBUTE_NORMAL );
	FILE *pFile = fopen( sName, "w+" );
	if( pFile == NULL )
		return;
	fprintf( pFile, "%s", sReport.c_str() );

	fclose( pFile );

	/* Discourage changing crashinfo.txt. */
	SetFileAttributes( sName, FILE_ATTRIBUTE_READONLY );
}

void ViewWithNotepad(const char *str)
{
	char buf[256] = "";
	strcat( buf, "notepad.exe " );
	strcat( buf, str );

	RString cwd = SpliceProgramPath( "" );

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
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


bool ReadCrashDataFromParent( int iFD, CompleteCrashData &Data )
{
	_setmode( _fileno(stdin), O_BINARY );

	/* 0. Read the parent handle. */
	if( !ReadFromParent(iFD, &SymbolLookup::g_hParent, sizeof(SymbolLookup::g_hParent)) )
		return false;

	/* 1. Read the CrashData. */
	if( !ReadFromParent(iFD, &Data.m_CrashInfo, sizeof(Data.m_CrashInfo)) )
		return false;

	/* 2. Read info. */
	int iSize;
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;

	char *pBuf = Data.m_sInfo.GetBuffer( iSize );
	if( !ReadFromParent(iFD, pBuf, iSize) )
		return false;
	Data.m_sInfo.ReleaseBuffer( iSize );

	/* 3. Read AdditionalLog. */
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;

	pBuf = Data.m_sAdditionalLog.GetBuffer( iSize );
	if( !ReadFromParent(iFD, pBuf, iSize) )
		return false;
	Data.m_sAdditionalLog.ReleaseBuffer( iSize );

	/* 4. Read RecentLogs. */
	int iCnt = 0;
	if( !ReadFromParent(iFD, &iCnt, sizeof(iCnt)) )
		return false;
	for( int i = 0; i < iCnt; ++i )
	{
		if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
			return false;
		RString sBuf;
		pBuf = sBuf.GetBuffer( iSize );
		if( !ReadFromParent(iFD, pBuf, iSize) )
			return false;
		Data.m_asRecent.push_back( sBuf );
		sBuf.ReleaseBuffer( iSize );
	}

	/* 5. Read CHECKPOINTs. */
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;

	RString sBuf;
	pBuf = sBuf.GetBuffer( iSize );
	if( !ReadFromParent(iFD, pBuf, iSize) )
		return false;

	split( sBuf, "$$", Data.m_asCheckpoints );
	sBuf.ReleaseBuffer( iSize );

	/* 6. Read the crashed thread's name. */
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;
	pBuf = Data.m_sCrashedThread.GetBuffer( iSize );
	if( !ReadFromParent(iFD, pBuf, iSize) )
		return false;
	Data.m_sCrashedThread.ReleaseBuffer();

	return true;
}

/*
 * Localization for the crash handler is different, and a little tricky.  We don't
 * have ThemeManager loaded, so we have to localize it ourself.  We can supply
 * translations with our own substitution function.  We need to figure out which
 * language to use.  Since these strings won't be pulled from the theme, defer
 * loading them until we use them.  XXX
 */
static LocalizedString A_CRASH_HAS_OCCURRED;
static LocalizedString REPORTING_THE_PROBLEM;
static LocalizedString CLOSE;
static LocalizedString CANCEL;
static LocalizedString VIEW_UPDATE;
static LocalizedString UPDATE_IS_AVAILABLE;
static LocalizedString UPDATE_IS_NOT_AVAILABLE;
static LocalizedString ERROR_SENDING_REPORT;

// #define AUTOMATED_CRASH_REPORTS
#define CRASH_REPORT_HOST "example.com"
#define CRASH_REPORT_PORT 80
#define CRASH_REPORT_PATH "/report.cgi"

void LoadLocalizedStrings()
{
#if defined(AUTOMATED_CRASH_REPORTS)
	A_CRASH_HAS_OCCURRED.Load( "CrashHandler",
		"A crash has occurred.  Would you like to automatically report the "
		"problem and check for updates?" );
#else
	A_CRASH_HAS_OCCURRED.Load( "CrashHandler",
		"A crash has occurred.  Diagnostic information has been saved to a file "
		"called \"crashinfo.txt\" in the game program directory." );
#endif
	REPORTING_THE_PROBLEM.Load( "CrashHandler",
		"Reporting the problem and checking for updates ..." );
	CLOSE.Load( "CrashHandler", "&Close" );
	CANCEL.Load( "CrashHandler", "&Cancel" );
	VIEW_UPDATE.Load( "CrashHandler", "View &update" );
	UPDATE_IS_AVAILABLE.Load( "CrashHandler", "An update is available." );
	UPDATE_IS_NOT_AVAILABLE.Load( "CrashHandler", "The error has been reported.  No updates are available." );
	ERROR_SENDING_REPORT.Load( "CrashHandler", "An error was encountered sending the report." );
}

class CrashDialog: public WindowsDialogBox
{
public:
	CrashDialog( const RString &sCrashReport, const CompleteCrashData &CrashData );
	~CrashDialog();

protected:
	virtual BOOL HandleMessage( UINT msg, WPARAM wParam, LPARAM lParam );

private:
	void SetDialogInitial();

	NetworkPostData *m_pPost;
	RString m_sUpdateURL;
	const RString m_sCrashReport;
	CompleteCrashData m_CrashData;
};

CrashDialog::CrashDialog( const RString &sCrashReport, const CompleteCrashData &CrashData ):
	m_sCrashReport( sCrashReport ),
	m_CrashData( CrashData )
{
	LoadLocalizedStrings();
	m_pPost = NULL;
}

CrashDialog::~CrashDialog()
{
	delete m_pPost;
}

void CrashDialog::SetDialogInitial()
{
	HWND hDlg = GetHwnd();

	SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), A_CRASH_HAS_OCCURRED.GetValue() );
	SetWindowText( GetDlgItem(hDlg, IDC_BUTTON_CLOSE), CLOSE.GetValue() );
	ShowWindow( GetDlgItem(hDlg, IDC_PROGRESS), false );
	ShowWindow( GetDlgItem(hDlg, IDC_BUTTON_AUTO_REPORT), true );
}

BOOL CrashDialog::HandleMessage( UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND hDlg = GetHwnd();

	switch(msg)
	{
	case WM_INITDIALOG:
		SetDialogInitial();
		DialogUtil::SetHeaderFont( hDlg, IDC_STATIC_HEADER_TEXT );
		return TRUE;

	case WM_CTLCOLORSTATIC:
		{
			HDC hdc = (HDC)wParam;
			HWND hwndStatic = (HWND)lParam;
			HBRUSH hbr = NULL;

			// TODO:  Change any attributes of the DC here
			switch( GetDlgCtrlID(hwndStatic) )
			{
			case IDC_STATIC_HEADER_TEXT:
			case IDC_STATIC_ICON:
				hbr = (HBRUSH)::GetStockObject(WHITE_BRUSH); 
				SetBkMode( hdc, OPAQUE );
				SetBkColor( hdc, RGB(255,255,255) );
				break;
			}

			// TODO:  Return a different brush if the default is not desired
			return (BOOL)hbr;
		}

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_CLOSE:
			if( m_pPost != NULL )
			{
				/* Cancel reporting, and revert the dialog as if "report" had not been pressed. */
				m_pPost->Cancel();
				KillTimer( hDlg, 0 );

				SetDialogInitial();
				SAFE_DELETE( m_pPost );
				return TRUE;
			}

			/* Close the dialog. */
			EndDialog(hDlg, FALSE);
			return TRUE;
		case IDOK:
			// EndDialog(hDlg, TRUE); /* don't always exit on ENTER */
			return TRUE;
		case IDC_VIEW_LOG:
			ViewWithNotepad("../log.txt");
			break;
		case IDC_CRASH_SAVE:
			ViewWithNotepad("../crashinfo.txt");
			return TRUE;
		case IDC_BUTTON_RESTART:
			Win32RestartProgram();
			EndDialog( hDlg, FALSE );
			break;
		case IDC_BUTTON_REPORT:
			GotoURL( REPORT_BUG_URL );
			break;
		case IDC_BUTTON_AUTO_REPORT:
			if( !m_sUpdateURL.empty() )
			{
				/* We already sent the report, were told that there's an update, and
				 * substituted the URL. */
				GotoURL( m_sUpdateURL );
				break;
			}

			ShowWindow( GetDlgItem(hDlg, IDC_BUTTON_AUTO_REPORT), false );
			ShowWindow( GetDlgItem(hDlg, IDC_PROGRESS), true );
			SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), REPORTING_THE_PROBLEM.GetValue() );
			SetWindowText( GetDlgItem(hDlg, IDC_BUTTON_CLOSE), CANCEL.GetValue() );
			SendDlgItemMessage( hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0,100) );
			SendDlgItemMessage( hDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0 );

			/* Create the form data to send. */
			m_pPost = new NetworkPostData;
			m_pPost->SetData( "Product", PRODUCT_ID );
			m_pPost->SetData( "Version", PRODUCT_VER );
			m_pPost->SetData( "Arch", HOOKS->GetArchName().c_str() );
			m_pPost->SetData( "Report", m_sCrashReport );
			m_pPost->SetData( "Reason", m_CrashData.m_CrashInfo.m_CrashReason );

			m_pPost->Start( CRASH_REPORT_HOST, CRASH_REPORT_PORT, CRASH_REPORT_PATH );

			SetTimer( hDlg, 0, 100, NULL );
			break;
		}
		break;
	case WM_TIMER:
		{
			if( m_pPost == NULL )
				break;

			float fProgress = m_pPost->GetProgress();
			SendDlgItemMessage( hDlg, IDC_PROGRESS, PBM_SETPOS, int(fProgress*100), 0 );

			if( m_pPost->IsFinished() )
			{
				KillTimer( hDlg, 0 );

				/* Grab the result, which is the data output from the HTTP request.  It's
				* simple XML. */
				RString sResult = m_pPost->GetResult();
				RString sError = m_pPost->GetError();
				if( sError.empty() && sResult.empty() )
					sError = "No data received";

				SAFE_DELETE( m_pPost );

				XNode xml;
				if( sError.empty() )
				{
					RString sError;
					XmlFileUtil::Load( &xml, sResult, sError );
					if( !sError.empty() )
					{
						sError = ssprintf( "Error parsing response: %s", sError.c_str() );
						xml.Clear();
					}
				}

				int iID;
				if( !sError.empty() )
				{
					/* On error, don't show the "report" button again.  If the submission was actually
					* successful, then it'd be too easy to accidentally spam the server by holding
					* down the button. */
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), ERROR_SENDING_REPORT.GetValue() );
				}
				else if( xml.GetAttrValue("UpdateAvailable", m_sUpdateURL) )
				{
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), UPDATE_IS_AVAILABLE.GetValue() );
					SetWindowText( GetDlgItem(hDlg, IDC_BUTTON_AUTO_REPORT), VIEW_UPDATE.GetValue() );
					ShowWindow( GetDlgItem(hDlg, IDC_BUTTON_AUTO_REPORT), true );
				}
				else if( xml.GetAttrValue("ReportId", iID) )
				{
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), UPDATE_IS_NOT_AVAILABLE.GetValue() );
				}
				else
				{
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), ERROR_SENDING_REPORT.GetValue() );
				}

				if( xml.GetAttrValue("ReportId", iID) )
				{
					char sBuf[1024];
					GetWindowText( hDlg, sBuf, 1024 );
					SetWindowText( hDlg, ssprintf("%s (#%i)", sBuf, iID) );
				}

				ShowWindow( GetDlgItem(hDlg, IDC_PROGRESS), false );
				SetWindowText( GetDlgItem(hDlg, IDC_BUTTON_CLOSE), CLOSE.GetValue() );
			}
		}
	}

	return FALSE;
}

void ChildProcess()
{
	/* Read the crash data from the crashed parent. */
	CompleteCrashData Data;
	ReadCrashDataFromParent( fileno(stdin), Data );

	RString sCrashReport;
	VDDebugInfo::VDDebugInfoInitFromFile( &g_debugInfo );
	MakeCrashReport( Data, sCrashReport );
	VDDebugInfo::VDDebugInfoDeinit( &g_debugInfo );

	DoSave( sCrashReport );

	/* Tell the crashing process that it can exit.  Be sure to write crashinfo.txt first. */
	fclose( stdout );

	/* Now that we've done that, the process is gone.  Don't use g_hParent. */
	CloseHandle( SymbolLookup::g_hParent );
	SymbolLookup::g_hParent = NULL;

	CrashDialog cd( sCrashReport, Data );
#if defined(AUTOMATED_CRASH_REPORTS)
	cd.Run( IDD_REPORT_CRASH );
#else
	cd.Run( IDD_DISASM_CRASH );
#endif
}

}

void CrashHandler::CrashHandlerHandleArgs( int argc, char* argv[] )
{
	if( argc == 2 && !strcmp(argv[1], CHILD_MAGIC_PARAMETER) )
	{
		ChildProcess();
		exit(0);
	}
}

/*
 * (c) 2003-2006 Glenn Maynard
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
