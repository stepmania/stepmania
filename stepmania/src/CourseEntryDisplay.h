#ifndef CourseEntryDisplay_H
#define CourseEntryDisplay_H
/*
-----------------------------------------------------------------------------
 Class: CourseEntryDisplay

 Desc: Holds course name and banner.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "TextBanner.h"
#include "ActorFrame.h"
#include "Sprite.h"
#include "Quad.h"
#include "GameConstantsAndTypes.h"
#include "Course.h"
class Course;
class Song;
class Steps;


class CourseEntryDisplay : public ActorFrame
{
public:
	void Load();

	void LoadFromCourseInfo( int iNum, const Course *pCourse, const Course::Info &ci );

private:
	Sprite		m_sprFrame;
	BitmapText	m_textNumber;
	TextBanner	m_TextBanner;
	BitmapText	m_textFoot;
	BitmapText	m_textDifficultyNumber;
	BitmapText	m_textModifiers;
};


#endif
