#pragma once
/*
-----------------------------------------------------------------------------
 File: GhostArrowRow.h

 Desc: A row of GhostArrow Actors

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "GhostArrow.h"
#include "GhostArrowBright.h"
#include "HoldGhostArrow.h"
#include "GameConstantsAndTypes.h"
#include "StyleDef.h"



class GhostArrowRow : public ActorFrame
{
public:
	GhostArrowRow();
	void Update( float fDeltaTime, float fSongBeat );
	virtual void DrawPrimitives();

	void Load( PlayerOptions po );
	
	void TapNote( int iCol, TapNoteScore score, bool bBright );
	void HoldNote( int iCol );
	
protected:
	int m_iNumCols;
	float m_fSongBeat;
	PlayerOptions m_PlayerOptions;

	GhostArrow			m_GhostArrowRow[MAX_NOTE_TRACKS];
	GhostArrowBright	m_GhostArrowRowBright[MAX_NOTE_TRACKS];
	HoldGhostArrow		m_HoldGhostArrowRow[MAX_NOTE_TRACKS];
};

