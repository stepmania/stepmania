#ifndef SCREEN_RANKING_H
#define SCREEN_RANKING_H

#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_RANKING_LINES
#include "Sprite.h"
#include "BitmapText.h"
#include "Banner.h"
#include "ListDisplay.h"
#include "ActorUtil.h"

class Course;
class Song;
class Trail;

enum PageType
{
	PAGE_TYPE_CATEGORY, 
	PAGE_TYPE_TRAIL, 
	PAGE_TYPE_ALL_STEPS, 
	PAGE_TYPE_ALL_COURSES,
	NUM_PAGE_TYPES
};
#define FOREACH_PageType( pt ) FOREACH_ENUM( PageType, NUM_PAGE_TYPES, pt )
const CString& PageTypeToString( PageType pt );


class ScreenRanking : public ScreenAttract
{
public:
	ScreenRanking( CString sName );
	~ScreenRanking();

	void HandleScreenMessage( const ScreenMessage SM );

protected:
	struct PageToShow
	{
		PageToShow()
		{
			pCourse = NULL;
			pTrail = NULL;
		}

		PageType		type;
		int				colorIndex;
		StepsType		nt;
		RankingCategory	category;
		Course*			pCourse;
		Trail*			pTrail;
	};

	float SetPage( PageToShow pts );
	void TweenPageOnScreen();
	void TweenPageOffScreen();


	Banner m_Banner;	// for course
	Sprite m_sprBannerFrame;	// for course
	BitmapText m_textCourseTitle; // for course
	BitmapText m_textCategory;	// for category
	BitmapText m_textStepsType;	// for category, course, all_steps
	AutoActor  m_sprPageType;

	Sprite	   m_sprBullets[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textNames[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textScores[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textPoints[NUM_RANKING_LINES];	// for course
	BitmapText m_textTime[NUM_RANKING_LINES];	// for course
	AutoActor  m_sprDifficulty[NUM_DIFFICULTIES];	// for all_steps
	struct StepsScoreRowItem : public ActorFrame	// for all_steps
	{
		Song *m_pSong;
		Sprite	m_sprSongFrame;
		BitmapText	m_textSongTitle;
		BitmapText m_textStepsScore[NUM_DIFFICULTIES];
	};
	vector<StepsScoreRowItem*> m_vpStepsScoreRowItem;	// for all_steps
	ListDisplay m_ListScoreRowItems;
	AutoActor m_sprCourseDifficulty[NUM_DIFFICULTIES];	// for all_courses
	struct CourseScoreRowItem : public ActorFrame	// for all_steps
	{
		Course *m_pCourse;
		Sprite	m_sprSongFrame;
		BitmapText	m_textSongTitle;
		BitmapText m_textStepsScore[NUM_DIFFICULTIES];
	};
	vector<CourseScoreRowItem*> m_vpCourseScoreRowItem;	// for all_courses
	ListDisplay m_ListCourseRowItems;

	vector<PageToShow>	m_vPagesToShow;
	vector<Difficulty>  m_vDiffsToShow;
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
