/* Class: EditCoursesSongMenu - Song selection subscreen for EditCoursesMenu */

#ifndef EDIT_COURSES_SONG_MENU_H
#define EDIT_COURSES_SONG_MENU_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "GameConstantsAndTypes.h"
#include "RageSound.h"
#include "Course.h"
#include "ScreenMessage.h"
#include "song.h"


class EditCoursesSongMenu: public ActorFrame 
{
public:
	EditCoursesSongMenu();
	~EditCoursesSongMenu();
	void SaveToCourseEntry( CourseEntry *pEntry );
	void LoadFromCourseEntry( const CourseEntry *pEntry );

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
		ROW_GROUP, 
		ROW_SONG, 
		ROW_TYPE, 
		ROW_DIFFICULTY, 
		ROW_LOW_METER,
		ROW_HIGH_METER, 
		ROW_BEST_WORST_VALUE, 
		NUM_ROWS 
	} m_SelectedRow;
	CString RowToString( Row r )
	{
		const CString s[NUM_ROWS] = 
		{
			"Group",
			"Song",
			"Type",
			"Difficulty",
			"Low Meter",
			"High Meter",
			"Best/Worst Value",
		};
		return s[r];
	}

	CString GetSelectedGroup() const		{ return m_aGroups[ m_iSelection[ROW_GROUP] ]; }
	Song *GetSelectedSong() const;
	Difficulty GetSelectedDifficulty() const { if( m_iSelection[ROW_DIFFICULTY] == 0 ) return DIFFICULTY_INVALID; else return (Difficulty) (m_iSelection[ROW_DIFFICULTY]-1); }
	CourseEntryType GetSelectedType() const { return (CourseEntryType) m_iSelection[ROW_TYPE]; }
	int GetLowMeter() const { return m_iSelection[ROW_LOW_METER] == 0? -1:m_iSelection[ROW_LOW_METER]; } // any, 1, 2, 3 ...
	int GetHighMeter() const { return m_iSelection[ROW_HIGH_METER] == 0? -1:m_iSelection[ROW_HIGH_METER]; }
	int GetBestWorst() const { return m_iSelection[ROW_BEST_WORST_VALUE]; }

private:
	void SetGroupByName( CString sGroup );
	void UpdateSongList();
	bool ChangeRow( int add );
	Sprite	m_sprArrows[2];

	int			m_iSelection[NUM_ROWS];
	BitmapText	m_textLabel[NUM_ROWS];
	BitmapText	m_textValue[NUM_ROWS];

	vector<Song*>		m_aSongs;
	vector<CString>		m_aGroups;

	void OnRowValueChanged( Row row );
	void ChangeToRow( Row newRow );

	RageSound	m_soundChangeRow;
	RageSound	m_soundChangeValue;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
