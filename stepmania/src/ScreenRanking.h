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

class Course;
class Song;

class ScreenRanking : public ScreenAttract
{
public:
	ScreenRanking( CString sName );
	~ScreenRanking();

	void HandleScreenMessage( const ScreenMessage SM );

protected:
	struct PageToShow
	{
		enum { TYPE_CATEGORY, TYPE_COURSE, TYPE_ALL_STEPS, TYPE_ALL_COURSES } type;
		int				colorIndex;
		StepsType		nt;
		RankingCategory	category;
		Course*			pCourse;
		PageToShow(): pCourse(NULL) { }
	};

	float SetPage( PageToShow pts );
	void TweenPageOnScreen();
	void TweenPageOffScreen();


	Banner m_Banner;	// for course
	Sprite m_sprBannerFrame;	// for course
	BitmapText m_textCourseTitle; // for course
	BitmapText m_textCategory;	// for category
	BitmapText m_textStepsType;	// for category, course, all_steps

	Sprite	   m_sprBullets[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textNames[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textScores[NUM_RANKING_LINES];	// for category and course
	BitmapText m_textPoints[NUM_RANKING_LINES];	// for course
	BitmapText m_textTime[NUM_RANKING_LINES];	// for course
	Sprite m_sprDifficulty[NUM_DIFFICULTIES];	// for all_steps
	struct StepsScoreRowItem : public ActorFrame	// for all_steps
	{
		Sprite	m_sprSongFrame;
		BitmapText	m_textSongTitle;
		BitmapText m_textStepsScore[NUM_DIFFICULTIES];
	};
	vector<StepsScoreRowItem*> m_vpStepsScoreRowItem;	// for all_steps
	ListDisplay m_ListScoreRowItems;
	Sprite m_sprCourseDifficulty[NUM_DIFFICULTIES];	// for all_courses
	vector<StepsScoreRowItem*> m_vpCourseScoreRowItem;	// for all_courses
	ListDisplay m_ListCourseRowItems;

	vector<PageToShow>	m_vPagesToShow;
	vector<Difficulty>  m_vDiffsToShow;
};



