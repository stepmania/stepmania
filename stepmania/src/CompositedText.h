#ifndef CompositedText_H
#define CompositedText_H
/*
-----------------------------------------------------------------------------
 Class: CompositedText

 Desc: A little graphic to the left of the song's text banner in the MusicWheel.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"

struct SDL_Surface;


SDL_Surface* CreateCompositedText( CString sFontFile, CString sText );


#endif
