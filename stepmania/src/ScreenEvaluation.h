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
#include "BonusInfoFrame.h"
#include "BannerWithFrame.h"


const int NUM_JUDGE_LINES =	6;	// perfect, great, good, boo, miss, ok
const int STAGES_TO_SHOW_IN_SUMMARY = 3;	// only show the latest three stages in a summary

class ScreenEvaluation : public Screen
{
public:
	ScreenEvaluation( bool bSummary );
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void TweenOnScreen();
	virtual void TweenOffScreen();
	virtual void MenuBack( const PlayerNumber p );
	virtual void MenuStart( const PlayerNumber p );

protected:
	enum ResultMode	{ RM_ARCADE_STAGE, RM_ARCADE_SUMMARY, RM_ONI };
	ResultMode			m_ResultMode;

	MenuElements		m_Menu;

	BannerWithFrame		m_BannerWithFrame[STAGES_TO_SHOW_IN_SUMMARY];

	Sprite				m_sprGradeFrame[NUM_PLAYERS];
	GradeDisplay		m_Grades[NUM_PLAYERS];
	BitmapText			m_textOniPercent[NUM_PLAYERS];

	BonusInfoFrame		m_BonusInfoFrame[NUM_PLAYERS];

	Sprite				m_sprJudgeLabels[NUM_JUDGE_LINES];
	BitmapText			m_textJudgeNumbers[NUM_JUDGE_LINES][NUM_PLAYERS];

	Sprite				m_sprScoreLabel;
	ScoreDisplayNormal	m_ScoreDisplay[NUM_PLAYERS];

	bool				m_bNewRecord[NUM_PLAYERS];
	BitmapText			m_textNewRecord[NUM_PLAYERS];

	bool				m_bTryExtraStage;
	BitmapText			m_textTryExtraStage;
};



