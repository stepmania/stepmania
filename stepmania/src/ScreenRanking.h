/*
-----------------------------------------------------------------------------
 Class: ScreenRanking

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_RANKING_LINES
#include "Style.h"


class Course;


class ScreenRanking : public ScreenAttract
{
public:
	ScreenRanking();

	void HandleScreenMessage( const ScreenMessage SM );

protected:
	struct PageToShow
	{
		enum { TYPE_CATEGORY, TYPE_COURSE } type;
		int				colorIndex;
		NotesType		nt;		// used in category and course
		RankingCategory	category;
		Course*			pCourse;
	};

	void SetPage( PageToShow pts );
	void TweenPageOnScreen();
	void TweenPageOffScreen();


	BitmapText m_textCategory;
	BitmapText m_textType;
	Sprite m_sprBullets[NUM_RANKING_LINES];
	BitmapText m_textNames[NUM_RANKING_LINES];
	BitmapText m_textScores[NUM_RANKING_LINES];	// for category
	BitmapText m_textPoints[NUM_RANKING_LINES];	// for course
	BitmapText m_textTime[NUM_RANKING_LINES];	// for course

	vector<PageToShow>	m_vPagesToShow;
};



