#include "global.h"

#include "SDL.h"
#include "SDL_utils.h"
#include "SDL_rotozoom.h"	// for setting icon
#include "RageSurface_Load.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageThreads.h"
#include "StepMania.h"

#include <errno.h>

/* Pull in all of our SDL libraries here. */
#ifdef _XBOX
	#ifdef DEBUG
	#pragma comment(lib, "SDLx-0.02/SDLxd.lib")
	#else
	#pragma comment(lib, "SDLx-0.02/SDLx.lib")
	#endif
#elif defined _WINDOWS
	#ifdef DEBUG
	#pragma comment(lib, "SDL-1.2.6/lib/SDLd.lib")
	#else
	#pragma comment(lib, "SDL-1.2.6/lib/SDL.lib")
	#endif
#endif



bool SDL_GetEvent( SDL_Event &event, int mask )
{
	/* SDL_PeepEvents returns error if video isn't initialized. */
	if(!SDL_WasInit(SDL_INIT_VIDEO))
		return false;

	switch(SDL_PeepEvents(&event, 1, SDL_GETEVENT, mask))
	{
	case 1: return true;
	case 0: return false;
	default: RageException::Throw("SDL_PeepEvents returned unexpected error: %s", SDL_GetError());
	}
}

/* Reads all currently queued SDL events, clearing them from the queue. */
void mySDL_GetAllEvents( vector<SDL_Event> &events )
{
	while(1)
	{
		SDL_Event ev;
		if(SDL_PollEvent(&ev) <= 0)
			break;

		events.push_back(ev);
	}
}

/* Pushes the given events onto the SDL event queue. */
void mySDL_PushEvents( vector<SDL_Event> &events )
{
	for(unsigned i = 0; i < events.size(); ++i)
		SDL_PushEvent(&events[i]);
}

/* For some bizarre reason, SDL_EventState flushes all events.  This is a pain, so
 * avoid it. */
uint8_t mySDL_EventState( uint8_t type, int state )
{
	if(state == SDL_QUERY)
		return SDL_EventState(type, state);

	vector<SDL_Event> events;
	mySDL_GetAllEvents(events);

	/* Set the event mask. */
	uint8_t ret = SDL_EventState(type, state);

	/* Don't readding events that we just turned off; they'll just sit around
	 * in the buffer. */
	for(unsigned i = 0; i < events.size(); )
	{
		if(state == SDL_IGNORE && events[i].type == type)
			events.erase(events.begin()+i);
		else
			i++;
	}

	/* Put them back. */
	mySDL_PushEvents(events);

	return ret;
}


/* SDL sometimes fails to set an error, in which case we get the null string.  We
 * sometimes use that as a sentinel return value.  This function returns "(none)"
 * if no error is set. */
CString mySDL_GetError()
{
	CString error = SDL_GetError();
	if( error == "" )
		return "(none)"; /* SDL sometimes fails to set an error */
	return error;
}


void mySDL_WM_SetIcon( CString sIconFile )
{
#if !defined(DARWIN)
	if( sIconFile.empty() )
	{
		SDL_WM_SetIcon(NULL, NULL);
		return;
	}

	CString error;
	RageSurface *srf = RageSurfaceUtils::LoadFile( sIconFile, error );
	if( srf == NULL )
		return;

	/* Windows icons are 32x32 and SDL can't resize them for us, which
	 * causes mask corruption.  (Actually, the above icon *is* 32x32;
	 * this is here just in case it changes.) */
	RageSurfaceUtils::Zoom( srf, 32, 32 );

	SDL_Surface *sdl_srf = SDLSurfaceFromRageSurface( srf );
	delete srf;

	SDL_SetAlpha( sdl_srf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE );
	SDL_WM_SetIcon( sdl_srf, NULL /* derive from alpha */ );
	SDL_FreeSurface( sdl_srf );
#endif
}

/* Create an SDL surface from a RageSurface. */
SDL_Surface *SDLSurfaceFromRageSurface( RageSurface *surf )
{
	/* SDL will free the data with free(), so we can't just give it surf->pixels. */
	char *buf = (char *) malloc( surf->pitch * surf->h );
	if( buf == NULL )
		RageException::Throw( "malloc(%i): %s", surf->pitch * surf->h, strerror(errno) );
	memcpy( buf, surf->pixels, surf->pitch * surf->h );
	SDL_Surface *ret = SDL_CreateRGBSurfaceFrom( buf,
		surf->w, surf->h, surf->fmt.BitsPerPixel,
		surf->pitch, surf->fmt.Rmask, surf->fmt.Gmask, surf->fmt.Bmask, surf->fmt.Amask );

	/* Copy the palette. */
	if( surf->format->BytesPerPixel == 1 )
	{
		ASSERT( sizeof(RageSurfaceColor) == sizeof(SDL_Color) );
		SDL_SetPalette( ret, SDL_LOGPAL, (SDL_Color *) surf->fmt.palette->colors, 0, 256 );

		/* If we have one alpha value, transfer it to the color key. */
		int iKey = -1;
		bool bUsesColorKey = true;

		for( int i = 0; i < surf->format->palette->ncolors; ++i )
		{
			if( surf->format->palette->colors[i].a == 0xFF )
				continue;
			if( surf->format->palette->colors[i].a != 0 )
			{
				bUsesColorKey = false;
				break;
			}

			/* a == 0 */
			if( iKey != -1 )
			{
				bUsesColorKey = false;
				break;
			}
			iKey = i;
		}

		if( bUsesColorKey && iKey != -1 )
			SDL_SetColorKey( ret, SDL_SRCCOLORKEY, iKey );

		/* XXX: If we have more than one alpha value, or any alpha values between 1 and
		 * 254, it needs to be converted to RGBA.  I'm not sure that we ever do that
		 * with surfaces passed to SDL, though. */
	}

	return ret;
}

RageSurface *RageSurfaceFromSDLSurface( SDL_Surface *surf )
{
	uint8_t *buf = new uint8_t[surf->pitch * surf->h];
	memcpy( buf, surf->pixels, surf->pitch * surf->h );
	RageSurface *ret = CreateSurfaceFrom( 
		surf->w, surf->h, surf->format->BitsPerPixel,
		surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask,
		buf, surf->pitch );

	/* Copy the palette. */
	if( surf->format->BytesPerPixel == 1 )
	{
		ASSERT( sizeof(RageSurfaceColor) == sizeof(SDL_Color) );
		memcpy( ret->fmt.palette->colors, surf->format->palette->colors,
				256 * sizeof(RageSurfaceColor) );
	}

	/* Convert SDL "colorkey" to an alpha palette entry. 's "unused" palette entry is undefined.  We use it for alpha, so set it to 255. */
	if( surf->format->BitsPerPixel == 8 )
	{
		for( int index = 0; index < ret->fmt.palette->ncolors; ++index )
			ret->fmt.palette->colors[index].a = 0xFF;

		/* If the surface had a color key set, transfer it. */
		if( surf->flags & SDL_SRCCOLORKEY )
		{
			ASSERT_M( (int)surf->format->colorkey < surf->format->palette->ncolors, ssprintf("%i",surf->format->colorkey) );
			int ckey = surf->format->colorkey;
			ret->fmt.palette->colors[ ckey ].a = 0;
		}
	}

	return ret;
}

void SDL_UpdateHWnd()
{
#ifdef _WINDOWS
	/* Grab the window manager specific information */
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if ( SDL_GetWMInfo(&info) < 0 ) 
		RageException::Throw( "SDL_GetWMInfo failed" );

	g_hWndMain = info.window;
#endif
}

#if defined(UNIX)
#include "archutils/Unix/EmergencyShutdown.h"
void EmergencyShutdownSDL()
{
	/* Only do this if the main thread crashes; trying to shut down from
	 * another thread causes crashes (eg. GL may be using TLS). */
	if( !strcmp(RageThread::GetCurThreadName(), "Main thread") && SDL_WasInit(SDL_INIT_VIDEO) )
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
#endif

void SetupSDL()
{
	static bool bDone = false;
	if( bDone )
		return;
	bDone = true;

	/* Start SDL (with no subsystems), to make sure we don't use SDL's error handler. */
	SDL_Init( SDL_INIT_NOPARACHUTE );

	/* Clean up on exit. */
	atexit( SDL_Quit );

#if defined(UNIX)
	/* Attempt to shut down the window on crash. */
        RegisterEmergencyShutdownCallback( EmergencyShutdownSDL );
#endif

#if defined(LINUX)
	static bool bSetVideoDriver = false;
	if( !bSetVideoDriver )
	{
		bSetVideoDriver = true;

		/* Most people don't have this set.  SDL has a habit of trying to
		 * fall back on other drivers (svgalib, aalib), so set it to "x11". */
		const char *sVideoDriver = getenv("SDL_VIDEODRIVER");
		if( sVideoDriver == NULL || sVideoDriver[0] == 0 )
		{
			static char env[] = "SDL_VIDEODRIVER=x11";
			putenv( env );
		} else if( strcmp( sVideoDriver, "x11" ) )
			LOG->Info( "SDL_VIDEODRIVER has been set to %s", sVideoDriver );
	}
#endif
}

/*
 * (c) 2002-2004 Glenn Maynard
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
