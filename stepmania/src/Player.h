#pragma once
/*
-----------------------------------------------------------------------------
 File: Player.h

 Desc: Object that accepts pad input, knocks down ColorNotes that were stepped on, 
		and keeps score for the player.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PrefsManager.h"	// for ScoreSummary
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
#include "ScoreDisplayRolling.h"
#include "LifeMeterBar.h"
#include "Judgement.h"
#include "HoldJudgement.h"
#include "Combo.h"
#include "NoteField.h"
#include "GrayArrowRow.h"
#include "GhostArrowRow.h"




class Player : public NoteData, public ActorFrame
{
public:
	Player();

	void Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference );
	void DrawPrimitives();

	void Load( PlayerNumber player_no, NoteData* pNoteData, const PlayerOptions& po, LifeMeterBar* pLM, ScoreDisplayRolling* pScore );
	void CrossedIndex( int iIndex );
	void HandlePlayerStep( float fSongBeat, ColumnNumber col, float fMaxBeatDiff );
	int UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );

	ScoreSummary GetScoreSummary();

	bool IsThereANoteAtIndex( int iIndex );

protected:
	void CheckForCompleteRow( float fSongBeat, ColumnNumber col, float fMaxBeatDiff );
	void OnRowDestroyed( float fSongBeat, ColumnNumber col, float fMaxBeatDiff, int iStepIndex );

	float			m_fSongBeat;
	PlayerNumber	m_PlayerNumber;
	PlayerOptions	m_PlayerOptions;

	// maintain this extra data in addition to the NoteData
	TapNote			m_TapNotesOriginal[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ROWS];	// the original Notes that were loaded into player
	TapNoteScore	m_TapNoteScores[MAX_TAP_NOTE_ROWS];
	HoldNoteScore	m_HoldNoteScores[MAX_HOLD_NOTE_ELEMENTS];


	GrayArrowRow		m_GrayArrowRow;
	NoteField			m_NoteField;
	GhostArrowRow		m_GhostArrowRow;

	HoldJudgement			m_HoldJudgement[MAX_NOTE_TRACKS];
	ActorFrame				m_frameJudgeAndCombo;
	Judgement				m_Judgement;
	Combo					m_Combo;
	LifeMeterBar*			m_pLifeMeter;
	ScoreDisplayRolling*	m_pScore;
	
};
