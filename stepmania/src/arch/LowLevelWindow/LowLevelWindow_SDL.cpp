#include "global.h"
#include "LowLevelWindow_SDL.h"
#include "SDL_utils.h"
#include "RageLog.h"
#include "RageDisplay.h" // for REFRESH_DEFAULT
#include "arch/ArchHooks/ArchHooks.h"
#include "DisplayResolutions.h"

LowLevelWindow_SDL::LowLevelWindow_SDL()
{
	/* By default, ignore all SDL events.  We'll enable them as we need them.
	 * We must not enable any events we don't actually want, since we won't
	 * query for them and they'll fill up the event queue. 
	 *
	 * This needs to be done after we initialize video, since it's really part
	 * of the SDL video system--it'll be reinitialized on us if we do this first. */
	SDL_EventState(0xFF /*SDL_ALLEVENTS*/, SDL_IGNORE);

	SDL_EventState( SDL_VIDEORESIZE, SDL_ENABLE );
	SDL_EventState( SDL_ACTIVEEVENT, SDL_ENABLE );
	SDL_EventState( SDL_QUIT, SDL_ENABLE );
}

LowLevelWindow_SDL::~LowLevelWindow_SDL()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	mySDL_EventState( SDL_VIDEORESIZE, SDL_IGNORE );
	mySDL_EventState( SDL_ACTIVEEVENT, SDL_IGNORE );
	mySDL_EventState( SDL_QUIT, SDL_IGNORE );
}

void *LowLevelWindow_SDL::GetProcAddress(RString s)
{
	return SDL_GL_GetProcAddress(s);
}

RString LowLevelWindow_SDL::TryVideoMode( const VideoModeParams &p, bool &bNewDeviceOut )
{
	bool wasWindowed = CurrentParams.windowed;
	
	CurrentParams = p;

	/* We need to preserve the event mask and all events, since they're lost by
	 * SDL_QuitSubSystem(SDL_INIT_VIDEO). */
	vector<SDL_Event> events;
	mySDL_GetAllEvents(events);
	Uint8 EventEnabled[SDL_NUMEVENTS];

	/* Queue up key-up events for all keys that are currently down (eg. alt-enter).
	 * This is normally done by SDL, but since we're shutting down the video system
	 * we're also shutting down the event system. */
	{
		const Uint8 *KeyState = SDL_GetKeyState(NULL);
		for ( SDLKey key=SDLK_FIRST; key<SDLK_LAST; key = (SDLKey)(key+1) )
		{
			if ( KeyState[key] != SDL_PRESSED )
				continue;
			SDL_Event e;
			memset(&e, 0, sizeof(e));
			e.key.type = SDL_KEYUP;
			e.key.keysym.sym = key;
			events.push_back(e);
		}
	}

	for( int i = 0; i < SDL_NUMEVENTS; ++i )
		EventEnabled[i] = mySDL_EventState( (Uint8) i, SDL_QUERY );

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	if( SDL_InitSubSystem(SDL_INIT_VIDEO) == -1 )
	{
		const RString err = mySDL_GetError();

		/* Check for a confusing SDL error message. */
		if( !err.CompareNoCase( "X11 driver not configured with OpenGL" ) )
			RageException::Throw( "Your installation of SDL was not compiled with OpenGL enabled." );
		RageException::Throw( "SDL_INIT_VIDEO failed: %s", err.c_str() );
	}
	
	/* Put them back. */
	for( int i = 0; i < SDL_NUMEVENTS; ++i)
		mySDL_EventState((Uint8) i, EventEnabled[i]);
	mySDL_PushEvents(events);

	/* Set SDL window title, icon and cursor -before- creating the window */
	SDL_WM_SetCaption( p.sWindowTitle, "");
	mySDL_WM_SetIcon( p.sIconFile );
	SDL_ShowCursor( p.windowed ? SDL_ENABLE : SDL_DISABLE );
	
	int flags = SDL_RESIZABLE | SDL_OPENGL;
	if( !p.windowed )
		flags |= SDL_FULLSCREEN;

	ASSERT( p.bpp == 16 || p.bpp == 32 );
	switch( p.bpp )
	{
	case 16:
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		break;
	case 32:
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	}

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

#if defined(LINUX)
	/* nVidia cards:
	 *
	 * This only works the first time we set up a window; after that, the
	 * drivers appear to cache the value, so you have to actually restart
	 * the program to change it again. */
	static char buf[128];
	strcpy( buf, "__GL_SYNC_TO_VBLANK=" );
	strcat( buf, p.vsync?"1":"0" );
	putenv( buf );
#endif

	SDL_Surface *screen = SDL_SetVideoMode( p.width, p.height, p.bpp, flags );
	if( !screen )
	{
		LOG->Trace( "SDL_SetVideoMode failed: %s", mySDL_GetError().c_str() );
		SDL_ShowCursor( wasWindowed ? SDL_ENABLE : SDL_DISABLE );
		return mySDL_GetError();	// failed to set mode
	}
	
	bNewDeviceOut = true;	// always a new context because we're resetting SDL_Video

	static bool bLogged = false;
	if( !bLogged )
	{
		bLogged = true;
		const SDL_version *ver = SDL_Linked_Version();
		LOG->Info( "SDL version: %i.%i.%i", ver->major, ver->minor, ver->patch );
	}

	{
		/* Find out what we really got. */
		int r,g,b,a, colorbits, depth, stencil;
		
		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &a);
		SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &colorbits);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
		SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencil);
		LOG->Info("Got %i bpp (%i%i%i%i), %i depth, %i stencil",
			colorbits, r, g, b, a, depth, stencil);
	}

	return "";	// we set the video mode successfully
}

void LowLevelWindow_SDL::SwapBuffers()
{
	SDL_GL_SwapBuffers();
}

void LowLevelWindow_SDL::Update()
{
	/* This needs to be called before anything that handles SDL events. */
	SDL_PumpEvents();

	SDL_Event event;
	while(SDL_GetEvent(event, SDL_VIDEORESIZEMASK|SDL_ACTIVEEVENTMASK|SDL_QUITMASK))
	{
		switch(event.type)
		{
		case SDL_VIDEORESIZE:
			CurrentParams.width = event.resize.w;
			CurrentParams.height = event.resize.h;

			/* Let DISPLAY know that our resolution has changed. */
			DISPLAY->ResolutionChanged();
			break;
		case SDL_ACTIVEEVENT:
			/* We don't care about mouse focus. */
			if(event.active.state == SDL_APPMOUSEFOCUS)
				break;

			if( event.active.gain  &&		// app regaining focus
				!DISPLAY->GetActualVideoModeParams().windowed )	// full screen
			{
				// need to reacquire an OGL context
				/* This hasn't been done in a long time, since HandleSDLEvents was
				 * handling this event before we got it.  I tried reenablng it, and
				 * it resulted in input not being regained and textures being erased,
				 * so I left it disabled; but this might be needed on some cards (with
				 * the above fixed) ... */
				// DISPLAY->SetVideoMode( DISPLAY->GetActualVideoModeParams() );
			}

			{
				uint8_t i = SDL_GetAppState();
				LOG->Trace( "SDL_GetAppState: %i", i );
				HOOKS->SetHasFocus( i&SDL_APPINPUTFOCUS && i&SDL_APPACTIVE );
			}
			break;
		case SDL_QUIT:
			LOG->Trace("SDL_QUIT: shutting down");
			ArchHooks::SetUserQuit();
			break;
		}
	}
}

void LowLevelWindow_SDL::GetDisplayResolutions( DisplayResolutions &out ) const
{
	SDL_Rect **modes = SDL_ListModes(NULL, SDL_RESIZABLE | SDL_OPENGL | SDL_FULLSCREEN );
	ASSERT_M( modes, "No modes available" );
	
	for(int i=0; modes[i]; ++i )
	{
		DisplayResolution res = { modes[i]->w, modes[i]->h };
		out.s.insert( res );
	}
}

/*
 * (c) 2003-2004 Chris Danford, Glenn Maynard
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
