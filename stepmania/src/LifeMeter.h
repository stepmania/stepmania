#pragma once
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBar

 Desc: A graphic displayed in the LifeMeterBar during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "GameConstantsAndTypes.h"
#include "NoteData.h"


class LifeMeter : public ActorFrame
{
public:
	LifeMeter() { m_fSongBeat = 0; };
	virtual ~LifeMeter() {};
	
	void SetPlayerOptions(const PlayerOptions &po) { m_po = po; }
	void SetBeat( float fSongBeat ) { m_fSongBeat = fSongBeat; };

	virtual void ChangeLife( TapNoteScore score ) = 0;
	virtual bool IsAboutToFail() = 0;
	virtual bool HasFailed() = 0;

protected:
	float m_fSongBeat;
	PlayerOptions	m_po;
};
