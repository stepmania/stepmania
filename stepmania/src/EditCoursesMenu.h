/* EditCoursesMenu - UI on Edit Courses screen. */

#ifndef EDIT_COURSES_MENU_H
#define EDIT_COURSES_MENU_H

#include "ActorFrame.h"
#include "Banner.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "RageSound.h"
#include "Course.h"
#include "ScreenMessage.h"
#include "EditCoursesSongMenu.h"

class EditCoursesMenu: public ActorFrame 
{
public:
	EditCoursesMenu();
	~EditCoursesMenu();
	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );

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
		case add_entry:				return "Duplicate Current Entry/Add Entry";
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
	RageSound	m_soundSave;

	EditCoursesSongMenu m_SongMenu;
	bool m_bInSongMenu;
};

#endif

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
