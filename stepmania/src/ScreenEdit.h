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
#include "GranularityIndicator.h"


class ScreenEdit : public Screen
{
public:
	ScreenEdit();
	virtual ~ScreenEdit();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void InputEdit( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void InputRecord( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void InputPlay( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	void TransitionToEditFromRecord();


protected:
	void OnSnapModeChange();


	enum EditMode { MODE_EDITING, MODE_RECORDING, MODE_PLAYING };
	EditMode m_EditMode;

	Song*			m_pSong;
	Notes*			m_pNotes;

	Sprite			m_sprBackground;

	NoteField				m_NoteFieldEdit;
	GranularityIndicator	m_GranularityIndicator;
	GrayArrowRow			m_GrayArrowRowEdit;

	BitmapText				m_textInfo;		// status information that changes
	BitmapText				m_textHelp;

	// keep track of where we are and what we're doing
	float				m_fBeat;	// make GAMESTATE->m_fSongBeat approach this

	NoteData			m_Clipboard;

	RageSoundSample		m_soundChangeLine;
	RageSoundSample		m_soundChangeSnap;
	RageSoundSample		m_soundMarker;
	RageSoundSample		m_soundInvalid;

	TransitionFade		m_Fade;


// for MODE_RECORD

	Quad			m_rectRecordBack;
	NoteField		m_NoteFieldRecord;
	GrayArrowRow	m_GrayArrowRowRecord;

// for MODE_PLAY

	Player					m_Player;

// for MODE_RECORD and MODE_PLAY

	RageSoundStream	m_soundMusic;
};



