#pragma once
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayRolling

 Desc: A graphic displayed in the ScoreDisplayRolling during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "Song.h"
#include "ActorFrame.h"
#include "BitmapText.h"


const int NUM_SCORE_DIGITS	=	9;


class ScoreDisplayRolling : public BitmapText
{
public:
	ScoreDisplayRolling();

	void Init( PlayerNumber pn, PlayerOptions po, int iTotalTapNotes, int iNotesMeter );

	void SetScore( int iNewScore );
	int GetScore();
	void AddToScore( TapNoteScore score, int iCurCombo );

	virtual void Update( float fDeltaTime );
	virtual void Draw();

protected:
	PlayerNumber m_PlayerNumber;
	PlayerOptions m_PlayerOptions;
	int m_iTotalTapNotes;
	int m_iNotesMeter;

	int m_iScore;

	float m_fTrailingScore;
	float m_fScoreVelocity;
};
