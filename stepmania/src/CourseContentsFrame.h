#pragma once
/*
-----------------------------------------------------------------------------
 Class: CourseContentsFrame

 Desc: Holds course name and banner.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "FootMeter.h"
#include "BitmapText.h"
#include "ActorFrame.h"
class Course;

const int MAX_COURSE_CONTENTS = 8;

class CourseContentsFrame : public ActorFrame
{
public:
	CourseContentsFrame();

	void Update( float fDeltaTime );

	void SetFromCourse( Course* pCourse );

protected:

	BitmapText		m_textContents[MAX_COURSE_CONTENTS];
	FootMeter		m_Meters[MAX_COURSE_CONTENTS];
};
