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

#include "GameInfo.h"	// for ScoreSummary
#include "Steps.h"
#include "Sprite.h"
#include "BitmapText.h"

#include "ColorArrow.h"
#include "GrayArrow.h"
#include "GhostArrow.h"
#include "GhostArrowBright.h"
#include "HoldGhostArrow.h"
#include "Player.h"
#include "ActorFrame.h"
#include "SoundSet.h"
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

	void Load( const Style& style, PlayerNumber player_no, const Steps& steps, const PlayerOptions& po );
	void CrossedIndex( int iIndex );
	void HandlePlayerStep( float fSongBeat, TapStep step, float fMaxBeatDiff );
	int UpdateStepsMissedOlderThan( float fMissIfOlderThanThisBeat );

	ScoreSummary GetScoreSummary();

	float GetLifePercentage() { return m_LifeMeter.GetLifePercentage(); };
	bool IsThereANoteAtIndex( int iIndex );

protected:
	void CheckForCompleteStep( float fSongBeat, TapStep step, float fMaxBeatDiff );
	void OnCompleteStep( float fSongBeat, TapStep step, float fMaxBeatDiff, int iStepIndex );

	float			m_fSongBeat;
	Style			m_Style;
	PlayerNumber	m_PlayerNumber;
	PlayerOptions	m_PlayerOptions;

	TapStep			m_TapStepsOriginal[MAX_TAP_STEP_ELEMENTS];	// the original steps that were loaded into player
	TapStep			m_TapStepsRemaining[MAX_TAP_STEP_ELEMENTS];	// mask off the bits as the player steps
	TapStepScore	m_TapStepScores[MAX_TAP_STEP_ELEMENTS];
	HoldStep		m_HoldSteps[MAX_HOLD_STEP_ELEMENTS];
	HoldStepScore	m_HoldStepScores[MAX_HOLD_STEP_ELEMENTS];
	int				m_iNumHoldSteps;


	GrayArrows		m_GrayArrows;
	ColorArrowField	m_ColorArrowField;
	GhostArrows		m_GhostArrows;

	Judgement						m_Judgement;
	HoldJudgement					m_HoldJudgement[MAX_NUM_COLUMNS];
	Combo							m_Combo;
	LifeMeterPills					m_LifeMeter;
	ScoreDisplayRollingWithFrame	m_Score;
	
	SoundSet		m_soundAssistTick;

};




#endif