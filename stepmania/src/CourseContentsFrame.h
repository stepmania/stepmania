#ifndef COURSECONTENTSFRAME_H
#define COURSECONTENTSFRAME_H
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
#include "TextBanner.h"
#include "ActorFrame.h"
#include "Sprite.h"
#include "Quad.h"
class Course;
class Song;
struct Notes;


const int MAX_VISIBLE_CONTENTS = 5;
const int MAX_TOTAL_CONTENTS = 56;


class CourseContentDisplay : public ActorFrame
{
public:
	CourseContentDisplay();

	void Load( int iNum, Song* pSong, Notes* pNotes );

	Sprite		m_sprFrame;
	BitmapText	m_textNumber;
	TextBanner	m_TextBanner;
	BitmapText	m_textFoot;
	BitmapText	m_textDifficultyNumber;
};


class CourseContentsFrame : public ActorFrame
{
public:
	CourseContentsFrame();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetFromCourse( Course* pCourse );

protected:

	Quad		m_quad;

	int						m_iNumContents;
	CourseContentDisplay	m_CourseContentDisplays[MAX_TOTAL_CONTENTS];
	int ContentsBarHeight, ContentsBarWidth;

	float m_fTimeUntilScroll;
	float m_fItemAtTopOfList;	// between 0 and m_iNumContents
};

#endif
