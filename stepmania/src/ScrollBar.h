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
	
	void SetPercentage( float fPercentageFromTop );

private:
	Sprite m_sprTopButton, m_sprBottomButton;
	Sprite m_sprBackground, m_sprScrollThumb;
};
