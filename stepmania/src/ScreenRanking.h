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
#include "Sprite.h"
#include "BitmapText.h"

class Course;
class Song;

class ScreenRanking : public ScreenAttract
{
public:
	ScreenRanking( CString sName );

	void HandleScreenMessage( const ScreenMessage SM );

protected:
	struct PageToShow
	{
		enum { TYPE_CATEGORY, TYPE_COURSE, TYPE_SONG } type;
		int				colorIndex;
		StepsType		nt;		// used in category and course
		RankingCategory	category;
		Course*			pCourse;
		Song*			pSong;
	};

	void SetPage( PageToShow pts );
	void TweenPageOnScreen();
	void TweenPageOffScreen();


	Sprite m_sprCategory;
	BitmapText m_textCategory;
	Sprite m_sprType;
	Sprite m_sprBullets[NUM_RANKING_LINES];
	BitmapText m_textNames[NUM_RANKING_LINES];
	BitmapText m_textScores[NUM_RANKING_LINES];	// for category
	BitmapText m_textPoints[NUM_RANKING_LINES];	// for course
	BitmapText m_textTime[NUM_RANKING_LINES];	// for course
	BitmapText m_textPercent[NUM_RANKING_LINES][NUM_DIFFICULTIES];	// for song

	vector<PageToShow>	m_vPagesToShow;
};



