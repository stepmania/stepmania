#include "global.h"
#include "ScreenEdit.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameSoundManager.h"
#include "GameState.h"
#include "InputMapper.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "ScreenMiniMenu.h"
#include "NoteSkinManager.h"
#include "Steps.h"
#include <utility>
#include "NoteFieldPositioning.h"
#include "arch/arch.h"
#include "NoteDataUtil.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "Foreach.h"


const float RECORD_HOLD_SECONDS = 0.3f;


//
// Defines specific to GameScreenTitleMenu
//

#define DEBUG_X			(SCREEN_LEFT + 10)
#define DEBUG_Y			(CENTER_Y-100)

#define SHORTCUTS_X		(CENTER_X - 150)
#define SHORTCUTS_Y		(CENTER_Y)

#define HELP_X			(SCREEN_LEFT)
#define HELP_Y			(CENTER_Y)

#define HELP_TEXT_X		(SCREEN_LEFT + 4)
#define HELP_TEXT_Y		(40)

#define INFO_X			(SCREEN_RIGHT)
#define INFO_Y			(CENTER_Y)

#define INFO_TEXT_X		(SCREEN_RIGHT - 114)
#define INFO_TEXT_Y		(40)

#define MENU_WIDTH		(110)
#define EDIT_X			(CENTER_X)
#define EDIT_GRAY_Y_STANDARD	(SCREEN_TOP+60)
#define EDIT_GRAY_Y_REVERSE		(SCREEN_BOTTOM-60)

#define PLAYER_X			(CENTER_X)
#define PLAYER_Y			(CENTER_Y)
#define PLAYER_HEIGHT		(360)
#define PLAYER_Y_STANDARD	(PLAYER_Y-PLAYER_HEIGHT/2)
#define PLAYER_Y_REVERSE	(PLAYER_Y+PLAYER_HEIGHT/2)


#define ACTION_MENU_ITEM_X			(CENTER_X-200)
#define ACTION_MENU_ITEM_START_Y	(SCREEN_TOP + 24)
#define ACTION_MENU_ITEM_SPACING_Y	(18)

#define NAMING_MENU_ITEM_X			(CENTER_X-200)
#define NAMING_MENU_ITEM_START_Y	(SCREEN_TOP + 24)
#define NAMING_MENU_ITEM_SPACING_Y	(18)

CachedThemeMetricF	 TICK_EARLY_SECONDS		("ScreenGameplay","TickEarlySeconds");


const ScreenMessage SM_BackFromMainMenu				= (ScreenMessage)(SM_User+1);
const ScreenMessage SM_BackFromAreaMenu				= (ScreenMessage)(SM_User+2);
const ScreenMessage SM_BackFromEditNotesStatistics	= (ScreenMessage)(SM_User+3);
const ScreenMessage SM_BackFromEditOptions			= (ScreenMessage)(SM_User+4);
const ScreenMessage SM_BackFromEditSongInfo			= (ScreenMessage)(SM_User+5);
const ScreenMessage SM_BackFromBGChange				= (ScreenMessage)(SM_User+6);
const ScreenMessage SM_BackFromPlayerOptions		= (ScreenMessage)(SM_User+7);
const ScreenMessage SM_BackFromSongOptions			= (ScreenMessage)(SM_User+8);
const ScreenMessage SM_BackFromInsertAttack			= (ScreenMessage)(SM_User+9);
const ScreenMessage SM_BackFromInsertAttackModifiers= (ScreenMessage)(SM_User+10);
const ScreenMessage SM_BackFromPrefs				= (ScreenMessage)(SM_User+11);
const ScreenMessage SM_BackFromCourseModeMenu		= (ScreenMessage)(SM_User+12);
const ScreenMessage SM_DoReloadFromDisk				= (ScreenMessage)(SM_User+13);
const ScreenMessage SM_DoUpdateTextInfo				= (ScreenMessage)(SM_User+14);

const CString HELP_TEXT = 
	"Up/Down:\n     change beat\n"
	"Left/Right:\n     change snap\n"
	"Number keys:\n     add/remove\n     tap note\n"
	"Create hold note:\n     Hold a number\n     while moving\n     Up or Down\n"
	"Space bar:\n     Set area\n     marker\n"
	"Enter:\n     Area Menu\n"
	"Escape:\n     Main Menu\n"
	"F1:\n     Show\n     keyboard\n     shortcuts\n";


static const MenuRow g_KeyboardShortcutsItems[] =
{
	{ "PgUp/PgDn: jump measure",						false, 0, { NULL } },
	{ "Home/End: jump to first/last beat",				false, 0, { NULL } },
	{ "Ctrl + Up/Down: Change zoom",					false, 0, { NULL } },
	{ "Shift + Up/Down: Drag area marker",				false, 0, { NULL } },
	{ "P: Play selection",								false, 0, { NULL } },
	{ "Ctrl + P: Play whole song",						false, 0, { NULL } },
	{ "Shift + P: Play current beat to end",			false, 0, { NULL } },
	{ "Ctrl + R: Record",								false, 0, { NULL } },
	{ "F4: Toggle assist tick",							false, 0, { NULL } },
	{ "F5/F6: Next/prev steps of same StepsType",		false, 0, { NULL } },
	{ "F7/F8: Decrease/increase BPM at cur beat",		false, 0, { NULL } },
	{ "F9/F10: Decrease/increase stop at cur beat",		false, 0, { NULL } },
	{ "F11/F12: Decrease/increase music offset",		false, 0, { NULL } },
		/* XXX: This would be better as a single submenu, to let people tweak
		 * and play the sample several times (without having to re-enter the
		 * menu each time), so it doesn't use a whole bunch of hotkeys. */
	{ "[ and ]: Decrease/increase sample music start",	false, 0, { NULL } },
	{ "{ and }: Decrease/increase sample music length",	false, 0, { NULL } },
	{ "M: Play sample music",							false, 0, { NULL } },
	{ "B: Add/Edit Background Change",					false, 0, { NULL } },
	{ "Insert: Insert beat and shift down",				false, 0, { NULL } },
	{ "Ctrl + Insert: Shift BPM changes and stops down one beat",
														false, 0, { NULL } },
	{ "Delete: Delete beat and shift up",				false, 0, { NULL } },
	{ "Ctrl + Delete: Shift BPM changes and stops up one beat",
														false, 0, { NULL } },
	{ "Shift + number: Lay mine",						false, 0, { NULL } },
	{ "Alt + number: Add to/remove from right half",	false, 0, { NULL } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_KeyboardShortcuts( "Keyboard Shortcuts", g_KeyboardShortcutsItems );

static const MenuRow g_MainMenuItems[] =
{
	{ "Edit Steps Statistics",		true, 0, { NULL } },
	{ "Play Whole Song",			true, 0, { NULL } },
	{ "Play Current Beat To End",	true, 0, { NULL } },
	{ "Save",						true, 0, { NULL } },
	{ "Reload from disk",			true, 0, { NULL } },
	{ "Player Options",				true, 0, { NULL } },
	{ "Song Options",				true, 0, { NULL } },
	{ "Edit Song Info",				true, 0, { NULL } },
	{ "Add/Edit BG Change",			true, 0, { NULL } },
	{ "Play preview music",			true, 0, { NULL } },
	{ "Preferences",				true, 0, { NULL } },
	{ "Exit (discards changes since last save)",true, 0, { NULL } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_MainMenu( "Main Menu", g_MainMenuItems );

static const MenuRow g_AreaMenuItems[] =
{
	{ "Cut",						true, 0, { NULL } },
	{ "Copy",						true, 0, { NULL } },
	{ "Paste at current beat",		true, 0, { NULL } },
	{ "Paste at begin marker",		true, 0, { NULL } },
	{ "Clear",						true, 0, { NULL } },
	{ "Quantize",					true, 0, { "4TH","8TH","12TH","16TH","24TH","32ND","48TH","64TH" } },
	{ "Turn",						true, 0, { "Left","Right","Mirror","Shuffle","Super Shuffle" } },
	{ "Transform",					true, 0, { "NoHolds","NoMines","Little","Wide","Big","Quick","BMRize","Skippy","Mines","Echo","Stomp","Planted","Floored","Twister","NoJumps","NoHands","NoQuads" } },
	{ "Alter",						true, 0, { "Backwards","Swap Sides","Copy Left To Right","Copy Right To Left","Clear Left","Clear Right","Collapse To One","Collapse Left","Shift Left","Shift Right" } },
	{ "Tempo",						true, 0, { "Compress 2x","Compress 3->2","Compress 4->3","Expand 3->4","Expand 2->3","Expand 2x" } },
	{ "Play selection",				true, 0, { NULL } },
	{ "Record in selection",		true, 0, { NULL } },
	{ "Insert beat and shift down",	true, 0, { NULL } },
	{ "Delete beat and shift up",	true, 0, { NULL } },
	{ "Shift pauses and BPM changes down",			
									true, 0, { NULL } },
	{ "Shift pauses and BPM changes up",
									true, 0, { NULL } },
	{ "Convert beats to pause",		true, 0, { NULL } },
	{ "Convert pause to beats",		true, 0, { NULL } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_AreaMenu( "Area Menu", g_AreaMenuItems );

static const MenuRow g_EditNotesStatisticsItems[] =
{
	{ "Difficulty",					true,  0, { "BEGINNER","EASY","MEDIUM","HARD","CHALLENGE","EDIT" } },
	{ "Meter",						true,  0, { "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15" } },
	{ "Description",				true,  0, { NULL } },
	{ "Predicted Meter",			false, 0, { NULL } },
	{ "Tap Steps",					false, 0, { NULL } },
	{ "Hold Steps",					false, 0, { NULL } },
	{ "Stream",						false, 0, { NULL } },
	{ "Voltage",					false, 0, { NULL } },
	{ "Air",						false, 0, { NULL } },
	{ "Freeze",						false, 0, { NULL } },
	{ "Chaos",						false, 0, { NULL } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_EditNotesStatistics( "Statistics", g_EditNotesStatisticsItems );

static const MenuRow g_EditSongInfoItems[] =
{
	{ "Main title",					true, 0, { NULL } },
	{ "Sub title",					true, 0, { NULL } },
	{ "Artist",						true, 0, { NULL } },
	{ "Credit",						true, 0, { NULL } },
	{ "Main title transliteration",	true, 0, { NULL } },
	{ "Sub title transliteration",	true, 0, { NULL } },
	{ "Artist transliteration",		true, 0, { NULL } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_EditSongInfo( "Edit Song Info", g_EditSongInfoItems );

static const MenuRow g_BGChangeItems[] =
{
	{ "Rate (applies to new adds)",			true, 10, { "0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%","120%","140%","160%","180%","200%" } },
	{ "Fade Last (applies to new adds)",	true, 0, { "NO","YES" } },
	{ "Rewind Movie (applies to new adds)",	true, 0, { "NO","YES" } },
	{ "Loop (applies to new adds)",			true, 1, { "NO","YES" } },
	{ "Add Change to random",				true, 0, { NULL } },
	{ "Add Change to song BGAnimation",		true, 0, { NULL } },
	{ "Add Change to song Movie",			true, 0, { NULL } },
	{ "Add Change to song Still",			true, 0, { NULL } },
	{ "Add Change to global Random Movie",	true, 0, { NULL } },
	{ "Add Change to global BGAnimation",	true, 0, { NULL } },
	{ "Add Change to global Visualization",	true, 0, { NULL } },
	{ "Remove Change",						true, 0, { NULL } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_BGChange( "Background Change", g_BGChangeItems );

static const MenuRow g_PrefsItems[] =
{
	{ "Show BGChanges during Play/Record",	true, 0, { "NO","YES" } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_Prefs( "Preferences", g_PrefsItems );

static const MenuRow g_InsertAttackItems[] =
{
	{ "Duration seconds",					true, 3, { "5","10","15","20","25","30","35","40","45" } },
	{ "Set modifiers",						true, 0, { "PRESS START" } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_InsertAttack( "Insert Attack", g_InsertAttackItems );

static const MenuRow g_CourseModeItems[] =
{
	{ "Play mods from course",				true, 0, { NULL } },
	{ NULL, true, 0, { NULL } }
};
static Menu g_CourseMode( "Course Display", g_CourseModeItems );

// HACK: need to remember the track we're inserting on so
// that we can lay the attack note after coming back from
// menus.
int g_iLastInsertAttackTrack = -1;
float g_fLastInsertAttackDurationSeconds = -1;

ScreenEdit::ScreenEdit( CString sName ) : Screen( sName )
{
	LOG->Trace( "ScreenEdit::ScreenEdit()" );

	/* We do this ourself. */
	SOUND->HandleSongTimer( false );

	TICK_EARLY_SECONDS.Refresh();

	// set both players to joined so the credit message doesn't show
	FOREACH_PlayerNumber( p )
		GAMESTATE->m_bSideIsJoined[p] = true;
	SCREENMAN->RefreshCreditsMessages();

	m_pSong = GAMESTATE->m_pCurSong;
	m_pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	m_pAttacksFromCourse = NULL;

	NoteData noteData;
	m_pSteps->GetNoteData( &noteData );


	m_EditMode = MODE_EDITING;
	GAMESTATE->m_bPastHereWeGo = false;

	GAMESTATE->m_bEditing = true;

	GAMESTATE->m_fSongBeat = 0;
	m_fTrailingBeat = GAMESTATE->m_fSongBeat;

	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed = 1;
	GAMESTATE->m_SongOptions.m_fMusicRate = 1;
	/* Not all games have a noteskin named "note" ... */
	if( NOTESKIN->DoesNoteSkinExist("note") )
		GAMESTATE->m_PlayerOptions[PLAYER_1].m_sNoteSkin = "note";	// change noteskin before loading all of the edit Actors
	GAMESTATE->ResetNoteSkins();
	GAMESTATE->StoreSelectedOptions();

	m_BGAnimation.LoadFromAniDir( THEME->GetPathToB("ScreenEdit background") );

	shiftAnchor = -1;


	m_SnapDisplay.SetXY( EDIT_X, PLAYER_Y_STANDARD );
	m_SnapDisplay.Load( PLAYER_1 );
	m_SnapDisplay.SetZoom( 0.5f );

	m_NoteFieldEdit.SetXY( EDIT_X, PLAYER_Y );
	m_NoteFieldEdit.SetZoom( 0.5f );
	m_NoteFieldEdit.Load( &noteData, PLAYER_1, -240, 800, PLAYER_HEIGHT*2 );

	m_rectRecordBack.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0) );

	m_NoteFieldRecord.SetXY( EDIT_X, PLAYER_Y );
	m_NoteFieldRecord.SetZoom( 1.0f );
	m_NoteFieldRecord.Load( &noteData, PLAYER_1, -150, 350, 350 );

	m_Clipboard.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );


	GAMESTATE->m_PlayerOptions[PLAYER_1].Init();	// don't allow weird options in editor.  It doesn't handle reverse well.
	// Set NoteSkin to note if available.
	// Change noteskin back to default before loading player.
	if( NOTESKIN->DoesNoteSkinExist("note") )
		GAMESTATE->m_PlayerOptions[PLAYER_1].m_sNoteSkin = "note";
	GAMESTATE->ResetNoteSkins();

	/* XXX: Do we actually have to send real note data here, and to m_NoteFieldRecord? 
	 * (We load again on play/record.) */
	m_Player.Load( PLAYER_1, &noteData, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
	GAMESTATE->m_PlayerController[PLAYER_1] = PC_HUMAN;
	m_Player.SetX( PLAYER_X );
	/* Why was this here?  Nothing ever sets Player Y values; this was causing
	 * the display in play mode to be offset half a screen down. */
//	m_Player.SetXY( PLAYER_X, PLAYER_Y );

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
	m_textHelp.SetShadowLength( 0 );

	m_sprInfo.Load( THEME->GetPathToG("ScreenEdit Info") );
	m_sprInfo.SetHorizAlign( Actor::align_right );
	m_sprInfo.SetXY( INFO_X, INFO_Y );

	m_textInfo.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textInfo.SetXY( INFO_TEXT_X, INFO_TEXT_Y );
	m_textInfo.SetHorizAlign( Actor::align_left );
	m_textInfo.SetVertAlign( Actor::align_top );
	m_textInfo.SetZoom( 0.5f );
	m_textInfo.SetShadowLength( 0 );
	//m_textInfo.SetText();	// set this below every frame

	m_soundChangeLine.Load( THEME->GetPathToS("ScreenEdit line") );
	m_soundChangeSnap.Load( THEME->GetPathToS("ScreenEdit snap") );
	m_soundMarker.Load(		THEME->GetPathToS("ScreenEdit marker") );


	m_soundMusic.Load(m_pSong->GetMusicPath());

	m_soundAssistTick.Load(		THEME->GetPathToS("ScreenEdit assist tick") );

	this->HandleScreenMessage( SM_DoUpdateTextInfo );
}

ScreenEdit::~ScreenEdit()
{
	LOG->Trace( "ScreenEdit::~ScreenEdit()" );
	m_soundMusic.StopPlaying();
}

// play assist ticks
void ScreenEdit::PlayTicks()
{
	if( !GAMESTATE->m_SongOptions.m_bAssistTick || m_EditMode != MODE_PLAYING )
		return;
			
	/* Sound cards have a latency between when a sample is Play()ed and when the sound
	 * will start coming out the speaker.  Compensate for this by boosting fPositionSeconds
	 * ahead.  This is just to make sure that we request the sound early enough for it to
	 * come out on time; the actual precise timing is handled by SetStartTime. */
	float fPositionSeconds = GAMESTATE->m_fMusicSeconds;
	fPositionSeconds += SOUND->GetPlayLatency() + (float)TICK_EARLY_SECONDS + 0.250f;
	const float fSongBeat = GAMESTATE->m_pCurSong->GetBeatFromElapsedTime( fPositionSeconds );

	const int iSongRow = max( 0, BeatToNoteRowNotRounded( fSongBeat ) );
	static int iRowLastCrossed = -1;
	if( iSongRow < iRowLastCrossed )
		iRowLastCrossed = iSongRow;

	int iTickRow = -1;
	for( int r=iRowLastCrossed+1; r<=iSongRow; r++ )  // for each index we crossed since the last update
		if( m_Player.IsThereATapOrHoldHeadAtRow( r ) )
			iTickRow = r;

	iRowLastCrossed = iSongRow;

	if( iTickRow != -1 )
	{
		const float fTickBeat = NoteRowToBeat( iTickRow );
		const float fTickSecond = GAMESTATE->m_pCurSong->m_Timing.GetElapsedTimeFromBeat( fTickBeat );
		float fSecondsUntil = fTickSecond - GAMESTATE->m_fMusicSeconds;
		fSecondsUntil /= m_soundMusic.GetPlaybackRate(); /* 2x music rate means the time until the tick is halved */

		RageSoundParams p;
		p.StartTime = GAMESTATE->m_LastBeatUpdate + (fSecondsUntil - (float)TICK_EARLY_SECONDS);
		m_soundAssistTick.Play( &p );
	}
}

void ScreenEdit::PlayPreviewMusic()
{
	SOUND->PlayMusic("");
	SOUND->PlayMusic( m_pSong->GetMusicPath(), false,
		m_pSong->m_fMusicSampleStartSeconds,
		m_pSong->m_fMusicSampleLengthSeconds,
		1.5f );
}

void ScreenEdit::Update( float fDeltaTime )
{
	if( m_soundMusic.IsPlaying() )
	{
		RageTimer tm;
		const float fSeconds = m_soundMusic.GetPositionSeconds( NULL, &tm );
		GAMESTATE->UpdateSongPosition( fSeconds, GAMESTATE->m_pCurSong->m_Timing, tm );
	}

	if( m_EditMode == MODE_RECORDING  )	
	{
		// add or extend holds

		for( int t=0; t<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; t++ )	// for each track
		{
			StyleInput StyleI( PLAYER_1, t );
			float fSecsHeld = INPUTMAPPER->GetSecsHeld( StyleI );

			if( fSecsHeld > RECORD_HOLD_SECONDS && GAMESTATE->m_fSongBeat > 0 )
			{
				// add or extend hold
				const float fHoldStartSeconds = m_soundMusic.GetPositionSeconds() - fSecsHeld;

				float fStartBeat = max( 0, m_pSong->GetBeatFromElapsedTime( fHoldStartSeconds ) );
				float fEndBeat = max( fStartBeat, GAMESTATE->m_fSongBeat );

				// Round hold start and end to the nearest snap interval
				fStartBeat = froundf( fStartBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );
				fEndBeat = froundf( fEndBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );

				// create a new hold note
				HoldNote newHN( t, BeatToNoteRow(fStartBeat), BeatToNoteRow(fEndBeat) );
				m_NoteFieldRecord.AddHoldNote( newHN );
			}
		}
	}

	if( m_EditMode == MODE_RECORDING  ||  m_EditMode == MODE_PLAYING )
	{
		if( PREFSMAN->m_bEditorShowBGChangesPlay )
		{
			m_Background.Update( fDeltaTime );
			m_Foreground.Update( fDeltaTime );
		}

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
	m_NoteFieldEdit.Update( fDeltaTime );
	m_In.Update( fDeltaTime );
	m_Out.Update( fDeltaTime );
	m_sprHelp.Update( fDeltaTime );
	m_textHelp.Update( fDeltaTime );
	m_sprInfo.Update( fDeltaTime );
	m_textInfo.Update( fDeltaTime );

	m_rectRecordBack.Update( fDeltaTime );

	if( m_EditMode == MODE_RECORDING )
		m_NoteFieldRecord.Update( fDeltaTime );

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
		{
			/* We're far off; move faster.  Be sure to not overshoot. */
			fMoveDelta = fDelta * fDeltaTime*5;
			float fNewDelta = GAMESTATE->m_fSongBeat - m_fTrailingBeat;
			if( fabsf(fMoveDelta) > fabsf(fNewDelta) )
				fMoveDelta = fNewDelta;
			m_fTrailingBeat += fMoveDelta;
		}
	}

	m_NoteFieldEdit.Update( fDeltaTime );

	PlayTicks();
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
	case NOTE_TYPE_48TH:	sNoteType = "48th notes";	break;
	case NOTE_TYPE_64TH:	sNoteType = "64th notes";	break;
	default:  ASSERT(0);
	}

	CString sText;
	// check Editor.ini for a few of these
	// entries used: CurBeatPlaces, CurSecPlaces, OffsetPlaces,
	//               PreviewStartPlaces, PreviewLengthPlaces
	/* No need for that (unneeded options is just a maintenance pain).  If you want
	 * more precision here, add it.  I doubt there's a need for precise preview output,
	 * though (it'd be nearly inaudible at the millisecond level, and it's approximate
	 * anyway). */
	sText += ssprintf( "Current Beat:\n     %.2f\n",		GAMESTATE->m_fSongBeat );
	sText += ssprintf( "Current Second:\n     %.2f\n",		m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat) );
	sText += ssprintf( "Snap to:\n     %s\n",				sNoteType.c_str() );
	sText += ssprintf( "Selection begin:\n     %s\n",		m_NoteFieldEdit.m_fBeginMarker==-1 ? "not set" : ssprintf("%.2f",m_NoteFieldEdit.m_fBeginMarker).c_str() );
	sText += ssprintf( "Selection end:\n     %s\n",			m_NoteFieldEdit.m_fEndMarker==-1 ? "not set" : ssprintf("%.2f",m_NoteFieldEdit.m_fEndMarker).c_str() );
	sText += ssprintf( "Difficulty:\n     %s\n",			DifficultyToString( m_pSteps->GetDifficulty() ).c_str() );
	sText += ssprintf( "Description:\n     %s\n",			GAMESTATE->m_pCurSteps[PLAYER_1] ? GAMESTATE->m_pCurSteps[PLAYER_1]->GetDescription().c_str() : "no description" );
	sText += ssprintf( "Main title:\n     %s\n",			m_pSong->m_sMainTitle.c_str() );
	sText += ssprintf( "Sub title:\n     %s\n",				m_pSong->m_sSubTitle.c_str() );
	sText += ssprintf( "Tap Steps:\n     %d\n",				iNumTapNotes );
	sText += ssprintf( "Hold Steps:\n     %d\n",			iNumHoldNotes );
	sText += ssprintf( "Beat 0 Offset:\n     %.3f secs\n",	m_pSong->m_Timing.m_fBeat0OffsetInSeconds );
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
			m_sprHelp.Draw();
			m_textHelp.Draw();
			m_sprInfo.Draw();
			m_textInfo.Draw();
			m_SnapDisplay.Draw();

			// HACK:  Make NoteFieldEdit draw using the trailing beat
			float fSongBeat = GAMESTATE->m_fSongBeat;	// save song beat
			GAMESTATE->m_fSongBeat = m_fTrailingBeat;	// put trailing beat in effect
			m_NoteFieldEdit.Draw();
			GAMESTATE->m_fSongBeat = fSongBeat;	// restore real song beat

			m_In.Draw();
			m_Out.Draw();
		}
		break;
	case MODE_RECORDING:
		if( PREFSMAN->m_bEditorShowBGChangesPlay )
			m_Background.Draw();
		else
			m_BGAnimation.Draw();

		m_NoteFieldRecord.Draw();
		if( PREFSMAN->m_bEditorShowBGChangesPlay )
			m_Foreground.Draw();
		break;
	case MODE_PLAYING:
		if( PREFSMAN->m_bEditorShowBGChangesPlay )
			m_Background.Draw();
		else
			m_BGAnimation.Draw();

		m_Player.Draw();
		if( PREFSMAN->m_bEditorShowBGChangesPlay )
			m_Foreground.Draw();
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

	/* Make sure the displayed time is up-to-date after possibly changing something,
	 * so it doesn't feel lagged. */
	UpdateTextInfo();
}


void ScreenEdit::InputEdit( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.device != DEVICE_KEYBOARD )
		return;

	if( type == IET_LEVEL_CHANGED )
		return;		// don't care

	if(type == IET_RELEASE)
	{
		switch( DeviceI.button ) {
		case KEY_LSHIFT:
		case KEY_RSHIFT:
			shiftAnchor = -1;
			break;
		}
		return;
	}

	switch( DeviceI.button )
	{
	case KEY_C1:
	case KEY_C2:
	case KEY_C3:
	case KEY_C4:
	case KEY_C5:
	case KEY_C6:
	case KEY_C7:
	case KEY_C8:
	case KEY_C9:
	case KEY_C0:
		{
			if( type != IET_FIRST_PRESS )
				break;	// We only care about first presses

			/* Why was this changed back to just "DeviceI.button - KEY_1"?  That causes
			 * crashes when 0 is pressed (0 - 1 = -1). -glenn */
//			int iCol = DeviceI.button - KEY_1;
			int iCol = DeviceI.button == KEY_C0? 9: DeviceI.button - KEY_C1;


			// Alt + number = input to right half
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RALT)))
				iCol += m_NoteFieldEdit.GetNumTracks()/2;


			const float fSongBeat = GAMESTATE->m_fSongBeat;
			const int iSongIndex = BeatToNoteRow( fSongBeat );

			if( iCol >= m_NoteFieldEdit.GetNumTracks() )	// this button is not in the range of columns for this Style
				break;

			int i;

			// check for to see if the user intended to remove a HoldNote
			for( i=0; i<m_NoteFieldEdit.GetNumHoldNotes(); i++ )	// for each HoldNote
			{
				const HoldNote &hn = m_NoteFieldEdit.GetHoldNote(i);
				if( iCol == hn.iTrack  &&		// the notes correspond
					hn.RowIsInRange(iSongIndex) )	// the cursor lies within this HoldNote
				{
					m_NoteFieldEdit.RemoveHoldNote( i );
					return;
				}
			}

			if( m_NoteFieldEdit.GetTapNote(iCol, iSongIndex).type != TapNote::empty )
			{
				m_NoteFieldEdit.SetTapNote( iCol, iSongIndex, TAP_EMPTY );
				return;
			}

			// Hold LShift to lay mine, hold RShift to lay an attack
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) )
			{
				m_NoteFieldEdit.SetTapNote(iCol, iSongIndex, TAP_ORIGINAL_MINE );
			}
			else if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)) )
			{
				g_iLastInsertAttackTrack = iCol;
				SCREENMAN->MiniMenu( &g_InsertAttack, SM_BackFromInsertAttack );
			}
			else
			{
				m_NoteFieldEdit.SetTapNote(iCol, iSongIndex, TAP_ORIGINAL_TAP );
			}
		}
		break;
	case KEY_UP:
	case KEY_DOWN:
	case KEY_PGUP:
	case KEY_PGDN:
		{
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_RCTRL)) )
			{
				float& fScrollSpeed = GAMESTATE->m_PlayerOptions[PLAYER_1].m_fScrollSpeed;
				float fNewScrollSpeed = fScrollSpeed;

				if( DeviceI.button == KEY_UP )
				{
					if( fScrollSpeed == 4 )
						fNewScrollSpeed = 2;
					else if( fScrollSpeed == 2 )
						fNewScrollSpeed = 1;
				}
				else if( DeviceI.button == KEY_DOWN )
				{
					if( fScrollSpeed == 2 )
						fNewScrollSpeed = 4;
					else if( fScrollSpeed == 1 )
						fNewScrollSpeed = 2;
				}

				if( fNewScrollSpeed != fScrollSpeed )
				{
					fScrollSpeed = fNewScrollSpeed;
					m_soundMarker.Play();
					GAMESTATE->StoreSelectedOptions();
				}
				break;
			}

			float fBeatsToMove=0.f;
			switch( DeviceI.button )
			{
			case KEY_UP:
			case KEY_DOWN:
				fBeatsToMove = NoteTypeToBeat( m_SnapDisplay.GetNoteType() );
				if( DeviceI.button == KEY_UP )	
					fBeatsToMove *= -1;
			break;
			case KEY_PGUP:
			case KEY_PGDN:
				fBeatsToMove = BEATS_PER_MEASURE;
				if( DeviceI.button == KEY_PGUP )	
					fBeatsToMove *= -1;
			}

			const float fStartBeat = GAMESTATE->m_fSongBeat;
			const float fEndBeat = GAMESTATE->m_fSongBeat + fBeatsToMove;

			// check to see if they're holding a button
			for( int n=0; n<=9; n++ )	// for each number key
			{
				int iCol = n;

				// Ctrl + number = input to right half
				if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) ||
					INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RALT)))
					iCol += m_NoteFieldEdit.GetNumTracks()/2;

				if( iCol >= m_NoteFieldEdit.GetNumTracks() )
					continue;	// skip

				int b = n < 9? KEY_C1+n: KEY_C0;
				const DeviceInput di(DEVICE_KEYBOARD, b);


				if( !INPUTFILTER->IsBeingPressed(di) )
					continue;

				// create a new hold note
				HoldNote newHN( iCol, BeatToNoteRow(min(fStartBeat, fEndBeat)), BeatToNoteRow(max(fStartBeat, fEndBeat)) );

				newHN.iStartRow = max(newHN.iStartRow, 0);
				newHN.iEndRow = max(newHN.iEndRow, 0);

				m_NoteFieldEdit.AddHoldNote( newHN );
			}

			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)))
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
	case KEY_HOME:
		GAMESTATE->m_fSongBeat = 0;
		m_soundChangeLine.Play();
		break;
	case KEY_END:
		GAMESTATE->m_fSongBeat = m_NoteFieldEdit.GetLastBeat();
		m_soundChangeLine.Play();
		break;
	case KEY_LEFT:
		if( m_SnapDisplay.PrevSnapMode() )
			OnSnapModeChange();
		break;
	case KEY_RIGHT:
		if( m_SnapDisplay.NextSnapMode() )
			OnSnapModeChange();
		break;
	case KEY_SPACE:
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
	case KEY_ENTER:
	case KEY_KP_ENTER:
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
			g_AreaMenu.rows[tempo].enabled = bAreaSelected;
			g_AreaMenu.rows[play].enabled = bAreaSelected;
			g_AreaMenu.rows[record].enabled = bAreaSelected;
			g_AreaMenu.rows[convert_beat_to_pause].enabled = bAreaSelected;
			SCREENMAN->MiniMenu( &g_AreaMenu, SM_BackFromAreaMenu );
		}
		break;
	case KEY_ESC:
		SCREENMAN->MiniMenu( &g_MainMenu, SM_BackFromMainMenu );
		break;

	case KEY_F1:
		SCREENMAN->MiniMenu( &g_KeyboardShortcuts, SM_None );
		break;
	case KEY_F4:
		GAMESTATE->m_SongOptions.m_bAssistTick ^= 1;
		break;
	case KEY_F5:
	case KEY_F6:
		{
			// save current steps
			Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
			ASSERT( pSteps );
			pSteps->SetNoteData( &m_NoteFieldEdit );

			// Get all Steps of this StepsType
			StepsType st = pSteps->m_StepsType;
			vector<Steps*> vSteps;
			GAMESTATE->m_pCurSong->GetSteps( vSteps, st );

			// Sort them by difficulty.
			StepsUtil::SortStepsByTypeAndDifficulty( vSteps );

			// Find out what index the current Steps are
			vector<Steps*>::iterator it = find( vSteps.begin(), vSteps.end(), pSteps );
			ASSERT( it != vSteps.end() );

			switch( DeviceI.button )
			{
			case KEY_F5:	
				if( it==vSteps.begin() )
				{
					SCREENMAN->PlayInvalidSound();
					return;
				}
				it--;
				break;
			case KEY_F6:
				it++;
				if( it==vSteps.end() )
				{
					SCREENMAN->PlayInvalidSound();
					return;
				}
				break;
			default:	ASSERT(0);	return;
			}

			pSteps = *it;
			GAMESTATE->m_pCurSteps[PLAYER_1] = m_pSteps = pSteps;
			pSteps->GetNoteData( &m_NoteFieldEdit );
			SCREENMAN->SystemMessage( ssprintf(
				"Switched to %s %s '%s'",
				GAMEMAN->StepsTypeToString( pSteps->m_StepsType ).c_str(),
				DifficultyToString( pSteps->GetDifficulty() ).c_str(),
				pSteps->GetDescription().c_str() ) );
			SOUND->PlayOnce( THEME->GetPathToS("ScreenEdit switch") );
		}
		break;
	case KEY_F7:
	case KEY_F8:
		{
			// MD 11/02/03 - start referring to Editor.ini entries
			// entries here: BPMDelta, BPMDeltaFine
			float fBPM = m_pSong->GetBPMAtBeat( GAMESTATE->m_fSongBeat );
			float fDeltaBPM;
			switch( DeviceI.button )
			{
			case KEY_F7:	fDeltaBPM = - 0.020f;		break;
			case KEY_F8:	fDeltaBPM = + 0.020f;		break;
			default:	ASSERT(0);						return;
			}
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
				fDeltaBPM /= 2; /* .010 */
			else switch( type )
			{
			case IET_SLOW_REPEAT:	fDeltaBPM *= 10;	break;
			case IET_FAST_REPEAT:	fDeltaBPM *= 40;	break;
			}
			
			float fNewBPM = fBPM + fDeltaBPM;
			m_pSong->SetBPMAtBeat( GAMESTATE->m_fSongBeat, fNewBPM );
		}
		break;
	case KEY_F9:
	case KEY_F10:
		{
			// MD 11/02/03 - start referring to Editor.ini entries
			// entries here: StopDelta, StopDeltaFine
			float fStopDelta;
			switch( DeviceI.button )
			{
			case KEY_F9:	fStopDelta = -0.02f;		break;
			case KEY_F10:	fStopDelta = +0.02f;		break;
			default:	ASSERT(0);						return;
			}
			// MD 11/02/03 - requested: fine adjust for stops as well
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
				fStopDelta /= 4; /* .005 */
			else switch( type )
			{
			case IET_SLOW_REPEAT:	fStopDelta *= 10;	break;
			case IET_FAST_REPEAT:	fStopDelta *= 40;	break;
			}

			unsigned i;
			for( i=0; i<m_pSong->m_Timing.m_StopSegments.size(); i++ )
			{
				if( m_pSong->m_Timing.m_StopSegments[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
					break;
			}

			if( i == m_pSong->m_Timing.m_StopSegments.size() )	// there is no BPMSegment at the current beat
			{
				// create a new StopSegment
				if( fStopDelta > 0 )
					m_pSong->AddStopSegment( StopSegment(GAMESTATE->m_fSongBeat, fStopDelta) );
			}
			else	// StopSegment being modified is m_Timing.m_StopSegments[i]
			{
				m_pSong->m_Timing.m_StopSegments[i].m_fStopSeconds += fStopDelta;
				if( m_pSong->m_Timing.m_StopSegments[i].m_fStopSeconds <= 0 )
					m_pSong->m_Timing.m_StopSegments.erase( m_pSong->m_Timing.m_StopSegments.begin()+i,
													  m_pSong->m_Timing.m_StopSegments.begin()+i+1);
			}
		}
		break;
	case KEY_F11:
	case KEY_F12:
		{
			// MD 11/02/03 - start referring to Editor.ini entries
			// entries here: OffsetDelta, OffsetDeltaFine
			float fOffsetDelta;
			switch( DeviceI.button )
			{
			case KEY_F11:	fOffsetDelta = -0.02f;		break;
			case KEY_F12:	fOffsetDelta = +0.02f;		break;
			default:	ASSERT(0);						return;
			}
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
				INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
				fOffsetDelta /= 20; /* 1ms */
			else switch( type )
			{
			case IET_SLOW_REPEAT:	fOffsetDelta *= 10;	break;
			case IET_FAST_REPEAT:	fOffsetDelta *= 40;	break;
			}

			m_pSong->m_Timing.m_fBeat0OffsetInSeconds += fOffsetDelta;
		}
		break;
	case KEY_LBRACKET:
	case KEY_RBRACKET:
		{
			// MD 11/02/03 - start referring to Editor.ini entries
			// entries here: SampleLengthDelta, SampleLengthDeltaFine
			//				 SampleStartDelta, SampleStartDeltaFine
			float fDelta;
			switch( DeviceI.button )
			{
			case KEY_LBRACKET:		fDelta = -0.02f;	break;
			case KEY_RBRACKET:		fDelta = +0.02f;	break;
			default:	ASSERT(0);						return;
			}
			switch( type )
			{
			case IET_SLOW_REPEAT:	fDelta *= 10;	break;
			case IET_FAST_REPEAT:	fDelta *= 40;	break;
			}

			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)))
			{
				m_pSong->m_fMusicSampleLengthSeconds += fDelta;
				m_pSong->m_fMusicSampleLengthSeconds = max(m_pSong->m_fMusicSampleLengthSeconds,0);
			} else {
				m_pSong->m_fMusicSampleStartSeconds += fDelta;
				m_pSong->m_fMusicSampleStartSeconds = max(m_pSong->m_fMusicSampleStartSeconds,0);
			}
		}
		break;
	case KEY_Cm:
		PlayPreviewMusic();
		break;
	case KEY_Cb:
		HandleMainMenuChoice( edit_bg_change, NULL );
		break;
	case KEY_Cc:
	{
		g_CourseMode.rows[0].choices.clear();
		g_CourseMode.rows[0].choices.push_back( "OFF" );
		g_CourseMode.rows[0].defaultChoice = 0;

		vector<Course*> courses;
		SONGMAN->GetAllCourses( courses, false );
		for( unsigned i = 0; i < courses.size(); ++i )
		{
			const Course *crs = courses[i];

			bool bUsesThisSong = false;
			for( unsigned e = 0; e < crs->m_entries.size(); ++e )
			{
				if( crs->m_entries[e].type != COURSE_ENTRY_FIXED )
					continue;
				if( crs->m_entries[e].pSong != m_pSong )
					continue;
				bUsesThisSong = true;
			}

			if( bUsesThisSong )
			{
				g_CourseMode.rows[0].choices.push_back( crs->GetFullDisplayTitle() );
				if( crs == m_pAttacksFromCourse )
					g_CourseMode.rows[0].defaultChoice = g_CourseMode.rows[0].choices.size()-1;
			}
		}

		SCREENMAN->MiniMenu( &g_CourseMode, SM_BackFromCourseModeMenu );
		break;
	}
	case KEY_Cp:
		{
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_RCTRL)) )
				HandleMainMenuChoice( play_whole_song, NULL );
			else if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_LSHIFT)) ||
					 INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_RSHIFT)) )
				HandleMainMenuChoice( play_current_beat_to_end, NULL );
			else
				if( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 )
					HandleAreaMenuChoice( play, NULL );
		}
		break;
	case KEY_Cr:
		{
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_RCTRL)) )
				if( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 )
					HandleAreaMenuChoice( record, NULL );
		}
		break;
	case KEY_INSERT:
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_RCTRL)) )
				HandleAreaMenuChoice( shift_pauses_forward, NULL );
			else
				HandleAreaMenuChoice( insert_and_shift, NULL );
			SCREENMAN->PlayInvalidSound();
		break;
	case KEY_DEL:
			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_LCTRL)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD,KEY_RCTRL)) )
				HandleAreaMenuChoice( shift_pauses_backward, NULL );
			else
				HandleAreaMenuChoice( delete_and_shift, NULL );
			SCREENMAN->PlayInvalidSound();
		break;
	}
}

void ScreenEdit::InputRecord( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.device == DEVICE_KEYBOARD  &&  DeviceI.button == KEY_ESC )
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

			m_NoteFieldRecord.SetTapNote(iCol, iRow, TAP_ORIGINAL_TAP);
			m_NoteFieldRecord.Step( iCol, TNS_MARVELOUS );
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
		case KEY_ESC:
			TransitionToEdit();
			break;
		case KEY_F4:
			GAMESTATE->m_SongOptions.m_bAssistTick ^= 1;
			break;
		case KEY_F8:
			{
				PREFSMAN->m_bAutoPlay = !PREFSMAN->m_bAutoPlay;
				FOREACH_PlayerNumber( p )
					if( GAMESTATE->IsHumanPlayer(p) )
						GAMESTATE->m_PlayerController[p] = PREFSMAN->m_bAutoPlay?PC_AUTOPLAY:PC_HUMAN;
			}
			break;
		case KEY_F11:
		case KEY_F12:
			{
				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case KEY_F11:	fOffsetDelta = -0.020f;		break;
				case KEY_F12:	fOffsetDelta = +0.020f;		break;
				default:	ASSERT(0);						return;
				}

				if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RALT)) ||
					INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LALT)) )
					fOffsetDelta /= 20; /* 1ms */
				else switch( type )
				{
				case IET_SLOW_REPEAT:	fOffsetDelta *= 10;	break;
				case IET_FAST_REPEAT:	fOffsetDelta *= 40;	break;
				}

				m_pSong->m_Timing.m_fBeat0OffsetInSeconds += fOffsetDelta;
			}
		break;
		}
	}

	switch( StyleI.player )
	{
	case PLAYER_1:	
		if( !PREFSMAN->m_bAutoPlay )
			m_Player.Step( StyleI.col, DeviceI.ts ); 
		break;
	}

}


/* Switch to editing. */
void ScreenEdit::TransitionToEdit()
{
	/* Important: people will stop playing, change the BG and start again; make sure we reload */
	m_Background.Unload();
	m_Foreground.Unload();

	m_EditMode = MODE_EDITING;
	GAMESTATE->m_bPastHereWeGo = false;
	m_soundMusic.StopPlaying();
	m_soundAssistTick.StopPlaying(); /* Stop any queued assist ticks. */
	m_rectRecordBack.StopTweening();
	m_rectRecordBack.BeginTweening( 0.5f );
	m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0) );

	/* Make sure we're snapped. */
	GAMESTATE->m_fSongBeat = froundf( GAMESTATE->m_fSongBeat, NoteTypeToBeat(m_SnapDisplay.GetNoteType()) );

	/* Playing and recording have lead-ins, which may start before beat 0;
	 * make sure we don't stay there if we escaped out early. */
	GAMESTATE->m_fSongBeat = max( GAMESTATE->m_fSongBeat, 0 );

	/* Stop displaying course attacks, if any. */
	GAMESTATE->RemoveAllActiveAttacks();
	GAMESTATE->RebuildPlayerOptionsFromActiveAttacks( PLAYER_1 );
	GAMESTATE->m_CurrentPlayerOptions[PLAYER_1] = GAMESTATE->m_PlayerOptions[PLAYER_1];
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

/* Helper for SM_DoReloadFromDisk */
static bool g_DoReload;
void ReloadFromDisk( void *p )
{
	g_DoReload = true;
}

void ScreenEdit::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		// Reload song from disk to discard changes.
		SONGMAN->RevertFromDisk( GAMESTATE->m_pCurSong, true );
		
		/* We might do something with m_pSteps (eg. UpdateTextInfo) before we end up
		 * in ScreenEditMenu, and m_pSteps might be invalid due to RevertFromDisk. */
		m_pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];

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
	case SM_BackFromPrefs:
		PREFSMAN->m_bEditorShowBGChangesPlay = !!ScreenMiniMenu::s_iLastAnswers[pref_show_bgs_play];
		PREFSMAN->SaveGlobalPrefsToDisk();
		break;
	case SM_BackFromCourseModeMenu:
	{
		const int num = ScreenMiniMenu::s_iLastAnswers[0];
		m_pAttacksFromCourse = NULL;
		if( num != 0 )
		{
			const CString name = g_CourseMode.rows[0].choices[num];
			m_pAttacksFromCourse = SONGMAN->FindCourse( name );
			ASSERT( m_pAttacksFromCourse );
		}
		break;
	}
	case SM_BackFromPlayerOptions:
	case SM_BackFromSongOptions:
		// coming back from PlayerOptions or SongOptions
		GAMESTATE->StoreSelectedOptions();

		// stop any music that screen may have been playing
		SOUND->StopMusic();

		break;
	case SM_BackFromInsertAttack:
		{
			int iDurationChoice = ScreenMiniMenu::s_iLastAnswers[0];
			g_fLastInsertAttackDurationSeconds = strtof( g_InsertAttackItems[0].choices[iDurationChoice], NULL );
			GAMESTATE->StoreSelectedOptions();	// save so that we don't lose the options chosen for edit and playback
			SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromInsertAttackModifiers );
		}
		break;
	case SM_BackFromInsertAttackModifiers:
		{
			PlayerOptions poChosen = GAMESTATE->m_PlayerOptions[PLAYER_1];
			CString sMods = poChosen.GetString();
			const int iSongIndex = BeatToNoteRow( GAMESTATE->m_fSongBeat );
			
			Attack attack;
			attack.level = ATTACK_LEVEL_1;	// does this matter?
			attack.fSecsRemaining = g_fLastInsertAttackDurationSeconds;
			attack.sModifier = sMods;
			
			m_NoteFieldEdit.SetTapAttackNote( g_iLastInsertAttackTrack, iSongIndex, attack );
			GAMESTATE->RestoreSelectedOptions();	// restore the edit and playback options
		}
		break;
	case SM_DoReloadFromDisk:
	{
		if( !g_DoReload )
			return;

		const StepsType st = m_pSteps->m_StepsType;
		StepsID id;
		id.FromSteps( m_pSteps );

		GAMESTATE->m_pCurSteps[PLAYER_1] = NULL; /* make RevertFromDisk not try to reset it */
		SONGMAN->RevertFromDisk( GAMESTATE->m_pCurSong );

		CString sMessage = "Reloaded from disk.";
		Steps *pSteps = id.ToSteps( GAMESTATE->m_pCurSong, false );

		// Don't allow an autogen match.  This can't be what they chose to 
		// edit originally because autogen steps are hidden.
		if( pSteps && pSteps->IsAutogen() )
			pSteps = NULL;

		/* If we couldn't find the steps we were on before, warn and use the first available. */
		if( pSteps == NULL )
		{
			pSteps = GAMESTATE->m_pCurSong->GetStepsByDifficulty( st, DIFFICULTY_INVALID, false );

			if( pSteps )
				sMessage = ssprintf( "old steps not found; changed to %s.",
					DifficultyToString(pSteps->GetDifficulty()).c_str() );
		}

		/* If we still couldn't find any steps, then all steps of the current StepsType
		 * were removed.  Don't create them; only do that in EditMenu. */
		if( pSteps == NULL )
		{
			SCREENMAN->SetNewScreen( "ScreenEditMenu" );
			return;
		}

		SCREENMAN->SystemMessage( sMessage );

		m_pSteps = GAMESTATE->m_pCurSteps[PLAYER_1] = pSteps;
		m_pSteps->GetNoteData( &m_NoteFieldEdit );

		break;
	}
	case SM_DoUpdateTextInfo:
		this->PostScreenMessage( SM_DoUpdateTextInfo, 0.5f );
		UpdateTextInfo();
		break;

	case SM_GainFocus:
		/* We do this ourself. */
		SOUND->HandleSongTimer( false );

		/* When another screen comes up, RageSounds takes over the sound timer.  When we come
		 * back, put the timer back to where it was. */
		GAMESTATE->m_fSongBeat = m_fTrailingBeat;
		break;
	case SM_LoseFocus:
		/* Snap the trailing beat, in case we lose focus while tweening. */
		m_fTrailingBeat = GAMESTATE->m_fSongBeat;
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
	Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSteps->SetDescription(sNew);
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
				/* XXX: If the difficulty is changed from EDIT, and pSteps->WasLoadedFromProfile()
				 * is true, we should warn that the steps will no longer be saved to the profile. */
				Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
				float fMusicSeconds = m_soundMusic.GetLengthSeconds();

				g_EditNotesStatistics.rows[difficulty].defaultChoice = pSteps->GetDifficulty();
				g_EditNotesStatistics.rows[meter].defaultChoice = clamp( pSteps->GetMeter()-1, 0, 14 );
				g_EditNotesStatistics.rows[predict_meter].choices.resize(1);g_EditNotesStatistics.rows[predict_meter].choices[0] = ssprintf("%f",pSteps->PredictMeter());
				g_EditNotesStatistics.rows[description].choices.resize(1);	g_EditNotesStatistics.rows[description].choices[0] = pSteps->GetDescription();
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
				// copy edit into current Steps
				Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
				ASSERT( pSteps );

				pSteps->SetNoteData( &m_NoteFieldEdit );
				GAMESTATE->m_pCurSong->Save();

				// we shouldn't say we're saving a DWI if we're on any game besides
				// dance, it just looks tacky and people may be wondering where the
				// DWI file is :-)
				if ((int)pSteps->m_StepsType <= (int)STEPS_TYPE_DANCE_SOLO) 
					SCREENMAN->SystemMessage( "Saved as SM and DWI." );
				else
					SCREENMAN->SystemMessage( "Saved as SM." );
				SOUND->PlayOnce( THEME->GetPathToS("ScreenEdit save") );
			}
			break;
		case reload:
			g_DoReload = false;
			SCREENMAN->Prompt( SM_DoReloadFromDisk,
				"Do you want to reload from disk?\n\nThis will destroy all changes.",
				true, false, ReloadFromDisk, NULL, NULL );
			break;
		case player_options:
			SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromPlayerOptions );
			break;
		case song_options:
			SCREENMAN->AddNewScreenToTop( "ScreenSongOptions", SM_BackFromSongOptions );
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
//		case edit_bpm:
//			break;
//		case edit_stop:
//			break;
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
				BackgroundChange bgChange; 
				FOREACH( BackgroundChange, m_pSong->m_BackgroundChanges, bgc )
				{
					if( bgc->m_fStartBeat == GAMESTATE->m_fSongBeat )
					{
						bAlreadyBGChangeHere = true;
						bgChange = *bgc;
					}
				}

				g_BGChange.rows[add_random].enabled = true;
				g_BGChange.rows[add_song_bganimation].enabled = g_BGChange.rows[add_song_bganimation].choices.size() > 0;
				g_BGChange.rows[add_song_movie].enabled = g_BGChange.rows[add_song_movie].choices.size() > 0;
				g_BGChange.rows[add_song_still].enabled = g_BGChange.rows[add_song_still].choices.size() > 0;
				g_BGChange.rows[add_global_random_movie].enabled = g_BGChange.rows[add_global_random_movie].choices.size() > 0;
				g_BGChange.rows[add_global_bganimation].enabled = g_BGChange.rows[add_global_bganimation].choices.size() > 0;
				g_BGChange.rows[add_global_visualization].enabled = g_BGChange.rows[add_global_visualization].choices.size() > 0;
				g_BGChange.rows[delete_change].enabled = bAlreadyBGChangeHere;
					

				// set default choices
				g_BGChange.rows[rate].						SetDefaultChoiceIfPresent( ssprintf("%2.0f%%",bgChange.m_fRate*100) );
				g_BGChange.rows[fade_last].defaultChoice	= bgChange.m_bFadeLast ? 1 : 0;
				g_BGChange.rows[rewind_movie].defaultChoice = bgChange.m_bRewindMovie ? 1 : 0;
				g_BGChange.rows[loop].defaultChoice			= bgChange.m_bLoop ? 1 : 0;
				g_BGChange.rows[add_song_bganimation].		SetDefaultChoiceIfPresent( bgChange.m_sBGName );
				g_BGChange.rows[add_song_movie].			SetDefaultChoiceIfPresent( bgChange.m_sBGName );
				g_BGChange.rows[add_song_still].			SetDefaultChoiceIfPresent( bgChange.m_sBGName );
				g_BGChange.rows[add_global_random_movie].	SetDefaultChoiceIfPresent( bgChange.m_sBGName );
				g_BGChange.rows[add_global_bganimation].	SetDefaultChoiceIfPresent( bgChange.m_sBGName );
				g_BGChange.rows[add_global_visualization].	SetDefaultChoiceIfPresent( bgChange.m_sBGName );


				SCREENMAN->MiniMenu( &g_BGChange, SM_BackFromBGChange );
			}
			break;
		case preferences:
			g_Prefs.rows[pref_show_bgs_play].defaultChoice = PREFSMAN->m_bEditorShowBGChangesPlay;

			SCREENMAN->MiniMenu( &g_Prefs, SM_BackFromPrefs );
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
				const NoteData OldClipboard( m_Clipboard );
				HandleAreaMenuChoice( cut, NULL );
				
				StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
				TurnType tt = (TurnType)iAnswers[c];
				switch( tt )
				{
				case left:			NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::left );			break;
				case right:			NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::right );			break;
				case mirror:		NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::mirror );		break;
				case shuffle:		NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::shuffle );		break;
				case super_shuffle:	NoteDataUtil::Turn( m_Clipboard, st, NoteDataUtil::super_shuffle );	break;
				default:		ASSERT(0);
				}

				HandleAreaMenuChoice( paste_at_begin_marker, NULL );
				m_Clipboard = OldClipboard;
			}
			break;
		case transform:
			{
				float fBeginBeat = m_NoteFieldEdit.m_fBeginMarker;
				float fEndBeat = m_NoteFieldEdit.m_fEndMarker;
				TransformType tt = (TransformType)iAnswers[c];
				StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
				switch( tt )
				{
				case noholds:	NoteDataUtil::RemoveHoldNotes( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case nomines:	NoteDataUtil::RemoveMines( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case little:	NoteDataUtil::Little( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case wide:		NoteDataUtil::Wide( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case big:		NoteDataUtil::Big( m_NoteFieldEdit, fBeginBeat, fEndBeat );		break;
				case quick:		NoteDataUtil::Quick( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case bmrize:	NoteDataUtil::BMRize( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case skippy:	NoteDataUtil::Skippy( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case mines:		NoteDataUtil::AddMines( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case echo:		NoteDataUtil::Echo( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case stomp:		NoteDataUtil::Stomp( m_NoteFieldEdit, st, fBeginBeat, fEndBeat );	break;
				case planted:	NoteDataUtil::Planted( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case floored:	NoteDataUtil::Floored( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case twister:	NoteDataUtil::Twister( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case nojumps:	NoteDataUtil::RemoveJumps( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case nohands:	NoteDataUtil::RemoveHands( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				case noquads:	NoteDataUtil::RemoveQuads( m_NoteFieldEdit, fBeginBeat, fEndBeat );	break;
				default:		ASSERT(0);
				}

				// bake in the additions
				NoteDataUtil::ConvertAdditionsToRegular( m_NoteFieldEdit );
			}
			break;
		case alter:
			{
				const NoteData OldClipboard( m_Clipboard );
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
				case collapse_left:			NoteDataUtil::CollapseLeft( m_Clipboard );		break;
				case shift_left:			NoteDataUtil::ShiftLeft( m_Clipboard );			break;
				case shift_right:			NoteDataUtil::ShiftRight( m_Clipboard );		break;
				default:		ASSERT(0);
				}

				HandleAreaMenuChoice( paste_at_begin_marker, NULL );
				m_Clipboard = OldClipboard;
			}
			break;
		case tempo:
			{
				// This affects all steps.
				const NoteData OldClipboard( m_Clipboard );
				HandleAreaMenuChoice( cut, NULL );
				
				AlterType at = (AlterType)iAnswers[c];
				float fScale = -1;
				switch( at )
				{
				case compress_2x:	fScale = 0.5f;		break;
				case compress_3_2:	fScale = 2.0f/3;	break;
				case compress_4_3:	fScale = 0.75f;		break;
				case expand_4_3:	fScale = 4.0f/3;	break;
				case expand_3_2:	fScale = 1.5f;		break;
				case expand_2x:		fScale = 2;			break;
				default:		ASSERT(0);
				}

				m_Clipboard.ConvertHoldNotesTo2sAnd3s();

				switch( at )
				{
				case compress_2x:	NoteDataUtil::Scale( m_Clipboard, fScale );	break;
				case compress_3_2:	NoteDataUtil::Scale( m_Clipboard, fScale );	break;
				case compress_4_3:	NoteDataUtil::Scale( m_Clipboard, fScale );	break;
				case expand_4_3:	NoteDataUtil::Scale( m_Clipboard, fScale );	break;
				case expand_3_2:	NoteDataUtil::Scale( m_Clipboard, fScale );	break;
				case expand_2x:		NoteDataUtil::Scale( m_Clipboard, fScale );	break;
				default:		ASSERT(0);
				}

				m_Clipboard.Convert2sAnd3sToHoldNotes();

//				float fOldClipboardEndBeat = m_NoteFieldEdit.m_fEndMarker;
				float fOldClipboardBeats = m_NoteFieldEdit.m_fEndMarker - m_NoteFieldEdit.m_fBeginMarker;
				float fNewClipboardBeats = fOldClipboardBeats * fScale;
				float fDeltaBeats = fNewClipboardBeats - fOldClipboardBeats;
				float fNewClipboardEndBeat = m_NoteFieldEdit.m_fBeginMarker + fNewClipboardBeats;
				NoteDataUtil::ShiftRows( m_NoteFieldEdit, m_NoteFieldEdit.m_fBeginMarker, fDeltaBeats );
				m_pSong->m_Timing.ScaleRegion( fScale, m_NoteFieldEdit.m_fBeginMarker, m_NoteFieldEdit.m_fEndMarker );

				HandleAreaMenuChoice( paste_at_begin_marker, NULL );

				const vector<Steps*> sIter = m_pSong->GetAllSteps();
				NoteData ndTemp;
				CString sTempStyle, sTempDiff;
				for( unsigned i = 0; i < sIter.size(); i++ )
				{
					if( sIter[i]->IsAutogen() )
						continue;

					/* XXX: Edits are distinguished by description.  Compare vs m_pSteps. */
					if( (sIter[i]->m_StepsType == GAMESTATE->m_pCurSteps[PLAYER_1]->m_StepsType) &&
						(sIter[i]->GetDifficulty() == GAMESTATE->m_pCurSteps[PLAYER_1]->GetDifficulty()) )
						continue;

					sIter[i]->GetNoteData( &ndTemp );
					ndTemp.ConvertHoldNotesTo2sAnd3s();
					NoteDataUtil::ScaleRegion( ndTemp, fScale, m_NoteFieldEdit.m_fBeginMarker, m_NoteFieldEdit.m_fEndMarker );
					ndTemp.Convert2sAnd3sToHoldNotes();
					sIter[i]->SetNoteData( &ndTemp );
				}

				m_NoteFieldEdit.m_fEndMarker = fNewClipboardEndBeat;

				float fOldBPM = m_pSong->GetBPMAtBeat( m_NoteFieldEdit.m_fBeginMarker );
				float fNewBPM = fOldBPM * fScale;
				m_pSong->SetBPMAtBeat( m_NoteFieldEdit.m_fBeginMarker, fNewBPM );
				m_pSong->SetBPMAtBeat( fNewClipboardEndBeat, fOldBPM );
			}
			break;
		case play:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );

				SOUND->PlayMusic("");

				m_EditMode = MODE_PLAYING;
				GAMESTATE->m_bPastHereWeGo = true;

				/* Reset the note skin, in case preferences have changed. */
				GAMESTATE->ResetNoteSkins();

				/* Give a 1 measure lead-in.  Set this before loading Player, so it knows
				 * where we're starting. */
				float fSeconds = m_pSong->m_Timing.GetElapsedTimeFromBeat( m_NoteFieldEdit.m_fBeginMarker - 4 );
				GAMESTATE->UpdateSongPosition( fSeconds, m_pSong->m_Timing );

				SetupCourseAttacks();

				m_Player.Load( PLAYER_1, &m_NoteFieldEdit, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
				GAMESTATE->m_PlayerController[PLAYER_1] = PREFSMAN->m_bAutoPlay?PC_AUTOPLAY:PC_HUMAN;

				m_rectRecordBack.StopTweening();
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0.8f) );
				const float fStartSeconds = m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat) ;
				LOG->Trace( "Starting playback at %f", fStartSeconds );
			
				if( PREFSMAN->m_bEditorShowBGChangesPlay )
				{
					/* FirstBeat affects backgrounds, so commit changes to memory (not to disk)
					 * and recalc it. */
					Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
					ASSERT( pSteps );
					pSteps->SetNoteData( &m_NoteFieldEdit );
					m_pSong->ReCalculateRadarValuesAndLastBeat();

					m_Background.Unload();
					m_Background.LoadFromSong( m_pSong );

					m_Foreground.Unload();
					m_Foreground.LoadFromSong( m_pSong );
				}

				RageSoundParams p;
				p.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
				p.m_StartSecond = fStartSeconds;
				p.AccurateSync = true;
				p.StopMode = RageSoundParams::M_CONTINUE;
				m_soundMusic.Play( &p );
			}
			break;
		case record:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );

				SOUND->PlayMusic("");

				m_EditMode = MODE_RECORDING;
				GAMESTATE->m_bPastHereWeGo = true;

				/* Reset the note skin, in case preferences have changed. */
				GAMESTATE->ResetNoteSkins();

				// initialize m_NoteFieldRecord
				m_NoteFieldRecord.Load( &m_NoteFieldEdit, PLAYER_1, -150, 350, 350 );
				m_NoteFieldRecord.SetNumTracks( m_NoteFieldEdit.GetNumTracks() );

				m_rectRecordBack.StopTweening();
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetDiffuse( RageColor(0,0,0,0.8f) );

				GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fBeginMarker - 4;	// give a 1 measure lead-in
				float fStartSeconds = m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat);
				LOG->Trace( "Starting playback at %f", fStartSeconds );

				RageSoundParams p;
				p.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
				p.m_StartSecond = fStartSeconds;
				p.AccurateSync = true;
				p.StopMode = RageSoundParams::M_CONTINUE;
				m_soundMusic.Play( &p );
			}
			break;
		case insert_and_shift:
			NoteDataUtil::ShiftRows( m_NoteFieldEdit, GAMESTATE->m_fSongBeat, 1 );
			break;
		case delete_and_shift:
			NoteDataUtil::ShiftRows( m_NoteFieldEdit, GAMESTATE->m_fSongBeat, -1 );
			break;
		case shift_pauses_forward:
			m_pSong->m_Timing.ShiftRows( GAMESTATE->m_fSongBeat, 1 );
			break;
		case shift_pauses_backward:
			m_pSong->m_Timing.ShiftRows( GAMESTATE->m_fSongBeat, -1 );
			break;
		// MD 11/02/03 - Converting selected region to a pause of the same length.
		case convert_beat_to_pause:
			{
				ASSERT( m_NoteFieldEdit.m_fBeginMarker!=-1 && m_NoteFieldEdit.m_fEndMarker!=-1 );
				// This was written horribly, using beats and not converting to time at all.
				float fMarkerStart = m_pSong->m_Timing.GetElapsedTimeFromBeat(m_NoteFieldEdit.m_fBeginMarker);
				float fMarkerEnd = m_pSong->m_Timing.GetElapsedTimeFromBeat(m_NoteFieldEdit.m_fEndMarker);
				float fStopLength = fMarkerEnd - fMarkerStart;
				// be sure not to clobber the row at the start - a row at the end
				// can be dropped safely, though
				NoteDataUtil::ShiftRows( m_NoteFieldEdit, 
										 m_NoteFieldEdit.m_fBeginMarker + 0.003f,
										 (-m_NoteFieldEdit.m_fEndMarker+m_NoteFieldEdit.m_fBeginMarker)
									   );
				m_pSong->m_Timing.ShiftRows( m_NoteFieldEdit.m_fBeginMarker + 0.003f,
										     (-m_NoteFieldEdit.m_fEndMarker+m_NoteFieldEdit.m_fBeginMarker)
										   );
				unsigned i;
				for( i=0; i<m_pSong->m_Timing.m_StopSegments.size(); i++ )
				{
					float fStart = m_pSong->m_Timing.GetElapsedTimeFromBeat(m_pSong->m_Timing.m_StopSegments[i].m_fStartBeat);
					float fEnd = fStart + m_pSong->m_Timing.m_StopSegments[i].m_fStopSeconds;
					if( fStart > fMarkerEnd || fEnd < fMarkerStart )
						continue;
					else {
						if (fStart > fMarkerStart)
							m_pSong->m_Timing.m_StopSegments[i].m_fStartBeat = m_NoteFieldEdit.m_fBeginMarker;
						m_pSong->m_Timing.m_StopSegments[i].m_fStopSeconds = fStopLength;
						break;
					}
				}

				if( i == m_pSong->m_Timing.m_StopSegments.size() )	// there is no BPMSegment at the current beat
					m_pSong->AddStopSegment( StopSegment(m_NoteFieldEdit.m_fBeginMarker, fStopLength) );
				m_NoteFieldEdit.m_fEndMarker = -1;
				break;
			}
		// MD 11/02/03 - Converting a pause at the current beat into beats.
		//    I know this will break holds that cross the pause.  Anyone who
		//    wants to rewrite this to fix that behavior is welcome to - I'm
		//    not sure how exactly to do it without making this a lot longer
		//    than it is.
		// NOTE: Fixed this so that holds aren't broken by it.  Working in 2s and
		// 3s makes this work better, too. :-)  It sorta makes you wonder WHY we
		// don't bring it into 2s and 3s when we bring up the editor.
		case convert_pause_to_beat:
			{
				float fBPMatPause = m_pSong->GetBPMAtBeat( GAMESTATE->m_fSongBeat );
				unsigned i;
				for( i=0; i<m_pSong->m_Timing.m_StopSegments.size(); i++ )
				{
					if( m_pSong->m_Timing.m_StopSegments[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
						break;
				}

				if( i == m_pSong->m_Timing.m_StopSegments.size() )	// there is no BPMSegment at the current beat
					break;
				else	// StopSegment being modified is m_Timing.m_StopSegments[i]
				{
					float fStopLength = m_pSong->m_Timing.m_StopSegments[i].m_fStopSeconds;
					m_pSong->m_Timing.m_StopSegments.erase( m_pSong->m_Timing.m_StopSegments.begin()+i,
												   m_pSong->m_Timing.m_StopSegments.begin()+i+1);
					fStopLength *= fBPMatPause;
					fStopLength /= 60;
					// don't move the step from where it is, just move everything later
					m_NoteFieldEdit.ConvertHoldNotesTo2sAnd3s();
					NoteDataUtil::ShiftRows( m_NoteFieldEdit, GAMESTATE->m_fSongBeat + 0.003f, fStopLength );
					m_pSong->m_Timing.ShiftRows( GAMESTATE->m_fSongBeat + 0.003f, fStopLength );
					m_NoteFieldEdit.Convert2sAnd3sToHoldNotes();

				}
			// Hello and welcome to I FEEL STUPID :-)
			break;
			}
		default:
			ASSERT(0);
	};

}

void ScreenEdit::HandleEditNotesStatisticsChoice( EditNotesStatisticsChoice c, int* iAnswers )
{
	Steps* pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	Difficulty dc = (Difficulty)iAnswers[difficulty];
	pSteps->SetDifficulty( dc );
	int iMeter = iAnswers[meter]+1;
	pSteps->SetMeter( iMeter );
	
	switch( c )
	{
	case description:
		SCREENMAN->TextEntry( SM_None, "Edit notes description.\nPress Enter to confirm,\nEscape to cancel.", m_pSteps->GetDescription(), ChangeDescription, NULL );
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
	BackgroundChange newChange;

	FOREACH( BackgroundChange, m_pSong->m_BackgroundChanges, iter )
	{
		if( iter->m_fStartBeat == GAMESTATE->m_fSongBeat )
		{
			newChange = *iter;
			// delete the old change.  We'll add a new one below.
			m_pSong->m_BackgroundChanges.erase( iter );
			break;
		}
	}

	newChange.m_fStartBeat = GAMESTATE->m_fSongBeat;

	switch( c )
	{
	case add_random:
		newChange.m_sBGName = "-random-";
		break;
	case add_song_bganimation:
	case add_song_movie:
	case add_song_still:
	case add_global_random_movie:
	case add_global_bganimation:
	case add_global_visualization:
		newChange.m_sBGName = g_BGChange.rows[c].choices[iAnswers[c]];
		break;
	case delete_change:
		newChange.m_sBGName = "";
		break;
	default:
		break;
	};

	newChange.m_fRate = strtof( g_BGChange.rows[rate].choices[iAnswers[rate]], NULL )/100.f;
	newChange.m_bFadeLast = !!iAnswers[fade_last];
	newChange.m_bRewindMovie = !!iAnswers[rewind_movie];
	newChange.m_bLoop = !!iAnswers[loop];

	if( newChange.m_sBGName != "" )
		m_pSong->AddBackgroundChange( newChange );
}

void ScreenEdit::SetupCourseAttacks()
{
	/* This is the first beat that can be changed without it being visible.  Until
	 * we draw for the first time, any beat can be changed. */
	GAMESTATE->m_fLastDrawnBeat[PLAYER_1] = -100;

	// Put course options into effect.
	GAMESTATE->m_ModsToApply[PLAYER_1].clear();
	GAMESTATE->RemoveActiveAttacksForPlayer( PLAYER_1 );


	if( m_pAttacksFromCourse )
	{
		m_pAttacksFromCourse->LoadFromCRSFile( m_pAttacksFromCourse->m_sPath );

		AttackArray Attacks;
		for( unsigned e = 0; e < m_pAttacksFromCourse->m_entries.size(); ++e )
		{
			if( m_pAttacksFromCourse->m_entries[e].type != COURSE_ENTRY_FIXED )
				continue;
			if( m_pAttacksFromCourse->m_entries[e].pSong != m_pSong )
				continue;

			Attacks = m_pAttacksFromCourse->m_entries[e].attacks;
			break;
		}

		for( unsigned i=0; i<Attacks.size(); ++i )
			GAMESTATE->LaunchAttack( PLAYER_1, Attacks[i] );
	}
	GAMESTATE->RebuildPlayerOptionsFromActiveAttacks( PLAYER_1 );
}

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
