#ifndef SCREENLOGO_H
#define SCREENLOGO_H
/*
-----------------------------------------------------------------------------
 Class: ScreenLogo

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"


class ScreenLogo : public ScreenAttract
{
public:
	ScreenLogo();

protected:
	Sprite				m_sprLogo;
	BitmapText			m_textVersion;
	BitmapText			m_textSongs;
};



#endif