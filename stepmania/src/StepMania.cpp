#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: StepMania.cpp

 Desc: Entry point for program.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "resource.h"

//
// Rage global classes
//
#include "RageLog.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"
#include "RageSound.h"
#include "RageMusic.h"
#include "RageInput.h"
#include "RageTimer.h"
#include "RageException.h"

//
// StepMania global classes
//
#include "PrefsManager.h"
#include "SongManager.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "ScreenManager.h"
#include "GameManager.h"
#include "FontManager.h"
#include "InputFilter.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "SongCacheIndex.h"

//
// StepMania common classes
//
#include "Font.h"
#include "GameConstantsAndTypes.h"
#include "GameInput.h"
#include "StyleInput.h"
#include "Song.h"
#include "StyleDef.h"
#include "NoteData.h"
#include "Notes.h"


#include "dxerr8.h"
//#include <Afxdisp.h>

//-----------------------------------------------------------------------------
// Links
//-----------------------------------------------------------------------------
#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "d3d8.lib")


//-----------------------------------------------------------------------------
// Application globals
//-----------------------------------------------------------------------------
const CString g_sAppName		= "StepMania";
const CString g_sAppClassName	= "StepMania Class";

HINSTANCE	g_hInstance;	// The Handle to Window Instance
HWND		g_hWndMain;		// Main Window Handle
HWND		g_hWndLoading;	// Loading Window Handle
HANDLE		g_hMutex;		// Used to check if an instance of our app is already
const DWORD g_dwWindowStyle = WS_POPUP|WS_CAPTION|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU;
BOOL		g_bIsActive		= FALSE;	// Whether the focus is on our app


//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
// Main game functions
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK ErrorWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK LoadingWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow );	// windows entry point
void MainLoop();		// put everything in here so we can wrap it in a try...catch block
void Update();			// Update the game logic
void Render();			// Render a frame
void ShowFrame();		// Display the contents of the back buffer to the Window

// Functions that work with game objects
HRESULT		CreateObjects( HWND hWnd );	// allocate and initialize game objects
HRESULT		InvalidateObjects();		// invalidate game objects before a display mode change
HRESULT		RestoreObjects();			// restore game objects after a display mode change
VOID		DestroyObjects();			// deallocate game objects when we're done with them

void CreateLoadingWindow();
void PaintLoadingWindow();
void DestroyLoadingWindow();

void ApplyGraphicOptions();	// Set the display mode according to the user's preferences

CString		g_sErrorString;

//-----------------------------------------------------------------------------
// Name: StructuredExceptionHandler()
// Desc: Callback for SEH exceptions
//-----------------------------------------------------------------------------
void StructuredExceptionHandler(unsigned int uCode,
								struct _EXCEPTION_POINTERS* /* pXPointers */)
{
	const char* msg;
	switch( uCode )
	{
	case EXCEPTION_ACCESS_VIOLATION:		msg = "Access Violation";						break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:	msg = "Array Bounds Exceeded";					break;
	case EXCEPTION_STACK_OVERFLOW:			msg = "Stack Overflow";							break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:	msg = "Floating Point Denormal Operation";		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:		msg = "Floating Point Divide by Zero";			break;
	case EXCEPTION_FLT_INVALID_OPERATION:	msg = "Floating Point Invalid Operation";		break;
	case EXCEPTION_FLT_UNDERFLOW:			msg = "Floating Point Underflow";				break;
	case EXCEPTION_FLT_OVERFLOW:			msg = "Floating Point Overflow";				break;
	case EXCEPTION_FLT_STACK_CHECK:			msg = "Floating Point Stack Over/Underflow";	break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:		msg = "Integer Divide by Zero";					break;
	case EXCEPTION_INT_OVERFLOW:			msg = "Integer Overflow";						break;
	default:								msg = "Unknown Exception";						break;
	}
	throw std::exception(msg);
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow )
{
	_set_se_translator( StructuredExceptionHandler );

	g_hInstance = hInstance;
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);
#endif

#ifndef _DEBUG
	try
	{
#endif

		CreateLoadingWindow();
		
		//
		// Check to see if the app is already running.
		//
		g_hMutex = CreateMutex( NULL, TRUE, g_sAppName );
		if( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			MessageBox( NULL, "StepMania is already running.", "FatalError", MB_OK );
			exit( 1 );
		}


		//
		// Make sure the current directory is the root program directory
		//
		if( !DoesFileExist("Songs") )
		{
			// change dir to path of the execuctable
			TCHAR szFullAppPath[MAX_PATH];
			GetModuleFileName(NULL, szFullAppPath, MAX_PATH);
			
			// strip off executable name
			LPSTR pLastBackslash = strrchr(szFullAppPath, '\\');
			*pLastBackslash = '\0';	// terminate the string

			SetCurrentDirectory( szFullAppPath );
		}

		//
		// Check for packages in the AutoInstall folder
		//
		CStringArray asPackagePaths;
		GetDirListing( "AutoInstall\\*.smzip", asPackagePaths, false, true );
		for( int i=0; i<asPackagePaths.GetSize(); i++ )
		{
			CString sCommandLine = ssprintf( "smpackage.exe %s", asPackagePaths[i] );

			PROCESS_INFORMATION pi;
			STARTUPINFO	si;
			ZeroMemory( &si, sizeof(si) );

			CreateProcess(
				NULL,		// pointer to name of executable module
				sCommandLine.GetBuffer(MAX_PATH),		// pointer to command line string
				NULL,  // process security attributes
				NULL,   // thread security attributes
				false,  // handle inheritance flag
				0, // creation flags
				NULL,  // pointer to new environment block
				NULL,   // pointer to current directory name
				&si,  // pointer to STARTUPINFO
				&pi  // pointer to PROCESS_INFORMATION
			);
		}


		CoInitialize (NULL);    // Initialize COM

		// Register the window class
		WNDCLASS wndClass = { 
			0,
			WndProc,	// callback handler
			0,			// cbClsExtra; 
			0,			// cbWndExtra; 
			hInstance,
			LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ICON) ), 
			LoadCursor( hInstance, IDC_ARROW),
			(HBRUSH)GetStockObject( BLACK_BRUSH ),
			NULL,				// lpszMenuName; 
			g_sAppClassName	// lpszClassName; 
		}; 
 		RegisterClass( &wndClass );


		// Set the window's initial width
		RECT rcWnd;
		SetRect( &rcWnd, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
		AdjustWindowRect( &rcWnd, g_dwWindowStyle, FALSE );


		// Create our main window
		g_hWndMain = CreateWindow(
						g_sAppClassName,// pointer to registered class name
						g_sAppName,		// pointer to window name
						g_dwWindowStyle,	// window StyleDef
						CW_USEDEFAULT,	// horizontal position of window
						CW_USEDEFAULT,	// vertical position of window
						RECTWIDTH(rcWnd),	// window width
						RECTHEIGHT(rcWnd),// window height
						NULL,				// handle to parent or owner window
						NULL,				// handle to menu, or child-window identifier
						hInstance,		// handle to application instance
						NULL				// pointer to window-creation data
					);
 		if( NULL == g_hWndMain )
			exit(1);

		ShowWindow( g_hWndMain, SW_HIDE );


		// Load keyboard accelerators
		HACCEL hAccel = LoadAccelerators( NULL, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

		// run the game
		CreateObjects( g_hWndMain );	// Create the game objects


		// Now we're ready to recieve and process Windows messages.
		MSG msg;
		ZeroMemory( &msg, sizeof(msg) );

		while( WM_QUIT != msg.message  )
		{
			// Look for messages, if none are found then 
			// update the state and display it
			if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{
				GetMessage(&msg, NULL, 0, 0 );

				// Translate and dispatch the message
				if( 0 == TranslateAccelerator( g_hWndMain, hAccel, &msg ) )
				{
					TranslateMessage( &msg ); 
					DispatchMessage( &msg );
				}
			}
			else	// No messages are waiting.  Render a frame during idle time.
			{
				Update();
				Render();
				if( !g_bIsActive  &&  DISPLAY  &&  DISPLAY->IsWindowed() )
					::Sleep( 0 );	// give some time to other processes
#ifdef _DEBUG
				::Sleep( 1 );
#endif
			}
		}	// end  while( WM_QUIT != msg.message  )

		LOG->Trace( "Recieved WM_QUIT message.  Shutting down..." );

		// clean up after a normal exit 
		DestroyObjects();			// deallocate our game objects and leave fullscreen
		ShowWindow( g_hWndMain, SW_HIDE );


#ifndef _DEBUG
	}
	catch( RageException e )
	{
		g_sErrorString = e.what();
	}
	catch( exception e )
	{
		g_sErrorString = e.what();
	}
	catch( ... )
	{
		g_sErrorString = "Unknown exception";
	}

	if( g_sErrorString != "" )
	{

		if( LOG )
		{
			LOG->Flush();

			LOG->Trace( 
				"\n"
				"//////////////////////////////////////////////////////\n"
				"Exception: %s\n"
				"//////////////////////////////////////////////////////\n"
				"\n",
				g_sErrorString
				);

		// throw up a pretty error dialog
		DialogBox(
			g_hInstance,
			MAKEINTRESOURCE(IDD_ERROR_DIALOG),
			NULL,
			ErrorWndProc
			);

		}

	}
#endif

	DestroyWindow( g_hWndMain );
	UnregisterClass( g_sAppClassName, hInstance );
	CoUninitialize();			// Uninitialize COM
	CloseHandle( g_hMutex );

#ifdef _DEBUG
	_CrtCheckMemory();

	_CrtDumpMemoryLeaks();
#endif

	return 0L;
}


void CreateLoadingWindow()
{
	// throw up a pretty error dialog
	g_hWndLoading = CreateDialog(
		g_hInstance,
		MAKEINTRESOURCE(IDD_LOADING_DIALOG),
		NULL,
		LoadingWndProc
		);
}

void PaintLoadingWindow()
{
	SendMessage( g_hWndLoading, WM_PAINT, 0, 0 );
}

void DestroyLoadingWindow()
{
	EndDialog( g_hWndLoading, 0 );
}

//-----------------------------------------------------------------------------
// Name: LoadingWndProc()
// Desc: Callback for all Windows messages
//-----------------------------------------------------------------------------
BOOL CALLBACK LoadingWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
		}
		break;
	case WM_COMMAND:
		break;
	case WM_PAINT:
		{
			CStringArray asMessageLines;
			if( GAMESTATE  &&  GAMESTATE->m_sLoadingMessage != "" )
				split( GAMESTATE->m_sLoadingMessage, "\n", asMessageLines, false );
			else
				asMessageLines.Add( "Initializing hardware..." );

			SendDlgItemMessage( 
				hWnd, 
				IDC_STATIC_MESSAGE1, 
				WM_SETTEXT, 
				0, 
				(LPARAM)(LPCTSTR)asMessageLines[0]
			);
			SendDlgItemMessage( 
				hWnd, 
				IDC_STATIC_MESSAGE2, 
				WM_SETTEXT, 
				0, 
				(LPARAM)(LPCTSTR)(asMessageLines.GetSize()>=2 ? asMessageLines[1] : "")
			);
			SendDlgItemMessage( 
				hWnd, 
				IDC_STATIC_MESSAGE3, 
				WM_SETTEXT, 
				0, 
				(LPARAM)(LPCTSTR)(asMessageLines.GetSize()>=3 ? asMessageLines[2] : "")
			);
		}
		break;

	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// Name: ErrorWndProc()
// Desc: Callback for all Windows messages
//-----------------------------------------------------------------------------
BOOL CALLBACK ErrorWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			CString sMessage = g_sErrorString;

			sMessage.Replace( "\n", "\r\n" );
			
			SendDlgItemMessage( 
				hWnd, 
				IDC_EDIT_ERROR, 
				WM_SETTEXT, 
				0, 
				(LPARAM)(LPCTSTR)sMessage
				);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_VIEW_LOG:
			{
				PROCESS_INFORMATION pi;
				STARTUPINFO	si;
				ZeroMemory( &si, sizeof(si) );

				CreateProcess(
					NULL,		// pointer to name of executable module
					"notepad.exe log.txt",		// pointer to command line string
					NULL,  // process security attributes
					NULL,   // thread security attributes
					false,  // handle inheritance flag
					0, // creation flags
					NULL,  // pointer to new environment block
					NULL,   // pointer to current directory name
					&si,  // pointer to STARTUPINFO
					&pi  // pointer to PROCESS_INFORMATION
				);
			}
			break;
		case IDC_BUTTON_REPORT:
			GotoURL( "http://sourceforge.net/tracker/?func=add&group_id=37892&atid=421366" );
			break;
		case IDC_BUTTON_RESTART:
			{
				// Launch StepMania
				PROCESS_INFORMATION pi;
				STARTUPINFO	si;
				ZeroMemory( &si, sizeof(si) );

				CreateProcess(
					NULL,		// pointer to name of executable module
					"stepmania.exe",		// pointer to command line string
					NULL,  // process security attributes
					NULL,   // thread security attributes
					false,  // handle inheritance flag
					0, // creation flags
					NULL,  // pointer to new environment block
					NULL,   // pointer to current directory name
					&si,  // pointer to STARTUPINFO
					&pi  // pointer to PROCESS_INFORMATION
				);
			}
			EndDialog( hWnd, 0 );
			break;
			// fall through
		case IDOK:
			EndDialog( hWnd, 0 );
			break;
		}
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Callback for all Windows messages
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_ACTIVATEAPP:
            // Check to see if we are losing our window...
			g_bIsActive = (BOOL)wParam;
			break;

		case WM_SIZE:
            // Check to see if we are losing our window...
			if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                g_bIsActive = FALSE;
            else
                g_bIsActive = TRUE;
            break;

		case WM_GETMINMAXINFO:
			{
				// Don't allow the window to be resized smaller than the screen resolution.
				// This should snap to multiples of the Window size two!
				RECT rcWnd;
				SetRect( &rcWnd, 0, 0, PREFSMAN->m_iDisplayResolution, PREFSMAN->GetDisplayHeight() );
				DWORD dwWindowStyle = GetWindowLong( g_hWndMain, GWL_STYLE );
				AdjustWindowRect( &rcWnd, dwWindowStyle, FALSE );

				((MINMAXINFO*)lParam)->ptMinTrackSize.x = RECTWIDTH(rcWnd);
				((MINMAXINFO*)lParam)->ptMinTrackSize.y = RECTHEIGHT(rcWnd);
			}
			break;

		case WM_SETCURSOR:
			// Turn off Windows cursor in fullscreen mode
			if( DISPLAY && !DISPLAY->IsWindowed() )
            {
                SetCursor( NULL );
                return TRUE; // prevent Windows from setting the cursor
            }
            break;

		case WM_SYSCOMMAND:
			// Prevent moving/sizing and power loss
			switch( wParam )
			{
				case SC_MOVE:
				case SC_SIZE:
				case SC_KEYMENU:
				case SC_MONITORPOWER:
					return 1;
				case SC_MAXIMIZE:
					//SendMessage( g_hWndMain, WM_COMMAND, IDM_TOGGLEFULLSCREEN, 0 );
					//return 1;
					break;
			}
			break;


		case WM_COMMAND:
		{
            switch( LOWORD(wParam) )
            {
				case IDM_TOGGLEFULLSCREEN:
					PREFSMAN->m_bWindowed = !PREFSMAN->m_bWindowed;
					ApplyGraphicOptions();
					return 0;
				case IDM_CHANGEDETAIL:
					if( PREFSMAN->m_iDisplayResolution != 640 )
						PREFSMAN->m_iDisplayResolution = 640;
					else
						PREFSMAN->m_iDisplayResolution = 400;

					ApplyGraphicOptions();
					return 0;
               case IDM_EXIT:
                    // Recieved key/menu command to exit app
                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0;
            }
            break;
		}
        case WM_NCHITTEST:
            // Prevent the user from selecting the menu in fullscreen mode
            if( DISPLAY && !DISPLAY->IsWindowed() )
                return HTCLIENT;
            break;

		case WM_PAINT:
			// redisplay the contents of the back buffer if the window needs to be redrawn
			ShowFrame();
			break;

		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}


//-----------------------------------------------------------------------------
// Name: CreateObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CreateObjects( HWND hWnd )
{

	/*
	//
	// Draw a splash bitmap so the user isn't looking at a black Window
	//
	HBITMAP hSplashBitmap = (HBITMAP)LoadImage( 
		GetModuleHandle( NULL ),
		TEXT("BITMAP_SPLASH"), 
		IMAGE_BITMAP,
		0, 0, LR_CREATEDIBSECTION );
    BITMAP bmp;
    RECT rc;
    GetClientRect( hWnd, &rc );

    // Display the splash bitmap in the window
    HDC hDCWindow = GetDC( hWnd );
    HDC hDCImage  = CreateCompatibleDC( NULL );
    SelectObject( hDCImage, hSplashBitmap );
    GetObject( hSplashBitmap, sizeof(bmp), &bmp );
    StretchBlt( hDCWindow, 0, 0, rc.right, rc.bottom,
                hDCImage, 0, 0,
                bmp.bmWidth, bmp.bmHeight, SRCCOPY );
    DeleteDC( hDCImage );
	
	// Delete the bitmap
	DeleteObject( hSplashBitmap );

    ReleaseDC( hWnd, hDCWindow );
*/


	//
	// Create game objects
	//
	srand( (unsigned)time(NULL) );	// seed number generator
	
	LOG			= new RageLog();
#ifdef _DEBUG
	LOG->ShowConsole();
#endif
	TIMER		= new RageTimer;
	GAMESTATE	= new GameState;
	PREFSMAN	= new PrefsManager;
	GAMEMAN		= new GameManager;
	THEME		= new ThemeManager;
	SOUND		= new RageSound( hWnd );
	MUSIC		= new RageSoundStream;
	INPUTMAN	= new RageInput( hWnd );
	ANNOUNCER	= new AnnouncerManager;
	INPUTFILTER	= new InputFilter();
	INPUTMAPPER	= new InputMapper();
	INPUTQUEUE	= new InputQueue();
	SONGINDEX	= new SongCacheIndex();
	/* depends on SONGINDEX: */
	SONGMAN		= new SongManager( PaintLoadingWindow );		// this takes a long time to load
	DISPLAY		= new RageDisplay( hWnd );

	DestroyLoadingWindow();

	ShowWindow( g_hWndMain, SW_SHOW );	// show the window

	// We can't do any texture loading unless the D3D device is created.  
	// Set the display mode to make sure the D3D device is created.
	ApplyGraphicOptions(); 

	TEXTUREMAN	= new RageTextureManager( DISPLAY );

	PREFSMAN->ReadGlobalPrefsFromDisk( true );

	// These things depend on the TextureManager, so do them after!
	FONT		= new FontManager;
	SCREENMAN	= new ScreenManager;

	BringWindowToTop( hWnd );
	SetForegroundWindow( hWnd );

	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
//	SCREENMAN->SetNewScreen( "ScreenSandbox" );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DestroyObjects()
// Desc:
//-----------------------------------------------------------------------------
void DestroyObjects()
{
	SAFE_DELETE( SCREENMAN );
	SAFE_DELETE( INPUTQUEUE );
	SAFE_DELETE( INPUTMAPPER );
	SAFE_DELETE( INPUTFILTER );
	SAFE_DELETE( SONGMAN );
	SAFE_DELETE( SONGINDEX );
	SAFE_DELETE( PREFSMAN );
	SAFE_DELETE( GAMESTATE );
	SAFE_DELETE( GAMEMAN );
	SAFE_DELETE( THEME );
	SAFE_DELETE( ANNOUNCER );
	SAFE_DELETE( INPUTMAN );
	SAFE_DELETE( MUSIC );
	SAFE_DELETE( SOUND );
	SAFE_DELETE( TIMER );
	SAFE_DELETE( FONT );
	SAFE_DELETE( TEXTUREMAN );
	SAFE_DELETE( DISPLAY );
	SAFE_DELETE( LOG );
}


//-----------------------------------------------------------------------------
// Name: RestoreObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT RestoreObjects()
{
	/////////////////////
	// Restore the window
	/////////////////////

    // Set window size
    RECT rcWnd;
	DWORD dwWindowStyle = g_dwWindowStyle;

	if( DISPLAY->IsWindowed() )
	{
//		dwWindowStyle &= WS_THICKFRAME;
//		SetWindowLong( g_hWndMain, GWL_STYLE, dwWindowStyle );
		SetRect( &rcWnd, 0, 0, PREFSMAN->m_iDisplayResolution, PREFSMAN->GetDisplayHeight() );
  		AdjustWindowRect( &rcWnd, dwWindowStyle, FALSE );
	}
	else	// if fullscreen
	{
//		SetWindowLong( g_hWndMain, GWL_STYLE, dwWindowStyle );
		SetRect( &rcWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) );
	}

	// Bring the window to the foreground
    SetWindowPos( g_hWndMain, 
				  HWND_NOTOPMOST, 
				  0, 
				  0, 
				  RECTWIDTH(rcWnd), 
				  RECTHEIGHT(rcWnd),
                  0 );



	///////////////////////////
	// Restore all game objects
	///////////////////////////

	DISPLAY->Restore();

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InvalidateObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT InvalidateObjects()
{
	DISPLAY->Invalidate();
	
	return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Update()
// Desc:
//-----------------------------------------------------------------------------
void Update()
{
	float fDeltaTime = TIMER->GetDeltaTime();
	
	// This was a hack to fix timing issues with the old ScreenSelectSong
	//
	if( fDeltaTime > 0.050f )	// we dropped a bunch of frames
		fDeltaTime = 0.050f;
	
	if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, DIK_TAB) ) )
		fDeltaTime *= 4;
	if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, DIK_GRAVE) ) )
		fDeltaTime /= 4;


	MUSIC->Update( fDeltaTime );

	SCREENMAN->Update( fDeltaTime );


	static InputEventArray ieArray;
	ieArray.SetSize( 0, 20 );	// zero the array
	INPUTFILTER->GetInputEvents( ieArray, fDeltaTime );

	for( int i=0; i<ieArray.GetSize(); i++ )
	{
		DeviceInput DeviceI = (DeviceInput)ieArray[i];
		InputEventType type = ieArray[i].type;
		GameInput GameI;
		MenuInput MenuI;
		StyleInput StyleI;

		INPUTMAPPER->DeviceToGame( DeviceI, GameI );
		
		if( GameI.IsValid()  &&  type == IET_FIRST_PRESS )
			INPUTQUEUE->RememberInput( GameI );
		if( GameI.IsValid() )
		{
			INPUTMAPPER->GameToMenu( GameI, MenuI );
			INPUTMAPPER->GameToStyle( GameI, StyleI );
		}

		SCREENMAN->Input( DeviceI, type, GameI, MenuI, StyleI );
	}

}


//-----------------------------------------------------------------------------
// Name: Render()
// Desc:
//-----------------------------------------------------------------------------
void Render()
{
	HRESULT hr = DISPLAY->BeginFrame();
	switch( hr )
	{
		case D3DERR_DEVICELOST:
			// The user probably alt-tabbed out of fullscreen.
			// Do not render a frame until we re-acquire the device
			break;
		case D3DERR_DEVICENOTRESET:
			InvalidateObjects();

            // Resize the device
            if( SUCCEEDED( hr = DISPLAY->Reset() ) )
            {
                // Initialize the app's device-dependent objects
                RestoreObjects();
				return;
            }
			else
			{
				throw RageException( hr, "Failed to DISPLAY->Reset()" );
			}

			break;
		case S_OK:
			{
				// calculate view and projection transforms
				D3DXMATRIX mat;
			
				D3DXMatrixOrthoOffCenterLH( &mat, 0, 640, 480, 0, -1000, 1000 );
				DISPLAY->SetProjectionTransform( &mat );

				D3DXMatrixIdentity( &mat );
				DISPLAY->SetViewTransform( &mat );

				DISPLAY->ResetMatrixStack();

				// draw the game
				SCREENMAN->Draw();

				DISPLAY->EndFrame();
			}
			break;
	}

	ShowFrame();
}


//-----------------------------------------------------------------------------
// Name: ShowFrame()
// Desc:
//-----------------------------------------------------------------------------
void ShowFrame()
{
	// display the contents of the back buffer to the front
	if( DISPLAY )
		DISPLAY->ShowFrame();
}


//-----------------------------------------------------------------------------
// Name: ApplyGraphicOptions()
// Desc:
//-----------------------------------------------------------------------------
void ApplyGraphicOptions()
{
	InvalidateObjects();

	bool &bWindowed			= PREFSMAN->m_bWindowed;
	
	int &iDisplayWidth	= PREFSMAN->m_iDisplayResolution;
	int iDisplayHeight	= PREFSMAN->GetDisplayHeight();

	int iDisplayBPP		= 16;
	
	int &iTextureSize	= PREFSMAN->m_iTextureResolution;
	switch( iTextureSize )
	{
	case 1024:	break;
	case 512:	break;
	case 256:	break;
	default:	throw RageException( "Invalid TextureResolution '%d'", iTextureSize );
	}

	int iTextureBPP		= 16;

	int &iRefreshRate	= PREFSMAN->m_iRefreshRate;

	CString sMessage = ssprintf( "%s - %dx%d, %dx%d textures%s", 
		bWindowed ? "Windowed" : "FullScreen", 
		iDisplayWidth, 
		iDisplayHeight, 
		iTextureSize, 
		iTextureSize,
		bWindowed ? "" : ssprintf(", %dHz", iRefreshRate) 
		);

	//
	// If the requested resolution doesn't work, keep switching until we find one that does.
	//
	if( DISPLAY->SwitchDisplayMode(bWindowed, iDisplayWidth, iDisplayHeight, iDisplayBPP, iRefreshRate) )
		goto success;

	// We failed.  Using default refresh rate.
	iRefreshRate = 0;
	if( DISPLAY->SwitchDisplayMode(bWindowed, iDisplayWidth, iDisplayHeight, iDisplayBPP, iRefreshRate) )
		goto success;

	// We failed.  Try full screen with same params.
	bWindowed = false;
	if( DISPLAY->SwitchDisplayMode(bWindowed, iDisplayWidth, iDisplayHeight, iDisplayBPP, iRefreshRate) )
		goto success;

	// Failed again.  Try 16 BPP
	iDisplayBPP = 16;
	if( DISPLAY->SwitchDisplayMode(bWindowed, iDisplayWidth, iDisplayHeight, iDisplayBPP, iRefreshRate) )
		goto success;

	// Failed again.  Try 640x480
	iDisplayWidth = 640;
	iDisplayHeight = 480;
	if( DISPLAY->SwitchDisplayMode(bWindowed, iDisplayWidth, iDisplayHeight, iDisplayBPP, iRefreshRate) )
		goto success;

	// Failed again.  Try 320x240
	iDisplayWidth = 320;
	iDisplayHeight = 240;
	if( DISPLAY->SwitchDisplayMode(bWindowed, iDisplayWidth, iDisplayHeight, iDisplayBPP, iRefreshRate) )
		goto success;

	throw RageException( "Tried every possible display mode, and couldn't find one that works." );

success:

	//
	// Let the texture manager know about our preferences
	//
	if( TEXTUREMAN != NULL )
		TEXTUREMAN->SetPrefs( iTextureSize, iTextureBPP );

	RestoreObjects();

	PREFSMAN->SaveGlobalPrefsToDisk();

	if( SCREENMAN )
		SCREENMAN->SystemMessage( sMessage );

}





