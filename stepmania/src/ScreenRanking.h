#ifndef SCREEN_RANKING_H
#define SCREEN_RANKING_H

#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_RANKING_LINES
#include "BitmapText.h"
#include "Banner.h"
#include "ActorScroller.h"
#include "ActorUtil.h"
#include "Difficulty.h"
#include "ThemeMetric.h"
#include "CommonMetrics.h"

class Course;
class Song;
class Trail;

enum PageType
{
	PAGE_TYPE_CATEGORY, 
	PAGE_TYPE_TRAIL, 
	PAGE_TYPE_ALL_STEPS, 
	PAGE_TYPE_NONSTOP_COURSES,
	PAGE_TYPE_ONI_COURSES,
	PAGE_TYPE_SURVIVAL_COURSES,
	NUM_PAGE_TYPES
};
#define FOREACH_PageType( pt ) FOREACH_ENUM( PageType, NUM_PAGE_TYPES, pt )
const CString& PageTypeToString( PageType pt );
PageType StringToPageType( const CString& s );

const int NUM_RANKING_LINES	= 5;

class ScreenRanking : public ScreenAttract
{
public:
	ScreenRanking( CString sName );
	virtual void Init();
	virtual void BeginScreen();
	~ScreenRanking();

	virtual void Input( const InputEventPlus &input );
	virtual void MenuLeft( PlayerNumber pn, const InputEventType type )		{ Scroll(-1); }
	virtual void MenuRight( PlayerNumber pn, const InputEventType type )	{ Scroll(+1); }
	virtual void MenuUp( PlayerNumber pn, const InputEventType type )		{ Scroll(-1); }
	virtual void MenuDown( PlayerNumber pn, const InputEventType type )		{ Scroll(+1); }
	virtual void Scroll( int iDir );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

	
	void HandleScreenMessage( const ScreenMessage SM );

protected:
	struct PageToShow
	{
		PageToShow()
		{
			pCourse = NULL;
			pTrail = NULL;
		}

		int				colorIndex;
		StepsType		st;
		RankingCategory	category;
		Course*			pCourse;
		Trail*			pTrail;
	};

	float SetPage( PageToShow pts );

	PageType m_PageType;

	Banner m_Banner;	// for course
	BitmapText m_textCourseTitle; // for course
	BitmapText m_textCategory;	// for category
	BitmapText m_textStepsType;	// for category, course, all_steps
	AutoActor  m_sprPageType;

	AutoActor  m_sprBullets[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textNames[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textScores[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textPoints[NUM_RANKING_LINES];	// for course
	BitmapText m_textTime[NUM_RANKING_LINES];	// for course
	
	AutoActor  m_sprDifficulty[NUM_DIFFICULTIES];	// for all_steps
	struct ScoreRowItem : public ActorFrame		// for all_steps and all_courses
	{
		ScoreRowItem() { m_pSong = NULL; m_pCourse = NULL; }

		Song *m_pSong;
		Course *m_pCourse;
		AutoActor	m_sprFrame;
		BitmapText	m_textTitle;
		BitmapText m_textScore[NUM_DIFFICULTIES];
	};
	vector<ScoreRowItem> m_vScoreRowItem;	// for all_steps
	ActorScroller m_ListScoreRowItems;

	vector<PageToShow>	m_vPagesToShow;
	unsigned m_iNextPageToShow;

	// Don't use the version in CommonMetrics because we may have multiple 
	// ranking screens that want to show different types and difficulties.
	ThemeMetricStepsTypesToShow			STEPS_TYPES_TO_SHOW;
	ThemeMetricDifficultiesToShow		DIFFICULTIES_TO_SHOW;

	ThemeMetric<bool>			SHOW_CATEGORIES;
	ThemeMetric<bool>			SHOW_STEPS_SCORES;
	ThemeMetric<bool>			SHOW_NONSTOP_COURSE_SCORES;
	ThemeMetric<bool>			SHOW_ONI_COURSE_SCORES;
	ThemeMetric<bool>			SHOW_SURVIVAL_COURSE_SCORES;
	ThemeMetric<bool>			SHOW_ONLY_MOST_RECENT_SCORES;
	ThemeMetric<int>			NUM_MOST_RECENT_SCORES_TO_SHOW;
	ThemeMetric<float>			SECONDS_PER_PAGE;
	ThemeMetric<float>			PAGE_FADE_SECONDS;
	ThemeMetric<CString>		NO_SCORE_NAME;

	ThemeMetric<float>			ROW_SPACING_X;
	ThemeMetric<float>			ROW_SPACING_Y;
	ThemeMetric<float>			COL_SPACING_X;
	ThemeMetric<float>			COL_SPACING_Y;
	ThemeMetric1D<RageColor>	STEPS_TYPE_COLOR;
	ThemeMetric<int>			SONG_SCORE_ROWS_TO_SHOW;
	ThemeMetric<float>			SONG_SCORE_SECONDS_PER_ROW;
	ThemeMetric<bool>			MANUAL_SCROLLING;
	ThemeMetric<bool>			SHOW_SURVIVAL_TIME;

	ThemeMetric<float>			BULLET_START_X;
	ThemeMetric<float>			BULLET_START_Y;
	ThemeMetric<float>			NAME_START_X;
	ThemeMetric<float>			NAME_START_Y;
	ThemeMetric<float>			SCORE_START_X;
	ThemeMetric<float>			SCORE_START_Y;
	ThemeMetric<float>			POINTS_START_X;
	ThemeMetric<float>			POINTS_START_Y;
	ThemeMetric<float>			TIME_START_X;
	ThemeMetric<float>			TIME_START_Y;
	ThemeMetric<float>			DIFFICULTY_START_X;
	ThemeMetric<float>			DIFFICULTY_Y;
	ThemeMetric<float>			SCORE_OFFSET_START_X;
	ThemeMetric<float>			SCORE_OFFSET_Y;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Ben Nordstrom
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
