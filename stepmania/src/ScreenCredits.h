/*
-----------------------------------------------------------------------------
 File: ScreenCredits.h

 Desc: Music plays and song names scroll across the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "TransitionBGAnimation.h"
#include "ActorScroller.h"


class ScreenCredits : public Screen
{
public:
	ScreenCredits();
	~ScreenCredits();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );

private:

	BGAnimation				m_Background;

	ActorScroller			m_ScrollerBackgrounds;
	vector<Actor*>			m_vBackgrounds;

	ActorScroller			m_ScrollerFrames;
	vector<Actor*>			m_vFrames;

	ActorScroller			m_ScrollerTexts;
	vector<Actor*>			m_vTexts;

	BGAnimation				m_Overlay;

	TransitionBGAnimation	m_In;
	TransitionBGAnimation	m_Out;
};


