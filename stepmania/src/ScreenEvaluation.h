/* ScreenEvaluation - Shows the user their score after gameplay has ended. */

#ifndef SCREEN_EVALUATION_H
#define SCREEN_EVALUATION_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "GradeDisplay.h"
#include "Banner.h"
#include "DifficultyIcon.h"
#include "DifficultyMeter.h"
#include "PercentageDisplay.h"
#include "ActorUtil.h"
#include "RageSound.h"
#include "ThemeMetric.h"

const int MAX_SONGS_TO_SHOW = 5;	// In summary, we show last 3 stages, plus extra stages if passed
enum JudgeLine
{
	JudgeLine_W1, 
	JudgeLine_W2, 
	JudgeLine_W3, 
	JudgeLine_W4, 
	JudgeLine_W5, 
	JudgeLine_Miss, 
	JudgeLine_Held, 
	JudgeLine_MaxCombo, 
	JudgeLine_Error, 
	NUM_JudgeLine
};
enum StatLine
{
	StatLine_Jumps, 
	StatLine_Holds, 
	StatLine_Mines, 
	StatLine_Hands, 
	StatLine_Rolls, 
	NUM_StatLine 
};

class ScreenEvaluation : public ScreenWithMenuElements
{
public:
	ScreenEvaluation( CString sClassName );
	virtual void Init();
	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void TweenOffScreen();
	virtual void MenuBack( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );

protected:
	void EndScreen();

	bool m_bSummary;

	// banner area
	Banner				m_LargeBanner;
	AutoActor			m_sprLargeBannerFrame;
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];
	DifficultyMeter		m_DifficultyMeter[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];
	Banner				m_SmallBanner[MAX_SONGS_TO_SHOW];
	AutoActor			m_sprSmallBannerFrame[MAX_SONGS_TO_SHOW];

	// grade area
	AutoActor			m_sprGradeFrame[NUM_PLAYERS];
	GradeDisplay		m_Grades[NUM_PLAYERS];

	// points area
	bool				m_bNewSongsUnlocked;
	PercentageDisplay	m_Percent[NUM_PLAYERS];
	AutoActor			m_sprPercentFrame[NUM_PLAYERS];

	// bonus area
	AutoActor			m_sprBonusFrame[NUM_PLAYERS];
	Sprite				m_sprPossibleBar[NUM_PLAYERS][NUM_RadarCategory];
	Sprite				m_sprActualBar[NUM_PLAYERS][NUM_RadarCategory];

	// survived area
	AutoActor			m_sprSurvivedFrame[NUM_PLAYERS];
	BitmapText			m_textSurvivedNumber[NUM_PLAYERS];

	// win area
	AutoActor			m_sprWinFrame[NUM_PLAYERS];
	Sprite				m_sprWin[NUM_PLAYERS];

	// judgment area
	Sprite				m_sprJudgeLabels[NUM_JudgeLine];
	BitmapText			m_textJudgeNumbers[NUM_JudgeLine][NUM_PLAYERS];

	// stats area
	AutoActor			m_sprStatsLabel[NUM_StatLine];
	BitmapText			m_textStatsText[NUM_StatLine][NUM_PLAYERS];

	// score area
	AutoActor			m_sprScoreLabel;
	BitmapText			m_textScore[NUM_PLAYERS];

	// total score area
	AutoActor			m_sprTotalScoreLabel;
	BitmapText			m_textTotalScore[NUM_PLAYERS];

	// time area
	AutoActor			m_sprTimeLabel;
	BitmapText			m_textTime[NUM_PLAYERS];

	// extra area
	AutoActor			m_sprMachineRecord[NUM_PLAYERS];
	AutoActor			m_sprPersonalRecord[NUM_PLAYERS];
	bool				m_bTryExtraStage;
	AutoActor			m_sprTryExtraStage;
	bool m_bFailed;

	RageSound	m_soundStart;	// sound played if the player passes or fails

	ThemeMetric<bool> SUMMARY;

	bool m_bSavedScreenshot[NUM_PLAYERS];
};

#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
