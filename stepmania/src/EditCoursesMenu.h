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
#include "RageSound.h"
#include "Course.h"
#include "ScreenMessage.h"


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
	void Start();
	void HandleScreenMessage( const ScreenMessage SM );

	enum Row 
	{ 
		ROW_COURSE, 
		ROW_COURSE_OPTIONS, 
		ROW_ACTION, 
		ROW_ENTRY,
		ROW_ENTRY_TYPE, 
		ROW_ENTRY_OPTIONS, 
		ROW_ENTRY_PLAYER_OPTIONS, 
		ROW_ENTRY_SONG_OPTIONS, 
		NUM_ROWS 
	} m_SelectedRow;
	CString RowToString( Row r )
	{
		const CString s[NUM_ROWS] = 
		{
			"Course",
			"Course Options",
			"Action",
			"Entry",
			"Entry Type",
			"Entry Options",
			"Entry Player Options",
			"Entry Song Options"
		};
		return s[r];
	}

	enum Action
	{ 
		save, 
		add_entry, 
		delete_selected_entry, 
		NUM_ACTIONS
	};
	CString ActionToString( Action a )
	{
		switch( a )
		{
		case save:					return "Save Current Course";
		case add_entry:				return "Add Entry";
		case delete_selected_entry:	return "Delete Selected Entry";
		default:	ASSERT(0);		return "";
		}
	}

	Course*		GetSelectedCourse()			{ return m_pCourses[m_iSelection[ROW_COURSE]]; }
	CourseEntry* GetSelectedEntry();
	Action GetSelectedAction()				{ return (Action)m_iSelection[ROW_ACTION]; }
	CourseEntryType GetSelectedEntryType()	{ return (CourseEntryType)m_iSelection[ROW_ENTRY_TYPE]; }

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

	RageSound	m_soundChangeRow;
	RageSound	m_soundChangeValue;
	RageSound	m_soundStart;
	RageSound	m_soundInvalid;
	RageSound	m_soundSave;
};

#endif
