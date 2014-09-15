#ifndef SCREEN_EVALUATION_H
#define SCREEN_EVALUATION_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "GradeDisplay.h"
#include "Banner.h"
#include "PercentageDisplay.h"
#include "ActorUtil.h"
#include "RageSound.h"
#include "ThemeMetric.h"
#include "RollingNumbers.h"

/**
 * @brief How many songs are shown at the end?
 *
 * In the traditional summary, only the last three stages are shown.
 * If any extra stages are passed, those get shown as well. */
const int MAX_SONGS_TO_SHOW = 5;
/** @brief The different judgment lines shown. */
enum JudgmentLine
{
	JudgmentLine_W1,	/**< The line showing absolutely perfect hits. */
	JudgmentLine_W2,	/**< The line showing just-a-smidge-off perfect hits. */
	JudgmentLine_W3,	/**< The line showing almost perfect hits. */
	JudgmentLine_W4,	/**< The line showing hits that were not that good. */
	JudgmentLine_W5,	/**< The line showing hits that were almost missed. */
	JudgmentLine_Miss,	/**< The line showing missed notes. */
	JudgmentLine_Held,	/**< The line showing held down notes. */
	JudgmentLine_MaxCombo,	/**< The line showing the player's max combo. */
	NUM_JudgmentLine,	/**< The number of judgment lines available. */
	JudgmentLine_Invalid
};
/** @brief The number of details based on the radar categories. */
enum DetailLine
{
	DetailLine_NumSteps,	/**< The number of steps hit. */
	DetailLine_Jumps,	/**< The number of jumps hit together. */
	DetailLine_Holds,	/**< The number of holds held. */
	DetailLine_Mines,	/**< The number of mines avoided. */
	DetailLine_Hands,	/**< The number of hands hit (somehow) */
	DetailLine_Rolls,	/**< The number of rolls hit repeatedly. */
	DetailLine_Lifts,	/**< The number of lifts lifted up. */
	DetailLine_Fakes,	/**< The number of fakes to be ignored. */
	NUM_DetailLine		/**< The nuber of detailed lines. */
};
/** @brief Shows the player their score after gameplay has ended. */
class ScreenEvaluation : public ScreenWithMenuElements
{
public:
	ScreenEvaluation();
	virtual ~ScreenEvaluation();
	virtual void Init();
	virtual bool Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual bool MenuBack( const InputEventPlus &input );
	virtual bool MenuStart( const InputEventPlus &input );
	virtual void PushSelf( lua_State *L );
	StageStats *GetStageStats() { return m_pStageStats; }

protected:
	void HandleMenuStart();

	bool			m_bSummary;
	StageStats		*m_pStageStats;
	StageStats		m_FinalEvalStageStats;

	// banner area
	Banner			m_LargeBanner;
	AutoActor		m_sprLargeBannerFrame;
	BitmapText		m_textPlayerOptions[NUM_PLAYERS];
	BitmapText		m_textSongOptions;
	AutoActor		m_sprDisqualified[NUM_PLAYERS];
	Banner			m_SmallBanner[MAX_SONGS_TO_SHOW];
	AutoActor		m_sprSmallBannerFrame[MAX_SONGS_TO_SHOW];

	// grade area
	AutoActor		m_sprGradeFrame[NUM_PLAYERS];
	GradeDisplay	m_Grades[NUM_PLAYERS];

	// points area
	PercentageDisplay	m_Percent[NUM_PLAYERS];
	AutoActor		m_sprPercentFrame[NUM_PLAYERS];

	// bonus area
	AutoActor		m_sprBonusFrame[NUM_PLAYERS];
	Sprite			m_sprPossibleBar[NUM_PLAYERS][NUM_RadarCategory];
	Sprite			m_sprActualBar[NUM_PLAYERS][NUM_RadarCategory];

	// survived area
	AutoActor		m_sprSurvivedFrame[NUM_PLAYERS];
	BitmapText		m_textSurvivedNumber[NUM_PLAYERS];

	// win area
	AutoActor		m_sprWinFrame[NUM_PLAYERS];
	Sprite			m_sprWin[NUM_PLAYERS];

	// judgment area
	AutoActor		m_sprSharedJudgmentLineLabels[NUM_JudgmentLine];
	RollingNumbers		m_textJudgmentLineNumber[NUM_JudgmentLine][NUM_PLAYERS];

	// stats area
	AutoActor		m_sprDetailFrame[NUM_PLAYERS];
	BitmapText		m_textDetailText[NUM_DetailLine][NUM_PLAYERS];

	// score area
	AutoActor		m_sprScoreLabel;
	RollingNumbers		m_textScore[NUM_PLAYERS];

	// time area
	AutoActor		m_sprTimeLabel;
	BitmapText		m_textTime[NUM_PLAYERS];

	RageSound		m_soundStart;	// sound played if the player passes or fails

	ThemeMetric<bool>	SUMMARY;
	ThemeMetric<RString> ROLLING_NUMBERS_CLASS;
	ThemeMetric<RString> ROLLING_NUMBERS_MAX_COMBO_CLASS;
	/** @brief Did a player save a screenshot of their score? */
	bool			m_bSavedScreenshot[NUM_PLAYERS];
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
