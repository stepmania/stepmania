#pragma once
/*
-----------------------------------------------------------------------------
 Class: ScrollBar

 Desc: A graphic displayed in the ScrollBar during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"


class ScrollBar : public ActorFrame
{
public:
	ScrollBar();
	
	void SetBarHeight( int iHeight );
	void SetPercentage( float fStartPercent, float fEndPercent);

protected:

	int		m_iBarHeight;

	Sprite	m_sprTopButton;
	Sprite	m_sprBottomButton;
	Sprite	m_sprBackground;
	Sprite	m_sprScrollThumbPart1;
	Sprite	m_sprScrollThumbPart2;
};
