#ifndef SCREEN_RANKING_H
#define SCREEN_RANKING_H

#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_RANKING_LINES
#include "Course.h"
#include "BitmapText.h"
#include "Banner.h"
#include "ActorScroller.h"
#include "DynamicActorScroller.h"
#include "ActorUtil.h"
#include "Difficulty.h"
#include "ThemeMetric.h"
#include "CommonMetrics.h"

class Course;
class Song;
class Trail;
struct HighScoreList;
typedef pair<Difficulty, StepsType> DifficultyAndStepsType;

enum PageType
{
	PageType_Category, 
	PageType_Trail, 
	PageType_AllSteps, 
	PageType_NonstopCourses,
	PageType_OniCourses,
	PageType_SurvivalCourses,
	PageType_AllCourses,
	NUM_PageType,
	PageType_Invalid
};
PageType StringToPageType( const RString& s );

class ScreenRanking : public ScreenAttract
{
public:
	virtual void Init();
	virtual void BeginScreen();

	virtual void Input( const InputEventPlus &input );
	virtual void MenuStart( const InputEventPlus &input );
	virtual void MenuBack( const InputEventPlus &input );

	void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual bool GenericTweenOn() const { return true; }

	struct PageToShow
	{
		PageToShow()
		{
			pCourse = NULL;
			pTrail = NULL;
		}

		int		colorIndex;
		vector<DifficultyAndStepsType> aTypes;
		RankingCategory	category;
		Course*		pCourse;
		Trail*		pTrail;
	};

	virtual float SetPage( const PageToShow &pts );

	PageType m_PageType;

	BitmapText m_textStepsType;	// for category, course, all_steps

	vector<PageToShow>		m_vPagesToShow;
	unsigned			m_iNextPageToShow;

	// Don't use the version in CommonMetrics because we may have multiple 
	// ranking screens that want to show different types and difficulties.
	ThemeMetricStepsTypesToShow	STEPS_TYPES_TO_SHOW;
	ThemeMetric<float>		SECONDS_PER_PAGE;
	ThemeMetric<float>		PAGE_FADE_SECONDS;
	ThemeMetric<bool>		MANUAL_SCROLLING;
};

class ScoreScroller: public DynamicActorScroller
{
public:
	ScoreScroller();
	void LoadSongs( bool bOnlyRecentScores, int iNumRecentScores );
	void LoadCourses( CourseType ct, bool bOnlyRecentScores, int iNumRecentScores );
	void Load( RString sClassName );
	void SetDisplay( const vector<DifficultyAndStepsType> &DifficultiesToShow );
	bool Scroll( int iDir );
	void ScrollTop();

protected:
	virtual void ConfigureActor( Actor *pActor, int iItem );
	vector<DifficultyAndStepsType> m_DifficultiesToShow;

	struct ScoreRowItemData // for all_steps and all_courses
	{
		ScoreRowItemData() { m_pSong = NULL; m_pCourse = NULL; }

		Song *m_pSong;
		Course *m_pCourse;
	};
	vector<ScoreRowItemData> m_vScoreRowItemData;

	ThemeMetric<int>	SONG_SCORE_ROWS_TO_DRAW;
};

class ScreenRankingScroller: public ScreenRanking 
{
public:
	virtual void Init();

	virtual void MenuLeft( const InputEventPlus &input )	{ DoScroll(-1); }
	virtual void MenuRight( const InputEventPlus &input )	{ DoScroll(+1); }
	virtual void MenuUp( const InputEventPlus &input )	{ DoScroll(-1); }
	virtual void MenuDown( const InputEventPlus &input )	{ DoScroll(+1); }

private:
	void DoScroll( int iDir );
	virtual float SetPage( const PageToShow &pts );

	ScoreScroller m_ListScoreRowItems;
};

static const int NUM_RANKING_LINES = 5;

class ScreenRankingLines: public ScreenRanking 
{
public:
	virtual void Init();
	virtual void BeginScreen();

private:
	virtual float SetPage( const PageToShow &pts );

	Banner m_Banner;	// for course
	BitmapText m_textCategory;	// for category
	BitmapText m_textCourseTitle; // for course

	AutoActor  m_sprBullets[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textNames[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textScores[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textPoints[NUM_RANKING_LINES];	// for course
	BitmapText m_textTime[NUM_RANKING_LINES];	// for course
	ThemeMetric1D<RageColor>	STEPS_TYPE_COLOR;

	LocalizedString		NO_SCORE_NAME;
	ThemeMetric<float>	ROW_SPACING_X;
	ThemeMetric<float>	ROW_SPACING_Y;
	ThemeMetric<float>	BULLET_START_X;
	ThemeMetric<float>	BULLET_START_Y;
	ThemeMetric<float>	NAME_START_X;
	ThemeMetric<float>	NAME_START_Y;
	ThemeMetric<float>	SCORE_START_X;
	ThemeMetric<float>	SCORE_START_Y;
	ThemeMetric<float>	POINTS_START_X;
	ThemeMetric<float>	POINTS_START_Y;
	ThemeMetric<float>	TIME_START_X;
	ThemeMetric<float>	TIME_START_Y;
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
