#pragma once
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayOni

 Desc: A graphic displayed in the ScoreDisplayOni during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Song.h"
#include "ScoreDisplay.h"



class ScoreDisplayOni : public ScoreDisplay
{
public:
	ScoreDisplayOni();

	virtual void Init( PlayerNumber pn, PlayerOptions po, int iOriginalNumNotes, int iNotesMeter );

	virtual void SetScore( float fNewScore );
	virtual int GetScore();
	virtual void AddToScore( TapNoteScore score, int iCurCombo );

	virtual void Update( float fDeltaTime );
	virtual void Draw();

protected:
	PlayerNumber m_PlayerNumber;
	PlayerOptions m_PlayerOptions;
	int m_iTotalNotes;
	int m_iNotesMeter;

	float m_fScore;

	float m_fTrailingScore;
	float m_fScoreVelocity;
};
