#include "global.h"

#include "SDL_utils.h"
#include "SDL_image.h"
#include "SDL_rotozoom.h"
#include "LoadingWindow_SDL.h"
#include "loading.xpm"
#include "StepMania.xpm" /* icon */

LoadingWindow_SDL::LoadingWindow_SDL()
{
    /* Initialize the SDL library */
    if( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
        RageException::Throw( "Couldn't initialize SDL: %s", SDL_GetError() );

	/* Set window title and icon */
	SDL_WM_SetCaption("Loading StepMania", "");

	SDL_Surface *srf = IMG_ReadXPMFromArray(icon);
	SDL_SetColorKey( srf, SDL_SRCCOLORKEY, SDL_MapRGB(srf->format, 0xFF, 0, 0xFF));

#if !defined(DARWIN) || !DARWIN
  /* Windows icons are 32x32 and SDL can't resize them for us, which
	 * causes mask corruption.  (Actually, the above icon *is* 32x32;
	 * this is here just in case it changes.) */
	ConvertSDLSurface(srf, srf->w, srf->h,
		32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	zoomSurface(srf, 32, 32);

	SDL_SetAlpha( srf, SDL_SRCALPHA, SDL_ALPHA_OPAQUE );
	SDL_WM_SetIcon(srf, NULL /* derive from alpha */);
	SDL_FreeSurface(srf);
#endif


	/* Load the BMP - we need it's dimensions */
    SDL_Surface *image = IMG_ReadXPMFromArray(loading);
    if( image == NULL )
        RageException::Throw("Couldn't load loading.bmp: %s",SDL_GetError());

    /* Initialize the window */
    loading_screen = SDL_SetVideoMode(image->w, image->h, 16, SDL_SWSURFACE|SDL_ANYFORMAT|SDL_NOFRAME);
    if( loading_screen == NULL )
        RageException::Throw( "Couldn't initialize loading window: %s", SDL_GetError() );

    SDL_BlitSurface(image, NULL, loading_screen, NULL);

    SDL_FreeSurface(image);
}

LoadingWindow_SDL::~LoadingWindow_SDL()
{
	SDL_QuitSubSystem( SDL_INIT_VIDEO );
}

void LoadingWindow_SDL::Paint()
{
	SDL_UpdateRect(loading_screen, 0,0,0,0);
}

/*
 * Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
 *
 * Chris Danford
 * Glenn Maynard
 */
