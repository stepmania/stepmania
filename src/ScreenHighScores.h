#ifndef ScreenHighScores_H
#define ScreenHighScores_H

#include "ScreenAttract.h"
#include "Course.h"
#include "DynamicActorScroller.h"

#include <vector>


typedef std::pair<Difficulty, StepsType> DifficultyAndStepsType;

enum HighScoresType
{
	HighScoresType_AllSteps,	// Top 1 HighScore for N Steps in each Song
	HighScoresType_NonstopCourses,	// Top 1 HighScore for N Trails in each Course
	HighScoresType_OniCourses,
	HighScoresType_SurvivalCourses,
	HighScoresType_AllCourses,
	NUM_HighScoresType,
	HighScoresType_Invalid
};
LuaDeclareType( HighScoresType );


class ScoreScroller: public DynamicActorScroller
{
public:
	ScoreScroller();
	void LoadSongs( int iNumRecentScores );
	void LoadCourses( CourseType ct, int iNumRecentScores );
	void Load( RString sClassName );
	void SetDisplay( const std::vector<DifficultyAndStepsType> &DifficultiesToShow );
	bool Scroll( int iDir );
	void ScrollTop();

protected:
	virtual void ConfigureActor( Actor *pActor, int iItem );
	std::vector<DifficultyAndStepsType> m_DifficultiesToShow;

	struct ScoreRowItemData // for all_steps and all_courses
	{
		ScoreRowItemData() { m_pSong = nullptr; m_pCourse = nullptr; }

		Song *m_pSong;
		Course *m_pCourse;
	};
	std::vector<ScoreRowItemData> m_vScoreRowItemData;

	ThemeMetric<int>	SCROLLER_ITEMS_TO_DRAW;
	ThemeMetric<float>	SCROLLER_SECONDS_PER_ITEM;
};

class ScreenHighScores: public ScreenAttract
{
public:
	virtual void Init();
	virtual void BeginScreen();

	void HandleScreenMessage( const ScreenMessage SM );
	virtual bool Input( const InputEventPlus &input );
	virtual bool MenuStart( const InputEventPlus &input );
	virtual bool MenuBack( const InputEventPlus &input );
	virtual bool MenuLeft( const InputEventPlus &input )	{ DoScroll(-1); return true; }
	virtual bool MenuRight( const InputEventPlus &input )	{ DoScroll(+1); return true; }
	virtual bool MenuUp( const InputEventPlus &input )	{ DoScroll(-1); return true; }
	virtual bool MenuDown( const InputEventPlus &input )	{ DoScroll(+1); return true; }

private:
	void DoScroll( int iDir );

	ThemeMetric<bool>	MANUAL_SCROLLING;
	ThemeMetric<HighScoresType>	HIGH_SCORES_TYPE;
	ThemeMetric<int>		NUM_COLUMNS;
	ThemeMetric1D<Difficulty>	COLUMN_DIFFICULTY;
	ThemeMetric1D<StepsType>	COLUMN_STEPS_TYPE;
	ThemeMetric<int>		MAX_ITEMS_TO_SHOW;
	ScoreScroller m_Scroller;
};

#endif

/*
 * (c) 2001-2007 Chris Danford, Ben Nordstrom, Glenn Maynard
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
