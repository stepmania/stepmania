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


class ScreenHowToPlay : public ScreenAttract
{
public:
	ScreenHowToPlay();
	~ScreenHowToPlay();
	
	virtual void Update( float fDelta );

protected:
	LifeMeterBar	m_LifeMeter;
	Player			m_Player;

	Song*			m_pSong;
	float			m_fFakeSecondsIntoSong;
};



