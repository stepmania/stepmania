/* ScreenEdit - Edit, record, playback, and synchronize notes. */

#ifndef SCREEN_EDIT_H
#define SCREEN_EDIT_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "Player.h"
#include "RageSound.h"
#include "SnapDisplay.h"
#include "Background.h"
#include "Foreground.h"
#include "NoteField.h"
#include "song.h"
#include "Steps.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "MenuInput.h"

#include <map>

const int NUM_EDIT_BUTTON_COLUMNS = 10;
struct MenuDef;

enum EditState
{
	STATE_EDITING,
	STATE_RECORDING,
	STATE_RECORDING_PAUSED,
	STATE_PLAYING,
	NUM_EditState,
	EditState_Invalid
};

enum EditButton
{
	EDIT_BUTTON_COLUMN_0,
	EDIT_BUTTON_COLUMN_1,
	EDIT_BUTTON_COLUMN_2,
	EDIT_BUTTON_COLUMN_3,
	EDIT_BUTTON_COLUMN_4,
	EDIT_BUTTON_COLUMN_5,
	EDIT_BUTTON_COLUMN_6,
	EDIT_BUTTON_COLUMN_7,
	EDIT_BUTTON_COLUMN_8,
	EDIT_BUTTON_COLUMN_9,

	/* These are modifiers to EDIT_BUTTON_COLUMN_*. */
	EDIT_BUTTON_RIGHT_SIDE,
	EDIT_BUTTON_LAY_MINE_OR_ROLL,
	EDIT_BUTTON_LAY_TAP_ATTACK,
	EDIT_BUTTON_LAY_LIFT,
	EDIT_BUTTON_REMOVE_NOTE,

	EDIT_BUTTON_SCROLL_UP_LINE,
	EDIT_BUTTON_SCROLL_UP_PAGE,
	EDIT_BUTTON_SCROLL_DOWN_LINE,
	EDIT_BUTTON_SCROLL_DOWN_PAGE,
	EDIT_BUTTON_SCROLL_NEXT_MEASURE,
	EDIT_BUTTON_SCROLL_PREV_MEASURE,
	EDIT_BUTTON_SCROLL_HOME,
	EDIT_BUTTON_SCROLL_END,
	EDIT_BUTTON_SCROLL_NEXT,
	EDIT_BUTTON_SCROLL_PREV,

	/* These are modifiers to EDIT_BUTTON_SCROLL_*. */
	EDIT_BUTTON_SCROLL_SELECT,

	EDIT_BUTTON_LAY_SELECT,

	EDIT_BUTTON_SCROLL_SPEED_UP,
	EDIT_BUTTON_SCROLL_SPEED_DOWN,

	EDIT_BUTTON_SNAP_NEXT,
	EDIT_BUTTON_SNAP_PREV,

	EDIT_BUTTON_OPEN_EDIT_MENU,
	EDIT_BUTTON_OPEN_AREA_MENU,
	EDIT_BUTTON_OPEN_BGCHANGE_LAYER1_MENU,
	EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU,
	EDIT_BUTTON_OPEN_COURSE_MENU,
	EDIT_BUTTON_OPEN_COURSE_ATTACK_MENU,
	EDIT_BUTTON_OPEN_INPUT_HELP,

	EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP,
	EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE,

	EDIT_BUTTON_PLAY_FROM_START,
	EDIT_BUTTON_PLAY_FROM_CURSOR,
	EDIT_BUTTON_PLAY_SELECTION,
	EDIT_BUTTON_RECORD_FROM_CURSOR,
	EDIT_BUTTON_RECORD_SELECTION,

	EDIT_BUTTON_RETURN_TO_EDIT,

	EDIT_BUTTON_INSERT,
	EDIT_BUTTON_DELETE,
	EDIT_BUTTON_INSERT_SHIFT_PAUSES,
	EDIT_BUTTON_DELETE_SHIFT_PAUSES,

	EDIT_BUTTON_OPEN_NEXT_STEPS,
	EDIT_BUTTON_OPEN_PREV_STEPS,
	EDIT_BUTTON_TOGGLE_ASSIST_TICK,
	EDIT_BUTTON_TOGGLE_AUTOPLAY,
	EDIT_BUTTON_PLAY_SAMPLE_MUSIC,

	EDIT_BUTTON_BPM_UP,
	EDIT_BUTTON_BPM_DOWN,
	EDIT_BUTTON_STOP_UP,
	EDIT_BUTTON_STOP_DOWN,
	EDIT_BUTTON_OFFSET_UP,
	EDIT_BUTTON_OFFSET_DOWN,
	EDIT_BUTTON_SAMPLE_START_UP,
	EDIT_BUTTON_SAMPLE_START_DOWN,
	EDIT_BUTTON_SAMPLE_LENGTH_UP,
	EDIT_BUTTON_SAMPLE_LENGTH_DOWN,

	/* This modifies offset, BPM, and stop segment changes. */
	EDIT_BUTTON_ADJUST_FINE,

	EDIT_BUTTON_UNDO,
	
	EDIT_BUTTON_ADD_COURSE_MODS,
	
	EDIT_BUTTON_SWITCH_PLAYERS,

	NUM_EditButton,		// leave this at the end
	EditButton_Invalid
};
#define FOREACH_EditButton( e ) FOREACH_ENUM( EditButton, e )
const int NUM_EDIT_TO_DEVICE_SLOTS = 2;
const int NUM_EDIT_TO_MENU_SLOTS = 2;

/*
 * g_MapEditToDI is a simple mapping: edit functions map to DeviceInputs.
 * If g_MapEditToDIHold for a given edit function is valid, then at least one
 * input in g_MapEditToDIHold must be held when pressing any key in g_MapEditToDI
 * for the input to occur.
 */
struct MapEditToDI
{
	DeviceInput button[NUM_EditButton][NUM_EDIT_TO_DEVICE_SLOTS];
	DeviceInput hold[NUM_EditButton][NUM_EDIT_TO_DEVICE_SLOTS];
	void Clear()
	{
		FOREACH_EditButton(e)
			for( int slot = 0; slot < NUM_EDIT_TO_DEVICE_SLOTS; ++slot )
			{
				button[e][slot].MakeInvalid();
				hold[e][slot].MakeInvalid();
			}
	}
};

// Like MapEditToDI, but maps MenuButton instead of DeviceInput.
struct MapEditButtonToMenuButton
{
	MenuButton button[NUM_EditButton][NUM_EDIT_TO_MENU_SLOTS];
	void Clear()
	{
		FOREACH_EditButton(e)
			for( int slot = 0; slot < NUM_EDIT_TO_MENU_SLOTS; ++slot )
				button[e][slot] = MenuButton_Invalid;
	}
};

class ScreenEdit : public ScreenWithMenuElements
{
public:
	virtual void Init();
	virtual void BeginScreen();
	virtual void EndScreen();

	virtual ~ScreenEdit();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const InputEventPlus &input );
	void InputEdit( const InputEventPlus &input, EditButton EditB );
	void InputRecord( const InputEventPlus &input, EditButton EditB );
	void InputRecordPaused( const InputEventPlus &input, EditButton EditB );
	void InputPlay( const InputEventPlus &input, EditButton EditB );
	virtual void HandleMessage( const Message &msg );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void SetDirty( bool bDirty )	{ m_bDirty = bDirty; }
	bool IsDirty() const		{ return m_bDirty; }

protected:
	virtual ScreenType GetScreenType() const { return m_EditState==STATE_PLAYING ? gameplay : ScreenWithMenuElements::GetScreenType(); }

	void TransitionEditState( EditState em );
	void ScrollTo( float fDestinationBeat );
	void PlayTicks();
	void PlayPreviewMusic();
	
	// Call this before modifying m_NoteDataEdit.
	void SaveUndo();
	// Revert m_NoteDataEdit using m_Undo.
	void Undo();
	void ClearUndo();
	// Call this after modifying m_NoteDataEdit.  It will Undo() if 
	// MAX_NOTES_PER_MEASURE was exceeded.
	void CheckNumberOfNotesAndUndo();

	void OnSnapModeChange();

	float GetMaximumBeatForNewNote() const;	// don't allow Down key to go past this beat.
	float GetMaximumBeatForMoving() const;	// don't allow Down key to go past this beat.

	void DoHelp();

	EditState		m_EditState;

	Song*			m_pSong;
	Steps*			m_pSteps;

	PlayerNumber		m_InputPlayerNumber;
	PlayerState		m_PlayerStateEdit;
	NoteField		m_NoteFieldEdit;
	NoteData		m_NoteDataEdit;
	SnapDisplay		m_SnapDisplay;

	BitmapText		m_textInputTips;

	void UpdateTextInfo();
	BitmapText		m_textInfo;		// status information that changes
	bool			m_bTextInfoNeedsUpdate;

	BitmapText		m_textPlayRecordHelp;

	// keep track of where we are and what we're doing
	float			m_fTrailingBeat;	// this approaches GAMESTATE->m_fSongBeat, which is the actual beat
	/* The location we were at when shift was pressed, or
	 * -1 when shift isn't pressed: */
	int			m_iShiftAnchor;

	NoteData		m_Clipboard;
	bool    		m_bHasUndo;
	NoteData		m_Undo;

	bool			m_bDirty;

	RageSound		m_soundAddNote;
	RageSound		m_soundRemoveNote;
	RageSound		m_soundChangeLine;
	RageSound		m_soundChangeSnap;
	RageSound		m_soundMarker;
	RageSound		m_soundValueIncrease;
	RageSound		m_soundValueDecrease;
	RageSound		m_soundSwitchSteps;
	RageSound		m_soundSave;

	// used for reverting
	void CopyToLastSave();
	void CopyFromLastSave();
	void RevertFromDisk();

	Song m_SongLastSave;
	map< Difficulty, Steps > m_mStepsLastSave;


// for MODE_RECORD

	NoteField		m_NoteFieldRecord;
	NoteData		m_NoteDataRecord;
	RageTimer		m_RemoveNoteButtonLastChanged;
	bool			m_bRemoveNoteButtonDown;

// for MODE_PLAY
	void SetupCourseAttacks();
	PlayerPlus		m_Player;
	Background		m_Background;
	Foreground		m_Foreground;
	bool			m_bReturnToRecordMenuAfterPlay;

// for MODE_RECORD and MODE_PLAY
	int			m_iStartPlayingAt, m_iStopPlayingAt;
	float			m_fBeatToReturnTo;

	RageSound		m_soundMusic;
	RageSound		m_soundAssistTick;

	ThemeMetric<EditMode> EDIT_MODE;

public:
	enum MainMenuChoice
	{
		play_selection,
		set_selection_start,
		set_selection_end,
		edit_steps_information,
		play_whole_song,
		play_selection_start_to_end,
		play_current_beat_to_end,
		save,
		revert_to_last_save,
		revert_from_disk,
		options,
		edit_song_info,
		edit_bpm,
		edit_stop,
		play_preview_music,
		exit,
		save_on_exit,
		NUM_MAIN_MENU_CHOICES,
		MAIN_MENU_CHOICE_INVALID
	};
	void HandleMainMenuChoice( MainMenuChoice c, const vector<int> &iAnswers );
	void HandleMainMenuChoice( MainMenuChoice c ) { const vector<int> v; HandleMainMenuChoice( c, v ); }
	MainMenuChoice m_CurrentAction;

	enum AreaMenuChoice
	{
		cut,
		copy,
		paste_at_current_beat,
		paste_at_begin_marker,
		clear,
		quantize,
		turn,
		transform,
		alter,
		tempo,
		play,
		record,
		insert_and_shift,
		delete_and_shift,
		shift_pauses_forward,
		shift_pauses_backward,
		convert_to_pause,
		convert_pause_to_beat,
		undo,
		NUM_AREA_MENU_CHOICES
	};
	void HandleAreaMenuChoice( AreaMenuChoice c, const vector<int> &iAnswers, bool bAllowUndo = true );
	void HandleAreaMenuChoice( AreaMenuChoice c, bool bAllowUndo = true ) { const vector<int> v; HandleAreaMenuChoice( c, v, bAllowUndo ); }
	enum TurnType
	{
		left,
		right,
		mirror,
		shuffle, 
		super_shuffle, 
		NUM_TURN_TYPES 
	};
	enum TransformType
	{
		noholds, 
		nomines, 
		little, 
		wide, 
		big, 
		quick, 
		skippy, 
		add_mines, 
		echo, 
		stomp, 
		planted, 
		floored, 
		twister, 
		nojumps, 
		nohands, 
		noquads, 
		nostretch,
		NUM_TRANSFORM_TYPES 
	};
	enum AlterType 
	{
		autogen_to_fill_width,
		backwards, 
		swap_sides, 
		copy_left_to_right, 
		copy_right_to_left, 
		clear_left, 
		clear_right, 
		collapse_to_one, 
		collapse_left, 
		shift_left, 
		shift_right, 
		NUM_ALTER_TYPES 
	};
	enum TempoType 
	{ 
		compress_2x, 
		compress_3_2, 
		compress_4_3, 
		expand_4_3, 
		expand_3_2, 
		expand_2x, 
		NUM_TEMPO_TYPES 
	};

	enum StepsInformationChoice
	{
		difficulty,
		meter,
		description,
		predict_meter,
		tap_notes,
		jumps,
		hands,
		quads,
		holds,
		mines,
		stream,
		voltage,
		air,
		freeze,
		chaos,
		NUM_STEPS_INFORMATION_CHOICES
	};
	void HandleStepsInformationChoice( StepsInformationChoice c, const vector<int> &iAnswers );

	enum SongInformationChoice
	{
		main_title,
		sub_title,
		artist,
		credit,
		main_title_transliteration,
		sub_title_transliteration,
		artist_transliteration,
		last_beat_hint,
		NUM_SONG_INFORMATION_CHOICES
	};
	void HandleSongInformationChoice( SongInformationChoice c, const vector<int> &iAnswers );

	enum BGChangeChoice
	{
		layer,
		rate,
		transition,
		effect,
		color1,
		color2,
		file1_type,
		file1_song_bganimation,
		file1_song_movie,
		file1_song_still,
		file1_global_bganimation,
		file1_global_movie,
		file1_global_movie_song_group,
		file1_global_movie_song_group_and_genre,
		file2_type,
		file2_song_bganimation,
		file2_song_movie,
		file2_song_still,
		file2_global_bganimation,
		file2_global_movie,
		file2_global_movie_song_group,
		file2_global_movie_song_group_and_genre,
		delete_change,
		NUM_BGCHANGE_CHOICES
	};

	void HandleBGChangeChoice( BGChangeChoice c, const vector<int> &iAnswers );

	enum CourseAttackChoice
	{
		duration,
		set_mods,
		remove,
		NUM_CourseAttackChoice
	};

	void InitEditMappings();
	EditButton DeviceToEdit( const DeviceInput &DeviceI ) const;
	EditButton MenuButtonToEditButton( MenuButton MenuI ) const;
	bool EditToDevice( EditButton button, int iSlotNum, DeviceInput &DeviceI ) const;
	bool EditPressed( EditButton button, const DeviceInput &DeviceI );
	bool EditIsBeingPressed( EditButton button ) const;
	const MapEditToDI *GetCurrentDeviceInputMap() const;
	const MapEditButtonToMenuButton *GetCurrentMenuButtonMap() const;
	MapEditToDI		m_EditMappingsDeviceInput;
	MapEditToDI		m_PlayMappingsDeviceInput;
	MapEditToDI		m_RecordMappingsDeviceInput;
	MapEditToDI		m_RecordPausedMappingsDeviceInput;
	MapEditButtonToMenuButton m_EditMappingsMenuButton;
	MapEditButtonToMenuButton m_PlayMappingsMenuButton;
	MapEditButtonToMenuButton m_RecordMappingsMenuButton;
	MapEditButtonToMenuButton m_RecordPausedMappingsMenuButton;

	void MakeFilteredMenuDef( const MenuDef* pDef, MenuDef &menu );
	void EditMiniMenu( const MenuDef* pDef, ScreenMessage SM_SendOnOK = SM_None, ScreenMessage SM_SendOnCancel = SM_None );
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
