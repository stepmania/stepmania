#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenEdit

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenEdit.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "InputMapper.h"
#include "RageLog.h"
#include <math.h>
#include "ThemeManager.h"
#include "SDL_keysym.h"		// for SDLKeys
#include "ScreenMiniMenu.h"
#include "NoteSkinManager.h"
#include "Notes.h"
#include <utility>
#include "NoteFieldPositioning.h"


const float RECORD_HOLD_SECONDS = 0.3f;


//
// Defines specific to GameScreenTitleMenu
//

const float MAX_SECONDS_CAN_BE_OFF_BY	=	0.20f;
const float GRAY_ARROW_Y				= ARROW_SIZE * 1.5;

const float DEBUG_X			= SCREEN_LEFT + 10;
const float DEBUG_Y			= CENTER_Y-100;

const float SHORTCUTS_X		= CENTER_X - 150;
const float SHORTCUTS_Y		= CENTER_Y;

const float HELP_X			= SCREEN_LEFT;
const float HELP_Y			= CENTER_Y;

const float HELP_TEXT_X		= SCREEN_LEFT + 4;
const float HELP_TEXT_Y		= 40;

const float INFO_X			= SCREEN_RIGHT;
const float INFO_Y			= CENTER_Y;

const float INFO_TEXT_X		= SCREEN_RIGHT - 114;
const float INFO_TEXT_Y		= 40;

const float MENU_WIDTH		=	110;
const float EDIT_X			=	CENTER_X;
const float EDIT_GRAY_Y		=	GRAY_ARROW_Y;

const float PLAYER_X		=	CENTER_X;
const float PLAYER_Y		=	SCREEN_TOP;

const float ACTION_MENU_ITEM_X			=	CENTER_X-200;
const float ACTION_MENU_ITEM_START_Y	=	SCREEN_TOP + 24;
const float ACTION_MENU_ITEM_SPACING_Y	=	18;

const float NAMING_MENU_ITEM_X			=	CENTER_X-200;
const float NAMING_MENU_ITEM_START_Y	=	SCREEN_TOP + 24;
const float NAMING_MENU_ITEM_SPACING_Y	=	18;

CachedThemeMetricF	 TICK_EARLY_SECONDS		("ScreenGameplay","TickEarlySeconds");


const ScreenMessage SM_BackFromMainMenu				= (ScreenMessage)(SM_User+1);
const ScreenMessage SM_BackFromAreaMenu				= (ScreenMessage)(SM_User+2);
const ScreenMessage SM_BackFromEditNotesStatistics	= (ScreenMessage)(SM_User+3);
const ScreenMessage SM_BackFromEditOptions			= (ScreenMessage)(SM_User+4);
const ScreenMessage SM_BackFromEditSongInfo			= (ScreenMessage)(SM_User+5);
const ScreenMessage SM_BackFromBGChange				= (ScreenMessage)(SM_User+6);


const CString HELP_TEXT = 
	"Up/Down:\n     change beat\n"
	"Left/Right:\n     change snap\n"
	"Number keys:\n     add/remove\n     tap note\n"
	"Create hold note:\n     Hold a number\n     while moving\n     Up or Down\n"
	"Space bar:\n     Set area\n     marker\n"
	"Enter:\n     Area Menu\n"
	"Escape:\n     Main Menu\n"
	"F1:\n     Show\n     keyboard\n     shortcuts\n";


Menu g_KeyboardShortcuts
(
	"Keyboard Shortcuts",
	MenuRow( "PgUp/PgDn: jump measure",							false ),
	MenuRow( "Home/End: jump to first/last beat",				false ),
	MenuRow( "Ctrl + Up/Down: Change zoom",						false ),
	MenuRow( "Shift + Up/Down: Drag area marker",				false ),
	MenuRow( "P: Play selection",								false ),
	MenuRow( "Ctrl + P: Play whole song",						false ),
	MenuRow( "Shift + P: Play current beat to end",				false ),
	MenuRow( "Ctrl + R: Record",								false ),
	MenuRow( "F4: Toggle assist tick",							false ),
	MenuRow( "F7/F8: Decrease/increase BPM at cur beat",		false ),
	MenuRow( "F9/F10: Decrease/increase stop at cur beat",		false ),
	MenuRow( "F11/F12: Decrease/increase music offset",			false ),
		/* XXX: This would be better as a single submenu, to let people tweak
		 * and play the sample several times (without having to re-enter the
		 * menu each time), so it doesn't use a whole bunch of hotkeys. */
	MenuRow( "[ and ]: Decrease/increase sample music start",	false ),
	MenuRow( "{ and }: Decrease/increase sample music length",	false ),
	MenuRow( "M: Play sample music",							false ),
	MenuRow( "B: Add/Edit Background Change",					false ),
	MenuRow( "Insert: Insert beat and shift down",				false ),
	MenuRow( "Delete: Delete beat and shift up",				false )
);

Menu g_MainMenu
(
	"Main Menu",
	MenuRow( "Edit Notes Statistics",		true ),
	MenuRow( "Play Whole Song",				true ),
	MenuRow( "Play Current Beat To End",	true ),
	MenuRow( "Save",						true ),
	MenuRow( "Player Options",				true ),
	MenuRow( "Song Options",				true ),
	MenuRow( "Edit Song Info",				true ),
	MenuRow( "Add/Edit BPM Change",			true ),
	MenuRow( "Add/Edit Stop",				true ),
	MenuRow( "Add/Edit BG Change",			true ),
	MenuRow( "Play preview music",			true ),
	MenuRow( "Exit (discards changes since last save)",true )
);

Menu g_AreaMenu
(
	"Area Menu",
	MenuRow( "Cut",							true ),
	MenuRow( "Copy",						true ),
	MenuRow( "Paste at current beat",		true ),
	MenuRow( "Paste at begin marker",		true ),
	MenuRow( "Clear",						true ),
	MenuRow( "Quantize",					true, 0, "4TH","8TH","12TH","16TH","24TH","32ND" ),
	MenuRow( "Turn",						true, 0, "Left","Right","Mirror","Shuffle","Super Shuffle" ),
	MenuRow( "Transform",					true, 0, "Little","Wide","Big","Quick","Skippy" ),
	MenuRow( "Alter",						true, 0, "Backwards","Swap Sides","Copy Left To Right","Copy Right To Left","Clear Left","Clear Right","Collapse To One","Shift Left","Shift Right" ),
	MenuRow( "Play selection",				true ),
	MenuRow( "Record in selection",			true ),
	MenuRow( "Insert beat and shift down",	true ),
	MenuRow( "Delete beat and shift up",	true )
);


Menu g_EditNotesStatistics
(
	"Statistics",
	MenuRow( "Difficulty",	true, 0, "BEGINNER","EASY","MEDIUM","HARD","CHALLENGE" ),
	MenuRow( "Meter",		true, 0, "1","2","3","4","5","6","7","8","9","10","11" ),
	MenuRow( "Description",	true ),
	MenuRow( "Tap Notes",	false ),
	MenuRow( "Hold Notes",	false ),
	MenuRow( "Stream",		false ),
	MenuRow( "Voltage",		false ),
	MenuRow( "Air",			false ),
	MenuRow( "Freeze",		false ),
	MenuRow( "Chaos",		false )
);


Menu g_EditSongInfo
(
	"Edit Song Info",
	MenuRow( "Main title",					true ),
	MenuRow( "Sub title",					true ),
	MenuRow( "Artist",						true ),
	MenuRow( "Credit",						true ),
	MenuRow( "Main title transliteration",	true ),
	MenuRow( "Sub title transliteration",	true ),
	MenuRow( "Artist transliteration",		true )
);


Menu g_BGChange
(
	"Background Change",
	MenuRow( "Rate (applies to new adds)",			true, 5, "50%","60%","70%","80%","90%","100%","110%","120%","130%","140%","150%","160%","170%","180%","190%","200%" ),
	MenuRow( "Fade Last (applies to new adds)",		true, 0, "NO","YES" ),
	MenuRow( "Rewind Movie (applies to new adds)",	true, 0, "NO","YES" ),
	MenuRow( "Loop (applies to new adds)",			true, 1, "NO","YES" ),
	MenuRow( "Add Change to random",				true ),
	MenuRow( "Add Change to song BGAnimation",		true ),
	MenuRow( "Add Change to song Movie",			true ),
	MenuRow( "Add Change to song Still",			true ),
	MenuRow( "Add Change to global Random Movie",	true ),
	MenuRow( "Add Change to global BGAnimation",	true ),
	MenuRow( "Add Change to global Visualization",	true ),
	MenuRow( "Remove Change",						true )
);


ScreenEdit::ScreenEdit() : Screen("ScreenEdit")
{
	LOG->Trace( "ScreenEdit::ScreenEdit()" );

	TICK_EARLY_SECONDS.Refresh();

	// set both players to joined so the credit message doesn't show
	for( int p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_bSideIsJoined[p] = true;
	SCREENMAN->RefreshCreditsMessages();

	m_iRowLastCrossed = -1;

	m_pSong = GAMESTATE->m_pCurSong;

	m_pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];


	NoteData noteData;
	m_pNotes->GetNoteData( &noteData );


	m_EditMode = MODE_EDITING;

	GAMESTATE->m_bEditing = true;

	GAMESTATE->m_fSongBeat = 0;
	m_fTrailingBeat = GAMESTATE->m_fSongBeat;

	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 1;
	GAMESTATE->m_SongOptions.m_fMusicRate = 1;
	/* Err, not all games have a noteskin named "note" ... */
//	GAMESTATE->m_PlayerOptions[PLAYER_1].m_sNoteSkin = "note";	// change noteskin before loading all of the edit Actors

	m_BGAnimation.LoadFromAniDir( THEME->GetPathToB("ScreenEdit background") );

	shiftAnchor = -1;
	m_SnapDisplay.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_SnapDisplay.Load( PLAYER_1 );
	m_SnapDisplay.SetZoom( 0.5f );

	m_GrayArrowRowEdit.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_GrayArrowRowEdit.Load( PLAYER_1 );
	m_GrayArrowRowEdit.SetZoom( 0.5f );

	m_NoteFieldEdit.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_NoteFieldEdit.SetZoom( 0.5f );
	m_NoteFieldEdit.Load( &noteData, PLAYER_1, -240, 800 );

	m_rectRecordBack.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0) );

	m_GrayArrowRowRecord.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_GrayArrowRowRecord.Load( PLAYER_1 );
	m_GrayArrowRowRecord.SetZoom( 1.0f );

	m_NoteFieldRecord.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_NoteFieldRecord.SetZoom( 1.0f );
	m_NoteFieldRecord.Load( &noteData, PLAYER_1, -150, 350 );

	m_Clipboard.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );


	GAMESTATE->m_PlayerOptions[PLAYER_1].m_sNoteSkin = "default";	// change noteskin back to default before loading player

	m_Player.Load( PLAYER_1, &noteData, NULL, NULL, NULL, NULL );
	GAMESTATE->m_PlayerController[PLAYER_1] = PC_HUMAN;
	m_Player.SetXY( PLAYER_X, PLAYER_Y );

	m_In.Load( THEME->GetPathToB("ScreenEdit in") );
	m_In.StartTransitioning();

	m_Out.Load( THEME->GetPathToB("ScreenEdit out") );

	m_sprHelp.Load( THEME->GetPathToG("ScreenEdit help") );
	m_sprHelp.SetHorizAlign( Actor::align_left );
	m_sprHelp.SetXY( HELP_X, HELP_Y );

	m_textHelp.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textHelp.SetXY( HELP_TEXT_X, HELP_TEXT_Y );
	m_textHelp.SetHorizAlign( Actor::align_left );
	m_textHelp.SetVertAlign( Actor::align_top );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetText( HELP_TEXT );
	m_textHelp.EnableShadow( false );

	m_sprInfo.Load( THEME->GetPathToG("ScreenEdit Info") );
	m_sprInfo.SetHorizAlign( Actor::align_right );
	m_sprInfo.SetXY( INFO_X, INFO_Y );

	m_textInfo.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textInfo.SetXY( INFO_TEXT_X, INFO_TEXT_Y );
	m_textInfo.SetHorizAlign( Actor::align_left );
	m_textInfo.SetVertAlign( Actor::align_top );
	m_textInfo.SetZoom( 0.5f );
	m_textInfo.EnableShadow( false );
	//m_textInfo.SetText();	// set this below every frame

	m_soundChangeLine.Load( THEME->GetPathToS("ScreenEdit line") );
	m_soundChangeSnap.Load( THEME->GetPathToS("ScreenEdit snap") );
	m_soundMarker.Load(		THEME->GetPathToS("ScreenEdit marker") );


	m_soundMusic.Load(m_pSong->GetMusicPath());
	m_soundMusic.SetAccurateSync(true);
	m_soundMusic.SetStopMode(RageSound::M_CONTINUE);

	m_soundAssistTick.Load(		THEME->GetPathToS("ScreenEdit assist tick") );
}

ScreenEdit::~ScreenEdit()
{
	LOG->Trace( "ScreenEdit::~ScreenEdit()" );
	m_soundMusic.StopPlaying();
}

// play assist ticks
bool ScreenEdit::PlayTicks() const
{
	// Sound cards have a latency between when a sample is Play()ed and when the sound
	// will start coming out the speaker.  Compensate for this by boosting
	// fPositionSeconds ahead

	if( !GAMESTATE->m_SongOptions.m_bAssistTick )
		return false;

	float fPositionSeconds = GAMESTATE->m_fMusicSeconds;

	// HACK:  Play the sound a little bit early to account for the fact that the middle of the tick sounds occurs 0.015 seconds into playing.
	fPositionSeconds += (SOUNDMAN->GetPlayLatency()+(float)TICK_EARLY_SECONDS) * m_soundMusic.GetPlaybackRate();
	float fSongBeat=GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

	int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
	iRowNow = max( 0, iRowNow );
	static int iRowLastCrossed = 0;

	bool bAnyoneHasANote = false;	// set this to true if any player has a note at one of the indicies we crossed

	for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
		bAnyoneHasANote |= m_Player.IsThereATapAtRow( r );

	iRowLastCrossed = iRowNow;

	return bAnyoneHasANote;
}

void ScreenEdit::PlayPreviewMusic()
{
	SOUNDMAN->PlayMusic("");
	SOUNDMAN->PlayMusic( m_pSong->GetMusicPath(), false,
		m_pSong->m_fMusicSampleStartSeconds,
		m_pSong->m_fMusicSampleLengthSeconds,
		1.5f );
}

void ScreenEdit::Update( float fDeltaTime )
{
	if( m_soundMusic.IsPlaying())
		GAMESTATE->UpdateSongPosition(m_soundMusic.GetPositionSeconds());

	if( m_EditMode == MODE_RECORDING  )	
	{
		// add or extend holds

		for( int t=0; t<GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer; t++ )	// for each track
		{
			StyleInput StyleI( PLAYER_1, t );
			float fSecsHeld = INPUTMAPPER->GetSecsHeld( StyleI );

			if( fSecsHeld > RECORD_HOLD_SECONDS )
			{
				// add or extend hold
				const float fHoldStartSeconds = m_soundMusic.GetPositionSeconds() - fSecsHeld;

				float fStartBeat = m_pSong->GetBeatFromElapsedTime( fHoldStartSeconds );
				float fEndBeat = GAMESTATE->m_fSongBeat;

				// Round hold start and end to the nearest snap interval
				fStartBeat = froundf( fStartBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );
				fEndBeat = froundf( fEndBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );

				// create a new hold note
				HoldNote newHN( t, fStartBeat, fEndBeat );
				m_NoteFieldRecord.AddHoldNote( newHN );
			}
		}
	}

	if( m_EditMode == MODE_RECORDING  ||  m_EditMode == MODE_PLAYING )
	{
		// check for end of playback/record

		if( GAMESTATE->m_fSongBeat > m_NoteFieldEdit.m_fEndMarker + 4 )		// give a one measure lead out
		{
			if( m_EditMode == MODE_RECORDING )
			{
				TransitionFromRecordToEdit();
			}
			else if( m_EditMode == MODE_PLAYING )
			{
				TransitionToEdit();
			}
			GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fEndMarker;
		}
	}

	m_BGAnimation.Update( fDeltaTime );
	m_SnapDisplay.Update( fDeltaTime );
	m_GrayArrowRowEdit.Update( fDeltaTime );
	m_NoteFieldEdit.Update( fDeltaTime );
	m_In.Update( fDeltaTime );
	m_Out.Update( fDeltaTime );
	m_sprHelp.Update( fDeltaTime );
	m_textHelp.Update( fDeltaTime );
	m_sprInfo.Update( fDeltaTime );
	m_textInfo.Update( fDeltaTime );

	m_rectRecordBack.Update( fDeltaTime );

	if( m_EditMode == MODE_RECORDING )
	{
		m_GrayArrowRowRecord.Update( fDeltaTime );
		m_NoteFieldRecord.Update( fDeltaTime );
	}

	if( m_EditMode == MODE_PLAYING )
	{
		m_Player.Update( fDeltaTime );
	}

	//LOG->Trace( "ScreenEdit::Update(%f)", fDeltaTime );
	Screen::Update( fDeltaTime );


	// Update trailing beat
	float fDelta = GAMESTATE->m_fSongBeat - m_fTrailingBeat;

	if( fabsf(fDelta) < 0.01 )
	{
		m_fTrailingBeat = GAMESTATE->m_fSongBeat;	// snap
	}
	else
	{
		float fSign = fDelta / fabsf(fDelta);
		float fMoveDelta = fSign*fDeltaTime*40 / GAMESTATE->m_CurrentPlayerOptions[PLAYER_1].m_fScrollSpeed;
		if( fabsf(fMoveDelta) > fabsf(fDelta) )
			fMoveDelta = fDelta;
		m_fTrailingBeat += fMoveDelta;

		if( fabsf(fDelta) > 10 )
			m_fTrailingBeat += fDelta * fDeltaTime*5;
	}

	m_NoteFieldEdit.Update( fDeltaTime );

	if(m_EditMode == MODE_PLAYING && PlayTicks())
		m_soundAssistTick.Play();

	static float fUpdateCounter = 0;
	fUpdateCounter -= fDeltaTime;
	if( fUpdateCounter < 0 )
	{
		fUpdateCounter = 0.5;
		UpdateTextInfo();
	}
}

void ScreenEdit::UpdateTextInfo()
{
	int iNumTapNotes = m_NoteFieldEdit.GetNumTapNotes();
	int iNumHoldNotes = m_NoteFieldEdit.GetNumHoldNotes();

	CString sNoteType;
	switch( m_SnapDisplay.GetNoteType() )
	{
	case NOTE_TYPE_4TH:		sNoteType = "4th notes";	break;
	case NOTE_TYPE_8TH:		sNoteType = "8th notes";	break;
	case NOTE_TYPE_12TH:	sNoteType = "12th notes";	break;
	case NOTE_TYPE_16TH:	sNoteType = "16th notes";	break;
	case NOTE_TYPE_24TH:	sNoteType = "24th notes";	break;
	case NOTE_TYPE_32ND:	sNoteType = "32nd notes";	break;
	default:  ASSERT(0);
	}

	CString sText;
	sText += ssprintf( "Current Beat:\n     %.2f\n",		GAMESTATE->m_fSongBeat );
	sText += ssprintf( "Snap to:\n     %s\n",				sNoteType.c_str() );
	sText += ssprintf( "Selection begin:\n     %s\n",		m_NoteFieldEdit.m_fBeginMarker==-1 ? "not set" : ssprintf("%.2f",m_NoteFieldEdit.m_fBeginMarker).c_str() );
	sText += ssprintf( "Selection end:\n     %s\n",			m_NoteFieldEdit.m_fEndMarker==-1 ? "not set" : ssprintf("%.2f",m_NoteFieldEdit.m_fEndMarker).c_str() );
	sText += ssprintf( "Difficulty:\n     %s\n",			DifficultyToString( m_pNotes->GetDifficulty() ).c_str() );
	sText += ssprintf( "Description:\n     %s\n",			GAMESTATE->m_pCurNotes[PLAYER_1] ? GAMESTATE->m_pCurNotes[PLAYER_1]->GetDescription().c_str() : "no description" );
	sText += ssprintf( "Main title:\n     %s\n",			m_pSong->m_sMainTitle.c_str() );
	sText += ssprintf( "Tap Notes:\n     %d\n",				iNumTapNotes );
	sText += ssprintf( "Hold Notes:\n     %d\n",			iNumHoldNotes );
	sText += ssprintf( "Beat 0 Offset:\n     %.3f secs\n",	m_pSong->m_fBeat0OffsetInSeconds );
	sText += ssprintf( "Preview Start:\n     %.2f secs\n",	m_pSong->m_fMusicSampleStartSeconds );
	sText += ssprintf( "Preview Length:\n     %.2f secs\n",m_pSong->m_fMusicSampleLengthSeconds );

	m_textInfo.SetText( sText );
}

void ScreenEdit::DrawPrimitives()
{
//	m_rectRecordBack.Draw();

	switch( m_EditMode )
	{
	case MODE_EDITING:
		{
			m_BGAnimation.Draw();
			m_SnapDisplay.Draw();
			m_GrayArrowRowEdit.Draw();

			// HACK:  Make NoteFieldEdit draw using the trailing beat
			float fSongBeat = GAMESTATE->m_fSongBeat;	// save song beat
			GAMESTATE->m_fSongBeat = m_fTrailingBeat;	// put trailing beat in effect
			m_NoteFieldEdit.Draw();
			GAMESTATE->m_fSongBeat = fSongBeat;	// restore real song beat

			m_sprHelp.Draw();
			m_textHelp.Draw();
			m_sprInfo.Draw();
			m_textInfo.Draw();
			m_In.Draw();
			m_Out.Draw();
		}
		break;
	case MODE_RECORDING:
		m_BGAnimation.Draw();
		m_GrayArrowRowRecord.Draw();
		m_NoteFieldRecord.Draw();
		break;
	case MODE_PLAYING:
		m_BGAnimation.Draw();
		m_Player.Draw();
		break;
	default:
		ASSERT(0);
	}

	Screen::DrawPrimitives();
}

void ScreenEdit::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenEdit::Input()" );

	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	switch( m_EditMode )
	{
	case MODE_EDITING:		InputEdit( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_RECORDING:	InputRecord( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_PLAYING:		InputPlay( DeviceI, type, GameI, MenuI, StyleI );	break;
	default:	ASSERT(0);
	}
}

void ScreenEdit::InputEdit( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.device != DEVICE_KEYBOARD )
		return;

	if(type == IET_RELEASE)
	{
		switch( DeviceI.button ) {
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			shiftAnchor = -1;
			break;
		}
		return;
	}

	switch( DeviceI.button )
	{
	case SDLK_1:
	case SDLK_2:
	case SDLK_3:
	case SDLK_4:
	case SDLK_5:
	case SDLK_6:
	case SDLK_7:
	case SDLK_8:
	case SDLK_9:
	case SDLK_0:
		{
			if( type != IET_FIRST_PRESS )
				break;	// We only care about first presses

			const int iCol = DeviceI.button == SDLK_0? 9: DeviceI.button - SDLK_1;

			const float fSongBeat = GAMESTATE->m_fSongBeat;
			const int iSongIndex = BeatToNoteRow( fSongBeat );

			if( iCol >= m_NoteFieldEdit.GetNumTracks() )	// this button is not in the range of columns for this StyleDef
				break;

			/* XXX: easier to do with 4s */
			// check for to see if the user intended to remove a HoldNote
			bool bRemovedAHoldNote = false;
			for( int i=0; i<m_NoteFieldEdit.GetNumHoldNotes(); i++ )	// for each HoldNote
			{
				const HoldNote &hn = m_NoteFieldEdit.GetHoldNote(i);
				if( iCol == hn.iTrack  &&		// the notes correspond
					fSongBeat >= hn.fStartBeat  &&  fSongBeat <= hn.fEndBeat )	// the cursor lies within this HoldNote
				{
					m_NoteFieldEdit.RemoveHoldNote( i );
					bRemovedAHoldNote = true;
					break;	// stop iterating over all HoldNotes
				}
			}

			if( !bRemovedAHoldNote )
			{
				// We didn't remove a HoldNote, so the user wants to add or delete a TapNote
				if( m_NoteFieldEdit.GetTapNote(iCol, iSongIndex) == TAP_EMPTY )
					m_NoteFieldEdit.SetTapNote(iCol, iSongIndex, TAP_TAP );
				else
					m_NoteFieldEdit.SetTapNote(iCol, iSongIndex, TAP_EMPTY );
			}
		}
		break;
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		{
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_RCTRL)) )
			{
				float& fScrollSpeed = GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed;

				if( DeviceI.button == SDLK_UP )
				{
					if( fScrollSpeed == 4 )
					{
						fScrollSpeed = 2;
						m_soundMarker.Play();
					}
					else if( fScrollSpeed == 2 )
					{
						fScrollSpeed = 1;
						m_soundMarker.Play();
					}
					break;
				}
				else if( DeviceI.button == SDLK_DOWN )
				{
					if( fScrollSpeed == 2 )
					{
						fScrollSpeed = 4;
						m_soundMarker.Play();
					}
					else if( fScrollSpeed == 1 )
					{
						fScrollSpeed = 2;
						m_soundMarker.Play();
					}
					break;
				}
			}

			float fBeatsToMove=0.f;
			switch( DeviceI.button )
			{
			case SDLK_UP:
			case SDLK_DOWN:
				fBeatsToMove = NoteTypeToBeat( m_SnapDisplay.GetNoteType() );
				if( DeviceI.button == SDLK_UP )	
					fBeatsToMove *= -1;
			break;
			case SDLK_PAGEUP:
			case SDLK_PAGEDOWN:
				fBeatsToMove = BEATS_PER_MEASURE;
				if( DeviceI.button == SDLK_PAGEUP )	
					fBeatsToMove *= -1;
			}

			const float fStartBeat = GAMESTATE->m_fSongBeat;
			const float fEndBeat = GAMESTATE->m_fSongBeat + fBeatsToMove;

			// check to see if they're holding a button
			for( int col=0; col<m_NoteFieldEdit.GetNumTracks() && col<=10; col++ )
			{
				const DeviceInput di(DEVICE_KEYBOARD, SDLK_1+col);

				if( !INPUTFILTER->IsBeingPressed(di) )
					continue;

				// create a new hold note
				HoldNote newHN( col, min(fStartBeat, fEndBeat), max(fStartBeat, fEndBeat) );

				newHN.fStartBeat = max(newHN.fStartBeat, 0);
				newHN.fEndBeat = max(newHN.fEndBeat, 0);

				m_NoteFieldEdit.AddHoldNote( newHN );
			}

			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_LSHIFT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_RSHIFT)))
			{
				/* Shift is being held. 
				 *
				 * If this is the first time we've moved since shift was depressed,
				 * the old position (before this move) becomes the start pos: */

				if(shiftAnchor == -1)
					shiftAnchor = fStartBeat;
				
				if(fEndBeat == shiftAnchor)
				{
					/* We're back at the anchor, so we have nothing selected. */
					m_NoteFieldEdit.m_fBeginMarker = m_NoteFieldEdit.m_fEndMarker = -1;
				}
				else
				{
					m_NoteFieldEdit.m_fBeginMarker = shiftAnchor;
					m_NoteFieldEdit.m_fEndMarker = fEndBeat;
					if(m_NoteFieldEdit.m_fBeginMarker > m_NoteFieldEdit.m_fEndMarker)
						swap(m_NoteFieldEdit.m_fBeginMarker, m_NoteFieldEdit.m_fEndMarker);
				}
			}

			GAMESTATE->m_fSongBeat += fBeatsToMove;
			GAMESTATE->m_fSongBeat = max( GAMESTATE->m_fSongBeat, 0 );
			GAMESTATE->m_fSongBeat = froundf( GAMESTATE->m_fSongBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );
			m_soundChangeLine.Play();
		}
		break;
	case SDLK_HOME:
		GAMESTATE->m_fSongBeat = 0;
		m_soundChangeLine.Play();
		break;
	case SDLK_END:
		GAMESTATE->m_fSongBeat = m_NoteFieldEdit.GetLastBeat();
		m_soundChangeLine.Play();
		break;
	case SDLK_LEFT:
		m_SnapDisplay.PrevSnapMode();
		OnSnapModeChange();
		break;
	case SDLK_RIGHT:
		m_SnapDisplay.NextSnapMode();
		OnSnapModeChange();
		break;
	case SDLK_SPACE:
		if( m_NoteFieldEdit.m_fBeginMarker==-1 && m_NoteFieldEdit.m_fEndMarker==-1 )
		{
			// lay begin marker
			m_NoteFieldEdit.m_fBeginMarker = GAMESTATE->m_fSongBeat;
		}
		else if( m_NoteFieldEdit.m_fEndMarker==-1 )	// only begin marker is laid
		{
			if( GAMESTATE->m_fSongBeat == m_NoteFieldEdit.m_fBeginMarker )
			{
				m_NoteFieldEdit.m_fBeginMarker = -1;
			}
			else
			{
				m_NoteFieldEdit.m_fEndMarker = max( m_NoteFieldEdit.m_fBeginMarker, GAMESTATE->m_fSongBeat );
				m_NoteFieldEdit.m_fBeginMarker = min( m_NoteFieldEdit.m_fBeginMarker, GAMESTATE->m_fSongBeat );
			}
		}
		else	// both markers are laid
		{
			m_NoteFieldEdit.m_fBeginMarker = GAMESTATE->m_fSongBeat;
			m_NoteFieldEdit.m_fEndMarker = -1;
		}
		m_soundMarker.Play();
		break;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		{
			// update enabled/disabled in g_AreaMenu
			bool bAreaSelected = m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1;
			g_AreaMenu.rows[cut].enabled = bAreaSelected;
			g_AreaMenu.rows[copy].enabled = bAreaSelected;
			g_AreaMenu.rows[paste_at_current_beat].enabled = this->m_Clipboard.GetLastBeat() != 0;
			g_AreaMenu.rows[paste_at_begin_marker].enabled = this->m_Clipboard.GetLastBeat() != 0 && m_NoteFieldEdit.m_fBeginMarker!=-1;
			g_AreaMenu.rows[clear].enabled = bAreaSelected;
			g_AreaMenu.rows[quantize].enabled = bAreaSelected;
			g_AreaMenu.rows[turn].enabled = bAreaSelected;
			g_AreaMenu.rows[transform].enabled = bAreaSelected;
			g_AreaMenu.rows[alter].enabled = bAreaSelected;
			g_AreaMenu.rows[play].enabled = bAreaSelected;
			g_AreaMenu.rows[record].enabled = bAreaSelected;
			SCREENMAN->MiniMenu( &g_AreaMenu, SM_BackFromAreaMenu );
		}
		break;
	case SDLK_ESCAPE:
		SCREENMAN->MiniMenu( &g_MainMenu, SM_BackFromMainMenu );
		break;

	case SDLK_F1:
		SCREENMAN->MiniMenu( &g_KeyboardShortcuts, SM_None );
		break;
	case SDLK_F4:
		GAMESTATE->m_SongOptions.m_bAssistTick ^= 1;
		break;
	case SDLK_F7:
	case SDLK_F8:
		{
			float fBPM = m_pSong->GetBPMAtBeat( GAMESTATE->m_fSongBeat );
			float fDeltaBPM;
			switch( DeviceI.button )
			{
			case SDLK_F7:	fDeltaBPM = - 0.020f;		break;
			case SDLK_F8:	fDeltaBPM = + 0.020f;		break;
			default:	ASSERT(0);						return;
			}
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LALT)) )
				fDeltaBPM /= 2; /* .010 */
			else switch( type )
			{
			case IET_SLOW_REPEAT:	fDeltaBPM *= 10;	break;
			case IET_FAST_REPEAT:	fDeltaBPM *= 40;	break;
			}
			float fNewBPM = fBPM + fDeltaBPM;

			unsigned i;
			for( i=0; i<m_pSong->m_BPMSegments.size(); i++ )
				if( m_pSong->m_BPMSegments[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
					break;

			if( i == m_pSong->m_BPMSegments.size() )	// there is no BPMSegment at the current beat
			{
				// create a new BPMSegment
				m_pSong->AddBPMSegment( BPMSegment(GAMESTATE->m_fSongBeat, fNewBPM) );
			}
			else	// BPMSegment being modified is m_BPMSegments[i]
			{
				if( i > 0  &&  fabsf(m_pSong->m_BPMSegments[i-1].m_fBPM - fNewBPM) < 0.009f )
					m_pSong->m_BPMSegments.erase( m_pSong->m_BPMSegments.begin()+i,
												  m_pSong->m_BPMSegments.begin()+i+1);
				else
					m_pSong->m_BPMSegments[i].m_fBPM = fNewBPM;
			}
		}
		break;
	case SDLK_F9:
	case SDLK_F10:
		{
			float fStopDelta;
			switch( DeviceI.button )
			{
			case SDLK_F9:	fStopDelta = -0.02f;		break;
			case SDLK_F10:	fStopDelta = +0.02f;		break;
			default:	ASSERT(0);						return;
			}
			switch( type )
			{
			case IET_SLOW_REPEAT:	fStopDelta *= 10;	break;
			case IET_FAST_REPEAT:	fStopDelta *= 40;	break;
			}

			unsigned i;
			for( i=0; i<m_pSong->m_StopSegments.size(); i++ )
			{
				if( m_pSong->m_StopSegments[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
					break;
			}

			if( i == m_pSong->m_StopSegments.size() )	// there is no BPMSegment at the current beat
			{
				// create a new StopSegment
				if( fStopDelta > 0 )
					m_pSong->AddStopSegment( StopSegment(GAMESTATE->m_fSongBeat, fStopDelta) );
			}
			else	// StopSegment being modified is m_StopSegments[i]
			{
				m_pSong->m_StopSegments[i].m_fStopSeconds += fStopDelta;
				if( m_pSong->m_StopSegments[i].m_fStopSeconds <= 0 )
					m_pSong->m_StopSegments.erase( m_pSong->m_StopSegments.begin()+i,
													  m_pSong->m_StopSegments.begin()+i+1);
			}
		}
		break;
	case SDLK_F11:
	case SDLK_F12:
		{
			float fOffsetDelta;
			switch( DeviceI.button )
			{
			case SDLK_F11:	fOffsetDelta = -0.02f;		break;
			case SDLK_F12:	fOffsetDelta = +0.02f;		break;
			default:	ASSERT(0);						return;
			}
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LALT)) )
				fOffsetDelta /= 20; /* 1ms */
			else switch( type )
			{
			case IET_SLOW_REPEAT:	fOffsetDelta *= 10;	break;
			case IET_FAST_REPEAT:	fOffsetDelta *= 40;	break;
			}

			m_pSong->m_fBeat0OffsetInSeconds += fOffsetDelta;
		}
		break;
	case SDLK_LEFTBRACKET:
	case SDLK_RIGHTBRACKET:
		{
			float fDelta;
			switch( DeviceI.button )
			{
			case SDLK_LEFTBRACKET:		fDelta = -0.02f;	break;
			case SDLK_RIGHTBRACKET:		fDelta = +0.02f;	break;
			default:	ASSERT(0);						return;
			}
			switch( type )
			{
			case IET_SLOW_REPEAT:	fDelta *= 10;	break;
			case IET_FAST_REPEAT:	fDelta *= 40;	break;
			}

			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_LSHIFT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, SDLK_RSHIFT)))
			{
				m_pSong->m_fMusicSampleLengthSeconds += fDelta;
				m_pSong->m_fMusicSampleLengthSeconds = max(m_pSong->m_fMusicSampleLengthSeconds,0);
			} else {
				m_pSong->m_fMusicSampleStartSeconds += fDelta;
				m_pSong->m_fMusicSampleStartSeconds = max(m_pSong->m_fMusicSampleStartSeconds,0);
			}
		}
		break;
	case SDLK_m:
		PlayPreviewMusic();
		break;
	case SDLK_b:
		HandleMainMenuChoice( edit_bg_change, NULL );
		break;
	case SDLK_p:
		{
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_RCTRL)) )
				HandleMainMenuChoice( play_whole_song, NULL );
			else if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_LSHIFT)) ||
					 INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_RSHIFT)) )
				HandleMainMenuChoice( play_current_beat_to_end, NULL );
			else
				if( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 )
					HandleAreaMenuChoice( play, NULL );
		}
		break;
	case SDLK_r:
		{
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,SDLK_RCTRL)) )
				if( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 )
					HandleAreaMenuChoice( record, NULL );
		}
		break;
	case SDLK_INSERT:
		HandleAreaMenuChoice( insert_and_shift, NULL );
		break;
	case SDLK_DELETE:
		HandleAreaMenuChoice( delete_and_shift, NULL );
		break;
	}

	/* Make sure the displayed time is up-to-date after possibly changing something,
	 * so it doesn't feel lagged. */
	UpdateTextInfo();
}

void ScreenEdit::InputRecord( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.device == DEVICE_KEYBOARD  &&  DeviceI.button == SDLK_ESCAPE )
	{
		TransitionFromRecordToEdit();
		return;
	}	

	if( StyleI.player != PLAYER_1 )
		return;		// ignore

	const int iCol = StyleI.col;

	switch( type )
	{
	case IET_FIRST_PRESS:
		{
			// Add a tap

			float fBeat = GAMESTATE->m_fSongBeat;
			fBeat = froundf( fBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );
			
			const int iRow = BeatToNoteRow( fBeat );
			if( iRow < 0 )
				break;

			m_NoteFieldRecord.SetTapNote(iCol, iRow, TAP_TAP);
			m_GrayArrowRowRecord.Step( iCol );
		}
		break;
	case IET_SLOW_REPEAT:
	case IET_FAST_REPEAT:
	case IET_RELEASE:
		// don't add or extend holds here
		break;
	}
}

void ScreenEdit::InputPlay( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return;

	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case SDLK_ESCAPE:
			TransitionToEdit();
			break;
		case SDLK_F4:
			GAMESTATE->m_SongOptions.m_bAssistTick ^= 1;
			break;
		case SDLK_F8:
			{
				PREFSMAN->m_bAutoPlay = !PREFSMAN->m_bAutoPlay;
				for( int p=0; p<NUM_PLAYERS; p++ )
					if( GAMESTATE->IsHumanPlayer(p) )
						GAMESTATE->m_PlayerController[p] = PREFSMAN->m_bAutoPlay?PC_AUTOPLAY:PC_HUMAN;
			}
			break;
		case SDLK_F11:
		case SDLK_F12:
			{
				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case SDLK_F11:	fOffsetDelta = -0.020f;		break;
				case SDLK_F12:	fOffsetDelta = +0.020f;		break;
				default:	ASSERT(0);						return;
				}

				if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RALT)) ||
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LALT)) )
					fOffsetDelta /= 20; /* 1ms */
				else switch( type )
				{
				case IET_SLOW_REPEAT:	fOffsetDelta *= 10;	break;
				case IET_FAST_REPEAT:	fOffsetDelta *= 40;	break;
				}

				m_pSong->m_fBeat0OffsetInSeconds += fOffsetDelta;
			}
		break;
		}
	}

	switch( StyleI.player )
	{
		case PLAYER_1:	
			if( !PREFSMAN->m_bAutoPlay )
				m_Player.Step( StyleI.col ); 
			return;
	}

}


/* Switch to editing. */
void ScreenEdit::TransitionToEdit()
{
	m_EditMode = MODE_EDITING;
	m_soundMusic.StopPlaying();
	m_rectRecordBack.StopTweening();
	m_rectRecordBack.BeginTweening( 0.5f );
	m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0) );

	/* Make sure we're snapped. */
	GAMESTATE->m_fSongBeat = froundf( GAMESTATE->m_fSongBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );

	/* Playing and recording have lead-ins, which may start before beat 0;
	 * make sure we don't stay there if we escaped out early. */
	GAMESTATE->m_fSongBeat = max( GAMESTATE->m_fSongBeat, 0 );
}

void ScreenEdit::TransitionFromRecordToEdit()
{
	TransitionToEdit();

	int iNoteIndexBegin = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
	int iNoteIndexEnd = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );

	// delete old TapNotes in the range
	m_NoteFieldEdit.ClearRange( iNoteIndexBegin, iNoteIndexEnd );

	m_NoteFieldEdit.CopyRange( &m_NoteFieldRecord, iNoteIndexBegin, iNoteIndexEnd, iNoteIndexBegin );
}


void ScreenEdit::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		// Reload song from disk to discard changes.
		GAMESTATE->m_pCurSong->LoadFromSongDir( GAMESTATE->m_pCurSong->GetSongDir() );
		SCREENMAN->SetNewScreen( "ScreenEditMenu" );
		break;
	case SM_BackFromMainMenu:
		HandleMainMenuChoice( (MainMenuChoice)ScreenMiniMenu::s_iLastLine, ScreenMiniMenu::s_iLastAnswers );
		break;
	case SM_BackFromAreaMenu:
		HandleAreaMenuChoice( (AreaMenuChoice)ScreenMiniMenu::s_iLastLine, ScreenMiniMenu::s_iLastAnswers );
		break;
	case SM_BackFromEditNotesStatistics:
		HandleEditNotesStatisticsChoice( (EditNotesStatisticsChoice)ScreenMiniMenu::s_iLastLine, ScreenMiniMenu::s_iLastAnswers );
		break;
	case SM_BackFromEditSongInfo:
		HandleEditSongInfoChoice( (EditSongInfoChoice)ScreenMiniMenu::s_iLastLine, ScreenMiniMenu::s_iLastAnswers );
		break;
	case SM_BackFromBGChange:
		HandleBGChangeChoice( (BGChangeChoice)ScreenMiniMenu::s_iLastLine, ScreenMiniMenu::s_iLastAnswers );
		break;
	case SM_RegainingFocus:
		// coming back from PlayerOptions or SongOptions
		m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
		break;
	}
}

void ScreenEdit::OnSnapModeChange()
{
	m_soundChangeSnap.Play();
			
	NoteType nt = m_SnapDisplay.GetNoteType();
	int iStepIndex = BeatToNoteRow( GAMESTATE->m_fSongBeat );
	int iElementsPerNoteType = BeatToNoteRow( NoteTypeToBeat(nt) );
	int iStepIndexHangover = iStepIndex % iElementsPerNoteType;
	GAMESTATE->m_fSongBeat -= NoteRowToBeat( iStepIndexHangover );
}



// Helper function for below

// Begin helper functions for InputEdit


void ChangeDescription( CString sNew )
{
	Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
	pNotes->SetDescription(sNew);
}

void ChangeMainTitle( CString sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sMainTitle = sNew;
}

void ChangeSubTitle( CString sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sSubTitle = sNew;
}

void ChangeArtist( CString sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sArtist = sNew;
}

void ChangeCredit( CString sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sCredit = sNew;
}

void ChangeMainTitleTranslit( CString sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sMainTitleTranslit = sNew;
}

void ChangeSubTitleTranslit( CString sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sSubTitleTranslit = sNew;
}

void ChangeArtistTranslit( CString sNew )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	pSong->m_sArtistTranslit = sNew;
}

// End helper functions




void ScreenEdit::HandleMainMenuChoice( MainMenuChoice c, int* iAnswers )
{
	switch( c )
	{
		case edit_notes_statistics:
			{
				Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
				float fMusicSeconds = m_soundMusic.GetLengthSeconds();

				g_EditNotesStatistics.rows[difficulty].defaultChoice = pNotes->GetDifficulty();
				g_EditNotesStatistics.rows[meter].defaultChoice = pNotes->GetMeter()-1;
				g_EditNotesStatistics.rows[description].choices.resize(1);	g_EditNotesStatistics.rows[description].choices[0] = pNotes->GetDescription();
				g_EditNotesStatistics.rows[tap_notes].choices.resize(1);	g_EditNotesStatistics.rows[tap_notes].choices[0] = ssprintf("%d", m_NoteFieldEdit.GetNumTapNotes());
				g_EditNotesStatistics.rows[hold_notes].choices.resize(1);	g_EditNotesStatistics.rows[hold_notes].choices[0] = ssprintf("%d", m_NoteFieldEdit.GetNumHoldNotes());
				g_EditNotesStatistics.rows[stream].choices.resize(1);		g_EditNotesStatistics.rows[stream].choices[0] = ssprintf("%f", NoteDataUtil::GetStreamRadarValue(m_NoteFieldEdit,fMusicSeconds));
				g_EditNotesStatistics.rows[voltage].choices.resize(1);		g_EditNotesStatistics.rows[voltage].choices[0] = ssprintf("%f", NoteDataUtil::GetVoltageRadarValue(m_NoteFieldEdit,fMusicSeconds));
				g_EditNotesStatistics.rows[air].choices.resize(1);			g_EditNotesStatistics.rows[air].choices[0] = ssprintf("%f", NoteDataUtil::GetAirRadarValue(m_NoteFieldEdit,fMusicSeconds));
				g_EditNotesStatistics.rows[freeze].choices.resize(1);		g_EditNotesStatistics.rows[freeze].choices[0] = ssprintf("%f", NoteDataUtil::GetFreezeRadarValue(m_NoteFieldEdit,fMusicSeconds));
				g_EditNotesStatistics.rows[chaos].choices.resize(1);		g_EditNotesStatistics.rows[chaos].choices[0] = ssprintf("%f", NoteDataUtil::GetChaosRadarValue(m_NoteFieldEdit,fMusicSeconds));
				SCREENMAN->MiniMenu( &g_EditNotesStatistics, SM_BackFromEditNotesStatistics );
			}
			break;
		case play_whole_song:
			{
				m_NoteFieldEdit.m_fBeginMarker = 0;
				m_NoteFieldEdit.m_fEndMarker = m_NoteFieldEdit.GetLastBeat();
				HandleAreaMenuChoice( play, NULL );
			}
			break;
		case play_current_beat_to_end:
			{
				m_NoteFieldEdit.m_fBeginMarker = GAMESTATE->m_fSongBeat;
				m_NoteFieldEdit.m_fEndMarker = m_NoteFieldEdit.GetLastBeat();
				HandleAreaMenuChoice( play, NULL );
			}
			break;
		case save:
			{
				// copy edit into current Notes
				Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
				ASSERT( pNotes );

				pNotes->SetNoteData( &m_NoteFieldEdit );
				GAMESTATE->m_pCurSong->Save();

				SCREENMAN->SystemMessage( "Saved as SM and DWI." );
				SOUNDMAN->PlayOnce( THEME->GetPathToS("ScreenEdit save") );
			}
			break;
		case player_options:
			SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions" );
			break;
		case song_options:
			SCREENMAN->AddNewScreenToTop( "ScreenSongOptions" );
			break;
		case edit_song_info:
			{
				Song* pSong = GAMESTATE->m_pCurSong;
				g_EditSongInfo.rows[main_title].choices.resize(1);					g_EditSongInfo.rows[main_title].choices[0] = pSong->m_sMainTitle;
				g_EditSongInfo.rows[sub_title].choices.resize(1);					g_EditSongInfo.rows[sub_title].choices[0] = pSong->m_sSubTitle;
				g_EditSongInfo.rows[artist].choices.resize(1);						g_EditSongInfo.rows[artist].choices[0] = pSong->m_sArtist;
				g_EditSongInfo.rows[credit].choices.resize(1);						g_EditSongInfo.rows[credit].choices[0] = pSong->m_sCredit;
				g_EditSongInfo.rows[main_title_transliteration].choices.resize(1);	g_EditSongInfo.rows[main_title_transliteration].choices[0] = pSong->m_sMainTitleTranslit;
				g_EditSongInfo.rows[sub_title_transliteration].choices.resize(1);	g_EditSongInfo.rows[sub_title_transliteration].choices[0] = pSong->m_sSubTitleTranslit;
				g_EditSongInfo.rows[artist_transliteration].choices.resize(1);		g_EditSongInfo.rows[artist_transliteration].choices[0] = pSong->m_sArtistTranslit;

				SCREENMAN->MiniMenu( &g_EditSongInfo, SM_BackFromEditSongInfo );
			}
			break;
		case edit_bpm:
			break;
		case edit_stop:
			break;
		case edit_bg_change:
			{
				//
				// Fill in option names
				//

				// m_pSong->GetSongDir() has trailing slash
				g_BGChange.rows[add_song_bganimation].choices.clear();
				GetDirListing( m_pSong->GetSongDir()+"*", g_BGChange.rows[add_song_bganimation].choices, true );

				g_BGChange.rows[add_song_movie].choices.clear();
				GetDirListing( m_pSong->GetSongDir()+"*.avi", g_BGChange.rows[add_song_movie].choices, false );
				GetDirListing( m_pSong->GetSongDir()+"*.mpg", g_BGChange.rows[add_song_movie].choices, false );
				GetDirListing( m_pSong->GetSongDir()+"*.mpeg", g_BGChange.rows[add_song_movie].choices, false );

				g_BGChange.rows[add_song_still].choices.clear();
				GetDirListing( m_pSong->GetSongDir()+"*.png", g_BGChange.rows[add_song_still].choices, false );
				GetDirListing( m_pSong->GetSongDir()+"*.jpg", g_BGChange.rows[add_song_still].choices, false );
				GetDirListing( m_pSong->GetSongDir()+"*.gif", g_BGChange.rows[add_song_still].choices, false );
				GetDirListing( m_pSong->GetSongDir()+"*.bmp", g_BGChange.rows[add_song_still].choices, false );

				g_BGChange.rows[add_global_random_movie].choices.clear();
				GetDirListing( RANDOMMOVIES_DIR+"*.avi", g_BGChange.rows[add_global_random_movie].choices, false );
				GetDirListing( RANDOMMOVIES_DIR+"*.mpg", g_BGChange.rows[add_global_random_movie].choices, false );
				GetDirListing( RANDOMMOVIES_DIR+"*.mpeg", g_BGChange.rows[add_global_random_movie].choices, false );

				g_BGChange.rows[add_global_bganimation].choices.clear();
				GetDirListing( BG_ANIMS_DIR+"*", g_BGChange.rows[add_global_bganimation].choices, true );

				g_BGChange.rows[add_global_visualization].choices.clear();
				GetDirListing( VISUALIZATIONS_DIR+"*.avi", g_BGChange.rows[add_global_visualization].choices, false );
				GetDirListing( VISUALIZATIONS_DIR+"*.mpg", g_BGChange.rows[add_global_visualization].choices, false );
				GetDirListing( VISUALIZATIONS_DIR+"*.mpeg", g_BGChange.rows[add_global_visualization].choices, false );


				//
				// Fill in line enabled/disabled
				//
				bool bAlreadyBGChangeHere = false;
				for( unsigned i=0; i<m_pSong->m_BackgroundChanges.size(); i++ )
					if( m_pSong->m_BackgroundChanges[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
						bAlreadyBGChangeHere = true;

				g_BGChange.rows[add_random].enabled = true;
				g_BGChange.rows[add_song_bganimation].enabled = g_BGChange.rows[add_song_bganimation].choices.size() > 0;
				g_BGChange.rows[add_song_movie].enabled = g_BGChange.rows[add_song_movie].choices.size() > 0;
				g_BGChange.rows[add_song_still].enabled = g_BGChange.rows[add_song_still].choices.size() > 0;
				g_BGChange.rows[add_global_random_movie].enabled = g_BGChange.rows[add_global_random_movie].choices.size() > 0;
				g_BGChange.rows[add_global_bganimation].enabled = g_BGChange.rows[add_global_bganimation].choices.size() > 0;
				g_BGChange.rows[add_global_visualization].enabled = g_BGChange.rows[add_global_visualization].choices.size() > 0;
				g_BGChange.rows[delete_change].enabled = bAlreadyBGChangeHere;
					


				SCREENMAN->MiniMenu( &g_BGChange, SM_BackFromBGChange );
			}
			break;
		case play_preview_music:
			PlayPreviewMusic();
			break;
		case exit:
			m_Out.StartTransitioning( SM_GoToNextScreen );
			break;
		default:
			ASSERT(0);
	};
}

void ScreenEdit::HandleAreaMenuChoice( AreaMenuChoice c, int* iAnswers )
{
	switch( c )
	{
		case cut:
			{
				HandleAreaMenuChoice( copy, NULL );
				HandleAreaMenuChoice( clear, NULL );
			}
			break;
		case copy:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );
				int iFirstRow = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
				int iLastRow  = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );
				m_Clipboard.ClearAll();
				m_Clipboard.CopyRange( &m_NoteFieldEdit, iFirstRow, iLastRow );
			}
			break;
		case paste_at_current_beat:
			{
				int iSrcFirstRow = 0;
				int iSrcLastRow  = BeatToNoteRow( m_Clipboard.GetLastBeat() );
				int iDestFirstRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
				m_NoteFieldEdit.CopyRange( &m_Clipboard, iSrcFirstRow, iSrcLastRow, iDestFirstRow );
			}
			break;
		case paste_at_begin_marker:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 );
				int iSrcFirstRow = 0;
				int iSrcLastRow  = BeatToNoteRow( m_Clipboard.GetLastBeat() );
				int iDestFirstRow = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
				m_NoteFieldEdit.CopyRange( &m_Clipboard, iSrcFirstRow, iSrcLastRow, iDestFirstRow );
			}
			break;
		case clear:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );
				int iFirstRow = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
				int iLastRow  = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );
				m_NoteFieldEdit.ClearRange( iFirstRow, iLastRow );
			}
			break;
		case quantize:
			{
				NoteType nt = (NoteType)iAnswers[c];
				NoteDataUtil::SnapToNearestNoteType( m_NoteFieldEdit, nt, nt, m_NoteFieldEdit.m_fBeginMarker, m_NoteFieldEdit.m_fEndMarker );
			}
			break;
		case turn:
			{
				HandleAreaMenuChoice( cut, NULL );
				
				TurnType tt = (TurnType)iAnswers[c];
				switch( tt )
				{
				case left:			NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::left );			break;
				case right:			NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::right );			break;
				case mirror:		NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::mirror );		break;
				case shuffle:		NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::shuffle );		break;
				case super_shuffle:	NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::super_shuffle );	break;
				default:		ASSERT(0);
				}

				HandleAreaMenuChoice( paste_at_begin_marker, NULL );
			}
			break;
		case transform:
			{
				HandleAreaMenuChoice( cut, NULL );
				
				TransformType tt = (TransformType)iAnswers[c];
				switch( tt )
				{
				case little:		NoteDataUtil::Little( m_Clipboard );	break;
				case wide:			NoteDataUtil::Wide( m_Clipboard );		break;
				case big:			NoteDataUtil::Big( m_Clipboard );		break;
				case quick:			NoteDataUtil::Quick( m_Clipboard );		break;
				case skippy:		NoteDataUtil::Skippy( m_Clipboard );	break;
				default:		ASSERT(0);
				}

				HandleAreaMenuChoice( paste_at_begin_marker, NULL );
			}
			break;
		case alter:
			{
				HandleAreaMenuChoice( cut, NULL );
				
				AlterType at = (AlterType)iAnswers[c];
				switch( at )
				{
				case backwards:				NoteDataUtil::Backwards( m_Clipboard );			break;
				case swap_sides:			NoteDataUtil::SwapSides( m_Clipboard );			break;
				case copy_left_to_right:	NoteDataUtil::CopyLeftToRight( m_Clipboard );	break;
				case copy_right_to_left:	NoteDataUtil::CopyRightToLeft( m_Clipboard );	break;
				case clear_left:			NoteDataUtil::ClearLeft( m_Clipboard );			break;
				case clear_right:			NoteDataUtil::ClearRight( m_Clipboard );		break;
				case collapse_to_one:		NoteDataUtil::CollapseToOne( m_Clipboard );		break;
				case shift_left:			NoteDataUtil::ShiftLeft( m_Clipboard );			break;
				case shift_right:			NoteDataUtil::ShiftRight( m_Clipboard );		break;
				default:		ASSERT(0);
				}

				HandleAreaMenuChoice( paste_at_begin_marker, NULL );
			}
			break;
		case play:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );

				SOUNDMAN->PlayMusic("");

				m_EditMode = MODE_PLAYING;

				m_Player.Load( PLAYER_1, (NoteData*)&m_NoteFieldEdit, NULL, NULL, NULL, NULL );
				GAMESTATE->m_PlayerController[PLAYER_1] = PREFSMAN->m_bAutoPlay?PC_AUTOPLAY:PC_HUMAN;

				m_rectRecordBack.StopTweening();
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0.8f) );

				GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fBeginMarker - 4;	// give a 1 measure lead-in
				float fStartSeconds = m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat) ;
				LOG->Trace( "Starting playback at %f", fStartSeconds );
				m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
				m_soundMusic.SetPositionSeconds( fStartSeconds );
				m_soundMusic.StartPlaying();
			}
			break;
		case record:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );

				SOUNDMAN->PlayMusic("");

				m_EditMode = MODE_RECORDING;

				// initialize m_NoteFieldRecord
				m_NoteFieldRecord.ClearAll();
				m_NoteFieldRecord.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );
				m_NoteFieldRecord.m_fBeginMarker = m_NoteFieldEdit.m_fBeginMarker;
				m_NoteFieldRecord.m_fEndMarker = m_NoteFieldEdit.m_fEndMarker;

				m_rectRecordBack.StopTweening();
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0.8f) );

				GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fBeginMarker - 4;	// give a 1 measure lead-in
				float fStartSeconds = m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat);
				LOG->Trace( "Starting playback at %f", fStartSeconds );
				m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
				m_soundMusic.SetPositionSeconds( fStartSeconds );
				m_soundMusic.StartPlaying();
			}
			break;
		case insert_and_shift:
			{
				NoteData temp;
				temp.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );
				int iTakeFromRow=0;
				int iPasteAtRow;

				iTakeFromRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
				iPasteAtRow = BeatToNoteRow( GAMESTATE->m_fSongBeat+1 );

				temp.CopyRange( &m_NoteFieldEdit, iTakeFromRow, m_NoteFieldEdit.GetLastRow() );
				m_NoteFieldEdit.ClearRange( min(iTakeFromRow,iPasteAtRow), m_NoteFieldEdit.GetLastRow()  );
				m_NoteFieldEdit.CopyRange( &temp, 0, temp.GetLastRow(), iPasteAtRow );
			}
			break;
		case delete_and_shift:
			{
				NoteData temp;
				temp.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );
				int iTakeFromRow=0;
				int iPasteAtRow;

				iTakeFromRow = BeatToNoteRow( GAMESTATE->m_fSongBeat+1 );
				iPasteAtRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );

				temp.CopyRange( &m_NoteFieldEdit, iTakeFromRow, m_NoteFieldEdit.GetLastRow() );
				m_NoteFieldEdit.ClearRange( min(iTakeFromRow,iPasteAtRow), m_NoteFieldEdit.GetLastRow()  );
				m_NoteFieldEdit.CopyRange( &temp, 0, temp.GetLastRow(), iPasteAtRow );		
			}
			break;
		default:
			ASSERT(0);
	};

}

void ScreenEdit::HandleEditNotesStatisticsChoice( EditNotesStatisticsChoice c, int* iAnswers )
{
	Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
	Difficulty dc = (Difficulty)iAnswers[difficulty];
	pNotes->SetDifficulty( dc );
	int iMeter = iAnswers[meter]+1;
	pNotes->SetMeter( iMeter );

	switch( c )
	{
	case description:
		SCREENMAN->TextEntry( SM_None, "Edit notes description.\nPress Enter to confirm,\nEscape to cancel.", m_pNotes->GetDescription(), ChangeDescription, NULL );
		break;
	}
}

void ScreenEdit::HandleEditSongInfoChoice( EditSongInfoChoice c, int* iAnswers )
{
	Song* pSong = GAMESTATE->m_pCurSong;

	switch( c )
	{
	case main_title:
		SCREENMAN->TextEntry( SM_None, "Edit main title.\nPress Enter to confirm,\nEscape to cancel.", pSong->m_sMainTitle, ChangeMainTitle, NULL );
		break;
	case sub_title:
		SCREENMAN->TextEntry( SM_None, "Edit sub title.\nPress Enter to confirm,\nEscape to cancel.", pSong->m_sSubTitle, ChangeSubTitle, NULL );
		break;
	case artist:
		SCREENMAN->TextEntry( SM_None, "Edit artist.\nPress Enter to confirm,\nEscape to cancel.", pSong->m_sArtist, ChangeArtist, NULL );
		break;
	case credit:
		SCREENMAN->TextEntry( SM_None, "Edit credit.\nPress Enter to confirm,\nEscape to cancel.", pSong->m_sCredit, ChangeCredit, NULL );
		break;
	case main_title_transliteration:
		SCREENMAN->TextEntry( SM_None, "Edit main title transliteration.\nPress Enter to confirm,\nEscape to cancel.", pSong->m_sMainTitleTranslit, ChangeMainTitleTranslit, NULL );
		break;
	case sub_title_transliteration:
		SCREENMAN->TextEntry( SM_None, "Edit sub title transliteration.\nPress Enter to confirm,\nEscape to cancel.", pSong->m_sSubTitleTranslit, ChangeSubTitleTranslit, NULL );
		break;
	case artist_transliteration:
		SCREENMAN->TextEntry( SM_None, "Edit artist transliteration.\nPress Enter to confirm,\nEscape to cancel.", pSong->m_sArtistTranslit, ChangeArtistTranslit, NULL );
		break;
	default:
		ASSERT(0);
	};
}

void ScreenEdit::HandleBGChangeChoice( BGChangeChoice c, int* iAnswers )
{
	BackgroundChange change;
	change.m_fStartBeat = GAMESTATE->m_fSongBeat;

	switch( c )
	{
	case add_random:
		change.m_sBGName = "-random-";
		break;
	case add_song_bganimation:
	case add_song_movie:
	case add_song_still:
	case add_global_random_movie:
	case add_global_bganimation:
	case add_global_visualization:
		change.m_sBGName = g_BGChange.rows[c].choices[iAnswers[c]];
		break;
	case delete_change:
		change.m_sBGName = "";
		break;
	default:
		SOUNDMAN->PlayOnce( THEME->GetPathToS("Common invalid") );
	};

	change.m_fRate = (float)atof( g_BGChange.rows[rate].choices[iAnswers[rate]] )/100.f;
	change.m_bFadeLast = !!iAnswers[fade_last];
	change.m_bRewindMovie = !!iAnswers[rewind_movie];
	change.m_bLoop = !!iAnswers[loop];

	unsigned i;
	for( i=0; i<m_pSong->m_BackgroundChanges.size(); i++ )
		if( m_pSong->m_BackgroundChanges[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
			break;

	if( i != m_pSong->m_BackgroundChanges.size() )	// there is already a BGChange here
		m_pSong->m_BackgroundChanges.erase( m_pSong->m_BackgroundChanges.begin()+i,
										  m_pSong->m_BackgroundChanges.begin()+i+1);

	// create a new BGChange
	if( change.m_sBGName != "" )
		m_pSong->AddBackgroundChange( change );
}
