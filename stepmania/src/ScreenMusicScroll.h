/*
-----------------------------------------------------------------------------
 File: ScreenMusicScroll.h

 Desc: Music plays and song names scroll across the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "Transition.h"
#include "MenuElements.h"


//const unsigned MAX_MUSIC_LINES = 1200;
//const unsigned MAX_CREDIT_LINES = 100;
//const unsigned MAX_TOTAL_LINES = MAX_MUSIC_LINES;// + MAX_CREDIT_LINES;


class ScreenMusicScroll : public Screen
{
public:
	ScreenMusicScroll();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );

private:

	BGAnimation		m_Background;
	vector<BitmapText *> m_textLines;
	float			m_fTimeLeftInScreen;

	Transition	m_In;
	Transition	m_Out;
};


