/*
-----------------------------------------------------------------------------
 Class: ScreenRanking

 Desc: Base class for all attraction screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"
#include "GameConstantsAndTypes.h"	// for NUM_HIGH_SCORE_LINES
#include "Style.h"


class Course;


class ScreenRanking : public ScreenAttract
{
public:
	ScreenRanking();

	void HandleScreenMessage( const ScreenMessage SM );

protected:
	struct CategoryToShow
	{
		enum { TYPE_CATEGORY, TYPE_COURSE } type;
		NotesType		nt;		// used in category and course
		RankingCategory	category;
		Course*			pCourse;
	};

	void ShowCategory( CategoryToShow cts );


	BitmapText m_textCategory;
	BitmapText m_textType;
	Sprite m_sprBullets[NUM_HIGH_SCORE_LINES];
	BitmapText m_textNames[NUM_HIGH_SCORE_LINES];
	BitmapText m_textScores[NUM_HIGH_SCORE_LINES];

	vector<CategoryToShow>	m_vCategoriesToShow;
};



