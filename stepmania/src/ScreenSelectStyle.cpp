#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle

 Desc: Testing the Screen class.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectStyle.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "ScreenCaution.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSandbox.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"


const float ICONS_START_X	= SCREEN_LEFT + 60;
const float ICONS_SPACING_X	= 76;
const float ICON_Y			= SCREEN_TOP + 100;

const float EXPLANATION_X	= SCREEN_RIGHT - 160;
const float EXPLANATION_Y	= CENTER_Y-70;

const float INFO_X			= SCREEN_RIGHT - 160;
const float INFO_Y			= CENTER_Y+40;

const float PREVIEW_X		= SCREEN_LEFT + 160;
const float PREVIEW_Y		= CENTER_Y;


const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);


ScreenSelectStyle::ScreenSelectStyle()
{
	LOG->Trace( "ScreenSelectStyle::ScreenSelectStyle()" );


	// Reset the current style and game
	GAMESTATE->m_CurStyle = STYLE_NONE;


	for( int s=0; s<NUM_STYLES; s++ )
	{
		Style style = (Style)s;
		if( StyleToGame(style) == GAMESTATE->m_CurGame )	// games match
			m_aPossibleStyles.Add( style );		
	}

	m_iSelection = 0;

	for( int i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		Style style = m_aPossibleStyles[i];
		m_sprIcon[i].Load( THEME->GetPathTo(GRAPHIC_SELECT_STYLE_ICONS) );
		m_sprIcon[i].StopAnimating();
		m_sprIcon[i].SetState( i );
		m_sprIcon[i].SetXY( ICONS_START_X + ICONS_SPACING_X*i, ICON_Y );
		this->AddSubActor( &m_sprIcon[i] );
	}

	m_sprExplanation.Load( THEME->GetPathTo(GRAPHIC_SELECT_STYLE_EXPLANATION) );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddSubActor( &m_sprExplanation );
	
	m_sprPreview.SetXY( PREVIEW_X, PREVIEW_Y );
	this->AddSubActor( &m_sprPreview );
	
	m_sprInfo.SetXY( INFO_X, INFO_Y );
	this->AddSubActor( &m_sprInfo );
	

	// Load dummy Sprites
	for( i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		ThemeElement te;

		te = (ThemeElement)(GRAPHIC_SELECT_STYLE_PREVIEW_0+i);
		m_sprDummyPreview[i].Load( THEME->GetPathTo(te) );

		te = (ThemeElement)(GRAPHIC_SELECT_STYLE_INFO_0+i);
		m_sprDummyInfo[i].Load( THEME->GetPathTo(te) );
	}


	m_Menu.Load( 	
		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_BACKGROUND), 
		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_TOP_EDGE),
		ssprintf("Use %c %c to select, then press START", char(1), char(2) ),
		false, true, 40 
		);
	this->AddSubActor( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo(SOUND_SELECT_STYLE_CHANGE) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );


	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_STYLE_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
        MUSIC->Play( true );
	}

	AfterChange();
	TweenOnScreen();
	m_Menu.TweenOnScreenFromBlack( SM_None );
}


ScreenSelectStyle::~ScreenSelectStyle()
{
	LOG->Trace( "ScreenSelectStyle::~ScreenSelectStyle()" );
}

void ScreenSelectStyle::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectStyle::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectStyle::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectStyle::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart(PLAYER_INVALID);
		break;
	case SM_GoToPrevState:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenSelectDifficulty );
		break;
	}
}

void ScreenSelectStyle::BeforeChange()
{
	m_sprIcon[m_iSelection].SetEffectNone();
}

void ScreenSelectStyle::AfterChange()
{
	m_sprIcon[m_iSelection].SetEffectGlowing();

	ThemeElement te;

	// Tween Preview
	te = (ThemeElement)(GRAPHIC_SELECT_STYLE_PREVIEW_0+m_iSelection);
	m_sprPreview.Load( THEME->GetPathTo(te) );

	m_sprPreview.StopTweening();
	m_sprPreview.SetAddColor( D3DXCOLOR(1,1,1,0) );
	m_sprPreview.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );

	m_sprPreview.BeginTweeningQueued( 0.25f );			// sleep

	m_sprPreview.BeginTweeningQueued( 0.2f );			// fade to white
	m_sprPreview.SetTweenAddColor( D3DXCOLOR(1,1,1,1) );
	m_sprPreview.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );

	m_sprPreview.BeginTweeningQueued( 0.01f );			// turn color on
	m_sprPreview.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_sprPreview.BeginTweeningQueued( 0.2f );			// fade to color
	m_sprPreview.SetTweenAddColor( D3DXCOLOR(1,1,1,0) );
	m_sprPreview.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );


	// Tween Info
	te = (ThemeElement)(GRAPHIC_SELECT_STYLE_INFO_0+m_iSelection);
	m_sprInfo.Load( THEME->GetPathTo(te) );
	m_sprInfo.StopTweening();
	m_sprInfo.SetZoomY( 0 );
	m_sprInfo.BeginTweeningQueued( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprInfo.SetTweenZoomY( 1 );
}

void ScreenSelectStyle::MenuLeft( const PlayerNumber p )
{
	if( m_iSelection == 0 )		// can't go left any further
		return;

	BeforeChange();
	m_iSelection--;
	m_soundChange.PlayRandom();
	AfterChange();
}


void ScreenSelectStyle::MenuRight( const PlayerNumber p )
{
	if( m_iSelection == m_aPossibleStyles.GetSize()-1 )		// can't go right any further
		return;

	BeforeChange();
	m_iSelection++;
	m_soundChange.PlayRandom();
	AfterChange();
}

void ScreenSelectStyle::MenuStart( const PlayerNumber p )
{
	if( p != PLAYER_INVALID )
		GAMESTATE->m_MasterPlayerNumber = p;
	GAMESTATE->m_CurStyle = GetSelectedStyle();

	AnnouncerElement ae;
	switch( GAMESTATE->m_CurStyle )
	{
		case STYLE_DANCE_SINGLE:		ae = ANNOUNCER_SELECT_STYLE_COMMENT_SINGLE;		break;
		case STYLE_DANCE_VERSUS:		ae = ANNOUNCER_SELECT_STYLE_COMMENT_VERSUS;		break;
		case STYLE_DANCE_DOUBLE:		ae = ANNOUNCER_SELECT_STYLE_COMMENT_DOUBLE;		break;
		case STYLE_DANCE_COUPLE:		ae = ANNOUNCER_SELECT_STYLE_COMMENT_COUPLE;		break;
		case STYLE_DANCE_SOLO:			ae = ANNOUNCER_SELECT_STYLE_COMMENT_SOLO;		break;
		case STYLE_PUMP_SINGLE:			ae = ANNOUNCER_SELECT_STYLE_COMMENT_SINGLE;		break;
		case STYLE_PUMP_VERSUS:			ae = ANNOUNCER_SELECT_STYLE_COMMENT_VERSUS;		break;
		default:	ASSERT(0);	break;	// invalid Style
	}
	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ae) );

	m_Menu.TweenOffScreenToMenu( SM_GoToNextState );

	m_soundSelect.PlayRandom();

	TweenOffScreen();
}

void ScreenSelectStyle::MenuBack( const PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );

//	m_Fade.CloseWipingLeft( SM_GoToPrevState );

//	TweenOffScreen();
}


void ScreenSelectStyle::TweenOffScreen()
{
	for( int i=0; i<NUM_STYLES; i++ )
	{
		m_sprIcon[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprIcon[i].SetTweenZoomY( 0 );
	}

	m_sprExplanation.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprExplanation.SetTweenZoomY( 0 );

	m_sprPreview.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprPreview.SetTweenZoomY( 0 );

	m_sprInfo.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprInfo.SetTweenZoomX( 0 );
}
  

void ScreenSelectStyle::TweenOnScreen() 
{
	float fOriginalZoom;

	for( int i=0; i<NUM_STYLES; i++ )
	{
		fOriginalZoom = m_sprIcon[i].GetZoomY();
		m_sprIcon[i].SetZoomY( 0 );
		m_sprIcon[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprIcon[i].SetTweenZoomY( fOriginalZoom );
	}

	fOriginalZoom = m_sprExplanation.GetZoomY();
	m_sprExplanation.SetZoomY( 0 );
	m_sprExplanation.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprExplanation.SetTweenZoomY( fOriginalZoom );


	// let AfterChange tween Preview and Info
	/*	
	fOriginalZoom = m_sprPreview.GetZoomY();
	m_sprPreview.SetZoomY( 0 );
	m_sprPreview.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprPreview.SetTweenZoomY( fOriginalZoom );

	fOriginalZoom = m_sprInfo.GetZoomY();
	m_sprInfo.SetZoomY( 0 );
	m_sprInfo.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprInfo.SetTweenZoomY( fOriginalZoom );
	*/
}
