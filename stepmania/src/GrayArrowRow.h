#pragma once
/*
-----------------------------------------------------------------------------
 Class: GrayArrowRow

 Desc: A row of GrayArrow objects

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrayArrow.h"
#include "ActorFrame.h"
#include "StyleDef.h"
#include "GameConstantsAndTypes.h"


class GrayArrowRow : public ActorFrame
{
public:
	GrayArrowRow();
	virtual void Update( float fDeltaTime, float fSongBeat );
	virtual void DrawPrimitives();

	void Load( PlayerNumber pn, StyleDef* pStyleDef, PlayerOptions po );

	void Step( int iCol );
	
protected:
	int m_iNumCols;
	float m_fSongBeat;
	PlayerOptions m_PlayerOptions;

	GrayArrow	m_GrayArrow[MAX_NOTE_TRACKS];
};
