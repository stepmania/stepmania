#ifndef EDITMENU_H
#define EDITMENU_H
/*
-----------------------------------------------------------------------------
 Class: EditMenu

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
		ROW_NOTES_TYPE, 
		ROW_DIFFICULTY,
		ROW_SOURCE_NOTES_TYPE, 
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
			"NotesType",
			"Difficulty",
			"Source NotesType",
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


	CString		GetSelectedGroup()				{ return m_sGroups[m_iSelection[ROW_GROUP]]; }
	Song*		GetSelectedSong()				{ return m_pSongs[m_iSelection[ROW_SONG]]; }
	NotesType	GetSelectedNotesType()			{ return m_NotesTypes[m_iSelection[ROW_NOTES_TYPE]]; }
	Difficulty	GetSelectedDifficulty()			{ return (Difficulty)m_iSelection[ROW_DIFFICULTY]; }
	NotesType	GetSelectedSourceNotesType()	{ return m_NotesTypes[m_iSelection[ROW_SOURCE_NOTES_TYPE]]; }
	Difficulty	GetSelectedSourceDifficulty()	{ return (Difficulty)m_iSelection[ROW_SOURCE_DIFFICULTY]; }
	Action		GetSelectedAction()				{ return m_Actions[m_iSelection[ROW_ACTION]]; }

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
	vector<NotesType>	m_NotesTypes;
	vector<Song*>		m_pSongs;
	vector<Action>		m_Actions;

	void OnRowValueChanged( Row row );
	void ChangeToRow( Row newRow );

	RandomSample	m_soundChangeRow;
	RandomSample	m_soundChangeValue;
};

#endif
