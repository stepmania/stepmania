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
#include "ScoreDisplayRolling.h"




const int MAX_NUM_COLUMNS = 8;


class Player : public ActorFrame
{
public:
	Player( PlayerOptions po, PlayerNumber pn );

	virtual void Update( float fDeltaTime, float fSongBeat, float fMaxBeatDifference );
	virtual void RenderPrimitives();

	void SetSteps( const Steps& newSteps, bool bLoadOnlyLeftSide = false, bool bLoadOnlyRightSide = false );
	void SetX( float fX );
	void CrossedIndex( int iIndex );

	ScoreSummary GetScoreSummary();
	int	UpdateStepsMissedOlderThan( float fMissIfOlderThanThisBeat );
	void HandlePlayerStep( float fSongBeat, Step player_step, float fMaxBeatDiff );

protected:
	void CheckForCompleteStep( float fSongBeat, Step player_step, float fMaxBeatDiff );
	void OnCompleteStep( float fSongBeat, Step player_step, float fMaxBeatDiff, int iStepIndex );

	PlayerOptions m_PlayerOptions;
	PlayerNumber	m_PlayerNumber;

	int		m_iCurCombo;
	int		m_iMaxCombo;
	float	m_fSongBeat;

	// index is quarter beat number (e.g. beat 30 is index 30*4)
	Step		m_OriginalStep[MAX_STEP_ELEMENTS];
	Step		m_LeftToStepOn[MAX_STEP_ELEMENTS];
	StepScore	m_StepScore[MAX_STEP_ELEMENTS];
	CArray<HoldStep, HoldStep&>		m_HoldSteps;
	//StepTiming	m_StepTiming[MAX_STEP_ELEMENTS];
	CArray<HoldStepScore, HoldStepScore>		m_HoldStepScores;

	// common to color and gray arrows
	float m_fArrowsCenterX;

	int m_iNumColumns;	// will vary depending on the number panels (4,6,8,etc)
	CMap<Step, Step, int, int>	m_StepToColumnNumber;
	CMap<int, int, Step, Step>	m_ColumnNumberToStep;
	CMap<int, int, float, float&>	m_ColumnToRotation;
	float GetArrowColumnX( int iColNum );

	// color arrows
	void SetColorArrowsX( int iX );
	void UpdateColorArrows( float fDeltaTime );
	float GetColorArrowYPos( int iStepIndex, float fSongBeat );
	float GetColorArrowYOffset( int iStepIndex, float fSongBeat );
	float GetColorArrowAlphaFromYOffset( float fYOffset );
	void DrawColorArrows();
	int			m_iColorArrowFrameOffset[MAX_STEP_ELEMENTS];
	ColorArrow	m_ColorArrow[MAX_NUM_COLUMNS];

	// gray arrows
	void SetGrayArrowsX( int iX );
	void UpdateGrayArrows( float fDeltaTime );
	float GetGrayArrowYPos();
	void DrawGrayArrows();
	void GrayArrowStep( int iCol, StepScore score );
	GrayArrow	m_GrayArrow[MAX_NUM_COLUMNS];

	// ghost arrows
	void SetGhostArrowsX( int iX );
	void UpdateGhostArrows( float fDeltaTime );
	void DrawGhostArrows();
	void GhostArrowStep( int iCol, StepScore score );
	GhostArrow	m_GhostArrow[MAX_NUM_COLUMNS];
	GhostArrowBright	m_GhostArrowBright[MAX_NUM_COLUMNS];
	HoldGhostArrow	m_HoldGhostArrow[MAX_NUM_COLUMNS];

	// holder for judgement and combo displays
	ActorFrame m_frameJudgementAndCombo;

	// judgement
	void SetJudgementX( int iX );
	void UpdateJudgement( float fDeltaTime );
	void DrawJudgement();
	void SetJudgement( StepScore score );
	float		m_fJudgementDisplayCountdown;
	Sprite		m_sprJudgement;

	void SetHoldJudgement( int iCol, HoldStepScore::HoldScore score );
	float		m_fHoldJudgementDisplayCountdown[MAX_NUM_COLUMNS];
	Sprite		m_sprHoldJudgement[MAX_NUM_COLUMNS];

	// combo
	void SetComboX( int iX );
	void UpdateCombo( float fDeltaTime );
	void DrawCombo();
	void ContinueCombo();
	void EndCombo();
	bool		m_bComboVisible;
	Sprite		m_sprCombo;
	BitmapText	m_textComboNumber;

	// life meter
	void SetLifeMeterX( int iX );
	void UpdateLifeMeter( float fDeltaTime );
	void DrawLifeMeter();
	void ChangeLife( StepScore score );
public:
	float GetLifePercentage() { return m_fLifePercentage; };
private:
	float		m_fLifePercentage;
	Sprite		m_sprLifeMeterFrame;
	Sprite		m_sprLifeMeterPills;

	// score
	void SetScoreX( int iX );
	void UpdateScore( float fDeltaTime );
	void DrawScore();
	void ChangeScore( StepScore stepscore, int iCurCombo );
	float				m_fScore;
	Sprite				m_sprScoreFrame;
	ScoreDisplayRolling	m_ScoreDisplay;

	// assist
	SoundSet m_soundAssistTick;

};




#endif