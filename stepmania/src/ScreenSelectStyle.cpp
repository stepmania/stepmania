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
#include "ScreenSelectGroup.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSandbox.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"


#define ICONS_START_X		THEME->GetMetricF("SelectStyle","IconsStartX")
#define ICONS_SPACING_X		THEME->GetMetricF("SelectStyle","IconsSpacingX")
#define ICONS_START_Y		THEME->GetMetricF("SelectStyle","IconsStartY")
#define ICONS_SPACING_Y		THEME->GetMetricF("SelectStyle","IconsSpacingY")
#define EXPLANATION_X		THEME->GetMetricF("SelectStyle","ExplanationX")
#define EXPLANATION_Y		THEME->GetMetricF("SelectStyle","ExplanationY")
#define INFO_X				THEME->GetMetricF("SelectStyle","InfoX")
#define INFO_Y				THEME->GetMetricF("SelectStyle","InfoY")
#define PREVIEW_X			THEME->GetMetricF("SelectStyle","PreviewX")
#define PREVIEW_Y			THEME->GetMetricF("SelectStyle","PreviewY")
#define HELP_TEXT			THEME->GetMetric("SelectStyle","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("SelectStyle","TimerSeconds")

#define SKIP_SELECT_DIFFICULTY		THEME->GetMetricB("General","SkipSelectDifficulty")

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);


ScreenSelectStyle::ScreenSelectStyle()
{
	LOG->Trace( "ScreenSelectStyle::ScreenSelectStyle()" );

	// Reset the current style and game
	GAMESTATE->m_CurStyle = STYLE_NONE;
	GAMESTATE->m_bPlayersCanJoin = true;

	GAMEMAN->GetGameplayStylesForGame( GAMESTATE->m_CurGame, m_aPossibleStyles );
	ASSERT( m_aPossibleStyles.GetSize() > 0 );	// every game should have at least one Style, or else why have the Game? :-)
	m_iSelection = 0;

	for( int i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		Style style = m_aPossibleStyles[i];
		m_sprIcon[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style icons game %d",GAMESTATE->m_CurGame) ) );
		m_sprIcon[i].StopAnimating();
		m_sprIcon[i].SetState( i );
		m_sprIcon[i].SetXY( ICONS_START_X + i*ICONS_SPACING_X, ICONS_START_Y + i*ICONS_SPACING_Y );
		this->AddSubActor( &m_sprIcon[i] );
	}

	UpdateEnabledDisabled();

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","select style explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddSubActor( &m_sprExplanation );
	
	m_sprPreview.SetXY( PREVIEW_X, PREVIEW_Y );
	this->AddSubActor( &m_sprPreview );
	
	m_sprInfo.SetXY( INFO_X, INFO_Y );
	this->AddSubActor( &m_sprInfo );
	

	// Load dummy Sprites
	for( i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		m_sprDummyPreview[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style preview game %d style %d",GAMESTATE->m_CurGame,i)) );
		m_sprDummyInfo[i].Load(    THEME->GetPathTo("Graphics",ssprintf("select style info game %d style %d",GAMESTATE->m_CurGame,i)) );
	}


	m_Menu.Load( 	
		THEME->GetPathTo("Graphics","select style background"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddSubActor( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo("Sounds","select style change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );


	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_STYLE_INTRO) );


	if( !MUSIC->IsPlaying()  ||  MUSIC->GetLoadedFilePath() != THEME->GetPathTo("Sounds","select style music") )
	{
		MUSIC->Load( THEME->GetPathTo("Sounds","select style music") );
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
		if( SKIP_SELECT_DIFFICULTY )
			SCREENMAN->SetNewScreen( new ScreenSelectGroup );
		else
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

	// Tween Preview
	m_sprPreview.Load( THEME->GetPathTo("Graphics",ssprintf("select style preview game %d style %d",GAMESTATE->m_CurGame,m_iSelection)) );

	m_sprPreview.StopTweening();
	m_sprPreview.SetGlowColor( D3DXCOLOR(1,1,1,0) );
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
	m_sprInfo.Load( THEME->GetPathTo("Graphics",ssprintf("select style info game %d style %d",GAMESTATE->m_CurGame,m_iSelection)) );
	m_sprInfo.StopTweening();
	m_sprInfo.SetZoomY( 0 );
	m_sprInfo.BeginTweeningQueued( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprInfo.SetTweenZoomY( 1 );
}

void ScreenSelectStyle::MenuLeft( const PlayerNumber p )
{
	// search for a style to the left of the current selection that is enabled
	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( int i=m_iSelection-1; i>=0; i-- )
	{
		if( IsEnabled(i) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}

	if( iSwitchToStyleIndex == -1 )
		return;

	BeforeChange();
	m_iSelection = iSwitchToStyleIndex;
	m_soundChange.PlayRandom();
	AfterChange();
}


void ScreenSelectStyle::MenuRight( const PlayerNumber p )
{
	// search for a style to the right of the current selection that is enabled
	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( int i=m_iSelection+1; i<m_aPossibleStyles.GetSize(); i++ )	
	{
		if( IsEnabled(i) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}

	if( iSwitchToStyleIndex == -1 )
		return;

	BeforeChange();
	m_iSelection = iSwitchToStyleIndex;
	m_soundChange.PlayRandom();
	AfterChange();
}

void ScreenSelectStyle::MenuStart( const PlayerNumber p )
{
	if( p!=PLAYER_INVALID  && !GAMESTATE->m_bSideIsJoined[p] )
	{
		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
		GAMESTATE->m_bSideIsJoined[p] = true;
		SCREENMAN->RefreshCreditsMessages();
		UpdateEnabledDisabled();
		return;	// don't fall through
	}

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

	m_Menu.StopTimer();

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

bool ScreenSelectStyle::IsEnabled( int iStyleIndex )
{
	Style style = m_aPossibleStyles[iStyleIndex];

	int iNumSidesJoined = 0;
	for( int c=0; c<2; c++ )
		if( GAMESTATE->m_bSideIsJoined[c] )
			iNumSidesJoined++;	// left side, and right side

	switch( GAMEMAN->GetStyleDefForStyle(style)->m_StyleType )
	{
	case StyleDef::ONE_PLAYER_ONE_CREDIT:	return iNumSidesJoined==1;
	case StyleDef::ONE_PLAYER_TWO_CREDITS:	return iNumSidesJoined==2;
	case StyleDef::TWO_PLAYERS_TWO_CREDITS:	return iNumSidesJoined==2;
	default:	ASSERT(0);	return false;
	}
}

void ScreenSelectStyle::UpdateEnabledDisabled()
{
	int i;
	for( i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		if( IsEnabled(i) )
			m_sprIcon[i].SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
		else
			m_sprIcon[i].SetDiffuseColor( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
	}

	// Select first enabled style
	BeforeChange();

	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		if( IsEnabled(i) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}
	ASSERT( iSwitchToStyleIndex != -1 );	// no styles are enabled.  We're stuck!  This should never happen

	m_iSelection = iSwitchToStyleIndex;
	AfterChange();
}

