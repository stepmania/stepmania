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
#include "SpriteSequence.h"

#include "ColorArrow.h"
#include "GrayArrow.h"
#include "GhostArrow.h"



const int MAX_NUM_COLUMNS = 8;


class Player
{
public:
	Player();

	void SetSteps( const Steps& newSteps );
	void SetX( float fX );
	void Update( const float &fDeltaTime, float fSongBeat, float fMaxBeatDifference );
	void Draw();


	ScoreSummary GetScoreSummary();
	int	UpdateStepsMissedOlderThan( float fMissIfOlderThanThisBeat );
	void HandlePlayerStep( float fSongBeat, Step player_step, float fMaxBeatDiff );

protected:
	void CheckForCompleteStep( float fSongBeat, Step player_step, float fMaxBeatDiff );
	void OnCompleteStep( float fSongBeat, Step player_step, float fMaxBeatDiff, int iStepIndex );

	int		m_iCurCombo;
	int		m_iMaxCombo;
	float	m_fSongBeat;

	enum StepScore{ none, perfect, great, good, boo, miss };
	enum StepTiming{ no_timing, early, late };
	// index is quarter beat number (e.g. beat 30 is index 30*4)
	Step		m_OriginalStep[MAX_STEP_ELEMENTS];
	Step		m_LeftToStepOn[MAX_STEP_ELEMENTS];
	StepScore	m_StepScore[MAX_STEP_ELEMENTS];
	//StepTiming	m_StepTiming[MAX_STEP_ELEMENTS];


	// common to color and gray arrows
	float m_fArrowsCenterX;

	int m_iNumColumns;	// will vary depending on the number panels (4,6,8,etc)
	CMap<Step, Step, int, int>	m_StepToColumnNumber;
	CMap<int, int, Step, Step>	m_ColumnNumberToStep;
	CMap<int, int, float, float&>	m_ColumnToRotation;
	float GetArrowColumnX( int iColNum );

	// color arrows
	void SetColorArrowsX( int iX );
	void UpdateColorArrows( const float& fDeltaTime );
	float GetColorArrowYPos( int iStepIndex, float fSongBeat );
	void DrawColorArrows();
//	Sprite		m_sprColorArrow[MAX_NUM_COLUMNS];
	int			m_iColorArrowFrameOffset[MAX_STEP_ELEMENTS];
	ColorArrow	m_ColorArrow[MAX_NUM_COLUMNS];


	// gray arrows
	void SetGrayArrowsX( int iX );
	void SetGhostArrowsX( int iX );
	void UpdateGrayArrows( const float& fDeltaTime );
	void DrawGrayArrows();
	void GrayArrowStep( int index );
	void GrayArrowGhostStep( int index );
//	Sprite		m_sprGrayArrow[MAX_NUM_COLUMNS];
//	Sprite		m_sprGrayArrowGhost[MAX_NUM_COLUMNS];
	GrayArrow	m_GrayArrow[MAX_NUM_COLUMNS];
	GhostArrow	m_GhostArrow[MAX_NUM_COLUMNS];


	// judgement
	void SetJudgementX( int iX );
	void UpdateJudgement( const float& fDeltaTime );
	void DrawJudgement();
	void SetJudgement( StepScore score );
	float		m_fJudgementDisplayCountdown;
	Sprite		m_sprJudgement;

	// combo
	void SetComboX( int iX );
	void UpdateCombo( const float& fDeltaTime );
	void DrawCombo();
	void SetCombo( int iNum );
	bool		m_bComboVisible;
	Sprite		m_sprCombo;
	SpriteSequence	m_ComboNumber;

	// life meter
	void SetLifeMeterX( int iX );
	void UpdateLifeMeter( const float& fDeltaTime );
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
	void UpdateScore( const float& fDeltaTime );
	void DrawScore();
	void ChangeScore( StepScore stepscore, int iCurCombo );
	float		m_fScore;
	Sprite		m_sprScoreFrame;
	SpriteSequence	m_ScoreNumber;

};




#endif