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

#include "FootMeter.h"
#include "BitmapText.h"
#include "TextBanner.h"
#include "ActorFrame.h"
#include "Sprite.h"
#include "Quad.h"
#include "CourseEntryDisplay.h"
class Course;
class Song;
struct Notes;


const int MAX_VISIBLE_CONTENTS = 5;
const int MAX_TOTAL_CONTENTS = 56;



class CourseContentsList : public ActorFrame
{
public:
	CourseContentsList();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetFromCourse( Course* pCourse );
	void TweenInAfterChangedCourse();

protected:

	Quad		m_quad;

	int						m_iNumContents;
	CourseEntryDisplay		m_CourseContentDisplays[MAX_TOTAL_CONTENTS];
	int ContentsBarHeight, ContentsBarWidth;

	float m_fTimeUntilScroll;
	float m_fItemAtTopOfList;	// between 0 and m_iNumContents
};

#endif
