#include "global.h"
#include <utility>
#include <float.h>
#include "ScreenEdit.h"
#include "ActorUtil.h"
#include "AdjustSync.h"
#include "ArrowEffects.h"
#include "BackgroundUtil.h"
#include "CommonMetrics.h"
#include "GameManager.h"
#include "GamePreferences.h"
#include "GameSoundManager.h"
#include "GameState.h"
#include "InputEventPlus.h"
#include "InputMapper.h"
#include "LocalizedString.h"
#include "NoteDataUtil.h"
#include "NoteSkinManager.h"
#include "NoteTypes.h"
#include "NotesWriterSM.h"
#include "PrefsManager.h"
#include "RageSoundManager.h"
#include "RageSoundReader_FileReader.h"
#include "RageInput.h"
#include "RageLog.h"
#include "ScreenDimensions.h"
#include "ScreenManager.h"
#include "ScreenMiniMenu.h"
#include "ScreenPrompt.h"
#include "ScreenSaveSync.h"
#include "ScreenTextEntry.h"
#include "SongManager.h"
#include "SongUtil.h"
#include "SpecialFiles.h"
#include "StepsUtil.h"
#include "Style.h"
#include "ThemeManager.h"
#include "ThemeMetric.h"
#include "TimingData.h"
#include "Game.h"
#include "RageSoundReader.h"

static Preference<float> g_iDefaultRecordLength( "DefaultRecordLength", 4 );
static Preference<bool> g_bEditorShowBGChangesPlay( "EditorShowBGChangesPlay", true );

/** @brief How long must the button be held to generate a hold in record mode? */
const float record_hold_default= 0.3f;
float record_hold_seconds = record_hold_default;
const float time_between_autosave= 300.0f; // 5 minutes. -Kyz

#define PLAYER_X		(SCREEN_CENTER_X)
#define PLAYER_Y		(SCREEN_CENTER_Y)
#define PLAYER_HEIGHT		(360)
#define PLAYER_Y_STANDARD	(PLAYER_Y-((SCREEN_HEIGHT/480)*(PLAYER_HEIGHT/2)))

#define EDIT_X			(SCREEN_CENTER_X)
#define EDIT_Y			(PLAYER_Y)

#define RECORD_X		(SCREEN_CENTER_X)
#define RECORD_Y		(SCREEN_CENTER_Y)

#define PLAY_RECORD_HELP_TEXT	THEME->GetString(m_sName,"PlayRecordHelpText")
#define EDIT_HELP_TEXT		THEME->GetString(m_sName,"EditHelpText")

#define SET_MOD_SCREEN THEME->GetMetric("ScreenEdit", "SetModScreen")
#define OPTIONS_SCREEN THEME->GetMetric("ScreenEdit", "OptionsScreen")

AutoScreenMessage( SM_UpdateTextInfo );
AutoScreenMessage( SM_BackFromMainMenu );
AutoScreenMessage( SM_BackFromAreaMenu );
AutoScreenMessage( SM_BackFromAlterMenu );
AutoScreenMessage( SM_BackFromArbitraryRemap );
AutoScreenMessage( SM_BackFromStepsInformation );
AutoScreenMessage( SM_BackFromStepsData );
AutoScreenMessage( SM_BackFromOptions );
AutoScreenMessage( SM_BackFromSongInformation );
AutoScreenMessage( SM_BackFromBGChange );
AutoScreenMessage( SM_BackFromInsertTapAttack );
AutoScreenMessage( SM_BackFromInsertTapAttackPlayerOptions );
AutoScreenMessage( SM_BackFromAttackAtTime );
AutoScreenMessage( SM_BackFromInsertStepAttack );
AutoScreenMessage( SM_BackFromAddingModToExistingAttack );
AutoScreenMessage( SM_BackFromEditingModToExistingAttack );
AutoScreenMessage( SM_BackFromEditingAttackStart );
AutoScreenMessage( SM_BackFromEditingAttackLength );
AutoScreenMessage( SM_BackFromAddingAttackToChart );
AutoScreenMessage( SM_BackFromInsertStepAttackPlayerOptions );
AutoScreenMessage( SM_BackFromInsertCourseAttack );
AutoScreenMessage( SM_BackFromInsertCourseAttackPlayerOptions );
AutoScreenMessage( SM_BackFromCourseModeMenu );
AutoScreenMessage( SM_BackFromKeysoundTrack );
AutoScreenMessage( SM_BackFromNewKeysound );
AutoScreenMessage( SM_DoRevertToLastSave );
AutoScreenMessage( SM_DoRevertFromDisk );
AutoScreenMessage( SM_ConfirmClearArea );
AutoScreenMessage( SM_BackFromTimingDataInformation );
AutoScreenMessage(SM_BackFromTimingDataChangeInformation);
AutoScreenMessage( SM_BackFromDifficultyMeterChange );
AutoScreenMessage( SM_BackFromBeat0Change );
AutoScreenMessage( SM_BackFromBPMChange );
AutoScreenMessage( SM_BackFromStopChange );
AutoScreenMessage( SM_BackFromDelayChange );
AutoScreenMessage( SM_BackFromTickcountChange );
AutoScreenMessage( SM_BackFromComboChange );
AutoScreenMessage( SM_BackFromLabelChange );
AutoScreenMessage( SM_BackFromWarpChange );
AutoScreenMessage( SM_BackFromSpeedPercentChange );
AutoScreenMessage( SM_BackFromSpeedWaitChange );
AutoScreenMessage( SM_BackFromSpeedModeChange );
AutoScreenMessage( SM_BackFromScrollChange );
AutoScreenMessage( SM_BackFromFakeChange );
AutoScreenMessage( SM_BackFromStepMusicChange );
AutoScreenMessage( SM_DoEraseStepTiming );
AutoScreenMessage( SM_DoSaveAndExit );
AutoScreenMessage( SM_DoExit );
AutoScreenMessage( SM_AutoSaveSuccessful );
AutoScreenMessage( SM_SaveSuccessful );
AutoScreenMessage( SM_SaveSuccessNoSM );
AutoScreenMessage( SM_SaveFailed );

static const char *EditStateNames[] = {
	"Edit",
	"Record",
	"RecordPaused",
	"Playing"
};
XToString( EditState );
LuaXType( EditState );

map<RString, EditButton> name_to_edit_button;

void ScreenEdit::InitEditMappings()
{
	// Created courtesy of query replace regex.
	name_to_edit_button["COLUMN_0"]= EDIT_BUTTON_COLUMN_0;
	name_to_edit_button["COLUMN_1"]= EDIT_BUTTON_COLUMN_1;
	name_to_edit_button["COLUMN_2"]= EDIT_BUTTON_COLUMN_2;
	name_to_edit_button["COLUMN_3"]= EDIT_BUTTON_COLUMN_3;
	name_to_edit_button["COLUMN_4"]= EDIT_BUTTON_COLUMN_4;
	name_to_edit_button["COLUMN_5"]= EDIT_BUTTON_COLUMN_5;
	name_to_edit_button["COLUMN_6"]= EDIT_BUTTON_COLUMN_6;
	name_to_edit_button["COLUMN_7"]= EDIT_BUTTON_COLUMN_7;
	name_to_edit_button["COLUMN_8"]= EDIT_BUTTON_COLUMN_8;
	name_to_edit_button["COLUMN_9"]= EDIT_BUTTON_COLUMN_9;

	name_to_edit_button["RIGHT_SIDE"]= EDIT_BUTTON_RIGHT_SIDE;
	name_to_edit_button["LAY_ROLL"]= EDIT_BUTTON_LAY_ROLL;
	name_to_edit_button["LAY_TAP_ATTACK"]= EDIT_BUTTON_LAY_TAP_ATTACK;
	name_to_edit_button["REMOVE_NOTE"]= EDIT_BUTTON_REMOVE_NOTE;

	name_to_edit_button["CYCLE_TAP_LEFT"]= EDIT_BUTTON_CYCLE_TAP_LEFT;
	name_to_edit_button["CYCLE_TAP_RIGHT"]= EDIT_BUTTON_CYCLE_TAP_RIGHT;

	name_to_edit_button["CYCLE_SEGMENT_LEFT"]= EDIT_BUTTON_CYCLE_SEGMENT_LEFT;
	name_to_edit_button["CYCLE_SEGMENT_RIGHT"]= EDIT_BUTTON_CYCLE_SEGMENT_RIGHT;

	name_to_edit_button["SCROLL_UP_LINE"]= EDIT_BUTTON_SCROLL_UP_LINE;
	name_to_edit_button["SCROLL_UP_PAGE"]= EDIT_BUTTON_SCROLL_UP_PAGE;
	name_to_edit_button["SCROLL_UP_TS"]= EDIT_BUTTON_SCROLL_UP_TS;
	name_to_edit_button["SCROLL_DOWN_LINE"]= EDIT_BUTTON_SCROLL_DOWN_LINE;
	name_to_edit_button["SCROLL_DOWN_PAGE"]= EDIT_BUTTON_SCROLL_DOWN_PAGE;
	name_to_edit_button["SCROLL_DOWN_TS"]= EDIT_BUTTON_SCROLL_DOWN_TS;
	name_to_edit_button["SCROLL_NEXT_MEASURE"]= EDIT_BUTTON_SCROLL_NEXT_MEASURE;
	name_to_edit_button["SCROLL_PREV_MEASURE"]= EDIT_BUTTON_SCROLL_PREV_MEASURE;
	name_to_edit_button["SCROLL_HOME"]= EDIT_BUTTON_SCROLL_HOME;
	name_to_edit_button["SCROLL_END"]= EDIT_BUTTON_SCROLL_END;
	name_to_edit_button["SCROLL_NEXT"]= EDIT_BUTTON_SCROLL_NEXT;
	name_to_edit_button["SCROLL_PREV"]= EDIT_BUTTON_SCROLL_PREV;

	name_to_edit_button["SEGMENT_NEXT"]= EDIT_BUTTON_SEGMENT_NEXT;
	name_to_edit_button["SEGMENT_PREV"]= EDIT_BUTTON_SEGMENT_PREV;

	name_to_edit_button["SCROLL_SELECT"]= EDIT_BUTTON_SCROLL_SELECT;

	name_to_edit_button["LAY_SELECT"]= EDIT_BUTTON_LAY_SELECT;

	name_to_edit_button["SCROLL_SPEED_UP"]= EDIT_BUTTON_SCROLL_SPEED_UP;
	name_to_edit_button["SCROLL_SPEED_DOWN"]= EDIT_BUTTON_SCROLL_SPEED_DOWN;

	name_to_edit_button["SNAP_NEXT"]= EDIT_BUTTON_SNAP_NEXT;
	name_to_edit_button["SNAP_PREV"]= EDIT_BUTTON_SNAP_PREV;

	name_to_edit_button["OPEN_EDIT_MENU"]= EDIT_BUTTON_OPEN_EDIT_MENU;
	name_to_edit_button["OPEN_TIMING_MENU"]= EDIT_BUTTON_OPEN_TIMING_MENU;
	name_to_edit_button["OPEN_ALTER_MENU"]= EDIT_BUTTON_OPEN_ALTER_MENU;
	name_to_edit_button["OPEN_AREA_MENU"]= EDIT_BUTTON_OPEN_AREA_MENU;
	name_to_edit_button["OPEN_BGCHANGE_LAYER1_MENU"]= EDIT_BUTTON_OPEN_BGCHANGE_LAYER1_MENU;
	name_to_edit_button["OPEN_BGCHANGE_LAYER2_MENU"]= EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU;
	name_to_edit_button["OPEN_COURSE_MENU"]= EDIT_BUTTON_OPEN_COURSE_MENU;
	name_to_edit_button["OPEN_COURSE_ATTACK_MENU"]= EDIT_BUTTON_OPEN_COURSE_ATTACK_MENU;

	name_to_edit_button["OPEN_STEP_ATTACK_MENU"]= EDIT_BUTTON_OPEN_STEP_ATTACK_MENU;
	name_to_edit_button["ADD_STEP_MODS"]= EDIT_BUTTON_ADD_STEP_MODS;

	name_to_edit_button["OPEN_INPUT_HELP"]= EDIT_BUTTON_OPEN_INPUT_HELP;

	name_to_edit_button["BAKE_RANDOM_FROM_SONG_GROUP"]= EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP;
	name_to_edit_button["BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE"]= EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE;

	name_to_edit_button["PLAY_FROM_START"]= EDIT_BUTTON_PLAY_FROM_START;
	name_to_edit_button["PLAY_FROM_CURSOR"]= EDIT_BUTTON_PLAY_FROM_CURSOR;
	name_to_edit_button["PLAY_SELECTION"]= EDIT_BUTTON_PLAY_SELECTION;
	name_to_edit_button["RECORD_FROM_CURSOR"]= EDIT_BUTTON_RECORD_FROM_CURSOR;
	name_to_edit_button["RECORD_SELECTION"]= EDIT_BUTTON_RECORD_SELECTION;

	name_to_edit_button["RECORD_HOLD_RESET"]= EDIT_BUTTON_RECORD_HOLD_RESET;
	name_to_edit_button["RECORD_HOLD_OFF"]= EDIT_BUTTON_RECORD_HOLD_OFF;
	name_to_edit_button["RECORD_HOLD_MORE"]= EDIT_BUTTON_RECORD_HOLD_MORE;
	name_to_edit_button["RECORD_HOLD_LESS"]= EDIT_BUTTON_RECORD_HOLD_LESS;

	name_to_edit_button["RETURN_TO_EDIT"]= EDIT_BUTTON_RETURN_TO_EDIT;

	name_to_edit_button["INSERT"]= EDIT_BUTTON_INSERT;
	name_to_edit_button["DELETE"]= EDIT_BUTTON_DELETE;
	name_to_edit_button["INSERT_SHIFT_PAUSES"]= EDIT_BUTTON_INSERT_SHIFT_PAUSES;
	name_to_edit_button["DELETE_SHIFT_PAUSES"]= EDIT_BUTTON_DELETE_SHIFT_PAUSES;

	name_to_edit_button["OPEN_NEXT_STEPS"]= EDIT_BUTTON_OPEN_NEXT_STEPS;
	name_to_edit_button["OPEN_PREV_STEPS"]= EDIT_BUTTON_OPEN_PREV_STEPS;
	name_to_edit_button["PLAY_SAMPLE_MUSIC"]= EDIT_BUTTON_PLAY_SAMPLE_MUSIC;

	name_to_edit_button["BPM_UP"]= EDIT_BUTTON_BPM_UP;
	name_to_edit_button["BPM_DOWN"]= EDIT_BUTTON_BPM_DOWN;
	name_to_edit_button["STOP_UP"]= EDIT_BUTTON_STOP_UP;
	name_to_edit_button["STOP_DOWN"]= EDIT_BUTTON_STOP_DOWN;

	name_to_edit_button["DELAY_UP"]= EDIT_BUTTON_DELAY_UP;
	name_to_edit_button["DELAY_DOWN"]= EDIT_BUTTON_DELAY_DOWN;

	name_to_edit_button["OFFSET_UP"]= EDIT_BUTTON_OFFSET_UP;
	name_to_edit_button["OFFSET_DOWN"]= EDIT_BUTTON_OFFSET_DOWN;
	name_to_edit_button["SAMPLE_START_UP"]= EDIT_BUTTON_SAMPLE_START_UP;
	name_to_edit_button["SAMPLE_START_DOWN"]= EDIT_BUTTON_SAMPLE_START_DOWN;
	name_to_edit_button["SAMPLE_LENGTH_UP"]= EDIT_BUTTON_SAMPLE_LENGTH_UP;
	name_to_edit_button["SAMPLE_LENGTH_DOWN"]= EDIT_BUTTON_SAMPLE_LENGTH_DOWN;

	name_to_edit_button["ADJUST_FINE"]= EDIT_BUTTON_ADJUST_FINE;

	name_to_edit_button["SAVE"]= EDIT_BUTTON_SAVE;

	name_to_edit_button["UNDO"]= EDIT_BUTTON_UNDO;

	name_to_edit_button["ADD_COURSE_MODS"]= EDIT_BUTTON_ADD_COURSE_MODS;

	name_to_edit_button["SWITCH_PLAYERS"]= EDIT_BUTTON_SWITCH_PLAYERS;

	name_to_edit_button["SWITCH_TIMINGS"]= EDIT_BUTTON_SWITCH_TIMINGS;

	m_EditMappingsDeviceInput.Clear();

	// Common mappings:
	switch( EDIT_MODE.GetValue() )
	{
	case EditMode_Practice:
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_PREV_MEASURE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_UP);
		m_EditMappingsMenuButton.button[EDIT_BUTTON_SCROLL_PREV_MEASURE][0] = GAME_BUTTON_UP;
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_NEXT_MEASURE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_DOWN);
		m_EditMappingsMenuButton.button[EDIT_BUTTON_SCROLL_NEXT_MEASURE][0] = GAME_BUTTON_DOWN;
		break;
	default:
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_UP_LINE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_UP);
		m_EditMappingsMenuButton.button[EDIT_BUTTON_SCROLL_UP_LINE][0] = GAME_BUTTON_UP;
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_DOWN_LINE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_DOWN);
		m_EditMappingsMenuButton.button[EDIT_BUTTON_SCROLL_DOWN_LINE][0] = GAME_BUTTON_DOWN;
		break;
	}

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_UP_PAGE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_PGUP);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_UP_PAGE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_SEMICOLON);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_DOWN_PAGE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_PGDN);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_DOWN_PAGE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_SQUOTE);

	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_UP_TS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_UP_TS][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_UP_TS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_PGUP);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_UP_TS][1] = DeviceInput(DEVICE_KEYBOARD, KEY_SEMICOLON);

	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_DOWN_TS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_DOWN_TS][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_DOWN_TS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_PGDN);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_DOWN_TS][1] = DeviceInput(DEVICE_KEYBOARD, KEY_SQUOTE);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_HOME][0] = DeviceInput(DEVICE_KEYBOARD, KEY_HOME);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_END][0] = DeviceInput(DEVICE_KEYBOARD, KEY_END);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_NEXT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_PERIOD);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_PREV][0] = DeviceInput(DEVICE_KEYBOARD, KEY_COMMA);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SEGMENT_NEXT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_PERIOD);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SEGMENT_NEXT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SEGMENT_NEXT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SEGMENT_PREV][0] = DeviceInput(DEVICE_KEYBOARD, KEY_COMMA);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SEGMENT_PREV][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SEGMENT_PREV][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_SELECT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_SELECT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_LAY_SELECT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_SPACE);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_PLAY_FROM_START][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cp);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_PLAY_FROM_START][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_PLAY_FROM_START][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_PLAY_FROM_CURSOR][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cp);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_PLAY_FROM_CURSOR][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_PLAY_FROM_CURSOR][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_PLAY_SELECTION][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cp);

	// EditMode-specific mappings
	switch( EDIT_MODE.GetValue() )
	{
	case EditMode_Practice:
		// Left = Zoom out (1x, 2x, 4x)
		// Right = Zoom in
		m_EditMappingsDeviceInput.button   [EDIT_BUTTON_SCROLL_SPEED_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LEFT);
		m_EditMappingsDeviceInput.button   [EDIT_BUTTON_SCROLL_SPEED_UP][0]   = DeviceInput(DEVICE_KEYBOARD, KEY_RIGHT);

		// F1 = Show help popup
		m_EditMappingsDeviceInput.button   [EDIT_BUTTON_OPEN_INPUT_HELP][0]   = DeviceInput(DEVICE_KEYBOARD, KEY_F1);

		// Esc = Show Edit Menu
		m_EditMappingsDeviceInput.button   [EDIT_BUTTON_OPEN_EDIT_MENU][0]    = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
		m_EditMappingsMenuButton.button   [EDIT_BUTTON_OPEN_EDIT_MENU][0]    = GAME_BUTTON_START;
		m_EditMappingsMenuButton.button   [EDIT_BUTTON_OPEN_EDIT_MENU][1]    = GAME_BUTTON_BACK;


		// Escape, Enter = exit play/record
		m_PlayMappingsDeviceInput.button   [EDIT_BUTTON_RETURN_TO_EDIT][0]    = DeviceInput(DEVICE_KEYBOARD, KEY_ENTER);
		m_PlayMappingsDeviceInput.button   [EDIT_BUTTON_RETURN_TO_EDIT][1]    = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
		m_PlayMappingsMenuButton.button   [EDIT_BUTTON_RETURN_TO_EDIT][1]    = GAME_BUTTON_BACK;
		return;
	case EditMode_CourseMods:
		// Left/Right = Snap to Next/Prev
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SNAP_NEXT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LEFT);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SNAP_PREV][0] = DeviceInput(DEVICE_KEYBOARD, KEY_RIGHT);

		// v = course playback menu
		// m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_COURSE_ATTACK_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cv);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_ADD_COURSE_MODS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Co);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_COURSE_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cv);

		// F1 = Show help popup
		m_EditMappingsDeviceInput.button   [EDIT_BUTTON_OPEN_INPUT_HELP][0]   = DeviceInput(DEVICE_KEYBOARD, KEY_F1);

		// Esc = Show Edit Menu
		m_EditMappingsDeviceInput.button   [EDIT_BUTTON_OPEN_EDIT_MENU][0]    = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
		m_EditMappingsMenuButton.button   [EDIT_BUTTON_OPEN_EDIT_MENU][0]    = GAME_BUTTON_START;
		m_EditMappingsMenuButton.button   [EDIT_BUTTON_OPEN_EDIT_MENU][1]    = GAME_BUTTON_BACK;

		// Escape, Enter = exit play/record
		m_PlayMappingsDeviceInput.button   [EDIT_BUTTON_RETURN_TO_EDIT][0]    = DeviceInput(DEVICE_KEYBOARD, KEY_ENTER);
		m_PlayMappingsDeviceInput.button   [EDIT_BUTTON_RETURN_TO_EDIT][1]    = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
		m_PlayMappingsMenuButton.button   [EDIT_BUTTON_RETURN_TO_EDIT][0]    = GAME_BUTTON_START;
		return;

	case EditMode_Full:
		// F4 = Show timing menu
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_TIMING_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F4);

		/* Don't allow F5/F6 in home mode. It breaks the "delay creation until first save" logic. */
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_PREV_STEPS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F5);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_NEXT_STEPS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F6);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_BPM_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F7);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_BPM_UP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F8);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_STOP_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F9);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_STOP_UP][0]  = DeviceInput(DEVICE_KEYBOARD, KEY_F10);

		m_EditMappingsDeviceInput.button[EDIT_BUTTON_DELAY_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F9);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_DELAY_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_DELAY_DOWN][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_DELAY_UP][0]  = DeviceInput(DEVICE_KEYBOARD, KEY_F10);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_DELAY_UP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_DELAY_UP][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);

		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OFFSET_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F11);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OFFSET_UP][0]  = DeviceInput(DEVICE_KEYBOARD, KEY_F12);

		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SAMPLE_START_UP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_RBRACKET);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SAMPLE_START_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LBRACKET);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SAMPLE_LENGTH_UP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_RBRACKET);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAMPLE_LENGTH_UP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAMPLE_LENGTH_UP][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_SAMPLE_LENGTH_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LBRACKET);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAMPLE_LENGTH_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAMPLE_LENGTH_DOWN][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);

		m_EditMappingsDeviceInput.button[EDIT_BUTTON_PLAY_SAMPLE_MUSIC][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cl);

		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_BGCHANGE_LAYER1_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cb);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cb);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);
		// m_EditMappingsDeviceInput.button[EDIT_BUTTON_ADD_STEP_MODS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cc);
		// m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_STEP_ATTACK_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cv);

		m_EditMappingsDeviceInput.button[EDIT_BUTTON_INSERT_SHIFT_PAUSES][0] = DeviceInput(DEVICE_KEYBOARD, KEY_INSERT);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_INSERT_SHIFT_PAUSES][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_INSERT_SHIFT_PAUSES][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_DELETE_SHIFT_PAUSES][0] = DeviceInput(DEVICE_KEYBOARD, KEY_DEL);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_DELETE_SHIFT_PAUSES][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_DELETE_SHIFT_PAUSES][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

		m_EditMappingsDeviceInput.button[EDIT_BUTTON_RECORD_HOLD_LESS][0]= DeviceInput(DEVICE_KEYBOARD, KEY_Cq);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_RECORD_HOLD_MORE][0]= DeviceInput(DEVICE_KEYBOARD, KEY_Cw);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_RECORD_HOLD_RESET][0]= DeviceInput(DEVICE_KEYBOARD, KEY_Ce);
		m_EditMappingsDeviceInput.button[EDIT_BUTTON_RECORD_HOLD_OFF][0]= DeviceInput(DEVICE_KEYBOARD, KEY_Cr);
		break;
	default: break;
	}

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_0][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C1);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_1][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C2);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_2][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C3);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_3][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C4);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_4][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C5);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_5][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C6);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_6][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C7);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_7][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C8);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_8][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C9);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_COLUMN_9][0] = DeviceInput(DEVICE_KEYBOARD, KEY_C0);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_RIGHT_SIDE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LALT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_RIGHT_SIDE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RALT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_LAY_ROLL][0]   = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_LAY_ROLL][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_CYCLE_TAP_LEFT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cn);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_CYCLE_TAP_RIGHT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cm);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_CYCLE_SEGMENT_LEFT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cn);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_CYCLE_SEGMENT_LEFT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_CYCLE_SEGMENT_LEFT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_CYCLE_SEGMENT_RIGHT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cm);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_CYCLE_SEGMENT_RIGHT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_CYCLE_SEGMENT_RIGHT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button    [EDIT_BUTTON_SCROLL_SPEED_UP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_UP);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_SPEED_UP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_SPEED_UP][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button    [EDIT_BUTTON_SCROLL_SPEED_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_DOWN);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_SPEED_DOWN][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SCROLL_SPEED_DOWN][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_SELECT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SCROLL_SELECT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SNAP_NEXT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LEFT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SNAP_PREV][0] = DeviceInput(DEVICE_KEYBOARD, KEY_RIGHT);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_EDIT_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
	m_EditMappingsMenuButton.button[EDIT_BUTTON_OPEN_EDIT_MENU][0] = GAME_BUTTON_START;
	m_EditMappingsMenuButton.button[EDIT_BUTTON_OPEN_EDIT_MENU][1] = GAME_BUTTON_BACK;
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_AREA_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_ENTER);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_ALTER_MENU][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Ca);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_OPEN_INPUT_HELP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_F1);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cb);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LALT);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RALT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cb);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_RECORD_SELECTION][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cr);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_RECORD_SELECTION][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_EditMappingsDeviceInput.hold[EDIT_BUTTON_RECORD_SELECTION][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_INSERT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_INSERT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_INSERT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_BACKSLASH);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_DELETE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_DEL);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_ADJUST_FINE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_RALT);
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_ADJUST_FINE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_LALT);

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SAVE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_Cs);
	#if defined(MACOSX)
		/* use cmd */
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAVE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LMETA);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAVE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RMETA);
	#else
		/* use ctrl */
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAVE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
		m_EditMappingsDeviceInput.hold[EDIT_BUTTON_SAVE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);
	#endif

	m_EditMappingsDeviceInput.button[EDIT_BUTTON_UNDO][1] = DeviceInput(DEVICE_KEYBOARD, KEY_Cu);

	// Switch players, if it makes sense to do so.
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SWITCH_PLAYERS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_SLASH);

	// Allow song and step timing to be swapped.
	m_EditMappingsDeviceInput.button[EDIT_BUTTON_SWITCH_TIMINGS][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Ct);

	m_PlayMappingsDeviceInput.button[EDIT_BUTTON_RETURN_TO_EDIT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_ENTER);
	m_PlayMappingsDeviceInput.button[EDIT_BUTTON_RETURN_TO_EDIT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
	m_PlayMappingsMenuButton.button[EDIT_BUTTON_RETURN_TO_EDIT][0] = GAME_BUTTON_BACK;
	m_PlayMappingsMenuButton.button[EDIT_BUTTON_RETURN_TO_EDIT][1] = GAME_BUTTON_START;

	m_RecordMappingsDeviceInput.button[EDIT_BUTTON_LAY_ROLL][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT);
	m_RecordMappingsDeviceInput.button[EDIT_BUTTON_LAY_ROLL][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT);
	m_RecordMappingsDeviceInput.button[EDIT_BUTTON_REMOVE_NOTE][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LALT);
	m_RecordMappingsDeviceInput.button[EDIT_BUTTON_REMOVE_NOTE][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RALT);
	m_RecordMappingsDeviceInput.button[EDIT_BUTTON_RETURN_TO_EDIT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
	m_RecordMappingsDeviceInput.button[EDIT_BUTTON_RETURN_TO_EDIT][1] = DeviceInput(DEVICE_KEYBOARD, KEY_ENTER);
	m_RecordMappingsMenuButton.button[EDIT_BUTTON_RETURN_TO_EDIT][0] = GAME_BUTTON_BACK;
	m_RecordMappingsMenuButton.button[EDIT_BUTTON_RETURN_TO_EDIT][1] = GAME_BUTTON_START;

	m_RecordPausedMappingsDeviceInput.button[EDIT_BUTTON_PLAY_SELECTION][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cp);
	m_RecordPausedMappingsDeviceInput.button[EDIT_BUTTON_RECORD_SELECTION][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cr);
	m_RecordPausedMappingsDeviceInput.hold[EDIT_BUTTON_RECORD_SELECTION][0] = DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL);
	m_RecordPausedMappingsDeviceInput.hold[EDIT_BUTTON_RECORD_SELECTION][1] = DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL);
	m_RecordPausedMappingsDeviceInput.button[EDIT_BUTTON_RECORD_FROM_CURSOR][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cr);
	m_RecordPausedMappingsDeviceInput.button[EDIT_BUTTON_RETURN_TO_EDIT][0] = DeviceInput(DEVICE_KEYBOARD, KEY_ESC);
	m_RecordPausedMappingsMenuButton.button[EDIT_BUTTON_RETURN_TO_EDIT][1] = GAME_BUTTON_BACK;
	m_RecordPausedMappingsDeviceInput.button[EDIT_BUTTON_UNDO][0] = DeviceInput(DEVICE_KEYBOARD, KEY_Cu);

	IniFile mapping_ini;
	// Only use the mappings file if it exists.  It's meant to be optional, and
	// only used in rare cases like someone having critical keys broken. -Kyz
	if(mapping_ini.ReadFile(SpecialFiles::EDIT_MODE_KEYMAPS_PATH))
	{
		LoadKeymapSectionIntoMappingsMember(mapping_ini.GetChild("Edit"),
			m_EditMappingsDeviceInput);
		LoadKeymapSectionIntoMappingsMember(mapping_ini.GetChild("Play"),
			m_PlayMappingsDeviceInput);
		LoadKeymapSectionIntoMappingsMember(mapping_ini.GetChild("Record"),
			m_RecordMappingsDeviceInput);
		LoadKeymapSectionIntoMappingsMember(mapping_ini.GetChild("RecordPaused"),
			m_RecordPausedMappingsDeviceInput);
	}
}

void ScreenEdit::LoadKeymapSectionIntoMappingsMember(XNode const* section, MapEditToDI& mappings)
{
	if(section == nullptr) {return;} // Not an error, sections are optional. -Kyz
	FOREACH_CONST_Attr(section, attr)
	{
		map<RString, EditButton>::iterator name_entry=
			name_to_edit_button.find(attr->first);
		if(name_entry != name_to_edit_button.end())
		{
			RString joined_names;
			attr->second->GetValue(joined_names);
			vector<RString> key_names;
			split(joined_names, DEVICE_INPUT_SEPARATOR, key_names, false);
			for(size_t k= 0; k < key_names.size() && k < NUM_EDIT_TO_DEVICE_SLOTS; ++k)
			{
				DeviceInput devi;
				devi.FromString(key_names[k]);
				if(devi.IsValid())
				{
					mappings.button[name_entry->second][k]= devi;
				}
			}
		}
	}
}


/* Given a DeviceInput that was just depressed, return an active edit function. */
EditButton ScreenEdit::DeviceToEdit( const DeviceInput &DeviceI ) const
{
	ASSERT( DeviceI.IsValid() );

	const MapEditToDI *pCurrentMap = GetCurrentDeviceInputMap();

	/* First, search to see if a key that requires a modifier is pressed. */
	FOREACH_EditButton( e )
	{
		for( int slot = 0; slot < NUM_EDIT_TO_DEVICE_SLOTS; ++slot )
		{
			if( pCurrentMap->button[e][slot] == DeviceI && pCurrentMap->hold[e][0].IsValid() )
			{
				/* The button maps to this function, and has one or more shift modifiers attached. */
				for( int holdslot = 0; holdslot < NUM_EDIT_TO_DEVICE_SLOTS; ++holdslot )
				{
					DeviceInput hDI = pCurrentMap->hold[e][holdslot];
					if( hDI.IsValid() && INPUTFILTER->IsBeingPressed(hDI) )
						return e;
				}
			}
		}
	}

	/* No shifted keys matched.  See if any unshifted inputs are bound to this key. */
	FOREACH_EditButton(e)
	{
		for( int slot = 0; slot < NUM_EDIT_TO_DEVICE_SLOTS; ++slot )
		{
			if( pCurrentMap->button[e][slot] == DeviceI && !pCurrentMap->hold[e][0].IsValid() )
			{
				/* The button maps to this function. */
				return e;
			}
		}
	}

	return EditButton_Invalid;
}

/* Given a DeviceInput that was just depressed, return an active edit function. */
EditButton ScreenEdit::MenuButtonToEditButton( GameButton MenuI ) const
{
	const MapEditButtonToMenuButton *pCurrentMap = GetCurrentMenuButtonMap();

	FOREACH_EditButton(e)
	{
		for( int slot = 0; slot < NUM_EDIT_TO_MENU_SLOTS; ++slot )
		{
			if( pCurrentMap->button[e][slot] == MenuI )
			{
				/* The button maps to this function. */
				return e;
			}
		}
	}

	return EditButton_Invalid;
}

/* If DeviceI was just pressed, return true if button is triggered.  (More than one
 * function may be mapped to a key.) */
bool ScreenEdit::EditPressed( EditButton button, const DeviceInput &DeviceI )
{
	ASSERT( DeviceI.IsValid() );

	const MapEditToDI *pCurrentMap = GetCurrentDeviceInputMap();

	/* First, search to see if a key that requires a modifier is pressed. */
	bool bPrimaryButtonPressed = false;
	for( int slot = 0; slot < NUM_EDIT_TO_DEVICE_SLOTS; ++slot )
	{
		if( pCurrentMap->button[button][slot] == DeviceI )
			bPrimaryButtonPressed = true;
	}

	if( !bPrimaryButtonPressed )
		return false;

	/* The button maps to this function.  Does the function has one or more shift modifiers attached? */
	if( !pCurrentMap->hold[button][0].IsValid() )
		return true;

	for( int holdslot = 0; holdslot < NUM_EDIT_TO_DEVICE_SLOTS; ++holdslot )
	{
		DeviceInput hDI = pCurrentMap->hold[button][holdslot];
		if( INPUTFILTER->IsBeingPressed(hDI) )
			return true;
	}

	/* No shifted keys matched. */
	return false;
}

bool ScreenEdit::EditToDevice( EditButton button, int iSlotNum, DeviceInput &DeviceI ) const
{
	ASSERT( iSlotNum < NUM_EDIT_TO_DEVICE_SLOTS );
	const MapEditToDI *pCurrentMap = GetCurrentDeviceInputMap();
	if( pCurrentMap->button[button][iSlotNum].IsValid() )
		DeviceI = pCurrentMap->button[button][iSlotNum];
	else if( pCurrentMap->hold[button][iSlotNum].IsValid() )
		DeviceI = pCurrentMap->hold[button][iSlotNum];
	return DeviceI.IsValid();
}

bool ScreenEdit::EditIsBeingPressed( EditButton button ) const
{
	for( int slot = 0; slot < NUM_EDIT_TO_DEVICE_SLOTS; ++slot )
	{
		DeviceInput DeviceI;
		if( EditToDevice( button, slot, DeviceI ) && INPUTFILTER->IsBeingPressed(DeviceI) )
			return true;
	}

	return false;
}

const MapEditToDI *ScreenEdit::GetCurrentDeviceInputMap() const
{
	switch( m_EditState )
	{
	DEFAULT_FAIL( m_EditState );
	case STATE_EDITING:		return &m_EditMappingsDeviceInput;
	case STATE_PLAYING:		return &m_PlayMappingsDeviceInput;
	case STATE_RECORDING:		return &m_RecordMappingsDeviceInput;
	case STATE_RECORDING_PAUSED:	return &m_RecordPausedMappingsDeviceInput;
	}
}

const MapEditButtonToMenuButton *ScreenEdit::GetCurrentMenuButtonMap() const
{
	switch( m_EditState )
	{
	DEFAULT_FAIL( m_EditState );
	case STATE_EDITING:		return &m_EditMappingsMenuButton;
	case STATE_PLAYING:		return &m_PlayMappingsMenuButton;
	case STATE_RECORDING:		return &m_RecordMappingsMenuButton;
	case STATE_RECORDING_PAUSED:	return &m_RecordPausedMappingsMenuButton;
	}
}

enum RowCountChoice
{
	RCC_4M,
	RCC_2M,
	RCC_1M,
	RCC_2ND,
	RCC_4TH,
	RCC_8TH,
	RCC_12TH,
	RCC_16TH,
	RCC_24TH,
	RCC_32ND,
	RCC_48TH,
	RCC_64TH,
	RCC_192ND,
};

#define RCC_CHOICES RCC_4TH, "4m", "2m", "1m", "2nd", "4th","8th","12th","16th","24th","32nd","48th","64th","192nd"

int GetRowsFromAnswers(int choice, const vector<int>& answers);
int GetRowsFromAnswers(int choice, const vector<int>& answers)
{
	if(answers.empty())
	{
		return 192 / 8;
	}
	switch(answers[choice])
	{
		case RCC_4M:
			return 192 * 4;
		case RCC_2M:
			return 192 * 2;
		case RCC_1M:
			return 192;
		case RCC_2ND:
			return 192 / 2;
		case RCC_4TH:
		default:
			return 192 / 4;
		case RCC_8TH:
			return 192 / 8;
		case RCC_12TH:
			return 192 / 12;
		case RCC_16TH:
			return 192 / 16;
		case RCC_24TH:
			return 192 / 24;
		case RCC_32ND:
			return 192 / 32;
		case RCC_48TH:
			return 192 / 48;
		case RCC_64TH:
			return 3;
		case RCC_192ND:
			return 1;
	}
}

static MenuDef g_EditHelp(
	"ScreenMiniMenuEditHelp"
	// fill this in dynamically
);

static MenuDef g_AttackAtTimeMenu(
	"ScreenMiniMenuAttackAtTimeMenu"
	// fill this in dynamically
);

static MenuDef g_IndividualAttack(
	"ScreenMiniMenuIndividualAttack"
	// fill this in dynamically
);

static MenuDef g_KeysoundTrack(
							   "ScreenMiniMenuKeysoundTrack"
							   ); // fill this in dynamically

static MenuDef g_MainMenu(
	"ScreenMiniMenuMainMenu",
	MenuRowDef(ScreenEdit::play_whole_song,
		"Play whole song",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::play_current_beat_to_end,
		"Play current beat to end",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::save,
		"Save",
		true, EditMode_Home, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::play_selection,
		"Play selection",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::set_selection_start,
		"Set selection start",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::set_selection_end,
		"Set selection end",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::revert_to_last_save,
		"Revert to last save",
		true, EditMode_Home, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::revert_from_disk,
		"Revert from disk",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::options,
		"Editor options",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::edit_song_info,
		"Edit song info",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::edit_steps_information,
		"Edit steps information",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::edit_timing_data,
		"Edit Timing Data",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::view_steps_data,
		"View steps data",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::play_preview_music,
		"Play preview music",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::exit,
		"Exit Edit Mode",
		true, EditMode_Practice, true, true, 0, nullptr )
);

static MenuDef g_AlterMenu(
	"ScreenMiniMenuAlterMenu",
	MenuRowDef(ScreenEdit::cut,
		"Cut",
		true,
		EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::copy,
		"Copy",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::clear,			"Clear area",				true,
	      EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::quantize,			"Quantize",				true,
	      EditMode_Practice, true, true, 0,
	      "4th","8th","12th","16th","24th","32nd","48th","64th","192nd"),
	MenuRowDef(ScreenEdit::turn,				"Turn",					true,
	      EditMode_Practice, true, true, 0, "Left","Right","Mirror","Backwards","Shuffle","SuperShuffle" ),
	MenuRowDef(ScreenEdit::transform,			"Transform",				true,
	      EditMode_Practice, true, true, 0, "NoHolds","NoMines","Little","Wide",
	      "Big","Quick","Skippy","Mines","Echo","Stomp","Planted","Floored",
	      "Twister","NoJumps","NoHands","NoQuads","NoStretch" ),
	MenuRowDef(ScreenEdit::alter,			"Alter",				true,
	      EditMode_Practice, true, true, 0, "Autogen To Fill Width","Backwards","Swap Sides",
	      "Copy Left To Right","Copy Right To Left","Clear Left","Clear Right",
		"Collapse To One","Collapse Left","Shift Left","Shift Right", "Swap Up/Down", "Arbitrary Remap Columns" ),
	MenuRowDef(ScreenEdit::tempo,			"Tempo",				true,
	      EditMode_Full, true, true, 0, "Compress 2x","Compress 3->2",
	      "Compress 4->3","Expand 3->4","Expand 2->3","Expand 2x" ),
	MenuRowDef(ScreenEdit::play,
		"Play selection",
		true,
		EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::record,
		"Record in selection",
		true,
		EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::preview_designation,
		"Designate as Music Preview",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::convert_to_pause,
		"Convert selection to pause",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::convert_to_delay,
		"Convert selection to delay",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::convert_to_warp,
		"Convert selection to warp",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::convert_to_fake,
		"Convert selection to fake",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::convert_to_attack,
		"Convert selection to attack",
		true,
		EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::routine_invert_notes,
		"Invert notes' player",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::routine_mirror_1_to_2,
		"Mirror Player 1 to 2",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::routine_mirror_2_to_1,
		"Mirror Player 2 to 1",
		true,
		EditMode_Full, true, true, 0, nullptr )
);

static MenuDef g_AreaMenu(
	"ScreenMiniMenuAreaMenu",
	MenuRowDef(ScreenEdit::paste_at_current_beat,
		"Paste at current beat",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::paste_at_begin_marker,
		"Paste at begin marker",
		true, EditMode_Practice, true, true, 0, nullptr ),
    MenuRowDef(ScreenEdit::insert_and_shift,
		"Insert beat and shift down",
		true, EditMode_Practice, true, true, RCC_CHOICES ),
	MenuRowDef(ScreenEdit::delete_and_shift,
		"Delete beat and shift up",
		true, EditMode_Practice, true, true, RCC_CHOICES ),
	MenuRowDef(ScreenEdit::shift_pauses_forward,
		"Shift all timing changes down",
		true, EditMode_Full, true, true, RCC_CHOICES ),
	MenuRowDef(ScreenEdit::shift_pauses_backward,
		"Shift all timing changes up",
		true, EditMode_Full, true, true, RCC_CHOICES ),
	MenuRowDef(ScreenEdit::convert_pause_to_beat,
		"Convert pause to beats",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::convert_delay_to_beat,
		"Convert delay to beats",
		true,
		EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::last_second_at_beat,
		"Designate last second at current beat",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::undo,
		"Undo",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::clear_clipboard,
		"Clear clipboard",
		true,
		EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::modify_attacks_at_row,
		"Modify Attacks at current beat",
		true, EditMode_CourseMods, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::modify_keysounds_at_row,
		"Modify Keysounds at current beat",
		true, EditMode_Full, true, true, 0, nullptr)

);

static MenuDef g_StepsInformation(
	"ScreenMiniMenuStepsInformation",
	MenuRowDef(ScreenEdit::difficulty,
		"Difficulty",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::meter,
		"Meter",
		true, EditMode_Practice, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::predict_meter,
		"Predicted Meter",
		false, EditMode_Full, true, true, 0, nullptr ),
    MenuRowDef(ScreenEdit::chartname,
		"Chart Name",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::description,
		"Description",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::chartstyle,
		"Chart Style",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::step_credit,
		"Step Author",
		true, EditMode_Practice, true, true, 0, nullptr ),
    MenuRowDef(ScreenEdit::step_display_bpm,
		"Display BPM",
		true, EditMode_Full, true, true, 0, "Actual", "Specified", "Random" ),
    MenuRowDef(ScreenEdit::step_min_bpm,
		"Min BPM",
		true, EditMode_Full, true, true, 0, nullptr ),
    MenuRowDef(ScreenEdit::step_max_bpm,
		"Max BPM",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::step_music,
		"Music File", true, EditMode_Full,true, true, 0, nullptr)
);

static MenuDef g_StepsData(
	"ScreenMiniMenuStepsData",
	MenuRowDef( ScreenEdit::tap_notes,	"Tap Steps",	false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::jumps,		"Jumps",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::hands,		"Hands",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::quads,		"Quads",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::holds,		"Holds",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::mines,		"Mines",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::rolls,		"Rolls",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::lifts,		"Lifts",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::fakes,		"Fakes",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::stream,		"Stream",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::voltage,	"Voltage",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::air,		"Air",			false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::freeze,		"Freeze",		false, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef( ScreenEdit::chaos,		"Chaos",		false, EditMode_Full, true, true, 0, nullptr )
);

static MenuDef g_SongInformation(
	"ScreenMiniMenuSongInformation",
	MenuRowDef(ScreenEdit::main_title,
		"Main title",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::sub_title,
		"Sub title",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::artist,
		"Artist",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::genre,
		"Genre",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::credit,
		"Credit",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::preview,
		"Preview",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::main_title_transliteration,
		"Main title transliteration",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::sub_title_transliteration,
		"Sub title transliteration",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::artist_transliteration,
		"Artist transliteration",
		true, EditMode_Practice, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::last_second_hint,
		"Last second hint",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::preview_start,
		"Preview Start",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::preview_length,
		"Preview Length",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::display_bpm,
		"Display BPM",
		true, EditMode_Full, true, true, 0, "Actual", "Specified", "Random" ),
	MenuRowDef(ScreenEdit::min_bpm,
		"Min BPM",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::max_bpm,
		"Max BPM",
		true, EditMode_Full, true, true, 0, nullptr )
);

// Ugh, I don't like making this global pointer to clipboardFullTiming, but
// it's the only way to make it visible to EnabledIfClipboardTimingIsSafe for
// making sure it's safe to paste as the timing data for the Steps/Song. -Kyz
static TimingData* clipboard_full_timing= nullptr;
static bool EnabledIfClipboardTimingIsSafe();
static bool EnabledIfClipboardTimingIsSafe()
{
	return clipboard_full_timing != nullptr && clipboard_full_timing->IsSafeFullTiming();
}
static MenuDef g_TimingDataInformation(
	"ScreenMiniMenuTimingDataInformation",
	MenuRowDef(ScreenEdit::beat_0_offset,
		"Beat 0 Offset",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::bpm,
		"Edit BPM change",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::stop,
		"Edit stop",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::delay,
		"Edit delay",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::label,
		"Edit label",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::tickcount,
		"Edit tickcount",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::combo,
		"Edit combo",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::warp,
		"Edit warp",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::speed_percent,
		"Edit speed (percent)",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::speed_wait,
		"Edit speed (wait)",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::speed_mode,
		"Edit speed (mode)",
		true, EditMode_Full, true, true, 0, "Beats", "Seconds" ),
	MenuRowDef(ScreenEdit::scroll,
		"Edit scrolling factor",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::fake,
		"Edit fake",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::shift_timing_in_region_down,
		"Shift timing in region down",
		true, EditMode_Full, true, true, RCC_CHOICES),
	MenuRowDef(ScreenEdit::shift_timing_in_region_up,
		"Shift timing in region up",
		true, EditMode_Full, true, true, RCC_CHOICES),
	MenuRowDef(ScreenEdit::copy_timing_in_region,
		"Copy timing in region",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::clear_timing_in_region,
		"Clear timing in region",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::paste_timing_from_clip,
		"Paste timing from clipboard",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::copy_full_timing,
		"Copy timing data",
		true, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::paste_full_timing,
		"Paste timing data",
		EnabledIfClipboardTimingIsSafe, EditMode_Full, true, true, 0, nullptr ),
	MenuRowDef(ScreenEdit::erase_step_timing,
		"Erase step timing",
		true, EditMode_Full, true, true, 0, nullptr )
);

static MenuDef g_TimingDataChangeInformation(
	"ScreenMiniMenuTimingDataChangeInformation",
	MenuRowDef(ScreenEdit::timing_all,
		"All timing",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_bpm,
		"BPM changes",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_stop,
		"Stops",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_delay,
		"Delays",
		true, EditMode_Full, true, true, 0, nullptr),
	// Time signatures disabled because they don't fully work. -Kyz
	// MenuRowDef(ScreenEdit::timing_time_sig,
	// 	"Time Signatures",
	// 	true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_warp,
		"Warps",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_label,
		"Labels",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_tickcount,
		"Tickcount",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_combo,
		"Combo segments",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_speed,
		"Speed segments",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_scroll,
		"Scroll segments",
		true, EditMode_Full, true, true, 0, nullptr),
	MenuRowDef(ScreenEdit::timing_fake,
		"Fakes",
		true, EditMode_Full, true, true, 0, nullptr)
);

// XXX: What are these enums used for?
enum
{
	song_bganimation,
	song_movie,
	song_bitmap,
	global_bganimation,
	global_movie,
	global_movie_song_group,
	global_movie_song_group_and_genre,
	dynamic_random,
	baked_random,
	none
};
static bool EnabledIfSet1SongBGAnimation();
static bool EnabledIfSet1SongMovie();
static bool EnabledIfSet1SongBitmap();
static bool EnabledIfSet1GlobalBGAnimation();
static bool EnabledIfSet1GlobalMovie();
static bool EnabledIfSet1GlobalMovieSongGroup();
static bool EnabledIfSet1GlobalMovieSongGroupAndGenre();
static bool EnabledIfSet2SongBGAnimation();
static bool EnabledIfSet2SongMovie();
static bool EnabledIfSet2SongBitmap();
static bool EnabledIfSet2GlobalBGAnimation();
static bool EnabledIfSet2GlobalMovie();
static bool EnabledIfSet2GlobalMovieSongGroup();
static bool EnabledIfSet2GlobalMovieSongGroupAndGenre();
static MenuDef g_BackgroundChange(
	"ScreenMiniMenuBackgroundChange",
	MenuRowDef(ScreenEdit::layer,
		"Layer",
		false,
		EditMode_Full, true, false, 0, "" ),
	MenuRowDef(ScreenEdit::rate,
		"Rate",
		true,
		EditMode_Full, true, false, 10, "0%","10%","20%","30%","40%","50%",
		"60%","70%","80%","90%","100%","120%","140%","160%","180%","200%",
		"220%","240%","260%","280%","300%","350%","400%" ),
	MenuRowDef(ScreenEdit::transition,
		"Force Transition",
		true,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::effect,
		"Force Effect",
		true,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::color1,
		"Force Color 1",
		true,
		EditMode_Full, true, false, 0, "-","#FFFFFF","#808080","#FFFFFF80",
		"#000000","#FF0000","#00FF00","#0000FF","#FFFF00","#00FFFF","#FF00FF" ),
	MenuRowDef(ScreenEdit::color2,
		"Force Color 2",
		true,
		EditMode_Full, true, false, 0, "-","#FFFFFF","#808080","#FFFFFF80",
		"#000000","#FF0000","#00FF00","#0000FF","#FFFF00","#00FFFF","#FF00FF" ),
	MenuRowDef(ScreenEdit::file1_type,
		"File1 Type",
		true,
		EditMode_Full, true, true, 0, "Song BGAnimation", "Song Movie",
		"Song Bitmap", "Global BGAnimation", "Global Movie",
		"Global Movie from Song Group", "Global Movie from Song Group and Genre",
		"Dynamic Random", "Baked Random", "None" ),
	MenuRowDef(ScreenEdit::file1_song_bganimation,
		"File1 Song BGAnimation",
		EnabledIfSet1SongBGAnimation,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file1_song_movie,
		"File1 Song Movie",
		EnabledIfSet1SongMovie,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file1_song_still,
		"File1 Song Still",
		EnabledIfSet1SongBitmap,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file1_global_bganimation,
		"File1 Global BGAnimation",
		EnabledIfSet1GlobalBGAnimation,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file1_global_movie,
		"File1 Global Movie",
		EnabledIfSet1GlobalMovie,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file1_global_movie_song_group,
		"File1 Global Movie (Group)",
		EnabledIfSet1GlobalMovieSongGroup,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file1_global_movie_song_group_and_genre,
		"File1 Global Movie (Group + Genre)",
		EnabledIfSet1GlobalMovieSongGroupAndGenre,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file2_type,
		"File2 Type",
		true,
		EditMode_Full, true, true, 0, "Song BGAnimation", "Song Movie",
		"Song Bitmap", "Global BGAnimation", "Global Movie",
		"Global Movie from Song Group", "Global Movie from Song Group and Genre",
		"Dynamic Random", "Baked Random", "None" ),
	MenuRowDef(ScreenEdit::file2_song_bganimation,
		"File2 Song BGAnimation",
		EnabledIfSet2SongBGAnimation,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file2_song_movie,
		"File2 Song Movie",
		EnabledIfSet2SongMovie,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file2_song_still,
		"File2 Song Still",
		EnabledIfSet2SongBitmap,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file2_global_bganimation,
		"File2 Global BGAnimation",
		EnabledIfSet2GlobalBGAnimation,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file2_global_movie,
		"File2 Global Movie",
		EnabledIfSet2GlobalMovie,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file2_global_movie_song_group,
		"File2 Global Movie (Group)",
		EnabledIfSet2GlobalMovieSongGroup,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::file2_global_movie_song_group_and_genre,
		"File2 Global Movie (Group + Genre)",
		EnabledIfSet2GlobalMovieSongGroupAndGenre,
		EditMode_Full, true, false, 0, nullptr ),
	MenuRowDef(ScreenEdit::delete_change,
		"Remove Change",
		true,
		EditMode_Full, true, true, 0, nullptr )
);
static bool EnabledIfSet1SongBGAnimation()		{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file1_type] == song_bganimation			&& !g_BackgroundChange.rows[ScreenEdit::file1_song_bganimation].choices.empty(); }
static bool EnabledIfSet1SongMovie()			{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file1_type] == song_movie				&& !g_BackgroundChange.rows[ScreenEdit::file1_song_movie].choices.empty(); }
static bool EnabledIfSet1SongBitmap()			{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file1_type] == song_bitmap				&& !g_BackgroundChange.rows[ScreenEdit::file1_song_still].choices.empty(); }
static bool EnabledIfSet1GlobalBGAnimation()		{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file1_type] == global_bganimation			&& !g_BackgroundChange.rows[ScreenEdit::file1_global_bganimation].choices.empty(); }
static bool EnabledIfSet1GlobalMovie()			{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file1_type] == global_movie			&& !g_BackgroundChange.rows[ScreenEdit::file1_global_movie].choices.empty(); }
static bool EnabledIfSet1GlobalMovieSongGroup()		{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file1_type] == global_movie_song_group		&& !g_BackgroundChange.rows[ScreenEdit::file1_global_movie_song_group].choices.empty(); }
static bool EnabledIfSet1GlobalMovieSongGroupAndGenre() { return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file1_type] == global_movie_song_group_and_genre	&& !g_BackgroundChange.rows[ScreenEdit::file1_global_movie_song_group_and_genre].choices.empty(); }
static bool EnabledIfSet2SongBGAnimation()		{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file2_type] == song_bganimation			&& !g_BackgroundChange.rows[ScreenEdit::file2_song_bganimation].choices.empty(); }
static bool EnabledIfSet2SongMovie()			{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file2_type] == song_movie				&& !g_BackgroundChange.rows[ScreenEdit::file2_song_movie].choices.empty(); }
static bool EnabledIfSet2SongBitmap()			{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file2_type] == song_bitmap				&& !g_BackgroundChange.rows[ScreenEdit::file2_song_still].choices.empty(); }
static bool EnabledIfSet2GlobalBGAnimation()		{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file2_type] == global_bganimation			&& !g_BackgroundChange.rows[ScreenEdit::file2_global_bganimation].choices.empty(); }
static bool EnabledIfSet2GlobalMovie()			{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file2_type] == global_movie			&& !g_BackgroundChange.rows[ScreenEdit::file2_global_movie].choices.empty(); }
static bool EnabledIfSet2GlobalMovieSongGroup()		{ return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file2_type] == global_movie_song_group		&& !g_BackgroundChange.rows[ScreenEdit::file2_global_movie_song_group].choices.empty(); }
static bool EnabledIfSet2GlobalMovieSongGroupAndGenre() { return ScreenMiniMenu::s_viLastAnswers[ScreenEdit::file2_type] == global_movie_song_group_and_genre	&& !g_BackgroundChange.rows[ScreenEdit::file2_global_movie_song_group_and_genre].choices.empty(); }

static RString GetOneBakedRandomFile( Song *pSong, bool bTryGenre = true )
{
	vector<RString> vsPaths;
	vector<RString> vsNames;
	BackgroundUtil::GetGlobalRandomMovies(
		pSong,
		"",
		vsPaths,
		vsNames,
		bTryGenre );

	return vsNames.empty() ? RString() : vsNames[RandomInt(vsNames.size())];
}

static MenuDef g_InsertTapAttack(
	"ScreenMiniMenuInsertTapAttack",
	MenuRowDef(-1,
		"Duration seconds",
		true, EditMode_Practice, true, false, 3, "5","10","15","20","25","30","35","40","45" ), // TODO: Replace
	MenuRowDef(-1,
		"Set modifiers",
		true, EditMode_Practice, true, true, 0, "Press Start" )
);

static MenuDef g_InsertCourseAttack(
	"ScreenMiniMenuInsertCourseAttack",
	MenuRowDef(ScreenEdit::duration,
		"Duration seconds",
		true, EditMode_Practice, true, false, 3, "5","10","15","20","25","30","35","40","45" ),
	MenuRowDef(ScreenEdit::set_mods,
		"Set modifiers",
		true, EditMode_Practice, true, true, 0, "Press Start" ),
	MenuRowDef(ScreenEdit::remove,
		"Remove",
		true, EditMode_Practice, true, true, 0, "Press Start" )
);

static MenuDef g_InsertStepAttack(
	"ScreenMiniMenuInsertCourseAttack",
	MenuRowDef(ScreenEdit::sa_duration,
		"Duration seconds",
		true, EditMode_Practice, true, false, 3, "5","10","15","20","25","30","35","40","45" ),
	MenuRowDef(ScreenEdit::sa_set_mods,
		"Set modifiers",
		true, EditMode_Practice, true, true, 0, "Press Start" ),
	MenuRowDef(ScreenEdit::sa_remove,
		"Remove",
		true, EditMode_Practice, true, true, 0, "Press Start" ));

static MenuDef g_CourseMode(
	"ScreenMiniMenuCourseDisplay",
	MenuRowDef( -1, "Play mods from course",	true, EditMode_Practice, true, false, 0, nullptr )
);

// HACK: need to remember the track we're inserting on so that we can lay the
// attack note after coming back from menus.
static int g_iLastInsertTapAttackTrack = -1;
static float g_fLastInsertAttackDurationSeconds = -1;
static float g_fLastInsertAttackPositionSeconds = -1;
static BackgroundLayer g_CurrentBGChangeLayer = BACKGROUND_LAYER_Invalid;

static void SetDefaultEditorNoteSkin( size_t num, RString &sNameOut, RString &defaultValueOut )
{
	sNameOut = ssprintf( "EditorNoteSkinP%d", int(num + 1) );

	// XXX: We need more supported noteskins.
	defaultValueOut = "default";
}

static Preference1D<RString> EDITOR_NOTE_SKINS( SetDefaultEditorNoteSkin, NUM_PLAYERS );

static ThemeMetric<RString> EDIT_MODIFIERS		("ScreenEdit","EditModifiers");

static ThemeMetric<bool> LOOP_ON_CHART_END		("ScreenEdit","LoopOnChartEnd");

REGISTER_SCREEN_CLASS( ScreenEdit );

void ScreenEdit::Init()
{
	m_pSoundMusic = nullptr;

	GAMESTATE->m_bIsUsingStepTiming = false;
	GAMESTATE->m_bInStepEditor = true;

	SubscribeToMessage( "Judgment" );

	ASSERT( GAMESTATE->m_pCurSong != nullptr );
	ASSERT( GAMESTATE->m_pCurSteps[PLAYER_1] != nullptr );

	EDIT_MODE.Load( m_sName, "EditMode" );
	ScreenWithMenuElements::Init();

	InitEditMappings();

	currentCycleSegment = SEGMENT_LABEL;

	// save the originals for reverting later
	CopyToLastSave();

	m_CurrentAction = MAIN_MENU_CHOICE_INVALID;
	m_InputPlayerNumber = PLAYER_INVALID;

	if( GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StyleType == StyleType_TwoPlayersSharedSides )
		m_InputPlayerNumber = PLAYER_1;

	FOREACH_PlayerNumber( p )
		GAMESTATE->m_bSideIsJoined[p] = false;
	GAMESTATE->m_bSideIsJoined[PLAYER_1] = true;

	m_pSong = GAMESTATE->m_pCurSong;
	m_pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];

	/*	The user will most likely switch into Step Timing after laying down
		some initial notes. It also throws off many people at first glance.
	*/
	if( m_pSong->IsStepsUsingDifferentTiming(m_pSteps) )
		GAMESTATE->m_bIsUsingStepTiming = true;

	m_bReturnToRecordMenuAfterPlay = false;
	m_fBeatToReturnTo = 0;

	m_selectedTap = TAP_ORIGINAL_TAP;

	GAMESTATE->m_bGameplayLeadIn.Set( true );
	GAMESTATE->m_EditMode = EDIT_MODE.GetValue();
	SetBeat(0);
	m_fTrailingBeat = 0;

	m_iShiftAnchor = -1;
	m_iStartPlayingAt = -1;
	m_iStopPlayingAt = -1;

	attackInProcess = -1;
	modInProcess = -1;

	this->AddChild( &m_Background );

	m_SnapDisplay.SetXY( EDIT_X, PLAYER_Y_STANDARD );
	m_SnapDisplay.Load();
	// xxx: hardcoded command -aj
	m_SnapDisplay.SetZoom( SCREEN_HEIGHT/480*0.5 );
	this->AddChild( &m_SnapDisplay );

	// We keep track of this bit of state so that when the user is in Edit/Sync Songs and makes a change to the NoteSkins,
	// that change is "sticky" across multiple ScreenEdits.  That is the way the rest of the options work.
	// TODO: It would be cleaner to do this by making it possible to set an option in metrics.ini.
	if( !GAMESTATE->m_bDidModeChangeNoteSkin )
	{
		GAMESTATE->m_bDidModeChangeNoteSkin = true;
		FOREACH_PlayerNumber( pn )
		{
			const RString &sNoteSkin = EDITOR_NOTE_SKINS[pn].Get();
			if( NOTESKIN->DoesNoteSkinExist( sNoteSkin ) )
				PO_GROUP_ASSIGN( GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions,
				                 ModsLevel_Preferred, m_sNoteSkin, sNoteSkin );
		}
	}
	m_PlayerStateEdit.SetPlayerNumber(PLAYER_1);
	m_PlayerStateEdit.m_NotefieldZoom= 1.0f;
	// If we always go with the GAMESTATE NoteSkin, we will have fun effects
	// like Vivid or Flat in the editor notefield. This is not conducive to
	// productive editing.
	// todo: We should allow certain noteskins (note-colored/rhythm) to be
	// displayed. (Perhaps this should be a noteskin metric.) -aj
	if( NOTESKIN->DoesNoteSkinExist( EDITOR_NOTE_SKINS[PLAYER_1].Get() ) )
	{
		PO_GROUP_ASSIGN( m_PlayerStateEdit.m_PlayerOptions, ModsLevel_Stage, m_sNoteSkin, EDITOR_NOTE_SKINS[PLAYER_1].Get() );
	}
	else
	{
		PO_GROUP_ASSIGN( m_PlayerStateEdit.m_PlayerOptions, ModsLevel_Stage, m_sNoteSkin, GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetStage().m_sNoteSkin );
	}
	m_PlayerStateEdit.m_PlayerOptions.FromString( ModsLevel_Stage, EDIT_MODIFIERS );

	this->originalPlayerOptions.FromString(ModsLevel_Stage, EDIT_MODIFIERS);

	m_pSteps->GetNoteData( m_NoteDataEdit );
	m_NoteFieldEdit.SetXY( EDIT_X, EDIT_Y );
	m_NoteFieldEdit.SetZoom( SCREEN_HEIGHT/480*0.5 );
	m_NoteFieldEdit.Init( &m_PlayerStateEdit, PLAYER_HEIGHT*2, false );
	m_NoteFieldEdit.Load( &m_NoteDataEdit, -240, 850 );
	this->AddChild( &m_NoteFieldEdit );

	m_NoteDataRecord.SetNumTracks( m_NoteDataEdit.GetNumTracks() );
	m_NoteFieldRecord.SetXY( RECORD_X, RECORD_Y );
	m_NoteFieldRecord.Init( GAMESTATE->m_pPlayerState[PLAYER_1], PLAYER_HEIGHT );
	m_NoteFieldRecord.Load( &m_NoteDataRecord, -120, 425 );
	this->AddChild( &m_NoteFieldRecord );

	m_EditState = EditState_Invalid;
	TransitionEditState( STATE_EDITING );

	m_bRemoveNoteButtonDown = false;

	m_Clipboard.SetNumTracks( m_NoteDataEdit.GetNumTracks() );

	clipboardFullTiming = GAMESTATE->m_pCurSong->m_SongTiming; // always have a backup.
	clipboard_full_timing= &clipboardFullTiming;

	m_bHasUndo = false;
	m_Undo.SetNumTracks( m_NoteDataEdit.GetNumTracks() );

	SetDirty(m_NoteDataEdit.IsEmpty()); // require saving if empty.
	if(GAMESTATE->m_pCurSong->WasLoadedFromAutosave())
	{
		SetDirty(true);
	}

	m_Player->Init( "Player", GAMESTATE->m_pPlayerState[PLAYER_1], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr );
	m_Player->CacheAllUsedNoteSkins();
	GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerController = PC_HUMAN;
	m_Player->SetXY( PLAYER_X, PLAYER_Y );
	m_Player->SetZoom( SCREEN_HEIGHT/480 );
	this->AddChild( m_Player );

	this->AddChild( &m_Foreground );

	m_textInputTips.SetName( "EditHelp" );
	m_textInputTips.LoadFromFont( THEME->GetPathF("ScreenEdit","EditHelp") );
	m_textInputTips.SetText( EDIT_HELP_TEXT );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_textInputTips );
	this->AddChild( &m_textInputTips );

	m_textInfo.SetName( "Info" );
	m_textInfo.LoadFromFont( THEME->GetPathF("ScreenEdit","Info") );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_textInfo );
	this->AddChild( &m_textInfo );

	m_textPlayRecordHelp.SetName( "PlayRecordHelp" );
	m_textPlayRecordHelp.LoadFromFont( THEME->GetPathF("ScreenEdit","PlayRecordHelp") );
	m_textPlayRecordHelp.SetText( PLAY_RECORD_HELP_TEXT );
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_textPlayRecordHelp );
	this->AddChild( &m_textPlayRecordHelp );

	m_soundAddNote.Load(		THEME->GetPathS("ScreenEdit","AddNote"), true );
	m_soundRemoveNote.Load(		THEME->GetPathS("ScreenEdit","RemoveNote"), true );
	m_soundChangeLine.Load(		THEME->GetPathS("ScreenEdit","line"), true );
	m_soundChangeSnap.Load(		THEME->GetPathS("ScreenEdit","snap"), true );
	m_soundMarker.Load(		THEME->GetPathS("ScreenEdit","marker"), true );
	m_soundValueIncrease.Load(	THEME->GetPathS("ScreenEdit","value increase"), true );
	m_soundValueDecrease.Load(	THEME->GetPathS("ScreenEdit","value decrease"), true );
	m_soundSwitchPlayer.Load(	THEME->GetPathS("ScreenEdit","switch player"), true );
	m_soundSwitchTiming.Load(	THEME->GetPathS("ScreenEdit","switch timing"), true );
	m_soundSwitchSteps.Load(	THEME->GetPathS("ScreenEdit","switch steps") );
	m_soundSave.Load(		THEME->GetPathS("ScreenEdit","save") );
	m_GameplayAssist.Init();

	m_AutoKeysounds.FinishLoading();
	m_pSoundMusic = m_AutoKeysounds.GetSound();

	this->HandleScreenMessage( SM_UpdateTextInfo );
	m_bTextInfoNeedsUpdate = true;

	SubscribeToMessage( Message_SongModified );
}

ScreenEdit::~ScreenEdit()
{
	// UGLY: Don't delete the Song's steps.
	m_SongLastSave.DetachSteps();

	LOG->Trace( "ScreenEdit::~ScreenEdit()" );
	m_pSoundMusic->StopPlaying();

	// Go back to Step Timing on leave.
	GAMESTATE->m_bIsUsingStepTiming = true;
	// DEFINITELY reset the InStepEditor variable.
	GAMESTATE->m_bInStepEditor = false;
}

void ScreenEdit::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();

	/* We do this ourself. */
	SOUND->HandleSongTimer( false );
}

void ScreenEdit::EndScreen()
{
	ScreenWithMenuElements::EndScreen();

	SOUND->HandleSongTimer( true );
}

// play assist ticks
void ScreenEdit::PlayTicks()
{
	if( m_EditState != STATE_PLAYING )
		return;

	m_GameplayAssist.PlayTicks( m_Player->GetNoteData(), m_Player->GetPlayerState() );
}

static ThemeMetric<float> FADE_IN_PREVIEW("ScreenEdit", "FadeInPreview");
static ThemeMetric<float> FADE_OUT_PREVIEW("ScreenEdit", "FadeOutPreview");

void ScreenEdit::PlayPreviewMusic()
{
	SOUND->StopMusic();
	SOUND->PlayMusic(
		m_pSong->GetPreviewMusicPath(),
		nullptr,
		false,
		m_pSong->GetPreviewStartSeconds(),
		m_pSong->m_fMusicSampleLengthSeconds,
		FADE_IN_PREVIEW,
		FADE_OUT_PREVIEW );
}

void ScreenEdit::MakeFilteredMenuDef( const MenuDef* pDef, MenuDef &menu )
{
	menu = *pDef;
	menu.rows.clear();

	vector<MenuRowDef> aRows;
	for (MenuRowDef const &r : pDef->rows)
	{
		// Don't add rows that aren't applicable to this edit mode.
		if( EDIT_MODE >= r.emShowIn )
			menu.rows.push_back( r );
	}
}

void ScreenEdit::EditMiniMenu( const MenuDef* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel )
{
	// Reload options.
	MenuDef menu("");
	MakeFilteredMenuDef( pDef, menu );
	ScreenMiniMenu::MiniMenu( &menu, SM_SendOnOK, SM_SendOnCancel );
}

void ScreenEdit::Update( float fDeltaTime )
{
	m_PlayerStateEdit.Update( fDeltaTime );

	const float fRate = PREFSMAN->m_bRateModsAffectTweens ? GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate : 1.0f;

	if( m_pSoundMusic->IsPlaying() )
	{
		RageTimer tm;
		const float fSeconds = m_pSoundMusic->GetPositionSeconds( nullptr, &tm );
		GAMESTATE->UpdateSongPosition( fSeconds, GAMESTATE->m_pCurSong->m_SongTiming, tm );
	}

	if(m_EditState == STATE_EDITING)
	{
		if(IsDirty() && m_next_autosave_time > -1.0f &&
			RageTimer::GetTimeSinceStartFast() > m_next_autosave_time)
		{
			PerformSave(true);
		}
	}


	if( m_EditState == STATE_RECORDING  )
	{
		// TODO: Find a way to prevent STATE_RECORDING when in Song Timing.

		for( int t=0; t<GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_iColsPerPlayer; t++ )	// for each track
		{
			vector<GameInput> GameI;
			GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->StyleInputToGameInput( t, PLAYER_1, GameI );
			float fSecsHeld= 0.0f;
			for(size_t i= 0; i < GameI.size(); ++i)
			{
				fSecsHeld= max(fSecsHeld, INPUTMAPPER->GetSecsHeld(GameI[i]));
			}
			fSecsHeld = min( fSecsHeld, m_RemoveNoteButtonLastChanged.Ago() );
			if( fSecsHeld == 0 )
				continue;

			// TODO: Currently, this will ignore a hold note that's
			// held past the end of selection.  Ideally the hold
			// note should be created, ending right at the end of
			// selection (or appropriate quantization).
			float fStartPlayingAtBeat = NoteRowToBeat(m_iStartPlayingAt);
			float fStopPlayingAtBeat = NoteRowToBeat(m_iStopPlayingAt);
			if( GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat <= fStartPlayingAtBeat ||
			    GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat >= fStopPlayingAtBeat )
				continue;

			float fStartedHoldingSeconds = m_pSoundMusic->GetPositionSeconds() - fSecsHeld;
			float fStartBeat = max( fStartPlayingAtBeat, m_pSteps->GetTimingData()->GetBeatFromElapsedTime(fStartedHoldingSeconds) );
			float fEndBeat = max( fStartBeat, GetBeat() );
			fEndBeat = min( fEndBeat, fStopPlayingAtBeat );

			// Round start and end to the nearest snap interval
			fStartBeat = Quantize( fStartBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );
			fEndBeat = Quantize( fEndBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );

			if( m_bRemoveNoteButtonDown )
			{
				m_NoteDataRecord.ClearRangeForTrack( BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat), t );
			}
			else if( fSecsHeld > record_hold_seconds )
			{
				// create or extend a hold or roll note
				TapNote tn = EditIsBeingPressed(EDIT_BUTTON_LAY_ROLL) ? TAP_ORIGINAL_ROLL_HEAD: TAP_ORIGINAL_HOLD_HEAD;

				tn.pn = m_InputPlayerNumber;
				m_NoteDataRecord.AddHoldNote( t, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat), tn );
			}
		}
	}

	//
	// check for end of playback/record
	//
	if( m_EditState == STATE_RECORDING  ||  m_EditState == STATE_PLAYING )
	{
		ArrowEffects::Update();
		/*
		 * If any arrow is being held, continue for up to half a second after
		 * the end marker.  This makes it possible to start a hold note near
		 * the end of the range.  We won't allow placing anything outside of the
		 * range.
		 */
		bool bButtonIsBeingPressed = false;
		for( int t=0; t<GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_iColsPerPlayer; t++ )	// for each track
		{
			vector<GameInput> GameI;
			GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->StyleInputToGameInput( t, PLAYER_1, GameI );
			if( INPUTMAPPER->IsBeingPressed(GameI) )
				bButtonIsBeingPressed = true;
		}

		float fLastBeat = NoteRowToBeat(m_iStopPlayingAt);
		if( bButtonIsBeingPressed && m_EditState == STATE_RECORDING )
		{
			float fSeconds = m_pSteps->GetTimingData()->GetElapsedTimeFromBeat( fLastBeat );
			fLastBeat = m_pSteps->GetTimingData()->GetBeatFromElapsedTime( fSeconds + 0.5f );
		}

		float fStopAtSeconds = m_pSteps->GetTimingData()->GetElapsedTimeFromBeat( NoteRowToBeat(m_iStopPlayingAt) ) + 1;
		if( GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fMusicSeconds > fStopAtSeconds )
		{
			TransitionEditState( ( LOOP_ON_CHART_END ? STATE_PLAYING : STATE_EDITING ) );
		}
	}

	//LOG->Trace( "ScreenEdit::Update(%f)", fDeltaTime );
	if( m_EditState == STATE_PLAYING )
	{
		ScreenWithMenuElements::Update( fDeltaTime * fRate );
	}
	else
	{
		ScreenWithMenuElements::Update( fDeltaTime );
	}


	// Update trailing beat
	float fDelta = GetBeat() - m_fTrailingBeat;
	if( fabsf(fDelta) < 10 )
		fapproach( m_fTrailingBeat, GetBeat(),
			fDeltaTime*40 / m_NoteFieldEdit.GetPlayerState()->m_PlayerOptions.GetCurrent().m_fScrollSpeed );
	else
		fapproach( m_fTrailingBeat, GetBeat(),
			fabsf(fDelta) * fDeltaTime*5 );

	PlayTicks();
}

static vector<int> FindAllAttacksAtTime(const AttackArray& attacks, float fStartTime)
{
	vector<int> ret;
	for (unsigned i = 0; i < attacks.size(); ++i)
	{
		if (fabs(attacks[i].fStartSecond - fStartTime) < 0.001f)
		{
			ret.push_back(i);
		}
	}
	return ret;
}

static int FindAttackAtTime( const AttackArray& attacks, float fStartTime )
{
	for( unsigned i = 0; i < attacks.size(); ++i )
	{
		if( fabs(attacks[i].fStartSecond - fStartTime) < 0.001f )
			return i;
	}
	return -1;
}

// TODO: Fix this mess. -Colby
static LocalizedString CURRENT_BEAT("ScreenEdit", "Current beat");
static LocalizedString CURRENT_SECOND("ScreenEdit", "Current second");
static LocalizedString SNAP_TO("ScreenEdit", "Snap to");
static LocalizedString NOTES("ScreenEdit", "%s notes");
static LocalizedString SELECTION_BEAT("ScreenEdit", "Selection beat");
static LocalizedString DIFFICULTY("ScreenEdit", "Difficulty");
static LocalizedString ROUTINE_PLAYER("ScreenEdit", "Routine Player");
static LocalizedString CHART_NAME("ScreenEdit", "Chart Name");
static LocalizedString DESCRIPTION("ScreenEdit", "Description");
static LocalizedString CHART_STYLE("ScreenEdit", "Chart Style");
static LocalizedString STEP_AUTHOR("ScreenEdit", "Step Author");
static LocalizedString MAIN_TITLE("ScreenEdit", "Main title");
static LocalizedString SUBTITLE("ScreenEdit", "Subtitle");
static LocalizedString TAP_NOTE_TYPE("ScreenEdit", "Tap Note");
static LocalizedString SEGMENT_TYPE("ScreenEdit", "Segment");
static LocalizedString TAP_STEPS("ScreenEdit", "Tap Steps");
static LocalizedString JUMPS("ScreenEdit", "Jumps");
static LocalizedString HANDS("ScreenEdit", "Hands");
static LocalizedString HOLDS("ScreenEdit", "Holds");
static LocalizedString MINES("ScreenEdit", "Mines");
static LocalizedString ROLLS("ScreenEdit", "Rolls");
static LocalizedString LIFTS("ScreenEdit", "Lifts");
static LocalizedString FAKES("ScreenEdit", "Fakes");
static LocalizedString TIMING_MODE("ScreenEdit", "Timing Mode");
static LocalizedString STEP_TIMING("ScreenEdit", "Step Timing");
static LocalizedString SONG_TIMING("ScreenEdit", "Song Timing");
static LocalizedString BEAT_0_OFFSET("ScreenEdit", "Beat 0 offset");
static LocalizedString PREVIEW_START("ScreenEdit", "Preview Start");
static LocalizedString PREVIEW_LENGTH("ScreenEdit", "Preview Length");
static LocalizedString RECORD_HOLD_TIME("ScreenEdit", "Record Hold Time");

static ThemeMetric<RString> CURRENT_BEAT_FORMAT("ScreenEdit", "CurrentBeatFormat");
static ThemeMetric<RString> CURRENT_SECOND_FORMAT("ScreenEdit", "CurrentSecondFormat");
static ThemeMetric<RString> SNAP_TO_FORMAT("ScreenEdit", "SnapToFormat");
static ThemeMetric<RString> SELECTION_BEAT_BEGIN_FORMAT("ScreenEdit", "SelectionBeatBeginFormat");
static ThemeMetric<RString> SELECTION_BEAT_UNFINISHED_FORMAT("ScreenEdit", "SelectionBeatUnfinishedFormat");
static ThemeMetric<RString> SELECTION_BEAT_END_FORMAT("ScreenEdit", "SelectionBeatEndFormat");
static ThemeMetric<RString> DIFFICULTY_FORMAT("ScreenEdit", "DifficultyFormat");
static ThemeMetric<RString> ROUTINE_PLAYER_FORMAT("ScreenEdit", "RoutinePlayerFormat");
static ThemeMetric<RString> CHART_NAME_FORMAT("ScreenEdit", "ChartNameFormat");
static ThemeMetric<RString> DESCRIPTION_FORMAT("ScreenEdit", "DescriptionFormat");
static ThemeMetric<RString> CHART_STYLE_FORMAT("ScreenEdit", "ChartStyleFormat");
static ThemeMetric<RString> STEP_AUTHOR_FORMAT("ScreenEdit", "StepAuthorFormat");
static ThemeMetric<RString> MAIN_TITLE_FORMAT("ScreenEdit", "MainTitleFormat");
static ThemeMetric<RString> SUBTITLE_FORMAT("ScreenEdit", "SubtitleFormat");
static ThemeMetric<RString> TAP_NOTE_TYPE_FORMAT("ScreenEdit", "TapNoteTypeFormat");
static ThemeMetric<RString> SEGMENT_TYPE_FORMAT("ScreenEdit", "SegmentTypeFormat");
static ThemeMetric<RString> NUM_STEPS_FORMAT("ScreenEdit", "NumStepsFormat");
static ThemeMetric<RString> NUM_JUMPS_FORMAT("ScreenEdit", "NumJumpsFormat");
static ThemeMetric<RString> NUM_HOLDS_FORMAT("ScreenEdit", "NumHoldsFormat");
static ThemeMetric<RString> NUM_MINES_FORMAT("ScreenEdit", "NumMinesFormat");
static ThemeMetric<RString> NUM_HANDS_FORMAT("ScreenEdit", "NumHandsFormat");
static ThemeMetric<RString> NUM_ROLLS_FORMAT("ScreenEdit", "NumRollsFormat");
static ThemeMetric<RString> NUM_LIFTS_FORMAT("ScreenEdit", "NumLiftsFormat");
static ThemeMetric<RString> NUM_FAKES_FORMAT("ScreenEdit", "NumFakesFormat");
static ThemeMetric<RString> NUM_STEPS_FORMAT_TWO_PLAYER("ScreenEdit", "NumStepsFormatTwoPlayer");
static ThemeMetric<RString> NUM_JUMPS_FORMAT_TWO_PLAYER("ScreenEdit", "NumJumpsFormatTwoPlayer");
static ThemeMetric<RString> NUM_HOLDS_FORMAT_TWO_PLAYER("ScreenEdit", "NumHoldsFormatTwoPlayer");
static ThemeMetric<RString> NUM_MINES_FORMAT_TWO_PLAYER("ScreenEdit", "NumMinesFormatTwoPlayer");
static ThemeMetric<RString> NUM_HANDS_FORMAT_TWO_PLAYER("ScreenEdit", "NumHandsFormatTwoPlayer");
static ThemeMetric<RString> NUM_ROLLS_FORMAT_TWO_PLAYER("ScreenEdit", "NumRollsFormatTwoPlayer");
static ThemeMetric<RString> NUM_LIFTS_FORMAT_TWO_PLAYER("ScreenEdit", "NumLiftsFormatTwoPlayer");
static ThemeMetric<RString> NUM_FAKES_FORMAT_TWO_PLAYER("ScreenEdit", "NumFakesFormatTwoPlayer");
static ThemeMetric<RString> TIMING_MODE_FORMAT("ScreenEdit", "TimingModeFormat");
static ThemeMetric<RString> BEAT_0_OFFSET_FORMAT("ScreenEdit", "Beat0OffsetFormat");
static ThemeMetric<RString> PREVIEW_START_FORMAT("ScreenEdit", "PreviewStartFormat");
static ThemeMetric<RString> PREVIEW_LENGTH_FORMAT("ScreenEdit", "PreviewLengthFormat");
static ThemeMetric<RString> RECORD_HOLD_TIME_FORMAT("ScreenEdit", "RecordHoldTimeFormat");
void ScreenEdit::UpdateTextInfo()
{
	if( m_pSteps == nullptr )
		return;

	// Don't update the text during playback or record.  It causes skips.
	if( m_EditState != STATE_EDITING )
		return;

	if( !m_bTextInfoNeedsUpdate )
		return;

	m_bTextInfoNeedsUpdate = false;

	RString sNoteType = ssprintf( NOTES.GetValue(), NoteTypeToLocalizedString(m_SnapDisplay.GetNoteType()).c_str() );

	RString sText;
	sText += ssprintf( CURRENT_BEAT_FORMAT.GetValue(), CURRENT_BEAT.GetValue().c_str(), GetBeat() );
	float second= GetAppropriateTiming().GetElapsedTimeFromBeatNoOffset(GetBeat());
	sText += ssprintf( CURRENT_SECOND_FORMAT.GetValue(), CURRENT_SECOND.GetValue().c_str(), second );
	switch( EDIT_MODE.GetValue() )
	{
	DEFAULT_FAIL( EDIT_MODE.GetValue() );
	case EditMode_Practice:
		break;
	case EditMode_CourseMods:
	case EditMode_Home:
	case EditMode_Full:
		sText += ssprintf( SNAP_TO_FORMAT.GetValue(), SNAP_TO.GetValue().c_str(), sNoteType.c_str() );
		break;
	}

	if( m_NoteFieldEdit.m_iBeginMarker != -1 )
	{
		sText += ssprintf( SELECTION_BEAT_BEGIN_FORMAT.GetValue(), SELECTION_BEAT.GetValue().c_str(), NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker) );

		if( m_NoteFieldEdit.m_iEndMarker != -1 )
			sText += ssprintf( SELECTION_BEAT_END_FORMAT.GetValue(), NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker) );
		else
			sText += SELECTION_BEAT_UNFINISHED_FORMAT;
	}

	if (EDIT_MODE.GetValue() == EditMode_Full)
	{
		sText += ssprintf( DIFFICULTY_FORMAT.GetValue(), DIFFICULTY.GetValue().c_str(), DifficultyToString( m_pSteps->GetDifficulty() ).c_str() );
		if ( m_InputPlayerNumber != PLAYER_INVALID )
			sText += ssprintf( ROUTINE_PLAYER_FORMAT.GetValue(), ROUTINE_PLAYER.GetValue().c_str(), m_InputPlayerNumber + 1 );
		//sText += ssprintf( DESCRIPTION_FORMAT.GetValue(), DESCRIPTION.GetValue().c_str(), m_pSteps->GetDescription().c_str() );
		sText += ssprintf( CHART_NAME_FORMAT.GetValue(), CHART_NAME.GetValue().c_str(), m_pSteps->GetChartName().c_str() );
		sText += ssprintf( STEP_AUTHOR_FORMAT.GetValue(), STEP_AUTHOR.GetValue().c_str(), m_pSteps->GetCredit().c_str() );
		//sText += ssprintf( CHART_STYLE_FORMAT.GetValue(), CHART_STYLE.GetValue().c_str(), m_pSteps->GetChartStyle().c_str() );
		sText += ssprintf( MAIN_TITLE_FORMAT.GetValue(), MAIN_TITLE.GetValue().c_str(), m_pSong->m_sMainTitle.c_str() );
		if( m_pSong->m_sSubTitle.size() )
			sText += ssprintf( SUBTITLE_FORMAT.GetValue(), SUBTITLE.GetValue().c_str(), m_pSong->m_sSubTitle.c_str() );
		sText += ssprintf( SEGMENT_TYPE_FORMAT.GetValue(), SEGMENT_TYPE.GetValue().c_str(), TimingSegmentTypeToString(currentCycleSegment).c_str() );
        const RString tapnoteType = TapNoteTypeToString( m_selectedTap.type );
		sText += ssprintf( TAP_NOTE_TYPE_FORMAT.GetValue(), TAP_NOTE_TYPE.GetValue().c_str(), tapnoteType.c_str() );

		AttackArray &attacks =
			(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
		float beat = GetAppropriateTiming().GetElapsedTimeFromBeat(GetBeat());
		sText += ssprintf("Attack here?: %s\n", FindAttackAtTime(attacks, beat) > -1 ? "YES" : "NO");
	}

	GAMESTATE->SetProcessedTimingData(m_pSteps->GetTimingData());
	const StepsTypeCategory &cat = GAMEMAN->GetStepsTypeInfo(m_pSteps->m_StepsType).m_StepsTypeCategory;
	if (cat == StepsTypeCategory_Couple || cat == StepsTypeCategory_Routine)
	{
		pair<int, int> tmp = m_NoteDataEdit.GetNumTapNotesTwoPlayer();
		sText += ssprintf(NUM_STEPS_FORMAT_TWO_PLAYER.GetValue(),
						  TAP_STEPS.GetValue().c_str(),
						  tmp.first, tmp.second);
		tmp = m_NoteDataEdit.GetNumJumpsTwoPlayer();
		sText += ssprintf(NUM_JUMPS_FORMAT_TWO_PLAYER.GetValue(),
						  JUMPS.GetValue().c_str(),
						  tmp.first, tmp.second);
		tmp = m_NoteDataEdit.GetNumHandsTwoPlayer();
		sText += ssprintf(NUM_HANDS_FORMAT_TWO_PLAYER.GetValue(),
						  HANDS.GetValue().c_str(),
						  tmp.first, tmp.second);
		tmp = m_NoteDataEdit.GetNumHoldNotesTwoPlayer();
		sText += ssprintf(NUM_HOLDS_FORMAT_TWO_PLAYER.GetValue(),
						  HOLDS.GetValue().c_str(),
						  tmp.first, tmp.second);
		tmp = m_NoteDataEdit.GetNumMinesTwoPlayer();
		sText += ssprintf(NUM_MINES_FORMAT_TWO_PLAYER.GetValue(),
						  MINES.GetValue().c_str(),
						  tmp.first, tmp.second);
		tmp = m_NoteDataEdit.GetNumRollsTwoPlayer();
		sText += ssprintf(NUM_ROLLS_FORMAT_TWO_PLAYER.GetValue(),
						  ROLLS.GetValue().c_str(),
						  tmp.first, tmp.second);
		tmp = m_NoteDataEdit.GetNumLiftsTwoPlayer();
		sText += ssprintf(NUM_LIFTS_FORMAT_TWO_PLAYER.GetValue(),
						  LIFTS.GetValue().c_str(),
						  tmp.first, tmp.second);
		tmp = m_NoteDataEdit.GetNumFakesTwoPlayer();
		sText += ssprintf(NUM_FAKES_FORMAT_TWO_PLAYER.GetValue(),
						  FAKES.GetValue().c_str(),
						  tmp.first, tmp.second);
	}
	else
	{
		sText += ssprintf( NUM_STEPS_FORMAT.GetValue(), TAP_STEPS.GetValue().c_str(), m_NoteDataEdit.GetNumTapNotes() );
		sText += ssprintf( NUM_JUMPS_FORMAT.GetValue(), JUMPS.GetValue().c_str(), m_NoteDataEdit.GetNumJumps() );
		sText += ssprintf( NUM_HANDS_FORMAT.GetValue(), HANDS.GetValue().c_str(), m_NoteDataEdit.GetNumHands() );
		sText += ssprintf( NUM_HOLDS_FORMAT.GetValue(), HOLDS.GetValue().c_str(), m_NoteDataEdit.GetNumHoldNotes() );
		sText += ssprintf( NUM_MINES_FORMAT.GetValue(), MINES.GetValue().c_str(), m_NoteDataEdit.GetNumMines() );
		sText += ssprintf( NUM_ROLLS_FORMAT.GetValue(), ROLLS.GetValue().c_str(), m_NoteDataEdit.GetNumRolls() );
		sText += ssprintf( NUM_LIFTS_FORMAT.GetValue(), LIFTS.GetValue().c_str(), m_NoteDataEdit.GetNumLifts() );
		sText += ssprintf( NUM_FAKES_FORMAT.GetValue(), FAKES.GetValue().c_str(), m_NoteDataEdit.GetNumFakes() );
	}
	switch( EDIT_MODE.GetValue() )
	{
	DEFAULT_FAIL( EDIT_MODE.GetValue() );
	case EditMode_Practice:
	case EditMode_CourseMods:
	case EditMode_Home:
		break;
	case EditMode_Full:
		sText += ssprintf( TIMING_MODE_FORMAT.GetValue(),
				  TIMING_MODE.GetValue().c_str(),
				  ( GAMESTATE->m_bIsUsingStepTiming ?
				   STEP_TIMING.GetValue().c_str() :
				   SONG_TIMING.GetValue().c_str() ) );
		sText += ssprintf( BEAT_0_OFFSET_FORMAT.GetValue(),
				  BEAT_0_OFFSET.GetValue().c_str(),
				  GetAppropriateTiming().m_fBeat0OffsetInSeconds );
		sText += ssprintf( PREVIEW_START_FORMAT.GetValue(), PREVIEW_START.GetValue().c_str(), m_pSong->m_fMusicSampleStartSeconds );
		sText += ssprintf( PREVIEW_LENGTH_FORMAT.GetValue(), PREVIEW_LENGTH.GetValue().c_str(), m_pSong->m_fMusicSampleLengthSeconds );
		if(record_hold_seconds < record_hold_default - .001f ||
			record_hold_seconds > record_hold_default + .001f)
		{
			sText += ssprintf(RECORD_HOLD_TIME_FORMAT.GetValue(), RECORD_HOLD_TIME.GetValue().c_str(), record_hold_seconds);
		}
		break;
	}

	m_textInfo.SetText( sText );

	GAMESTATE->SetProcessedTimingData(nullptr);
}

void ScreenEdit::DrawPrimitives()
{

	if( m_pSoundMusic->IsPlaying() )
	{
		ScreenWithMenuElements::DrawPrimitives();
		return;
	}

	// HACK:  Draw using the trailing beat
	PlayerState *pPlayerState = const_cast<PlayerState *> (m_NoteFieldEdit.GetPlayerState());

	float fPlayerSongBeat = pPlayerState->m_Position.m_fSongBeat;	// save song beat
	float fPlayerSongBeatNoOffset = pPlayerState->m_Position.m_fSongBeatNoOffset;
	float fPlayerSongBeatVisible = pPlayerState->m_Position.m_fSongBeatVisible;

	float fGameSongBeat = GAMESTATE->m_Position.m_fSongBeat;	// save song beat
	float fGameSongBeatNoOffset = GAMESTATE->m_Position.m_fSongBeatNoOffset;
	float fGameSongBeatVisible = GAMESTATE->m_Position.m_fSongBeatVisible;

	pPlayerState->m_Position.m_fSongBeat = m_fTrailingBeat;	// put trailing beat in effect
	pPlayerState->m_Position.m_fSongBeatNoOffset = m_fTrailingBeat;	// put trailing beat in effect
	pPlayerState->m_Position.m_fSongBeatVisible = m_fTrailingBeat;	// put trailing beat in effect

	GAMESTATE->m_Position.m_fSongBeat = m_fTrailingBeat;	// put trailing beat in effect
	GAMESTATE->m_Position.m_fSongBeatNoOffset = m_fTrailingBeat;	// put trailing beat in effect
	GAMESTATE->m_Position.m_fSongBeatVisible = m_fTrailingBeat;	// put trailing beat in effect

	ScreenWithMenuElements::DrawPrimitives();

	pPlayerState->m_Position.m_fSongBeat = fPlayerSongBeat;	// restore real song beat
	pPlayerState->m_Position.m_fSongBeatNoOffset = fPlayerSongBeatNoOffset;
	pPlayerState->m_Position.m_fSongBeatVisible = fPlayerSongBeatVisible;

	GAMESTATE->m_Position.m_fSongBeat = fGameSongBeat;	// restore real song beat
	GAMESTATE->m_Position.m_fSongBeatNoOffset = fGameSongBeatNoOffset;
	GAMESTATE->m_Position.m_fSongBeatVisible = fGameSongBeatVisible;

}

bool ScreenEdit::Input( const InputEventPlus &input )
{
//	LOG->Trace( "ScreenEdit::Input()" );

	// invalidate input if cmd/meta is being held.
	if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LMETA)) ||
	    INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RMETA)) )
		return false;

	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return false;

	EditButton EditB = DeviceToEdit( input.DeviceI );
	if( EditB == EditButton_Invalid )
		EditB = MenuButtonToEditButton( input.MenuI );

	if( EditB == EDIT_BUTTON_REMOVE_NOTE )
	{
		/* Ugly: we need to know when the button was pressed or released, so we
		 * can clamp operations to that time.  Should InputFilter keep track of
		 * last release, too? */
		m_bRemoveNoteButtonDown = (input.type != IET_RELEASE);
		m_RemoveNoteButtonLastChanged.Touch();
	}

	switch( m_EditState )
	{
	DEFAULT_FAIL( m_EditState );
	case STATE_EDITING:
		m_bTextInfoNeedsUpdate = true;
		return InputEdit( input, EditB );
	case STATE_RECORDING:
		return InputRecord( input, EditB );
	case STATE_RECORDING_PAUSED:
		return InputRecordPaused( input, EditB );
	case STATE_PLAYING:
		return InputPlay( input, EditB );
	}
}

static void ShiftToRightSide( int &iCol, int iNumTracks )
{
	switch( GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StyleType )
	{
	DEFAULT_FAIL( GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StyleType );
	case StyleType_OnePlayerOneSide:
		break;
	case StyleType_TwoPlayersTwoSides:
	case StyleType_OnePlayerTwoSides:
	case StyleType_TwoPlayersSharedSides:
		iCol += iNumTracks/2;
		break;
	}
}

static LocalizedString BG_CHANGE_STEP_TIMING	( "ScreenEdit", "You must be in Song Timing Mode to edit BG Changes." );
static LocalizedString ALTER_MENU_NO_SELECTION	( "ScreenEdit", "You must have an area selected to enter the Alter Menu." );
static LocalizedString SWITCHED_TO		( "ScreenEdit", "Switched to" );
static LocalizedString NO_BACKGROUNDS_AVAILABLE	( "ScreenEdit", "No backgrounds available" );
static ThemeMetric<bool> INVERT_SCROLL_BUTTONS	( "ScreenEdit", "InvertScrollSpeedButtons" );

bool ScreenEdit::InputEdit( const InputEventPlus &input, EditButton EditB )
{
	if( input.type == IET_RELEASE )
	{
		if( EditPressed( EDIT_BUTTON_SCROLL_SELECT, input.DeviceI ) )
			m_iShiftAnchor = -1;
		// XXX Key releases usually don't count as "handled," but what
		// does it mean in this case?
		return false;
	}
	const TimingData &sTiming = GetAppropriateTiming();
	float playerBeat = GetAppropriatePosition().m_fSongBeat;
	int beatsPerMeasure = sTiming.GetTimeSignatureSegmentAtBeat( playerBeat )->GetNum();

	switch( EditB )
	{
	case EDIT_BUTTON_COLUMN_0:
	case EDIT_BUTTON_COLUMN_1:
	case EDIT_BUTTON_COLUMN_2:
	case EDIT_BUTTON_COLUMN_3:
	case EDIT_BUTTON_COLUMN_4:
	case EDIT_BUTTON_COLUMN_5:
	case EDIT_BUTTON_COLUMN_6:
	case EDIT_BUTTON_COLUMN_7:
	case EDIT_BUTTON_COLUMN_8:
	case EDIT_BUTTON_COLUMN_9:
		{
			if( input.type != IET_FIRST_PRESS )
				return false;	// We only care about first presses

			int iCol = EditB - EDIT_BUTTON_COLUMN_0;


			// Alt + number = input to right half
			if( EditIsBeingPressed(EDIT_BUTTON_RIGHT_SIDE) )
				ShiftToRightSide( iCol, m_NoteDataEdit.GetNumTracks() );

			if( iCol >= m_NoteDataEdit.GetNumTracks() )
				return false; // this button is not in the range of columns for this Style

			const float fSongBeat = GetBeat();
			const int iSongIndex = BeatToNoteRow( fSongBeat );

			// check for to see if the user intended to remove a HoldNote
			int iHeadRow;
			if( m_NoteDataEdit.IsHoldNoteAtRow( iCol, iSongIndex, &iHeadRow ) )
			{
				m_soundRemoveNote.Play(true);
				SetDirty( true );
				SaveUndo();
				m_NoteDataEdit.SetTapNote( iCol, iHeadRow, TAP_EMPTY );
				// Don't CheckNumberOfNotesAndUndo.  We don't want to revert any change that removes notes.
			}
			else if( m_NoteDataEdit.GetTapNote(iCol, iSongIndex).type != TapNoteType_Empty )
			{
				m_soundRemoveNote.Play(true);
				SetDirty( true );
				SaveUndo();
				m_NoteDataEdit.SetTapNote( iCol, iSongIndex, TAP_EMPTY );
				// Don't CheckNumberOfNotesAndUndo.  We don't want to revert any change that removes notes.
			}
			else if( EditIsBeingPressed(EDIT_BUTTON_LAY_TAP_ATTACK) )
			{
				g_iLastInsertTapAttackTrack = iCol;
				EditMiniMenu( &g_InsertTapAttack, SM_BackFromInsertTapAttack );
			}
			else
			{
				m_soundAddNote.Play(true);
				SetDirty( true );
				SaveUndo();
				TapNote tn = m_selectedTap;
				tn.pn = m_InputPlayerNumber;
				m_NoteDataEdit.SetTapNote(iCol, iSongIndex, tn );
				CheckNumberOfNotesAndUndo();
			}
		}
		return true;

	case EDIT_BUTTON_CYCLE_TAP_LEFT:
		{
			switch ( m_selectedTap.type )
			{
				case TapNoteType_Tap:	m_selectedTap = TAP_ORIGINAL_FAKE;	break;
				case TapNoteType_Mine:	m_selectedTap = TAP_ORIGINAL_TAP;	break;
				case TapNoteType_Lift:	m_selectedTap = TAP_ORIGINAL_MINE;	break;
				case TapNoteType_Fake:	m_selectedTap = TAP_ORIGINAL_LIFT;	break;
				DEFAULT_FAIL( m_selectedTap.type );
			}
			return true;
		}
	case EDIT_BUTTON_CYCLE_TAP_RIGHT:
		{
			switch ( m_selectedTap.type )
			{
				case TapNoteType_Tap:	m_selectedTap = TAP_ORIGINAL_MINE;	break;
				case TapNoteType_Mine:	m_selectedTap = TAP_ORIGINAL_LIFT;	break;
				case TapNoteType_Lift:	m_selectedTap = TAP_ORIGINAL_FAKE;	break;
				case TapNoteType_Fake:	m_selectedTap = TAP_ORIGINAL_TAP;	break;
				DEFAULT_FAIL( m_selectedTap.type );
			}
			return true;
		}
	case EDIT_BUTTON_CYCLE_SEGMENT_LEFT:
	{
		int tmp = enum_add2( this->currentCycleSegment, -1 );
		wrap( *ConvertValue<int>(&tmp), NUM_TimingSegmentType );
		currentCycleSegment = (TimingSegmentType)tmp;
		return true;
	}
	case EDIT_BUTTON_CYCLE_SEGMENT_RIGHT:
	{
		int tmp = enum_add2( this->currentCycleSegment, +1 );
		wrap( *ConvertValue<int>(&tmp), NUM_TimingSegmentType );
		currentCycleSegment = (TimingSegmentType)tmp;
		return true;
	}
	case EDIT_BUTTON_SCROLL_SPEED_UP:
	case EDIT_BUTTON_SCROLL_SPEED_DOWN:
		{
			PlayerState *pPlayerState = const_cast<PlayerState *> (m_NoteFieldEdit.GetPlayerState());
			float fScrollSpeed = pPlayerState->m_PlayerOptions.GetSong().m_fScrollSpeed;

			const float fSpeeds[] = { 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 8.0f };
			int iSpeed = 0;
			for( unsigned i = 0; i < ARRAYLEN(fSpeeds); ++i )
			{
				if( fSpeeds[i] == fScrollSpeed )
				{
					iSpeed = i;
					break;
				}
			}

			switch( EditB )
			{
			DEFAULT_FAIL(EditB);
			case EDIT_BUTTON_SCROLL_SPEED_DOWN:
				INVERT_SCROLL_BUTTONS ? ++iSpeed : --iSpeed;
				break;
			case EDIT_BUTTON_SCROLL_SPEED_UP:
				INVERT_SCROLL_BUTTONS ? --iSpeed : ++iSpeed;
				break;
			}
			iSpeed = clamp( iSpeed, 0, (int) ARRAYLEN(fSpeeds)-1 );

			if( fSpeeds[iSpeed] != fScrollSpeed )
			{
				m_soundMarker.Play(true);
				fScrollSpeed = fSpeeds[iSpeed];
			}

			PO_GROUP_ASSIGN( pPlayerState->m_PlayerOptions, ModsLevel_Song, m_fScrollSpeed, fScrollSpeed );
		}

		return true;
	case EDIT_BUTTON_SCROLL_HOME:
		ScrollTo( 0 );
		return true;
	case EDIT_BUTTON_SCROLL_END:
		ScrollTo( m_NoteDataEdit.GetLastBeat() );
		return true;
	case EDIT_BUTTON_SCROLL_UP_LINE:
	case EDIT_BUTTON_SCROLL_UP_PAGE:
	case EDIT_BUTTON_SCROLL_UP_TS:
	case EDIT_BUTTON_SCROLL_DOWN_LINE:
	case EDIT_BUTTON_SCROLL_DOWN_PAGE:
	case EDIT_BUTTON_SCROLL_DOWN_TS:
		{
			float fBeatsToMove=0.f;
			switch( EditB )
			{
			DEFAULT_FAIL( EditB );
			case EDIT_BUTTON_SCROLL_UP_LINE:
			case EDIT_BUTTON_SCROLL_DOWN_LINE:
				fBeatsToMove = NoteTypeToBeat( m_SnapDisplay.GetNoteType() );
				if( EditB == EDIT_BUTTON_SCROLL_UP_LINE )
					fBeatsToMove *= -1;
				break;
			case EDIT_BUTTON_SCROLL_UP_PAGE:
			case EDIT_BUTTON_SCROLL_DOWN_PAGE:
				fBeatsToMove = 4;
				if( EditB == EDIT_BUTTON_SCROLL_UP_PAGE )
					fBeatsToMove *= -1;
				break;
			case EDIT_BUTTON_SCROLL_UP_TS:
			case EDIT_BUTTON_SCROLL_DOWN_TS:
				fBeatsToMove = beatsPerMeasure;
				if( EditB == EDIT_BUTTON_SCROLL_UP_TS )
					fBeatsToMove *= -1;
				break;
			}

			if( m_PlayerStateEdit.m_PlayerOptions.GetSong().m_fScrolls[PlayerOptions::SCROLL_REVERSE] > 0.5 )
				fBeatsToMove *= -1;

			float fDestinationBeat = GetBeat() + fBeatsToMove;
			fDestinationBeat = Quantize( fDestinationBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );

			ScrollTo( fDestinationBeat );
		}
		return true;
	case EDIT_BUTTON_SCROLL_NEXT_MEASURE:
		{
			float fDestinationBeat = GetBeat() + beatsPerMeasure;
			fDestinationBeat = ftruncf( fDestinationBeat, (float)beatsPerMeasure );
			ScrollTo( fDestinationBeat );
			return true;
		}
	case EDIT_BUTTON_SCROLL_PREV_MEASURE:
		{
			float fDestinationBeat = QuantizeUp( GetBeat(), (float)beatsPerMeasure );
			fDestinationBeat -= (float)beatsPerMeasure;
			ScrollTo( fDestinationBeat );
			return true;
		}
	case EDIT_BUTTON_SCROLL_NEXT:
		{
			int iRow = BeatToNoteRow( GetBeat() );
			NoteDataUtil::GetNextEditorPosition( m_NoteDataEdit, iRow );
			ScrollTo( NoteRowToBeat(iRow) );
		}
		return true;
	case EDIT_BUTTON_SCROLL_PREV:
		{
			int iRow = BeatToNoteRow( GetBeat() );
			NoteDataUtil::GetPrevEditorPosition( m_NoteDataEdit, iRow );
			ScrollTo( NoteRowToBeat(iRow) );
		}
		return true;
	case EDIT_BUTTON_SEGMENT_NEXT:
		{
			// TODO: Work around Stops and Delays. We MAY have to separate them.
			const TimingData &timing = GetAppropriateTiming();
			ScrollTo(timing.GetNextSegmentBeatAtBeat(this->currentCycleSegment,
													 GetBeat()));
		}
		return true;
	case EDIT_BUTTON_SEGMENT_PREV:
		{
			// TODO: Work around Stops and Delays. We MAY have to separate them.
			const TimingData &timing = GetAppropriateTiming();
			ScrollTo(timing.GetPreviousSegmentBeatAtBeat(this->currentCycleSegment,
														 GetBeat()));
		}
		return true;
	case EDIT_BUTTON_SNAP_NEXT:
		if( m_SnapDisplay.PrevSnapMode() )
			OnSnapModeChange();
		return true;
	case EDIT_BUTTON_SNAP_PREV:
		if( m_SnapDisplay.NextSnapMode() )
			OnSnapModeChange();
		return true;
	case EDIT_BUTTON_LAY_SELECT:
		{
			const int iCurrentRow = BeatToNoteRow(GetAppropriatePosition().m_fSongBeat);
			if( m_NoteFieldEdit.m_iBeginMarker==-1 && m_NoteFieldEdit.m_iEndMarker==-1 )
			{
				// lay begin marker
				m_NoteFieldEdit.m_iBeginMarker = BeatToNoteRow(GetAppropriatePosition().m_fSongBeat);
			}
			else if( m_NoteFieldEdit.m_iEndMarker==-1 )	// only begin marker is laid
			{
				if( iCurrentRow == m_NoteFieldEdit.m_iBeginMarker )
				{
					m_NoteFieldEdit.m_iBeginMarker = -1;
				}
				else
				{
					m_NoteFieldEdit.m_iEndMarker = max( m_NoteFieldEdit.m_iBeginMarker, iCurrentRow );
					m_NoteFieldEdit.m_iBeginMarker = min( m_NoteFieldEdit.m_iBeginMarker, iCurrentRow );
				}
			}
			else	// both markers are laid
			{
				m_NoteFieldEdit.m_iBeginMarker = iCurrentRow;
				m_NoteFieldEdit.m_iEndMarker = -1;
			}
			m_soundMarker.Play(true);
		}
		return true;
	case EDIT_BUTTON_OPEN_AREA_MENU:
		{
			// TODO: Improve behavior if timing changes are shifted down on beat 0.
			g_AreaMenu.rows[shift_pauses_forward].bEnabled = (GetBeat() != 0);
			g_AreaMenu.rows[paste_at_current_beat].bEnabled = !m_Clipboard.IsEmpty();
			g_AreaMenu.rows[paste_at_begin_marker].bEnabled = !m_Clipboard.IsEmpty() != 0 && m_NoteFieldEdit.m_iBeginMarker!=-1;
			g_AreaMenu.rows[undo].bEnabled = m_bHasUndo;
			EditMiniMenu( &g_AreaMenu, SM_BackFromAreaMenu );
		}
		return true;
	case EDIT_BUTTON_OPEN_ALTER_MENU:
		{
			bool bAreaSelected = m_NoteFieldEdit.m_iBeginMarker!=-1 && m_NoteFieldEdit.m_iEndMarker!=-1;
			if (!bAreaSelected)
			{
				SCREENMAN->SystemMessage( ALTER_MENU_NO_SELECTION );
				SCREENMAN->PlayInvalidSound();
			}
			else
			{
				bool isRoutine = (m_InputPlayerNumber != PLAYER_INVALID);
				g_AlterMenu.rows[routine_invert_notes].bEnabled = isRoutine;
				g_AlterMenu.rows[routine_mirror_1_to_2].bEnabled = isRoutine;
				g_AlterMenu.rows[routine_mirror_2_to_1].bEnabled = isRoutine;
				EditMiniMenu(&g_AlterMenu, SM_BackFromAlterMenu);
			}
			return true;

		}
	case EDIT_BUTTON_OPEN_EDIT_MENU:
		EditMiniMenu( &g_MainMenu, SM_BackFromMainMenu );
		return true;
	case EDIT_BUTTON_OPEN_INPUT_HELP:
		DoHelp();
		return true;
	case EDIT_BUTTON_OPEN_TIMING_MENU:
		{
			DisplayTimingMenu();
			return true;
		}
	case EDIT_BUTTON_OPEN_NEXT_STEPS:
	case EDIT_BUTTON_OPEN_PREV_STEPS:
		{
			// don't keep undo when changing Steps
			ClearUndo();

			// get the second of the current step.
			float curSecond = GetAppropriateTiming().GetElapsedTimeFromBeat(GetBeat());

			// save current steps
			Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
			ASSERT( pSteps != nullptr );
			pSteps->SetNoteData( m_NoteDataEdit );

			// Get all Steps of this StepsType
			const StepsType st = pSteps->m_StepsType;
			vector<Steps*> vSteps;
			SongUtil::GetSteps( GAMESTATE->m_pCurSong, vSteps, st );

			// Sort them by difficulty.
			StepsUtil::SortStepsByTypeAndDifficulty( vSteps );

			// Find out what index the current Steps are
			vector<Steps*>::iterator it = find( vSteps.begin(), vSteps.end(), pSteps );
			ASSERT( it != vSteps.end() );

			switch( EditB )
			{
			DEFAULT_FAIL( EditB );
			case EDIT_BUTTON_OPEN_PREV_STEPS:
				if( it==vSteps.begin() )
				{
					SCREENMAN->PlayInvalidSound();
					return true;
				}
				it--;
				break;
			case EDIT_BUTTON_OPEN_NEXT_STEPS:
				it++;
				if( it==vSteps.end() )
				{
					SCREENMAN->PlayInvalidSound();
					return true;
				}
				break;
			}

			pSteps = *it;
			GAMESTATE->m_pCurSteps[PLAYER_1].Set( pSteps );
			m_pSteps = pSteps;
			pSteps->GetNoteData( m_NoteDataEdit );

			RString s = ssprintf(
				SWITCHED_TO.GetValue() + " %s %s '%s' (%d of %d)",
				GAMEMAN->GetStepsTypeInfo( st ).szName,
				DifficultyToString( pSteps->GetDifficulty() ).c_str(),
				pSteps->GetChartName().c_str(),
				it - vSteps.begin() + 1,
				int(vSteps.size()) );
			SCREENMAN->SystemMessage( s );
			m_soundSwitchSteps.Play(true);
			// Reload the music because it can be different for every steps. -Kyz
			m_AutoKeysounds.FinishLoading();
			m_pSoundMusic = m_AutoKeysounds.GetSound();

			ScrollTo( GetAppropriateTiming().GetBeatFromElapsedTime(curSecond) );
		}
		return true;
	case EDIT_BUTTON_BPM_UP:
	case EDIT_BUTTON_BPM_DOWN:
		{
			float fBPM = GetAppropriateTiming().GetBPMAtBeat( GetBeat() );
			float fDelta;
			switch( EditB )
			{
			DEFAULT_FAIL( EditB );
			case EDIT_BUTTON_BPM_UP:	fDelta = +0.020f;	break;
			case EDIT_BUTTON_BPM_DOWN:	fDelta = -0.020f;	break;
			}
			if( EditIsBeingPressed( EDIT_BUTTON_ADJUST_FINE ) )
			{
				fDelta /= 20;	// .001 bpm
			}
			else if( input.type == IET_REPEAT )
			{
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fDelta *= 10;
				else
					fDelta *= 40;
			}

			float fNewBPM = fBPM + fDelta;
			if(fNewBPM > 0.0f)
			{
				GetAppropriateTimingForUpdate().AddSegment(BPMSegment(GetRow(), fNewBPM));
			}
			(fDelta>0 ? m_soundValueIncrease : m_soundValueDecrease).Play(true);
			SetDirty( true );
		}
		return true;
	case EDIT_BUTTON_STOP_UP:
	case EDIT_BUTTON_STOP_DOWN:
		{
			float fDelta;
			switch( EditB )
			{
			DEFAULT_FAIL( EditB );
			case EDIT_BUTTON_STOP_UP:	fDelta = +0.020f;	break;
			case EDIT_BUTTON_STOP_DOWN:	fDelta = -0.020f;	break;
			}
			if( EditIsBeingPressed( EDIT_BUTTON_ADJUST_FINE ) )
			{
				fDelta /= 20; // 1ms
			}
			else if( input.type == IET_REPEAT )
			{
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fDelta *= 10;
				else
					fDelta *= 40;
			}

			// is there a StopSegment on the current row?
			TimingData & timing = GetAppropriateTimingForUpdate();
			StopSegment *seg = timing.GetStopSegmentAtRow( GetRow() );
			int i = timing.GetSegmentIndexAtRow(SEGMENT_STOP, GetRow());
			if (i == -1 || seg->GetRow() != GetRow()) // invalid
			{
				if( fDelta > 0 )
					timing.AddSegment( StopSegment(GetRow(), fDelta) );
				else
					return false;
			}
			else
			{
				vector<TimingSegment *> &stops = timing.GetTimingSegments(SEGMENT_STOP);
				seg->SetPause(seg->GetPause() + fDelta);
				if( seg->GetPause() <= 0 )
					stops.erase( stops.begin()+i, stops.begin()+i+1);
			}

			(fDelta>0 ? m_soundValueIncrease : m_soundValueDecrease).Play(true);
			SetDirty( true );
		}
		return true;
	// TODO: Combine the stop and delay call somehow?
	case EDIT_BUTTON_DELAY_UP:
	case EDIT_BUTTON_DELAY_DOWN:
		{
			float fDelta;
			switch( EditB )
			{
					DEFAULT_FAIL( EditB );
				case EDIT_BUTTON_DELAY_UP:		fDelta = +0.020f;	break;
				case EDIT_BUTTON_DELAY_DOWN:	fDelta = -0.020f;	break;
			}
			if( EditIsBeingPressed( EDIT_BUTTON_ADJUST_FINE ) )
			{
				fDelta /= 20; // 1ms
			}
			else if( input.type == IET_REPEAT )
			{
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fDelta *= 10;
				else
					fDelta *= 40;
			}

			// is there a StopSegment on the current row?
			TimingData & timing = GetAppropriateTimingForUpdate();
			DelaySegment *seg = timing.GetDelaySegmentAtRow( GetRow() );
			int i = timing.GetSegmentIndexAtRow(SEGMENT_DELAY, GetRow());
			if (i == -1 || seg->GetRow() != GetRow()) // invalid
			{
				if( fDelta > 0 )
					timing.AddSegment( DelaySegment(GetRow(), fDelta) );
				else
					return false;
			}
			else
			{
				vector<TimingSegment *> &stops = timing.GetTimingSegments(SEGMENT_DELAY);
				seg->SetPause(seg->GetPause() + fDelta);
				if( seg->GetPause() <= 0 )
					stops.erase( stops.begin()+i, stops.begin()+i+1);
			}

			(fDelta>0 ? m_soundValueIncrease : m_soundValueDecrease).Play(true);
			SetDirty( true );
		}
		return true;
	case EDIT_BUTTON_OFFSET_UP:
	case EDIT_BUTTON_OFFSET_DOWN:
		{
			float fDelta;
			switch( EditB )
			{
			DEFAULT_FAIL( EditB );
			case EDIT_BUTTON_OFFSET_DOWN:	fDelta = -0.02f;	break;
			case EDIT_BUTTON_OFFSET_UP:	fDelta = +0.02f;	break;
			}
			if( EditIsBeingPressed( EDIT_BUTTON_ADJUST_FINE ) )
			{
				fDelta /= 20; /* 1ms */
			}
			else if( input.type == IET_REPEAT )
			{
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fDelta *= 10;
				else
					fDelta *= 40;
			}
			GetAppropriateTimingForUpdate().m_fBeat0OffsetInSeconds += fDelta;
			(fDelta>0 ? m_soundValueIncrease : m_soundValueDecrease).Play(true);
			if (GAMESTATE->m_bIsUsingStepTiming)
			{
				GAMESTATE->m_pCurSteps[PLAYER_1]->m_Attacks.UpdateStartTimes(fDelta);
			}
			else
			{
				GAMESTATE->m_pCurSong->m_Attacks.UpdateStartTimes(fDelta);
				GAMESTATE->m_pCurSong->m_fMusicSampleStartSeconds += fDelta;
			}
			SetDirty( true );
		}
		return true;
	case EDIT_BUTTON_SAMPLE_START_UP:
	case EDIT_BUTTON_SAMPLE_START_DOWN:
	case EDIT_BUTTON_SAMPLE_LENGTH_DOWN:
	case EDIT_BUTTON_SAMPLE_LENGTH_UP:
		{
			float fDelta;
			switch( EditB )
			{
			DEFAULT_FAIL( EditB );
			case EDIT_BUTTON_SAMPLE_START_DOWN:		fDelta = -0.02f;	break;
			case EDIT_BUTTON_SAMPLE_START_UP:		fDelta = +0.02f;	break;
			case EDIT_BUTTON_SAMPLE_LENGTH_DOWN:		fDelta = -0.02f;	break;
			case EDIT_BUTTON_SAMPLE_LENGTH_UP:		fDelta = +0.02f;	break;
			}

			if( input.type == IET_REPEAT )
			{
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fDelta *= 10;
				else
					fDelta *= 40;
			}

			if( EditB == EDIT_BUTTON_SAMPLE_LENGTH_DOWN || EditB == EDIT_BUTTON_SAMPLE_LENGTH_UP )
			{
				m_pSong->m_fMusicSampleLengthSeconds += fDelta;
				m_pSong->m_fMusicSampleLengthSeconds = max(m_pSong->m_fMusicSampleLengthSeconds,0);
			}
			else
			{
				m_pSong->m_fMusicSampleStartSeconds += fDelta;
				m_pSong->m_fMusicSampleStartSeconds = max(m_pSong->m_fMusicSampleStartSeconds,0);
			}
			(fDelta>0 ? m_soundValueIncrease : m_soundValueDecrease).Play(true);
			SetDirty( true );
		}
		return true;
	case EDIT_BUTTON_PLAY_SAMPLE_MUSIC:
		PlayPreviewMusic();
		return true;
	case EDIT_BUTTON_OPEN_BGCHANGE_LAYER1_MENU:
	case EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU:
		if( !GAMESTATE->m_bIsUsingStepTiming )
		{
			switch( EditB )
			{
			DEFAULT_FAIL( EditB );
			case EDIT_BUTTON_OPEN_BGCHANGE_LAYER1_MENU: g_CurrentBGChangeLayer = BACKGROUND_LAYER_1; break;
			case EDIT_BUTTON_OPEN_BGCHANGE_LAYER2_MENU: g_CurrentBGChangeLayer = BACKGROUND_LAYER_2; break;
			}

			{
				// Fill in option names
				vector<RString> vThrowAway;

				MenuDef &menu = g_BackgroundChange;

				menu.rows[layer].choices[0] = ssprintf("%d",g_CurrentBGChangeLayer);
				BackgroundUtil::GetBackgroundTransitions(	"", vThrowAway, menu.rows[transition].choices );
				g_BackgroundChange.rows[transition].choices.insert( g_BackgroundChange.rows[transition].choices.begin(), "None" );	// add "no transition"
				BackgroundUtil::GetBackgroundEffects(		"", vThrowAway, menu.rows[effect].choices );
				menu.rows[effect].choices.insert( menu.rows[effect].choices.begin(), "Default" );	// add "default effect"

				BackgroundUtil::GetSongBGAnimations(   m_pSong, "", vThrowAway, menu.rows[file1_song_bganimation].choices );
				BackgroundUtil::GetSongMovies(         m_pSong, "", vThrowAway, menu.rows[file1_song_movie].choices );
				BackgroundUtil::GetSongBitmaps(        m_pSong, "", vThrowAway, menu.rows[file1_song_still].choices );
				BackgroundUtil::GetGlobalBGAnimations( m_pSong, "", vThrowAway, menu.rows[file1_global_bganimation].choices );	// nullptr to get all background files
				BackgroundUtil::GetGlobalRandomMovies( m_pSong, "", vThrowAway, menu.rows[file1_global_movie].choices, false, false );	// all backgrounds
				BackgroundUtil::GetGlobalRandomMovies( m_pSong, "", vThrowAway, menu.rows[file1_global_movie_song_group].choices, false, true );	// song group's backgrounds
				BackgroundUtil::GetGlobalRandomMovies( m_pSong, "", vThrowAway, menu.rows[file1_global_movie_song_group_and_genre].choices, true, true );	// song group and genre's backgrounds

				menu.rows[file2_type].choices					= menu.rows[file1_type].choices;
				menu.rows[file2_song_bganimation].choices			= menu.rows[file1_song_bganimation].choices;
				menu.rows[file2_song_movie].choices				= menu.rows[file1_song_movie].choices;
				menu.rows[file2_song_still].choices				= menu.rows[file1_song_still].choices;
				menu.rows[file2_global_bganimation].choices			= menu.rows[file1_global_bganimation].choices;
				menu.rows[file2_global_movie].choices				= menu.rows[file1_global_movie].choices;
				menu.rows[file2_global_movie_song_group].choices		= menu.rows[file1_global_movie_song_group].choices;
				menu.rows[file2_global_movie_song_group_and_genre].choices	= menu.rows[file1_global_movie_song_group_and_genre].choices;


				// Fill in lines enabled/disabled
				bool bAlreadyBGChangeHere = false;
				BackgroundChange bgChange;
				for (BackgroundChange &bgc : m_pSong->GetBackgroundChanges(g_CurrentBGChangeLayer))
				{
					if( bgc.m_fStartBeat == GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat )
					{
						bAlreadyBGChangeHere = true;
						bgChange = bgc;
					}
				}

#define FILL_ENABLED( x )	menu.rows[x].bEnabled	= menu.rows[x].choices.size() > 0;
				FILL_ENABLED( transition );
				FILL_ENABLED( effect );
				FILL_ENABLED( file1_song_bganimation );
				FILL_ENABLED( file1_song_movie );
				FILL_ENABLED( file1_song_still );
				FILL_ENABLED( file1_global_bganimation );
				FILL_ENABLED( file1_global_movie );
				FILL_ENABLED( file1_global_movie_song_group );
				FILL_ENABLED( file1_global_movie_song_group_and_genre );
				FILL_ENABLED( file2_song_bganimation );
				FILL_ENABLED( file2_song_movie );
				FILL_ENABLED( file2_song_still );
				FILL_ENABLED( file2_global_bganimation );
				FILL_ENABLED( file2_global_movie );
				FILL_ENABLED( file2_global_movie_song_group );
				FILL_ENABLED( file2_global_movie_song_group_and_genre );
#undef FILL_ENABLED
				menu.rows[delete_change].bEnabled = bAlreadyBGChangeHere;

				// set default choices
				menu.rows[rate].					SetDefaultChoiceIfPresent( ssprintf("%2.0f%%",bgChange.m_fRate*100) );
				menu.rows[transition].					SetDefaultChoiceIfPresent( bgChange.m_sTransition );
				menu.rows[effect].					SetDefaultChoiceIfPresent( bgChange.m_def.m_sEffect );
				menu.rows[file1_type].iDefaultChoice			= none;
				if( bgChange.m_def.m_sFile1 == RANDOM_BACKGROUND_FILE )								menu.rows[file1_type].iDefaultChoice	= dynamic_random;
				if( menu.rows[file1_song_bganimation].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile1 ) )	menu.rows[file1_type].iDefaultChoice	= song_bganimation;
				if( menu.rows[file1_song_movie].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile1 ) )	menu.rows[file1_type].iDefaultChoice	= song_movie;
				if( menu.rows[file1_song_still].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile1 ) )	menu.rows[file1_type].iDefaultChoice	= song_bitmap;
				if( menu.rows[file1_global_bganimation].		SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile1 ) )	menu.rows[file1_type].iDefaultChoice	= global_bganimation;
				if( menu.rows[file1_global_movie].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile1 ) )	menu.rows[file1_type].iDefaultChoice	= global_movie;
				if( menu.rows[file1_global_movie_song_group].		SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile1 ) )	menu.rows[file1_type].iDefaultChoice	= global_movie_song_group;
				if( menu.rows[file1_global_movie_song_group_and_genre].	SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile1 ) )	menu.rows[file1_type].iDefaultChoice	= global_movie_song_group_and_genre;
				menu.rows[file2_type].iDefaultChoice			= none;
				if( bgChange.m_def.m_sFile2 == RANDOM_BACKGROUND_FILE )								menu.rows[file2_type].iDefaultChoice	= dynamic_random;
				if( menu.rows[file2_song_bganimation].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile2 ) )	menu.rows[file2_type].iDefaultChoice	= song_bganimation;
				if( menu.rows[file2_song_movie].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile2 ) )	menu.rows[file2_type].iDefaultChoice	= song_movie;
				if( menu.rows[file2_song_still].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile2 ) )	menu.rows[file2_type].iDefaultChoice	= song_bitmap;
				if( menu.rows[file2_global_bganimation].		SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile2 ) )	menu.rows[file2_type].iDefaultChoice	= global_bganimation;
				if( menu.rows[file2_global_movie].			SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile2 ) )	menu.rows[file2_type].iDefaultChoice	= global_movie;
				if( menu.rows[file2_global_movie_song_group].		SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile2 ) )	menu.rows[file2_type].iDefaultChoice	= global_movie_song_group;
				if( menu.rows[file2_global_movie_song_group_and_genre].	SetDefaultChoiceIfPresent( bgChange.m_def.m_sFile2 ) )	menu.rows[file2_type].iDefaultChoice	= global_movie_song_group_and_genre;
				menu.rows[color1].					SetDefaultChoiceIfPresent( bgChange.m_def.m_sColor1 );
				menu.rows[color2].					SetDefaultChoiceIfPresent( bgChange.m_def.m_sColor2 );

				EditMiniMenu( &g_BackgroundChange, SM_BackFromBGChange );
			}
		}
		else
		{
			SCREENMAN->SystemMessage( BG_CHANGE_STEP_TIMING );
			SCREENMAN->PlayInvalidSound();
		}
		return true;

	case EDIT_BUTTON_OPEN_COURSE_MENU:
		{
			g_CourseMode.rows[0].choices.clear();
			g_CourseMode.rows[0].choices.push_back( CommonMetrics::LocalizeOptionItem("Off",false) );
			g_CourseMode.rows[0].iDefaultChoice = 0;

			vector<Course*> courses;
			SONGMAN->GetAllCourses( courses, false );
			for( unsigned i = 0; i < courses.size(); ++i )
			{
				const Course *crs = courses[i];

				bool bUsesThisSong = false;
				for( unsigned e = 0; e < crs->m_vEntries.size(); ++e )
				{
					if( crs->m_vEntries[e].songID.ToSong() != m_pSong )
						continue;
					bUsesThisSong = true;
				}

				if( bUsesThisSong )
				{
					g_CourseMode.rows[0].choices.push_back( crs->GetDisplayFullTitle() );
					if( crs == GAMESTATE->m_pCurCourse )
						g_CourseMode.rows[0].iDefaultChoice = g_CourseMode.rows[0].choices.size()-1;
				}
			}

			EditMiniMenu( &g_CourseMode, SM_BackFromCourseModeMenu );
		}
		return true;
	case EDIT_BUTTON_OPEN_STEP_ATTACK_MENU:
	{
		this->DoStepAttackMenu();
		return true;
	}
	case EDIT_BUTTON_OPEN_COURSE_ATTACK_MENU:
		{
			// TODO: Give Song/Step Timing switches/functions here?
			Course *pCourse = GAMESTATE->m_pCurCourse;
			if( pCourse == nullptr )
				return false;
			CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
			float fStartTime = m_pSteps->GetTimingData()->GetElapsedTimeFromBeat( GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat );
			int iAttack = FindAttackAtTime( ce.attacks, fStartTime );

			if( iAttack >= 0 )
			{
				const RString sDuration = std::to_string(ce.attacks[iAttack].fSecsRemaining );

				g_InsertCourseAttack.rows[remove].bEnabled = true;
				if( g_InsertCourseAttack.rows[duration].choices.size() == 9 )
					g_InsertCourseAttack.rows[duration].choices.push_back( sDuration );
				else
					g_InsertCourseAttack.rows[duration].choices.back() = sDuration;
				g_InsertCourseAttack.rows[duration].iDefaultChoice = 9;
			}
			else
			{
				if( g_InsertCourseAttack.rows[duration].choices.size() == 10 )
					g_InsertCourseAttack.rows[duration].choices.pop_back();
				g_InsertCourseAttack.rows[duration].iDefaultChoice = 3;
			}

			EditMiniMenu( &g_InsertCourseAttack, SM_BackFromInsertCourseAttack );
		}
		return true;
	case EDIT_BUTTON_ADD_STEP_MODS:
	{
		float start = -1;
		float end = -1;
		PlayerOptions po;

		if (m_NoteFieldEdit.m_iBeginMarker == -1) // not highlighted
		{
			po.FromString("");
		}
		else
		{
			const TimingData &timing = GetAppropriateTiming();
			start = timing.GetElapsedTimeFromBeat(NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker));
			AttackArray &attacks =
				(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
			int index = FindAttackAtTime(attacks, start);

			if (index >= 0)
			{
				po.FromString("");
			}
			if (m_NoteFieldEdit.m_iEndMarker == -1)
			{
				end = m_pSong->m_fMusicLengthSeconds;
			}
			else
			{
				end = timing.GetElapsedTimeFromBeat(NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker));
			}

		}
		ModsGroup<PlayerOptions> &toEdit = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions;
		this->originalPlayerOptions.Assign(ModsLevel_Preferred, toEdit.GetPreferred());
		g_fLastInsertAttackPositionSeconds = start;
		g_fLastInsertAttackDurationSeconds = end - start;
		toEdit.Assign( ModsLevel_Stage, po );
		SCREENMAN->AddNewScreenToTop( SET_MOD_SCREEN, SM_BackFromInsertStepAttackPlayerOptions );
		return true;
	}
	case EDIT_BUTTON_ADD_COURSE_MODS:
		{
			float fStart, fEnd;
			PlayerOptions po;
			const Course *pCourse = GAMESTATE->m_pCurCourse;
			if( pCourse == nullptr )
				return false;
			const CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];

			if( m_NoteFieldEdit.m_iBeginMarker == -1 )
			{
				fStart = -1;
				fEnd = -1;
				po.FromString( ce.sModifiers );
			}
			else
			{
				// TODO: Give Song/Step Timing switches/functions here?
				TimingData *timing = m_pSteps->GetTimingData();
				fStart = timing->GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker) );
				int iAttack = FindAttackAtTime( ce.attacks, fStart );

				if( iAttack >= 0 )
					po.FromString( ce.attacks[iAttack].sModifiers );

				if( m_NoteFieldEdit.m_iEndMarker == -1 )
					fEnd = m_pSong->m_fMusicLengthSeconds;
				else
					fEnd = timing->GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker) );
			}
			g_fLastInsertAttackPositionSeconds = fStart;
			g_fLastInsertAttackDurationSeconds = fEnd - fStart;
			GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.Assign( ModsLevel_Stage, po );
			SCREENMAN->AddNewScreenToTop( SET_MOD_SCREEN, SM_BackFromInsertCourseAttackPlayerOptions );

		}
		return true;
	case EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP:
	case EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE:
		{
			bool bTryGenre = EditB == EDIT_BUTTON_BAKE_RANDOM_FROM_SONG_GROUP_AND_GENRE;
			RString sName = GetOneBakedRandomFile(m_pSong, bTryGenre);
			if( sName.empty() )
			{
				SCREENMAN->PlayInvalidSound();
				SCREENMAN->SystemMessage( NO_BACKGROUNDS_AVAILABLE );
			}
			else
			{
				BackgroundLayer iLayer = BACKGROUND_LAYER_1;
				BackgroundChange bgChange;
				bgChange.m_fStartBeat = GAMESTATE->m_Position.m_fSongBeat;
				auto & changes = m_pSong->GetBackgroundChanges(iLayer);
				for (auto bgc = changes.begin(); bgc != changes.end(); ++bgc)
				{
					if( bgc->m_fStartBeat == GAMESTATE->m_Position.m_fSongBeat )
					{
						bgChange = *bgc;
						changes.erase( bgc );
						break;
					}
				}
				bgChange.m_def.m_sFile1 = sName;
				m_pSong->AddBackgroundChange( iLayer, bgChange );
				m_soundMarker.Play(true);
			}
		}
		return true;

	case EDIT_BUTTON_PLAY_FROM_START:
		HandleMainMenuChoice( play_whole_song );
		return true;

	case EDIT_BUTTON_PLAY_FROM_CURSOR:
		HandleMainMenuChoice( play_current_beat_to_end );
		return true;

	case EDIT_BUTTON_PLAY_SELECTION:
		HandleMainMenuChoice( play_selection );
		return true;
	case EDIT_BUTTON_RECORD_SELECTION:
		if( m_NoteFieldEdit.m_iBeginMarker!=-1 && m_NoteFieldEdit.m_iEndMarker!=-1 )
		{
			HandleAlterMenuChoice( record );
		}
		else
		{
			if( g_iDefaultRecordLength.Get() == -1 )
			{
				m_iStartPlayingAt = BeatToNoteRow(GetAppropriatePosition().m_fSongBeat);
				m_iStopPlayingAt = max( m_iStartPlayingAt, m_NoteDataEdit.GetLastRow() + 1 );
			}
			else
			{
				m_iStartPlayingAt = BeatToNoteRow( ftruncf(GetAppropriatePosition().m_fSongBeat, g_iDefaultRecordLength.Get()) );
				m_iStopPlayingAt = m_iStartPlayingAt + BeatToNoteRow( g_iDefaultRecordLength.Get() );
			}

			if( GAMESTATE->m_pCurSteps[0]->IsAnEdit() )
				m_iStopPlayingAt = min( m_iStopPlayingAt, BeatToNoteRow(GetMaximumBeatForNewNote()) );

			if( m_iStartPlayingAt >= m_iStopPlayingAt )
			{
				SCREENMAN->PlayInvalidSound();
				return true;
			}

			TransitionEditState( STATE_RECORDING );
		}
		return true;
	case EDIT_BUTTON_RECORD_FROM_CURSOR:
		m_iStartPlayingAt = BeatToNoteRow(GetAppropriatePosition().m_fSongBeat);
		m_iStopPlayingAt = max( m_iStartPlayingAt, m_NoteDataEdit.GetLastRow() );
		TransitionEditState( STATE_RECORDING );
		return true;

	case EDIT_BUTTON_INSERT:
		HandleAreaMenuChoice( insert_and_shift );
		SCREENMAN->PlayInvalidSound();
		return true;

	case EDIT_BUTTON_INSERT_SHIFT_PAUSES:
		HandleAreaMenuChoice( shift_pauses_forward );
		SCREENMAN->PlayInvalidSound();
		return true;

	case EDIT_BUTTON_DELETE:
		HandleAreaMenuChoice( delete_and_shift );
		SCREENMAN->PlayInvalidSound();
		return true;

	case EDIT_BUTTON_DELETE_SHIFT_PAUSES:
		HandleAreaMenuChoice( shift_pauses_backward );
		SCREENMAN->PlayInvalidSound();
		return true;

	case EDIT_BUTTON_RECORD_HOLD_LESS:
		record_hold_seconds-= .01f;
		if(record_hold_seconds <= 0.0f)
		{
			record_hold_seconds= 0.0f;
		}
		return true;

	case EDIT_BUTTON_RECORD_HOLD_MORE:
		record_hold_seconds+= .01f;
		return true;

	case EDIT_BUTTON_RECORD_HOLD_RESET:
		record_hold_seconds= record_hold_default;
		return true;

	case EDIT_BUTTON_RECORD_HOLD_OFF:
		record_hold_seconds= 120.0f;
		return true;

	case EDIT_BUTTON_SAVE:
		HandleMainMenuChoice( ScreenEdit::save );
		return true;

	case EDIT_BUTTON_UNDO:
		Undo();
		return true;

	case EDIT_BUTTON_SWITCH_PLAYERS:
		if( m_InputPlayerNumber == PLAYER_INVALID )
			return false;
		enum_add( m_InputPlayerNumber, 1 );
		if( m_InputPlayerNumber == NUM_PLAYERS )
			m_InputPlayerNumber = PLAYER_1;
		m_soundSwitchPlayer.Play(true);
		return true;

	case EDIT_BUTTON_SWITCH_TIMINGS:
		GAMESTATE->m_bIsUsingStepTiming = !GAMESTATE->m_bIsUsingStepTiming;
		m_soundSwitchTiming.Play(true);
		return true;
	default:
		return false;
	}
}

bool ScreenEdit::InputRecord( const InputEventPlus &input, EditButton EditB )
{
	if( input.type == IET_FIRST_PRESS  &&  EditB == EDIT_BUTTON_RETURN_TO_EDIT )
	{
		TransitionEditState( STATE_EDITING );
		return true;
	}

	if( input.pn != PLAYER_1 )
		return false;		// ignore

	const int iCol = GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->GameInputToColumn( input.GameI );

	//Is this actually a column? If not, ignore the input.
	if( iCol == -1 )
		return false;

	switch( input.type )
	{
	DEFAULT_FAIL( input.type );
	case IET_FIRST_PRESS:
		{
			if( EditIsBeingPressed(EDIT_BUTTON_REMOVE_NOTE) )
			{
				// Remove notes in Update.
				return false;
			}

			// Add a tap
			float fBeat = GetBeat();
			fBeat = Quantize( fBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );

			const int iRow = BeatToNoteRow( fBeat );
			if( iRow < 0 )
				return false;

			// Don't add outside of the range.
			if( iRow < m_iStartPlayingAt || iRow >= m_iStopPlayingAt )
				return false;

			// Remove hold if any so that we don't have taps inside of a hold.
			int iHeadRow;
			if( m_NoteDataRecord.IsHoldNoteAtRow( iCol, iRow, &iHeadRow ) )
				m_NoteDataRecord.SetTapNote( iCol, iHeadRow, TAP_EMPTY );

			TapNote tn = TAP_ORIGINAL_TAP;
			tn.pn = m_InputPlayerNumber;
			m_NoteDataRecord.SetTapNote( iCol, iRow, tn );
			m_NoteFieldRecord.Step( iCol, TNS_W1 );
		}
		return true;
	case IET_REPEAT:
	case IET_RELEASE:
		// don't add or extend holds here; we do it in Update()
		return false;
	}
}

bool ScreenEdit::InputRecordPaused( const InputEventPlus &input, EditButton EditB )
{
	if( input.type != IET_FIRST_PRESS )
		return false;	// don't care

	switch( EditB )
	{
	case EDIT_BUTTON_UNDO:
		/* We've already actually committed changes to m_NoteDataEdit, so all we have
		 * to do to undo is Undo() as usual, and copy the note data back in. */
		Undo();
		m_NoteDataRecord.CopyAll( m_NoteDataEdit );
		return true;

	case EDIT_BUTTON_PLAY_SELECTION:
		TransitionEditState( STATE_PLAYING );
		return true;

	case EDIT_BUTTON_RECORD_SELECTION:
		TransitionEditState( STATE_RECORDING );
		return true;

	case EDIT_BUTTON_RECORD_FROM_CURSOR:
		if( GAMESTATE->m_pCurSteps[0]->IsAnEdit() )
		{
			float fMaximumBeat = GetMaximumBeatForNewNote();
			if( NoteRowToBeat(m_iStopPlayingAt) >= fMaximumBeat )
			{
				// We're already at the end.
				SCREENMAN->PlayInvalidSound();
				return true;
			}
		}

		// Pick up where we left off.
		{
			int iSize = m_iStopPlayingAt - m_iStartPlayingAt;
			m_iStartPlayingAt = m_iStopPlayingAt;
			m_iStopPlayingAt += iSize;

			if( GAMESTATE->m_pCurSteps[0]->IsAnEdit() )
				m_iStopPlayingAt = min( m_iStopPlayingAt, BeatToNoteRow(GetMaximumBeatForNewNote()) );
		}

		TransitionEditState( STATE_RECORDING );
		return true;

	case EDIT_BUTTON_RETURN_TO_EDIT:
		TransitionEditState( STATE_EDITING );
		return true;
	default:
		return false;
	}
}

bool ScreenEdit::InputPlay( const InputEventPlus &input, EditButton EditB )
{
	switch( input.type )
	{
	case IET_RELEASE:
	case IET_FIRST_PRESS:
		break;
	default:
		return false;
	}

	GameButtonType gbt = GAMESTATE->m_pCurGame->GetPerButtonInfo(input.GameI.button)->m_gbt;

	if( GamePreferences::m_AutoPlay == PC_HUMAN && GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetCurrent().m_fPlayerAutoPlay == 0 )
	{
		const int iCol = GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->GameInputToColumn( input.GameI );
		bool bRelease = input.type == IET_RELEASE;
		switch( input.pn )
		{
		case PLAYER_2:
			// ignore player 2 input unless this mode requires it
			if( GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StyleType != StyleType_TwoPlayersSharedSides )
				break;

		// fall through to input handling logic:
		case PLAYER_1:
			{
				switch( gbt )
				{
				case GameButtonType_Step:
					if( iCol != -1 )
						m_Player->Step( iCol, -1, input.DeviceI.ts, false, bRelease );
					return true;
				default:
					break;
				}
			}
		default: break;
		}
	}

	if( gbt == GameButtonType_Menu  &&  input.type == IET_FIRST_PRESS )
	{
		switch( EditB )
		{
		case EDIT_BUTTON_RETURN_TO_EDIT:
			/* When exiting play mode manually, leave the cursor where it is. */
			m_fBeatToReturnTo = GetAppropriatePosition().m_fSongBeat;
			TransitionEditState( STATE_EDITING );
			return true;
		case EDIT_BUTTON_OFFSET_UP:
		case EDIT_BUTTON_OFFSET_DOWN:
		{
			float fOffsetDelta;
			switch( EditB )
			{
				DEFAULT_FAIL( EditB );
				case EDIT_BUTTON_OFFSET_DOWN:	fOffsetDelta = -0.020f;		break;
				case EDIT_BUTTON_OFFSET_UP:	fOffsetDelta = +0.020f;		break;
			}

			if( EditIsBeingPressed( EDIT_BUTTON_ADJUST_FINE ) )
			{
				fOffsetDelta /= 20; /* 1ms */
			}
			else if( input.type == IET_REPEAT )
			{
				if( INPUTFILTER->GetSecsHeld(input.DeviceI) < 1.0f )
					fOffsetDelta *= 10;
				else
					fOffsetDelta *= 40;
			}

			GetAppropriateTimingForUpdate().m_fBeat0OffsetInSeconds += fOffsetDelta;
			if (!GAMESTATE->m_bIsUsingStepTiming)
			{
				GAMESTATE->m_pCurSong->m_fMusicSampleStartSeconds += fOffsetDelta;
			}
		}
			return true;
		default: break;
		}
	}
	return false;
}

void ScreenEdit::TransitionEditState( EditState em )
{
	EditState old = m_EditState;

	// If we're going from recording to paused, come back when we're done.
	if( old == STATE_RECORDING_PAUSED && em == STATE_PLAYING )
		m_bReturnToRecordMenuAfterPlay = true;

	const bool bStateChanging = em != old;

#if 0
	// If switching out of record, open the menu.
	{
		bool bGoToRecordMenu = (old == STATE_RECORDING);
		if( m_bReturnToRecordMenuAfterPlay && old == STATE_PLAYING )
		{
			bGoToRecordMenu = true;
			m_bReturnToRecordMenuAfterPlay = false;
		}

		if( bGoToRecordMenu )
			em = STATE_RECORDING_PAUSED;
	}
#endif

	// If we're playing music or assist ticks when changing modes, stop.
	SOUND->StopMusic();
	if( m_pSoundMusic )
		m_pSoundMusic->StopPlaying();
	m_GameplayAssist.StopPlaying();
	GAMESTATE->m_bGameplayLeadIn.Set( true );

	if( bStateChanging )
	{
		switch( old )
		{
		case STATE_EDITING:
			// If exiting EDIT mode, save the cursor position.
			m_fBeatToReturnTo = GetAppropriatePosition().m_fSongBeat;
			break;

		case STATE_PLAYING:
			AdjustSync::HandleSongEnd();
			if (!GAMESTATE->m_bIsUsingStepTiming)
				GAMESTATE->m_pCurSteps[PLAYER_1]->m_Timing = backupStepTiming;
			if( AdjustSync::IsSyncDataChanged() )
				ScreenSaveSync::PromptSaveSync();
			break;

		case STATE_RECORDING:
			SetDirty( true );
			if (!GAMESTATE->m_bIsUsingStepTiming)
				GAMESTATE->m_pCurSteps[PLAYER_1]->m_Timing = backupStepTiming;
			SaveUndo();

			// delete old TapNotes in the range
			m_NoteDataEdit.ClearRange( m_iStartPlayingAt, m_iStopPlayingAt );
			m_NoteDataEdit.CopyRange( m_NoteDataRecord, m_iStartPlayingAt, m_iStopPlayingAt, m_iStartPlayingAt );
			m_NoteDataRecord.ClearAll();

			CheckNumberOfNotesAndUndo();
			break;
		default: break;
		}
	}

	// Set up player options for this mode. (EDITING uses m_PlayerStateEdit,
	// which we don't need to change.)
	if( em != STATE_EDITING )
	{
		// Stop displaying course attacks, if any.
		GAMESTATE->m_pPlayerState[PLAYER_1]->RemoveActiveAttacks();
		// Load the player's default PlayerOptions.
		GAMESTATE->m_pPlayerState[PLAYER_1]->RebuildPlayerOptionsFromActiveAttacks();

		// Snap to current options.
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.SetCurrentToLevel( ModsLevel_Stage );
	}

	switch( em )
	{
	DEFAULT_FAIL( em );
	case STATE_EDITING:
		// Important: people will stop playing, change the BG and start again; make sure we reload
		m_Background.Unload();
		m_Foreground.Unload();

		// Restore the cursor position + Quantize + Clamp
		SetBeat( max( 0, Quantize( m_fBeatToReturnTo, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) ) ) );
		GAMESTATE->m_bInStepEditor = true;
		break;

	case STATE_PLAYING:
	case STATE_RECORDING:
	{
		m_NoteDataEdit.RevalidateATIs(vector<int>(), false);
		if( bStateChanging )
			AdjustSync::ResetOriginalSyncData();

		/* Give a lead-in.  If we're loading Player, this must be done first.
		 * Also be sure to get the right timing. */
		float fSeconds = GetAppropriateTiming().GetElapsedTimeFromBeat( NoteRowToBeat(m_iStartPlayingAt) ) - PREFSMAN->m_EditRecordModeLeadIn;
		GAMESTATE->UpdateSongPosition( fSeconds, GetAppropriateTiming(), RageZeroTimer );

		GAMESTATE->m_bGameplayLeadIn.Set( false );

		if (!GAMESTATE->m_bIsUsingStepTiming)
		{
			// Substitute the song timing for the step timing during
			// preview if we're in song mode
			backupStepTiming = GAMESTATE->m_pCurSteps[PLAYER_1]->m_Timing;
			GAMESTATE->m_pCurSteps[PLAYER_1]->m_Timing.Clear();
		}

		/* Reset the note skin, in case preferences have changed. */
		// XXX
		// GAMESTATE->ResetNoteSkins();
		//GAMESTATE->res
		GAMESTATE->m_bInStepEditor = false;
		break;
	}
	case STATE_RECORDING_PAUSED:
		GAMESTATE->m_bInStepEditor = false;
		break;
	}

	switch( em )
	{
	case STATE_PLAYING:
		// If we're in course display mode, set that up.
		SetupCourseAttacks();

		m_Player.Load( m_NoteDataEdit );

		if( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetCurrent().m_fPlayerAutoPlay != 0 )
			GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerController = PC_AUTOPLAY;
		else
			GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerController = GamePreferences::m_AutoPlay;

		if( g_bEditorShowBGChangesPlay )
		{
			/* FirstBeat affects backgrounds, so commit changes to memory (not to disk)
			 * and recalc it. */
			Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
			ASSERT( pSteps != nullptr );
			pSteps->SetNoteData( m_NoteDataEdit );
			m_pSong->ReCalculateRadarValuesAndLastSecond();

			m_Background.Unload();
			m_Background.LoadFromSong( m_pSong );

			m_Foreground.Unload();
			m_Foreground.LoadFromSong( m_pSong );
		}

		break;
	case STATE_RECORDING:
	case STATE_RECORDING_PAUSED:
		// initialize m_NoteFieldRecord
		m_NoteDataRecord.CopyAll( m_NoteDataEdit );

		// highlight the section being recorded
		m_NoteFieldRecord.m_iBeginMarker = m_iStartPlayingAt;
		m_NoteFieldRecord.m_iEndMarker = m_iStopPlayingAt;

		break;
	default: break;
	}

	// Show/hide depending on edit state (em)
	m_sprOverlay->PlayCommand( EditStateToString(em) );
	m_sprUnderlay->PlayCommand( EditStateToString(em) );

	m_Background.SetVisible( g_bEditorShowBGChangesPlay  &&  em != STATE_EDITING );
	m_textInputTips.SetVisible( em == STATE_EDITING );
	m_textInfo.SetVisible( em == STATE_EDITING );
	// Play the OnCommands again so that these will be re-hidden if the OnCommand hides them.
	if( em == STATE_EDITING )
	{
		m_textInputTips.PlayCommand( "On" );
		m_textInfo.PlayCommand( "On" );
	}
	m_textPlayRecordHelp.SetVisible( em != STATE_EDITING );
	m_SnapDisplay.SetVisible( em == STATE_EDITING );
	m_NoteFieldEdit.SetVisible( em == STATE_EDITING );
	m_NoteFieldRecord.SetVisible( em == STATE_RECORDING  ||  em == STATE_RECORDING_PAUSED );
	m_Player->SetVisible( em == STATE_PLAYING );
	m_Foreground.SetVisible( g_bEditorShowBGChangesPlay  &&  em != STATE_EDITING );

	switch( em )
	{
	case STATE_PLAYING:
	case STATE_RECORDING:
		{
		const float fStartSeconds = GetAppropriateTiming().GetElapsedTimeFromBeat( GetBeat() );
		LOG->Trace( "Starting playback at %f", fStartSeconds );

		RageSoundParams p;
		p.m_fSpeed = GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;
		p.m_StartSecond = fStartSeconds;
		p.StopMode = RageSoundParams::M_CONTINUE;
		m_pSoundMusic->SetProperty( "AccurateSync", true );
		m_pSoundMusic->Play(false, &p);
		break;
		}
	default: break;
	}

	m_EditState = em;
}

void ScreenEdit::ScrollTo( float fDestinationBeat )
{
	CLAMP( fDestinationBeat, 0, GetMaximumBeatForMoving() );

	// Don't play the sound and do the hold note logic below if our position didn't change.
	const float fOriginalBeat = GetAppropriatePosition().m_fSongBeat;
	if( fOriginalBeat == fDestinationBeat )
		return;

	SetBeat(fDestinationBeat);

	// check to see if they're holding a button
	for( int n=0; n<NUM_EDIT_BUTTON_COLUMNS; n++ )
	{
		int iCol = n;

		// Ctrl + number = input to right half
		if( EditIsBeingPressed(EDIT_BUTTON_RIGHT_SIDE) )
			ShiftToRightSide( iCol, m_NoteDataEdit.GetNumTracks() );

		if( iCol >= m_NoteDataEdit.GetNumTracks() )
			continue;	// skip

		EditButton b = EditButton(EDIT_BUTTON_COLUMN_0+n);
		if( !EditIsBeingPressed(b) )
			continue;

		// create a new hold note
		int iStartRow = BeatToNoteRow( min(fOriginalBeat, fDestinationBeat) );
		int iEndRow = BeatToNoteRow( max(fOriginalBeat, fDestinationBeat) );

		// Don't SaveUndo.  We want to undo the whole hold, not just the last segment
		// that the user made.  Dragging the hold bigger can only absorb and remove
		// other taps, so dragging won't cause us to exceed the note limit.
		TapNote tn = EditIsBeingPressed(EDIT_BUTTON_LAY_ROLL) ? TAP_ORIGINAL_ROLL_HEAD : TAP_ORIGINAL_HOLD_HEAD;

		tn.pn = m_InputPlayerNumber;
		m_NoteDataEdit.AddHoldNote( iCol, iStartRow, iEndRow, tn );
	}

	if( EditIsBeingPressed(EDIT_BUTTON_SCROLL_SELECT) )
	{
		/* Shift is being held.
		 * If this is the first time we've moved since shift was depressed,
		 * the old position (before this move) becomes the start pos: */
		int iDestinationRow = BeatToNoteRow( fDestinationBeat );
		if( m_iShiftAnchor == -1 )
			m_iShiftAnchor = BeatToNoteRow(fOriginalBeat);

		if( iDestinationRow == m_iShiftAnchor )
		{
			// We're back at the anchor, so we have nothing selected.
			m_NoteFieldEdit.m_iBeginMarker = m_NoteFieldEdit.m_iEndMarker = -1;
		}
		else
		{
			m_NoteFieldEdit.m_iBeginMarker = m_iShiftAnchor;
			m_NoteFieldEdit.m_iEndMarker = iDestinationRow;
			if( m_NoteFieldEdit.m_iBeginMarker > m_NoteFieldEdit.m_iEndMarker )
				swap( m_NoteFieldEdit.m_iBeginMarker, m_NoteFieldEdit.m_iEndMarker );
		}
	}

	m_soundChangeLine.Play(true);
}

static LocalizedString NEW_KEYSOUND_FILE("ScreenEdit", "Enter New Keysound File");

void ScreenEdit::HandleMessage( const Message &msg )
{
	if( msg == "Judgment" )
	{
		PlayerNumber pn;
		msg.GetParam( "Player", pn );

		if( GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetCurrent().m_bMuteOnError )
		{
			RageSoundReader *pSoundReader = m_AutoKeysounds.GetPlayerSound( pn );
			if( pSoundReader == nullptr )
				pSoundReader = m_AutoKeysounds.GetSharedSound();

			HoldNoteScore hns;
			msg.GetParam( "HoldNoteScore", hns );
			TapNoteScore tns;
			msg.GetParam( "TapNoteScore", tns );

			bool bOn = false;
			if( hns != HoldNoteScore_Invalid )
				bOn = hns != HNS_LetGo;
			else
				bOn = tns != TNS_Miss;

			if( pSoundReader )
				pSoundReader->SetProperty( "Volume", bOn? 1.0f:0.0f );
		}
	}
	if( msg == Message_SongModified )
	{
		SetDirty( true );
	}
	Screen::HandleMessage( msg );
}

static LocalizedString SAVE_SUCCESSFUL				( "ScreenEdit", "Save successful." );
static LocalizedString AUTOSAVE_SUCCESSFUL				( "ScreenEdit", "Autosave successful." );
static LocalizedString SAVE_SUCCESS_NO_SM_SPLIT_TIMING("ScreenEdit", "save_success_no_sm_split_timing");

static LocalizedString ADD_NEW_MOD ("ScreenEdit", "Adding New Mod");
static LocalizedString ADD_NEW_ATTACK ("ScreenEdit", "Adding New Attack");
static LocalizedString EDIT_EXISTING_MOD ("ScreenEdit", "Edit Existing Mod");
static LocalizedString EDIT_ATTACK_START ("ScreenEdit", "Edit Attack Start");
static LocalizedString EDIT_ATTACK_LENGTH ("ScreenEdit", "Edit Attack Length");

void ScreenEdit::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM != SM_UpdateTextInfo )
		m_bTextInfoNeedsUpdate = true;

	if( SM == SM_UpdateTextInfo )
	{
		UpdateTextInfo();
		this->PostScreenMessage( SM_UpdateTextInfo, 0.1f );
	}
	else if( SM == SM_GoToNextScreen )
	{
		GAMESTATE->m_EditMode = EditMode_Invalid;
	}
	else if( SM == SM_BackFromMainMenu )
	{
		HandleMainMenuChoice( (MainMenuChoice)ScreenMiniMenu::s_iLastRowCode, ScreenMiniMenu::s_viLastAnswers );
	}
	else if( SM == SM_BackFromAreaMenu )
	{
		AreaMenuChoice amc = static_cast<AreaMenuChoice>(ScreenMiniMenu::s_iLastRowCode);
		const vector<int> &answers = ScreenMiniMenu::s_viLastAnswers;
		HandleAreaMenuChoice( amc, answers );
	}
	else if( SM == SM_BackFromAlterMenu )
	{
		HandleAlterMenuChoice( (AlterMenuChoice)ScreenMiniMenu::s_iLastRowCode, ScreenMiniMenu::s_viLastAnswers );
	}
	else if( SM == SM_BackFromArbitraryRemap )
	{
		HandleArbitraryRemapping(ScreenTextEntry::s_sLastAnswer);
	}
	else if( SM == SM_BackFromStepsInformation )
	{
		HandleStepsInformationChoice( (StepsInformationChoice)ScreenMiniMenu::s_iLastRowCode, ScreenMiniMenu::s_viLastAnswers );
	}
	else if (SM == SM_BackFromStepsData)
	{
		HandleStepsDataChoice((StepsDataChoice)ScreenMiniMenu::s_iLastRowCode,
							  ScreenMiniMenu::s_viLastAnswers);
	}

	else if( SM == SM_BackFromSongInformation )
	{
		HandleSongInformationChoice( (SongInformationChoice)ScreenMiniMenu::s_iLastRowCode, ScreenMiniMenu::s_viLastAnswers );
	}
	else if( SM == SM_BackFromTimingDataInformation )
	{
		HandleTimingDataInformationChoice( (TimingDataInformationChoice)ScreenMiniMenu::s_iLastRowCode, ScreenMiniMenu::s_viLastAnswers );
	}
	else if(SM == SM_BackFromTimingDataChangeInformation)
	{
		HandleTimingDataChangeChoice((TimingDataChangeChoice)ScreenMiniMenu::s_iLastRowCode, ScreenMiniMenu::s_viLastAnswers);
	}
	else if( SM == SM_BackFromDifficultyMeterChange )
	{
		int i = std::stoi( ScreenTextEntry::s_sLastAnswer );
		GAMESTATE->m_pCurSteps[PLAYER_1]->SetMeter(i);
		SetDirty( true );
	}
	else if( SM == SM_BackFromBeat0Change && !ScreenTextEntry::s_bCancelledLast )
	{
		float fBeat0 = StringToFloat( ScreenTextEntry::s_sLastAnswer );

		TimingData &timing = GetAppropriateTimingForUpdate();
		float old = timing.m_fBeat0OffsetInSeconds;
		timing.m_fBeat0OffsetInSeconds = fBeat0;
		float delta = timing.m_fBeat0OffsetInSeconds - old;

		if (GAMESTATE->m_bIsUsingStepTiming)
		{
			GAMESTATE->m_pCurSteps[PLAYER_1]->m_Attacks.UpdateStartTimes(delta);
		}
		else
		{
			GAMESTATE->m_pCurSong->m_Attacks.UpdateStartTimes(delta);
			GAMESTATE->m_pCurSong->m_fMusicSampleStartSeconds += delta;
		}

		SetDirty( true );
	}
	else if( SM == SM_BackFromBPMChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fBPM = StringToFloat( ScreenTextEntry::s_sLastAnswer );

		if( fBPM > 0 )
			GetAppropriateTimingForUpdate().AddSegment( BPMSegment(GetRow(), fBPM) );

		SetDirty( true );
	}
	else if( SM == SM_BackFromStopChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fStop = StringToFloat( ScreenTextEntry::s_sLastAnswer );

		if( fStop >= 0 )
			GetAppropriateTimingForUpdate().AddSegment( StopSegment(GetRow(), fStop) );

		SetDirty( true );
	}
	else if( SM == SM_BackFromDelayChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fDelay = StringToFloat( ScreenTextEntry::s_sLastAnswer );

		if( fDelay >= 0 )
			GetAppropriateTimingForUpdate().AddSegment( DelaySegment(GetRow(), fDelay) );

		SetDirty( true );
	}
	else if ( SM == SM_BackFromTickcountChange && !ScreenTextEntry::s_bCancelledLast )
	{
		int iTick = std::stoi( ScreenTextEntry::s_sLastAnswer );

		if ( iTick >= 0 && iTick <= ROWS_PER_BEAT )
			GetAppropriateTimingForUpdate().AddSegment( TickcountSegment( GetRow(), iTick) );

		SetDirty( true );
	}
	else if ( SM == SM_BackFromComboChange && !ScreenTextEntry::s_bCancelledLast )
	{
		int iCombo, iMiss;

		if (sscanf(ScreenTextEntry::s_sLastAnswer.c_str(), " %d / %d ", &iCombo, &iMiss) == 2)
			GetAppropriateTimingForUpdate().AddSegment( ComboSegment(GetRow(), iCombo, iMiss) );

		SetDirty( true );
	}
	else if ( SM == SM_BackFromLabelChange && !ScreenTextEntry::s_bCancelledLast )
	{
		RString sLabel = ScreenTextEntry::s_sLastAnswer;

		if ( !GetAppropriateTiming().DoesLabelExist(sLabel) )
		{
			// XXX: these should be in the NotesWriters where they're needed.
			sLabel.Replace("=", "_");
			sLabel.Replace(",", "_");
			GetAppropriateTimingForUpdate().AddSegment( LabelSegment(GetRow(), sLabel) );
			SetDirty( true );
		}
	}
	else if ( SM == SM_BackFromWarpChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fWarp = StringToFloat( ScreenTextEntry::s_sLastAnswer );
		if( fWarp >= 0 ) // allow 0 to kill a warp.
		{
			GetAppropriateTimingForUpdate().SetWarpAtBeat( GetBeat(), fWarp );
			SetDirty( true );
		}
	}
	else if( SM == SM_BackFromSpeedPercentChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fNum = StringToFloat( ScreenTextEntry::s_sLastAnswer );
		GetAppropriateTimingForUpdate().SetSpeedPercentAtBeat( GetBeat(), fNum );
		SetDirty( true );
	}
	else if ( SM == SM_BackFromSpeedWaitChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fDen = StringToFloat( ScreenTextEntry::s_sLastAnswer );
		if( fDen >= 0)
		{
			GetAppropriateTimingForUpdate().SetSpeedWaitAtBeat( GetBeat(), fDen );
		}
		SetDirty( true );
	}
	else if ( SM == SM_BackFromSpeedModeChange && !ScreenTextEntry::s_bCancelledLast )
	{
		if( ScreenTextEntry::s_sLastAnswer.substr(0, 1) == "b" || ScreenTextEntry::s_sLastAnswer.substr(0, 1) == "B" )
		{
			GetAppropriateTimingForUpdate().SetSpeedModeAtBeat( GetBeat(), SpeedSegment::UNIT_BEATS );
		}
		else if( ScreenTextEntry::s_sLastAnswer.substr(0, 1) == "s" || ScreenTextEntry::s_sLastAnswer.substr(0, 1) == "S" )
		{
			GetAppropriateTimingForUpdate().SetSpeedModeAtBeat( GetBeat(), SpeedSegment::UNIT_SECONDS );
		}
		else
		{
			int tmp = std::stoi(ScreenTextEntry::s_sLastAnswer );

			SpeedSegment::BaseUnit unit = (tmp == 0 ) ?
			  SpeedSegment::UNIT_BEATS : SpeedSegment::UNIT_SECONDS;

			GetAppropriateTimingForUpdate().SetSpeedModeAtBeat( GetBeat(), unit );
		}
		SetDirty( true );
	}
	else if( SM == SM_BackFromScrollChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fNum = StringToFloat( ScreenTextEntry::s_sLastAnswer );
		GetAppropriateTimingForUpdate().SetScrollAtBeat( GetBeat(), fNum );
		SetDirty( true );
	}
	else if ( SM == SM_BackFromFakeChange && !ScreenTextEntry::s_bCancelledLast )
	{
		float fFake = StringToFloat( ScreenTextEntry::s_sLastAnswer );
		if( fFake >= 0 ) // allow 0 to kill a fake.
		{
			GetAppropriateTimingForUpdate().AddSegment( FakeSegment(GetRow(), fFake) );
			SetDirty( true );
		}
	}
	else if ( SM == SM_BackFromStepMusicChange && !ScreenTextEntry::s_bCancelledLast )
	{
		// Reload the music because it just changed. -Kyz
		m_AutoKeysounds.FinishLoading();
		m_pSoundMusic = m_AutoKeysounds.GetSound();
	}
	else if( SM == SM_BackFromBGChange )
	{
		HandleBGChangeChoice( (BGChangeChoice)ScreenMiniMenu::s_iLastRowCode, ScreenMiniMenu::s_viLastAnswers );
		SetDirty( true );
	}
	else if( SM == SM_BackFromCourseModeMenu )
	{
		const int num = ScreenMiniMenu::s_viLastAnswers[0];
		GAMESTATE->m_pCurCourse.Set(nullptr);
		if( num != 0 )
		{
			const RString name = g_CourseMode.rows[0].choices[num];
			Course *pCourse = SONGMAN->FindCourse( name );

			int iCourseEntryIndex = -1;
			int index = 0;
			for (CourseEntry const &i : pCourse->m_vEntries)
			{
				if( i.songID.ToSong() == GAMESTATE->m_pCurSong.Get() )
					iCourseEntryIndex = index;
				++index;
			}

			ASSERT( iCourseEntryIndex != -1 );

			GAMESTATE->m_pCurCourse.Set( pCourse );
			GAMESTATE->m_iEditCourseEntryIndex.Set( iCourseEntryIndex );
			ASSERT( GAMESTATE->m_pCurCourse != nullptr );
		}
	}
	else if (SM == SM_BackFromKeysoundTrack)
	{
		const int track = ScreenMiniMenu::s_iLastRowCode;
		const int tracks = m_NoteDataEdit.GetNumTracks();
		const int row = this->GetRow();
		unsigned int sound = ScreenMiniMenu::s_viLastAnswers[track];
		vector<RString> &kses = m_pSong->m_vsKeysoundFile;

		if (track < tracks)
		{
			if (sound == kses.size())
			{
				// create a new sound (filename), point it.
				// if it's empty, make it an auto keysound.
				ScreenTextEntry::TextEntry(SM_BackFromNewKeysound, NEW_KEYSOUND_FILE, "", 64);
				return;
			}
			const TapNote &oldNote = m_NoteDataEdit.GetTapNote(track, row);
			TapNote newNote = oldNote; // need to lose the const. not feeling like casting.
			if (sound < kses.size())
			{
				// set note at this row to use this keysound file.
				// if it's empty, make it an auto keysound.
				newNote.iKeysoundIndex = sound;
				if (newNote.type == TapNoteType_Empty)
				{
					newNote.type = TapNoteType_AutoKeysound; // keysounds need something non empty.
				}
			}
			else // sound > kses.size()
			{
				// remove the sound. if it's an auto keysound, make it empty.
				newNote.iKeysoundIndex = -1;
				if (newNote.type == TapNoteType_AutoKeysound)
				{
					newNote.type = TapNoteType_Empty; // autoKeysound with no sound is pointless.
				}
			}
			m_NoteDataEdit.SetTapNote(track, row, newNote);
		}
		else if (track == tracks)
		{
			kses.erase(kses.begin() + sound);
			// TODO: Make the following a part of NoteData?
			for (int t = 0; t < tracks; ++t)
			{
				FOREACH_NONEMPTY_ROW_IN_TRACK(m_NoteDataEdit, t, r)
				{
					const TapNote &oldNote = m_NoteDataEdit.GetTapNote(t, r);
					TapNote newNote = oldNote; // need to lose the const. not feeling like casting.
					if (newNote.iKeysoundIndex == static_cast<int>(sound))
					{
						newNote.iKeysoundIndex = -1;
						if (newNote.type == TapNoteType_AutoKeysound)
							newNote.type = TapNoteType_Empty;
					}
					else if (newNote.iKeysoundIndex > static_cast<int>(sound))
						newNote.iKeysoundIndex--;

					m_NoteDataEdit.SetTapNote(t, r, newNote);
				}
			}
		}
		SetDirty(true);
	}
	else if (SM == SM_BackFromNewKeysound && !ScreenTextEntry::s_bCancelledLast)
	{
		RString answer = ScreenTextEntry::s_sLastAnswer;
		const int track = ScreenMiniMenu::s_iLastRowCode; // still keeps the same value.
		const int row = this->GetRow();
		const TapNote &oldNote = m_NoteDataEdit.GetTapNote(track, row);
		TapNote newNote = oldNote; // need to lose the const. not feeling like casting.
		vector<RString> &kses = m_pSong->m_vsKeysoundFile;
		unsigned pos = find(kses.begin(), kses.end(), answer) - kses.begin();
		if (pos == kses.size())
		{
			newNote.iKeysoundIndex = kses.size();
			kses.push_back(answer);
		}
		else
		{
			newNote.iKeysoundIndex = pos;
		}
		if (newNote.type == TapNoteType_Empty)
			newNote.type = TapNoteType_AutoKeysound; // keysounds need something non empty.
		m_NoteDataEdit.SetTapNote(track, row, newNote);
		SetDirty(true);
	}
	else if( SM == SM_BackFromOptions )
	{
		// The options may have changed the note skin.
		m_NoteFieldRecord.CacheAllUsedNoteSkins();
		m_Player->CacheAllUsedNoteSkins();

		// stop any music that screen may have been playing
		SOUND->StopMusic();
	}
	else if( SM == SM_BackFromInsertTapAttack )
	{
		int iDurationChoice = ScreenMiniMenu::s_viLastAnswers[0];
		g_fLastInsertAttackDurationSeconds = StringToFloat( g_InsertTapAttack.rows[0].choices[iDurationChoice] );

		SCREENMAN->AddNewScreenToTop( SET_MOD_SCREEN, SM_BackFromInsertTapAttackPlayerOptions );
	}
	else if( SM == SM_BackFromInsertTapAttackPlayerOptions )
	{
		PlayerOptions poChosen = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetPreferred();
		RString sMods = poChosen.GetString();
		const int row = BeatToNoteRow( GAMESTATE->m_Position.m_fSongBeat );

		TapNote tn(
			TapNoteType_Attack,
			TapNoteSubType_Invalid,
			TapNoteSource_Original,
			sMods,
			g_fLastInsertAttackDurationSeconds,
			-1 );
		tn.pn = m_InputPlayerNumber;
		SetDirty( true );
		SaveUndo();
		m_NoteDataEdit.SetTapNote( g_iLastInsertTapAttackTrack, row, tn );
		CheckNumberOfNotesAndUndo();
	}
	else if (SM == SM_BackFromEditingAttackStart && !ScreenTextEntry::s_bCancelledLast)
	{
		float time = StringToFloat(ScreenTextEntry::s_sLastAnswer);
		AttackArray &attacks =
		(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
		Attack &attack = attacks[attackInProcess];
		attack.fStartSecond = time;
		SetDirty(true);
	}
	else if (SM == SM_BackFromEditingAttackLength && !ScreenTextEntry::s_bCancelledLast)
	{
		float time = StringToFloat(ScreenTextEntry::s_sLastAnswer);
		AttackArray &attacks =
		(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
		Attack &attack = attacks[attackInProcess];
		attack.fSecsRemaining = time;
		SetDirty(true);
	}
	else if (SM == SM_BackFromAddingModToExistingAttack && !ScreenTextEntry::s_bCancelledLast)
	{
		RString mod = ScreenTextEntry::s_sLastAnswer;
		Trim(mod);
		if (mod.length() > 0)
		{
			AttackArray &attacks =
			(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
			Attack &attack = attacks[attackInProcess];
			attack.sModifiers += "," + mod;
			SetDirty(true);
		}

	}
	else if (SM == SM_BackFromEditingModToExistingAttack && !ScreenTextEntry::s_bCancelledLast)
	{
		AttackArray &attacks =
		(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
		Attack &attack = attacks[attackInProcess];
		vector<RString> mods;
		split(attack.sModifiers, ",", mods);
		RString mod = ScreenTextEntry::s_sLastAnswer;
		Trim(mod);
		if (mod.length() > 0)
		{
			mods[modInProcess - 2] = mod;
		}
		else
		{
			mods.erase(mods.begin() + (modInProcess - 2));
		}
		if (mods.size() > 0)
		{
			attack.sModifiers = join(",", mods);
		}
		else
		{
			attacks.erase(attacks.begin() + attackInProcess);
		}
		SetDirty(true);
	}
	else if (SM == SM_BackFromInsertStepAttack)
	{
		unsigned option = ScreenMiniMenu::s_iLastRowCode;
		AttackArray &attacks =
		(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
		Attack &attack = attacks[attackInProcess];
		vector<RString> mods;
		split(attack.sModifiers, ",", mods);
		modInProcess = option;
		if (option == 0) // adjusting the starting time
		{
			ScreenTextEntry::TextEntry(SM_BackFromEditingAttackStart,
									   EDIT_ATTACK_START,
									   std::to_string(attack.fStartSecond),
									   10);
		}
		else if (option == 1) // adjusting the length of the attack
		{
			ScreenTextEntry::TextEntry(SM_BackFromEditingAttackLength,
									   EDIT_ATTACK_LENGTH,
									   std::to_string(attack.fSecsRemaining),
									   10);
		}
		else if (option >= 2 + mods.size()) // adding a new mod
		{
			ScreenTextEntry::TextEntry(SM_BackFromAddingModToExistingAttack,
									   ADD_NEW_MOD,
									   "",
									   64);
		}
		else // modifying existing mod.
		{
			ScreenTextEntry::TextEntry(SM_BackFromEditingModToExistingAttack,
									   EDIT_EXISTING_MOD,
									   mods[option - 2],
									   64);
		}
	}
	else if (SM == SM_BackFromAddingAttackToChart && !ScreenTextEntry::s_bCancelledLast)
	{
		RString mod = ScreenTextEntry::s_sLastAnswer;
		Trim(mod);
		if (mod.length() > 0)
		{
			AttackArray &attacks =
			(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
			Attack attack;
			attack.fStartSecond = GetAppropriateTiming().GetElapsedTimeFromBeat(GetBeat());
			attack.fSecsRemaining = 5; // Users can change later.
			attack.sModifiers = mod;
			attacks.push_back(attack);
			SetDirty(true);
		}
	}
	else if (SM == SM_BackFromAttackAtTime)
	{
		int attackChoice = ScreenMiniMenu::s_iLastRowCode;
		int attackDecision = ScreenMiniMenu::s_viLastAnswers[attackChoice];
		const TimingData &timing = GetAppropriateTiming();
		float startTime = timing.GetElapsedTimeFromBeat(GetBeat());
		AttackArray &attacks =
		(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
		vector<int> points = FindAllAttacksAtTime(attacks, startTime);
		if (attackChoice == (int)points.size())
		{
			// TODO: Add attack code.
			ScreenTextEntry::TextEntry(SM_BackFromAddingAttackToChart,
									   ADD_NEW_ATTACK,
									   "",
									   64);
		}
		else
		{
			attackInProcess = points[attackChoice];
			if (attackDecision == 1) // remove
			{
				attacks.erase(attacks.begin() + attackInProcess);
				SetDirty(true);
			}
			else
			{
				Attack &attack = attacks[attackInProcess];
				g_IndividualAttack.rows.clear();

				g_IndividualAttack.rows.push_back(MenuRowDef(0,
															 "Starting Time",
															 true,
															 EditMode_CourseMods,
															 true,
															 true,
															 0,
															 nullptr));
				g_IndividualAttack.rows[0].SetOneUnthemedChoice(std::to_string(attack.fStartSecond));
				g_IndividualAttack.rows.push_back(MenuRowDef(1,
															 "Secs Remaining",
															 true,
															 EditMode_CourseMods,
															 true,
															 true,
															 0,
															 nullptr));
				g_IndividualAttack.rows[1].SetOneUnthemedChoice(std::to_string(attack.fSecsRemaining));
				vector<RString> mods;
				split(attack.sModifiers, ",", mods);
				for (unsigned i = 0; i < mods.size(); ++i)
				{
					unsigned col = i + 2;
					g_IndividualAttack.rows.push_back(MenuRowDef(col,
																 ssprintf("Attack %d", i + 1),
																 true,
																 EditMode_CourseMods,
																 false,
																 true,
																 0,
																 nullptr));
					g_IndividualAttack.rows[col].SetOneUnthemedChoice(mods[i].c_str());
				}

				g_IndividualAttack.rows.push_back(MenuRowDef(mods.size() + 2,
															 "Add Mod",
															 true,
															 EditMode_CourseMods,
															 true,
															 true,
															 0,
															 nullptr));

				EditMiniMenu(&g_IndividualAttack, SM_BackFromInsertStepAttack);
			}
		}
	}
	else if (SM == SM_BackFromInsertStepAttack)
	{
		int iDurationChoice = ScreenMiniMenu::s_viLastAnswers[0];
		const TimingData &timing = GetAppropriateTiming();
		g_fLastInsertAttackPositionSeconds = timing.GetElapsedTimeFromBeat( GetBeat() );
		g_fLastInsertAttackDurationSeconds = StringToFloat( g_InsertStepAttack.rows[0].choices[iDurationChoice] );
		AttackArray &attacks = GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks;
		int iAttack = FindAttackAtTime(attacks, g_fLastInsertAttackPositionSeconds);

		if (ScreenMiniMenu::s_iLastRowCode == ScreenEdit::remove )
		{
			if (iAttack > 0)
				attacks.erase(attacks.begin() + iAttack);
		}
		else
		{
			ModsGroup<PlayerOptions> &toEdit = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions;
			this->originalPlayerOptions.Assign(ModsLevel_Preferred, toEdit.GetPreferred());
			PlayerOptions po;
			if (iAttack >= 0)
				po.FromString(attacks[iAttack].sModifiers);

			toEdit.Assign( ModsLevel_Preferred, po );
			SCREENMAN->AddNewScreenToTop( SET_MOD_SCREEN, SM_BackFromInsertStepAttackPlayerOptions );
		}
		SetDirty(true);
	}
	else if( SM == SM_BackFromInsertCourseAttack )
	{
		int iDurationChoice = ScreenMiniMenu::s_viLastAnswers[0];
		Course *pCourse = GAMESTATE->m_pCurCourse;
		CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
		int iAttack;

		// TODO: Handle Song/Step Timing functions/switches here?

		g_fLastInsertAttackPositionSeconds = m_pSteps->GetTimingData()->GetElapsedTimeFromBeat( GAMESTATE->m_Position.m_fSongBeat );
		g_fLastInsertAttackDurationSeconds = StringToFloat( g_InsertCourseAttack.rows[0].choices[iDurationChoice] );
		iAttack = FindAttackAtTime( ce.attacks, g_fLastInsertAttackPositionSeconds );

		if( ScreenMiniMenu::s_iLastRowCode == ScreenEdit::remove )
		{
			ASSERT( iAttack >= 0 );
			ce.attacks.erase( ce.attacks.begin() + iAttack );
		}
		else
		{
			PlayerOptions po;

			if( iAttack >= 0 )
				po.FromString( ce.attacks[iAttack].sModifiers );

			GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.Assign( ModsLevel_Preferred, po );
			SCREENMAN->AddNewScreenToTop( SET_MOD_SCREEN, SM_BackFromInsertCourseAttackPlayerOptions );
		}
	}
	else if (SM == SM_BackFromInsertStepAttackPlayerOptions)
	{
		ModsGroup<PlayerOptions> &toRestore = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions;
		PlayerOptions poChosen = toRestore.GetPreferred();
		RString mods = poChosen.GetString();

		if (g_fLastInsertAttackPositionSeconds >= 0)
		{
			Attack a(ATTACK_LEVEL_1,
				 g_fLastInsertAttackPositionSeconds,
				 g_fLastInsertAttackDurationSeconds,
				 mods,
				 false,
				 false);
			AttackArray &attacks = GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks;
			int index = FindAttackAtTime(attacks, g_fLastInsertAttackPositionSeconds);
			if (index >= 0)
				attacks[index] = a;
			else
				attacks.push_back(a);
		}
		toRestore.Assign(ModsLevel_Preferred, this->originalPlayerOptions.GetPreferred());
	}
	else if( SM == SM_BackFromInsertCourseAttackPlayerOptions )
	{
		PlayerOptions poChosen = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetPreferred();
		RString sMods = poChosen.GetString();

		Course *pCourse = GAMESTATE->m_pCurCourse;
		CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
		if( g_fLastInsertAttackPositionSeconds < 0 )
		{
			ce.sModifiers = sMods;
		}
		else
		{
			Attack a(
				 ATTACK_LEVEL_1,
				 g_fLastInsertAttackPositionSeconds,
				 g_fLastInsertAttackDurationSeconds,
				 sMods,
				 false,
				 false );
			int iAttack = FindAttackAtTime( ce.attacks, g_fLastInsertAttackPositionSeconds );

			if( iAttack >= 0 )
				ce.attacks[iAttack] = a;
			else
				ce.attacks.push_back( a );
		}
	}
	else if( SM == SM_DoRevertToLastSave )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			SaveUndo();
			CopyFromLastSave();
			m_pSteps->GetNoteData( m_NoteDataEdit );
			SetDirty( false );
		}
	}
	else if( SM == SM_DoRevertFromDisk )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			SaveUndo();
			RevertFromDisk();
			m_pSteps->GetNoteData( m_NoteDataEdit );
			SetDirty( false );
		}
	}
	else if(SM == SM_ConfirmClearArea)
	{
		if(ScreenPrompt::s_LastAnswer == ANSWER_YES)
		{
			m_NoteDataEdit.ClearRange(
				m_NoteFieldEdit.m_iBeginMarker, m_NoteFieldEdit.m_iEndMarker);
		}
	}
	else if( SM == SM_DoEraseStepTiming )
	{
		if( ScreenPrompt::s_LastAnswer == ANSWER_YES )
		{
			SaveUndo();
			m_pSteps->m_Timing.Clear();
			SetDirty( true );
		}
	}
	else if( SM == SM_DoSaveAndExit ) // just asked "save before exiting? yes, no, cancel"
	{
		switch( ScreenPrompt::s_LastAnswer )
		{
		DEFAULT_FAIL( ScreenPrompt::s_LastAnswer );
		case ANSWER_YES:
			// This will send SM_SaveSuccessful or SM_SaveFailed.
			HandleMainMenuChoice( ScreenEdit::save_on_exit );
			return;
		case ANSWER_NO:
			// Don't save; just exit.
			m_pSong->RemoveAutosave();
			SCREENMAN->SendMessageToTopScreen( SM_DoExit );
			return;
		case ANSWER_CANCEL:
			break; // do nothing
		}
	}
	else if(SM == SM_SaveSuccessful || SM == SM_SaveSuccessNoSM)
	{
		LOG->Trace( "Save successful." );
		CopyToLastSave();
		SetDirty( false );
		SONGMAN->Invalidate( GAMESTATE->m_pCurSong );

		LocalizedString const* message= &SAVE_SUCCESSFUL;
		if(SM == SM_SaveSuccessNoSM)
		{
			message= &SAVE_SUCCESS_NO_SM_SPLIT_TIMING;
		}

		if( m_CurrentAction == save_on_exit )
		{
			ScreenPrompt::Prompt( SM_DoExit, *message );
		}
		else
		{
			SCREENMAN->SystemMessage( *message );
		}
	}
	else if( SM == SM_AutoSaveSuccessful )
	{
		LOG->Trace("AutoSave successful.");
		m_next_autosave_time= RageTimer::GetTimeSinceStartFast() + time_between_autosave;
		SCREENMAN->SystemMessage(AUTOSAVE_SUCCESSFUL);
	}
	else if( SM == SM_SaveFailed ) // save failed; stay in the editor
	{
		/* We committed the steps to SongManager. Revert to the last save, and
		 * recommit the reversion to SongManager. */
		LOG->Trace( "Save failed. Changes uncommitted from memory." );
		CopyFromLastSave();
		m_pSteps->SetNoteData( m_NoteDataEdit );
	}
	else if( SM == SM_DoExit )
	{
		// IMPORTANT: CopyFromLastSave before deleting the Steps below
		CopyFromLastSave();

		/* The user has been given a choice to save.
		 * Delete all unsaved steps before exiting the editor. */
		/* FIXME: This code causes all the steps to be deleted if you quit
		 * without saving. However, without this code, any new steps will get
		 * saved on quit. -aj */

		// At this point, the last good song copy is in use.
		Song *pSong = GAMESTATE->m_pCurSong;
		const vector<Steps*> &apSteps = pSong->GetAllSteps();
		vector<Steps*> apToDelete;
		for (Steps *s : apSteps)
		{
			// If we're not on the same style, let it go.
			if( GAMESTATE->m_pCurSteps[PLAYER_1]->m_StepsType != s->m_StepsType )
				continue;
			// If autogenned, it isn't being saved.
			if( s->IsAutogen() )
				continue;
			// If the notedata has content, let it go.
			if( !s->GetNoteData().IsEmpty() )
				continue;
			// It's hard to say if these steps were saved to disk or not.
			/*
			if( !(s->GetSavedToDisk() )
				continue;
			 */
			apToDelete.push_back( s );
		}
		for (Steps *pSteps : apToDelete)
		{
			pSong->DeleteSteps( pSteps );
			if( m_pSteps == pSteps )
				m_pSteps = nullptr;
			if( GAMESTATE->m_pCurSteps[PLAYER_1].Get() == pSteps )
				GAMESTATE->m_pCurSteps[PLAYER_1].Set(nullptr);
		}


		m_Out.StartTransitioning( SM_GoToNextScreen );
	}
	else if( SM == SM_GainFocus )
	{
		/* When another screen comes up, RageSounds takes over the sound timer.  When we come
		 * back, put the timer back to where it was. */
		SetBeat(m_fTrailingBeat);
	}
	else if( SM == SM_LoseFocus )
	{
		// Snap the trailing beat, in case we lose focus while tweening.
		m_fTrailingBeat = GetBeat(); //GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat;
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenEdit::SetDirty(bool dirty)
{
	if(EDIT_MODE.GetValue() != EditMode_Full)
	{
		m_dirty= false;
		m_next_autosave_time= -1.0f;
		return;
	}
	if(dirty)
	{
		if(!m_dirty)
		{
			m_next_autosave_time= RageTimer::GetTimeSinceStartFast() + time_between_autosave;
		}
	}
	else
	{
		m_next_autosave_time= -1.0f;
	}
	m_dirty= dirty;
}

void ScreenEdit::PerformSave(bool autosave)
{
	// copy edit into current Steps
	m_pSteps->SetNoteData( m_NoteDataEdit );

	// don't forget the attacks.
	m_pSong->m_Attacks = GAMESTATE->m_pCurSong->m_Attacks;
	m_pSong->m_sAttackString = GAMESTATE->m_pCurSong->m_Attacks.ToVectorString();
	m_pSteps->m_Attacks = GAMESTATE->m_pCurSteps[PLAYER_1]->m_Attacks;
	m_pSteps->m_sAttackString = GAMESTATE->m_pCurSteps[PLAYER_1]->m_Attacks.ToVectorString();

	// If one of the charts uses split timing, then it cannot be accurately
	// saved in the .sm format.  So saving the .sm is disabled.
	bool uses_split= m_pSong->AnyChartUsesSplitTiming();
	const ScreenMessage save_message= autosave ? SM_AutoSaveSuccessful
		: (uses_split ? SM_SaveSuccessNoSM : SM_SaveSuccessful);

	switch( EDIT_MODE.GetValue() )
	{
		DEFAULT_FAIL( EDIT_MODE.GetValue() );
		case EditMode_Home:
			{
				ASSERT( m_pSteps->IsAnEdit() );

				RString sError;
				m_pSteps->CalculateRadarValues( m_pSong->m_fMusicLengthSeconds );
				if( !NotesWriterSM::WriteEditFileToMachine(m_pSong, m_pSteps, sError) )
				{
					ScreenPrompt::Prompt( SM_None, sError );
					break;
				}

				m_pSteps->SetSavedToDisk( true );

				// HACK: clear undo, so "exit" below knows we don't need to save.
				// This only works because important non-steps data can't be changed in
				// home mode (BPMs, stops).
				ClearUndo();

				SCREENMAN->ZeroNextUpdate();

				HandleScreenMessage(save_message);

				/* FIXME
					 RString s;
					 switch( c )
					 {
					 case save:			s = "ScreenMemcardSaveEditsAfterSave";	break;
					 case save_on_exit:	s = "ScreenMemcardSaveEditsAfterExit";	break;
					 default:		FAIL_M(ssprintf("Invalid menu choice: %i", c));
					 }
					 SCREENMAN->AddNewScreenToTop( s );
				*/
			}
			break;
		case EditMode_Full:
			{
				// This will recalculate radar values.
				m_pSong->Save(autosave);
				SCREENMAN->ZeroNextUpdate();

				HandleScreenMessage(save_message);
			}
			break;
		case EditMode_CourseMods:
		case EditMode_Practice:
			break;
	}
	m_soundSave.Play(true);
}

void ScreenEdit::OnSnapModeChange()
{
	m_soundChangeSnap.Play(true);

	NoteType nt = m_SnapDisplay.GetNoteType();
	int iStepIndex = BeatToNoteRow( GetBeat() );
	int iElementsPerNoteType = BeatToNoteRow( NoteTypeToBeat(nt) );
	int iStepIndexHangover = iStepIndex % iElementsPerNoteType;
	SetBeat( GetBeat() - NoteRowToBeat( iStepIndexHangover ) );
}



// Helper function for below

// Begin helper functions for InputEdit


static void ChangeDescription( const RString &sNew )
{
	Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];

	// Don't erase edit descriptions.
	if( sNew.empty() && pSteps->GetDifficulty() == Difficulty_Edit )
		return;

	pSteps->SetDescription(sNew);
}

static void ChangeChartName( const RString &sNew )
{
	Steps *pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSteps->SetChartName(sNew);
}

static void ChangeChartStyle( const RString &sNew )
{
	Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSteps->SetChartStyle(sNew);
}

static void ChangeStepCredit( const RString &sNew )
{
	Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSteps->SetCredit(sNew);
}

static void ChangeStepMeter( const RString &sNew )
{
	int diff = std::stoi(sNew);
	GAMESTATE->m_pCurSteps[PLAYER_1]->SetMeter(max(diff, 1));
}

static void ChangeStepMusic(const RString& sNew)
{
	Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSteps->SetMusicFile(sNew);
}

static void ChangeMainTitle( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sMainTitle = sNew;
}

static void ChangeSubTitle( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sSubTitle = sNew;
}

static void ChangeArtist( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sArtist = sNew;
}

static void ChangeGenre( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sGenre = sNew;
}

static void ChangeCredit( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sCredit = sNew;
}

static void ChangePreview(const RString& sNew)
{
	Song* pSong = GAMESTATE->m_pCurSong;
	if(!sNew.empty())
	{
		RString error;
		RageSoundReader* sample= RageSoundReader_FileReader::OpenFile(pSong->GetPreviewMusicPath(), error);
		if(sample == nullptr)
		{
			LOG->UserLog( "Preview file", pSong->GetPreviewMusicPath(), "couldn't be opened: %s", error.c_str() );
		}
		else
		{
			pSong->m_fMusicSampleLengthSeconds= sample->GetLength() / 1000.0f;
			pSong->m_PreviewFile= sNew;
			delete sample;
		}
	}
	else
	{
		pSong->m_PreviewFile= sNew;
	}
}

static void ChangeMainTitleTranslit( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sMainTitleTranslit = sNew;
}

static void ChangeSubTitleTranslit( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sSubTitleTranslit = sNew;
}

static void ChangeArtistTranslit( const RString &sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sArtistTranslit = sNew;
}

static void ChangeLastSecondHint( const RString &sNew )
{
	Song &s = *GAMESTATE->m_pCurSong;
	s.SetSpecifiedLastSecond(StringToFloat(sNew));
}

static void ChangePreviewStart( const RString &sNew )
{
	GAMESTATE->m_pCurSong->m_fMusicSampleStartSeconds = StringToFloat( sNew );
}

static void ChangePreviewLength( const RString &sNew )
{
	GAMESTATE->m_pCurSong->m_fMusicSampleLengthSeconds = StringToFloat( sNew );
}

static void ChangeMinBPM( const RString &sNew )
{
	GAMESTATE->m_pCurSong->m_fSpecifiedBPMMin = StringToFloat( sNew );
}

static void ChangeStepsMinBPM(const RString &sNew)
{
	Steps *step = GAMESTATE->m_pCurSteps[PLAYER_1];
	step->SetMinBPM(StringToFloat(sNew));
}

static void ChangeMaxBPM( const RString &sNew )
{
	GAMESTATE->m_pCurSong->m_fSpecifiedBPMMax = StringToFloat( sNew );
}

static void ChangeStepsMaxBPM(const RString &sNew)
{
	Steps *step = GAMESTATE->m_pCurSteps[PLAYER_1];
	step->SetMaxBPM(StringToFloat(sNew));
}

const TimingData & ScreenEdit::GetAppropriateTiming() const
{
	if( GAMESTATE->m_bIsUsingStepTiming )
	{
		return *m_pSteps->GetTimingData();
	}
	return m_pSong->m_SongTiming;
}

TimingData & ScreenEdit::GetAppropriateTimingForUpdate()
{
	if( GAMESTATE->m_bIsUsingStepTiming )
	{
		// Copy from song if there is no step timing
		if( m_pSteps->m_Timing.empty() )
			m_pSteps->m_Timing = m_pSong->m_SongTiming;
		return m_pSteps->m_Timing;
	}
	return m_pSong->m_SongTiming;
}

SongPosition & ScreenEdit::GetAppropriatePosition() const
{
	if (GAMESTATE->m_bIsUsingStepTiming)
	{
		return GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position;
	}
	return GAMESTATE->m_Position;
}

inline void ScreenEdit::SetBeat(float fBeat)
{
	if( !GAMESTATE->m_bIsUsingStepTiming )
	{
		GAMESTATE->m_Position.m_fSongBeat = fBeat;
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat = m_pSteps->GetTimingData()->GetBeatFromElapsedTime(m_pSong->m_SongTiming.GetElapsedTimeFromBeat(fBeat));
	}
	else
	{
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat = fBeat;
		GAMESTATE->m_Position.m_fSongBeat = m_pSong->m_SongTiming.GetBeatFromElapsedTime(m_pSteps->GetTimingData()->GetElapsedTimeFromBeat(fBeat));
	}
}

inline float ScreenEdit::GetBeat()
{
	if( !GAMESTATE->m_bIsUsingStepTiming )
	{
		return GAMESTATE->m_Position.m_fSongBeat;
	}
	return GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat;
}

inline int ScreenEdit::GetRow()
{
	return BeatToNoteRow(GetBeat());
}

void ScreenEdit::DisplayTimingMenu()
{
	int row = GetRow();
	const TimingData &pTime = GetAppropriateTiming();
	bool bHasSpeedOnThisRow = pTime.GetSpeedSegmentAtRow( row )->GetRow() == row;
	// bool bIsSelecting = ( (m_NoteFieldEdit.m_iEndMarker != -1) && (m_NoteFieldEdit.m_iBeginMarker != -1) );


	g_TimingDataInformation.rows[beat_0_offset].SetOneUnthemedChoice( std::to_string(pTime.m_fBeat0OffsetInSeconds) );
	g_TimingDataInformation.rows[bpm].SetOneUnthemedChoice( std::to_string(pTime.GetBPMAtRow( row ) ) );
	g_TimingDataInformation.rows[stop].SetOneUnthemedChoice( std::to_string(pTime.GetStopAtRow( row ) ) ) ;
	g_TimingDataInformation.rows[delay].SetOneUnthemedChoice( std::to_string(pTime.GetDelayAtRow( row ) ) );

	g_TimingDataInformation.rows[label].SetOneUnthemedChoice( pTime.GetLabelAtRow( row ).c_str() );
	g_TimingDataInformation.rows[tickcount].SetOneUnthemedChoice( ssprintf("%d", pTime.GetTickcountAtRow( row ) ) );
	g_TimingDataInformation.rows[combo].SetOneUnthemedChoice( ssprintf("%d / %d",
																	   pTime.GetComboAtRow( row ),
																	   pTime.GetMissComboAtRow( row ) ) );
	g_TimingDataInformation.rows[warp].SetOneUnthemedChoice( std::to_string(pTime.GetWarpAtRow( row ) ) );
	g_TimingDataInformation.rows[speed_percent].SetOneUnthemedChoice( bHasSpeedOnThisRow ? std::to_string(pTime.GetSpeedPercentAtRow( row ) ) : "---" );
	g_TimingDataInformation.rows[speed_wait].SetOneUnthemedChoice( bHasSpeedOnThisRow ? std::to_string(pTime.GetSpeedWaitAtRow( row ) ) : "---" );

	RString starting = ( pTime.GetSpeedModeAtRow( row ) == 1 ? "Seconds" : "Beats" );
	g_TimingDataInformation.rows[speed_mode].SetOneUnthemedChoice( starting.c_str() );

	g_TimingDataInformation.rows[scroll].SetOneUnthemedChoice( std::to_string(pTime.GetScrollAtRow( row ) ) );
	g_TimingDataInformation.rows[fake].SetOneUnthemedChoice( std::to_string(pTime.GetFakeAtRow( row ) ) );

	// g_TimingDataInformation.rows[speed_percent].bEnabled = !bIsSelecting;
	g_TimingDataInformation.rows[speed_wait].bEnabled = bHasSpeedOnThisRow;
	g_TimingDataInformation.rows[speed_mode].bEnabled = bHasSpeedOnThisRow;

	EditMiniMenu( &g_TimingDataInformation, SM_BackFromTimingDataInformation );
}

void ScreenEdit::DisplayTimingChangeMenu()
{
	int row= GetRow();
	EditMiniMenu(&g_TimingDataChangeInformation, SM_BackFromTimingDataChangeInformation);
}

// End helper functions

static LocalizedString REVERT_LAST_SAVE			( "ScreenEdit", "Do you want to revert to your last save?" );
static LocalizedString DESTROY_ALL_UNSAVED_CHANGES	( "ScreenEdit", "This will destroy all unsaved changes." );
static LocalizedString REVERT_FROM_DISK			( "ScreenEdit", "Do you want to revert from disk?" );
static LocalizedString SAVE_CHANGES_BEFORE_EXITING	( "ScreenEdit", "Do you want to save changes before exiting?" );

int ScreenEdit::GetSongOrNotesEnd()
{
	return max(m_iStartPlayingAt, max(m_NoteDataEdit.GetLastRow(),
			BeatToNoteRow(m_pSteps->GetTimingData()->GetBeatFromElapsedTime(
					GAMESTATE->m_pCurSong->m_fMusicLengthSeconds))));
}

void ScreenEdit::HandleMainMenuChoice( MainMenuChoice c, const vector<int> &iAnswers )
{
	GAMESTATE->SetProcessedTimingData(m_pSteps->GetTimingData());
	switch( c )
	{
		DEFAULT_FAIL( c );
		case play_selection:
			if( m_NoteFieldEdit.m_iBeginMarker!=-1 && m_NoteFieldEdit.m_iEndMarker!=-1 )
				HandleAlterMenuChoice( play );
			else if( m_NoteFieldEdit.m_iBeginMarker!=-1 )
				HandleMainMenuChoice( play_selection_start_to_end );
			else
				HandleMainMenuChoice( play_current_beat_to_end );
			break;
		case play_whole_song:
			{
				m_iStartPlayingAt = 0;
				m_iStopPlayingAt= GetSongOrNotesEnd();
				TransitionEditState( STATE_PLAYING );
			}
			break;
		case play_selection_start_to_end:
			{
				m_iStartPlayingAt = m_NoteFieldEdit.m_iBeginMarker;
				m_iStopPlayingAt = max( m_iStartPlayingAt, m_NoteDataEdit.GetLastRow() );
				TransitionEditState( STATE_PLAYING );
			}
			break;
		case play_current_beat_to_end:
			{
				m_iStartPlayingAt = BeatToNoteRow(GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat);
				m_iStopPlayingAt= GetSongOrNotesEnd();
				TransitionEditState( STATE_PLAYING );
			}
			break;
		case set_selection_start:
			{
				const int iCurrentRow = BeatToNoteRow(GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat);
				if( m_NoteFieldEdit.m_iEndMarker!=-1 && iCurrentRow >= m_NoteFieldEdit.m_iEndMarker )
				{
					SCREENMAN->PlayInvalidSound();
				}
				else
				{
					m_NoteFieldEdit.m_iBeginMarker = iCurrentRow;
					m_soundMarker.Play(true);
				}
			}
			break;
		case set_selection_end:
			{
				const int iCurrentRow = BeatToNoteRow(GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat);
				if( m_NoteFieldEdit.m_iBeginMarker!=-1 && iCurrentRow <= m_NoteFieldEdit.m_iBeginMarker )
				{
					SCREENMAN->PlayInvalidSound();
				}
				else
				{
					m_NoteFieldEdit.m_iEndMarker = iCurrentRow;
					m_soundMarker.Play(true);
				}
			}
			break;
		case edit_steps_information:
			{
				/* XXX: If the difficulty is changed from EDIT, and pSteps->WasLoadedFromProfile()
				 * is true, we should warn that the steps will no longer be saved to the profile. */
				Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];

				g_StepsInformation.rows[difficulty].choices.clear();
				FOREACH_ENUM( Difficulty, dc )
				{
					g_StepsInformation.rows[difficulty].choices.push_back( "|" + CustomDifficultyToLocalizedString( GetCustomDifficulty(pSteps->m_StepsType, dc, CourseType_Invalid) ) );
				}
				g_StepsInformation.rows[difficulty].iDefaultChoice = pSteps->GetDifficulty();
				g_StepsInformation.rows[difficulty].bEnabled = (EDIT_MODE.GetValue() >= EditMode_Full);
				g_StepsInformation.rows[meter].SetOneUnthemedChoice( ssprintf("%d", pSteps->GetMeter()) );
				g_StepsInformation.rows[meter].bEnabled = (EDIT_MODE.GetValue() >= EditMode_Home);
				g_StepsInformation.rows[predict_meter].SetOneUnthemedChoice( ssprintf("%.2f",pSteps->PredictMeter()) );
				g_StepsInformation.rows[chartname].bEnabled = (EDIT_MODE.GetValue() >= EditMode_Full);
				g_StepsInformation.rows[chartname].SetOneUnthemedChoice(pSteps->GetChartName());
				g_StepsInformation.rows[description].bEnabled = (EDIT_MODE.GetValue() >= EditMode_Full);
				g_StepsInformation.rows[description].SetOneUnthemedChoice( pSteps->GetDescription() );
				g_StepsInformation.rows[chartstyle].bEnabled = (EDIT_MODE.GetValue() >= EditMode_Full);
				g_StepsInformation.rows[chartstyle].SetOneUnthemedChoice( pSteps->GetChartStyle() );
				g_StepsInformation.rows[step_credit].bEnabled = (EDIT_MODE.GetValue() >= EditMode_Full);
				g_StepsInformation.rows[step_credit].SetOneUnthemedChoice( pSteps->GetCredit() );
				g_StepsInformation.rows[step_display_bpm].iDefaultChoice = pSteps->GetDisplayBPM();
				g_StepsInformation.rows[step_min_bpm].SetOneUnthemedChoice( std::to_string(pSteps->GetMinBPM()));
				g_StepsInformation.rows[step_max_bpm].SetOneUnthemedChoice( std::to_string(pSteps->GetMaxBPM()));
				g_StepsInformation.rows[step_music].bEnabled = (EDIT_MODE.GetValue() >= EditMode_Full);
				g_StepsInformation.rows[step_music].SetOneUnthemedChoice( pSteps->GetMusicFile() );
				EditMiniMenu( &g_StepsInformation, SM_BackFromStepsInformation, SM_None );
			}
			break;
		case view_steps_data:
		{
			float fMusicSeconds = m_pSoundMusic->GetLengthSeconds();
			Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
			const StepsTypeCategory &cat = GAMEMAN->GetStepsTypeInfo(pSteps->m_StepsType).m_StepsTypeCategory;
			if (cat == StepsTypeCategory_Couple || cat == StepsTypeCategory_Routine)
			{
				pair<int, int> tmp = m_NoteDataEdit.GetNumTapNotesTwoPlayer();
				g_StepsData.rows[tap_notes].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumJumpsTwoPlayer();
				g_StepsData.rows[jumps].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumHandsTwoPlayer();
				g_StepsData.rows[hands].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumQuadsTwoPlayer();
				g_StepsData.rows[quads].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumHoldNotesTwoPlayer();
				g_StepsData.rows[holds].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumMinesTwoPlayer();
				g_StepsData.rows[mines].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumRollsTwoPlayer();
				g_StepsData.rows[rolls].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumLiftsTwoPlayer();
				g_StepsData.rows[lifts].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
				tmp = m_NoteDataEdit.GetNumFakesTwoPlayer();
				g_StepsData.rows[fakes].SetOneUnthemedChoice( ssprintf("%d / %d", tmp.first, tmp.second) );
			}
			else
			{
				g_StepsData.rows[tap_notes].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumTapNotes()) );
				g_StepsData.rows[jumps].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumJumps()) );
				g_StepsData.rows[hands].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumHands()) );
				g_StepsData.rows[quads].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumQuads()) );
				g_StepsData.rows[holds].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumHoldNotes()) );
				g_StepsData.rows[mines].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumMines()) );
				g_StepsData.rows[rolls].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumRolls()) );
				g_StepsData.rows[lifts].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumLifts()) );
				g_StepsData.rows[fakes].SetOneUnthemedChoice( ssprintf("%d", m_NoteDataEdit.GetNumFakes()) );
			}
			RadarValues radar;
			radar.Zero();
			NoteDataUtil::CalculateRadarValues(m_NoteDataEdit, fMusicSeconds, radar);
			g_StepsData.rows[stream].SetOneUnthemedChoice(ssprintf("%.2f", radar[RadarCategory_Stream]));
			g_StepsData.rows[voltage].SetOneUnthemedChoice(ssprintf("%.2f", radar[RadarCategory_Voltage]));
			g_StepsData.rows[air].SetOneUnthemedChoice(ssprintf("%.2f", radar[RadarCategory_Air]));
			g_StepsData.rows[freeze].SetOneUnthemedChoice(ssprintf("%.2f", radar[RadarCategory_Freeze]));
			g_StepsData.rows[chaos].SetOneUnthemedChoice(ssprintf("%.2f", radar[RadarCategory_Chaos]));
			EditMiniMenu( &g_StepsData, SM_BackFromStepsData, SM_None );
			break;
		}
		case save:
		case save_on_exit:
			m_CurrentAction = c;
			PerformSave(false);
			break;
		case revert_to_last_save:
			ScreenPrompt::Prompt( SM_DoRevertToLastSave, REVERT_LAST_SAVE.GetValue() + "\n\n" + DESTROY_ALL_UNSAVED_CHANGES.GetValue(), PROMPT_YES_NO, ANSWER_NO );
			break;
		case revert_from_disk:
			ScreenPrompt::Prompt( SM_DoRevertFromDisk, REVERT_FROM_DISK.GetValue() + "\n\n" + DESTROY_ALL_UNSAVED_CHANGES.GetValue(), PROMPT_YES_NO, ANSWER_NO );
			break;
		case options:
			SCREENMAN->AddNewScreenToTop( OPTIONS_SCREEN, SM_BackFromOptions );
			break;
		case edit_song_info:
			{
				const Song* pSong = GAMESTATE->m_pCurSong;
				g_SongInformation.rows[main_title].SetOneUnthemedChoice( pSong->m_sMainTitle );
				g_SongInformation.rows[sub_title].SetOneUnthemedChoice( pSong->m_sSubTitle );
				g_SongInformation.rows[artist].SetOneUnthemedChoice( pSong->m_sArtist );
				g_SongInformation.rows[genre].SetOneUnthemedChoice( pSong->m_sGenre );
				g_SongInformation.rows[credit].SetOneUnthemedChoice( pSong->m_sCredit );
				g_SongInformation.rows[preview].SetOneUnthemedChoice(pSong->m_PreviewFile);
				g_SongInformation.rows[main_title_transliteration].SetOneUnthemedChoice( pSong->m_sMainTitleTranslit );
				g_SongInformation.rows[sub_title_transliteration].SetOneUnthemedChoice( pSong->m_sSubTitleTranslit );
				g_SongInformation.rows[artist_transliteration].SetOneUnthemedChoice( pSong->m_sArtistTranslit );
				g_SongInformation.rows[last_second_hint].SetOneUnthemedChoice( std::to_string(pSong->GetSpecifiedLastSecond()) );
				g_SongInformation.rows[preview_start].SetOneUnthemedChoice( std::to_string(pSong->m_fMusicSampleStartSeconds) );
				g_SongInformation.rows[preview_length].SetOneUnthemedChoice( std::to_string(pSong->m_fMusicSampleLengthSeconds) );
				g_SongInformation.rows[display_bpm].iDefaultChoice = pSong->m_DisplayBPMType;
				g_SongInformation.rows[min_bpm].SetOneUnthemedChoice( std::to_string(pSong->m_fSpecifiedBPMMin) );
				g_SongInformation.rows[max_bpm].SetOneUnthemedChoice( std::to_string(pSong->m_fSpecifiedBPMMax) );

				EditMiniMenu( &g_SongInformation, SM_BackFromSongInformation );
			}
			break;
		case edit_timing_data:
			{
				DisplayTimingMenu();
			}
			break;

		case play_preview_music:
			PlayPreviewMusic();
			break;
		case exit:
			switch( EDIT_MODE.GetValue() )
			{
			DEFAULT_FAIL( EDIT_MODE.GetValue() );
			case EditMode_Full:
			case EditMode_Home:
				if( IsDirty() )
					ScreenPrompt::Prompt( SM_DoSaveAndExit, SAVE_CHANGES_BEFORE_EXITING, PROMPT_YES_NO_CANCEL, ANSWER_CANCEL );
				else
					SCREENMAN->SendMessageToTopScreen( SM_DoExit );
				break;
			case EditMode_Practice:
			case EditMode_CourseMods:
				SCREENMAN->SendMessageToTopScreen( SM_DoExit );
				break;
			}
			break;
	};
	GAMESTATE->SetProcessedTimingData(nullptr);
}

static LocalizedString ENTER_ARBITRARY_MAPPING( "ScreenEdit", "Enter the new track mapping." );
static LocalizedString TOO_MANY_TRACKS("ScreenEdit", "Too many tracks specified.");
static LocalizedString NOT_A_TRACK("ScreenEdit", "'%s' is not a track id.");
static LocalizedString OUT_OF_RANGE_ID("ScreenEdit", "Entry %d, '%d', is out of range 1 to %d.");
static LocalizedString CONFIRM_CLEAR("ScreenEdit", "Are you sure you want to clear %d notes?");

static bool ConvertMappingInputToMapping(RString const& mapstr, int* mapping, RString& error)
{
	vector<RString> mapping_input;
	split(mapstr, ",", mapping_input);
	size_t tracks_for_type= GAMEMAN->GetStepsTypeInfo(GAMESTATE->m_pCurSteps[0]->m_StepsType).iNumTracks;
	if(mapping_input.size() > tracks_for_type)
	{
		error= TOO_MANY_TRACKS;
		return false;
	}
	// mapping_input.size() < tracks_for_type is not checked because
	// unspecified tracks are mapped directly. -Kyz
	size_t track= 0;
	// track will be used for filling in the unspecified part of the mapping.
	for(; track < mapping_input.size(); ++track)
	{
		if(mapping_input[track].empty() || mapping_input[track] == " ")
		{
			// This allows blank entries to mean "pass through".
			mapping[track]= track+1;
		}
		else if(!(mapping_input[track] >> mapping[track]))
		{
			error= ssprintf(NOT_A_TRACK.GetValue(), mapping_input[track].c_str());
			return false;
		}
		if(mapping[track] < 1 || mapping[track] > static_cast<int>(tracks_for_type))
		{
			error= ssprintf(OUT_OF_RANGE_ID.GetValue(), track+1, mapping[track], tracks_for_type);
			return false;
		}
		// Simpler for the user if they input track ids starting at 1.
		--mapping[track];
	}
	for(; track < tracks_for_type; ++track)
	{
		mapping[track]= track;
	}
	return true;
}

static bool ArbitraryRemapValidate(const RString& answer, RString& error_out)
{
	int mapping[MAX_NOTE_TRACKS];
	return ConvertMappingInputToMapping(answer, mapping, error_out);
}

void ScreenEdit::HandleArbitraryRemapping(RString const& mapstr)
{
	const NoteData OldClipboard( m_Clipboard );
	HandleAlterMenuChoice( cut, false );
	int mapping[MAX_NOTE_TRACKS];
	RString error;
	// error is actually reported by the validate function, and unused here.
	if(ConvertMappingInputToMapping(mapstr, mapping, error))
	{
		NoteDataUtil::ArbitraryRemap(m_Clipboard, mapping);
	}
	HandleAreaMenuChoice( paste_at_begin_marker, false );
	m_Clipboard = OldClipboard;
}

void ScreenEdit::HandleAlterMenuChoice(AlterMenuChoice c, const vector<int> &answers, bool allow_undo, bool prompt_clear)
{
	ASSERT_M(m_NoteFieldEdit.m_iBeginMarker!=-1 && m_NoteFieldEdit.m_iEndMarker!=-1,
			 "You can only alter a selection of notes with a selection to begin with!");

	bool bSaveUndo = true;
	switch (c)
	{
		case play:
		case record:
		case cut:
		case copy:
		{
			bSaveUndo = false;
		}
		default:
			break;
	}

	if( bSaveUndo )
		SetDirty( true );

	/* We call HandleAreaMenuChoice recursively. Only the outermost
	 * HandleAreaMenuChoice should allow Undo so that the inner calls don't
	 * also save Undo and mess up the outermost */
	if(!allow_undo)
		bSaveUndo = false;

	if( bSaveUndo )
		SaveUndo();

	switch(c)
	{
		case cut:
		{
			HandleAlterMenuChoice(copy, false, false);
			HandleAlterMenuChoice(clear, false, false);
		}
			break;
		case copy:
		{

			m_Clipboard.ClearAll();
			m_Clipboard.CopyRange( m_NoteDataEdit, m_NoteFieldEdit.m_iBeginMarker, m_NoteFieldEdit.m_iEndMarker );
		}
			break;
		case clear:
		{
			int note_count= m_NoteDataEdit.GetNumTapNotesNoTiming(
				m_NoteFieldEdit.m_iBeginMarker, m_NoteFieldEdit.m_iEndMarker);
			if(note_count >= PREFSMAN->m_EditClearPromptThreshold && prompt_clear)
			{
				ScreenPrompt::Prompt(SM_ConfirmClearArea, ssprintf(CONFIRM_CLEAR.GetValue(), note_count), PROMPT_YES_NO);
			}
			else
			{
				m_NoteDataEdit.ClearRange(
					m_NoteFieldEdit.m_iBeginMarker, m_NoteFieldEdit.m_iEndMarker);
			}
		}
			break;
		case quantize:
		{
			NoteType nt = (NoteType)answers[c];
			NoteDataUtil::SnapToNearestNoteType(m_NoteDataEdit, nt, nt,
							    m_NoteFieldEdit.m_iBeginMarker,
							    m_NoteFieldEdit.m_iEndMarker );
			break;
		}
		case turn:
		{
			const NoteData OldClipboard( m_Clipboard );
			HandleAlterMenuChoice( cut, false );

			StepsType st = GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StepsType;
			TurnType tt = (TurnType)answers[c];
			switch( tt )
			{
				DEFAULT_FAIL( tt );
				case left:		NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::left );		break;
				case right:		NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::right );		break;
				case mirror:		NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::mirror );		break;
				case turn_backwards:		NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::backwards );		break;
				case shuffle:		NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::shuffle );		break;
				case super_shuffle:	NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::super_shuffle );	break;
			}

			HandleAreaMenuChoice( paste_at_begin_marker, false );
			m_Clipboard = OldClipboard;
		}
			break;
		case transform:
		{
			int iBeginRow = m_NoteFieldEdit.m_iBeginMarker;
			int iEndRow = m_NoteFieldEdit.m_iEndMarker;
			TransformType tt = (TransformType)answers[c];
			StepsType st = GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StepsType;

			switch( tt )
			{
					DEFAULT_FAIL( tt );
				case noholds:		NoteDataUtil::RemoveHoldNotes( m_NoteDataEdit, iBeginRow, iEndRow );	break;
				case nomines:		NoteDataUtil::RemoveMines( m_NoteDataEdit, iBeginRow, iEndRow );	break;
				case little:		NoteDataUtil::Little( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case wide:		NoteDataUtil::Wide( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case big:		NoteDataUtil::Big( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case quick:		NoteDataUtil::Quick( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case skippy:		NoteDataUtil::Skippy( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case add_mines:		NoteDataUtil::AddMines( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case echo:		NoteDataUtil::Echo( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case stomp:		NoteDataUtil::Stomp( m_NoteDataEdit, st, iBeginRow, iEndRow );		break;
				case planted:		NoteDataUtil::Planted( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case floored:		NoteDataUtil::Floored( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case twister:		NoteDataUtil::Twister( m_NoteDataEdit, iBeginRow, iEndRow );		break;
				case nojumps:		NoteDataUtil::RemoveJumps( m_NoteDataEdit, iBeginRow, iEndRow );	break;
				case nohands:		NoteDataUtil::RemoveHands( m_NoteDataEdit, iBeginRow, iEndRow );	break;
				case noquads:		NoteDataUtil::RemoveQuads( m_NoteDataEdit, iBeginRow, iEndRow );	break;
				case nostretch:		NoteDataUtil::RemoveStretch( m_NoteDataEdit, st, iBeginRow, iBeginRow );break;
			}

			// bake in the additions
			NoteDataUtil::ConvertAdditionsToRegular( m_NoteDataEdit );
			break;
		}
		case alter:
		{
			const NoteData OldClipboard( m_Clipboard );
			HandleAlterMenuChoice( cut, false );

			AlterType at = (AlterType)answers[c];
			switch( at )
			{
				DEFAULT_FAIL( at );
				case autogen_to_fill_width:
				{
					NoteData temp( m_Clipboard );
					int iMaxNonEmptyTrack = NoteDataUtil::GetMaxNonEmptyTrack( temp );
					if( iMaxNonEmptyTrack == -1 )
						break;
					temp.SetNumTracks( iMaxNonEmptyTrack+1 );
					NoteDataUtil::LoadTransformedSlidingWindow( temp, m_Clipboard, m_Clipboard.GetNumTracks() );
					NoteDataUtil::RemoveStretch( m_Clipboard, GAMESTATE->m_pCurSteps[0]->m_StepsType );
				}
					break;
				case backwards:			NoteDataUtil::Backwards( m_Clipboard );			break;
				case swap_sides:		NoteDataUtil::SwapSides( m_Clipboard );			break;
				case copy_left_to_right:	NoteDataUtil::CopyLeftToRight( m_Clipboard );		break;
				case copy_right_to_left:	NoteDataUtil::CopyRightToLeft( m_Clipboard );		break;
				case clear_left:		NoteDataUtil::ClearLeft( m_Clipboard );			break;
				case clear_right:		NoteDataUtil::ClearRight( m_Clipboard );		break;
				case collapse_to_one:		NoteDataUtil::CollapseToOne( m_Clipboard );		break;
				case collapse_left:		NoteDataUtil::CollapseLeft( m_Clipboard );		break;
				case shift_left:		NoteDataUtil::ShiftLeft( m_Clipboard );			break;
				case shift_right:		NoteDataUtil::ShiftRight( m_Clipboard );		break;
				case swap_up_down: NoteDataUtil::SwapUpDown(m_Clipboard, GAMESTATE->m_pCurSteps[0]->m_StepsType); break;
				case arbitrary_remap:
					ScreenTextEntry::TextEntry(
						SM_BackFromArbitraryRemap, ENTER_ARBITRARY_MAPPING,
						"1, 2, 3, 4", MAX_NOTE_TRACKS * 4,
						// 2 chars for digit, one for comma, one for space.
						ArbitraryRemapValidate
				);
					break;
			}

			HandleAreaMenuChoice( paste_at_begin_marker, false );
			m_Clipboard = OldClipboard;
			break;
		}
		case tempo:
		{
			// This affects all steps.
			AlterType at = (AlterType)answers[c];
			float fScale = -1;

			switch( at )
			{
					DEFAULT_FAIL( at );
				case compress_2x:	fScale = 0.5f;		break;
				case compress_3_2:	fScale = 2.0f/3;	break;
				case compress_4_3:	fScale = 0.75f;		break;
				case expand_4_3:	fScale = 4.0f/3;	break;
				case expand_3_2:	fScale = 1.5f;		break;
				case expand_2x:		fScale = 2;		break;
			}

			int iStartIndex  = m_NoteFieldEdit.m_iBeginMarker;
			int iEndIndex    = m_NoteFieldEdit.m_iEndMarker;
			int iNewEndIndex = iEndIndex + lrintf( (iEndIndex - iStartIndex) * (fScale - 1) );

			// scale currently editing notes
			NoteDataUtil::ScaleRegion( m_NoteDataEdit, fScale, iStartIndex, iEndIndex );

			// scale timing data
			GetAppropriateTimingForUpdate().ScaleRegion(fScale,
							   m_NoteFieldEdit.m_iBeginMarker,
							   m_NoteFieldEdit.m_iEndMarker, true );

			m_NoteFieldEdit.m_iEndMarker = iNewEndIndex;
			break;

		}

		case play:
			m_iStartPlayingAt = m_NoteFieldEdit.m_iBeginMarker;
			m_iStopPlayingAt = m_NoteFieldEdit.m_iEndMarker;
			TransitionEditState( STATE_PLAYING );
			break;
		case record:
			m_iStartPlayingAt = m_NoteFieldEdit.m_iBeginMarker;
			m_iStopPlayingAt = m_NoteFieldEdit.m_iEndMarker;
			TransitionEditState( STATE_RECORDING );
			break;
		case preview_designation:
		{
			float fMarkerStart = GetAppropriateTiming().GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker) );
			float fMarkerEnd = GetAppropriateTiming().GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker) );
			GAMESTATE->m_pCurSong->m_fMusicSampleStartSeconds = fMarkerStart;
			GAMESTATE->m_pCurSong->m_fMusicSampleLengthSeconds = fMarkerEnd - fMarkerStart;
			break;
		}
		case convert_to_pause:
		{
			float fMarkerStart = GetAppropriateTiming().GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker) );
			float fMarkerEnd = GetAppropriateTiming().GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker) );

			// The length of the stop segment we're going to create.  This includes time spent in any
			// stops in the selection, which will be deleted and subsumed into the new stop.
			float fStopLength = fMarkerEnd - fMarkerStart;

			// be sure not to clobber the row at the start - a row at the end
			// can be dropped safely, though
			NoteDataUtil::DeleteRows( m_NoteDataEdit,
						 m_NoteFieldEdit.m_iBeginMarker + 1,
						 m_NoteFieldEdit.m_iEndMarker-m_NoteFieldEdit.m_iBeginMarker
						 );
			// For TimingData, it makes more sense not to offset by a row
			GetAppropriateTimingForUpdate().DeleteRows( m_NoteFieldEdit.m_iBeginMarker,
							  m_NoteFieldEdit.m_iEndMarker-m_NoteFieldEdit.m_iBeginMarker );
			GetAppropriateTimingForUpdate().SetStopAtRow( m_NoteFieldEdit.m_iBeginMarker, fStopLength );
			m_NoteFieldEdit.m_iBeginMarker = -1;
			m_NoteFieldEdit.m_iEndMarker = -1;
			break;
		}
		case convert_to_delay:
		{
			float fMarkerStart = GetAppropriateTiming().GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker) );
			float fMarkerEnd = GetAppropriateTiming().GetElapsedTimeFromBeat( NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker) );

			// The length of the delay segment we're going to create.  This includes time spent in any
			// stops in the selection, which will be deleted and subsumed into the new stop.
			float fStopLength = fMarkerEnd - fMarkerStart;

			NoteDataUtil::DeleteRows( m_NoteDataEdit,
						 m_NoteFieldEdit.m_iBeginMarker,
						 m_NoteFieldEdit.m_iEndMarker-m_NoteFieldEdit.m_iBeginMarker
						 );
			GetAppropriateTimingForUpdate().DeleteRows( m_NoteFieldEdit.m_iBeginMarker,
							  m_NoteFieldEdit.m_iEndMarker-m_NoteFieldEdit.m_iBeginMarker );
			GetAppropriateTimingForUpdate().SetDelayAtRow( m_NoteFieldEdit.m_iBeginMarker, fStopLength );
			m_NoteFieldEdit.m_iBeginMarker = -1;
			m_NoteFieldEdit.m_iEndMarker = -1;
			break;
		}
		case convert_to_warp:
		{
			float startBeat = NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker);
			float lengthBeat = NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker) - startBeat;
			GetAppropriateTimingForUpdate().SetWarpAtBeat(startBeat,lengthBeat);
			SetDirty(true);
			break;
		}
		case convert_to_attack:
		{
			float startBeat = NoteRowToBeat(m_NoteFieldEdit.m_iBeginMarker);
			float endBeat = NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker);
			const TimingData &timing = GetAppropriateTiming();
			float &start = g_fLastInsertAttackPositionSeconds;
			float &length = g_fLastInsertAttackDurationSeconds;
			start = timing.GetElapsedTimeFromBeat(startBeat);
			length = timing.GetElapsedTimeFromBeat(endBeat) - start;

			AttackArray &attacks = GAMESTATE->m_bIsUsingStepTiming ?
				m_pSteps->m_Attacks : m_pSong->m_Attacks;
			int iAttack = FindAttackAtTime(attacks, start);

			ModsGroup<PlayerOptions> &toEdit = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions;
			this->originalPlayerOptions.Assign(ModsLevel_Preferred, toEdit.GetPreferred());
			PlayerOptions po;
			if (iAttack >= 0)
				po.FromString(attacks[iAttack].sModifiers);

			toEdit.Assign( ModsLevel_Preferred, po );
			SCREENMAN->AddNewScreenToTop( SET_MOD_SCREEN, SM_BackFromInsertStepAttackPlayerOptions );
			SetDirty(true);
			break;
		}
		case convert_to_fake:
		{
			int startRow = m_NoteFieldEdit.m_iBeginMarker;
			float lengthBeat = NoteRowToBeat(m_NoteFieldEdit.m_iEndMarker) - NoteRowToBeat(startRow);
			GetAppropriateTimingForUpdate().AddSegment( FakeSegment(startRow,lengthBeat) );
			SetDirty(true);
			break;
		}
		case routine_invert_notes:
		{
			NoteData &nd = this->m_NoteDataEdit;
			NoteField &nf = this->m_NoteFieldEdit;
			FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE(nd, r,
							      nf.m_iBeginMarker,
							      nf.m_iEndMarker)
			{
				for (int t = 0; t < nd.GetNumTracks(); t++)
				{
					const TapNote &tn = nd.GetTapNote(t, r);
					if (tn.type != TapNoteType_Empty)
					{
						TapNote nTap = tn;
						nTap.pn = (tn.pn == PLAYER_1 ?
							   PLAYER_2 : PLAYER_1);
						m_NoteDataEdit.SetTapNote(t, r, nTap);
					}
				}
			}
			break;
		}
		case routine_mirror_1_to_2:
		case routine_mirror_2_to_1:
		{
			PlayerNumber oPN = (c == routine_mirror_1_to_2 ?
					    PLAYER_1 : PLAYER_2);
			PlayerNumber nPN = (c == routine_mirror_1_to_2 ?
					    PLAYER_2 : PLAYER_1);
			int nTrack = -1;
			NoteData &nd = this->m_NoteDataEdit;
			NoteField &nf = this->m_NoteFieldEdit;
			int tracks = nd.GetNumTracks();
			FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE(nd, r,
							      nf.m_iBeginMarker,
							      nf.m_iEndMarker)
			{
				for (int t = 0; t < tracks; t++)
				{
					const TapNote &tn = nd.GetTapNote(t, r);
					if (tn.type != TapNoteType_Empty && tn.pn == oPN)
					{
						TapNote nTap = tn;
						nTap.pn = nPN;
						StepsType curType = GAMESTATE->m_pCurSteps[PLAYER_1]->m_StepsType;
						// TODO: Find a better way to do this.
						if (curType == StepsType_dance_routine)
						{
							nTrack = tracks - t - 1;
						}
						else if (curType == StepsType_pump_routine)
						{
							switch (t)
							{
								case 0: nTrack = 8; break;
								case 1: nTrack = 9; break;
								case 2: nTrack = 7; break;
								case 3: nTrack = 5; break;
								case 4: nTrack = 6; break;
								case 5: nTrack = 3; break;
								case 6: nTrack = 4; break;
								case 7: nTrack = 2; break;
								case 8: nTrack = 0; break;
								case 9: nTrack = 1; break;
								default: FAIL_M(ssprintf("Invalid column %d for pump-routine", t)); break;
							}
						}
						m_NoteDataEdit.SetTapNote(nTrack, r, nTap);
					}
				}
			}
			break;
		}
		default: break;
	}

}

void ScreenEdit::HandleAreaMenuChoice( AreaMenuChoice c, const vector<int> &iAnswers, bool bAllowUndo )
{
	bool bSaveUndo = true;
	switch( c )
	{
		case clear_clipboard:
		case undo:
			bSaveUndo = false;
			break;
		default:
			break;
	}

	if( bSaveUndo )
		SetDirty( true );

	/* We call HandleAreaMenuChoice recursively. Only the outermost
	 * HandleAreaMenuChoice should allow Undo so that the inner calls don't
	 * also save Undo and mess up the outermost */
	if( !bAllowUndo )
		bSaveUndo = false;

	if( bSaveUndo )
		SaveUndo();

	switch( c )
	{
		DEFAULT_FAIL( c );

		case paste_at_current_beat:
		case paste_at_begin_marker:
			{
				int iDestFirstRow = -1;
				switch( c )
				{
					DEFAULT_FAIL( c );
					case paste_at_current_beat:
						iDestFirstRow = BeatToNoteRow( GetAppropriatePosition().m_fSongBeat );
						break;
					case paste_at_begin_marker:
						ASSERT( m_NoteFieldEdit.m_iBeginMarker!=-1 );
						iDestFirstRow = m_NoteFieldEdit.m_iBeginMarker;
						break;
				}

				int iRowsToCopy = m_Clipboard.GetLastRow()+1;
				m_NoteDataEdit.CopyRange( m_Clipboard, 0, iRowsToCopy, iDestFirstRow );
			}
			break;

		case insert_and_shift:
			NoteDataUtil::InsertRows( m_NoteDataEdit, GetRow(),
				GetRowsFromAnswers(c, iAnswers));
			break;
		case delete_and_shift:
			NoteDataUtil::DeleteRows( m_NoteDataEdit, GetRow(),
				GetRowsFromAnswers(c, iAnswers));
			break;
		case shift_pauses_forward:
			GetAppropriateTimingForUpdate().InsertRows( GetRow(),
				GetRowsFromAnswers(c, iAnswers));
			break;
		case shift_pauses_backward:
			GetAppropriateTimingForUpdate().DeleteRows( GetRow(),
				GetRowsFromAnswers(c, iAnswers));
			break;

		case convert_pause_to_beat:
		{
			float fStopSeconds = GetAppropriateTiming().GetStopAtRow(GetRow());
			GetAppropriateTimingForUpdate().SetStopAtBeat( GetBeat() , 0 );

			float fStopBeats = fStopSeconds * GetAppropriateTiming().GetBPMAtBeat( GetBeat() ) / 60;

			// don't move the step from where it is, just move everything later
			NoteDataUtil::InsertRows( m_NoteDataEdit, GetRow() + 1, BeatToNoteRow(fStopBeats) );
			GetAppropriateTimingForUpdate().InsertRows( GetRow() + 1, BeatToNoteRow(fStopBeats) );
		}
		break;
		case convert_delay_to_beat:
		{
			TimingData &timing = GetAppropriateTimingForUpdate();
			float pause = timing.GetDelayAtRow(GetRow());
			timing.SetDelayAtRow(GetRow(), 0);

			float pauseBeats = pause * timing.GetBPMAtBeat(GetBeat()) / 60;

			NoteDataUtil::InsertRows(m_NoteDataEdit, GetRow(), BeatToNoteRow(pauseBeats));
			timing.InsertRows(GetRow(), BeatToNoteRow(pauseBeats));
			break;
		}
		case last_second_at_beat:
		{
			const TimingData &timing = GetAppropriateTiming();
			Song &s = *GAMESTATE->m_pCurSong;
			s.SetSpecifiedLastSecond(timing.GetElapsedTimeFromBeat(GetBeat()));
			break;
		}
		case undo:
			Undo();
			break;
		case clear_clipboard:
		{
			m_Clipboard.ClearAll();
			break;
		}
		case modify_attacks_at_row:
		{
			this->DoStepAttackMenu();
			break;
		}
		case modify_keysounds_at_row:
		{
			this->DoKeyboardTrackMenu();
			break;
		}
	};

	if( bSaveUndo )
		CheckNumberOfNotesAndUndo();
}

void ScreenEdit::HandleStepsDataChoice( StepsDataChoice c, const vector<int> &iAnswers )
{
	return; // nothing is done with the choices. Yet.
}

static LocalizedString ENTER_NEW_DESCRIPTION( "ScreenEdit", "Enter a new description." );
static LocalizedString ENTER_NEW_CHART_NAME("ScreenEdit", "Enter a new chart name.");
static LocalizedString ENTER_NEW_CHART_STYLE( "ScreenEdit", "Enter a new chart style." );
static LocalizedString ENTER_NEW_STEP_AUTHOR( "ScreenEdit", "Enter the author who made this step pattern." );
static LocalizedString ENTER_NEW_METER( "ScreenEdit", "Enter a new meter." );
static LocalizedString ENTER_MIN_BPM			("ScreenEdit","Enter a new min BPM.");
static LocalizedString ENTER_MAX_BPM			("ScreenEdit","Enter a new max BPM.");
static LocalizedString ENTER_NEW_STEP_MUSIC("ScreenEdit", "Enter the music file for this chart.");
void ScreenEdit::HandleStepsInformationChoice( StepsInformationChoice c, const vector<int> &iAnswers )
{
	Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	Difficulty dc = (Difficulty)iAnswers[difficulty];
	pSteps->SetDifficulty( dc );
	pSteps->SetDisplayBPM(static_cast<DisplayBPM>(iAnswers[step_display_bpm]));

	switch( c )
	{
		case chartname:
		{
			ScreenTextEntry::TextEntry(SM_None,
									   ENTER_NEW_CHART_NAME,
									   m_pSteps->GetChartName(),
									   MAX_STEPS_DESCRIPTION_LENGTH,
									   SongUtil::ValidateCurrentStepsChartName,
									   ChangeChartName,
									   nullptr);
			break;
		}
		case description:
		{
			ScreenTextEntry::TextEntry(SM_None,
									   ENTER_NEW_DESCRIPTION,
									   m_pSteps->GetDescription(),
									   MAX_STEPS_DESCRIPTION_LENGTH,
									   SongUtil::ValidateCurrentStepsDescription,
									   ChangeDescription,
									   nullptr);
			break;
		}
		case chartstyle:
		{
			ScreenTextEntry::TextEntry(SM_None,
									   ENTER_NEW_CHART_STYLE,
									   m_pSteps->GetChartStyle(),
									   255,
									   nullptr,
									   ChangeChartStyle,
									   nullptr);
			break;
		}
		case step_credit:
		{
			ScreenTextEntry::TextEntry(SM_None,
									   ENTER_NEW_STEP_AUTHOR,
									   m_pSteps->GetCredit(),
									   255,
									   SongUtil::ValidateCurrentStepsCredit,
									   ChangeStepCredit,
									   nullptr);
			break;
		}
		case meter:
		{
			ScreenTextEntry::TextEntry(SM_BackFromDifficultyMeterChange,
									   ENTER_NEW_METER,
									   ssprintf("%d", m_pSteps->GetMeter()),
									   4,
									   ScreenTextEntry::IntValidate,
									   ChangeStepMeter,
									   nullptr);
			break;
		}
		case step_min_bpm:
		{
			ScreenTextEntry::TextEntry(SM_None, ENTER_MIN_BPM,
									   std::to_string(pSteps->GetMinBPM()), 20,
									   ScreenTextEntry::FloatValidate,
									   ChangeStepsMinBPM, nullptr);
			break;
		}
		case step_max_bpm:
		{
			ScreenTextEntry::TextEntry(SM_None, ENTER_MAX_BPM,
									   std::to_string(pSteps->GetMaxBPM()), 20,
									   ScreenTextEntry::FloatValidate,
									   ChangeStepsMaxBPM, nullptr);
			break;
		}
		case step_music:
		{
			ScreenTextEntry::TextEntry(SM_BackFromStepMusicChange,
									   ENTER_NEW_STEP_MUSIC,
									   m_pSteps->GetMusicFile(),
									   255,
									   SongUtil::ValidateCurrentStepsMusic,
									   ChangeStepMusic,
									   nullptr);
			break;
		}
	default:
		break;
	}
	SetDirty(true);
}

static LocalizedString ENTER_MAIN_TITLE			("ScreenEdit","Enter a new main title.");
static LocalizedString ENTER_SUB_TITLE			("ScreenEdit","Enter a new sub title.");
static LocalizedString ENTER_ARTIST			("ScreenEdit","Enter a new artist.");
static LocalizedString ENTER_GENRE			("ScreenEdit","Enter a new genre.");
static LocalizedString ENTER_CREDIT			("ScreenEdit","Enter a new credit.");
static LocalizedString ENTER_PREVIEW			("ScreenEdit","Enter a preview file.");
static LocalizedString ENTER_MAIN_TITLE_TRANSLIT	("ScreenEdit","Enter a new main title transliteration.");
static LocalizedString ENTER_SUB_TITLE_TRANSLIT		("ScreenEdit","Enter a new sub title transliteration.");
static LocalizedString ENTER_ARTIST_TRANSLIT		("ScreenEdit","Enter a new artist transliteration.");
static LocalizedString ENTER_LAST_SECOND_HINT		("ScreenEdit","Enter a new last second hint.");
static LocalizedString ENTER_PREVIEW_START		("ScreenEdit","Enter a new preview start.");
static LocalizedString ENTER_PREVIEW_LENGTH		("ScreenEdit","Enter a new preview length.");
void ScreenEdit::HandleSongInformationChoice( SongInformationChoice c, const vector<int> &iAnswers )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_DisplayBPMType = static_cast<DisplayBPM>(iAnswers[display_bpm]);

	switch( c )
	{
	case main_title:
		ScreenTextEntry::TextEntry( SM_None, ENTER_MAIN_TITLE, pSong->m_sMainTitle, 100, nullptr, ChangeMainTitle, nullptr );
		break;
	case sub_title:
		ScreenTextEntry::TextEntry( SM_None, ENTER_SUB_TITLE, pSong->m_sSubTitle, 100, nullptr, ChangeSubTitle, nullptr );
		break;
	case artist:
		ScreenTextEntry::TextEntry( SM_None, ENTER_ARTIST, pSong->m_sArtist, 100, nullptr, ChangeArtist, nullptr );
		break;
	case genre:
		ScreenTextEntry::TextEntry( SM_None, ENTER_GENRE, pSong->m_sGenre, 100, nullptr, ChangeGenre, nullptr );
		break;
	case credit:
		ScreenTextEntry::TextEntry( SM_None, ENTER_CREDIT, pSong->m_sCredit, 100, nullptr, ChangeCredit, nullptr );
		break;
	case preview:
		ScreenTextEntry::TextEntry(SM_None, ENTER_PREVIEW, pSong->m_PreviewFile, 100, SongUtil::ValidateCurrentSongPreview, ChangePreview, nullptr);
		break;
	case main_title_transliteration:
		ScreenTextEntry::TextEntry( SM_None, ENTER_MAIN_TITLE_TRANSLIT, pSong->m_sMainTitleTranslit, 100, nullptr, ChangeMainTitleTranslit, nullptr );
		break;
	case sub_title_transliteration:
		ScreenTextEntry::TextEntry( SM_None, ENTER_SUB_TITLE_TRANSLIT, pSong->m_sSubTitleTranslit, 100, nullptr, ChangeSubTitleTranslit, nullptr );
		break;
	case artist_transliteration:
		ScreenTextEntry::TextEntry( SM_None, ENTER_ARTIST_TRANSLIT, pSong->m_sArtistTranslit, 100, nullptr, ChangeArtistTranslit, nullptr );
		break;
	case last_second_hint:
		ScreenTextEntry::TextEntry( SM_None, ENTER_LAST_SECOND_HINT,
					   std::to_string(pSong->GetSpecifiedLastSecond()), 20,
					   ScreenTextEntry::FloatValidate, ChangeLastSecondHint, nullptr );
		break;
	case preview_start:
		ScreenTextEntry::TextEntry( SM_None, ENTER_PREVIEW_START,
					   std::to_string(pSong->m_fMusicSampleStartSeconds), 20,
					   ScreenTextEntry::FloatValidate, ChangePreviewStart, nullptr );
		break;
	case preview_length:
		ScreenTextEntry::TextEntry( SM_None, ENTER_PREVIEW_LENGTH,
					   std::to_string(pSong->m_fMusicSampleLengthSeconds), 20,
					   ScreenTextEntry::FloatValidate, ChangePreviewLength, nullptr );
		break;
	case min_bpm:
		ScreenTextEntry::TextEntry( SM_None, ENTER_MIN_BPM,
					   std::to_string(pSong->m_fSpecifiedBPMMin), 20,
					   ScreenTextEntry::FloatValidate, ChangeMinBPM, nullptr );
		break;
	case max_bpm:
		ScreenTextEntry::TextEntry( SM_None, ENTER_MAX_BPM,
					   std::to_string(pSong->m_fSpecifiedBPMMax), 20,
					   ScreenTextEntry::FloatValidate, ChangeMaxBPM, nullptr );
		break;
	default: break;
	};
	SetDirty(true);
}


static LocalizedString ENTER_BEAT_0_OFFSET			( "ScreenEdit", "Enter the offset for the song.");
static LocalizedString ENTER_BPM_VALUE				( "ScreenEdit", "Enter a new BPM value." );
static LocalizedString ENTER_STOP_VALUE				( "ScreenEdit", "Enter a new Stop value." );
static LocalizedString ENTER_DELAY_VALUE			( "ScreenEdit", "Enter a new Delay value." );
static LocalizedString ENTER_TICKCOUNT_VALUE			( "ScreenEdit", "Enter a new Tickcount value." );
static LocalizedString ENTER_COMBO_VALUE			( "ScreenEdit", "Enter a new Combo value." );
static LocalizedString ENTER_LABEL_VALUE			( "ScreenEdit", "Enter a new Label value." );
static LocalizedString ENTER_WARP_VALUE				( "ScreenEdit", "Enter a new Warp value." );
static LocalizedString ENTER_SPEED_PERCENT_VALUE		( "ScreenEdit", "Enter a new Speed percent value." );
static LocalizedString ENTER_SPEED_WAIT_VALUE			( "ScreenEdit", "Enter a new Speed wait value." );
static LocalizedString ENTER_SPEED_MODE_VALUE			( "ScreenEdit", "Enter a new Speed mode value." );
static LocalizedString ENTER_SCROLL_VALUE			( "ScreenEdit", "Enter a new Scroll value." );
static LocalizedString ENTER_FAKE_VALUE				( "ScreenEdit", "Enter a new Fake value." );
static LocalizedString CONFIRM_TIMING_ERASE			( "ScreenEdit", "Are you sure you want to erase this chart's timing data?" );
void ScreenEdit::HandleTimingDataInformationChoice( TimingDataInformationChoice c, const vector<int> &iAnswers )
{
	switch( c )
	{
	DEFAULT_FAIL( c );
	case beat_0_offset:
		ScreenTextEntry::TextEntry(
			SM_BackFromBeat0Change,
			ENTER_BEAT_0_OFFSET,
			std::to_string(GetAppropriateTiming().m_fBeat0OffsetInSeconds),
			20
			);
		break;
	case bpm:
		ScreenTextEntry::TextEntry(
			SM_BackFromBPMChange,
			ENTER_BPM_VALUE,
			std::to_string( GetAppropriateTiming().GetBPMAtBeat( GetBeat() ) ),
			10
			);
		break;
	case stop:
		ScreenTextEntry::TextEntry(
			SM_BackFromStopChange,
			ENTER_STOP_VALUE,
			std::to_string( GetAppropriateTiming().GetStopAtBeat( GetBeat() ) ),
			10
			);
		break;
	case delay:
		ScreenTextEntry::TextEntry(
			SM_BackFromDelayChange,
			ENTER_DELAY_VALUE,
			std::to_string( GetAppropriateTiming().GetDelayAtBeat( GetBeat() ) ),
			10
		);
		break;
	case tickcount:
		ScreenTextEntry::TextEntry(
			SM_BackFromTickcountChange,
			ENTER_TICKCOUNT_VALUE,
			ssprintf( "%d", GetAppropriateTiming().GetTickcountAtBeat( GetBeat() ) ),
			2
			);
		break;
	case combo:
	{
		const ComboSegment *cs = GetAppropriateTiming().GetComboSegmentAtBeat(GetBeat());
		ScreenTextEntry::TextEntry(SM_BackFromComboChange,
								   ENTER_COMBO_VALUE,
								   ssprintf( "%d/%d",
											cs->GetCombo(),
											cs->GetMissCombo()),
								   7);
		break;
	}
	case label:
		ScreenTextEntry::TextEntry(
		   SM_BackFromLabelChange,
		   ENTER_LABEL_VALUE,
		   ssprintf( "%s", GetAppropriateTiming().GetLabelAtBeat( GetBeat() ).c_str() ),
		   64
		   );
		break;
	case warp:
		ScreenTextEntry::TextEntry(
		   SM_BackFromWarpChange,
		   ENTER_WARP_VALUE,
		   std::to_string( GetAppropriateTiming().GetWarpAtBeat( GetBeat() ) ),
		   10
		   );
		break;
	case speed_percent:
		ScreenTextEntry::TextEntry(
		   SM_BackFromSpeedPercentChange,
		   ENTER_SPEED_PERCENT_VALUE,
		   std::to_string( GetAppropriateTiming().GetSpeedSegmentAtBeat( GetBeat() )->GetRatio() ),
		   10
		   );
		break;
	case scroll:
		ScreenTextEntry::TextEntry(
		   SM_BackFromScrollChange,
		   ENTER_SCROLL_VALUE,
		   std::to_string( GetAppropriateTiming().GetScrollSegmentAtBeat( GetBeat() )->GetRatio() ),
		   10
		   );
		break;
	case speed_wait:
		ScreenTextEntry::TextEntry(
		   SM_BackFromSpeedWaitChange,
		   ENTER_SPEED_WAIT_VALUE,
		   std::to_string( GetAppropriateTiming().GetSpeedSegmentAtBeat( GetBeat() )->GetDelay() ),
		   10
		   );
		break;
	case speed_mode:
		{
			ScreenTextEntry::TextEntry(
						SM_BackFromSpeedModeChange,
						   ENTER_SPEED_MODE_VALUE,
						   "",
						   3
			);

			break;
		}
	case fake:
		{
			ScreenTextEntry::TextEntry(
				SM_BackFromFakeChange,
				ENTER_FAKE_VALUE,
			        std::to_string(GetAppropriateTiming().GetFakeAtBeat( GetBeat() ) ),
				10
			);
			break;
		}
		case shift_timing_in_region_down:
			m_timing_change_menu_purpose= menu_is_for_shifting;
			m_timing_rows_being_shitted= GetRowsFromAnswers(c, iAnswers);
			DisplayTimingChangeMenu();
			break;
		case shift_timing_in_region_up:
			m_timing_change_menu_purpose= menu_is_for_shifting;
			m_timing_rows_being_shitted= -GetRowsFromAnswers(c, iAnswers);
			DisplayTimingChangeMenu();
			break;
		case copy_timing_in_region:
			m_timing_change_menu_purpose= menu_is_for_copying;
			DisplayTimingChangeMenu();
			break;
		case clear_timing_in_region:
			m_timing_change_menu_purpose= menu_is_for_clearing;
			DisplayTimingChangeMenu();
			break;
		case paste_timing_from_clip:
			clipboardFullTiming.CopyRange(0, MAX_NOTE_ROW, TimingSegmentType_Invalid, GetRow(), GetAppropriateTimingForUpdate());
			break;
	case copy_full_timing:
	{
		clipboardFullTiming = GetAppropriateTiming();
		break;
	}
	case paste_full_timing:
	{
		if(GAMESTATE->m_bIsUsingStepTiming)
		{
			GAMESTATE->m_pCurSteps[PLAYER_1]->m_Timing = clipboardFullTiming;
		}
		else
		{
			GAMESTATE->m_pCurSong->m_SongTiming = clipboardFullTiming;
		}
		SetDirty(true);
		break;
	}
	case erase_step_timing:
		ScreenPrompt::Prompt( SM_DoEraseStepTiming, CONFIRM_TIMING_ERASE , PROMPT_YES_NO, ANSWER_NO );
	break;

	}
}

void ScreenEdit::HandleTimingDataChangeChoice(TimingDataChangeChoice choice,
	const vector<int>& answers)
{
	TimingSegmentType change_type= TimingSegmentType_Invalid;
	switch(choice)
	{
		case timing_all:
			change_type= TimingSegmentType_Invalid;
			break;
		case timing_bpm:
			change_type= SEGMENT_BPM;
			break;
		case timing_stop:
			change_type= SEGMENT_STOP;
			break;
		case timing_delay:
			change_type= SEGMENT_DELAY;
			break;
		case timing_time_sig:
			change_type= SEGMENT_TIME_SIG;
			break;
		case timing_warp:
			change_type= SEGMENT_WARP;
			break;
		case timing_label:
			change_type= SEGMENT_LABEL;
			break;
		case timing_tickcount:
			change_type= SEGMENT_TICKCOUNT;
			break;
		case timing_combo:
			change_type= SEGMENT_COMBO;
			break;
		case timing_speed:
			change_type= SEGMENT_SPEED;
			break;
		case timing_scroll:
			change_type= SEGMENT_SCROLL;
			break;
		case timing_fake:
			change_type= SEGMENT_FAKE;
			break;
		default: break;
	}
	int begin= m_NoteFieldEdit.m_iBeginMarker;
	int end= m_NoteFieldEdit.m_iEndMarker;
	if(begin < 0)
	{
		begin= GetRow();
	}
	if(end < 0)
	{
		end= MAX_NOTE_ROW;
	}
	switch(m_timing_change_menu_purpose)
	{
		case menu_is_for_copying:
			clipboardFullTiming.Clear();
			GetAppropriateTiming().CopyRange(begin, end, change_type, 0, clipboardFullTiming);
			break;
		case menu_is_for_shifting:
			GetAppropriateTimingForUpdate().ShiftRange(begin, end, change_type, m_timing_rows_being_shitted);
			break;
		case menu_is_for_clearing:
			GetAppropriateTimingForUpdate().ClearRange(begin, end, change_type);
			break;
		default: break;
	}
}

void ScreenEdit::HandleBGChangeChoice( BGChangeChoice c, const vector<int> &iAnswers )
{
	BackgroundChange newChange;

	auto &changes = m_pSong->GetBackgroundChanges(g_CurrentBGChangeLayer);
	for (auto iter = changes.begin(); iter != changes.end(); ++iter)
	{
		if( iter->m_fStartBeat == GAMESTATE->m_Position.m_fSongBeat )
		{
			newChange = *iter;
			// delete the old change.  We'll add a new one below.
			changes.erase( iter );
			break;
		}
	}

	newChange.m_fStartBeat    = GAMESTATE->m_Position.m_fSongBeat;
	newChange.m_fRate         = StringToFloat( g_BackgroundChange.rows[rate].choices[iAnswers[rate]] )/100.f;
	newChange.m_sTransition   = iAnswers[transition] ? g_BackgroundChange.rows[transition].choices[iAnswers[transition]] : RString();
	newChange.m_def.m_sEffect = iAnswers[effect]     ? g_BackgroundChange.rows[effect].choices[iAnswers[effect]]         : RString();
	newChange.m_def.m_sColor1 = iAnswers[color1]     ? g_BackgroundChange.rows[color1].choices[iAnswers[color1]]         : RString();
	newChange.m_def.m_sColor2 = iAnswers[color2]     ? g_BackgroundChange.rows[color2].choices[iAnswers[color2]]         : RString();
	switch( iAnswers[file1_type] )
	{
	DEFAULT_FAIL( iAnswers[file1_type] );
	case none:			newChange.m_def.m_sFile1 = "";					break;
	case dynamic_random:		newChange.m_def.m_sFile1 = RANDOM_BACKGROUND_FILE;		break;
	case baked_random:		newChange.m_def.m_sFile1 = GetOneBakedRandomFile( m_pSong );	break;
	case song_bganimation:
	case song_movie:
	case song_bitmap:
	case global_bganimation:
	case global_movie:
	case global_movie_song_group:
	case global_movie_song_group_and_genre:
		{
			BGChangeChoice row1 = (BGChangeChoice)(file1_song_bganimation + iAnswers[file1_type]);
			newChange.m_def.m_sFile1 = g_BackgroundChange.rows[row1].choices.empty() ? "" : g_BackgroundChange.rows[row1].choices[iAnswers[row1]];
		}
		break;
	}
	switch( iAnswers[file2_type] )
	{
	DEFAULT_FAIL(iAnswers[file2_type]);
	case none:				newChange.m_def.m_sFile2 = "";				break;
	case dynamic_random:		newChange.m_def.m_sFile2 = RANDOM_BACKGROUND_FILE;		break;
	case baked_random:		newChange.m_def.m_sFile2 = GetOneBakedRandomFile( m_pSong );	break;
	case song_bganimation:
	case song_movie:
	case song_bitmap:
	case global_bganimation:
	case global_movie:
	case global_movie_song_group:
	case global_movie_song_group_and_genre:
		{
			BGChangeChoice row2 = (BGChangeChoice)(file2_song_bganimation + iAnswers[file2_type]);
			newChange.m_def.m_sFile2 = g_BackgroundChange.rows[row2].choices.empty() ? "" : g_BackgroundChange.rows[row2].choices[iAnswers[row2]];
		}
		break;
	}


	if( c == delete_change || newChange.m_def.m_sFile1.empty() )
	{
		// don't add
	}
	else
	{
		m_pSong->AddBackgroundChange( g_CurrentBGChangeLayer, newChange );
	}
	g_CurrentBGChangeLayer = BACKGROUND_LAYER_Invalid;
}

void ScreenEdit::SetupCourseAttacks()
{
	/* This is the first beat that can be changed without it being visible.  Until
	 * we draw for the first time, any beat can be changed. */
	GAMESTATE->m_pPlayerState[PLAYER_1]->m_fLastDrawnBeat = -100;

	// Put course options into effect.
	GAMESTATE->m_pPlayerState[PLAYER_1]->m_ModsToApply.clear();
	GAMESTATE->m_pPlayerState[PLAYER_1]->RemoveActiveAttacks();


	if( GAMESTATE->m_pCurCourse )
	{
		AttackArray Attacks;

		if( EDIT_MODE == EditMode_CourseMods )
		{
			Attacks = GAMESTATE->m_pCurCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex].attacks;
		}
		else
		{
			GAMESTATE->m_pCurCourse->RevertFromDisk();	// Remove this and have a separate reload key?

			for( unsigned e = 0; e < GAMESTATE->m_pCurCourse->m_vEntries.size(); ++e )
			{
				if( GAMESTATE->m_pCurCourse->m_vEntries[e].songID.ToSong() != m_pSong )
					continue;

				Attacks = GAMESTATE->m_pCurCourse->m_vEntries[e].attacks;
				break;
			}
		}

		for (Attack &attack: Attacks)
			GAMESTATE->m_pPlayerState[PLAYER_1]->LaunchAttack( attack );
	}
	else
	{
		const PlayerOptions &p = GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetCurrent();
		if (GAMESTATE->m_pCurSong && p.m_fNoAttack == 0 && p.m_fRandAttack == 0 )
		{
			AttackArray &attacks = GAMESTATE->m_bIsUsingStepTiming ?
				GAMESTATE->m_pCurSteps[PLAYER_1]->m_Attacks :
				GAMESTATE->m_pCurSong->m_Attacks;

			if (attacks.size() > 0)
			{
				for (Attack &attack : attacks)
				{
					// LaunchAttack is actually a misnomer.  The function actually adds
					// the attack to a list in the PlayerState which is checked and
					// updated every tick to see which ones to actually activate. -Kyz
					GAMESTATE->m_pPlayerState[PLAYER_1]->LaunchAttack( attack );
				}
			}
		}
	}

	GAMESTATE->m_pPlayerState[PLAYER_1]->RebuildPlayerOptionsFromActiveAttacks();
}

void ScreenEdit::CopyToLastSave()
{
	ASSERT( GAMESTATE->m_pCurSong != nullptr );
	ASSERT( GAMESTATE->m_pCurSteps[PLAYER_1] != nullptr );
	m_SongLastSave = *GAMESTATE->m_pCurSong;
	m_vStepsLastSave.clear();
	const vector<Steps*> &vSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( GAMESTATE->m_pCurSteps[PLAYER_1]->m_StepsType );
	for (Steps *it : vSteps)
		m_vStepsLastSave.push_back( *it );
}

void ScreenEdit::CopyFromLastSave()
{
	// We are assuming two things here:
	// 1) No steps can be created by ScreenEdit
	// 2) No steps can be deleted by ScreenEdit (except possibly when we exit)
	*GAMESTATE->m_pCurSong = m_SongLastSave;
	const vector<Steps*> &vSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( GAMESTATE->m_pCurSteps[PLAYER_1]->m_StepsType );
	ASSERT_M( vSteps.size() == m_vStepsLastSave.size(), ssprintf("Step sizes don't match: %d, %d", int(vSteps.size()), int(m_vStepsLastSave.size())) );
	for( unsigned i = 0; i < vSteps.size(); ++i )
		*vSteps[i] = m_vStepsLastSave[i];
}

void ScreenEdit::RevertFromDisk()
{
	ASSERT( GAMESTATE->m_pCurSteps[PLAYER_1] != nullptr );
	StepsID id;
	id.FromSteps( GAMESTATE->m_pCurSteps[PLAYER_1] );
	ASSERT( id.IsValid() );

	// If m_bInStepEditor is true while the song is reloaded, it screws up
	// loading and results in the steps being cleared.  -Kyz
	GAMESTATE->m_bInStepEditor= false;
	GAMESTATE->m_pCurSong->ReloadFromSongDir();
	GAMESTATE->m_bInStepEditor= true;

	Steps *pNewSteps = id.ToSteps( GAMESTATE->m_pCurSong, true );
	if( !pNewSteps )
	{
		// If the Steps we were currently editing vanished when we did the revert,
		// put a blank Steps in its place.  Note that this does not have to be the
		// work of someone maliciously changing the simfile; it could happen to
		// someone editing a new stepchart and reverting from disk, for example.
		pNewSteps = GAMESTATE->m_pCurSong->CreateSteps();
		pNewSteps->CreateBlank( id.GetStepsType() );
		pNewSteps->SetDifficulty( id.GetDifficulty() );
		GAMESTATE->m_pCurSong->AddSteps( pNewSteps );
	}
	GAMESTATE->m_pCurSteps[PLAYER_1].Set( pNewSteps );
	m_pSteps = pNewSteps;

	CopyToLastSave();
	SetDirty( false );
	SONGMAN->Invalidate( GAMESTATE->m_pCurSong );
}

void ScreenEdit::SaveUndo()
{
	m_bHasUndo = true;
	m_Undo.CopyAll( m_NoteDataEdit );
}

static LocalizedString UNDO			("ScreenEdit", "Undo");
static LocalizedString CANT_UNDO		("ScreenEdit", "Can't undo - no undo data.");
void ScreenEdit::Undo()
{
	if( m_bHasUndo )
	{
		swap( m_Undo, m_NoteDataEdit );
		SCREENMAN->SystemMessage( UNDO );
	}
	else
	{
		SCREENMAN->SystemMessage( CANT_UNDO );
		SCREENMAN->PlayInvalidSound();
	}
}

void ScreenEdit::ClearUndo()
{
	m_bHasUndo = false;
	m_Undo.ClearAll();
}

static LocalizedString CREATES_MORE_THAN_NOTES	( "ScreenEdit", "This change creates more than %d notes in a measure." );
static LocalizedString CREATES_NOTES_PAST_END	( "ScreenEdit", "This change creates notes past the end of the music and is not allowed." );
static LocalizedString CHANGE_REVERTED		( "ScreenEdit", "The change has been reverted." );
void ScreenEdit::CheckNumberOfNotesAndUndo()
{
	if( EDIT_MODE.GetValue() != EditMode_Home )
		return;

	const float fBeat = GAMESTATE->m_pPlayerState[PLAYER_1]->m_Position.m_fSongBeat;
	const TimeSignatureSegment * curTime = GAMESTATE->m_pCurSong->m_SongTiming.GetTimeSignatureSegmentAtBeat( fBeat );
	int rowsPerMeasure = curTime->GetDen() * curTime->GetNum();

	for( int row=0; row<=m_NoteDataEdit.GetLastRow(); row+=rowsPerMeasure )
	{
		int iNumNotesThisMeasure = 0;
		FOREACH_NONEMPTY_ROW_ALL_TRACKS_RANGE( m_NoteDataEdit, r, row, row+rowsPerMeasure )
			iNumNotesThisMeasure += m_NoteDataEdit.GetNumTapNonEmptyTracks( r );
	}

	if( GAMESTATE->m_pCurSteps[0]->IsAnEdit() )
	{
		/* Check that the action didn't push notes any farther past the last
		 * measure. This blocks Insert Beat from pushing past the end, but allows
		 * Delete Beat to pull back the notes that are already past the end.
		 */
		float fNewLastBeat = m_NoteDataEdit.GetLastBeat();
		bool bLastBeatIncreased = fNewLastBeat > m_Undo.GetLastBeat();
		bool bPassedTheEnd = fNewLastBeat > GetMaximumBeatForNewNote();
		if( bLastBeatIncreased && bPassedTheEnd )
		{
			Undo();
			m_bHasUndo = false;
			RString sError = CREATES_NOTES_PAST_END.GetValue() + "\n\n" + CHANGE_REVERTED.GetValue();
			ScreenPrompt::Prompt( SM_None, sError );
			return;
		}
	}
}

float ScreenEdit::GetMaximumBeatForNewNote() const
{
	switch( EDIT_MODE.GetValue() )
	{
	DEFAULT_FAIL( EDIT_MODE.GetValue() );
	case EditMode_Practice:
	case EditMode_CourseMods:
	case EditMode_Home:
		{
			Song &s = *GAMESTATE->m_pCurSong;
			float fEndBeat = s.GetLastBeat();

			/* Round up to the next measure end.  Some songs end on weird beats
			 * mid-measure, and it's odd to have movement capped to these weird
			 * beats. */
			TimingData &timing = s.m_SongTiming;
			float playerBeat = GetAppropriatePosition().m_fSongBeat;
			int beatsPerMeasure = timing.GetTimeSignatureSegmentAtBeat( playerBeat )->GetNum();
			fEndBeat += beatsPerMeasure;
			fEndBeat = ftruncf( fEndBeat, (float)beatsPerMeasure );

			return fEndBeat;
		}
	case EditMode_Full:
		return NoteRowToBeat(MAX_NOTE_ROW);
	}
}

float ScreenEdit::GetMaximumBeatForMoving() const
{
	float fEndBeat = GetMaximumBeatForNewNote();

	/* Jump to GetLastBeat even if it's past the song's last beat
	 * so that users can delete garbage steps past then end that they have
	 * have inserted in a text editor.  Once they delete all steps on
	 * GetLastBeat() and move off of that beat, they won't be able to return. */
	fEndBeat = max( fEndBeat, m_NoteDataEdit.GetLastBeat() );

	return fEndBeat;
}

struct EditHelpLine
{
	const char *szEnglishDescription;
	vector<EditButton> veb;

	EditHelpLine(
		const char *_szEnglishDescription,
		EditButton eb0,
		EditButton eb1 = EditButton_Invalid,
		EditButton eb2 = EditButton_Invalid,
		EditButton eb3 = EditButton_Invalid,
		EditButton eb4 = EditButton_Invalid,
		EditButton eb5 = EditButton_Invalid,
		EditButton eb6 = EditButton_Invalid,
		EditButton eb7 = EditButton_Invalid,
		EditButton eb8 = EditButton_Invalid,
		EditButton eb9 = EditButton_Invalid )
	{
		szEnglishDescription = _szEnglishDescription;
#define PUSH_IF_VALID( x ) if( x != EditButton_Invalid ) veb.push_back( x );
		PUSH_IF_VALID( eb0 );
		PUSH_IF_VALID( eb1 );
		PUSH_IF_VALID( eb2 );
		PUSH_IF_VALID( eb3 );
		PUSH_IF_VALID( eb4 );
		PUSH_IF_VALID( eb5 );
		PUSH_IF_VALID( eb6 );
		PUSH_IF_VALID( eb7 );
		PUSH_IF_VALID( eb8 );
		PUSH_IF_VALID( eb9 );
#undef PUSH_IF_VALID
	}
};
// TODO: Identify which of these can be removed and sent to a readme.
static const EditHelpLine g_EditHelpLines[] =
{
	EditHelpLine( "Move cursor",					EDIT_BUTTON_SCROLL_UP_LINE,		EDIT_BUTTON_SCROLL_DOWN_LINE ),
	EditHelpLine( "Jump measure",					EDIT_BUTTON_SCROLL_UP_PAGE,		EDIT_BUTTON_SCROLL_DOWN_PAGE ),
	EditHelpLine( "Jump measure",					EDIT_BUTTON_SCROLL_PREV_MEASURE,	EDIT_BUTTON_SCROLL_NEXT_MEASURE ),
	EditHelpLine( "Select region",					EDIT_BUTTON_SCROLL_SELECT ),
	EditHelpLine( "Jump to first/last beat",			EDIT_BUTTON_SCROLL_HOME,		EDIT_BUTTON_SCROLL_END ),
	EditHelpLine( "Change zoom",					EDIT_BUTTON_SCROLL_SPEED_UP,		EDIT_BUTTON_SCROLL_SPEED_DOWN ),
	EditHelpLine( "Play",						EDIT_BUTTON_PLAY_SELECTION ),
	EditHelpLine( "Play current beat to end",			EDIT_BUTTON_PLAY_FROM_CURSOR ),
	EditHelpLine( "Play whole song",				EDIT_BUTTON_PLAY_FROM_START ),
	EditHelpLine( "Record",						EDIT_BUTTON_RECORD_SELECTION ),
	EditHelpLine( "Set selection",					EDIT_BUTTON_LAY_SELECT ),
	EditHelpLine( "Next/prev steps of same StepsType",		EDIT_BUTTON_OPEN_PREV_STEPS, 		EDIT_BUTTON_OPEN_NEXT_STEPS ),
	EditHelpLine( "Decrease/increase BPM at cur beat",		EDIT_BUTTON_BPM_DOWN,			EDIT_BUTTON_BPM_UP ),
	EditHelpLine( "Decrease/increase stop at cur beat",		EDIT_BUTTON_STOP_DOWN,			EDIT_BUTTON_STOP_UP ),
	EditHelpLine( "Decrease/increase delay at cur beat",		EDIT_BUTTON_DELAY_DOWN,			EDIT_BUTTON_DELAY_UP ),
	EditHelpLine( "Decrease/increase music offset",			EDIT_BUTTON_OFFSET_DOWN,		EDIT_BUTTON_OFFSET_UP ),
	EditHelpLine( "Decrease/increase sample music start",		EDIT_BUTTON_SAMPLE_START_DOWN,		EDIT_BUTTON_SAMPLE_START_UP ),
	EditHelpLine( "Decrease/increase sample music length",		EDIT_BUTTON_SAMPLE_LENGTH_DOWN,		EDIT_BUTTON_SAMPLE_LENGTH_UP ),
	EditHelpLine( "Play sample music",				EDIT_BUTTON_PLAY_SAMPLE_MUSIC ),
	EditHelpLine( "Add/Edit Background Change",			EDIT_BUTTON_OPEN_BGCHANGE_LAYER1_MENU ),
	EditHelpLine( "Insert beat and shift down",			EDIT_BUTTON_INSERT ),
	EditHelpLine( "Shift BPM changes and stops down one beat",	EDIT_BUTTON_INSERT_SHIFT_PAUSES ),
	EditHelpLine( "Delete beat and shift up",			EDIT_BUTTON_DELETE ),
	EditHelpLine( "Shift BPM changes and stops up one beat",	EDIT_BUTTON_DELETE_SHIFT_PAUSES ),
	EditHelpLine( "Cycle between tap notes",			EDIT_BUTTON_CYCLE_TAP_LEFT,		EDIT_BUTTON_CYCLE_TAP_RIGHT ),
	EditHelpLine( "Add to/remove from right half",			EDIT_BUTTON_RIGHT_SIDE ),
	EditHelpLine( "Switch Timing",					EDIT_BUTTON_SWITCH_TIMINGS ),
	EditHelpLine( "Switch player (Routine only)",			EDIT_BUTTON_SWITCH_PLAYERS ),
};

static bool IsMapped( EditButton eb, const MapEditToDI &editmap )
{
	for( int s=0; s<NUM_EDIT_TO_DEVICE_SLOTS; s++ )
	{
		DeviceInput diPress = editmap.button[eb][s];
		if( diPress.IsValid() )
			return true;
	}
	return false;
}

static void ProcessKeyName( RString &s )
{
	s.Replace( "Key_", "" );
}

static void ProcessKeyNames( vector<RString> &vs, bool doSort )
{
	for (RString &s : vs)
		ProcessKeyName( s );

	if (doSort)
		sort( vs.begin(), vs.end() );
	vector<RString>::iterator toDelete = unique( vs.begin(), vs.end() );
	vs.erase(toDelete, vs.end());
}

static RString GetDeviceButtonsLocalized( const vector<EditButton> &veb, const MapEditToDI &editmap )
{
	vector<RString> vsPress;
	vector<RString> vsHold;
	for (EditButton const &eb : veb)
	{
		if( !IsMapped( eb, editmap ) )
			continue;

		for( int s=0; s<NUM_EDIT_TO_DEVICE_SLOTS; s++ )
		{
			DeviceInput diPress = editmap.button[eb][s];
			DeviceInput diHold = editmap.hold[eb][s];
			if( diPress.IsValid() )
				vsPress.push_back( Capitalize(INPUTMAN->GetLocalizedInputString(diPress)) );
			if( diHold.IsValid() )
				vsHold.push_back( Capitalize(INPUTMAN->GetLocalizedInputString(diHold)) );
		}
	}

	ProcessKeyNames( vsPress, false );
	ProcessKeyNames( vsHold, true );

	RString s = join("/",vsPress);
	if( !vsHold.empty() )
		s = join("/",vsHold) + " + " + s;
	return s;
}

void ScreenEdit::DoStepAttackMenu()
{
	const TimingData &timing = GetAppropriateTiming();
	float startTime = timing.GetElapsedTimeFromBeat(GetBeat());
	AttackArray &attacks =
		(GAMESTATE->m_bIsUsingStepTiming ? m_pSteps->m_Attacks : m_pSong->m_Attacks);
	vector<int> points = FindAllAttacksAtTime(attacks, startTime);

	g_AttackAtTimeMenu.rows.clear();
	unsigned index = 0;

	for (int &i : points)
	{
		const Attack &attack = attacks[i];
		RString desc = ssprintf("%g -> %g (%d mod[s])",
			startTime, startTime + attack.fSecsRemaining,
			attack.GetNumAttacks());

		g_AttackAtTimeMenu.rows.push_back(MenuRowDef(index++,
			desc,
			true,
			EditMode_CourseMods,
			false,
			false,
			0,
			"Modify",
			"Delete"));
	}
	g_AttackAtTimeMenu.rows.push_back(MenuRowDef(index,
		"Add Attack",
		true,
		EditMode_CourseMods,
		true,
		true,
		0,
		nullptr));

	EditMiniMenu(&g_AttackAtTimeMenu, SM_BackFromAttackAtTime);
}

static LocalizedString TRACK_NUM("ScreenEdit", "Track %d");
static LocalizedString NO_KEYSND("ScreenEdit", "None");
static LocalizedString NEWKEYSND("ScreenEdit", "New Sound");

void ScreenEdit::DoKeyboardTrackMenu()
{
	g_KeysoundTrack.rows.clear();
	vector<RString> &kses = m_pSong->m_vsKeysoundFile;

	vector<RString> choices;
	for (RString const &ks : kses)
	{
		choices.push_back(ks);
	}
	choices.push_back(NEWKEYSND);
	choices.push_back(NO_KEYSND);
	int numKeysounds = kses.size();
	int foundKeysounds = 0;
	for (int i = 0; i < m_NoteDataEdit.GetNumTracks(); ++i)
	{
		const TapNote &tn = m_NoteDataEdit.GetTapNote(i, this->GetRow());
		int keyIndex = tn.iKeysoundIndex;
		if (keyIndex == -1)
		{
			keyIndex = numKeysounds;
		}
		else
		{
			++foundKeysounds;
		}

		g_KeysoundTrack.rows.push_back(MenuRowDef(i, ssprintf(TRACK_NUM.GetValue(), i + 1),
												  true, EditMode_Full, false, false, keyIndex, choices));
	}
	g_KeysoundTrack.rows.push_back(MenuRowDef(m_NoteDataEdit.GetNumTracks(), "Remove Keysound",
											  foundKeysounds > 0, EditMode_Full, false, false, 0, kses));

	EditMiniMenu(&g_KeysoundTrack, SM_BackFromKeysoundTrack);
}

void ScreenEdit::DoHelp()
{
	g_EditHelp.rows.clear();

	for( unsigned i=0; i<ARRAYLEN(g_EditHelpLines); ++i )
	{
		const EditHelpLine &hl = g_EditHelpLines[i];

		if( !IsMapped(hl.veb[0],m_EditMappingsDeviceInput) )
			continue;

		RString sButtons = GetDeviceButtonsLocalized( hl.veb, m_EditMappingsDeviceInput );
		RString sDescription = THEME->GetString( "EditHelpDescription", hl.szEnglishDescription );

		// TODO: Better way of hiding routine only key on non-routine.
		if( hl.veb[0] == EDIT_BUTTON_SWITCH_PLAYERS && m_InputPlayerNumber == PLAYER_INVALID )
		{
			continue;
		}

		g_EditHelp.rows.push_back( MenuRowDef( -1, sDescription, false, EditMode_Practice, false, false, 0, sButtons ) );
	}

	EditMiniMenu( &g_EditHelp );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to ScreenEdit. */
class LunaScreenEdit: public Luna<ScreenEdit>
{
public:
	DEFINE_METHOD( GetEditState, GetEditState() )
	LunaScreenEdit()
	{
		ADD_METHOD( GetEditState );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenEdit, ScreenWithMenuElements )

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
