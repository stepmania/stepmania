/*
-----------------------------------------------------------------------------
 File: Player.h

 Desc: Object that accepts pad input, knocks down ColorArrows that were stepped on, 
		and keeps score for the player.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "PrefsManager.h"	// for ScoreSummary
#include "Pattern.h"
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
#include "ColorArrowField.h"
#include "GrayArrows.h"
#include "GhostArrows.h"




class Player : public ActorFrame
{
public:
	Player();

	void Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference );
	void RenderPrimitives();

	void Load( const StyleDef& StyleDef, PlayerNumber player_no, const Pattern& pattern, const PlayerOptions& po );
	void CrossedIndex( int iIndex );
	void HandlePlayerStep( float fSongBeat, TapNote step, float fMaxBeatDiff );
	int UpdateStepsMissedOlderThan( float fMissIfOlderThanThisBeat );

	ScoreSummary GetScoreSummary();

	float GetLifePercentage() { return m_LifeMeter.GetLifePercentage(); };
	bool IsThereANoteAtIndex( int iIndex );

protected:
	void CheckForCompleteStep( float fSongBeat, TapNote step, float fMaxBeatDiff );
	void OnCompleteStep( float fSongBeat, TapNote step, float fMaxBeatDiff, int iStepIndex );

	float			m_fSongBeat;
	StyleDef			m_Style;
	PlayerNumber	m_PlayerNumber;
	PlayerOptions	m_PlayerOptions;

	TapNote			m_TapNotesOriginal[MAX_TAP_NOTE_ELEMENTS];	// the original Pattern that were loaded into player
	TapNote			m_TapNotesRemaining[MAX_TAP_NOTE_ELEMENTS];	// mask off the bits as the player Pattern
	TapNoteScore	m_TapNoteScores[MAX_TAP_NOTE_ELEMENTS];
	HoldNote		m_HoldNotes[MAX_HOLD_NOTE_ELEMENTS];
	HoldNoteScore	m_HoldNoteScores[MAX_HOLD_NOTE_ELEMENTS];
	int				m_iNumHoldNotes;


	GrayArrows		m_GrayArrows;
	ColorArrowField	m_ColorArrowField;
	GhostArrows		m_GhostArrows;

	Judgement						m_Judgement;
	HoldJudgement					m_HoldJudgement[MAX_NUM_COLUMNS];
	Combo							m_Combo;
	LifeMeterPills					m_LifeMeter;
	ScoreDisplayRollingWithFrame	m_Score;
	
};




#endif