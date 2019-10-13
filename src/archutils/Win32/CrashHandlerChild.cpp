#include "global.h"
#include "CrashHandlerInternal.h"
#include "Crash.h"
#include <errno.h>

#include <windows.h>
#include <commctrl.h>
#include "archutils/Win32/ddk/dbghelp.h"
#include <io.h>
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#include <shellapi.h>

#include "arch/ArchHooks/ArchHooks.h"
#include "archutils/Win32/WindowsResources.h"
#include "archutils/Win32/DialogUtil.h"
#include "archutils/Win32/ErrorStrings.h"
#include "archutils/Win32/GotoURL.h"
#include "archutils/Win32/RestartProgram.h"
#include "archutils/Win32/CrashHandlerNetworking.h"
#include "archutils/Win32/WindowsDialogBox.h"
#include "archutils/Win32/SpecialDirs.h"
#include "ProductInfo.h"
#include "RageUtil.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "LocalizedString.h"
#include "RageFileDriverDeflate.h"
#include "ver.h"

#if defined(_MSC_VER)
#pragma comment(lib, "dbghelp.lib")
#endif

// XXX: What happens when we *don't* have version info? Does that ever actually happen?
#include "ver.h"

#if _WIN64
#define ADDRESS_ZEROS "016"
#else
#define ADDRESS_ZEROS "08"
#endif

// VDI symbol lookup:
namespace VDDebugInfo
{
	struct Context
	{
		Context() { pRVAHeap=nullptr; }
		bool Loaded() const { return pRVAHeap != nullptr; }
		RString sRawBlock;

		int nBuildNumber;

		const unsigned char *pRVAHeap;
		uintptr_t nFirstRVA;

		const char *pFuncNameHeap;
		const uintptr_t (*pSegments)[2];
		int nSegments;
		char sFilename[1024];
		RString sError;
	};

	static void GetVDIPath( char *buf, int bufsiz )
	{
		GetModuleFileName( nullptr, buf, bufsiz );
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

		pctx->pRVAHeap = nullptr;

		static const char *header = "symbolic debug information";
		if( memcmp(src, header, strlen(header)) )
		{
			pctx->sError = "header doesn't match";
			return false;
		}

		// Extract fields

		src += 64;
		const int* pVer = reinterpret_cast<const int*>(src);
		const size_t* pRVASize = reinterpret_cast<const size_t*>(src + sizeof(int));
		const size_t* pFNamSize = reinterpret_cast<const size_t*>(src + sizeof(int) + sizeof(size_t));
		const int* pSegCnt = reinterpret_cast<const int*>(src + sizeof(int) + 2 * sizeof(size_t));
		src += 2 * (sizeof(int) + sizeof(size_t));

		pctx->nBuildNumber		= *pVer;
		pctx->pRVAHeap			= reinterpret_cast<const unsigned char*>(src + sizeof(uintptr_t));
		pctx->nFirstRVA			= *reinterpret_cast<const uintptr_t*>(src);
		pctx->pFuncNameHeap		= reinterpret_cast<const char*>(src + *pRVASize);
		pctx->pSegments			= reinterpret_cast<const uintptr_t(*)[2]>(src + *pRVASize + *pFNamSize);
		pctx->nSegments			= *pSegCnt;

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
		pctx->pRVAHeap = nullptr;
		GetVDIPath( pctx->sFilename, ARRAYLEN(pctx->sFilename) );
		pctx->sError = RString();

		HANDLE h = CreateFile( pctx->sFilename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
		if( h == INVALID_HANDLE_VALUE )
		{
			pctx->sError = werr_ssprintf( GetLastError(), "CreateFile failed" );
			return false;
		}

		do {
			DWORD dwFileSize = GetFileSize( h, nullptr );
			if( dwFileSize == INVALID_FILE_SIZE )
				break;

			char *buffer = new char[static_cast<size_t>(dwFileSize) + 1];
			std::fill(buffer, buffer + dwFileSize + 1, '\0' );

			DWORD dwActual;
			int iRet = ReadFile(h, buffer, dwFileSize, &dwActual, nullptr);
			CloseHandle(h);
			pctx->sRawBlock = buffer;
			delete[] buffer;

			if( !iRet || dwActual != dwFileSize )
				break;

			if( VDDebugInfoInitFromMemory(pctx) )
				return true;
		} while(0);

		VDDebugInfoDeinit(pctx);
		return false;
	}

	static bool PointerIsInAnySegment( const Context *pctx, uintptr_t rva )
	{
		for( int i=0; i<pctx->nSegments; ++i )
		{
			if (rva >= pctx->pSegments[i][0] && rva < pctx->pSegments[i][0] + pctx->pSegments[i][1])
				return true;
		}

		return false;
	}

	static const char *GetNameFromHeap(const char *heap, size_t idx)
	{
		while(idx--)
			while(*heap++);

		return heap;
	}

	intptr_t VDDebugInfoLookupRVA( const Context *pctx, uintptr_t rva, char *buf, int buflen )
	{
		if( !PointerIsInAnySegment(pctx, rva) )
			return -1;

		const unsigned char *pr = pctx->pRVAHeap;
		const unsigned char *pr_limit = (const unsigned char *)pctx->pFuncNameHeap;
		size_t idx = 0;

		// Linearly unpack RVA deltas and find lower_bound
		rva -= pctx->nFirstRVA;

		if( static_cast<intptr_t>(rva) < 0 )
			return -1;

		while( pr < pr_limit )
		{
			unsigned char c;
			uintptr_t diff = 0;

			do
			{
				c = *pr++;

				diff = (diff << 7) | (c & 0x7f);
			} while(c & 0x80);

			rva -= diff;

			if (static_cast<intptr_t>(rva) < 0) {
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

		return static_cast<intptr_t>(rva);
	}
}

bool ReadFromParent( int fd, void *p, int size )
{
	char *buf = (char *) p;
	int got = 0;
	while( got < size )
	{
		int ret = _read( fd, buf+got, size-got );
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

			if( !SymInitialize(g_hParent, nullptr, TRUE) )
				return false;

			bInitted = true;
		}

		return true;
	}

	SYMBOL_INFO *GetSym( uintptr_t ptr, DWORD64 &disp )
	{
		InitDbghelp();

		static BYTE buffer[1024];
		SYMBOL_INFO *pSymbol = (PSYMBOL_INFO)buffer;

		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = sizeof(buffer) - sizeof(SYMBOL_INFO) + 1;

		if( !SymFromAddr(g_hParent, ptr, &disp, pSymbol) )
			return nullptr;

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
			strcat( obuf, "()" ); // _main -> _main()
			return obuf+1; // _main -> main
		}

		return obuf;
	}

	RString CrashChildGetModuleBaseName( HMODULE hMod )
	{
		_write( _fileno(stdout), &hMod,  sizeof(hMod) );

		int iFD = _fileno(stdin);
		int iSize;
		if (!ReadFromParent(iFD, &iSize, sizeof(iSize)))
		{
			return "???";
		}
		RString sName;
		char *buffer = new char[static_cast<size_t>(iSize) + 1];
		std::fill(buffer, buffer + iSize + 1, '\0');
		if (!ReadFromParent(iFD, buffer, iSize))
		{
			sName = "???";
		}
		else
		{
			sName = buffer;
		}
		delete[] buffer;
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
		intptr_t iAddress = VDDebugInfo::VDDebugInfoLookupRVA(pctx, reinterpret_cast<uintptr_t>(ptr), tmp, sizeof(tmp));
		if( iAddress >= 0 )
		{
			wsprintf( buf, "%" ADDRESS_ZEROS "Ix: %s [%" ADDRESS_ZEROS "Ix+%Ix+%Ix]", reinterpret_cast<uintptr_t>(ptr), Demangle(tmp),
				pctx->nFirstRVA,
				reinterpret_cast<uintptr_t>(ptr) - pctx->nFirstRVA - iAddress,
				iAddress );
			return;
		}

		RString sName = CrashChildGetModuleBaseName( (HMODULE)meminfo.AllocationBase );

		DWORD64 disp;
		SYMBOL_INFO *pSymbol = GetSym( reinterpret_cast<uintptr_t>(ptr), disp );

		if( pSymbol )
		{
			wsprintf( buf, "%" ADDRESS_ZEROS "Ix: %s!%s [%" ADDRESS_ZEROS "Ix+%Ix+%Ix]",
				reinterpret_cast<uintptr_t>(ptr), sName.c_str(), pSymbol->Name,
				reinterpret_cast<uintptr_t>(meminfo.AllocationBase),
				static_cast<uintptr_t>(pSymbol->Address) - reinterpret_cast<uintptr_t>(meminfo.AllocationBase),
				static_cast<ULONG_PTR>(disp));
			return;
		}

		wsprintf( buf, "%" ADDRESS_ZEROS "Ix: %s!%" ADDRESS_ZEROS "Ix",
			reinterpret_cast<uintptr_t>(ptr), sName.c_str(), 
			reinterpret_cast<uintptr_t>(meminfo.AllocationBase) );
	}
}

namespace
{

RString SpliceProgramPath( RString fn )
{
	char szBuf[MAX_PATH];
	GetModuleFileName( nullptr, szBuf, sizeof(szBuf) );

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
		/*
		if( g_debugInfo.nBuildNumber != int(version_num) )
		{
			return ssprintf( "Incorrect %s file (build %d, expected %d) for this version of " PRODUCT_FAMILY " -- call stack unavailable.\n",
				g_debugInfo.sFilename, g_debugInfo.nBuildNumber, int(version_num) );
		}
		*/
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
			"%s crash report (build %s, %s @ %s)\n"
			"--------------------------------------\n\n",
			(string(PRODUCT_FAMILY) + product_version).c_str(), ::sm_version_git_hash, version_date, version_time );

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
	if( pFile == nullptr )
		return;
	fprintf( pFile, "%s", sReport.c_str() );

	fclose( pFile );

	// Discourage changing crashinfo.txt.
	SetFileAttributes( sName, FILE_ATTRIBUTE_READONLY );
}

bool ReadCrashDataFromParent( int iFD, CompleteCrashData &Data )
{
	_setmode( _fileno(stdin), O_BINARY );

	// 0. Read the parent handle.
	if( !ReadFromParent(iFD, &SymbolLookup::g_hParent, sizeof(SymbolLookup::g_hParent)) )
		return false;

	// 1. Read the CrashData.
	if( !ReadFromParent(iFD, &Data.m_CrashInfo, sizeof(Data.m_CrashInfo)) )
		return false;

	// 2. Read info.
	int iSize;
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;

	char *buffer = new char[static_cast<size_t>(iSize) + 1];
	std::fill(buffer, buffer + iSize + 1, '\0');
	bool wasReadSuccessful = ReadFromParent(iFD, buffer, iSize);
	RString tmp = buffer;
	delete[] buffer;
	if (!wasReadSuccessful)
	{
		return false;
	}
	Data.m_sInfo = tmp;

	// 3. Read AdditionalLog.
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;

	buffer = new char[iSize + 1];
	std::fill(buffer, buffer + iSize + 1, '\0');
	wasReadSuccessful = ReadFromParent(iFD, buffer, iSize);
	tmp = buffer;
	delete[] buffer;
	if (!wasReadSuccessful)
	{
		return false;
	}
	Data.m_sAdditionalLog = tmp;

	// 4. Read RecentLogs.
	int iCnt = 0;
	if( !ReadFromParent(iFD, &iCnt, sizeof(iCnt)) )
		return false;
	for( int i = 0; i < iCnt; ++i )
	{
		if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
			return false;
		buffer = new char[iSize + 1];
		std::fill(buffer, buffer + iSize + 1, '\0');
		wasReadSuccessful = ReadFromParent(iFD, buffer, iSize);
		tmp = buffer;
		delete[] buffer;
		if (!wasReadSuccessful)
		{
			return false;
		}
		Data.m_asRecent.push_back(tmp);
	}

	// 5. Read CHECKPOINTs.
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;

	buffer = new char[iSize + 1];
	std::fill(buffer, buffer + iSize + 1, '\0');
	wasReadSuccessful = ReadFromParent(iFD, buffer, iSize);
	tmp = buffer;
	delete[] buffer;
	if (!wasReadSuccessful)
	{
		return false;
	}
	split(tmp, "$$", Data.m_asCheckpoints);

	// 6. Read the crashed thread's name.
	if( !ReadFromParent(iFD, &iSize, sizeof(iSize)) )
		return false;
	buffer = new char[iSize + 1];
	std::fill(buffer, buffer + iSize + 1, '\0');
	wasReadSuccessful = ReadFromParent(iFD, buffer, iSize);
	tmp = buffer;
	delete[] buffer;
	if (!wasReadSuccessful)
	{
		return false;
	}
	Data.m_sCrashedThread = tmp;

	return true;
}

/* Localization for the crash handler is different, and a little tricky. We don't
 * have ThemeManager loaded, so we have to localize it ourself. We can supply
 * translations with our own substitution function. We need to figure out which
 * language to use. Since these strings won't be pulled from the theme, defer
 * loading them until we use them. XXX */
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
	virtual INT_PTR HandleMessage( UINT msg, WPARAM wParam, LPARAM lParam );

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
	m_pPost = nullptr;
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

INT_PTR CrashDialog::HandleMessage( UINT msg, WPARAM wParam, LPARAM lParam )
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
			HBRUSH hbr = nullptr;

			// TODO: Change any attributes of the DC here
			switch( GetDlgCtrlID(hwndStatic) )
			{
			case IDC_STATIC_HEADER_TEXT:
			case IDC_STATIC_ICON:
				hbr = (HBRUSH)::GetStockObject(WHITE_BRUSH); 
				SetBkMode( hdc, OPAQUE );
				SetBkColor( hdc, RGB(255,255,255) );
				break;
			}

			// TODO: Return a different brush if the default is not desired
			return reinterpret_cast<INT_PTR>(hbr);
		}

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_CLOSE:
			if( m_pPost != nullptr )
			{
				// Cancel reporting, and revert the dialog as if "report" had not been pressed.
				m_pPost->Cancel();
				KillTimer( hDlg, 0 );

				SetDialogInitial();
				SAFE_DELETE( m_pPost );
				return TRUE;
			}

			// Close the dialog.
			EndDialog(hDlg, FALSE);
			return TRUE;
		case IDOK:
			// EndDialog(hDlg, TRUE); // don't always exit on ENTER
			return TRUE;
		case IDC_VIEW_LOG:
			{
				RString sLogPath;
				FILE *pFile = fopen( SpliceProgramPath("../Portable.ini"), "r" );
				if(pFile != nullptr)
				{
					sLogPath = SpliceProgramPath("../Logs/log.txt");
					fclose( pFile );
				}
				else
					sLogPath = SpecialDirs::GetAppDataDir() + PRODUCT_ID +"/Logs/log.txt";

				ShellExecute( nullptr, "open", sLogPath, "", "", SW_SHOWNORMAL );
			}
			break;
		case IDC_CRASH_SAVE:
			ShellExecute( nullptr, "open", SpliceProgramPath("../crashinfo.txt"), "", "", SW_SHOWNORMAL );
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
				/* We already sent the report, were told that there's an update,
				 * and substituted the URL. */
				GotoURL( m_sUpdateURL );
				break;
			}

			ShowWindow( GetDlgItem(hDlg, IDC_BUTTON_AUTO_REPORT), false );
			ShowWindow( GetDlgItem(hDlg, IDC_PROGRESS), true );
			SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), REPORTING_THE_PROBLEM.GetValue() );
			SetWindowText( GetDlgItem(hDlg, IDC_BUTTON_CLOSE), CANCEL.GetValue() );
			SendDlgItemMessage( hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0,100) );
			SendDlgItemMessage( hDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0 );

			// Create the form data to send.
			m_pPost = new NetworkPostData;
			m_pPost->SetData( "Product", PRODUCT_ID );
			m_pPost->SetData( "Version", product_version );
			m_pPost->SetData( "Arch", HOOKS->GetArchName().c_str() );
			m_pPost->SetData( "Report", m_sCrashReport );
			m_pPost->SetData( "Reason", m_CrashData.m_CrashInfo.m_CrashReason );

			m_pPost->Start( CRASH_REPORT_HOST, CRASH_REPORT_PORT, CRASH_REPORT_PATH );

			SetTimer( hDlg, 0, 100, nullptr );
			break;
		}
		break;
	case WM_TIMER:
		{
			if( m_pPost == nullptr )
				break;

			float fProgress = m_pPost->GetProgress();
			SendDlgItemMessage( hDlg, IDC_PROGRESS, PBM_SETPOS, int(fProgress*100), 0 );

			if( m_pPost->IsFinished() )
			{
				KillTimer( hDlg, 0 );

				/* Grab the result, which is the data output from the HTTP request.
				 * It's simple XML. */
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
					/* On error, don't show the "report" button again. If the submission was actually
					* successful, then it'd be too easy to accidentally spam the server by holding
					* down the button. */
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), ERROR_SENDING_REPORT.GetValue() );
				}
				else if( xml.GetChildValue("UpdateAvailable", m_sUpdateURL) )
				{
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), UPDATE_IS_AVAILABLE.GetValue() );
					SetWindowText( GetDlgItem(hDlg, IDC_BUTTON_AUTO_REPORT), VIEW_UPDATE.GetValue() );
					ShowWindow( GetDlgItem(hDlg, IDC_BUTTON_AUTO_REPORT), true );
				}
				else if( xml.GetChildValue("ReportId", iID) )
				{
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), UPDATE_IS_NOT_AVAILABLE.GetValue() );
				}
				else
				{
					SetWindowText( GetDlgItem(hDlg, IDC_MAIN_TEXT), ERROR_SENDING_REPORT.GetValue() );
				}

				if( xml.GetChildValue("ReportId", iID) )
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
	// Read the crash data from the crashed parent.
	CompleteCrashData Data;
	ReadCrashDataFromParent( _fileno(stdin), Data );

	RString sCrashReport;
	VDDebugInfo::VDDebugInfoInitFromFile( &g_debugInfo );
	MakeCrashReport( Data, sCrashReport );
	VDDebugInfo::VDDebugInfoDeinit( &g_debugInfo );

	DoSave( sCrashReport );

	// Tell the crashing process that it can exit. Be sure to write crashinfo.txt first.
	fclose( stdout );

	// Now that we've done that, the process is gone. Don't use g_hParent.
	CloseHandle( SymbolLookup::g_hParent );
	SymbolLookup::g_hParent = nullptr;

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
