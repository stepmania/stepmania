#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: StepMania.cpp

 Desc: Entry point for program.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

//
// Rage global classes
//
#include "RageLog.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"
#include "RageSoundManager.h"
#include "RageInput.h"
#include "RageTimer.h"
#include "RageException.h"
#include "RageMath.h"

#include "arch/arch.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "arch/ErrorDialog/ErrorDialog.h"

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

#include "tls.h"
#include "crash.h"


#include "SDL.h"
#include "SDL_opengl.h"


#ifdef _DEBUG
#pragma comment(lib, "SDL-1.2.5/lib/SDLmaind.lib")
#else
#pragma comment(lib, "SDL-1.2.5/lib/SDLmain.lib")
#endif
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.Lib")

#ifdef WIN32
HWND g_hWndMain = NULL;
#endif

#include <time.h>


static void ChangeToDirOfExecutable(const char *argv0)
{
	/* Make sure the current directory is the root program directory
	 * We probably shouldn't do this; rather, we should know where things
	 * are and use paths as needed, so we don't depend on the binary being
	 * in the same place as "Songs" ... */
	if( !DoesFileExist("Songs") )
	{
		// strip off executable name
		CString dir = argv0;
		unsigned n = dir.find_last_of("/\\");
		if (n != dir.npos) dir.erase(n);

		chdir( dir.GetString() );
	}
}

void ApplyGraphicOptions()
{ 
	bool ReloadTextures = DISPLAY->SetVideoMode( 
		PREFSMAN->m_bWindowed, 
		PREFSMAN->m_iDisplayWidth, 
		PREFSMAN->m_iDisplayHeight, 
		PREFSMAN->m_iDisplayColorDepth, 
		PREFSMAN->m_iRefreshRate,
		PREFSMAN->m_bVsync );

	if(TEXTUREMAN->SetPrefs( PREFSMAN->m_iTextureColorDepth, PREFSMAN->m_iUnloadTextureDelaySeconds, PREFSMAN->m_iMaxTextureResolution ))
		ReloadTextures = true;

	if(ReloadTextures)
		TEXTUREMAN->ReloadAll(); 
}

void Exit()
{
	SDL_Event *event;
	event = (SDL_Event *) malloc(sizeof(event));
	event->type = SDL_QUIT;
	SDL_PushEvent(event);
}

void Reset()
{
	GAMESTATE->Reset();
	PREFSMAN->ReadGamePrefsFromDisk();
	INPUTMAPPER->ReadMappingsFromDisk();
	GAMESTATE->m_bPlayersCanJoin = true;

	if( !GAMEMAN->DoesNoteSkinExist( GAMEMAN->GetCurNoteSkin() ) )
	{
		CStringArray asNoteSkinNames;
		GAMEMAN->GetNoteSkinNames( asNoteSkinNames );
		GAMEMAN->SwitchNoteSkin( asNoteSkinNames[0] );
	}
	if( !THEME->DoesThemeExist( THEME->GetCurThemeName() ) )
	{
		CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
		if( THEME->DoesThemeExist( sGameName ) )
			THEME->SwitchTheme( sGameName );
		else
			THEME->SwitchTheme( "default" );
	}
	PREFSMAN->SaveGamePrefsToDisk();

	SCREENMAN->SetNewScreen( "ScreenCompany" );	// Change this to set a different initial screen
}

static void GameLoop();
#include "SDL_utils.h"
#include "SDL_image.h"
#include "SDL_rotozoom.h"
#include "StepMania.xpm" /* icon */

static void SetIcon()
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

int main(int argc, char* argv[])
{
	ChangeToDirOfExecutable(argv[0]);

    atexit(SDL_Quit);   /* Clean up on exit */

	SetUnhandledExceptionFilter(CrashHandler);
	InitThreadData("Main thread");
	VDCHECKPOINT;

	/* Fire up the SDL, but don't actually start any subsystems. */
	int SDL_flags = 0;
#ifdef WIN32
	/* We use our own error handler in Windows. */
	SDL_flags |= SDL_INIT_NOPARACHUTE;
#endif
	SDL_Init(SDL_flags);

	atexit(SDL_Quit);

	CString  g_sErrorString = "";

#ifndef _DEBUG
	try{
#endif

	/* Initialize the SDL library. */
    if( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
        RageException::Throw( "Couldn't initialize SDL: %s\n", SDL_GetError() );

	SetIcon();

	LoadingWindow *loading_window = MakeLoadingWindow();
	/* This might be using SDL, so reset the caption. */
	SDL_WM_SetCaption("StepMania", "StepMania");
	loading_window->Paint();

	//
	// Create game objects
	//
	srand( (unsigned)time(NULL) );	// seed number generator
	
	LOG			= new RageLog();
#ifdef _DEBUG
	LOG->ShowConsole();
#endif
	GAMESTATE	= new GameState;
	PREFSMAN	= new PrefsManager;
	GAMEMAN		= new GameManager;
	THEME		= new ThemeManager;
	SOUNDMAN	= new RageSoundManager(PREFSMAN->m_bSoundDrivers);
	ANNOUNCER	= new AnnouncerManager;
	INPUTFILTER	= new InputFilter;
	INPUTMAPPER	= new InputMapper;
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
	TEXTUREMAN	= new RageTextureManager( PREFSMAN->m_iTextureColorDepth, PREFSMAN->m_iUnloadTextureDelaySeconds, PREFSMAN->m_iMaxTextureResolution );

	/* Grab the window manager specific information */
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if ( SDL_GetWMInfo(&info) < 0 ) 
		RageException::Throw( "SDL_GetWMInfo failed" );

#ifdef WIN32
	g_hWndMain = info.window;
#endif

	INPUTMAN	= new RageInput();

	// These things depend on the TextureManager, so do them after!
	FONT		= new FontManager;
	SCREENMAN	= new ScreenManager;

	Reset();

	/* Run the main loop. */
	GameLoop();

	PREFSMAN->SaveGlobalPrefsToDisk();
	PREFSMAN->SaveGamePrefsToDisk();

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
	SAFE_DELETE( SOUNDMAN );
	SAFE_DELETE( FONT );
	SAFE_DELETE( TEXTUREMAN );
	SAFE_DELETE( DISPLAY );
	SAFE_DELETE( LOG );


	if( g_sErrorString != "" )
	{
		// throw up a pretty error dialog
		ErrorDialog *d = MakeErrorDialog();
		d->SetErrorText(g_sErrorString);
		d->ShowError();
		delete d;
	}

	return 0;
}

static void HandleInputEvents(float fDeltaTime)
{
	static InputEventArray ieArray;
	ieArray.clear();	// empty the array
	INPUTFILTER->GetInputEvents( ieArray, fDeltaTime );
	for( unsigned i=0; i<ieArray.size(); i++ )
	{
		DeviceInput DeviceI = (DeviceInput)ieArray[i];
		InputEventType type = ieArray[i].type;

		if(type == IET_FIRST_PRESS && DeviceI == DeviceInput(DEVICE_KEYBOARD, SDLK_F4))
		{
			if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RALT)) ||
				INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LALT)) )
			{
				// pressed Alt+F4
				SDL_Event *event;
				event = (SDL_Event *) malloc(sizeof(SDL_Event));
				event->type = SDL_QUIT;
				SDL_PushEvent(event);
				continue;
			}
			else
			{
				// pressed just F4
				PREFSMAN->m_bWindowed = !PREFSMAN->m_bWindowed;
				ApplyGraphicOptions();
				continue;
				/* why fall through? other code shouldn't be using 
				 * globally-bound keys -glenn */
				/* you're right.  -Chris */
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
}

static void GameLoop()
{
	RageTimer timer;
	while(1)
	{
		// process all queued events
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT:
				LOG->Trace("SDL_QUIT: shutting down");
				return; /* exit the main loop */
			case SDL_VIDEORESIZE:
				DISPLAY->ResolutionChanged(event.resize.w, event.resize.h);
				break;
			}
		}

		/*
		 * Update
		 */
		float fDeltaTime = timer.GetDeltaTime();
		
		if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_TAB) ) ) {
			if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_BACKQUOTE) ) )
				fDeltaTime = 0; /* both; stop time */
			else
				fDeltaTime *= 4;
		} else
			if( INPUTMAN->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_BACKQUOTE) ) )
				fDeltaTime /= 4;

		TEXTUREMAN->Update( fDeltaTime );
		SCREENMAN->Update( fDeltaTime );
		SOUNDMAN->Update( fDeltaTime );
		HandleInputEvents( fDeltaTime );

		/*
		 * Render
		 */
		DISPLAY->Clear();

		SCREENMAN->Draw();		// draw the game

		DISPLAY->Flip();

		::Sleep( 0 );	// give some time to other processes and threads
	}
}

