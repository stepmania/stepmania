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
#include "ScoreDisplayRolling.h"
#include "LifeMeterBar.h"
#include "Judgement.h"
#include "HoldJudgement.h"
#include "Combo.h"
#include "NoteField.h"
#include "GrayArrowRow.h"
#include "GhostArrowRow.h"
#include "NoteDataWithScoring.h"


struct GameplayStatistics;

class Player : public NoteDataWithScoring, public ActorFrame
{
public:
	Player();

	void Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference );
	void DrawPrimitives();

	void Load( PlayerNumber player_no, StyleDef *pStyleDef, NoteData* pNoteData, const PlayerOptions& po, LifeMeterBar* pLM, ScoreDisplayRolling* pScore, int iOriginalNumNotes );
	void CrossedRow( int iNoteRow, float fSongBeat, float fMaxBeatDiff );
	void HandlePlayerStep( float fSongBeat, int col, float fMaxBeatDiff );
	int UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );

	GameplayStatistics GetGameplayStatistics();


protected:
	void OnRowDestroyed( float fSongBeat, int col, float fMaxBeatDiff, int iStepIndex );

	float			m_fSongBeat;
	PlayerNumber	m_PlayerNumber;
	PlayerOptions	m_PlayerOptions;

	float			m_fHoldNoteLife[MAX_TAP_NOTE_ROWS];	// 1.0 means this HoldNote has full life.
														// 0.0 means this HoldNote is dead
														// When this value hits 0.0 for the first time, 
														// m_HoldScore becomes HSS_NG.
														// If the life is > 0.0 when the HoldNote ends, then
														// m_HoldScore becomes HSS_OK.


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
