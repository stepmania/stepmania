/*
-----------------------------------------------------------------------------
 File: ScreenEditMenu.h

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "ColorNote.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"
#include "Banner.h"
#include "TextBanner.h"
#include "RandomSample.h"
#include "TransitionInvisible.h"
#include "MenuElements.h"



class ScreenEditMenu : public Screen
{
public:
	ScreenEditMenu();
	virtual ~ScreenEditMenu();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	void BeforeRowChange();
	void AfterRowChange();

	void OnGroupChange();
	void OnSongChange();
	void OnNotesTypeChange();
	void OnStepsChange();

	CString		GetSelectedGroup()		{ return m_sGroups[m_iSelectedGroup]; };
	Song*		GetSelectedSong()		{ return m_pSongs[m_iSelectedSong]; };
	NotesType	GetSelectedNotesType()	{ return m_CurNotesType; };
	Notes*		GetSelectedNotes()		{ return m_pNotess[m_iSelectedNotes]; };
	
	void MenuUp( const PlayerNumber p );
	void MenuDown( const PlayerNumber p );
	void MenuLeft( const PlayerNumber p );
	void MenuRight( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );


	enum SelectedRow { ROW_GROUP, ROW_SONG, ROW_NOTES_TYPE, ROW_STEPS, NUM_ROWS };
	SelectedRow m_SelectedRow;

	MenuElements m_Menu;

	CStringArray m_sGroups;
	int m_iSelectedGroup;		// index into m_sGroups
	BitmapText m_textGroup;

	CArray<Song*, Song*> m_pSongs;
	int m_iSelectedSong;		// index into m_pSongs
	Banner  m_Banner;
	TextBanner  m_TextBanner;
	Sprite  m_sprArrowLeft;
	Sprite  m_sprArrowRight;

	NotesType m_CurNotesType;	// index into enum GameMode
	BitmapText m_textNotesType;

	CArray<Notes*, Notes*> m_pNotess;
	int m_iSelectedNotes;		// index into m_pNotess
	BitmapText m_textNotes;


	BitmapText m_textExplanation;

	TransitionFade		m_Fade;

	RandomSample m_soundChangeMusic;
	RandomSample m_soundSelect;

};



