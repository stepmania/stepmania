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

#include <utility>


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

CachedThemeMetric	 TICK_EARLY_SECONDS		("ScreenGameplay","TickEarlySeconds");


const ScreenMessage SM_BackFromMainMenu				= (ScreenMessage)(SM_User+1);
const ScreenMessage SM_BackFromAreaMenu				= (ScreenMessage)(SM_User+2);
const ScreenMessage SM_BackFromEditNotesStatistics	= (ScreenMessage)(SM_User+3);
const ScreenMessage SM_BackFromEditOptions			= (ScreenMessage)(SM_User+4);
const ScreenMessage SM_BackFromEditSongInfo			= (ScreenMessage)(SM_User+5);


const CString HELP_TEXT = 
	"Up/Down:\n     change beat\n"
	"Left/Right:\n     change snap\n"
	"PgUp/PgDn:\n     jump measure\n"
	"Home/End:\n     jump to first/\n     last beat\n"
	"Number keys:\n     add/remove\n     tap note\n"
	"Create hold note:\n     Hold a number\n     while moving\n     Up or Down\n"
	"F7/F8:\n     Dec/inc BPM\n     at cur beat\n"
	"F9/F10:\n     Dec/inc stop\n     at cur beat\n"
	"F11/F12:\n     Dec/inc music\n     offset\n"
	"[ and ]:\n     Dec/inc sample\n     music start\n"
	"{ and }:\n     Dec/inc sample\n     music length\n";


MiniMenuDefinition g_MainMenu =
{
	"Main Menu",
	ScreenEdit::NUM_MAIN_MENU_CHOICES,
	{
		{ "Edit Notes Statistics",	true, 1, 0, {""} },
		{ "Play Whole Song",		true, 1, 0, {""} },
		{ "Save",					true, 1, 0, {""} },
		{ "Editor Options",			true, 1, 0, {""} },
		{ "Player Options",			true, 1, 0, {""} },
		{ "Song Options",			true, 1, 0, {""} },
		{ "Edit Song Info",			true, 1, 0, {""} },
		{ "Add/Edit BPM Change",	true, 1, 0, {""} },
		{ "Add/Edit Stop",			true, 1, 0, {""} },
		{ "Add/Edit BG Change",		true, 1, 0, {""} },
		{ "Play preview music",		true, 1, 0, {""} },
		{ "Exit",					true, 1, 0, {""} },
	}
};

MiniMenuDefinition g_AreaMenu =
{
	"Area Menu",
	ScreenEdit::NUM_AREA_MENU_CHOICES,
	{
		{ "Cut",				true, 1, 0, {""} },
		{ "Copy",				true, 1, 0, {""} },
		{ "Paste",				true, 1, 0, {""} },
		{ "Clear",				true, 1, 0, {""} },
		{ "Quantize",			true, NUM_NOTE_TYPES, 0, {"4TH","8TH","12TH","16TH","24TH","32ND"} },
		{ "Transform",			true, ScreenEdit::NUM_TRANSFORM_TYPES, 0, {"Little","Big","Left","Right","Mirror","Shuffle","Super Shuffle","Backwards","Swap Sides"} },
		{ "Play selection",		true, 1, 0, {""} },
		{ "Record in selection",true, 1, 0, {""} },
		{ "Insert blank beat and shift down", true, 1, 0, {""} },
		{ "Delete blank beat and shift up", true, 1, 0, {""} },
	}
};


MiniMenuDefinition g_EditNotesStatistics =
{
	"Statistics",
	ScreenEdit::NUM_EDIT_NOTES_STATISTICS_CHOICES,
	{
		{ "Difficulty",	true, 5, 0, {"BEGINNER","EASY","MEDIUM","HARD","CHALLENGE"} },
		{ "Meter",		true, 11, 0, {"1","2","3","4","5","6","7","8","9","10","11"} },
		{ "Description",true, 1, 0, {""} },
		{ "Tap Notes",	false, 1, 0, {""} },
		{ "Hold Notes",	false, 1, 0, {""} },
		{ "Stream",		false, 1, 0, {""} },
		{ "Voltage",	false, 1, 0, {""} },
		{ "Air",		false, 1, 0, {""} },
		{ "Freeze",		false, 1, 0, {""} },
		{ "Chaos",		false, 1, 0, {""} },
	}
};


MiniMenuDefinition g_EditOptions =
{
	"Edit Options",
	ScreenEdit::NUM_EDIT_OPTIONS_CHOICES,
	{
		{ "Zoom",				true, 3, 0, {"1X","2X","4X"} },
		{ "Max Notes Per Row",	true, 4, 0, {"NO LIMIT","2","3","4"} },
		{ "After Note Add",		true, 2, 0, {"STAY","NEXT LINE"} },
	}
};

MiniMenuDefinition g_EditSongInfo =
{
	"Edit Song Info",
	ScreenEdit::NUM_EDIT_SONG_INFO_CHOICES,
	{
		{ "Main title",					true, 1, 0, {""} },
		{ "Sub title",					true, 1, 0, {""} },
		{ "Artist",						true, 1, 0, {""} },
		{ "Main title transliteration",	true, 1, 0, {""} },
		{ "Sub title transliteration",	true, 1, 0, {""} },
		{ "Artist transliteration",		true, 1, 0, {""} },
	}
};


ScreenEdit::ScreenEdit()
{
	LOG->Trace( "ScreenEdit::ScreenEdit()" );

	TICK_EARLY_SECONDS.Refresh();

	// set both players to joined so the credit message doesn't show
	for( int p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_bSideIsJoined[p] = true;
	SCREENMAN->RefreshCreditsMessages();

	m_iRowLastCrossed = -1;
	m_fDestinationScrollSpeed = 1;

	m_pSong = GAMESTATE->m_pCurSong;

	m_pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
	
	/* Make EditMenu responsible for creating new Notes */
	//if( m_pNotes == NULL )
	//{
	//	m_pNotes = new Notes;
	//	m_pNotes->SetDifficulty(DIFFICULTY_MEDIUM);
	//	m_pNotes->m_NotesType = GAMESTATE->GetCurrentStyleDef()->m_NotesType;
	//	m_pNotes->SetDescription("Untitled");

	//	// In ScreenEditMenu, the screen preceding this one,
	//	// GAMEMAN->m_CurStyle is set to the target game style
	//	// of the current edit. Naturally, this is where we'll
	//	// want to extract the NotesType for a (NEW) sequence.

	//	m_pSong->m_apNotes.push_back( m_pNotes );

	//	GAMESTATE->m_pCurNotes[PLAYER_1] = m_pNotes;
	//}



	NoteData noteData;
	m_pNotes->GetNoteData( &noteData );


	m_EditMode = MODE_EDITING;

	GAMESTATE->m_bEditing = true;

	GAMESTATE->m_fSongBeat = 0;
	m_fTrailingBeat = GAMESTATE->m_fSongBeat;

	GAMESTATE->m_SongOptions.m_fMusicRate = 1;
	
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 1;
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_ColorType = PlayerOptions::COLOR_NOTE;

	m_BGAnimation.LoadFromAniDir( THEME->GetPathTo("BGAnimations","edit") );

	shiftAnchor = -1;
	m_SnapDisplay.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_SnapDisplay.Load( PLAYER_1 );
	m_SnapDisplay.SetZoom( 0.5f );

	m_GrayArrowRowEdit.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_GrayArrowRowEdit.Load( PLAYER_1 );
	m_GrayArrowRowEdit.SetZoom( 0.5f );

	m_NoteFieldEdit.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_NoteFieldEdit.SetZoom( 0.5f );
	m_NoteFieldEdit.Load( &noteData, PLAYER_1, -200, 800 );

	m_rectRecordBack.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0) );

	m_GrayArrowRowRecord.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_GrayArrowRowRecord.Load( PLAYER_1 );
	m_GrayArrowRowRecord.SetZoom( 1.0f );

	m_NoteFieldRecord.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_NoteFieldRecord.SetZoom( 1.0f );
	m_NoteFieldRecord.Load( &noteData, PLAYER_1, -150, 350 );

	m_Clipboard.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );

	m_Player.Load( PLAYER_1, &noteData, NULL, NULL );
	m_Player.SetXY( PLAYER_X, PLAYER_Y );

	m_Fade.SetClosed();

	m_sprHelp.Load( THEME->GetPathTo("Graphics","edit help") );
	m_sprHelp.SetHorizAlign( Actor::align_left );
	m_sprHelp.SetXY( HELP_X, HELP_Y );

	m_textHelp.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textHelp.SetXY( HELP_TEXT_X, HELP_TEXT_Y );
	m_textHelp.SetHorizAlign( Actor::align_left );
	m_textHelp.SetVertAlign( Actor::align_top );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetText( HELP_TEXT );

	m_sprInfo.Load( THEME->GetPathTo("Graphics","edit info") );
	m_sprInfo.SetHorizAlign( Actor::align_right );
	m_sprInfo.SetXY( INFO_X, INFO_Y );

	m_textInfo.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textInfo.SetXY( INFO_TEXT_X, INFO_TEXT_Y );
	m_textInfo.SetHorizAlign( Actor::align_left );
	m_textInfo.SetVertAlign( Actor::align_top );
	m_textInfo.SetZoom( 0.5f );
	//m_textInfo.SetText();	// set this below every frame

	m_soundChangeLine.Load( THEME->GetPathTo("Sounds","edit change line") );
	m_soundChangeSnap.Load( THEME->GetPathTo("Sounds","edit change snap") );
	m_soundMarker.Load(		THEME->GetPathTo("Sounds","edit marker") );
	m_soundInvalid.Load(	THEME->GetPathTo("Sounds","menu invalid") );


	m_soundMusic.Load(m_pSong->GetMusicPath());
	m_soundMusic.SetAccurateSync(true);

	m_soundAssistTick.Load(		THEME->GetPathTo("Sounds","gameplay assist tick") );

	m_Fade.OpenWipingRight();
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

	if( GAMESTATE->m_SongOptions.m_AssistType != SongOptions::ASSIST_TICK )
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
		bAnyoneHasANote |= m_Player.IsThereANoteAtRow( r );

	iRowLastCrossed = iRowNow;

	return bAnyoneHasANote;
}

void ScreenEdit::Update( float fDeltaTime )
{
	// approach destonation scroll speed
	if( m_fDestinationScrollSpeed != GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed )
	{
		float fDelta = m_fDestinationScrollSpeed - GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed;
		float fSign = fDelta / fabsf(fDelta);
		float fToMove = fSign * fDeltaTime * 4 * (m_fDestinationScrollSpeed>2 ? 2 : 1);
		CLAMP( fToMove, -fabsf(fDelta), fabsf(fDelta) );
		GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed += fToMove;
	}


	float fPositionSeconds = m_soundMusic.GetPositionSeconds();
	float fSongBeat, fBPS;
	bool bFreeze;
	m_pSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );


	// update the global music statistics for other classes to access
	GAMESTATE->m_fMusicSeconds = fPositionSeconds;
	GAMESTATE->m_fCurBPS = fBPS;
	GAMESTATE->m_bFreeze = bFreeze;


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
				HoldNote newHN = { t, fStartBeat, fEndBeat };
				m_NoteFieldRecord.AddHoldNote( newHN );
			}
		}
	}

	if( m_EditMode == MODE_RECORDING  ||  m_EditMode == MODE_PLAYING )
	{
		GAMESTATE->m_fSongBeat = fSongBeat;

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
	m_Fade.Update( fDeltaTime );
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
		float fMoveDelta = fSign*fDeltaTime*40 / GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed;
		if( fabsf(fMoveDelta) > fabsf(fDelta) )
			fMoveDelta = fDelta;
		m_fTrailingBeat += fMoveDelta;

		if( fabsf(fDelta) > 10 )
			m_fTrailingBeat += fDelta * fDeltaTime*5;
	}

	m_NoteFieldEdit.Update( fDeltaTime );

	if(m_EditMode == MODE_PLAYING && PlayTicks())
		m_soundAssistTick.Play();



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


	// Only update stats every 100 frames because it's slow
	static int iNumTapNotes = 0, iNumHoldNotes = 0;
	static int iCounter = 0;
	iCounter++;
	if( iCounter % 100 == 0 )
	{
		iNumTapNotes = m_NoteFieldEdit.GetNumTapNotes();
		iNumHoldNotes = m_NoteFieldEdit.GetNumHoldNotes();
	}

	CString sText;
	sText += ssprintf( "Current Beat:\n     %.2f\n",		GAMESTATE->m_fSongBeat );
	sText += ssprintf( "Snap to:\n     %s\n",				sNoteType.GetString() );
	sText += ssprintf( "Selection begin:\n     %.2f\n",		m_NoteFieldEdit.m_fBeginMarker );
	sText += ssprintf( "Selection end:\n     %.2f\n",		m_NoteFieldEdit.m_fEndMarker );
	sText += ssprintf( "Difficulty:\n     %s\n",			DifficultyToString( m_pNotes->GetDifficulty() ).GetString() );
	sText += ssprintf( "Description:\n     %s\n",			GAMESTATE->m_pCurNotes[PLAYER_1] ? GAMESTATE->m_pCurNotes[PLAYER_1]->GetDescription().GetString() : "no description" );
	sText += ssprintf( "Main title:\n     %s\n",			m_pSong->m_sMainTitle.GetString() );
	sText += ssprintf( "Sub title:\n     %s\n",				m_pSong->m_sSubTitle.GetString() );
	sText += ssprintf( "Tap Notes:\n     %d\n",				iNumTapNotes );
	sText += ssprintf( "Hold Notes:\n     %d\n",			iNumHoldNotes );
	sText += ssprintf( "Beat 0 Offset:\n     %.2f secs\n",	m_pSong->m_fBeat0OffsetInSeconds );
	sText += ssprintf( "Preview Start:\n     %.2f secs\n",	m_pSong->m_fMusicSampleStartSeconds );
	sText += ssprintf( "Preview Length:\n     %.2f secs\n",m_pSong->m_fMusicSampleLengthSeconds );

	m_textInfo.SetText( sText );
}


void ScreenEdit::DrawPrimitives()
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
	m_Fade.Draw();


	m_rectRecordBack.Draw();

	if( m_EditMode == MODE_RECORDING )
	{
		m_GrayArrowRowRecord.Draw();
		m_NoteFieldRecord.Draw();
	}

	if( m_EditMode == MODE_PLAYING )
	{
		m_Player.Draw();
	}

	Screen::DrawPrimitives();
}

void ScreenEdit::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEdit::Input()" );

	switch( m_EditMode )
	{
	case MODE_EDITING:		InputEdit( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_RECORDING:	InputRecord( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_PLAYING:		InputPlay( DeviceI, type, GameI, MenuI, StyleI );	break;
	default:	ASSERT(0);
	}
}

// Begin helper functions for InputEdit

void AddBGChange( CString sBGName )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	unsigned i;
	for( i=0; i<pSong->m_BackgroundChanges.size(); i++ )
	{
		if( pSong->m_BackgroundChanges[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
			break;
	}

	if( i != pSong->m_BackgroundChanges.size() )	// there is already a BGChange here
		pSong->m_BackgroundChanges.erase( pSong->m_BackgroundChanges.begin()+i,
										  pSong->m_BackgroundChanges.begin()+i+1);

	// create a new BGChange
	if( sBGName != "" )
		pSong->AddBackgroundChange( BackgroundChange(GAMESTATE->m_fSongBeat, sBGName) );
}

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

// End helper functions for InputEdit

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

			int iCol = DeviceI.button - SDLK_1;
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
				if( DeviceI.button == SDLK_UP )
				{
					if( m_fDestinationScrollSpeed == 4 )
					{
						m_fDestinationScrollSpeed = 2;
						m_soundMarker.Play();
					}
					else if( m_fDestinationScrollSpeed == 2 )
					{
						m_fDestinationScrollSpeed = 1;
						m_soundMarker.Play();
					}
					break;
				}
				else if( DeviceI.button == SDLK_DOWN )
				{
					if( m_fDestinationScrollSpeed == 1 )
					{
						m_fDestinationScrollSpeed = 2;
						m_soundMarker.Play();
					}
					else if( m_fDestinationScrollSpeed == 2 )
					{
						m_fDestinationScrollSpeed = 4;
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
				HoldNote newHN;
				newHN.iTrack = col;
				newHN.fStartBeat = min(fStartBeat, fEndBeat);
				newHN.fEndBeat = max(fStartBeat, fEndBeat);

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
			GAMESTATE->m_fSongBeat = clamp( GAMESTATE->m_fSongBeat, 0, MAX_BEATS-1 );
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
	case SDLK_RIGHT:
		m_SnapDisplay.PrevSnapMode();
		OnSnapModeChange();
		break;
	case SDLK_LEFT:
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
			if( GAMESTATE->m_fSongBeat == m_NoteFieldEdit.m_fBeginMarker )
			{
				m_NoteFieldEdit.m_fBeginMarker = m_NoteFieldEdit.m_fEndMarker;
				m_NoteFieldEdit.m_fEndMarker = -1;
			}
			else if( GAMESTATE->m_fSongBeat == m_NoteFieldEdit.m_fEndMarker )
			{
				m_NoteFieldEdit.m_fEndMarker = -1;
			}
			else
			{
				m_NoteFieldEdit.m_fBeginMarker = GAMESTATE->m_fSongBeat;
				m_NoteFieldEdit.m_fEndMarker = -1;
			}
		}
		m_soundMarker.Play();
		break;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		{
			// update enabled/disabled in g_AreaMenu
			bool bAreaSelected = m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1;
			g_AreaMenu.lines[cut].bEnabled = bAreaSelected;
			g_AreaMenu.lines[copy].bEnabled = bAreaSelected;
			g_AreaMenu.lines[paste].bEnabled = this->m_Clipboard.GetLastBeat() != 0;
			g_AreaMenu.lines[clear].bEnabled = bAreaSelected;
			g_AreaMenu.lines[quantize].bEnabled = bAreaSelected;
			g_AreaMenu.lines[transform].bEnabled = bAreaSelected;
			g_AreaMenu.lines[record].bEnabled = bAreaSelected;
			g_AreaMenu.lines[insert_and_shift].bEnabled = !bAreaSelected;
			g_AreaMenu.lines[delete_and_shift].bEnabled = !bAreaSelected;
			SCREENMAN->MiniMenu( &g_AreaMenu, SM_BackFromAreaMenu );
		}
		break;
	case SDLK_ESCAPE:
		SCREENMAN->MiniMenu( &g_MainMenu, SM_BackFromMainMenu );
		break;

	case SDLK_d:
		{
			Difficulty dc = Difficulty( (m_pNotes->GetDifficulty()+1)%NUM_DIFFICULTIES );
			m_pNotes->SetDifficulty(dc);
		}
		break;

	case SDLK_e:
		SCREENMAN->TextEntry( SM_None, "Edit notes description.\nPress Enter to confirm,\nEscape to cancel.", m_pNotes->GetDescription(), ChangeDescription, NULL );
		break;

	case SDLK_b:
		{
			CString sOldBackground;
			unsigned i;
			for( i=0; i<m_pSong->m_BackgroundChanges.size(); i++ )
			{
				if( m_pSong->m_BackgroundChanges[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
					break;
			}
			if( i != m_pSong->m_BackgroundChanges.size() )	// there is already a BGChange here
				sOldBackground = m_pSong->m_BackgroundChanges[i].m_sBGName;

			SCREENMAN->TextEntry( SM_None, "Type a background name.\nPress Enter to keep,\nEscape to cancel.\nEnter an empty string to remove\nthe Background Change.", sOldBackground, AddBGChange, NULL );
		}
		break;

	case SDLK_i:
		if( GAMESTATE->m_SongOptions.m_AssistType==SongOptions::ASSIST_TICK )
			GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_NONE;
		else
			GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_TICK;
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
			switch( type )
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
				if( i > 0  &&  fabsf(m_pSong->m_BPMSegments[i-1].m_fBPM - fNewBPM) < 0.025f )
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
			switch( type )
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
	}
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
				switch( type )
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
	m_rectRecordBack.SetTweenDiffuse( RageColor(0,0,0,0) );

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
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenEditMenu" );
		break;
	case SM_GoToNextScreen:
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
	case SM_BackFromEditOptions:
		HandleEditOptionsChoice( (EditOptionsChoice)ScreenMiniMenu::s_iLastLine, ScreenMiniMenu::s_iLastAnswers );
		break;
	case SM_BackFromEditSongInfo:
		HandleEditSongInfoChoice( (EditSongInfoChoice)ScreenMiniMenu::s_iLastLine, ScreenMiniMenu::s_iLastAnswers );
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

void ScreenEdit::HandleMainMenuChoice( MainMenuChoice c, int* iAnswers )
{
	switch( c )
	{
		case edit_notes_statistics:
			{
				Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
				float fMusicSeconds = m_soundMusic.GetLengthSeconds();

				g_EditNotesStatistics.lines[difficulty].iDefaultOption = pNotes->GetDifficulty();
				g_EditNotesStatistics.lines[meter].iDefaultOption = pNotes->GetMeter()+1;
				strcpy( g_EditNotesStatistics.lines[description].szOptionsText[0], pNotes->GetDescription() );
				strcpy( g_EditNotesStatistics.lines[tap_notes].szOptionsText[0], ssprintf("%d", m_NoteFieldEdit.GetNumTapNotes()) );
				strcpy( g_EditNotesStatistics.lines[hold_notes].szOptionsText[0], ssprintf("%d", m_NoteFieldEdit.GetNumHoldNotes()) );
				strcpy( g_EditNotesStatistics.lines[stream].szOptionsText[0], ssprintf("%f", NoteDataUtil::GetStreamRadarValue(m_NoteFieldEdit,fMusicSeconds)) );
				strcpy( g_EditNotesStatistics.lines[voltage].szOptionsText[0], ssprintf("%f", NoteDataUtil::GetVoltageRadarValue(m_NoteFieldEdit,fMusicSeconds)) );
				strcpy( g_EditNotesStatistics.lines[air].szOptionsText[0], ssprintf("%f", NoteDataUtil::GetAirRadarValue(m_NoteFieldEdit,fMusicSeconds)) );
				strcpy( g_EditNotesStatistics.lines[freeze].szOptionsText[0], ssprintf("%f", NoteDataUtil::GetFreezeRadarValue(m_NoteFieldEdit,fMusicSeconds)) );
				strcpy( g_EditNotesStatistics.lines[chaos].szOptionsText[0], ssprintf("%f", NoteDataUtil::GetChaosRadarValue(m_NoteFieldEdit,fMusicSeconds)) );
				SCREENMAN->MiniMenu( &g_EditNotesStatistics, SM_BackFromEditNotesStatistics );
			}
			break;
		case play_whole_song:
			break;
		case save:
			{
				// copy edit into current Notes
				Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
				ASSERT( pNotes );

				pNotes->SetNoteData( &m_NoteFieldEdit );
				GAMESTATE->m_pCurSong->Save();

				SCREENMAN->SystemMessage( "Saved as SM and DWI." );
				SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","edit save") );
			}
			break;
		case editor_options:
			SCREENMAN->MiniMenu( &g_EditOptions, SM_BackFromEditOptions );
			break;
		case player_options:
			break;
		case song_options:
			break;
		case edit_song_info:
			SCREENMAN->MiniMenu( &g_EditSongInfo, SM_BackFromEditSongInfo );
			break;
		case edit_bpm:
			break;
		case edit_stop:
			break;
		case edit_bg_change:
			break;
		case play_preview_music:
			SOUNDMAN->PlayMusic("");
			SOUNDMAN->PlayMusic( m_pSong->GetMusicPath(), false,
				m_pSong->m_fMusicSampleStartSeconds,
				m_pSong->m_fMusicSampleLengthSeconds,
				1.5f );
			break;
		case exit:
			SCREENMAN->SetNewScreen( "ScreenEditMenu" );
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
				HandleAreaMenuChoice( copy, iAnswers );
				HandleAreaMenuChoice( clear, iAnswers );
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
		case paste:
			{
				int iSrcFirstRow = 0;
				int iSrcLastRow  = BeatToNoteRow( m_Clipboard.GetLastBeat() );
				int iDestFirstRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
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
		case transform:
			{
				HandleAreaMenuChoice( cut, iAnswers );

				TransformType tt = (TransformType)iAnswers[c];
				switch( tt )
				{
				case little:		NoteDataUtil::Little( m_Clipboard );							break;
				case big:			NoteDataUtil::Big( m_Clipboard );								break;
				case left:			NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::left );			break;
				case right:			NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::right );			break;
				case shuffle:		NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::shuffle );		break;
				case super_shuffle:	NoteDataUtil::Turn( m_Clipboard, NoteDataUtil::super_shuffle );	break;
				case backwards:		NoteDataUtil::Backwards( m_Clipboard );							break;
				case swap_sides:	NoteDataUtil::SwapSides( m_Clipboard );							break;
				default:		ASSERT(0);
				}

				HandleAreaMenuChoice( paste, iAnswers );
			}
			break;
		case play:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );

				m_EditMode = MODE_PLAYING;

				m_Player.Load( PLAYER_1, (NoteData*)&m_NoteFieldEdit, NULL, NULL );

				m_rectRecordBack.StopTweening();
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetTweenDiffuse( RageColor(0,0,0,0.8f) );

				GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fBeginMarker - 4;	// give a 1 measure lead-in
				float fStartSeconds = m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat) ;
				m_soundMusic.SetPositionSeconds( fStartSeconds );
				m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
				m_soundMusic.StartPlaying();
			}
			break;
		case record:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );

				m_EditMode = MODE_RECORDING;

				// initialize m_NoteFieldRecord
				m_NoteFieldRecord.ClearAll();
				m_NoteFieldRecord.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );
				m_NoteFieldRecord.m_fBeginMarker = m_NoteFieldEdit.m_fBeginMarker;
				m_NoteFieldRecord.m_fEndMarker = m_NoteFieldEdit.m_fEndMarker;

				m_rectRecordBack.StopTweening();
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetTweenDiffuse( RageColor(0,0,0,0.8f) );

				GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fBeginMarker - 4;	// give a 1 measure lead-in
				float fStartSeconds = m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat) ;
				m_soundMusic.SetPositionSeconds( fStartSeconds );
				m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
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
	switch( c )
	{
	case difficulty:
		break;
	case meter:
		break;
	case description:
		break;
	case tap_notes:
		break;
	case hold_notes:
		break;
	case stream:
		break;
	case voltage:
		break;
	case air:
		break;
	case freeze:
		break;
	case chaos:
		break;
	default:
		ASSERT(0);
	};

}

void ScreenEdit::HandleEditOptionsChoice( EditOptionsChoice c, int* iAnswers )
{
	switch( c )
	{
	case zoom:
		break;
	case max_notes_per_row:
		break;
	case after_note_add:
		break;
	default:
		ASSERT(0);
	};
}

void ScreenEdit::HandleEditSongInfoChoice( EditSongInfoChoice c, int* iAnswers )
{
	switch( c )
	{
	case main_title:
		break;
	case sub_title:
		break;
	case artist:
		break;
	case main_title_transliteration:
		break;
	case sub_title_transliteration:
		break;
	case artist_transliteration:
		break;
	default:
		ASSERT(0);
	};
}