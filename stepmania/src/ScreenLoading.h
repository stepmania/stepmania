#ifndef SCREENLOADING_H
#define SCREENLOADING_H
/*
-----------------------------------------------------------------------------
 Class: ScreenLoading

 Desc: Shows while game loads.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"
#include "Sprite.h"

class ScreenLoading : public Screen
{
public:
	ScreenLoading();

	virtual void DrawPrimitives();

protected:
	BitmapText m_textMessage;
	Sprite m_sprLoading;

};


#endif
