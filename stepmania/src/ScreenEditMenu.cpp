#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenEditMenu.h

 Desc: The main title screen and menu.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenEditMenu.h"
#include "SongManager.h"
#include "ScreenManager.h"
#include "ScreenTitleMenu.h"
#include "ScreenEdit.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "ScreenPrompt.h"
#include "RageLog.h"
#include "GameState.h"


//
// Defines specific to ScreenEditMenu
//
const float GROUP_X				=	CENTER_X;
const float GROUP_Y				=	CENTER_Y - 160;

const float SONG_BANNER_X		=	CENTER_X;
const float SONG_BANNER_Y		=	CENTER_Y - 80;

const float ARROWS_X[2]			=	{ SONG_BANNER_X - 200, SONG_BANNER_X + 200 };
const float ARROWS_Y[2]			=	{ SONG_BANNER_Y,       SONG_BANNER_Y };

const float SONG_TEXT_BANNER_X	=	CENTER_X;
const float SONG_TEXT_BANNER_Y	=	CENTER_Y - 10;

const float GAME_STYLE_X		=	CENTER_X;
const float GAME_STYLE_Y		=	CENTER_Y + 40;

const float STEPS_X				=	CENTER_X;
const float STEPS_Y				=	CENTER_Y + 90;

const float EXPLANATION_X		=	CENTER_X;
const float EXPLANATION_Y		=	SCREEN_BOTTOM - 70;
const CString EXPLANATION_TEXT	= 
	"In this mode, you can edit existing notes patterns,\n"
	"create note patterns, or synchronize notes with the music.";

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);


ScreenEditMenu::ScreenEditMenu()
{
	LOG->Trace( "ScreenEditMenu::ScreenEditMenu()" );


	// data structures
	m_SelectedRow = ROW_GROUP;

	SONGMAN->GetGroupNames( m_sGroups );
	m_iSelectedGroup = 0;
	m_iSelectedSong = 0;
	m_CurNotesType = NOTES_TYPE_DANCE_SINGLE;
	m_iSelectedNotes = 0;

	m_textGroup.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textGroup.SetXY( GROUP_X, GROUP_Y );
	m_textGroup.SetDiffuseColor( D3DXCOLOR(0.7f,0.7f,0.7f,1) );
	this->AddSubActor( &m_textGroup );

	m_Banner.SetXY( SONG_BANNER_X, SONG_BANNER_Y );
	this->AddSubActor( &m_Banner );

	m_TextBanner.SetXY( SONG_TEXT_BANNER_X, SONG_TEXT_BANNER_Y );
	this->AddSubActor( &m_TextBanner );
	
	m_sprArrowLeft.Load( THEME->GetPathTo(GRAPHIC_ARROWS_LEFT) );
	m_sprArrowLeft.SetXY( ARROWS_X[0], ARROWS_Y[0] );
	m_sprArrowLeft.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_sprArrowLeft );

	m_sprArrowRight.Load( THEME->GetPathTo(GRAPHIC_ARROWS_RIGHT) );
	m_sprArrowRight.SetXY( ARROWS_X[1], ARROWS_Y[1] );
	m_sprArrowRight.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	this->AddSubActor( &m_sprArrowRight );

	m_textNotesType.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textNotesType.SetXY( GAME_STYLE_X, GAME_STYLE_Y );
	m_textNotesType.SetDiffuseColor( D3DXCOLOR(0.7f,0.7f,0.7f,1) );
	this->AddSubActor( &m_textNotesType );

	m_textNotes.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textNotes.SetXY( STEPS_X, STEPS_Y );
	m_textNotes.SetDiffuseColor( D3DXCOLOR(0.7f,0.7f,0.7f,1) );
	this->AddSubActor( &m_textNotes );


	AfterRowChange();
	OnGroupChange();


	m_textExplanation.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetText( EXPLANATION_TEXT );
	m_textExplanation.SetZoom( 0.7f );
	this->AddSubActor( &m_textExplanation );

	m_Menu.Load( 
		THEME->GetPathTo(GRAPHIC_EDIT_BACKGROUND), 
		THEME->GetPathTo(GRAPHIC_EDIT_TOP_EDGE),
		ssprintf("%c %c change line    %c %c change value    START to continue", char(3), char(4), char(1), char(2) ),
		false, false, 40 
		);
	this->AddSubActor( &m_Menu );


	m_Fade.SetOpened();
	this->AddSubActor( &m_Fade);


	m_soundChangeMusic.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_CHANGE_MUSIC) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );

	MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
	MUSIC->Play( true );


	m_Menu.TweenOnScreenFromBlack( SM_None );
}


ScreenEditMenu::~ScreenEditMenu()
{
	LOG->Trace( "ScreenEditMenu::~ScreenEditMenu()" );
}

void ScreenEditMenu::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenEditMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEditMenu::Input()" );

	
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEditMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		// set the current style based on the notes type

		// Dro Kulix:
		// A centralized solution for this switching mess...
		// (See GameConstantsAndTypes.h)
		//
		// Chris:
		//	Find the first Style that will play the selected notes type.
		//  Set the current Style, then let ScreenEdit infer the desired
		//  NotesType from that Style.
		NotesType nt = GetSelectedNotesType();
		Style style = GAMEMAN->GetStyleThatPlaysNotesType( nt );
		GAMESTATE->m_CurStyle = style;
		GAMESTATE->m_CurGame = StyleToGame(style);

		SCREENMAN->SetNewScreen( new ScreenEdit );
		break;
	}
}
	
void ScreenEditMenu::BeforeRowChange()
{
	m_textGroup.SetEffectNone();
	m_sprArrowLeft.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_sprArrowRight.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_textNotesType.SetEffectNone();
	m_textNotes.SetEffectNone();
}

void ScreenEditMenu::AfterRowChange()
{
	switch( m_SelectedRow )
	{
	case ROW_GROUP:			m_textGroup.SetEffectGlowing();			break;
	case ROW_SONG:
		m_sprArrowLeft.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		m_sprArrowRight.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		break;
	case ROW_NOTES_TYPE:	m_textNotesType.SetEffectGlowing();		break;
	case ROW_STEPS:			m_textNotes.SetEffectGlowing();			break;
	default:		ASSERT(false);
	}
}

void ScreenEditMenu::OnGroupChange()
{
	m_iSelectedGroup = clamp( m_iSelectedGroup, 0, m_sGroups.GetSize()-1 );

	m_textGroup.SetText( SONGMAN->ShortenGroupName(GetSelectedGroup()) );

	// reload songs
	m_pSongs.RemoveAll();
	SONGMAN->GetSongsInGroup( GetSelectedGroup(), m_pSongs );

	OnSongChange();
}

void ScreenEditMenu::OnSongChange()
{
	m_iSelectedSong = clamp( m_iSelectedSong, 0, m_pSongs.GetSize()-1 );

	m_Banner.LoadFromSong( GetSelectedSong() );
	m_TextBanner.LoadFromSong( GetSelectedSong() );

	OnNotesTypeChange();
}

void ScreenEditMenu::OnNotesTypeChange()
{
	m_CurNotesType = (NotesType)clamp( m_CurNotesType, 0, NUM_NOTES_TYPES );

	m_textNotesType.SetText( NotesTypeToString( GetSelectedNotesType() ) );

	m_pNotess.RemoveAll();
	GetSelectedSong()->GetNotesThatMatch( GetSelectedNotesType(), m_pNotess );
	SortNotesArrayByDifficulty( m_pNotess );
	m_pNotess.Add( NULL );		// marker for "(NEW)"
	m_iSelectedNotes = 0;


	OnStepsChange();
}

void ScreenEditMenu::OnStepsChange()
{
	m_iSelectedNotes = clamp( m_iSelectedNotes, 0, m_pNotess.GetSize()-1 );

	if( GetSelectedNotes() == NULL )
		m_textNotes.SetText( "(NEW)" );
	else
		m_textNotes.SetText( GetSelectedNotes()->m_sDescription );

}

void ScreenEditMenu::MenuUp( const PlayerNumber p )
{
	if( m_SelectedRow == 0 )	// can't go up any further
		return;	

	BeforeRowChange();
	m_SelectedRow = SelectedRow(m_SelectedRow-1);
	AfterRowChange();
}

void ScreenEditMenu::MenuDown( const PlayerNumber p )
{
	if( m_SelectedRow == NUM_ROWS-1 )	// can't go down any further
		return;	

	BeforeRowChange();
	m_SelectedRow = SelectedRow(m_SelectedRow+1);
	AfterRowChange();
}

void ScreenEditMenu::MenuLeft( const PlayerNumber p )
{
	switch( m_SelectedRow )
	{
	case ROW_GROUP:
		if( m_iSelectedGroup == 0 )	// can't go left any further
			return;
		m_iSelectedGroup--;
		OnGroupChange();
		break;
	case ROW_SONG:
		if( m_iSelectedSong == 0 )	// can't go left any further
			return;
		m_iSelectedSong--;
		OnSongChange();
		break;
	case ROW_NOTES_TYPE:
		if( m_CurNotesType == 0 )	// can't go left any further
			return;
		m_CurNotesType = NotesType( m_CurNotesType-1 );
		OnNotesTypeChange();
		break;
	case ROW_STEPS:
		if( m_iSelectedNotes == 0 )	// can't go left any further
			return;
		m_iSelectedNotes--;
		OnStepsChange();
		break;
	default:
		ASSERT(false);
	}
}

void ScreenEditMenu::MenuRight( const PlayerNumber p )
{
	switch( m_SelectedRow )
	{
	case ROW_GROUP:
		if( m_iSelectedGroup == m_sGroups.GetSize()-1 )	// can't go right any further
			return;
		m_iSelectedGroup++;
		OnGroupChange();
		break;
	case ROW_SONG:
		if( m_iSelectedSong == m_pSongs.GetSize()-1 )	// can't go right any further
			return;
		m_iSelectedSong++;
		OnSongChange();
		break;
	case ROW_NOTES_TYPE:
		if( m_CurNotesType == NUM_NOTES_TYPES-1 )	// can't go right any further
			return;
		m_CurNotesType = NotesType( m_CurNotesType+1 );
		OnNotesTypeChange();
		break;
	case ROW_STEPS:
		if( m_iSelectedNotes == m_pNotess.GetSize()-1 )	// can't go right any further
			return;
		m_iSelectedNotes++;
		OnStepsChange();
		break;
	default:
		ASSERT(false);
	}
}

void ScreenEditMenu::MenuStart( const PlayerNumber p )
{
	m_Menu.TweenOffScreenToBlack( SM_None, false );

	MUSIC->Stop();

	GAMESTATE->m_pCurSong = GetSelectedSong();

	// find the first style that matches this notes type
	GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	GAMESTATE->m_CurStyle = GAMEMAN->GetStyleThatPlaysNotesType( GetSelectedNotesType() );
	GAMESTATE->m_pCurNotes[PLAYER_1] = GetSelectedNotes();

	m_soundSelect.PlayRandom();

	m_Fade.CloseWipingRight( SM_GoToNextState );
}

void ScreenEditMenu::MenuBack( const PlayerNumber p )
{	
	m_Menu.TweenOffScreenToBlack( SM_None, true );


	MUSIC->Stop();

	m_Fade.CloseWipingLeft( SM_GoToPrevState );
}
