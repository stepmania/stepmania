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
// #include "RageSoundManager.h"
#include "RageMusic.h"
#include "RageInput.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageNetworkClient.h"
#include "RageMath.h"

#include "arch/arch.h"
#include "arch/LoadingWindow/LoadingWindow.h"

#include "SDL.h"
#include "SDL_syswm.h"		// for SDL_SysWMinfo

//
// StepMania global classes
//
#include "ThemeManager.h"
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
#include "GameConstantsAndTypes.h"

//#include "tls.h"
//#include "crash.h"


#include "SDL.h"
#include "SDL_opengl.h"


#ifdef _DEBUG
#pragma comment(lib, "SDL-1.2.5/lib/SDLmaind.lib")
#else
#pragma comment(lib, "SDL-1.2.5/lib/SDLmain.lib")
#endif
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.Lib")



// command line arguments
CString		g_sSongPath = "";
CString		g_sServerIP = "";

const int SM_PORT = 573;	// "Ko" + "na" + "mitsu"

/*------------------------------------------------
	Common stuff
------------------------------------------------*/
int		flags = 0;		/* SDL video flags */
int		window_w = SCREEN_WIDTH, window_h = SCREEN_HEIGHT;	/* window width and height */
LoadingWindow *loading_window = NULL;
CString  g_sErrorString = "";

void ChangeToDirOfExecutable()
{
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
}

void CreateLoadingWindow()
{
	ASSERT( loading_window == NULL );

	loading_window = MakeLoadingWindow();
	SDL_WM_SetCaption("StepMania", "StepMania");

	loading_window->Paint();
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
				/* Clear the startup mutex, since we're starting a new
				 * instance before ending ourself. */
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
// Name: ApplyGraphicOptions()
// Desc:
//-----------------------------------------------------------------------------
void ApplyGraphicOptions()
{ 
	bool ReloadTextures = DISPLAY->SetVideoMode( 
		PREFSMAN->m_bWindowed, 
		PREFSMAN->m_iDisplayWidth, 
		PREFSMAN->m_iDisplayHeight, 
		PREFSMAN->m_iDisplayColorDepth, 
		PREFSMAN->m_iRefreshRate,
		PREFSMAN->m_bVsync );

	if(TEXTUREMAN->SetPrefs( PREFSMAN->m_iTextureColorDepth, PREFSMAN->m_iUnloadTextureDelaySeconds ))
		ReloadTextures = true;

	if(ReloadTextures)
		TEXTUREMAN->ReloadAll(); 
}


void GameLoop();
#include "SDL_utils.h"
#include "SDL_image.h"
#include "SDL_rotozoom.h"
#include "StepMania.xpm" /* icon */

void SetIcon()
{
	SDL_Surface *srf = IMG_ReadXPMFromArray(icon);
	SDL_SetColorKey( srf, SDL_SRCCOLORKEY, SDL_MapRGB(srf->format, 0xFF, 0, 0xFF));

	/* Windows icons are 32x32 and SDL can't resize them for us, which
	 * causes mask corruption.  (Actually, the above icon *is* 32x32;
	 * this is here just in case it changes.) */
	ConvertSDLSurface(srf, srf->w, srf->h,
		32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	zoomSurface(srf, 32, 32);

	SDL_SetAlpha( srf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE );
	SDL_WM_SetIcon(srf, NULL /* derive from alpha */);
	SDL_FreeSurface(srf);
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Application entry point
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	ChangeToDirOfExecutable();

    atexit(SDL_Quit);   /* Clean up on exit */

	/*
	 * Handle command line args.
	 * Only allow one command line arg so we can validate the number of 
	 * parameters easier.
	 */
	if( argc > 1 )
		g_sSongPath = argv[1];
	if( argc > 2 )
		g_sServerIP = argv[2];

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	//	SetUnhandledExceptionFilter(CrashHandler);
//	InitThreadData("Main thread");
//	VDCHECKPOINT;

	SDL_Init(0);	/* Fire up the SDL, but don't actually start any subsystems. */

	atexit(SDL_Quit);

#ifndef _DEBUG
	try{
#endif

	/* Initialize the SDL library. */
    if( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
        throw RageException( "Couldn't initialize SDL: %s\n", SDL_GetError() );

	SetIcon();

	CreateLoadingWindow();

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
	SOUND		= new RageSound;
	MUSIC		= new RageSoundStream;
	ANNOUNCER	= new AnnouncerManager;
	INPUTFILTER	= new InputFilter;
	INPUTMAPPER	= new InputMapper;
	CLIENT		= new RageNetworkClient;
	INPUTQUEUE	= new InputQueue;
	SONGINDEX	= new SongCacheIndex;
	/* depends on SONGINDEX: */
	SONGMAN		= new SongManager( loading_window );		// this takes a long time to load

	delete loading_window;		// destroy this before init'ing Display

	PREFSMAN->ReadGlobalPrefsFromDisk( true );

	DISPLAY		= new RageDisplay(
		PREFSMAN->m_bWindowed, 
		PREFSMAN->m_iDisplayWidth, 
		PREFSMAN->m_iDisplayHeight, 
		PREFSMAN->m_iDisplayColorDepth, 
		PREFSMAN->m_iRefreshRate,
		PREFSMAN->m_bVsync );
	TEXTUREMAN	= new RageTextureManager( PREFSMAN->m_iTextureColorDepth, PREFSMAN->m_iUnloadTextureDelaySeconds );

	/* Grab the window manager specific information */
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if ( SDL_GetWMInfo(&info) < 0 ) 
		throw RageException( "SDL_GetWMInfo failed" );
	HWND hwnd = info.window;

	INPUTMAN	= new RageInput();
//	SOUNDMAN	= new RageSoundManager();

	// These things depend on the TextureManager, so do them after!
	FONT		= new FontManager;
	SCREENMAN	= new ScreenManager;

	/*
	 * Load initial screen depending on network mode
	 */
	if( g_sServerIP != "" )
	{
		// immediately try to connect to server
		GAMESTATE->m_pCurSong = SONGMAN->GetSongFromDir( g_sSongPath );
		if( GAMESTATE->m_pCurSong == NULL )
			throw RageException( "The song '%s' is required to play this network game.", g_sSongPath.GetString() );
		CLIENT->Connect( (const char*)g_sServerIP, SM_PORT );
		SCREENMAN->SetNewScreen( "ScreenNetworkWaiting" );
	}
	else
	{
		// normal game
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
	}

	/* Run the main loop. */
	GameLoop();

#ifndef _DEBUG
	}
	catch( RageException e )
	{
		g_sErrorString = e.what();

		LOG->Trace( 
			"\n"
			"//////////////////////////////////////////////////////\n"
			"Exception: %s\n"
			"//////////////////////////////////////////////////////\n"
			"\n",
			g_sErrorString.GetString()
			);

		LOG->Flush();
	}
#endif

	SAFE_DELETE( SCREENMAN );
	SAFE_DELETE( CLIENT );
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
//	SAFE_DELETE( SOUNDMAN );
	SAFE_DELETE( TIMER );
	SAFE_DELETE( FONT );
	SAFE_DELETE( TEXTUREMAN );
	SAFE_DELETE( DISPLAY );
	SAFE_DELETE( LOG );


	if( g_sErrorString != "" )
	{
		// throw up a pretty error dialog
		DialogBox(
			NULL,
			MAKEINTRESOURCE(IDD_ERROR_DIALOG),
			NULL,
			ErrorWndProc
			);
	}

	return 0;
}

void GameLoop()
{
	bool do_exit = false;
	SDL_Event	event;
	while(!do_exit)
	{
		// process all queued events
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				do_exit = 1;
				break;
			case SDL_VIDEORESIZE:
				PREFSMAN->m_iDisplayWidth = event.resize.w;
				PREFSMAN->m_iDisplayHeight = event.resize.h;
				ApplyGraphicOptions();
				break;
			}
		}

		/*
		 * Update
		 */
		float fDeltaTime = TIMER->GetDeltaTime();
		
		// This was a hack to fix timing issues with the old ScreenSelectSong
		// See ScreenManager::Update comments for why we shouldn't do this. -glenn
		//if( fDeltaTime > 0.050f )	// we dropped a bunch of frames
		// 	fDeltaTime = 0.050f;
		if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_TAB) ) ) {
			if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_QUOTEDBL) ) )
				fDeltaTime = 0; /* both; stop time */
			else
				fDeltaTime *= 4;
		} else
			if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_QUOTEDBL) ) )
				fDeltaTime /= 4;

		TEXTUREMAN->Update( fDeltaTime );
		MUSIC->Update( fDeltaTime );
		SCREENMAN->Update( fDeltaTime );
		CLIENT->Update( fDeltaTime );

		static InputEventArray ieArray;
		ieArray.clear();	// empty the array
		INPUTFILTER->GetInputEvents( ieArray, fDeltaTime );
		for( unsigned i=0; i<ieArray.size(); i++ )
		{
			DeviceInput DeviceI = (DeviceInput)ieArray[i];
			InputEventType type = ieArray[i].type;

			/* ALT-F4 -> quit (better place for this? in ScreenManager perhaps?) */
			/* Nah.  this is fine.  -Chris */
			if(type == IET_FIRST_PRESS && DeviceI == DeviceInput(DEVICE_KEYBOARD, SDLK_F4))
			{
				if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RALT)) ||
					INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LALT)) )
				{
					// pressed Alt+F4
					SDL_Event *event;
					event = (SDL_Event *) malloc(sizeof(event));
					event->type = SDL_QUIT;
					SDL_PushEvent(event);
					continue;
				}
				else
				{
					// pressed just F4
					PREFSMAN->m_bWindowed = !PREFSMAN->m_bWindowed;
					ApplyGraphicOptions();
					// fall through
				}
			}
			else if( type == IET_FIRST_PRESS && DeviceI == DeviceInput(DEVICE_KEYBOARD, SDLK_F5))
			{
				// pressed F5.  Toggle detail.
				if(PREFSMAN->m_iDisplayWidth != 640)
				{
					PREFSMAN->m_iDisplayWidth = 640;
					PREFSMAN->m_iDisplayHeight = 480;
					ApplyGraphicOptions();
				}			
				else
				{
					PREFSMAN->m_iDisplayWidth = 320;
					PREFSMAN->m_iDisplayHeight = 240;
					ApplyGraphicOptions();
				}			
	
			}

			if(type == IET_FIRST_PRESS && DeviceI == DeviceInput(DEVICE_KEYBOARD, SDLK_RETURN))
			{
				if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RALT)) ||
					INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LALT)) )
				{
					/* alt-enter */
					PREFSMAN->m_bWindowed = !PREFSMAN->m_bWindowed;
					ApplyGraphicOptions();
					continue;
				}
			}

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

		/*
		 * Render
		 */
		DISPLAY->Clear();
		DISPLAY->ResetMatrixStack();

		RageMatrix mat;

		RageMatrixIdentity( &mat );
		DISPLAY->SetViewTransform( &mat );

		SCREENMAN->Draw();		// draw the game

		DISPLAY->FlushQueue();
		DISPLAY->Flip();

		if( DISPLAY  &&  DISPLAY->IsWindowed() )
			::Sleep( 0 );	// give some time to other processes
	}
}

