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
#include "BitmapText.h"
#include "GradeDisplay.h"
#include "MenuElements.h"
#include "Banner.h"
#include "ScoreDisplayNormal.h"
#include "Banner.h"
#include "DifficultyIcon.h"


const int MAX_SONGS_TO_SHOW = 5;	// In summary, we show last 3 stages, plus extra stages if passed
const int NUM_JUDGE_LINES =	8;	// marvelous, perfect, great, good, boo, miss, ok, max_combo
const int NUM_SCORE_LINES = 2;	// score, time


class ScreenEvaluation : public Screen
{
public:
	enum Type	{ stage, summary, course };
	ScreenEvaluation( CString sClassName, Type type );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

//	virtual void TweenOnScreen();
	virtual void TweenOffScreen();
	virtual void MenuLeft( PlayerNumber pn );
	virtual void MenuRight( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );

protected:
	CString				m_sClassName;
	Type				m_Type;

	MenuElements		m_Menu;

	//
	// banner area
	//
	Banner				m_Banner[MAX_SONGS_TO_SHOW];
	Sprite				m_sprBannerFrame[MAX_SONGS_TO_SHOW];
	Sprite				m_sprStage;
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];

	//
	// grade area
	//
	Sprite				m_sprGradeFrame[NUM_PLAYERS];
	GradeDisplay		m_Grades[NUM_PLAYERS];
	BitmapText			m_textPercentWhole[NUM_PLAYERS];
	BitmapText			m_textPercentRemainder[NUM_PLAYERS];
	BitmapText			m_textDancePoints[NUM_PLAYERS];

	//
	// bonus area
	//
	Sprite				m_sprBonusFrame[NUM_PLAYERS];
	Sprite				m_sprPossibleBar[NUM_PLAYERS][NUM_RADAR_CATEGORIES];
	Sprite				m_sprActualBar[NUM_PLAYERS][NUM_RADAR_CATEGORIES];
	BitmapText			m_textSongsSurvived[NUM_PLAYERS];

	//
	// judgement area
	//
	Sprite				m_sprJudgeLabels[NUM_JUDGE_LINES];
	BitmapText			m_textJudgeNumbers[NUM_JUDGE_LINES][NUM_PLAYERS];

	//
	// score area
	//
	Sprite				m_sprScoreLabel[NUM_SCORE_LINES];
	ScoreDisplayNormal	m_ScoreDisplay[NUM_SCORE_LINES][NUM_PLAYERS];
	Sprite				m_sprNewRecord[NUM_PLAYERS];
	bool				m_bTryExtraStage;
	Sprite				m_sprTryExtraStage;
};

class ScreenEvaluationStage : public ScreenEvaluation
{
public:
	ScreenEvaluationStage() : ScreenEvaluation("ScreenEvaluationStage",ScreenEvaluation::stage) {};
};

class ScreenEvaluationSummary : public ScreenEvaluation
{
public:
	ScreenEvaluationSummary() : ScreenEvaluation("ScreenEvaluationSummary",ScreenEvaluation::summary) {};
};

class ScreenEvaluationCourse : public ScreenEvaluation
{
public:
	ScreenEvaluationCourse() : ScreenEvaluation("ScreenEvaluationCourse",ScreenEvaluation::course) {};
};


