/*
-----------------------------------------------------------------------------
 File: ScreenEdit.h

 Desc: The music plays, the notes scroll, and the Player is pressing buttons.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
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


	enum EditMode { MODE_EDIT, MODE_RECORD, MODE_PLAY };
	EditMode m_Mode;

	Song*			m_pSong;

	PlayerOptions	m_PlayerOptions;

	Sprite			m_sprBackground;

	NoteField				m_NoteFieldEdit;
	GranularityIndicator	m_GranularityIndicator;
	GrayArrowRow			m_GrayArrowRowEdit;

	BitmapText				m_textExplanation;
	BitmapText				m_textInfo;		// status information that changes

	// keep track of where we are and what we're doing
	float			m_fTrailingBeat;	// this approaches m_fBeat
	float			m_fBeat;


	RandomSample		m_soundChangeLine;
	RandomSample		m_soundChangeSnap;
	RandomSample		m_soundMarker;
	RandomSample		m_soundInvalid;

	TransitionFade	m_Fade;


// for MODE_RECORD

	Quad			m_rectRecordBack;
	NoteField			m_NoteFieldRecord;
	GrayArrowRow				m_GrayArrowRowRecord;

// for MODE_PLAY

	Player					m_Player;

// for MODE_RECORD and MODE_PLAY

	RageSoundStream	m_soundMusic;
};



