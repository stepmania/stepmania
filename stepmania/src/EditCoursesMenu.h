#ifndef EditCoursesMenu_H
#define EditCoursesMenu_H
/*
-----------------------------------------------------------------------------
 Class: EditCoursesMenu

 Desc: UI on Edit Menu screen.  Create Steps, delete Steps, or launch Steps 
	in editor.
 
 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Banner.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "DifficultyMeter.h"
#include "RandomSample.h"


class EditCoursesMenu: public ActorFrame 
{
public:
	EditCoursesMenu();
	~EditCoursesMenu();
	virtual void DrawPrimitives();

	bool CanGoUp();
	bool CanGoDown();
	bool CanGoLeft();
	bool CanGoRight();

	void Up();
	void Down();
	void Left();
	void Right();

	enum Row 
	{ 
		ROW_COURSE, 
		ROW_SAVE, 
		ROW_COURSE_OPTIONS, 
		ROW_ENTRY,
		ROW_ENTRY_TYPE, 
		ROW_ENTRY_OPTIONS, 
		ROW_ENTRY_MODIFIERS, 
		NUM_ROWS 
	} m_SelectedRow;
	CString RowToString( Row r )
	{
		const CString s[NUM_ROWS] = 
		{
			"Course",
			"Save",
			"Course Options",
			"Entry",
			"Entry Type",
			"Entry Options",
			"Entry Modifiers"
		};
		return s[r];
	}

	Course*		GetSelectedCourse()				{ return m_pCourses[m_iSelection[ROW_COURSE]]; }

private:
	Sprite	m_sprArrows[2];

	int			m_iSelection[NUM_ROWS];
	BitmapText	m_textLabel[NUM_ROWS];
	BitmapText	m_textValue[NUM_ROWS];

	Banner		m_CourseBanner;
	Banner		m_EntryBanner;
	TextBanner  m_EntryTextBanner;

	vector<Course*>		m_pCourses;

	void OnRowValueChanged( Row row );
	void ChangeToRow( Row newRow );

	RandomSample	m_soundChangeRow;
	RandomSample	m_soundChangeValue;
};

#endif
