#ifndef OPTIONSCURSOR_H
#define OPTIONSCURSOR_H
/*
-----------------------------------------------------------------------------
 Class: OptionsCursor

 Desc: A graphic displayed in the OptionsCursor during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "ActorFrame.h"
#include "GameConstantsAndTypes.h"


class OptionsCursor : public ActorFrame
{
public:
	OptionsCursor();

	void Load( PlayerNumber pn, bool bUnderline );
	void SetBarWidth( int iWidth );
	void TweenBarWidth( int iNewWidth );

protected:
	void TweenBarWidth( int iNewWidth, float fTweenTime );

	Sprite m_sprLeft;
	Sprite m_sprMiddle;
	Sprite m_sprRight;
};

#endif
