#ifndef CourseContentsList_H
#define CourseContentsList_H
/*
-----------------------------------------------------------------------------
 Class: CourseContentsList

 Desc: Holds course name and banner.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include "ActorFrame.h"
#include "Sprite.h"
#include "Quad.h"
#include "CourseEntryDisplay.h"
class Course;
class Song;
class Steps;


const int MAX_VISIBLE_CONTENTS = 5;
const int MAX_TOTAL_CONTENTS = 56;



class CourseContentsList : public ActorFrame
{
public:
	CourseContentsList();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetFromCourse( const Course* pCourse );
	void TweenInAfterChangedCourse();

protected:

	Quad		m_quad;

	int						m_iNumContents;
	CourseEntryDisplay		m_CourseContentDisplays[MAX_TOTAL_CONTENTS];
	float ContentsBarHeight, ContentsBarWidth;

	float m_fTimeUntilScroll;
	float m_fItemAtTopOfList;	// between 0 and m_iNumContents
};

#endif
