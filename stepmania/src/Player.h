#pragma once
/*
-----------------------------------------------------------------------------
 Class: Player

 Desc: Object that accepts pad input, knocks down ColorNotes that were stepped on, 
		and keeps score for the player.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PrefsManager.h"	// for GameplayStatistics
#include "Notes.h"
#include "Sprite.h"
#include "BitmapText.h"

#include "ColorNote.h"
#include "GrayArrow.h"
#include "GhostArrow.h"
#include "GhostArrowBright.h"
#include "HoldGhostArrow.h"
#include "ActorFrame.h"
#include "RandomSample.h"
#include "ScoreDisplay.h"
#include "LifeMeterBar.h"
#include "Judgement.h"
#include "HoldJudgement.h"
#include "Combo.h"
#include "NoteField.h"
#include "GrayArrowRow.h"
#include "GhostArrowRow.h"
#include "NoteDataWithScoring.h"


class Player : public NoteDataWithScoring, public ActorFrame
{
public:
	Player();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Load( PlayerNumber player_no, NoteData* pNoteData, LifeMeter* pLM, ScoreDisplay* pScore );
	void CrossedRow( int iNoteRow );
	void Step( int col );


	void	FadeToFail()	{ m_NoteField.FadeToFail(); };
	
protected:
	int UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );
	void OnRowDestroyed( int col, int iStepIndex );
	void HandleNoteScore( TapNoteScore score );
	void HandleNoteScore( HoldNoteScore score );

	static float GetMaxBeatDifference();

	PlayerNumber	m_PlayerNumber;

	int				m_iNumTapNotes;	// num of TapNotes for the current notes needed by scoring
	int				m_iTapNotesHit;	// number of notes judged so far, needed by scoring
	int				m_iMeter;		// meter of current steps, needed by scoring

	GrayArrowRow		m_GrayArrowRow;
	NoteField			m_NoteField;
	GhostArrowRow		m_GhostArrowRow;

	HoldJudgement			m_HoldJudgement[MAX_NOTE_TRACKS];

	ActorFrame				m_frameJudgement;
	Judgement				m_Judgement;
	
	ActorFrame				m_frameCombo;
	Combo					m_Combo;

	LifeMeter*				m_pLifeMeter;
	ScoreDisplay*			m_pScore;
	
};
