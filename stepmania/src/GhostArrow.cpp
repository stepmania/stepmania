#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GhostArrow

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GhostArrow.h"
#include "NoteSkinManager.h"


GhostArrow::GhostArrow()
{
	SetDiffuse( RageColor(1,1,1,0) );
}

void GhostArrow::Init( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	for( int i=0; i<NUM_TAP_NOTE_SCORES; i++ )
	{
		CString sJudge = TapNoteScoreToString( (TapNoteScore)i );
		CString sCommand = Capitalize(sJudge)+"Command";
		m_sScoreCommand[i] = NOTESKIN->GetMetric(m_PlayerNumber,m_sName,sCommand);
	}
}

void GhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void GhostArrow::Step( TapNoteScore score )
{
	this->Command( m_sScoreCommand[score] );
}
