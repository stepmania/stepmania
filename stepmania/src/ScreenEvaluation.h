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
	CString				m_sName;
	Type				m_Type;

	MenuElements		m_Menu;

	// banner area
	Banner				m_LargeBanner;
	Sprite				m_sprLargeBannerFrame;
	Sprite				m_sprStage;
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];
	Banner				m_SmallBanner[MAX_SONGS_TO_SHOW];
	Sprite				m_sprSmallBannerFrame[MAX_SONGS_TO_SHOW];

	// grade area
	Sprite				m_sprGradeFrame[NUM_PLAYERS];
	GradeDisplay		m_Grades[NUM_PLAYERS];

	// points area
	bool				m_bNewSongsUnlocked;
	Sprite				m_sprPercentFrame[NUM_PLAYERS];
	BitmapText			m_textPercentWhole[NUM_PLAYERS];
	BitmapText			m_textPercentRemainder[NUM_PLAYERS];
	BitmapText			m_textDancePoints[NUM_PLAYERS];

	// bonus area
	Sprite				m_sprBonusFrame[NUM_PLAYERS];
	Sprite				m_sprPossibleBar[NUM_PLAYERS][NUM_RADAR_CATEGORIES];
	Sprite				m_sprActualBar[NUM_PLAYERS][NUM_RADAR_CATEGORIES];

	// survived area
	Sprite				m_sprSurvivedFrame[NUM_PLAYERS];
	BitmapText			m_textSurvivedNumber[NUM_PLAYERS];

	// win area
	Sprite				m_sprWinFrame[NUM_PLAYERS];
	Sprite				m_sprWin[NUM_PLAYERS];

	// judgment area
	Sprite				m_sprJudgeLabels[NUM_JUDGE_LINES];
	BitmapText			m_textJudgeNumbers[NUM_JUDGE_LINES][NUM_PLAYERS];

	// score area
	Sprite				m_sprScoreLabel;
	BitmapText			m_textScore[NUM_PLAYERS];

	// time area
	Sprite				m_sprTimeLabel;
	BitmapText			m_textTime[NUM_PLAYERS];

	// extra area
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

class ScreenEvaluationNonstop : public ScreenEvaluation
{
public:
	ScreenEvaluationNonstop() : ScreenEvaluation("ScreenEvaluationNonstop",ScreenEvaluation::course) {};
};

class ScreenEvaluationOni : public ScreenEvaluation
{
public:
	ScreenEvaluationOni() : ScreenEvaluation("ScreenEvaluationOni",ScreenEvaluation::course) {};
};

class ScreenEvaluationEndless : public ScreenEvaluation
{
public:
	ScreenEvaluationEndless() : ScreenEvaluation("ScreenEvaluationEndless",ScreenEvaluation::course) {};
};

class ScreenEvaluationBattle : public ScreenEvaluation
{
public:
	ScreenEvaluationBattle() : ScreenEvaluation("ScreenEvaluationBattle",ScreenEvaluation::stage) {};
};

class ScreenEvaluationRave : public ScreenEvaluation
{
public:
	ScreenEvaluationRave() : ScreenEvaluation("ScreenEvaluationRave",ScreenEvaluation::stage) {};
};


