#include "../../stdafx.h"

#include "SDL_Image.h"
#include "LoadingWindow_SDL.h"

#include "loading.xpm"

LoadingWindow_SDL::LoadingWindow_SDL()
{
    /* Initialize the SDL library */
    if( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
        RageException::Throw( "Couldn't initialize SDL: %s\n", SDL_GetError() );

	/* Load the BMP file into a surface */
    SDL_Surface *image = IMG_ReadXPMFromArray(loading);
    if( image == NULL )
        RageException::Throw("Couldn't load loading.bmp: %s\n",SDL_GetError());

    /* Initialize the display in a 640x480 16-bit mode */
    loading_screen = SDL_SetVideoMode(image->w, image->h, 16, SDL_SWSURFACE|SDL_ANYFORMAT|SDL_NOFRAME);
    if( loading_screen == NULL )
        RageException::Throw( "Couldn't initialize loading window: %s\n", SDL_GetError() );

    /* Blit onto the screen surface */
    SDL_BlitSurface(image, NULL, loading_screen, NULL);

    /* Free the allocated BMP surface */
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
