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

protected:
	LifeMeterBar	m_LifeMeter;
	Player			m_Player;
	Model			m_mCharacter;

	Song*			m_pSong;
	float			m_fFakeSecondsIntoSong;
	float			m_fSecondsUntilNextKeyFrame;
	
	/*#define NUM_KEY_FRAMES 10
	float			m_fKeyFrames[NUM_KEY_FRAMES];
	float			m_fCurKeyFrame;*/
};



