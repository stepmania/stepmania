/*
-----------------------------------------------------------------------------
 Class: ScreenEdit

 Desc: Edit, record, playback, or synchronize notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "TransitionFade.h"
#include "BitmapText.h"
#include "Player.h"
#include "RandomSample.h"
#include "FocusingSprite.h"
#include "RageMusic.h"
#include "MotionBlurSprite.h"
#include "Background.h"
#include "SnapDisplay.h"


const int NUM_MENU_ITEMS = 19;


class ScreenEdit : public Screen
{
public:
	ScreenEdit();
	virtual ~ScreenEdit();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void InputEdit( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void InputMenu( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void InputRecord( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void InputPlay( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	void TransitionFromRecordToEdit();


protected:
	void OnSnapModeChange();
	void MenuItemGainFocus( int iItemIndex );
	void MenuItemLoseFocus( int iItemIndex );

	enum EditMode { MODE_EDITING, MODE_MENU, MODE_RECORDING, MODE_PLAYING };
	EditMode m_EditMode;

	Song*			m_pSong;
	Notes*			m_pNotes;

	Sprite			m_sprBackground;

	NoteField				m_NoteFieldEdit;
	SnapDisplay				m_SnapDisplay;
	GrayArrowRow			m_GrayArrowRowEdit;

	BitmapText				m_textInfo;		// status information that changes
	BitmapText				m_textHelp;
	BitmapText				m_textShortcuts;

	// keep track of where we are and what we're doing
	float				m_fTrailingBeat;	// this approaches GAMESTATE->m_fSongBeat, which is the actual beat

	NoteData			m_Clipboard;

	RageSoundSample		m_soundChangeLine;
	RageSoundSample		m_soundChangeSnap;
	RageSoundSample		m_soundMarker;
	RageSoundSample		m_soundInvalid;

	TransitionFade		m_Fade;


// for MODE_RECORD

	NoteField		m_NoteFieldRecord;
	GrayArrowRow	m_GrayArrowRowRecord;

// for MODE_PLAY

	Player			m_Player;

// for MODE_RECORD and MODE_PLAY

	Quad			m_rectRecordBack;
	RageSoundStream	m_soundMusic;

	int				m_iMenuSelection;
	BitmapText		m_textMenu[NUM_MENU_ITEMS];
};



