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
#include "ScreenSelectMusic.h"
#include "ScreenEvaluation.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "GameManager.h"
#include "ScreenEditMenu.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"


//
// Defines specific to GameScreenTitleMenu
//


const float MAX_SECONDS_CAN_BE_OFF_BY	=	0.20f;
const float GRAY_ARROW_Y					= ARROW_SIZE * 1.5;

const float DEBUG_X			=	SCREEN_LEFT + 10;
const float DEBUG_Y			=	CENTER_Y-100;

const float HELP_X	= SCREEN_LEFT + 10;
const float HELP_Y	= CENTER_Y;	// top aligned

const float INFO_X	= SCREEN_RIGHT - 10 ;
const float INFO_Y	= SCREEN_BOTTOM - 10;		// bottom aligned

const float MENU_WIDTH		=	110;
const float EDIT_CENTER_X	=	CENTER_X + 100;

const float EDIT_GRAY_Y		=	CENTER_Y - 2.0f * (float)ARROW_SIZE;

const CString HELP_TEXT = 
	"Esc: exit\n"
	"S: save changes\n"
	"Up/Down: change beat\n"
	"Left/Right: change snap\n"
	"PgUp/PgDn: jump 1 measure\n"
	"Home/End: jump to first/last beat\n"
	"Number keys: add or remove tap note\n"
	"To create a hold note, hold a number key\n" 
	"      while changing the beat pressing Up/Down\n"
	"Enter/Space: set begin/end selection markers\n"
	"G/H/J/K/L: Snap selected notes to nearest\n"
	"      4th / 8th / 12th / 16th / 12th or 16th\n"
	"P: Play selected area\n"
	"R: Record in selected area\n"
	"X: Cut selected area\n"
	"C: Copy selected area\n"
	"V: Paste at current beat\n"
	"D: Toggle difficulty\n"
	"Ins: Insert blank beat\n"
	"Del: Delete current beat and shift\n"
	"F1,F2/F3,F4: Decrease/increase BPM at cur beat\n"
	"F5,F6/F7,F8: Dec/inc freeze secs at cur beat\n"
	"F9,F10/F11,F12: Decrease/increase music offset\n"
	"NumPad / * - + : Dec/inc music preview start\n"
	"NumPad 0/period: Dec/inc music length\n"
	"M: Play music preview\n";



const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);


ScreenEdit::ScreenEdit()
{
	LOG->WriteLine( "ScreenEdit::ScreenEdit()" );

	m_pSong = SONGMAN->GetCurrentSong();

	m_Mode = MODE_EDIT;

	m_fBeat = 0.0f;
	m_fTrailingBeat = m_fBeat;
	
	m_PlayerOptions.m_fArrowScrollSpeed = 1;
	m_PlayerOptions.m_ColorType = PlayerOptions::COLOR_NOTE;
//	m_PlayerOptions.m_bShowMeasureBars = true;

	m_DifficultyClass = CLASS_EASY;

	m_sprBackground.Load( THEME->GetPathTo( GRAPHIC_EDIT_BACKGROUND ) );
	m_sprBackground.StretchTo( CRect(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );


	m_GranularityIndicator.SetXY( EDIT_CENTER_X, EDIT_GRAY_Y );
	m_GranularityIndicator.Load();
	m_GranularityIndicator.SetZoom( 0.5f );

	m_GrayArrowRowEdit.SetXY( EDIT_CENTER_X, EDIT_GRAY_Y );
	m_GrayArrowRowEdit.Load( PLAYER_1, GAMEMAN->GetCurrentStyleDef(), m_PlayerOptions );
	m_GrayArrowRowEdit.SetZoom( 0.5f );

	NoteData noteData;
	noteData.m_iNumTracks = GAMEMAN->GetCurrentStyleDef()->m_iColsPerPlayer;
	if( SONGMAN->m_pCurNotes[PLAYER_1] != NULL )
		noteData = *SONGMAN->GetCurrentNotes(PLAYER_1)->GetNoteData();

	m_NoteFieldEdit.SetXY( EDIT_CENTER_X, EDIT_GRAY_Y );
	m_NoteFieldEdit.SetZoom( 0.5f );
	m_NoteFieldEdit.Load( &noteData, PLAYER_1, GAMEMAN->GetCurrentStyleDef(), m_PlayerOptions, 10, 12, NoteField::MODE_EDITING );

	m_rectRecordBack.StretchTo( CRect(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );
	m_rectRecordBack.SetDiffuseColor( D3DXCOLOR(0,0,0,0) );

	m_GrayArrowRowRecord.SetXY( EDIT_CENTER_X, EDIT_GRAY_Y );
	m_GrayArrowRowRecord.Load( PLAYER_1, GAMEMAN->GetCurrentStyleDef(), m_PlayerOptions );
	m_GrayArrowRowRecord.SetZoom( 1.0f );

	m_NoteFieldRecord.SetXY( EDIT_CENTER_X, EDIT_GRAY_Y );
	m_NoteFieldRecord.SetZoom( 1.0f );
	m_NoteFieldRecord.Load( &noteData, PLAYER_1, GAMEMAN->GetCurrentStyleDef(), m_PlayerOptions, 2, 5, NoteField::MODE_EDITING );

	m_Player.Load( PLAYER_1, GAMEMAN->GetCurrentStyleDef(), &noteData, PlayerOptions(), NULL, NULL, 1 );
	m_Player.SetXY( EDIT_CENTER_X, EDIT_GRAY_Y );

	m_Fade.SetClosed();

	m_textInfo.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textInfo.SetXY( INFO_X, INFO_Y );
	m_textInfo.SetHorizAlign( Actor::align_right );
	m_textInfo.SetVertAlign( Actor::align_top );
	m_textInfo.SetZoom( 0.5f );
	m_textInfo.SetShadowLength( 2 );
	//m_textInfo.SetText();	// set this below every frame

	m_textHelp.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textHelp.SetXY( HELP_X, HELP_Y );
	m_textHelp.SetHorizAlign( Actor::align_left );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetShadowLength( 2 );
	m_textHelp.SetText( HELP_TEXT );


	m_soundChangeLine.Load( THEME->GetPathTo(SOUND_EDIT_CHANGE_LINE), 10 );
	m_soundChangeSnap.Load( THEME->GetPathTo(SOUND_EDIT_CHANGE_SNAP) );
	m_soundMarker.Load( THEME->GetPathTo(SOUND_EDIT_CHANGE_SNAP) );
	m_soundInvalid.Load( THEME->GetPathTo(SOUND_MENU_INVALID) );


	m_soundMusic.Load( m_pSong->GetMusicPath() );
	m_soundMusic.SetPlaybackRate( 0.5f );


	m_Fade.OpenWipingRight();
}

ScreenEdit::~ScreenEdit()
{
	LOG->WriteLine( "ScreenEdit::~ScreenEdit()" );
	m_soundMusic.Stop();
}


void ScreenEdit::Update( float fDeltaTime )
{
	float fSongBeat, fBPS;
	float fPositionSeconds = m_soundMusic.GetPositionSeconds();
	m_pSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS );

	if( m_Mode == MODE_RECORD  ||  m_Mode == MODE_PLAY )
	{
		m_fBeat = fSongBeat;

		if( m_fBeat > m_NoteFieldEdit.m_fEndMarker )
		{
			if( m_Mode == MODE_RECORD )
			{
				TransitionToEditFromRecord();
			}
			else if( m_Mode == MODE_PLAY )
			{
				m_soundMusic.Stop();
				m_Mode = MODE_EDIT;
			}
		}
	}

	m_sprBackground.Update( fDeltaTime );
	m_GranularityIndicator.Update( fDeltaTime );
	m_GrayArrowRowEdit.Update( fDeltaTime, m_fBeat );
	m_NoteFieldEdit.Update( fDeltaTime, m_fBeat );
	m_Fade.Update( fDeltaTime );
	m_textHelp.Update( fDeltaTime );
	m_textInfo.Update( fDeltaTime );

	if( m_Mode == MODE_RECORD  ||  m_Mode == MODE_PLAY )
	{
		m_rectRecordBack.Update( fDeltaTime );
	}

	if( m_Mode == MODE_RECORD )
	{
		m_GrayArrowRowRecord.Update( fDeltaTime, m_fBeat );
		m_NoteFieldRecord.Update( fDeltaTime, m_fBeat );
	}

	if( m_Mode == MODE_PLAY )
	{
		m_Player.Update( fDeltaTime, m_fBeat, 0.35f );
	}

	//LOG->WriteLine( "ScreenEdit::Update(%f)", fDeltaTime );
	Screen::Update( fDeltaTime );


	// Update trailing beat
	float fDelta = m_fBeat - m_fTrailingBeat;

	if( fabsf(fDelta) < 0.01 )
	{
		m_fTrailingBeat = m_fBeat;	// snap
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


//	float fSongBeat, fBPS;
///	float fPositionSeconds = m_soundMusic.GetPositionSeconds();

//	fPositionSeconds += 0.08f;	// HACK:  The assist ticks are playing too late, so make them play a tiny bit earlier
//	m_pSong->GetBeatAndBPSFromElapsedTime( fPositionSeconds, fSongBeat, fBPS );
//	LOG->WriteLine( "fPositionSeconds = %f, fSongBeat = %f, fBPS = %f", fPositionSeconds, fSongBeat, fBPS );


	m_NoteFieldEdit.Update( fDeltaTime, m_fTrailingBeat );

	int iIndexNow = BeatToNoteRowNotRounded( m_fBeat );	


	CString sNoteType;
	switch( m_GranularityIndicator.GetSnapMode() )
	{
	case NOTE_4TH:	sNoteType = "quarter notes";	break;
	case NOTE_8TH:	sNoteType = "eighth notes";		break;
	case NOTE_12TH:	sNoteType = "triplets";			break;
	case NOTE_16TH:	sNoteType = "sixteenth notes";	break;
	default:  ASSERT( false );
	}

	static int iNumTapNotes = 0, iNumHoldNotes = 0;
	static int iCounter = 0;

	iCounter++;
	if( iCounter % 30 == 0 )
	{
		iNumTapNotes = m_NoteFieldEdit.GetNumTapNotes();
		iNumHoldNotes = m_NoteFieldEdit.GetNumHoldNotes();
	}

	m_textInfo.SetText( ssprintf(
		"Snap = %s\n"
		"Beat = %.2f\n"
		"Selection = begin: %.2f, end: %.2f\n"
		"Difficulty = %s\n"
		"Description = %s\n"
		"Num notes tap: %d, hold: %d\n"
		"Preview start: %.2f, length = %.2f\n",
		sNoteType,
		m_fBeat,
		m_NoteFieldEdit.m_fBeginMarker,	m_NoteFieldEdit.m_fEndMarker,
		DifficultyClassToString( m_DifficultyClass ),
		SONGMAN->GetCurrentNotes(PLAYER_1) ? SONGMAN->GetCurrentNotes(PLAYER_1)->m_sDescription : "no description",
		iNumTapNotes, iNumHoldNotes,
		m_pSong->m_fMusicSampleStartSeconds, m_pSong->m_fMusicSampleLengthSeconds
		) );
}


void ScreenEdit::DrawPrimitives()
{
	m_sprBackground.Draw();
	m_GranularityIndicator.Draw();
	m_GrayArrowRowEdit.Draw();
	m_textHelp.Draw();
	m_NoteFieldEdit.Draw();
	m_textInfo.Draw();
	m_Fade.Draw();

	if( m_Mode == MODE_RECORD  ||  m_Mode == MODE_PLAY )
	{
		m_rectRecordBack.Draw();
	}

	if( m_Mode == MODE_RECORD )
	{
		m_GrayArrowRowRecord.Draw();
		m_NoteFieldRecord.Draw();
	}

	if( m_Mode == MODE_PLAY )
	{
		m_Player.Draw();
	}

	Screen::DrawPrimitives();
}

void ScreenEdit::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->WriteLine( "ScreenEdit::Input()" );

	switch( m_Mode )
	{
	case MODE_EDIT:		InputEdit( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_RECORD:	InputRecord( DeviceI, type, GameI, MenuI, StyleI );	break;
	case MODE_PLAY:		InputPlay( DeviceI, type, GameI, MenuI, StyleI );	break;
	}
}

void ScreenEdit::InputEdit( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	///////////////////////////
	// handle pad inputs
	///////////////////////////

	if( DeviceI.device == DEVICE_KEYBOARD )
	{
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
				const int iNoteIndex = BeatToNoteRow( m_fBeat );

				if( iCol >= m_NoteFieldEdit.m_iNumTracks )	// this button is not in the range of columns for this StyleDef
					break;

				// check for to see if the user intended to remove a HoldNote
				bool bRemovedAHoldNote = false;
				for( int i=0; i<m_NoteFieldEdit.m_iNumHoldNotes; i++ )	// for each HoldNote
				{
					HoldNote &hn = m_NoteFieldEdit.m_HoldNotes[i];
					if( iCol == hn.m_iTrack  &&		// the notes correspond
						iNoteIndex >= hn.m_iStartIndex  &&  iNoteIndex <= hn.m_iEndIndex )	// the cursor lies within this HoldNote
					{
						m_NoteFieldEdit.RemoveHoldNote( i );
						bRemovedAHoldNote = true;
						break;	// stop iterating over all HoldNotes
					}
				}

				if( !bRemovedAHoldNote )
				{
					// We didn't remove a HoldNote, so the user wants to add or delete a TapNote
					if( m_NoteFieldEdit.m_TapNotes[iCol][iNoteIndex] == '0' )
						m_NoteFieldEdit.m_TapNotes[iCol][iNoteIndex] = '1';
					else
						m_NoteFieldEdit.m_TapNotes[iCol][iNoteIndex] = '0';
				}
			}
			break;
		case DIK_ESCAPE:
			SCREENMAN->SetNewScreen( new ScreenEditMenu );
			break;
		case DIK_S:
			{
				// copy edit into current Notes
				Notes* pNotes;
				pNotes = SONGMAN->GetCurrentNotes(PLAYER_1);

				if( pNotes == NULL )
				{
					// allocate a new Notes
					SONGMAN->GetCurrentSong()->m_arrayNotes.SetSize( SONGMAN->GetCurrentSong()->m_arrayNotes.GetSize() + 1 );
					pNotes = SONGMAN->GetCurrentSong()->m_arrayNotes[ SONGMAN->GetCurrentSong()->m_arrayNotes.GetSize()-1 ];
					pNotes->m_NotesType = GAMEMAN->m_CurNotesType;
					pNotes->m_sDescription = "Untitled";
					pNotes->m_iMeter = 1;
				}

				pNotes->SetNoteData( (NoteData*)&m_NoteFieldEdit );
				SONGMAN->GetCurrentSong()->Save();
			}
			break;
		case DIK_UP:
		case DIK_DOWN:
		case DIK_PGUP:
		case DIK_PGDN:
			{
				float fBeatsToMove;
				switch( DeviceI.button )
				{
				case DIK_UP:
				case DIK_DOWN:
					fBeatsToMove = NoteTypeToBeat( m_GranularityIndicator.GetSnapMode() );
					if( DeviceI.button == DIK_UP )	
						fBeatsToMove *= -1;
				break;
				case DIK_PGUP:
				case DIK_PGDN:
					fBeatsToMove = BEATS_PER_MEASURE;
					if( DeviceI.button == DIK_PGUP )	
						fBeatsToMove *= -1;
				}

				const int iStartIndex = BeatToNoteRow(m_fBeat);
				const int iEndIndex = BeatToNoteRow(m_fBeat + fBeatsToMove);

				// check to see if they're holding a button
				for( int col=0; col<m_NoteFieldEdit.m_iNumTracks && col<=10; col++ )
				{
					const DeviceInput di(DEVICE_KEYBOARD, DIK_1+col);
					BOOL bIsBeingHeld = INPUTMAN->IsBeingPressed(di);

					if( bIsBeingHeld )
					{
						// create a new hold note
						HoldNote newHN;
						newHN.m_iTrack = col;
						newHN.m_iStartIndex = min(iStartIndex, iEndIndex);
						newHN.m_iEndIndex = max(iStartIndex, iEndIndex);
						m_NoteFieldEdit.AddHoldNote( newHN );
					}
				}

				m_fBeat += fBeatsToMove;
				m_fBeat = clamp( m_fBeat, 0, MAX_BEATS-1 );
				m_fBeat = froundf( m_fBeat, NoteTypeToBeat(m_GranularityIndicator.GetSnapMode()) );
				m_soundChangeLine.Play();
			}
			break;
		case DIK_HOME:
			m_fBeat = 0;
			m_soundChangeLine.Play();
			break;
		case DIK_END:
			m_fBeat = m_NoteFieldEdit.GetLastBeat();
			m_soundChangeLine.Play();
			break;
		case DIK_RIGHT:
			m_GranularityIndicator.PrevSnapMode();
			OnSnapModeChange();
			break;
		case DIK_LEFT:
			m_GranularityIndicator.NextSnapMode();
			OnSnapModeChange();
			break;
		case DIK_RETURN:
			if( m_NoteFieldEdit.m_fEndMarker != -1  &&  m_fBeat > m_NoteFieldEdit.m_fEndMarker )
			{
				// invalid!  The begin maker must be placed before the end marker
				m_soundInvalid.Play();
			}
			else
			{
				m_NoteFieldEdit.m_fBeginMarker = m_fBeat;
				m_soundMarker.Play();
			}
			break;
		case DIK_SPACE:
			if( m_NoteFieldEdit.m_fBeginMarker != -1  &&  m_fBeat < m_NoteFieldEdit.m_fBeginMarker )
			{
				// invalid!  The end maker must be placed after the begin marker
				m_soundInvalid.Play();
			}
			else
			{
				m_NoteFieldEdit.m_fEndMarker = m_fBeat;
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
						case DIK_G:	noteType1 = NOTE_4TH;	noteType2 = NOTE_4TH;	break;
						case DIK_H:	noteType1 = NOTE_8TH;	noteType2 = NOTE_8TH;	break;
						case DIK_J:	noteType1 = NOTE_12TH;	noteType2 = NOTE_12TH;	break;
						case DIK_K:	noteType1 = NOTE_16TH;	noteType2 = NOTE_16TH;	break;
						case DIK_L:	noteType1 = NOTE_12TH;	noteType2 = NOTE_16TH;	break;
						default:	ASSERT( false );
					}

					m_NoteFieldEdit.SnapToNearestNoteType( noteType1, noteType2, m_NoteFieldEdit.m_fBeginMarker, m_NoteFieldEdit.m_fEndMarker );
				}
			}
			break;
		case DIK_P:
			{
				if( m_NoteFieldEdit.m_fBeginMarker == -1  ||  m_NoteFieldEdit.m_fEndMarker == -1 )
				{
					m_soundInvalid.Play();
					break;
				}

				m_fBeat = m_NoteFieldEdit.m_fBeginMarker;

				m_Mode = MODE_PLAY;

				m_Player.Load( PLAYER_1, GAMEMAN->GetCurrentStyleDef(), (NoteData*)&m_NoteFieldEdit, PlayerOptions(), NULL, NULL, 1 );

				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetTweenDiffuseColor( D3DXCOLOR(0,0,0,0.5f) );

				float fElapsedSeconds = max( 0, m_pSong->GetElapsedTimeFromBeat(m_fBeat) );
				m_soundMusic.SetPositionSeconds( fElapsedSeconds );
				m_soundMusic.Play();
				m_soundMusic.SetPlaybackRate( 1.0f );
			}
			break;
		case DIK_R:
			{
				if( m_NoteFieldEdit.m_fBeginMarker == -1  ||  m_NoteFieldEdit.m_fEndMarker == -1 )
				{
					m_soundInvalid.Play();
					break;
				}

				m_fBeat = m_NoteFieldEdit.m_fBeginMarker;

				// initialize m_NoteFieldRecord
				m_NoteFieldRecord.ClearAll();
				m_NoteFieldRecord.m_iNumTracks = m_NoteFieldEdit.m_iNumTracks;
				m_NoteFieldRecord.m_fBeginMarker = m_NoteFieldEdit.m_fBeginMarker;
				m_NoteFieldRecord.m_fEndMarker = m_NoteFieldEdit.m_fEndMarker;


				m_Mode = MODE_RECORD;
				m_rectRecordBack.BeginTweening( 0.5f );
				m_rectRecordBack.SetTweenDiffuseColor( D3DXCOLOR(0,0,0,0.5f) );

				float fElapsedSeconds = max( 0, m_pSong->GetElapsedTimeFromBeat(m_fBeat) );
				m_soundMusic.SetPositionSeconds( fElapsedSeconds );
				m_soundMusic.Play();
				m_soundMusic.SetPlaybackRate( 0.5f );
			}
			break;
		case DIK_INSERT:
		case DIK_DELETE:
			{
				NoteData temp;
				int iTakeFromRow;
				int iPasteAtRow;
				switch( DeviceI.button )
				{
				case DIK_INSERT:
					iTakeFromRow = BeatToNoteRow( m_fBeat );
					iPasteAtRow = BeatToNoteRow( m_fBeat+1 );
					break;
				case DIK_DELETE:
					iTakeFromRow = BeatToNoteRow( m_fBeat+1 );
					iPasteAtRow = BeatToNoteRow( m_fBeat );
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
				int iSrcFirstRow = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
				int iSrcLastRow  = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );
				int iDestFirstRow = BeatToNoteRow( m_fBeat );

				m_NoteFieldEdit.CopyRange( &m_Clipboard, iSrcFirstRow, iSrcLastRow, iDestFirstRow );
			}
			break;

		case DIK_D:
			{
				m_DifficultyClass = DifficultyClass( (m_DifficultyClass+1)%NUM_DIFFICULTY_CLASSES );
			}
			break;

		case DIK_F1:
		case DIK_F2:
		case DIK_F3:
		case DIK_F4:
			{
				float fBPM = m_pSong->GetBPMAtBeat( m_fBeat );
				float fNewBPM;
				switch( DeviceI.button )
				{
				case DIK_F1:	fNewBPM = fBPM - 0.1f;		break;
				case DIK_F2:	fNewBPM = fBPM + 0.1f;		break;
				case DIK_F3:	fNewBPM = fBPM - 0.01f;		break;
				case DIK_F4:	fNewBPM = fBPM + 0.01f;		break;
				default:	ASSERT(0);
				}

				for( int i=0; i<m_pSong->m_BPMSegments.GetSize(); i++ )
					if( m_pSong->m_BPMSegments[i].m_fStartBeat == m_fBeat )
						break;

				if( i == m_pSong->m_BPMSegments.GetSize() )	// there is no BPMSegment at the current beat
				{
					// create a new BPMSegment
					m_pSong->AddBPMSegment( BPMSegment(m_fBeat, fNewBPM) );
				}
				else	// BPMSegment being modified is m_BPMSegments[i]
				{
					if( i > 0  &&  m_pSong->m_BPMSegments[i-1].m_fBPM == fNewBPM )
						m_pSong->m_BPMSegments.RemoveAt( i );
					else
						m_pSong->m_BPMSegments[i].m_fBPM = fNewBPM;
				}
			}
			break;
		case DIK_F5:
		case DIK_F6:
		case DIK_F7:
		case DIK_F8:
			{
				float fFreezeDelta;
				switch( DeviceI.button )
				{
				case DIK_F5:	fFreezeDelta = -0.1f;		break;
				case DIK_F6:	fFreezeDelta = +0.1f;		break;
				case DIK_F7:	fFreezeDelta = -0.01f;		break;
				case DIK_F8:	fFreezeDelta = +0.01f;		break;
				default:	ASSERT(0);
				}

				for( int i=0; i<m_pSong->m_FreezeSegments.GetSize(); i++ )
				{
					if( m_pSong->m_FreezeSegments[i].m_fStartBeat == m_fBeat )
						break;
				}

				if( i == m_pSong->m_FreezeSegments.GetSize() )	// there is no BPMSegment at the current beat
				{
					// create a new FreezeSegment
					if( fFreezeDelta > 0 )
						m_pSong->AddFreezeSegment( FreezeSegment(m_fBeat, fFreezeDelta) );
				}
				else	// FreezeSegment being modified is m_FreezeSegments[i]
				{
					m_pSong->m_FreezeSegments[i].m_fFreezeSeconds += fFreezeDelta;
					if( m_pSong->m_FreezeSegments[i].m_fFreezeSeconds < 0 )
						m_pSong->m_FreezeSegments.RemoveAt( i );
				}
			}
			break;
		case DIK_F9:	m_pSong->m_fOffsetInSeconds -= 0.1f;		break;
		case DIK_F10:	m_pSong->m_fOffsetInSeconds += 0.1f;		break;
		case DIK_F11:	m_pSong->m_fOffsetInSeconds -= 0.01f;		break;
		case DIK_F12:	m_pSong->m_fOffsetInSeconds += 0.01f;		break;
		case DIK_M:
			MUSIC->Load( m_pSong->GetMusicPath() );
			MUSIC->Play( false, m_pSong->m_fMusicSampleStartSeconds, m_pSong->m_fMusicSampleLengthSeconds );
			break;
		case DIK_DIVIDE:		m_pSong->m_fMusicSampleStartSeconds -= 0.1f;	break;
		case DIK_MULTIPLY:		m_pSong->m_fMusicSampleStartSeconds += 0.1f;	break;
		case DIK_SUBTRACT:		m_pSong->m_fMusicSampleStartSeconds -= 0.01f;	break;
		case DIK_ADD:			m_pSong->m_fMusicSampleStartSeconds += 0.01f;	break;
		case DIK_NUMPAD0:		m_pSong->m_fMusicSampleLengthSeconds -= 0.1f;	break;
		case DIK_NUMPADPERIOD:	m_pSong->m_fMusicSampleLengthSeconds += 0.1f;	break;

	}
	}
}

void ScreenEdit::InputRecord( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( DeviceI.device == DEVICE_KEYBOARD )
	{
		switch( DeviceI.button )
		{
		case DIK_ESCAPE:
			TransitionToEditFromRecord();
			break;
		}
	}
	switch( StyleI.player )
	{
		case PLAYER_1:
			int iCol;
			iCol = StyleI.col;
		
			int iNoteIndex;
			iNoteIndex = BeatToNoteRow( m_fBeat );

			if( type == IET_FIRST_PRESS )
			{
				m_NoteFieldRecord.m_TapNotes[iCol][iNoteIndex] = '1';
				m_NoteFieldRecord.SnapToNearestNoteType( NOTE_12TH, NOTE_16TH, max(0,m_fBeat-1), m_fBeat+1);
				m_GrayArrowRowRecord.Step( iCol );
			}
			else
			{
				const float fHoldEndSeconds = m_soundMusic.GetPositionSeconds();
				const float fHoldStartSeconds = m_soundMusic.GetPositionSeconds() - TIME_BEFORE_SLOW_REPEATS * m_soundMusic.GetPlaybackRate();

				float fStartBeat, fEndBeat, fThrowAway;
				m_pSong->GetBeatAndBPSFromElapsedTime( fHoldStartSeconds, fStartBeat, fThrowAway );
				m_pSong->GetBeatAndBPSFromElapsedTime( fHoldEndSeconds, fEndBeat, fThrowAway );

				const int iStartIndex = BeatToNoteRow(fStartBeat) - 1;
				const int iEndIndex = BeatToNoteRow(fEndBeat);

				// create a new hold note
				HoldNote newHN;
				newHN.m_iTrack = iCol;
				newHN.m_iStartIndex = iStartIndex;
				newHN.m_iEndIndex = iEndIndex;

				m_NoteFieldRecord.AddHoldNote( newHN );
				m_NoteFieldRecord.SnapToNearestNoteType( NOTE_12TH, NOTE_16TH, max(0,m_fBeat-2), m_fBeat+2);
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
			m_Mode = MODE_EDIT;
			m_soundMusic.Stop();

			m_fBeat = froundf( m_fBeat, NoteTypeToBeat(m_GranularityIndicator.GetSnapMode()) );
			break;
		}
	}

	switch( StyleI.player )
	{
		case PLAYER_1:	
			m_Player.HandlePlayerStep( m_fBeat, StyleI.col, 0.35f ); 
			return;
	}

}


void ScreenEdit::TransitionToEditFromRecord()
{
	m_Mode = MODE_EDIT;
	m_soundMusic.Stop();

	int iNoteIndexBegin = BeatToNoteRow( m_NoteFieldEdit.m_fBeginMarker );
	int iNoteIndexEnd = BeatToNoteRow( m_NoteFieldEdit.m_fEndMarker );

	// delete old TapNotes in the range
	m_NoteFieldEdit.ClearRange( iNoteIndexBegin, iNoteIndexEnd );

	m_NoteFieldEdit.CopyRange( (NoteData*)&m_NoteFieldRecord, iNoteIndexBegin, iNoteIndexEnd );

	m_fBeat = froundf( m_fBeat, NoteTypeToBeat(m_GranularityIndicator.GetSnapMode()) );
}


void ScreenEdit::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenEditMenu );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenEditMenu );
		break;
	}


}

void ScreenEdit::OnSnapModeChange()
{
	m_soundChangeSnap.Play();
			
	NoteType nt = m_GranularityIndicator.GetSnapMode();
	int iStepIndex = BeatToNoteRow( m_fBeat );
	int iElementsPerNoteType = BeatToNoteRow( NoteTypeToBeat(nt) );
	int iStepIndexHangover = iStepIndex % iElementsPerNoteType;
	m_fBeat -= NoteRowToBeat( iStepIndexHangover );
}
