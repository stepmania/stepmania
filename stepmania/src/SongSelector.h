/*
-----------------------------------------------------------------------------
 Class: SongSelector

 Desc: Choose a game, style, song and notes on one screen.
 
   Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef SONG_SELECTOR_H
#define SONG_SELECTOR_H

#include "stdafx.h"
#include "ActorFrame.h"

#include "MenuElements.h"
#include "Banner.h"
#include "TextBanner.h"

class SongSelector: public ActorFrame {
public:
	SongSelector();
	~SongSelector();
	virtual void DrawPrimitives();

	void Up();
	void Down();
	void Left();
	void Right();

	CString		GetSelectedGroup()		{ return m_sGroups[m_iSelectedGroup]; };
	Song*		GetSelectedSong()		{ return m_pSongs[m_iSelectedSong]; };
	NotesType	GetSelectedNotesType()	{ return m_NotesTypes[m_iSelectedNotesType]; };
	Notes*		GetSelectedNotes()		{ return m_pNotess[m_iSelectedNotes]; };

	void TweenOffScreenToBlack( ScreenMessage smSendWhenDone, bool bPlayBackSound );

	/* not used yet */
	bool NewNotes;
	void AllowNewNotes(bool NewNotes_=true) { NewNotes = NewNotes; }

private:
	enum SelectedRow { ROW_GROUP, ROW_SONG, ROW_NOTES_TYPE, ROW_STEPS, NUM_ROWS };
	SelectedRow m_SelectedRow;

	MenuElements m_Menu;

	CStringArray m_sGroups;
	int			m_iSelectedGroup;		// index into m_sGroups
	BitmapText	m_textGroup;

	CArray<Song*, Song*> m_pSongs;
	int			m_iSelectedSong;	// index into m_pSongs
	Banner		m_Banner;
	TextBanner  m_TextBanner;

	Sprite		m_sprArrowLeft;
	Sprite		m_sprArrowRight;
	CArray<NotesType, NotesType> m_NotesTypes;
	int			m_iSelectedNotesType;	// index into m_NotesTypes
	BitmapText	m_textNotesType;

	CArray<Notes*, Notes*> m_pNotess;
	int			m_iSelectedNotes;	// index into m_pNotess
	BitmapText	m_textNotes;

	RandomSample	m_soundChangeMusic;

	void OnGroupChange();
	void OnSongChange();
	void OnNotesTypeChange();
	void OnNotesChange();
	
	void ChangeSelectedRow( SelectedRow row );
};

#endif
