#ifndef LOADING_WINDOW_SDL_H
#define LOADING_WINDOW_SDL_H

#include "LoadingWindow.h"
#include "SDL.h"

class LoadingWindow_SDL: public LoadingWindow {
	SDL_Surface *loading_screen;

public:
	LoadingWindow_SDL();
	~LoadingWindow_SDL();

	void Paint();
};

#define HAVE_LOADING_WINDOW_NULL

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
