#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenTitleMenu

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenTitleMenu.h"
#include "ScreenManager.h"
#include "ScreenCaution.h"
#include "ScreenMapInstruments.h"
#include "ScreenGameOptions.h"
#include "ScreenSynchronizeMenu.h"
#include "ScreenEdit.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "ThemeManager.h"
#include "ScreenEditMenu.h"
#include "ScreenSelectStyle.h"
#include "ScreenSelectGame.h"
#include "RageLog.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "ScreenEz2SelectPlayer.h"

#include "GameManager.h"


//
// Defines specific to ScreenTitleMenu
//

const CString CHOICE_TEXT[ScreenTitleMenu::NUM_TITLE_MENU_CHOICES] = {
	"GAME START",
	"SWITCH GAME",
	"CONFIG INSTRUMENTS",
	"GAME OPTIONS",
	"SYNCHRONIZE",
	"EDIT/RECORD",
	"EXIT",
};

const float CHOICES_START_Y		= 66;
const float CHOICES_GAP_Y		= 50;

const float HELP_X				= CENTER_X;
const float HELP_Y				= SCREEN_HEIGHT-55;


const ScreenMessage SM_PlayAttract			=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToCaution			=	ScreenMessage(SM_User+2);
const ScreenMessage SM_GoToSelectStyle		=	ScreenMessage(SM_User+3);
const ScreenMessage SM_GoToSelectGame		=	ScreenMessage(SM_User+4);
const ScreenMessage SM_GoToMapInstruments	=	ScreenMessage(SM_User+5);
const ScreenMessage SM_GoToGameOptions		=	ScreenMessage(SM_User+6);
const ScreenMessage SM_GoToSynchronize		=	ScreenMessage(SM_User+9);
const ScreenMessage SM_GoToEdit				=	ScreenMessage(SM_User+10);
const ScreenMessage SM_DoneOpening			=	ScreenMessage(SM_User+11);
const ScreenMessage SM_GoToEz2				=	ScreenMessage(SM_User+12);

ScreenTitleMenu::ScreenTitleMenu()
{
	LOG->WriteLine( "ScreenTitleMenu::ScreenTitleMenu()" );

	int i;

	m_sprBG.Load( THEME->GetPathTo(GRAPHIC_TITLE_MENU_BACKGROUND) );
	m_sprBG.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	m_sprBG.SetDiffuseColor( D3DXCOLOR(0.6f,0.6f,0.6f,1) );
	m_sprBG.TurnShadowOff();
	this->AddActor( &m_sprBG );

	m_sprLogo.Load( THEME->GetPathTo(GRAPHIC_TITLE_MENU_LOGO) );
	m_sprLogo.SetXY( CENTER_X, CENTER_Y );
	m_sprLogo.SetAddColor( D3DXCOLOR(1,1,1,1) );
	m_sprLogo.SetZoomY( 0 );
	m_sprLogo.BeginTweeningQueued( 0.5f );
	m_sprLogo.BeginTweeningQueued( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprLogo.SetEffectGlowing(1, D3DXCOLOR(1,1,1,0.1f), D3DXCOLOR(1,1,1,0.3f) );
	m_sprLogo.SetTweenZoom( 1 );
	this->AddActor( &m_sprLogo );

	m_textHelp.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textHelp.SetText( ssprintf("Use %c %c to select, then press START", char(3), char(4)) );
	m_textHelp.SetXY( CENTER_X, SCREEN_BOTTOM - 30 );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetEffectBlinking();
	m_textHelp.SetShadowLength( 2 );
	this->AddActor( &m_textHelp );

	
	m_textVersion.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textVersion.SetHorizAlign( Actor::align_right );
	m_textVersion.SetText( "v3.0 compatibility test" );
	m_textVersion.SetDiffuseColor( D3DXCOLOR(0.6f,0.6f,0.6f,1) );	// light gray
	m_textVersion.SetXY( SCREEN_RIGHT-16, SCREEN_BOTTOM-20 );
	m_textVersion.SetZoom( 0.5f );
	this->AddActor( &m_textVersion );


	m_textSongs.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textSongs.SetHorizAlign( Actor::align_left );
	m_textSongs.SetText( ssprintf("Found %d Songs", SONGMAN->m_pSongs.GetSize()) );
	m_textSongs.SetDiffuseColor( D3DXCOLOR(0.6f,0.6f,0.6f,1) );	// light gray
	m_textSongs.SetXY( SCREEN_LEFT+16, SCREEN_HEIGHT-20 );
	m_textSongs.SetZoom( 0.5f );
	this->AddActor( &m_textSongs );


	for( i=0; i< NUM_TITLE_MENU_CHOICES; i++ )
	{
		m_textChoice[i].Load( THEME->GetPathTo(FONT_HEADER1) );
		m_textChoice[i].SetText( CHOICE_TEXT[i] );
		m_textChoice[i].SetXY( CENTER_X, CHOICES_START_Y + i*CHOICES_GAP_Y );
		m_textChoice[i].SetShadowLength( 5 );
		this->AddActor( &m_textChoice[i] );
	}	
	
	m_Fade.SetClosed();
	m_Fade.OpenWipingRight( SM_DoneOpening );
	this->AddActor( &m_Fade );


	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_TITLE_MENU_GAME_NAME) );

	m_soundChange.Load( THEME->GetPathTo(SOUND_TITLE_MENU_CHANGE) );	
	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );
	m_soundInvalid.Load( THEME->GetPathTo(SOUND_MENU_INVALID) );

	for( i=0; i<3000; i++ )
		this->SendScreenMessage( SM_PlayAttract, (float)15+i*15 );


	m_TitleMenuChoice = CHOICE_GAME_START;
	GainFocus( m_TitleMenuChoice );


	MUSIC->Stop();

	//this->SendScreenMessage( SM_TimeToFadeOut, 30.0 );
}


ScreenTitleMenu::~ScreenTitleMenu()
{
	LOG->WriteLine( "ScreenTitleMenu::~ScreenTitleMenu()" );
}


void ScreenTitleMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenTitleMenu::Input()" );

	if( m_Fade.IsClosing() )
		return;
	
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenTitleMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_DoneOpening:
		if( PREFSMAN->m_bAnnouncer )
			m_soundTitle.PlayRandom();
		break;
	case SM_PlayAttract:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_TITLE_MENU_ATTRACT) );
		break;
	case SM_GoToCaution:
		SCREENMAN->SetNewScreen( new ScreenCaution );
		break;
	case SM_GoToSelectStyle:
		SCREENMAN->SetNewScreen( new ScreenSelectStyle );
		break;
	case SM_GoToSelectGame:
		SCREENMAN->SetNewScreen( new ScreenSelectGame );
		break;
	case SM_GoToMapInstruments:
		SCREENMAN->SetNewScreen( new ScreenMapInstruments );
		break;
	case SM_GoToGameOptions:
		SCREENMAN->SetNewScreen( new ScreenGameOptions );
		break;
	case SM_GoToSynchronize:
		SCREENMAN->SetNewScreen( new ScreenSynchronizeMenu );
		break;
	case SM_GoToEdit:
		SCREENMAN->SetNewScreen( new ScreenEditMenu );
		break;
	case SM_GoToEz2:
		SCREENMAN->SetNewScreen( new ScreenEz2SelectPlayer );
		break;
	}
}


void ScreenTitleMenu::LoseFocus( int iChoiceIndex )
{
	m_soundChange.PlayRandom();

	m_textChoice[iChoiceIndex].SetEffectNone();
	m_textChoice[iChoiceIndex].SetAddColor( D3DXCOLOR(0,0,0,0) );
	m_textChoice[iChoiceIndex].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetTweenZoom( 0.9f );

}

void ScreenTitleMenu::GainFocus( int iChoiceIndex )
{
	m_textChoice[iChoiceIndex].SetDiffuseColor( D3DXCOLOR(0.5f,1,0.5f,1) );
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetTweenZoom( 1.2f );
	m_textChoice[iChoiceIndex].SetEffectCamelion( 2.5f, D3DXCOLOR(0.5f,1,0.5f,1), D3DXCOLOR(0.3f,0.6f,0.3f,1) );
}

void ScreenTitleMenu::MenuUp( const PlayerNumber p )
{
	LoseFocus( m_TitleMenuChoice );

	if( m_TitleMenuChoice == 0 ) // wrap around
		m_TitleMenuChoice = (ScreenTitleMenu::TitleMenuChoice)((int)NUM_TITLE_MENU_CHOICES); 
	
	m_TitleMenuChoice = TitleMenuChoice( m_TitleMenuChoice-1 );

	GainFocus( m_TitleMenuChoice );
}


void ScreenTitleMenu::MenuDown( const PlayerNumber p )
{
	LoseFocus( m_TitleMenuChoice );

	if( m_TitleMenuChoice == (int)ScreenTitleMenu::NUM_TITLE_MENU_CHOICES-1 ) 
		m_TitleMenuChoice = (TitleMenuChoice)-1; // wrap around

	m_TitleMenuChoice = TitleMenuChoice( m_TitleMenuChoice+1 );

	GainFocus( m_TitleMenuChoice );
}


void ScreenTitleMenu::MenuStart( const PlayerNumber p )
{	
	GAMEMAN->m_sMasterPlayerNumber = p;

	switch( m_TitleMenuChoice )
	{
	case CHOICE_GAME_START:
		if ( GAMEMAN->m_CurGame == GAME_EZ2 )
		{
			m_soundSelect.PlayRandom();
			m_Fade.CloseWipingRight( SM_GoToEz2 );
		}
		else
		{
			m_soundSelect.PlayRandom();
			m_Fade.CloseWipingRight( SM_GoToCaution );
		}
		return;
	case CHOICE_SELECT_GAME:
		m_soundSelect.PlayRandom();
		m_Fade.CloseWipingRight( SM_GoToSelectGame );
		return;
	case CHOICE_MAP_INSTRUMENTS:
		m_soundSelect.PlayRandom();
		m_Fade.CloseWipingRight( SM_GoToMapInstruments );
		return;
	case CHOICE_GAME_OPTIONS:
		m_soundSelect.PlayRandom();
		m_Fade.CloseWipingRight( SM_GoToGameOptions );
		return;
	case CHOICE_SYNCHRONIZE:
		m_soundSelect.PlayRandom();
		m_Fade.CloseWipingRight( SM_GoToSynchronize );
		return;
	case CHOICE_EDIT:
		m_soundSelect.PlayRandom();
		m_Fade.CloseWipingRight( SM_GoToEdit );
		return;
/*	case CHOICE_HELP:
		m_soundSelect.PlayRandom();
		PREFSMAN->m_bWindowed = false;
		ApplyGraphicOptions();
		GotoURL( "Docs/index.htm" );
		return;
	case CHOICE_CHECK_FOR_UPDATE:
		m_soundSelect.PlayRandom();
		PREFSMAN->m_bWindowed = false;
		ApplyGraphicOptions();
		GotoURL( "http://www.stepmania.com" );
		return;
		*/
	case CHOICE_EXIT:
		m_soundSelect.PlayRandom();
		PostQuitMessage(0);
		return;
	}
}

void ScreenTitleMenu::MenuBack( const PlayerNumber p )
{	
	//m_Fade.CloseWipingLeft( SM_GoToIntroCovers );
}

