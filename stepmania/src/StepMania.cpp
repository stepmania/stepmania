#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: StepMania.cpp

 Desc: Entry point for program.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "StepMania.h"

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
#include "time.h"

#include "SDL.h"
#include "SDL_syswm.h"		// for SDL_SysWMinfo

//
// StepMania global classes
//
#include "ThemeManager.h"
#include "NoteSkinManager.h"
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

/* This is also a global class; we own it. */
ArchHooks *HOOKS = NULL;


#ifdef DEBUG
#pragma comment(lib, "SDL-1.2.5/lib/SDLmaind.lib")
#else
#pragma comment(lib, "SDL-1.2.5/lib/SDLmain.lib")
#endif
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.Lib")

#ifdef WIN32
HWND g_hWndMain = NULL;
#endif

static bool g_bHasFocus = true;


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
	bool bNeedReload = false;

	bNeedReload |= DISPLAY->SetVideoMode( 
		PREFSMAN->m_bWindowed, 
		PREFSMAN->m_iDisplayWidth, 
		PREFSMAN->m_iDisplayHeight, 
		PREFSMAN->m_iDisplayColorDepth, 
		PREFSMAN->m_iRefreshRate,
		PREFSMAN->m_bVsync );

	bNeedReload |= TEXTUREMAN->SetPrefs( 
		PREFSMAN->m_iTextureColorDepth, 
		PREFSMAN->m_iUnloadTextureDelaySeconds, 
		PREFSMAN->m_iMaxTextureResolution );

	if( bNeedReload )
		TEXTUREMAN->ReloadAll(); 
}

void ExitGame()
{
	SDL_Event *event;
	event = (SDL_Event *) malloc(sizeof(SDL_Event));
	event->type = SDL_QUIT;
	SDL_PushEvent(event);
}

void ResetGame()
{
	GAMESTATE->Reset();
	PREFSMAN->ReadGamePrefsFromDisk();
	INPUTMAPPER->ReadMappingsFromDisk();
	GAMESTATE->m_bPlayersCanJoin = true;

	if( !NOTESKIN->DoesNoteSkinExist( NOTESKIN->GetCurNoteSkinName() ) )
	{
		CStringArray asNoteSkinNames;
		NOTESKIN->GetNoteSkinNames( asNoteSkinNames );
		NOTESKIN->SwitchNoteSkin( asNoteSkinNames[0] );
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

	SCREENMAN->SetNewScreen( THEME->GetMetric("Common","InitialScreen") );
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

static bool ChangeAppPri()
{
	if(PREFSMAN->m_iBoostAppPriority == 0)
		return false;

	/* If -1 and this is a debug build, don't.  It makes the debugger sluggish. */
#ifdef DEBUG
	if(PREFSMAN->m_iBoostAppPriority == -1)
		return false;
#endif

	return true;
}

static void BoostAppPri()
{
	if(!ChangeAppPri())
		return;

#ifdef WIN32
	/* We just want a slight boost, so we don't skip needlessly if something happens
	 * in the background.  We don't really want to be high-priority--above normal should
	 * be enough.  However, ABOVE_NORMAL_PRIORITY_CLASS is only supported in Win2000
	 * and later. */
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize=sizeof(version);
	if(!GetVersionEx(&version))
	{
		LOG->Warn(werr_ssprintf(GetLastError(), "GetVersionEx failed"));
		return;
	}

#ifndef ABOVE_NORMAL_PRIORITY_CLASS
#define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
#endif

	DWORD pri = HIGH_PRIORITY_CLASS;
	if(version.dwMajorVersion >= 5)
		pri = ABOVE_NORMAL_PRIORITY_CLASS;

	/* Be sure to boost the app, not the thread, to make sure the
	 * sound thread stays higher priority than the main thread. */
	SetPriorityClass(GetCurrentProcess(), pri);
#endif
}

static void RestoreAppPri()
{
	if(!ChangeAppPri())
		return;

	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
}

int main(int argc, char* argv[])
{
	/* Set up arch hooks first.  This may set up crash handling. */
	HOOKS = MakeArchHooks();

	ChangeToDirOfExecutable(argv[0]);

	/* Set this up second.  Do this early, since it's needed for RageException::Throw. 
	 * Do it after ChangeToDirOfExecutable, so the log ends up in the right place. */
	LOG			= new RageLog();
#ifdef _DEBUG
	LOG->ShowConsole();
#endif

	/* Whew--we should be able to crash safely now! */

	atexit(SDL_Quit);   /* Clean up on exit */

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

	// changed to use time.  GetTimeSinceStart is silly because it always return 0! -Chris
	srand( time(NULL) );	// seed number generator	
	
	//
	// Create game objects
	//
	GAMESTATE	= new GameState;
	PREFSMAN	= new PrefsManager;
	GAMEMAN		= new GameManager;
	THEME		= new ThemeManager;
	NOTESKIN	= new NoteSkinManager;
	SOUNDMAN	= new RageSoundManager(PREFSMAN->m_sSoundDrivers);
	SOUNDMAN->SetPrefs(PREFSMAN->m_fSoundVolume);
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

	/* People may want to do something else while songs are loading, so do
	 * this after loading songs. */
	BoostAppPri();

	ResetGame();
	if( DISPLAY->IsSoftwareRenderer() )
		SCREENMAN->Prompt( 
			SM_None, 
			"OpenGL hardware acceleration\n"
			"was not detected.\n\n"
			"StepMania will use the Microsoft\n"
			"software OpenGL renderer.\n"
			"However, the game is not playable\n"
			"with this software renderer.\n"
			"Please install the latest video\n"
			"driver from your graphics card vendor\n"
			"to enable OpenGL hardware acceleration."
			);

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
	SAFE_DELETE( NOTESKIN );
	SAFE_DELETE( THEME );
	SAFE_DELETE( ANNOUNCER );
	SAFE_DELETE( INPUTMAN );
	SAFE_DELETE( SOUNDMAN );
	SAFE_DELETE( FONT );
	SAFE_DELETE( TEXTUREMAN );
	SAFE_DELETE( DISPLAY );
	SAFE_DELETE( LOG );
	SAFE_DELETE( HOOKS );
	
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

		// HACK:  Numlock is read is being pressed if the NumLock light is on.
		// Filter out all NumLock repeat messages
		if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_NUMLOCK && type != IET_FIRST_PRESS )
			continue;	// skip

		
						
		/* Global operator key.. like arcade, allows quick immediate access
		   to the adminstrative options panel. A global boolean has been
		   added to not allow this to function on system option screens,
		   or in the step editor. This will save the hassle of an "accidental
		   keystroke, and your edit is gone".  -- Miryokuteki
		*/
		if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_LCTRL  && type == IET_FIRST_PRESS )
		{
			if( !GAMESTATE->m_bIsOnSystemMenu )
			{
			SCREENMAN->SystemMessage("OPERATOR");
			SCREENMAN->SetNewScreen("ScreenOptionsMenu");
			}
		}
		
		// Global credit key.. accepts a credit anywhere, like the arcade -- Mirykouteki
		if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F1 && type == IET_FIRST_PRESS )
		{
			switch( PREFSMAN->m_CoinMode )
			{
				case PrefsManager::COIN_FREE:
				case PrefsManager::COIN_HOME:
				case PrefsManager::COIN_PAY:
					GAMESTATE->m_iCoins++;
					SCREENMAN->RefreshCreditsMessages();
					SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","insert coin") );
					break;
				default:
					ASSERT(0);
			}
		}



		if(DeviceI == DeviceInput(DEVICE_KEYBOARD, SDLK_F4))
		{
			if(type != IET_FIRST_PRESS) continue;
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
			}
		}
		else if(DeviceI == DeviceInput(DEVICE_KEYBOARD, SDLK_F5))	// F5 conflicts with editor and AutoSync
		{
			if(type != IET_FIRST_PRESS) continue;

			// pressed F6.  Save Screenshot.
			int i=0;
			CString sPath;
			for( i=0; i<1000; i++ )
			{
				sPath = ssprintf("screen%04d.bmp",i);
				if( !DoesFileExist(sPath) )
					break;
			}
			DISPLAY->SaveScreenshot( sPath );
			SCREENMAN->SystemMessage( "Saved screenshot: " + sPath );
		}

		if(DeviceI == DeviceInput(DEVICE_KEYBOARD, SDLK_RETURN))
		{
			if( INPUTMAN->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_RALT)) ||
				INPUTMAN->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_LALT)) )
			{
				if(type != IET_FIRST_PRESS) continue;
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
			case SDL_ACTIVEEVENT:
			{
				/* We don't care about mouse focus. */
				if(event.active.state == SDL_APPMOUSEFOCUS)
					break;

				Uint8 i = SDL_GetAppState();
				
				g_bHasFocus = i&SDL_APPINPUTFOCUS && i&SDL_APPACTIVE;
				LOG->Trace("App %s focus (%i%i)", g_bHasFocus? "has":"doesn't have",
					i&SDL_APPINPUTFOCUS, i&SDL_APPACTIVE);

				if(g_bHasFocus)
					BoostAppPri();
				else
					RestoreAppPri();
			}
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

		/* Important:  Process input before updaing game logic, or else game logic will lag for one frame */
		HandleInputEvents( fDeltaTime );

		SCREENMAN->Update( fDeltaTime );
		SOUNDMAN->Update( fDeltaTime );

		/*
		 * Render
		 */
		DISPLAY->Clear();

		SCREENMAN->Draw();		// draw the game

		DISPLAY->Flip();

		if(g_bHasFocus)
			::Sleep( 0 );	// give some time to other processes and threads
		else
			::Sleep( 2 );	// give more time to other processes and threads, but not so much that we skip sound
	}
}

