#pragma once
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBar

 Desc: A graphic displayed in the LifeMeterBar during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"
#include "PlayerOptions.h"


class Song;


class LifeMeter : public ActorFrame
{
public:
	LifeMeter() { m_fSongBeat = 0; };
	virtual ~LifeMeter() {};
	
	virtual void Load( PlayerNumber p, const PlayerOptions &po ) { m_PlayerNumber = p; m_po = po; }
	virtual void SetBeat( float fSongBeat ) { m_fSongBeat = fSongBeat; };

	virtual void NextSong( Song* pSong ) {};
	virtual void ChangeLife( TapNoteScore score ) = 0;
	virtual bool IsInDanger() = 0;
	virtual bool IsHot() = 0;
	virtual bool IsFailing() = 0;
	virtual bool FailedEarlier() = 0;

protected:
	float m_fSongBeat;
	PlayerNumber	m_PlayerNumber;
	PlayerOptions	m_po;
};
