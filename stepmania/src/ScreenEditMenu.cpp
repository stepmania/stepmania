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
#include "ThemeManager.h"
#include "GameManager.h"
#include "ScreenPrompt.h"
#include "RageLog.h"


//
// Defines specific to ScreenEditMenu
//
const float LINE_START_Y	=	CENTER_Y-150;
const float LINE_GAP		=	50;

const float GROUP_X			=	CENTER_X;
const float GROUP_Y			=	LINE_START_Y + LINE_GAP;

const float SONG_X			=	CENTER_X;
const float SONG_Y			=	GROUP_Y + LINE_GAP;

const float GAME_STYLE_X	=	CENTER_X;
const float GAME_STYLE_Y	=	SONG_Y + LINE_GAP;

const float STEPS_X			=	CENTER_X;
const float STEPS_Y			=	GAME_STYLE_Y + LINE_GAP;

const float EXPLANATION_X	=	CENTER_X;
const float EXPLANATION_Y	=	STEPS_Y + LINE_GAP*2;


const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);


ScreenEditMenu::ScreenEditMenu()
{
	LOG->WriteLine( "ScreenEditMenu::ScreenEditMenu()" );


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
	this->AddActor( &m_textGroup );

	m_textSong.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textSong.SetXY( SONG_X, SONG_Y );
	m_textSong.SetDiffuseColor( D3DXCOLOR(0.7f,0.7f,0.7f,1) );
	this->AddActor( &m_textSong );

	m_textNotesType.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textNotesType.SetXY( GAME_STYLE_X, GAME_STYLE_Y );
	m_textNotesType.SetDiffuseColor( D3DXCOLOR(0.7f,0.7f,0.7f,1) );
	this->AddActor( &m_textNotesType );

	m_textNotes.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textNotes.SetXY( STEPS_X, STEPS_Y );
	m_textNotes.SetDiffuseColor( D3DXCOLOR(0.7f,0.7f,0.7f,1) );
	this->AddActor( &m_textNotes );


	AfterRowChange();
	OnGroupChange();


	m_textExplanation.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetText( ssprintf("This mode will allow you to\nedit an existing or\n create a new Notes Notes.") );
	m_textExplanation.SetZoom( 0.7f );
	this->AddActor( &m_textExplanation );

	m_Menu.Load( 
		THEME->GetPathTo(GRAPHIC_EDIT_BACKGROUND), 
		THEME->GetPathTo(GRAPHIC_EDIT_TOP_EDGE),
		ssprintf("%s %s change music    NEXT to continue", CString(char(1)), CString(char(2)) )
		);
	this->AddActor( &m_Menu );


	m_Fade.SetOpened();
	this->AddActor( &m_Fade);


	m_soundChangeMusic.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_CHANGE_MUSIC) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );

	MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
	MUSIC->Play( true );


	m_Menu.TweenOnScreenFromBlack( SM_None );
}


ScreenEditMenu::~ScreenEditMenu()
{
	LOG->WriteLine( "ScreenEditMenu::~ScreenEditMenu()" );
}

void ScreenEditMenu::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenEditMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenEditMenu::Input()" );

	
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
		SCREENMAN->SetNewScreen( new ScreenEdit );
		break;
	}
}
	
void ScreenEditMenu::BeforeRowChange()
{
	m_textGroup.SetEffectNone();
	m_textSong.SetEffectNone();
	m_textNotesType.SetEffectNone();
	m_textNotes.SetEffectNone();
}

void ScreenEditMenu::AfterRowChange()
{
	switch( m_SelectedRow )
	{
	case ROW_GROUP:			m_textGroup.SetEffectGlowing();			break;
	case ROW_SONG:			m_textSong.SetEffectGlowing();			break;
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

	m_textSong.SetText( GetSelectedSong()->GetMainTitle() );

	OnNotesTypeChange();
}

void ScreenEditMenu::OnNotesTypeChange()
{
	m_CurNotesType = (NotesType)clamp( m_CurNotesType, 0, NUM_NOTES_TYPES );

	m_textNotesType.SetText( NotesTypeToString( GetSelectedNotesType() ) );

	m_pNotess.RemoveAll();
	GetSelectedSong()->GetNotesThatMatch( GetSelectedNotesType(), m_pNotess );
	SortNotesArrayByDifficultyClass( m_pNotess );
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

	SONGMAN->SetCurrentSong( GetSelectedSong() );
	GAMEMAN->m_CurNotesType = GetSelectedNotesType();
	SONGMAN->SetCurrentNotes( PLAYER_1, GetSelectedNotes() );

	m_soundSelect.PlayRandom();

	m_Fade.CloseWipingRight( SM_GoToNextState );
}

void ScreenEditMenu::MenuBack( const PlayerNumber p )
{	
	m_Menu.TweenOffScreenToBlack( SM_None, true );


	MUSIC->Stop();

	m_Fade.CloseWipingLeft( SM_GoToPrevState );
}
