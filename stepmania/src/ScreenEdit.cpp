#include "stdafx.h"
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
#include "InputFilter.h"


//
// Defines specific to GameScreenTitleMenu
//

const float MAX_SECONDS_CAN_BE_OFF_BY	=	0.20f;
const float GRAY_ARROW_Y				= ARROW_SIZE * 1.5;

const float DEBUG_X			= SCREEN_LEFT + 10;
const float DEBUG_Y			= CENTER_Y-100;

const float HELP_X			= SCREEN_LEFT + 10;
const float HELP_Y			= SCREEN_BOTTOM - 10;

const float SHORTCUTS_X		= CENTER_X - 150;
const float SHORTCUTS_Y		= CENTER_Y;

const float INFO_X			= SCREEN_RIGHT - 10 ;
const float INFO_Y			= SCREEN_BOTTOM - 10;

const float MENU_WIDTH		=	110;
const float EDIT_X			=	CENTER_X;
const float EDIT_GRAY_Y		=	GRAY_ARROW_Y;

const float PLAYER_X		=	EDIT_X;
const float PLAYER_Y		=	SCREEN_TOP;

const float MENU_ITEM_X			=	CENTER_X-200;
const float MENU_ITEM_START_Y	=	SCREEN_TOP + 24;
const float MENU_ITEM_SPACING_Y	=	18;

const CString HELP_TEXT = 
	"Esc: show command menu\n"
//	"Hold F1 to show more commands\n"
	"Hold keys for faster change:\n"
	"F7/F8: Decrease/increase BPM at cur beat\n"
	"F9/F10: Dec/inc freeze secs at cur beat\n"
	"F11/F12: Decrease/increase music offset\n"
	"[/]: Dec/inc sample music start\n"
	"{/}: Dec/inc sample music length\n"
	"Up/Down: change beat\n"
	"Left/Right: change snap\n"
	"PgUp/PgDn: jump 1 measure\n"
	"Home/End: jump to first/last beat\n"
	"Number keys: add or remove tap note\n"
	"To create a hold note, hold a number key\n" 
	"      while changing the beat pressing Up/Down\n";

const CString SHORTCUT_TEXT = "";
/*	"S: save changes\n"
	"W: save as SM and DWI (lossy)\n"
	"Enter/Space: set begin/end selection markers\n"
	"G/H/J/K/L: Snap selection to nearest\n"
	"      4th / 8th / 12th / 16th / 12th or 16th\n"
	"P: Play selected area\n"
	"R: Record in selected area\n"
	"T: Toggle Play/Record rate\n"
	"X: Cut selected area\n"
	"C: Copy selected area\n"
	"V: Paste at current beat\n"
	"D: Toggle difficulty\n"
	"E: Edit description\n"
	"A: Edit main title\n"
	"U: Edit sub title\n"
	"B: Add/Edit background change\n"
	"Ins: Insert blank beat\n"
	"Del: Delete current beat and shift\n"
	"M: Play sample music\n";
*/

const CString MENU_ITEM_TEXT[NUM_MENU_ITEMS] = {
	"Enter:  Set begin marker (Enter)",
	"Space:  Set end marker (Space)",
	"P:      Play selection",
	"R:      Record in selection",
	"T:      Toggle Play/Record rate",
	"X:      Cut selection",
	"C:      Copy selection",
	"V:      Paste clipboard at current beat",
	"D:      Toggle difficulty",
	"E:      Edit description",
	"A:      Edit main title",
	"U:      Edit subtitle",
	"I:      Toggle assist tick",
	"B:      Add/Edit background change at current beat",
	"Ins:    Insert blank beat and shift down",
	"Del:    Delete blank beat and shift up",
	"G:      Snap selection to nearest quarter note",
	"H:      Snap selection to nearest eighth note",
	"J:      Snap selection to nearest triplet",
	"K:      Snap selection to nearest sixteenth note",
	"L:      Snap selection to nearest triplet or sixteenth note",
	"M:      Play sample music",
	"S:      Save changes as SM and DWI",
	"Q:      Quit"
};
int MENU_ITEM_KEY[NUM_MENU_ITEMS] = {
	DIK_RETURN,
	DIK_SPACE,
	DIK_P,
	DIK_R,
	DIK_T,
	DIK_X,
	DIK_C,
	DIK_V,
	DIK_D,
	DIK_E,
	DIK_A,
	DIK_U,
	DIK_I,
	DIK_B,
	DIK_INSERT,
	DIK_DELETE,
	DIK_G,
	DIK_H,
	DIK_J,
	DIK_K,
	DIK_L,
	DIK_M,
	DIK_S,
	DIK_Q,
};

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+2);


ScreenEdit::ScreenEdit()
{
	LOG->Trace( "ScreenEdit::ScreenEdit()" );

	// set both players to joined so the credit message doesn't show
	for( int p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_bSideIsJoined[p] = true;
	SCREENMAN->RefreshCreditsMessages();

	m_iRowLastCrossed = -1;

	m_pSong = GAMESTATE->m_pCurSong;

	m_pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
	if( m_pNotes == NULL )
	{
		m_pNotes = new Notes;
		m_pNotes->m_DifficultyClass = CLASS_MEDIUM;
		m_pNotes->m_NotesType = GAMESTATE->GetCurrentStyleDef()->m_NotesTypes[0];
		m_pNotes->m_sDescription = "Untitled";

		// In ScreenEditMenu, the screen preceding this one,
		// GAMEMAN->m_CurStyle is set to the target game style
		// of the current edit. Naturally, this is where we'll
		// want to extract the NotesType for a (NEW) sequence.

		m_pSong->m_apNotes.Add( m_pNotes );

		GAMESTATE->m_pCurNotes[PLAYER_1] = m_pNotes;
	}

	NoteData noteData;
	m_pNotes->GetNoteData( &noteData );


	m_EditMode = MODE_EDITING;

	GAMESTATE->m_bEditing = true;

	GAMESTATE->m_fSongBeat = 0;
	m_fTrailingBeat = GAMESTATE->m_fSongBeat;

	GAMESTATE->m_SongOptions.m_fMusicRate = 1;
	
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_fArrowScrollSpeed = 1;
	GAMESTATE->m_PlayerOptions[PLAYER_1].m_ColorType = PlayerOptions::COLOR_NOTE;

	m_sprBackground.Load( THEME->GetPathTo("Graphics","edit background") );
	m_sprBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );

	shiftAnchor = -1;
	m_SnapDisplay.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_SnapDisplay.Load( PLAYER_1 );
	m_SnapDisplay.SetZoom( 0.5f );

	m_GrayArrowRowEdit.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_GrayArrowRowEdit.Load( PLAYER_1 );
	m_GrayArrowRowEdit.SetZoom( 0.5f );

	m_NoteFieldEdit.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_NoteFieldEdit.SetZoom( 0.5f );
	m_NoteFieldEdit.Load( &noteData, PLAYER_1, 200, 800 );

	m_rectRecordBack.StretchTo( CRect(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_rectRecordBack.SetDiffuse( D3DXCOLOR(0,0,0,0) );

	m_GrayArrowRowRecord.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_GrayArrowRowRecord.Load( PLAYER_1 );
	m_GrayArrowRowRecord.SetZoom( 1.0f );

	m_NoteFieldRecord.SetXY( EDIT_X, EDIT_GRAY_Y );
	m_NoteFieldRecord.SetZoom( 1.0f );
	m_NoteFieldRecord.Load( &noteData, PLAYER_1, 60, 300 );

	m_Clipboard.m_iNumTracks = m_NoteFieldEdit.m_iNumTracks;

	m_Player.Load( PLAYER_1, &noteData, NULL, NULL );
	m_Player.SetXY( PLAYER_X, PLAYER_Y );

	m_Fade.SetClosed();

	m_textInfo.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textInfo.SetXY( INFO_X, INFO_Y );
	m_textInfo.SetHorizAlign( Actor::align_right );
	m_textInfo.SetVertAlign( Actor::align_bottom );
	m_textInfo.SetZoom( 0.5f );
	m_textInfo.SetShadowLength( 2 );
	//m_textInfo.SetText();	// set this below every frame

	m_textHelp.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textHelp.SetXY( HELP_X, HELP_Y );
	m_textHelp.SetHorizAlign( Actor::align_left );
	m_textHelp.SetVertAlign( Actor::align_bottom );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetShadowLength( 2 );
	m_textHelp.SetText( HELP_TEXT );

	m_rectShortcutsBack.StretchTo( CRect(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_rectShortcutsBack.SetDiffuse( D3DXCOLOR(0,0,0,0.8f) );

	m_textShortcuts.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textShortcuts.SetXY( SHORTCUTS_X, SHORTCUTS_Y );
	m_textShortcuts.SetHorizAlign( Actor::align_left );
	m_textShortcuts.SetVertAlign( Actor::align_middle );
	m_textShortcuts.SetZoom( 0.5f );
	m_textShortcuts.SetShadowLength( 2 );
	m_textShortcuts.SetText( SHORTCUT_TEXT );


	m_soundChangeLine.Load( THEME->GetPathTo("Sounds","edit change line"), 10 );
	m_soundChangeSnap.Load( THEME->GetPathTo("Sounds","edit change snap") );
	m_soundMarker.Load(		THEME->GetPathTo("Sounds","edit marker") );
	m_soundInvalid.Load(	THEME->GetPathTo("Sounds","menu invalid") );


	m_soundMusic.Load( m_pSong->GetMusicPath(), true );	// enable accurate sync

	m_soundAssistTick.Load(		THEME->GetPathTo("Sounds","gameplay assist tick") );

	for( int i=0; i<NUM_MENU_ITEMS; i++ )
	{
		m_textMenu[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textMenu[i].SetXY( MENU_ITEM_X, MENU_ITEM_START_Y + MENU_ITEM_SPACING_Y*i );
		m_textMenu[i].SetHorizAlign( Actor::align_left );
		m_textMenu[i].SetZoom( 0.5f );
		m_textMenu[i].SetShadowLength( 2 );
		m_textMenu[i].SetText( MENU_ITEM_TEXT[i] );
	}
	m_iMenuSelection = 0;

	m_Fade.OpenWipingRight();
}

ScreenEdit::~ScreenEdit()
{
	LOG->Trace( "ScreenEdit::~ScreenEdit()" );
	m_soundMusic.Stop();
}


void ScreenEdit::Update( float fDeltaTime )
{
	m_soundMusic.Update( fDeltaTime );

	float fPositionSeconds = m_soundMusic.GetPositionSeconds();
	float fSongBeat, fBPS;
	bool bFreeze;
	m_pSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );


	// update the global music statistics for other classes to access
	GAMESTATE->m_fMusicSeconds = fPositionSeconds;
//	GAMESTATE->m_fSongBeat = fSongBeat;
	GAMESTATE->m_fCurBPS = fBPS;
	GAMESTATE->m_bFreeze = bFreeze;


	if( m_EditMode == MODE_RECORDING  ||  m_EditMode == MODE_PLAYING )
	{
		GAMESTATE->m_fSongBeat = fSongBeat;

		if( GAMESTATE->m_fSongBeat > m_NoteFieldEdit.m_fEndMarker + 4 )		// give a one measure lead out
		{
			if( m_EditMode == MODE_RECORDING )
			{
				TransitionFromRecordToEdit();
				GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fEndMarker;
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0) );
			}
			else if( m_EditMode == MODE_PLAYING )
			{
				m_soundMusic.Stop();
				m_EditMode = MODE_EDITING;
				GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fEndMarker;
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0) );
			}
		}
	}

	m_sprBackground.Update( fDeltaTime );
	m_SnapDisplay.Update( fDeltaTime );
	m_GrayArrowRowEdit.Update( fDeltaTime );
	m_NoteFieldEdit.Update( fDeltaTime );
	m_Fade.Update( fDeltaTime );
	m_textHelp.Update( fDeltaTime );
	m_textInfo.Update( fDeltaTime );
	m_rectShortcutsBack.Update( fDeltaTime );
	m_textShortcuts.Update( fDeltaTime );

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
		float fMoveDelta = fSign*fDeltaTime*40;
		if( fabsf(fMoveDelta) > fabsf(fDelta) )
			fMoveDelta = fDelta;
		m_fTrailingBeat += fMoveDelta;

		if( fabsf(fDelta) > 10 )
			m_fTrailingBeat += fDelta * fDeltaTime*5;
	}

	if( m_EditMode == MODE_MENU )
	{
		for( int i=0; i<NUM_MENU_ITEMS; i++ )
			m_textMenu[i].Update( fDeltaTime );
	}

	m_NoteFieldEdit.Update( fDeltaTime );

	// 
	// play assist ticks
	//
	// Sound cards have a latency between when a sample is Play()ed and when the sound
	// will start coming out the speaker.  Compensate for this by boosting
	// fPositionSeconds ahead
	if( m_EditMode == MODE_PLAYING  &&  GAMESTATE->m_SongOptions.m_AssistType == SongOptions::ASSIST_TICK )
	{
		fPositionSeconds += (SOUND->GetPlayLatency()+0.018f) * m_soundMusic.GetPlaybackRate();	// HACK:  Add 0.015 seconds to account for the fact that the middle of the tick sounds occurs 0.015 seconds into playing.
		GAMESTATE->m_pCurSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS, bFreeze );

		int iRowNow = BeatToNoteRowNotRounded( fSongBeat );
		iRowNow = max( 0, iRowNow );
		static int iRowLastCrossed = 0;

		bool bAnyoneHasANote = false;	// set this to true if any player has a note at one of the indicies we crossed

		for( int r=iRowLastCrossed+1; r<=iRowNow; r++ )  // for each index we crossed since the last update
			bAnyoneHasANote |= m_Player.IsThereANoteAtRow( r );

		if( bAnyoneHasANote )
			m_soundAssistTick.Play();

		iRowLastCrossed = iRowNow;
	}



	CString sNoteType;
	switch( m_SnapDisplay.GetSnapMode() )
	{
	case NOTE_TYPE_4TH:		sNoteType = "quarter notes";		break;
	case NOTE_TYPE_8TH:		sNoteType = "eighth notes";			break;
	case NOTE_TYPE_12TH:	sNoteType = "triplets";				break;
	case NOTE_TYPE_16TH:	sNoteType = "sixteenth notes";		break;
	case NOTE_TYPE_24TH:	sNoteType = "twentyforth notes";	break;
	case NOTE_TYPE_32ND:	sNoteType = "thirtysecond notes";	break;
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

	m_textInfo.SetText( ssprintf(
		"Snap = %s\n"
		"Beat = %.2f\n"
		"Selection = begin: %.2f, end: %.2f\n"
		"Play/Record rate: %.1f\n"
		"Difficulty = %s\n"
		"Description = %s\n"
		"Main title = %s\n"
		"Sub title = %s\n"
		"Num notes tap: %d, hold: %d\n"
		"Assist tick is %s\n"
		"MusicOffsetSeconds: %.2f\n"
		"Preview start: %.2f, length = %.2f\n",
		sNoteType,
		GAMESTATE->m_fSongBeat,
		m_NoteFieldEdit.m_fBeginMarker,	m_NoteFieldEdit.m_fEndMarker,
		GAMESTATE->m_SongOptions.m_fMusicRate,
		DifficultyClassToString( m_pNotes->m_DifficultyClass ),
		GAMESTATE->m_pCurNotes[PLAYER_1] ? GAMESTATE->m_pCurNotes[PLAYER_1]->m_sDescription : "no description",
		m_pSong->m_sMainTitle,
		m_pSong->m_sSubTitle,
		iNumTapNotes, iNumHoldNotes,
		GAMESTATE->m_SongOptions.m_AssistType==SongOptions::ASSIST_TICK ? "ON" : "OFF",
		m_pSong->m_fBeat0OffsetInSeconds,
		m_pSong->m_fMusicSampleStartSeconds, m_pSong->m_fMusicSampleLengthSeconds
		) );
}


void ScreenEdit::DrawPrimitives()
{
	m_sprBackground.Draw();
	m_SnapDisplay.Draw();
	m_GrayArrowRowEdit.Draw();

	// HACK:  Make NoteFieldEdit draw using the trailing beat
	float fSongBeat = GAMESTATE->m_fSongBeat;	// save song beat
	GAMESTATE->m_fSongBeat = m_fTrailingBeat;	// put trailing beat in effect
	m_NoteFieldEdit.Draw();
	GAMESTATE->m_fSongBeat = fSongBeat;	// restore real song beat

	m_textHelp.Draw();
	m_textInfo.Draw();
	m_Fade.Draw();
	if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD,DIK_F1) ) )
	{
		m_rectShortcutsBack.Draw();
		m_textShortcuts.Draw();
	}

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

	if( m_EditMode == MODE_RECORDING )
	{
		/*
		for( int t=0; t<GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer; t++ )
		{
			if( m_bLayingAHold[t] )
			{
				bool bHoldingButton = false;
				for( int p=0; p<NUM_PLAYERS; p++ )
					bHoldingButton |= INPUTMAPPER->IsButtonDown( StyleInput(PlayerInput(p), t) );
				if( bHoldingButton 
			}
		}
		*/
	}

	if( m_EditMode == MODE_MENU )
	{
		for( int i=0; i<NUM_MENU_ITEMS; i++ )
			m_textMenu[i].Draw();
	}

	Screen::DrawPrimitives();
}

void ScreenEdit::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEdit::Input()" );

	switch( m_EditMode )
	{
	case MODE_EDITING:		InputEdit( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_MENU:			InputMenu( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_RECORDING:	InputRecord( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_PLAYING:		InputPlay( DeviceI, type, GameI, MenuI, StyleI );	break;
	default:	ASSERT(0);
	}
}

// Begin helper functions for InputEdit

void AddBGChange( CString sBGName )
{
	Song* pSong = GAMESTATE->m_pCurSong;

	for( int i=0; i<pSong->m_BackgroundChanges.GetSize(); i++ )
	{
		if( pSong->m_BackgroundChanges[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
			break;
	}

	if( i != pSong->m_BackgroundChanges.GetSize() )	// there is already a BGChange here
		pSong->m_BackgroundChanges.RemoveAt( i );

	// create a new BGChange
	if( sBGName != "" )
		pSong->AddBackgroundChange( BackgroundChange(GAMESTATE->m_fSongBeat, sBGName) );
}

void ChangeDescription( CString sNew )
{
	Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
	pNotes->m_sDescription = sNew;
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

// End helper functions for InputEdit

void ScreenEdit::InputEdit( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.device != DEVICE_KEYBOARD )
		return;

	if(type == IET_RELEASE)
	{
		switch( DeviceI.button ) {
		case DIK_LSHIFT:
		case DIK_RSHIFT:
			shiftAnchor = -1;
			break;
		}
		return;
	}

	switch( DeviceI.button )
	{
	case DIK_1:
	case DIK_2:
	case DIK_3:
	case DIK_4:
	case DIK_5:
	case DIK_6:
	case DIK_7:
	case DIK_8:
	case DIK_9:
	case DIK_0:
		{
			if( type != IET_FIRST_PRESS )
				break;	// We only care about first presses

			int iCol = DeviceI.button - DIK_1;
			const float fSongBeat = GAMESTATE->m_fSongBeat;
			const int iSongIndex = BeatToNoteRow( fSongBeat );

			if( iCol >= m_NoteFieldEdit.m_iNumTracks )	// this button is not in the range of columns for this StyleDef
				break;

			// check for to see if the user intended to remove a HoldNote
			bool bRemovedAHoldNote = false;
			for( int i=0; i<m_NoteFieldEdit.m_iNumHoldNotes; i++ )	// for each HoldNote
			{
				HoldNote &hn = m_NoteFieldEdit.m_HoldNotes[i];
				if( iCol == hn.m_iTrack  &&		// the notes correspond
					fSongBeat >= hn.m_fStartBeat  &&  fSongBeat <= hn.m_fEndBeat )	// the cursor lies within this HoldNote
				{
					m_NoteFieldEdit.RemoveHoldNote( i );
					bRemovedAHoldNote = true;
					break;	// stop iterating over all HoldNotes
				}
			}

			if( !bRemovedAHoldNote )
			{
				// We didn't remove a HoldNote, so the user wants to add or delete a TapNote
				if( m_NoteFieldEdit.m_TapNotes[iCol][iSongIndex] == '0' )
					m_NoteFieldEdit.m_TapNotes[iCol][iSongIndex] = '1';
				else
					m_NoteFieldEdit.m_TapNotes[iCol][iSongIndex] = '0';
			}
		}
		break;
	case DIK_Q:
		SCREENMAN->SetNewScreen( "ScreenEditMenu" );
		break;
	case DIK_S:
		{
			// copy edit into current Notes
//			Song* pSong = GAMESTATE->m_pCurSong;
			Notes* pNotes = GAMESTATE->m_pCurNotes[PLAYER_1];
			
			// strip out the autogen marker if any.  The autogen marker would have caused these Notes not to be saved to disk.
			pNotes->m_sDescription.Replace( " (autogen)", "" );

			ASSERT( pNotes );
/*
			if( pNotes == NULL )
			{
				// allocate a new Notes
				pNotes = new Notes;
				pSong->m_apNotes.Add( pNotes );
				pNotes->m_NotesType = GAMEMAN->GetStyleDefForStyle( GAMESTATE->m_CurStyle )->m_NotesType;
				pNotes->m_sDescription = "Untitled";
				pNotes->m_iMeter = 1;
			}
*/
			pNotes->SetNoteData( (NoteData*)&m_NoteFieldEdit );
			GAMESTATE->m_pCurSong->Save();
			SCREENMAN->SystemMessage( "Saved as SM and DWI." );

			SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","edit save") );
		}
		break;
	case DIK_UP:
	case DIK_DOWN:
	case DIK_PGUP:
	case DIK_PGDN:
		{
			float fBeatsToMove=0.f;
			switch( DeviceI.button )
			{
			case DIK_UP:
			case DIK_DOWN:
				fBeatsToMove = NoteTypeToBeat( m_SnapDisplay.GetSnapMode() );
				if( DeviceI.button == DIK_UP )	
					fBeatsToMove *= -1;
			break;
			case DIK_PGUP:
			case DIK_PGDN:
				fBeatsToMove = BEATS_PER_MEASURE;
				if( DeviceI.button == DIK_PGUP )	
					fBeatsToMove *= -1;
			}

			const float fStartBeat = GAMESTATE->m_fSongBeat;
			const float fEndBeat = GAMESTATE->m_fSongBeat + fBeatsToMove;

			// check to see if they're holding a button
			for( int col=0; col<m_NoteFieldEdit.m_iNumTracks && col<=10; col++ )
			{
				const DeviceInput di(DEVICE_KEYBOARD, DIK_1+col);

				if( !INPUTFILTER->IsBeingPressed(di) )
					continue;

				// create a new hold note
				HoldNote newHN;
				newHN.m_iTrack = col;
				newHN.m_fStartBeat = min(fStartBeat, fEndBeat);
				newHN.m_fEndBeat = max(fStartBeat, fEndBeat);

				newHN.m_fStartBeat = max(newHN.m_fStartBeat, 0);
				newHN.m_fEndBeat = max(newHN.m_fEndBeat, 0);

				m_NoteFieldEdit.AddHoldNote( newHN );
			}

			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, DIK_LSHIFT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, DIK_RSHIFT)))
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
			GAMESTATE->m_fSongBeat = froundf( GAMESTATE->m_fSongBeat, NoteTypeToBeat(m_SnapDisplay.GetSnapMode()) );
			m_soundChangeLine.Play();
		}
		break;
	case DIK_HOME:
		GAMESTATE->m_fSongBeat = 0;
		m_soundChangeLine.Play();
		break;
	case DIK_END:
		GAMESTATE->m_fSongBeat = m_NoteFieldEdit.GetLastBeat();
		m_soundChangeLine.Play();
		break;
	case DIK_RIGHT:
		m_SnapDisplay.PrevSnapMode();
		OnSnapModeChange();
		break;
	case DIK_LEFT:
		m_SnapDisplay.NextSnapMode();
		OnSnapModeChange();
		break;
	case DIK_RETURN:
		if( m_NoteFieldEdit.m_fEndMarker != -1  &&  GAMESTATE->m_fSongBeat > m_NoteFieldEdit.m_fEndMarker )
		{
			// invalid!  The begin maker must be placed before the end marker
			m_soundInvalid.Play();
		}
		else
		{
			m_NoteFieldEdit.m_fBeginMarker = GAMESTATE->m_fSongBeat;
			m_soundMarker.Play();
		}
		break;
	case DIK_SPACE:
		if( m_NoteFieldEdit.m_fBeginMarker != -1  &&  GAMESTATE->m_fSongBeat < m_NoteFieldEdit.m_fBeginMarker )
		{
			// invalid!  The end maker must be placed after the begin marker
			m_soundInvalid.Play();
		}
		else
		{
			m_NoteFieldEdit.m_fEndMarker = GAMESTATE->m_fSongBeat;
			m_soundMarker.Play();
		}
		break;
	case DIK_G:
	case DIK_H:
	case DIK_J:
	case DIK_K:
	case DIK_L:
		{
			if( m_NoteFieldEdit.m_fBeginMarker == -1  ||  m_NoteFieldEdit.m_fEndMarker == -1 )
			{
				m_soundInvalid.Play();
			}
			else
			{
				NoteType noteType1;
				NoteType noteType2;
				switch( DeviceI.button )
				{
					case DIK_G:	noteType1 = NOTE_TYPE_4TH;	noteType2 = NOTE_TYPE_4TH;	break;
					case DIK_H:	noteType1 = NOTE_TYPE_8TH;	noteType2 = NOTE_TYPE_8TH;	break;
					case DIK_J:	noteType1 = NOTE_TYPE_12TH;	noteType2 = NOTE_TYPE_12TH;	break;
					case DIK_K:	noteType1 = NOTE_TYPE_16TH;	noteType2 = NOTE_TYPE_16TH;	break;
					case DIK_L:	noteType1 = NOTE_TYPE_12TH;	noteType2 = NOTE_TYPE_16TH;	break;
					default:	ASSERT( false );		return;
				}

				m_NoteFieldEdit.SnapToNearestNoteType( noteType1, noteType2, m_NoteFieldEdit.m_fBeginMarker, m_NoteFieldEdit.m_fEndMarker );
			}
		}
		break;
	case DIK_ESCAPE:
		{
			m_EditMode = MODE_MENU;
			m_iMenuSelection = 0;
			MenuItemGainFocus( m_iMenuSelection );

			m_rectRecordBack.BeginTweening( 0.5f );
			m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0.8f) );
		}
		break;
	case DIK_P:
		{
			if( m_NoteFieldEdit.m_fBeginMarker == -1 )
				m_NoteFieldEdit.m_fBeginMarker = m_pSong->m_fFirstBeat;
			if( m_NoteFieldEdit.m_fEndMarker == -1 )
				m_NoteFieldEdit.m_fEndMarker = m_pSong->m_fLastBeat;

			m_EditMode = MODE_PLAYING;

			m_Player.Load( PLAYER_1, (NoteData*)&m_NoteFieldEdit, NULL, NULL );

			m_rectRecordBack.BeginTweening( 0.5f );
			m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0.8f) );

			GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fBeginMarker - 4;	// give a 1 measure lead-in
			const float fStartSeconds = m_pSong->GetElapsedTimeFromBeat( GAMESTATE->m_fSongBeat );
			m_soundMusic.SetPositionSeconds( fStartSeconds );
			m_soundMusic.Play();
			m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
		}
		break;
	case DIK_R:
		{
			if( m_NoteFieldEdit.m_fBeginMarker == -1 )
				m_NoteFieldEdit.m_fBeginMarker = m_pSong->m_fFirstBeat;
			if( m_NoteFieldEdit.m_fEndMarker == -1 )
				m_NoteFieldEdit.m_fEndMarker = m_pSong->m_fLastBeat;


			// initialize m_NoteFieldRecord
			m_NoteFieldRecord.ClearAll();
			m_NoteFieldRecord.m_iNumTracks = m_NoteFieldEdit.m_iNumTracks;
			m_NoteFieldRecord.m_fBeginMarker = m_NoteFieldEdit.m_fBeginMarker;
			m_NoteFieldRecord.m_fEndMarker = m_NoteFieldEdit.m_fEndMarker;


			m_EditMode = MODE_RECORDING;
			m_rectRecordBack.BeginTweening( 0.5f );
			m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0.8f) );

			GAMESTATE->m_fSongBeat = m_NoteFieldEdit.m_fBeginMarker - 4;	// give a 1 measure lead-in
			float fElapsedSeconds = max( 0, m_pSong->GetElapsedTimeFromBeat(GAMESTATE->m_fSongBeat) );
			m_soundMusic.SetPositionSeconds( fElapsedSeconds );
			m_soundMusic.Play();
			m_soundMusic.SetPlaybackRate( GAMESTATE->m_SongOptions.m_fMusicRate );
		}
		break;
	case DIK_T:
		if(     GAMESTATE->m_SongOptions.m_fMusicRate == 1.0f )		GAMESTATE->m_SongOptions.m_fMusicRate = 0.9f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 0.9f )	GAMESTATE->m_SongOptions.m_fMusicRate = 0.8f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 0.8f )	GAMESTATE->m_SongOptions.m_fMusicRate = 0.7f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 0.7f )	GAMESTATE->m_SongOptions.m_fMusicRate = 1.5f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 1.5f )	GAMESTATE->m_SongOptions.m_fMusicRate = 1.4f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 1.4f )	GAMESTATE->m_SongOptions.m_fMusicRate = 1.3f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 1.3f )	GAMESTATE->m_SongOptions.m_fMusicRate = 1.2f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 1.2f )	GAMESTATE->m_SongOptions.m_fMusicRate = 1.1f;
		else if( GAMESTATE->m_SongOptions.m_fMusicRate == 1.1f )	GAMESTATE->m_SongOptions.m_fMusicRate = 1.0f;
		break;
	case DIK_INSERT:
	case DIK_DELETE:
		{
			NoteData temp;
			temp.m_iNumTracks = m_NoteFieldEdit.m_iNumTracks;
			int iTakeFromRow=0;
			int iPasteAtRow;
			switch( DeviceI.button )
			{
			case DIK_INSERT:
				iTakeFromRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
				iPasteAtRow = BeatToNoteRow( GAMESTATE->m_fSongBeat+1 );
				break;
			case DIK_DELETE:
				iTakeFromRow = BeatToNoteRow( GAMESTATE->m_fSongBeat+1 );
				iPasteAtRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
				break;
			}
			temp.CopyRange( &m_NoteFieldEdit, iTakeFromRow, MAX_TAP_NOTE_ROWS );
			m_NoteFieldEdit.ClearRange( min(iTakeFromRow,iPasteAtRow), MAX_TAP_NOTE_ROWS  );
			m_NoteFieldEdit.CopyRange( &temp, 0, MAX_TAP_NOTE_ROWS-iTakeFromRow, iPasteAtRow );
		}
		break;
	case DIK_X:
		if( m_NoteFieldEdit.m_fBeginMarker == -1  ||  m_NoteFieldEdit.m_fEndMarker == -1 )
		{
			m_soundInvalid.Play();
		}
		else
		{
			int iFirstRow = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
			int iLastRow  = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );

			m_Clipboard.CopyRange( &m_NoteFieldEdit, iFirstRow, iLastRow );
			m_NoteFieldEdit.ClearRange( iFirstRow, iLastRow  );
		}
		break;
	case DIK_C:
		if( m_NoteFieldEdit.m_fBeginMarker == -1  ||  m_NoteFieldEdit.m_fEndMarker == -1 )
		{
			m_soundInvalid.Play();
		}
		else
		{
			int iFirstRow = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
			int iLastRow  = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );

			m_Clipboard.CopyRange( &m_NoteFieldEdit, iFirstRow, iLastRow );
		}
		break;
	case DIK_V:
		{
			int iSrcFirstRow = 0;
			int iSrcLastRow  = BeatToNoteRow( m_Clipboard.GetLastBeat() );
			int iDestFirstRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );

			m_NoteFieldEdit.CopyRange( &m_Clipboard, iSrcFirstRow, iSrcLastRow, iDestFirstRow );
		}
		break;

	case DIK_D:
		{
			DifficultyClass &dc = m_pNotes->m_DifficultyClass;
			dc = DifficultyClass( (dc+1)%NUM_DIFFICULTY_CLASSES );
		}
		break;

	case DIK_E:
		SCREENMAN->TextEntry( SM_None, "Edit notes description.\nPress Enter to confirm,\nEscape to cancel.", m_pNotes->m_sDescription, ChangeDescription, NULL );
		break;

	case DIK_A:
		SCREENMAN->TextEntry( SM_None, "Edit song main title.\nPress Enter to confirm,\nEscape to cancel.", m_pSong->m_sMainTitle, ChangeMainTitle, NULL );
		break;

	case DIK_U:
		SCREENMAN->TextEntry( SM_None, "Edit song sub title.\nPress Enter to confirm,\nEscape to cancel.", m_pSong->m_sSubTitle, ChangeSubTitle, NULL );
		break;

	case DIK_B:
		{
			CString sOldBackground;
			for( int i=0; i<m_pSong->m_BackgroundChanges.GetSize(); i++ )
			{
				if( m_pSong->m_BackgroundChanges[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
					break;
			}
			if( i != m_pSong->m_BackgroundChanges.GetSize() )	// there is already a BGChange here
				sOldBackground = m_pSong->m_BackgroundChanges[i].m_sBGName;

			SCREENMAN->TextEntry( SM_None, "Type a background name.\nPress Enter to keep,\nEscape to cancel.\nEnter an empty string to remove\nthe Background Change.", sOldBackground, AddBGChange, NULL );
		}
		break;

	case DIK_I:
		if( GAMESTATE->m_SongOptions.m_AssistType==SongOptions::ASSIST_TICK )
			GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_NONE;
		else
			GAMESTATE->m_SongOptions.m_AssistType = SongOptions::ASSIST_TICK;
		break;

	case DIK_F7:
	case DIK_F8:
		{
			float fBPM = m_pSong->GetBPMAtBeat( GAMESTATE->m_fSongBeat );
			float fDeltaBPM;
			switch( DeviceI.button )
			{
			case DIK_F7:	fDeltaBPM = - 0.020f;		break;
			case DIK_F8:	fDeltaBPM = + 0.020f;		break;
			default:	ASSERT(0);						return;
			}
			switch( type )
			{
			case IET_SLOW_REPEAT:	fDeltaBPM *= 10;	break;
			case IET_FAST_REPEAT:	fDeltaBPM *= 40;	break;
			}
			float fNewBPM = fBPM + fDeltaBPM;

			for( int i=0; i<m_pSong->m_BPMSegments.GetSize(); i++ )
				if( m_pSong->m_BPMSegments[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
					break;

			if( i == m_pSong->m_BPMSegments.GetSize() )	// there is no BPMSegment at the current beat
			{
				// create a new BPMSegment
				m_pSong->AddBPMSegment( BPMSegment(GAMESTATE->m_fSongBeat, fNewBPM) );
			}
			else	// BPMSegment being modified is m_BPMSegments[i]
			{
				if( i > 0  &&  fabsf(m_pSong->m_BPMSegments[i-1].m_fBPM - fNewBPM) < 0.025f )
					m_pSong->m_BPMSegments.RemoveAt( i );
				else
					m_pSong->m_BPMSegments[i].m_fBPM = fNewBPM;
			}
		}
		break;
	case DIK_F9:
	case DIK_F10:
		{
			float fStopDelta;
			switch( DeviceI.button )
			{
			case DIK_F9:	fStopDelta = -0.020f;		break;
			case DIK_F10:	fStopDelta = +0.020f;		break;
			default:	ASSERT(0);						return;
			}
			switch( type )
			{
			case IET_SLOW_REPEAT:	fStopDelta *= 10;	break;
			case IET_FAST_REPEAT:	fStopDelta *= 40;	break;
			}

			for( int i=0; i<m_pSong->m_StopSegments.GetSize(); i++ )
			{
				if( m_pSong->m_StopSegments[i].m_fStartBeat == GAMESTATE->m_fSongBeat )
					break;
			}

			if( i == m_pSong->m_StopSegments.GetSize() )	// there is no BPMSegment at the current beat
			{
				// create a new StopSegment
				if( fStopDelta > 0 )
					m_pSong->AddStopSegment( StopSegment(GAMESTATE->m_fSongBeat, fStopDelta) );
			}
			else	// StopSegment being modified is m_StopSegments[i]
			{
				m_pSong->m_StopSegments[i].m_fStopSeconds += fStopDelta;
				if( m_pSong->m_StopSegments[i].m_fStopSeconds <= 0 )
					m_pSong->m_StopSegments.RemoveAt( i );
			}
		}
		break;
	case DIK_F11:
	case DIK_F12:
		{
			float fOffsetDelta;
			switch( DeviceI.button )
			{
			case DIK_F11:	fOffsetDelta = -0.020f;		break;
			case DIK_F12:	fOffsetDelta = +0.020f;		break;
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
	case DIK_M:
		MUSIC->Load( m_pSong->GetMusicPath() );
		MUSIC->Play( false, m_pSong->m_fMusicSampleStartSeconds, m_pSong->m_fMusicSampleLengthSeconds );
		break;
	case DIK_LBRACKET:
	case DIK_RBRACKET:
		{
			float fDelta;
			switch( DeviceI.button )
			{
			case DIK_LBRACKET:		fDelta = -0.025f;	break;
			case DIK_RBRACKET:		fDelta = +0.025f;	break;
			default:	ASSERT(0);						return;
			}
			switch( type )
			{
			case IET_SLOW_REPEAT:	fDelta *= 10;	break;
			case IET_FAST_REPEAT:	fDelta *= 40;	break;
			}

			if( INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, DIK_LSHIFT)) ||
				INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, DIK_RSHIFT)))
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
	if(type == IET_RELEASE) return; // don't care
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case DIK_ESCAPE:
			TransitionFromRecordToEdit();
			
			m_rectRecordBack.BeginTweening( 0.5f );
			m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0) );

			break;
		}
	}
	switch( StyleI.player )
	{
		case PLAYER_1:
			int iCol;
			iCol = StyleI.col;
		
			int iNoteIndex;
			iNoteIndex = BeatToNoteRow( GAMESTATE->m_fSongBeat );

			if( type == IET_FIRST_PRESS )
			{
				m_NoteFieldRecord.m_TapNotes[iCol][iNoteIndex] = '1';
				m_NoteFieldRecord.SnapToNearestNoteType( NOTE_TYPE_12TH, NOTE_TYPE_16TH, max(0,GAMESTATE->m_fSongBeat-1), GAMESTATE->m_fSongBeat+1);
				m_GrayArrowRowRecord.Step( iCol );
			}
			else
			{
				const float fHoldEndSeconds = m_soundMusic.GetPositionSeconds();
				const float fHoldStartSeconds = m_soundMusic.GetPositionSeconds() - TIME_BEFORE_SLOW_REPEATS * m_soundMusic.GetPlaybackRate();

				float fStartBeat, fEndBeat, fThrowAway;
				bool bFreeze;
				m_pSong->GetBeatAndBPSFromElapsedTime( fHoldStartSeconds, fStartBeat, fThrowAway, bFreeze );
				m_pSong->GetBeatAndBPSFromElapsedTime( fHoldEndSeconds, fEndBeat, fThrowAway, bFreeze );

				// create a new hold note
				HoldNote newHN;
				newHN.m_iTrack = iCol;
				newHN.m_fStartBeat = fStartBeat;
				newHN.m_fEndBeat = fEndBeat;

				m_NoteFieldRecord.AddHoldNote( newHN );
				m_NoteFieldRecord.SnapToNearestNoteType( NOTE_TYPE_12TH, NOTE_TYPE_16TH, max(0,GAMESTATE->m_fSongBeat-2), GAMESTATE->m_fSongBeat+2);
			}
			break;
	}
}

void ScreenEdit::InputMenu( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.device != DEVICE_KEYBOARD )
		return;
	if(type == IET_RELEASE) return; // don't care

	switch( DeviceI.button )
	{
	case DIK_UP:
		MenuItemLoseFocus( m_iMenuSelection );
		m_iMenuSelection = (m_iMenuSelection-1+NUM_MENU_ITEMS) % NUM_MENU_ITEMS;
		printf( "%d\n", m_iMenuSelection );
		MenuItemGainFocus( m_iMenuSelection );
		break;
	case DIK_DOWN:
		MenuItemLoseFocus( m_iMenuSelection );
		m_iMenuSelection = (m_iMenuSelection+1) % NUM_MENU_ITEMS;
		printf( "%d\n", m_iMenuSelection );
		MenuItemGainFocus( m_iMenuSelection );
		break;
	case DIK_RETURN:
	case DIK_ESCAPE:
		m_EditMode = MODE_EDITING;
		m_rectRecordBack.BeginTweening( 0.5f );
		m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0) );
		MenuItemLoseFocus( m_iMenuSelection );
		if( DeviceI.button == DIK_RETURN )
		{
			SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
			int iMenuKey = MENU_ITEM_KEY[m_iMenuSelection];
			InputEdit( DeviceInput(DEVICE_KEYBOARD,iMenuKey), IET_FIRST_PRESS, GameInput(), MenuInput(), StyleInput() );
		}
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
		case DIK_ESCAPE:
			m_EditMode = MODE_EDITING;
			m_soundMusic.Stop();
			m_rectRecordBack.BeginTweening( 0.5f );
			m_rectRecordBack.SetTweenDiffuse( D3DXCOLOR(0,0,0,0) );

			GAMESTATE->m_fSongBeat = froundf( GAMESTATE->m_fSongBeat, NoteTypeToBeat(m_SnapDisplay.GetSnapMode()) );
			break;
		case DIK_F11:
		case DIK_F12:
			{
				float fOffsetDelta;
				switch( DeviceI.button )
				{
				case DIK_F11:	fOffsetDelta = -0.020f;		break;
				case DIK_F12:	fOffsetDelta = +0.020f;		break;
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


void ScreenEdit::TransitionFromRecordToEdit()
{
	m_EditMode = MODE_EDITING;
	m_soundMusic.Stop();

	int iNoteIndexBegin = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
	int iNoteIndexEnd = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );

	// delete old TapNotes in the range
	m_NoteFieldEdit.ClearRange( iNoteIndexBegin, iNoteIndexEnd );

	m_NoteFieldEdit.CopyRange( (NoteData*)&m_NoteFieldRecord, iNoteIndexBegin, iNoteIndexEnd, iNoteIndexBegin );

	GAMESTATE->m_fSongBeat = froundf( GAMESTATE->m_fSongBeat, NoteTypeToBeat(m_SnapDisplay.GetSnapMode()) );
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
	}


}

void ScreenEdit::OnSnapModeChange()
{
	m_soundChangeSnap.Play();
			
	NoteType nt = m_SnapDisplay.GetSnapMode();
	int iStepIndex = BeatToNoteRow( GAMESTATE->m_fSongBeat );
	int iElementsPerNoteType = BeatToNoteRow( NoteTypeToBeat(nt) );
	int iStepIndexHangover = iStepIndex % iElementsPerNoteType;
	GAMESTATE->m_fSongBeat -= NoteRowToBeat( iStepIndexHangover );
}

void ScreenEdit::MenuItemGainFocus( int iItemIndex )
{
	m_textMenu[iItemIndex].SetEffectCamelion( 2.5, D3DXCOLOR(1,1,1,1), D3DXCOLOR(0,1,0,1) );
	m_textMenu[iItemIndex].SetZoom( 0.7f );
	SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","edit menu change") );
}

void ScreenEdit::MenuItemLoseFocus( int iItemIndex )
{
	m_textMenu[iItemIndex].SetEffectNone();
	m_textMenu[iItemIndex].SetZoom( 0.5f );
}
