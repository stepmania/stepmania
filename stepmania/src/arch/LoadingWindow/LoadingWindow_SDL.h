#ifndef LOADING_WINDOW_SDL_H
#define LOADING_WINDOW_SDL_H

#include "LoadingWindow.h"
#include "SDL.h"

#ifdef _XBOX
#define XFONT_TRUETYPE 
#include <xfont.h>
#endif

class LoadingWindow_SDL: public LoadingWindow {
	SDL_Surface *loading_screen;

public:
	LoadingWindow_SDL();
	~LoadingWindow_SDL();

	void Paint();

#ifdef _XBOX

	void SetText(CString str);

	CString m_cstrText ;
	XFONT*      m_pConsoleTTF;    // Pointer to the Arial TrueTypeFont
#endif 
};

#define HAVE_LOADING_WINDOW_SDL

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
