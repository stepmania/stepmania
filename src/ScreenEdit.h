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
#include "NoteTypes.h"
#include "Song.h"
#include "Steps.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "GameInput.h"
#include "GameplayAssist.h"
#include "AutoKeysounds.h"
#include "EnumHelper.h"

#include <map>
#include <vector>

const int NUM_EDIT_BUTTON_COLUMNS = 10;
struct MenuDef;

/** @brief What is going on with the Editor? */
enum EditState
{
	STATE_EDITING, /**< The person is making adjustments to the Steps. */
	STATE_RECORDING, /**< The person is recording some Steps live. */
	STATE_RECORDING_PAUSED, /**< The person has temporarily paused the recording of Steps. */
	STATE_PLAYING, /**< The person is just trying out the Steps. */
	NUM_EditState,
	EditState_Invalid
};
const RString& EditStateToString( EditState es );
LuaDeclareType( EditState );

enum EditButton
{
	// Add to the name_to_edit_button list when adding to this enum. -Kyz
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

	// These are modifiers to EDIT_BUTTON_COLUMN_*.
	EDIT_BUTTON_RIGHT_SIDE,
	EDIT_BUTTON_LAY_ROLL,
	EDIT_BUTTON_LAY_TAP_ATTACK,
	EDIT_BUTTON_REMOVE_NOTE,

	// These are modifiers to change the present tap note.
	EDIT_BUTTON_CYCLE_TAP_LEFT, /**< Rotate the available tap notes once to the "left". */
	EDIT_BUTTON_CYCLE_TAP_RIGHT, /**< Rotate the available tap notes once to the "right". */

	EDIT_BUTTON_CYCLE_SEGMENT_LEFT, /**< Select one segment to the left for jumping. */
	EDIT_BUTTON_CYCLE_SEGMENT_RIGHT, /**< Select one segment to the right for jumping. */

	EDIT_BUTTON_SCROLL_UP_LINE,
	EDIT_BUTTON_SCROLL_UP_PAGE,
	EDIT_BUTTON_SCROLL_UP_TS,
	EDIT_BUTTON_SCROLL_DOWN_LINE,
	EDIT_BUTTON_SCROLL_DOWN_PAGE,
	EDIT_BUTTON_SCROLL_DOWN_TS,
	EDIT_BUTTON_SCROLL_NEXT_MEASURE,
	EDIT_BUTTON_SCROLL_PREV_MEASURE,
	EDIT_BUTTON_SCROLL_HOME,
	EDIT_BUTTON_SCROLL_END,
	EDIT_BUTTON_SCROLL_NEXT,
	EDIT_BUTTON_SCROLL_PREV,

	EDIT_BUTTON_SEGMENT_NEXT, /**< Jump to the start of the next segment downward. */
	EDIT_BUTTON_SEGMENT_PREV, /**< Jump to the start of the previous segment upward. */

	// These are modifiers to EDIT_BUTTON_SCROLL_*.
	EDIT_BUTTON_SCROLL_SELECT,

	EDIT_BUTTON_LAY_SELECT,

	EDIT_BUTTON_SCROLL_SPEED_UP,
	EDIT_BUTTON_SCROLL_SPEED_DOWN,

	EDIT_BUTTON_SNAP_NEXT,
	EDIT_BUTTON_SNAP_PREV,

	EDIT_BUTTON_OPEN_EDIT_MENU,
	EDIT_BUTTON_OPEN_TIMING_MENU,
	EDIT_BUTTON_OPEN_ALTER_MENU,
	EDIT_BUTTON_OPEN_AREA_MENU,
	EDIT_BUTTON_OPEN_BGCHANGE_LAYER1_MENU,
	EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU,
	EDIT_BUTTON_OPEN_COURSE_MENU,
	EDIT_BUTTON_OPEN_COURSE_ATTACK_MENU,

	EDIT_BUTTON_OPEN_STEP_ATTACK_MENU, /**< Open up the Step Attacks menu. */
	EDIT_BUTTON_ADD_STEP_MODS, /**< Add a mod attack to the row. */

	EDIT_BUTTON_OPEN_INPUT_HELP,

	EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP,
	EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE,

	EDIT_BUTTON_PLAY_FROM_START,
	EDIT_BUTTON_PLAY_FROM_CURSOR,
	EDIT_BUTTON_PLAY_SELECTION,
	EDIT_BUTTON_RECORD_FROM_CURSOR,
	EDIT_BUTTON_RECORD_SELECTION,

	EDIT_BUTTON_RECORD_HOLD_RESET,
	EDIT_BUTTON_RECORD_HOLD_OFF,
	EDIT_BUTTON_RECORD_HOLD_MORE,
	EDIT_BUTTON_RECORD_HOLD_LESS,

	EDIT_BUTTON_RETURN_TO_EDIT,

	EDIT_BUTTON_INSERT,
	EDIT_BUTTON_DELETE,
	EDIT_BUTTON_INSERT_SHIFT_PAUSES,
	EDIT_BUTTON_DELETE_SHIFT_PAUSES,

	EDIT_BUTTON_OPEN_NEXT_STEPS,
	EDIT_BUTTON_OPEN_PREV_STEPS,
	EDIT_BUTTON_PLAY_SAMPLE_MUSIC,

	EDIT_BUTTON_BPM_UP,
	EDIT_BUTTON_BPM_DOWN,
	EDIT_BUTTON_STOP_UP,
	EDIT_BUTTON_STOP_DOWN,

	EDIT_BUTTON_DELAY_UP,
	EDIT_BUTTON_DELAY_DOWN,

	EDIT_BUTTON_OFFSET_UP,
	EDIT_BUTTON_OFFSET_DOWN,
	EDIT_BUTTON_SAMPLE_START_UP,
	EDIT_BUTTON_SAMPLE_START_DOWN,
	EDIT_BUTTON_SAMPLE_LENGTH_UP,
	EDIT_BUTTON_SAMPLE_LENGTH_DOWN,

	EDIT_BUTTON_ADJUST_FINE, /**< This button modifies offset, BPM, and stop segment changes. */

	EDIT_BUTTON_SAVE, /**< Save the present changes into the chart. */

	EDIT_BUTTON_UNDO, /**< Undo a recent change. */

	EDIT_BUTTON_ADD_COURSE_MODS,

	EDIT_BUTTON_SWITCH_PLAYERS, /**< Allow entering notes for a different Player. */

	EDIT_BUTTON_SWITCH_TIMINGS, /**< Allow switching between Song and Step TimingData. */

	// Add to the name_to_edit_button list when adding to this enum. -Kyz
	NUM_EditButton, // leave this at the end
	EditButton_Invalid
};
/** @brief A special foreach loop for the different edit buttons. */
#define FOREACH_EditButton( e ) FOREACH_ENUM( EditButton, e )
const int NUM_EDIT_TO_DEVICE_SLOTS = 2;
const int NUM_EDIT_TO_MENU_SLOTS = 2;

/* g_MapEditToDI is a simple mapping: edit functions map to DeviceInputs.
 * If g_MapEditToDIHold for a given edit function is valid, then at least one
 * input in g_MapEditToDIHold must be held when pressing any key in g_MapEditToDI
 * for the input to occur. */
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

/**
 * @brief This is similar to MapEditToDI,
 * but maps GameButton instead of DeviceInput. */
struct MapEditButtonToMenuButton
{
	GameButton button[NUM_EditButton][NUM_EDIT_TO_MENU_SLOTS];
	void Clear()
	{
		FOREACH_EditButton(e)
			for( int slot = 0; slot < NUM_EDIT_TO_MENU_SLOTS; ++slot )
				button[e][slot] = GameButton_Invalid;
	}
};
/** @brief Edit, record, playback, and synchronize notes. */
class ScreenEdit : public ScreenWithMenuElements
{
public:
	virtual void Init();
	virtual void BeginScreen();
	virtual void EndScreen();

	virtual ~ScreenEdit();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual bool Input( const InputEventPlus &input );
	bool InputEdit( const InputEventPlus &input, EditButton EditB );
	bool InputRecord( const InputEventPlus &input, EditButton EditB );
	bool InputRecordPaused( const InputEventPlus &input, EditButton EditB );
	bool InputPlay( const InputEventPlus &input, EditButton EditB );
	virtual void HandleMessage( const Message &msg );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void SetDirty(bool dirty);
	bool IsDirty() const { return m_dirty; }

	void PerformSave(bool autosave);

	EditState GetEditState(){ return m_EditState; }

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	virtual ScreenType GetScreenType() const
	{
		return m_EditState==STATE_PLAYING ?
		gameplay :
		ScreenWithMenuElements::GetScreenType();
	}

	void TransitionEditState( EditState em );
	void ScrollTo( float fDestinationBeat );
	void PlayTicks();
	void PlayPreviewMusic();

	// Call this before modifying m_NoteDataEdit.
	void SaveUndo();
	/** @brief Revert the last change made to m_NoteDataEdit. */
	void Undo();
	/** @brief Remove the previously stored NoteData to prevent undoing. */
	void ClearUndo();
	/**
	 * @brief This is to be called after modifying m_NoteDataEdit.
	 *
	 * It will Undo itself if MAX_NOTES_PER_MEASURE was exceeded. */
	void CheckNumberOfNotesAndUndo();

	void OnSnapModeChange();

	float GetMaximumBeatForNewNote() const;	// don't allow Down key to go past this beat.
	float GetMaximumBeatForMoving() const;	// don't allow Down key to go past this beat.

	/** @brief Display the keyboard track menu for the current row. */
	void DoKeyboardTrackMenu();

	/** @brief Display the step attack menu for the current row. */
	void DoStepAttackMenu();

	void DoHelp();

	/** @brief Display the TimingData menu for editing song and step timing. */
	void DisplayTimingMenu();

	enum TimingChangeMenuPurpose
	{
		menu_is_for_copying,
		menu_is_for_shifting,
		menu_is_for_clearing
	};
	TimingChangeMenuPurpose m_timing_change_menu_purpose;
	int m_timing_rows_being_shitted; // How far the timing is being shitted.
	void DisplayTimingChangeMenu();

	EditState		m_EditState;

	Song*			m_pSong;
	Steps*			m_pSteps;

	PlayerNumber	m_InputPlayerNumber;
	PlayerState		m_PlayerStateEdit;
	NoteField		m_NoteFieldEdit;
	NoteData		m_NoteDataEdit;
	SnapDisplay		m_SnapDisplay;

	BitmapText		m_textInputTips;

	/** @brief The player options before messing with attacks. */
	ModsGroup<PlayerOptions>	originalPlayerOptions;

	/**
	 * @brief Keep a backup of the present Step TimingData when
	 * entering a playing or recording state.
	 *
	 * This is mainly to allow playing a chart with Song Timing. */
	TimingData		backupStepTiming;

	/**
	 * @brief Allow for copying and pasting a song's (or steps's) full Timing Data.
	 */
	TimingData		clipboardFullTiming;

	/** @brief The current TapNote that would be inserted. */
	TapNote			m_selectedTap;

	/** @brief The type of segment users will jump back and forth between. */
	TimingSegmentType	currentCycleSegment;

	void UpdateTextInfo();
	BitmapText		m_textInfo; // status information that changes
	bool			m_bTextInfoNeedsUpdate;

	BitmapText		m_textPlayRecordHelp;

	// keep track of where we are and what we're doing
	float			m_fTrailingBeat;
	// the above approaches GAMESTATE->m_fSongBeat, which is the actual beat
	/**
	 * @brief The location we were at when shift was pressed.
	 *
	 * If shift wasn't pressed, this will be -1. */
	int			m_iShiftAnchor;

	/** @brief The NoteData that has been cut or copied. */
	NoteData		m_Clipboard;
	bool    		m_bHasUndo;
	/**
	 * @brief The NoteData as it once just one action prior.
	 *
	 * TODO: Convert this into a stack or vector of NoteData to allow multiple undos. -aj
	 * TODO: Look into a redo option. -aj */
	NoteData		m_Undo;

	/** @brief Has the NoteData been changed such that a user should be prompted to save? */
	bool			m_dirty;
	float m_next_autosave_time;

	/** @brief The sound that is played when a note is added. */
	RageSound		m_soundAddNote;
	/** @brief The sound that is played when a note is removed. */
	RageSound		m_soundRemoveNote;
	RageSound		m_soundChangeLine;
	RageSound		m_soundChangeSnap;
	RageSound		m_soundMarker;
	RageSound		m_soundValueIncrease;
	RageSound		m_soundValueDecrease;
	/** @brief The sound that is played when switching players for Routine. */
	RageSound		m_soundSwitchPlayer;
	/** @brief The sound that is played when switching song/step timing. */
	RageSound		m_soundSwitchTiming;
	/** @brief The sound that is played when switching to a different chart. */
	RageSound		m_soundSwitchSteps;
	/** @brief The sound that is played when the chart is saved. */
	RageSound		m_soundSave;

	// used for reverting
	void CopyToLastSave();
	void CopyFromLastSave();
	void RevertFromDisk();

	Song m_SongLastSave;
	std::vector<Steps> m_vStepsLastSave;


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
	int				m_iStartPlayingAt, m_iStopPlayingAt;
	float			m_fBeatToReturnTo;

	AutoKeysounds	m_AutoKeysounds;
	RageSound		*m_pSoundMusic;
	GameplayAssist	m_GameplayAssist;

	ThemeMetric<EditMode> EDIT_MODE;

public:
	/** @brief What are the choices that one can make on the main menu? */
	enum MainMenuChoice
	{
		play_selection,
		set_selection_start,
		set_selection_end,
		edit_steps_information,
		play_whole_song, /**< Play the entire chart from the beginning. */
		play_selection_start_to_end,
		play_current_beat_to_end,
		save, /**< Save the current chart to disk. */
		revert_to_last_save,
		revert_from_disk,
		options, /**< Modify the PlayerOptions and SongOptions. */
		edit_song_info, /**< Edit some general information about the song. */
		edit_timing_data, /**< Edit the chart's timing data. */
		view_steps_data, /**< View step statistics. */
		play_preview_music, /**< Play the song's preview music. */
		exit,
		save_on_exit,
		NUM_MAIN_MENU_CHOICES,
		MAIN_MENU_CHOICE_INVALID
	};
	int GetSongOrNotesEnd();
	void HandleMainMenuChoice( MainMenuChoice c, const std::vector<int> &iAnswers );
	void HandleMainMenuChoice( MainMenuChoice c ) { const std::vector<int> v; HandleMainMenuChoice( c, v ); }
	MainMenuChoice m_CurrentAction;

	/** @brief How does one alter a selection of NoteData? */
	enum AlterMenuChoice
	{
		cut, /**< Cut the notes. */
		copy, /**< Copy the notes. */
		clear, /**< Erase the notes, without putting them in the clipboard. */
		quantize, /**< Sync the notes to an exact level. */
		turn, /**< Rotate the notes. */
		transform, /**< Activate a specific mod. */
		alter, /**< Perform other transformations. */
		tempo, /**< Modify the tempo of the notes. */
		play, /**< Play the notes in the range. */
		record, /**< Record new notes in the range. */
		preview_designation, /**< Set the area as the music preview. */
		convert_to_pause, /**< Convert the range into a StopSegment. */
		convert_to_delay, /**< Convert the range into a DelaySegment. */
		convert_to_warp, /**< Convert the range into a WarpSegment. */
		convert_to_fake, /**< Convert the range into a FakeSegment. */
		convert_to_attack, /**< Convert the range into an Attack. */
		routine_invert_notes, /**< Switch which player hits the note. */
		routine_mirror_1_to_2, /**< Mirror Player 1's notes for Player 2. */
		routine_mirror_2_to_1, /**< Mirror Player 2's notes for Player 1. */
		NUM_ALTER_MENU_CHOICES

	};

	enum AreaMenuChoice
	{
		paste_at_current_beat, /**< Paste note data starting at the current beat. */
		paste_at_begin_marker, /**< Paste note data starting at the first market. */
		insert_and_shift,
		delete_and_shift,
		shift_pauses_forward, /**< Shift all timing changes forward one beat. */
		shift_pauses_backward, /**< Shift all timing changes backward one beat. */
		convert_pause_to_beat,
		convert_delay_to_beat,
		last_second_at_beat,
		undo,
		clear_clipboard, /**< Clear the clipboards. */
		modify_attacks_at_row, /**< Modify the chart attacks at this row. */
		modify_keysounds_at_row, /**< Modify the keysounds at this row. */
		NUM_AREA_MENU_CHOICES
	};
	void HandleArbitraryRemapping(RString const& mapstr);
	void HandleAlterMenuChoice(AlterMenuChoice choice,
		const std::vector<int> &answers, bool allow_undo= true,
		bool prompt_clear= true);
	void HandleAlterMenuChoice(AlterMenuChoice c, bool allow_undo= true,
		bool prompt_clear= true)
	{
		const std::vector<int> v; HandleAlterMenuChoice(c, v, allow_undo, prompt_clear);
	}

	void HandleAreaMenuChoice( AreaMenuChoice c, const std::vector<int> &iAnswers, bool bAllowUndo = true );
	void HandleAreaMenuChoice( AreaMenuChoice c, bool bAllowUndo = true )
	{
		const std::vector<int> v; HandleAreaMenuChoice( c, v, bAllowUndo );
	}
	/** @brief How should the selected notes be transformed? */
	enum TurnType
	{
		left, /**< Turn the notes as if you were facing to the left. */
		right, /**< Turn the notes as if you were facing to the right. */
		mirror, /**< Flip the notes vertically. */
		lrmirror, /**< Swap the left and right columns. */
		udmirror, /**< Swap the up and down columns. */
		turn_backwards, /**< Turn the notes as if you were facing away from the machine. */
		shuffle, /**< Replace one column with another column. */
		super_shuffle, /**< Replace each note individually. */
		hyper_shuffle, /**< Replace each note individually but more fairly. */
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
		swap_up_down,
		arbitrary_remap,
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
		difficulty, /**< What is the difficulty of this chart? */
		meter, /**< What is the numerical rating of this chart? */
		predict_meter, /**< What does the game think this chart's rating should be? */
		chartname, /**< What is the name of this chart? */
		description, /**< What is the description of this chart? */
		chartstyle, /**< How is this chart meant to be played? */
		step_credit, /**< Who wrote this individual chart? */
		step_display_bpm,
		step_min_bpm,
		step_max_bpm,
		step_music,
		NUM_STEPS_INFORMATION_CHOICES
	};
	void HandleStepsInformationChoice( StepsInformationChoice c, const std::vector<int> &iAnswers );

	enum StepsDataChoice
	{
		tap_notes,
		jumps,
		hands,
		quads,
		holds,
		mines,
		rolls,
		lifts,
		fakes,
		stream,
		voltage,
		air,
		freeze,
		chaos,
		NUM_STEPS_DATA_CHOICES
	};
	void HandleStepsDataChoice(StepsDataChoice c, const std::vector<int> &answers);

	enum SongInformationChoice
	{
		main_title,
		sub_title,
		artist,
		genre,
		credit,
		preview,
		main_title_transliteration,
		sub_title_transliteration,
		artist_transliteration,
		last_second_hint,
		preview_start,
		preview_length,
		display_bpm,
		min_bpm,
		max_bpm,
		NUM_SONG_INFORMATION_CHOICES
	};
	void HandleSongInformationChoice( SongInformationChoice c, const std::vector<int> &iAnswers );

	enum TimingDataInformationChoice
	{
		beat_0_offset,
		bpm,
		stop,
		delay,
//		time_signature,
		label,
		tickcount,
		combo,
		warp,
//		speed,
		speed_percent,
		speed_wait,
		speed_mode,
		scroll,
		fake,
		shift_timing_in_region_down,
		shift_timing_in_region_up,
		copy_timing_in_region,
		clear_timing_in_region,
		paste_timing_from_clip,
		copy_full_timing,
		paste_full_timing,
		erase_step_timing,
		NUM_TIMING_DATA_INFORMATION_CHOICES
	};
	void HandleTimingDataInformationChoice (TimingDataInformationChoice c,
						const std::vector<int> &iAnswers );

	enum TimingDataChangeChoice
	{
		timing_all,
		timing_bpm,
		timing_stop,
		timing_delay,
		timing_time_sig,
		timing_warp,
		timing_label,
		timing_tickcount,
		timing_combo,
		timing_speed,
		timing_scroll,
		timing_fake,
		NUM_TimingDataChangeChoices
	};
	void HandleTimingDataChangeChoice(TimingDataChangeChoice choice,
		const std::vector<int>& answers);

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

	enum SpeedSegmentModes
	{
		SSMODE_Beats,
		SSMODE_Seconds
	};

	/**
	 * @brief Take care of any background changes that the user wants.
	 *
	 * It is important that this is only called in Song Timing mode.
	 * @param c the Background Change style requested.
	 * @param iAnswers the other settings involving the change. */
	void HandleBGChangeChoice( BGChangeChoice c, const std::vector<int> &iAnswers );

	enum CourseAttackChoice
	{
		duration,
		set_mods,
		remove,
		NUM_CourseAttackChoice
	};

	enum StepAttackChoice
	{
		sa_duration,
		sa_set_mods,
		sa_remove,
		NUM_StepAttackChoice
	};

	void InitEditMappings();
	EditButton DeviceToEdit( const DeviceInput &DeviceI ) const;
	EditButton MenuButtonToEditButton( GameButton MenuI ) const;
	bool EditToDevice( EditButton button, int iSlotNum, DeviceInput &DeviceI ) const;
	bool EditPressed( EditButton button, const DeviceInput &DeviceI );
	bool EditIsBeingPressed( EditButton button ) const;
	const MapEditToDI *GetCurrentDeviceInputMap() const;
	const MapEditButtonToMenuButton *GetCurrentMenuButtonMap() const;
	void LoadKeymapSectionIntoMappingsMember(XNode const* section, MapEditToDI& mappings);
	MapEditToDI		m_EditMappingsDeviceInput;
	MapEditToDI		m_PlayMappingsDeviceInput;
	MapEditToDI		m_RecordMappingsDeviceInput;
	MapEditToDI		m_RecordPausedMappingsDeviceInput;
	MapEditButtonToMenuButton m_EditMappingsMenuButton;
	MapEditButtonToMenuButton m_PlayMappingsMenuButton;
	MapEditButtonToMenuButton m_RecordMappingsMenuButton;
	MapEditButtonToMenuButton m_RecordPausedMappingsMenuButton;

	void MakeFilteredMenuDef( const MenuDef* pDef, MenuDef &menu );
	void EditMiniMenu(const MenuDef* pDef,
			  ScreenMessage SM_SendOnOK = SM_None,
			  ScreenMessage SM_SendOnCancel = SM_None );

	int attackInProcess;
	int modInProcess;
private:
	/**
	 * @brief Retrieve the appropriate TimingData based on GAMESTATE.
	 * @return the proper TimingData. */
	const TimingData & GetAppropriateTiming() const;
	/**
	 * @brief Retrieve the appropriate TimingData to use for updating. */
	TimingData & GetAppropriateTimingForUpdate();
	/**
	 * @brief Retrieve the appropriate SongPosition data based on GAMESTATE.
	 * @return the proper SongPosition. */
	SongPosition & GetAppropriatePosition() const;
	void SetBeat(float fBeat);
	float GetBeat();
	int GetRow();

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
