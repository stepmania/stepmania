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
	virtual void Step();
	LifeMeterBar*	m_pLifeMeterBar;
	BGAnimation		m_Overlay;
	Player*			m_pPlayer;
	Model*			m_pmCharacter;
	Model*			m_pmDancePad;
	int				m_iPerfects;
	int				m_iNumPerfects;

	Song			m_Song;
	NoteData		m_NoteData;
	float			m_fFakeSecondsIntoSong;
};



