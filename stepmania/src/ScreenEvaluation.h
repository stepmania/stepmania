/*
-----------------------------------------------------------------------------
 Class: ScreenEvaluation

 Desc: Shows the user their score after gameplay has ended.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Screen.h"
#include "Sprite.h"
#include "RandomStream.h"
#include "BitmapText.h"
#include "GradeDisplay.h"
#include "MenuElements.h"
#include "Banner.h"
#include "ScoreDisplayNormal.h"
#include "StageBox.h"
#include "BannerWithFrame.h"


const int NUM_JUDGE_LINES =	7;	// perfect, great, good, boo, miss, ok, max_combo
const int STAGES_TO_SHOW_IN_SUMMARY = 3;	// only show the latest three stages in a summary


class ScreenEvaluation : public Screen
{
public:
	ScreenEvaluation( bool bFinalEval = false );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void TweenOnScreen();
	virtual void TweenOffScreen();
	virtual void MenuBack( PlayerNumber p );
	virtual void MenuStart( PlayerNumber p );

protected:
	enum ResultMode	{ RM_ARCADE_STAGE, RM_ARCADE_SUMMARY, RM_ONI };
	ResultMode			m_ResultMode;

	MenuElements		m_Menu;

	BannerWithFrame		m_BannerWithFrame[STAGES_TO_SHOW_IN_SUMMARY];
	BitmapText			m_textStage;

	// used in arcade
	Sprite				m_sprGradeFrame[NUM_PLAYERS];
	GradeDisplay		m_Grades[NUM_PLAYERS];

	// used in Oni/Endless
	Sprite				m_sprPercentFrame[NUM_PLAYERS];
	BitmapText			m_textOniPercentLarge[NUM_PLAYERS];
	BitmapText			m_textOniPercentSmall[NUM_PLAYERS];

	// used in arcade
	Sprite				m_sprBonusFrame[NUM_PLAYERS];
	Sprite				m_sprPossibleBar[NUM_PLAYERS][NUM_RADAR_VALUES];
	Sprite				m_sprActualBar[NUM_PLAYERS][NUM_RADAR_VALUES];

	// used in Oni/Endless
	Sprite				m_sprCourseFrame[NUM_PLAYERS];
	BitmapText			m_textTime[NUM_PLAYERS];
	BitmapText			m_textSongsSurvived[NUM_PLAYERS];

	Sprite				m_sprJudgeLabels[NUM_JUDGE_LINES];
	BitmapText			m_textJudgeNumbers[NUM_JUDGE_LINES][NUM_PLAYERS];

	Sprite				m_sprScoreLabel;
	ScoreDisplayNormal	m_ScoreDisplay[NUM_PLAYERS];

	BitmapText			m_textNewRecord[NUM_PLAYERS];

	bool				m_bTryExtraStage;
	BitmapText			m_textTryExtraStage;
};

class ScreenFinalEvaluation : public ScreenEvaluation
{
public:
	ScreenFinalEvaluation() : ScreenEvaluation(true) {};
};


