/*
-----------------------------------------------------------------------------
 Class: ScreenHowToPlay

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenAttract.h"
#include "Player.h"
#include "LifeMeterBar.h"
#include "Character.h"


class ScreenHowToPlay : public ScreenAttract
{
public:
	ScreenHowToPlay();
	~ScreenHowToPlay();
	
	virtual void Update( float fDelta );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void DrawPrimitives();

protected:
	LifeMeterBar	m_LifeMeterBar;
	BGAnimation		m_Overlay;
	Player			m_Player;
	Model			m_mCharacter;
	Model			m_mDancePad;

	Song*			m_pSong;
	float			m_fFakeSecondsIntoSong;
	float			m_fSecondsUntilNextKeyFrame;
};



