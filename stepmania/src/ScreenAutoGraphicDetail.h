/*
-----------------------------------------------------------------------------
 Class: ScreenAutoGraphicDetail

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Player.h"

class ScreenAutoGraphicDetail : public Screen
{
public:
	ScreenAutoGraphicDetail();
	~ScreenAutoGraphicDetail();
	
	virtual void Update( float fDelta );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	BGAnimation		m_Background;
	Player			m_Player[NUM_PLAYERS];
	BGAnimation		m_Overlay;
	BitmapText		m_textMessage;

	Song*			m_pSong;
	float			m_fFakeSecondsIntoSong;
};



