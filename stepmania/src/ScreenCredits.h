/*
-----------------------------------------------------------------------------
 File: ScreenCredits.h

 Desc: Music plays and song names scroll across the screen.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"
#include "Transition.h"
#include "ActorScroller.h"


class ScreenCredits : public ScreenAttract
{
public:
	ScreenCredits( CString sName );
	virtual ~ScreenCredits();

	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	ActorScroller			m_ScrollerBackgrounds;
	ActorScroller			m_ScrollerFrames;
	ActorScroller			m_ScrollerTexts;

	BGAnimation				m_Overlay;
};


