/*
-----------------------------------------------------------------------------
 Class: ScreenRanking

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Ben Nordstrom
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_RANKING_LINES
#include "Style.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "Banner.h"
#include "ListDisplay.h"
#include "ActorUtil.h"

class Course;
class Song;

enum PageType
{
	PAGE_TYPE_CATEGORY, 
	PAGE_TYPE_COURSE, 
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
		}

		PageType		type;
		int				colorIndex;
		StepsType		nt;
		RankingCategory	category;
		Course*			pCourse;
		CourseDifficulty	cd;
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
	Sprite m_sprDifficulty[NUM_DIFFICULTIES];	// for all_steps
	struct StepsScoreRowItem : public ActorFrame	// for all_steps
	{
		Song *m_pSong;
		Sprite	m_sprSongFrame;
		BitmapText	m_textSongTitle;
		BitmapText m_textStepsScore[NUM_DIFFICULTIES];
	};
	vector<StepsScoreRowItem*> m_vpStepsScoreRowItem;	// for all_steps
	ListDisplay m_ListScoreRowItems;
	Sprite m_sprCourseDifficulty[NUM_DIFFICULTIES];	// for all_courses
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



