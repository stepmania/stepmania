/* EditMenu - UI on Edit Menu screen.  Create Steps, delete Steps, or launch Steps in editor. */

#ifndef EDIT_MENU_H
#define EDIT_MENU_H

#include "ActorFrame.h"
#include "Banner.h"
#include "TextBanner.h"
#include "GameConstantsAndTypes.h"
#include "DifficultyMeter.h"
#include "RandomSample.h"


class EditMenu: public ActorFrame 
{
public:
	EditMenu();
	~EditMenu();
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
		ROW_GROUP, 
		ROW_SONG, 
		ROW_STEPS_TYPE, 
		ROW_DIFFICULTY,
		ROW_SOURCE_STEPS_TYPE, 
		ROW_SOURCE_DIFFICULTY, 
		ROW_ACTION, 
		NUM_ROWS 
	} m_SelectedRow;
	CString RowToString( Row r )
	{
		const CString s[NUM_ROWS] = 
		{
			"Group",
			"Song",
			"StepsType",
			"Difficulty",
			"Source StepsType",
			"Source Difficulty",
			"Action"
		};
		return s[r];
	}

	enum Action
	{
		ACTION_EDIT,
		ACTION_DELETE,
		ACTION_COPY,
		ACTION_AUTOGEN,
		ACTION_BLANK,
		NUM_ACTIONS
	};
	CString ActionToString( Action a )
	{
		const CString s[NUM_ACTIONS] = 
		{
			"Edit Existing",
			"Delete Existing",
			"Create from Source by Copy",
			"Create from Souce by AutoGen",
			"Create with Blank"
		};
		return s[a];
	}

	void RefreshNotes();


	CString		GetSelectedGroup() const			{ ASSERT(m_iSelection[ROW_GROUP] < (int)m_sGroups.size()); return m_sGroups[m_iSelection[ROW_GROUP]]; }
	Song*		GetSelectedSong() const				{ ASSERT(m_iSelection[ROW_SONG] < (int)m_pSongs.size()); return m_pSongs[m_iSelection[ROW_SONG]]; }
	StepsType	GetSelectedStepsType() const		{ ASSERT(m_iSelection[ROW_STEPS_TYPE] < (int)m_StepsTypes.size()); return m_StepsTypes[m_iSelection[ROW_STEPS_TYPE]]; }
	Difficulty	GetSelectedDifficulty() const		{ return (Difficulty)m_iSelection[ROW_DIFFICULTY]; }
	StepsType	GetSelectedSourceStepsType() const	{ ASSERT(m_iSelection[ROW_SOURCE_STEPS_TYPE] < (int)m_StepsTypes.size()); return m_StepsTypes[m_iSelection[ROW_SOURCE_STEPS_TYPE]]; }
	Difficulty	GetSelectedSourceDifficulty() const { return (Difficulty)m_iSelection[ROW_SOURCE_DIFFICULTY]; }
	Action		GetSelectedAction() const			{ ASSERT(m_iSelection[ROW_ACTION] < (int)m_Actions.size()); return m_Actions[m_iSelection[ROW_ACTION]]; }

	Steps*		GetSelectedNotes();
	Steps*		GetSelectedSourceNotes();

private:
	Sprite	m_sprArrows[2];

	int			m_iSelection[NUM_ROWS];
	BitmapText	m_textLabel[NUM_ROWS];
	BitmapText	m_textValue[NUM_ROWS];

	Banner		m_GroupBanner;
	Banner		m_SongBanner;
	TextBanner  m_SongTextBanner;
	DifficultyMeter	m_Meter;
	DifficultyMeter	m_SourceMeter;

	CStringArray		m_sGroups;
	vector<StepsType>	m_StepsTypes;
	vector<Song*>		m_pSongs;
	vector<Action>		m_Actions;

	void OnRowValueChanged( Row row );
	void ChangeToRow( Row newRow );

	RandomSample	m_soundChangeRow;
	RandomSample	m_soundChangeValue;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
