/*
-----------------------------------------------------------------------------
 Class: ScreenStage

 Desc: Shows the stage number.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Transition.h"
#include "Sprite.h"
#include "Character.h"
#include "BitmapText.h"
#include "Banner.h"

class ScreenStage : public Screen
{
public:
	ScreenStage( CString sName );

	virtual void HandleScreenMessage( const ScreenMessage SM );
	
	virtual void MenuBack( PlayerNumber pn );

private:
	Transition	m_In, m_Out, m_Back;
	BGAnimation				m_Background;
	BGAnimation				m_Overlay; // overlays all elements except bitmaptexts
	Banner			m_Banner;
	BitmapText		m_SongTitle;
	BitmapText		m_Artist;

	// elements that cannot be created with BGAnimation
	Sprite m_sprCharacterIcon[NUM_PLAYERS];
protected:

};


