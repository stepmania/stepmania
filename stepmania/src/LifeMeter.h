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
	LifeMeter() {};
	virtual ~LifeMeter() {};
	
	virtual void Load( PlayerNumber p ) { m_PlayerNumber = p; }
	virtual void Update( float fDeltaTime ) { ActorFrame::Update(fDeltaTime); };

	virtual void SongEnded() {};
	virtual void ChangeLife( TapNoteScore score ) = 0;
	virtual void OnDancePointsChange() = 0;	// look in GAMESTATE and update the display
	virtual bool IsInDanger() = 0;
	virtual bool IsHot() = 0;
	virtual bool IsFailing() = 0;
	virtual bool FailedEarlier() = 0;

protected:
	PlayerNumber	m_PlayerNumber;
};
