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
#include "ScreenGraphicOptions.h"
#include "ScreenGameOptions.h"
#include "ScreenEdit.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "ScreenEditMenu.h"
#include "ScreenSelectStyle.h"
#include "ScreenSelectGame.h"
#include "ScreenAppearanceOptions.h"
#include "RageLog.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "ScreenEz2SelectPlayer.h"
#include "GameState.h"
#include "GameManager.h"
#include "ScreenGameplay.h"


const CString CHOICE_TEXT[ScreenTitleMenu::NUM_TITLE_MENU_CHOICES] = {
	"GAME START",
	"SWITCH GAME",
	"CONFIG KEY/JOY",
	"GAME OPTIONS",
	"GRAPHIC OPTIONS",
	"APPEARANCE OPTIONS",
	"EDIT/RECORD/SYNCH",
	"EXIT",
};

#define CHOICES_X						THEME->GetMetricF("TitleMenu","ChoicesX")
#define CHOICES_START_Y					THEME->GetMetricF("TitleMenu","ChoicesStartY")
#define CHOICES_SPACING_Y				THEME->GetMetricF("TitleMenu","ChoicesSpacingY")
#define HELP_X							THEME->GetMetricF("TitleMenu","HelpX")
#define HELP_Y							THEME->GetMetricF("TitleMenu","HelpY")
#define LOGO_X							THEME->GetMetricF("TitleMenu","LogoX")
#define LOGO_Y							THEME->GetMetricF("TitleMenu","LogoY")
#define VERSION_X						THEME->GetMetricF("TitleMenu","VersionX")
#define VERSION_Y						THEME->GetMetricF("TitleMenu","VersionY")
#define SONGS_X							THEME->GetMetricF("TitleMenu","SongsX")
#define SONGS_Y							THEME->GetMetricF("TitleMenu","SongsY")
#define COLOR_NOT_SELECTED				THEME->GetMetricC("TitleMenu","ColorNotSelected")
#define COLOR_SELECTED					THEME->GetMetricC("TitleMenu","ColorSelected")
#define SECONDS_BEFORE_DEMONSTRATION	THEME->GetMetricF("TitleMenu","SecondsBeforeDemonstration")
#define SECONDS_BETWEEN_ATTRACT			THEME->GetMetricF("TitleMenu","SecondsBetweenAttract")

#define USE_CAUTION_OR_SELECT_PLAYER	THEME->GetMetricB("General","UseCautionOrSelectPlayer")

const ScreenMessage SM_PlayAttract			=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User+12);
const ScreenMessage SM_FadeToDemonstration	=	ScreenMessage(SM_User+13);
const ScreenMessage SM_GoToDemonstration	=	ScreenMessage(SM_User+14);


ScreenTitleMenu::ScreenTitleMenu()
{
	MUSIC->Stop();
	LOG->Trace( "ScreenTitleMenu::ScreenTitleMenu()" );


	//
	// I think it's better to do all the initialization here rather than have it scattered
	// about in all the global singleton classes
	//
	GAMESTATE->Reset();
	PREFSMAN->ReadGamePrefsFromDisk();
	INPUTMAPPER->ReadMappingsFromDisk();
	GAMESTATE->m_bPlayersCanJoin = true;
	if( !GAMEMAN->DoesNoteSkinExist( GAMEMAN->GetCurNoteSkin() ) )
	{
		CStringArray asNoteSkinNames;
		GAMEMAN->GetNoteSkinNames( asNoteSkinNames );
		GAMEMAN->SwitchNoteSkin( asNoteSkinNames[0] );
	}
	if( !THEME->DoesThemeExist( THEME->GetCurThemeName() ) )
	{
		CStringArray asThemeNames;
		THEME->GetThemeNamesForCurGame( asThemeNames );
		THEME->SwitchTheme( asThemeNames[0] );
	}


	m_sprBG.Load( THEME->GetPathTo("Graphics","title menu background") );
	m_sprBG.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	this->AddSubActor( &m_sprBG );

	m_sprLogo.Load( THEME->GetPathTo("Graphics",ssprintf("title menu logo game %d",GAMESTATE->m_CurGame)) );
	m_sprLogo.SetXY( LOGO_X, LOGO_Y );
	m_sprLogo.SetAddColor( D3DXCOLOR(1,1,1,1) );
	m_sprLogo.SetZoomY( 0 );
	m_sprLogo.BeginTweeningQueued( 0.5f );	// sleep
	m_sprLogo.BeginTweeningQueued( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprLogo.SetEffectGlowing(1, D3DXCOLOR(1,1,1,0.1f), D3DXCOLOR(1,1,1,0.3f) );
	m_sprLogo.SetTweenZoom( 1 );
	this->AddSubActor( &m_sprLogo );

	m_textHelp.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textHelp.SetText( ssprintf("Use %c %c to select, then press START", char(3), char(4)) );
	m_textHelp.SetXY( HELP_X, HELP_Y );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetEffectBlinking();
	m_textHelp.SetShadowLength( 2 );
	this->AddSubActor( &m_textHelp );

	
	m_textVersion.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textVersion.SetText( "v3.0 beta 6" );
	m_textVersion.SetDiffuseColor( D3DXCOLOR(0.6f,0.6f,0.6f,1) );	// light gray
	m_textVersion.SetXY( VERSION_X, VERSION_Y );
	m_textVersion.SetZoom( 0.5f );
	m_textVersion.SetShadowLength( 2 );
	this->AddSubActor( &m_textVersion );


	m_textSongs.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongs.SetHorizAlign( Actor::align_left );
	m_textSongs.SetText( ssprintf("Found %d Songs", SONGMAN->m_pSongs.GetSize()) );
	m_textSongs.SetDiffuseColor( D3DXCOLOR(0.6f,0.6f,0.6f,1) );	// light gray
	m_textSongs.SetXY( SONGS_X, SONGS_Y );
	m_textSongs.SetZoom( 0.5f );
	m_textSongs.SetShadowLength( 2 );
	this->AddSubActor( &m_textSongs );


	for( int i=0; i< NUM_TITLE_MENU_CHOICES; i++ )
	{
		m_textChoice[i].LoadFromFont( THEME->GetPathTo("Fonts","header1") );
		m_textChoice[i].SetText( CHOICE_TEXT[i] );
		m_textChoice[i].SetXY( CHOICES_X, CHOICES_START_Y + i*CHOICES_SPACING_Y );
		m_textChoice[i].SetShadowLength( 5 );
		this->AddSubActor( &m_textChoice[i] );
	}	
	
	m_Fade.SetClosed();
	m_Fade.OpenWipingRight( SM_None );

	this->AddSubActor( &m_Fade );


	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_TITLE_MENU_GAME_NAME) );


	m_soundAttract.Load( ANNOUNCER->GetPathTo(ANNOUNCER_TITLE_MENU_ATTRACT) );
	m_soundChange.Load( THEME->GetPathTo("Sounds","title menu change") );	
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundInvalid.Load( THEME->GetPathTo("Sounds","menu invalid") );


	m_TitleMenuChoice = CHOICE_GAME_START;
	GainFocus( m_TitleMenuChoice );
	
	MUSIC->Stop();

	for( float f=SECONDS_BETWEEN_ATTRACT; f<SECONDS_BEFORE_DEMONSTRATION; f+=SECONDS_BETWEEN_ATTRACT )
		this->SendScreenMessage( SM_PlayAttract, f );
	this->SendScreenMessage( SM_FadeToDemonstration, SECONDS_BEFORE_DEMONSTRATION );
}


ScreenTitleMenu::~ScreenTitleMenu()
{
	LOG->Trace( "ScreenTitleMenu::~ScreenTitleMenu()" );
}


void ScreenTitleMenu::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenTitleMenu::Input()" );

	if( m_Fade.IsClosing() )
		return;
	
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenTitleMenu::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PlayAttract:
		m_soundAttract.PlayRandom();
		break;
	case SM_GoToNextScreen:
		switch( m_TitleMenuChoice )
		{
		case CHOICE_GAME_START:
			if( USE_CAUTION_OR_SELECT_PLAYER )
				SCREENMAN->SetNewScreen( new ScreenEz2SelectPlayer );
			else
				SCREENMAN->SetNewScreen( new ScreenCaution );
			break;
		case CHOICE_SELECT_GAME:
			SCREENMAN->SetNewScreen( new ScreenSelectGame );
			break;
		case CHOICE_MAP_INSTRUMENTS:
			SCREENMAN->SetNewScreen( new ScreenMapInstruments );
			break;
		case CHOICE_GAME_OPTIONS:
			SCREENMAN->SetNewScreen( new ScreenGameOptions );
			break;
		case CHOICE_GRAPHIC_OPTIONS:
			SCREENMAN->SetNewScreen( new ScreenGraphicOptions );
			break;
		case CHOICE_APPEARANCE_OPTIONS:
			SCREENMAN->SetNewScreen( new ScreenAppearanceOptions );
			break;
		case CHOICE_EDIT:
			SCREENMAN->SetNewScreen( new ScreenEditMenu );
			break;
		case CHOICE_EXIT:
		default:
			ASSERT(0);	// should never get here
			break;
		}
		break;
	case SM_FadeToDemonstration:
		{	
			// Set up the game state for a demonstration
			switch( GAMESTATE->m_CurGame )
			{
			case GAME_DANCE:	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;			break; 
			case GAME_PUMP:		GAMESTATE->m_CurStyle = STYLE_PUMP_VERSUS;			break; 
			case GAME_EZ2:		GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_VERSUS;	break; 
			default:	ASSERT(0);
			}

			GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;

			if( SONGMAN->m_pSongs.GetSize() == 0 )
				goto abort_demonstration;

			// choose a Song and Notes
			Song* pSongToPlay;
			Notes* pNotesToPlay;
			int i;
			for( i=0; i<600; i++ )	// try 600 times
			{
				Song* pSong = SONGMAN->m_pSongs[ rand()%SONGMAN->m_pSongs.GetSize() ];
				for( int j=0; j<3; j++ )	// try 3 times
				{
					if( pSong->m_apNotes.GetSize() == 0 )
						continue;

					Notes* pNotes = pSong->m_apNotes[ rand()%pSong->m_apNotes.GetSize() ];
					if( pNotes->m_NotesType == GAMESTATE->GetCurrentStyleDef()->m_NotesType )
					{
						// found something we can use!
						pSongToPlay = pSong;
						pNotesToPlay = pNotes;
						goto found_song_and_notes;
					}
				}
			}
abort_demonstration:
			// Couldn't find Song/Notes to play.  Abort demonstration!
			GAMESTATE->Reset();
			return;

found_song_and_notes:
			ASSERT( pSongToPlay );

			GAMESTATE->m_pCurSong = pSongToPlay;
			for( int p=0; p<NUM_PLAYERS; p++ )
				GAMESTATE->m_pCurNotes[p] = pNotesToPlay;

			// choose some cool options
			for( p=0; p<NUM_PLAYERS; p++ )
			{
				if( GAMESTATE->IsPlayerEnabled(p) )
				{
					if( RandomFloat(0,1)>0.8f )
						GAMESTATE->m_PlayerOptions[p].m_fArrowScrollSpeed = 1.5f;
					GAMESTATE->m_PlayerOptions[p].m_EffectType = PlayerOptions::EffectType(rand()%PlayerOptions::NUM_EFFECT_TYPES);
					if( RandomFloat(0,1)>0.9f )
						GAMESTATE->m_PlayerOptions[p].m_AppearanceType = PlayerOptions::APPEARANCE_HIDDEN;
					if( RandomFloat(0,1)>0.9f )
						GAMESTATE->m_PlayerOptions[p].m_AppearanceType = PlayerOptions::APPEARANCE_SUDDEN;
					if( RandomFloat(0,1)>0.7f )
						GAMESTATE->m_PlayerOptions[p].m_bReverseScroll = true;
					if( RandomFloat(0,1)>0.9f )
						GAMESTATE->m_PlayerOptions[p].m_bDark = true;
				}
			}
			GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LifeType(rand()%SongOptions::NUM_LIFE_TYPES);
			GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_OFF;

			GAMESTATE->m_bDemonstration = true;
			m_Fade.CloseWipingRight( SM_GoToDemonstration );
		}
		break;
	case SM_GoToDemonstration:
		{
			ASSERT( GAMESTATE->m_pCurSong );
			SCREENMAN->SetNewScreen( new ScreenGameplay );
		}
		break;
	}
}


void ScreenTitleMenu::LoseFocus( int iChoiceIndex )
{
	m_soundChange.PlayRandom();

	m_textChoice[iChoiceIndex].SetEffectNone();
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetTweenZoom( 0.9f );

}

void ScreenTitleMenu::GainFocus( int iChoiceIndex )
{
	m_textChoice[iChoiceIndex].BeginTweening( 0.3f );
	m_textChoice[iChoiceIndex].SetTweenZoom( 1.2f );
	D3DXCOLOR color1, color2;
	color1 = COLOR_SELECTED;
	color2 = color1 * 0.5f;
	color2.a = 1;
	m_textChoice[iChoiceIndex].SetEffectCamelion( 2.5f, color1, color2 );
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
	GAMESTATE->m_bIsJoined[p] = true;

	GAMESTATE->m_bPlayersCanJoin = false;


	switch( m_TitleMenuChoice )
	{
	case CHOICE_GAME_START:
	case CHOICE_SELECT_GAME:
	case CHOICE_MAP_INSTRUMENTS:
	case CHOICE_GAME_OPTIONS:
	case CHOICE_GRAPHIC_OPTIONS:
	case CHOICE_APPEARANCE_OPTIONS:
		m_soundSelect.PlayRandom();
		m_Fade.CloseWipingRight( SM_GoToNextScreen );
		break;
	case CHOICE_EDIT:
		if( SONGMAN->m_pSongs.GetSize() == 0 )
		{
			m_soundInvalid.PlayRandom();
		}
		else
		{
			m_soundSelect.PlayRandom();
			m_Fade.CloseWipingRight( SM_GoToNextScreen );
		}
		break;
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
	default:
		ASSERT(0);
	}
}

void ScreenTitleMenu::MenuBack( const PlayerNumber p )
{	
	this->SendScreenMessage( SM_FadeToDemonstration, 0 );
}

