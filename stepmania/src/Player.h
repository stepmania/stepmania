/*
-----------------------------------------------------------------------------
 File: Player.h

 Desc: Object that accepts pad input, knocks down ColorArrows that were stepped on, 
		and keeps score for the player.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "PrefsManager.h"	// for ScoreSummary
#include "NoteMetadata.h"
#include "Sprite.h"
#include "BitmapText.h"

#include "ColorArrow.h"
#include "GrayArrow.h"
#include "GhostArrow.h"
#include "GhostArrowBright.h"
#include "HoldGhostArrow.h"
#include "ActorFrame.h"
#include "RandomSample.h"
#include "ScoreDisplayRollingWithFrame.h"
#include "LifeMeterPills.h"
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
	void RenderPrimitives();

	void Load( PlayerNumber player_no, NoteData* pNoteData, const PlayerOptions& po );
	void CrossedIndex( int iIndex );
	void HandlePlayerStep( float fSongBeat, NoteColumn col, float fMaxBeatDiff );
	int UpdateTapNotesMissedOlderThan( float fMissIfOlderThanThisBeat );

	ScoreSummary GetScoreSummary();

	float GetLifePercentage() { return m_LifeMeter.GetLifePercentage(); };
	bool IsThereANoteAtIndex( int iIndex );

protected:
	void CheckForCompleteRow( float fSongBeat, NoteColumn col, float fMaxBeatDiff );
	void OnRowDestroyed( float fSongBeat, NoteColumn col, float fMaxBeatDiff, int iStepIndex );

	float			m_fSongBeat;
	PlayerNumber	m_PlayerNumber;
	PlayerOptions	m_PlayerOptions;

	// maintain this extra data in addition to the NoteData
	TapNote			m_TapNotesOriginal[MAX_NOTE_TRACKS][MAX_TAP_NOTE_ELEMENTS];	// the original NoteMetadata that were loaded into player
	TapNoteScore	m_TapNoteScores[MAX_TAP_NOTE_ELEMENTS];
	HoldNoteScore	m_HoldNoteScores[MAX_HOLD_NOTE_ELEMENTS];


	GrayArrowRow		m_GrayArrowRow;
	NoteField		m_NoteField;
	GhostArrowRow		m_GhostArrowRow;

	Judgement						m_Judgement;
	HoldJudgement					m_HoldJudgement[MAX_NOTE_TRACKS];
	Combo							m_Combo;
	LifeMeterPills					m_LifeMeter;
	ScoreDisplayRollingWithFrame	m_Score;
	
};




#endif