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
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"


#define ICONS_START_X		THEME->GetMetricF("ScreenSelectStyle","IconsStartX")
#define ICONS_SPACING_X		THEME->GetMetricF("ScreenSelectStyle","IconsSpacingX")
#define ICONS_START_Y		THEME->GetMetricF("ScreenSelectStyle","IconsStartY")
#define ICONS_SPACING_Y		THEME->GetMetricF("ScreenSelectStyle","IconsSpacingY")
#define EXPLANATION_X		THEME->GetMetricF("ScreenSelectStyle","ExplanationX")
#define EXPLANATION_Y		THEME->GetMetricF("ScreenSelectStyle","ExplanationY")
#define INFO_X				THEME->GetMetricF("ScreenSelectStyle","InfoX")
#define INFO_Y				THEME->GetMetricF("ScreenSelectStyle","InfoY")
#define PREVIEW_X			THEME->GetMetricF("ScreenSelectStyle","PreviewX")
#define PREVIEW_Y			THEME->GetMetricF("ScreenSelectStyle","PreviewY")
#define HELP_TEXT			THEME->GetMetric("ScreenSelectStyle","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenSelectStyle","TimerSeconds")
#define NEXT_SCREEN			THEME->GetMetric("ScreenSelectStyle","NextScreen")


const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User + 2);


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
		this->AddChild( &m_sprIcon[i] );
	}

	UpdateEnabledDisabled();

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","select style explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddChild( &m_sprExplanation );
	
	m_sprPreview.SetXY( PREVIEW_X, PREVIEW_Y );
	this->AddChild( &m_sprPreview );
	
	m_sprInfo.SetXY( INFO_X, INFO_Y );
	this->AddChild( &m_sprInfo );
	

	// Load dummy Sprites
	for( i=0; i<m_aPossibleStyles.GetSize(); i++ )
	{
		m_sprDummyPreview[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style preview game %d style %d",GAMESTATE->m_CurGame,i)) );
		m_sprDummyInfo[i].Load(    THEME->GetPathTo("Graphics",ssprintf("select style info game %d style %d",GAMESTATE->m_CurGame,i)) );
	}


	m_Menu.Load( 	
		THEME->GetPathTo("Graphics","select style background"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo("Sounds","select style change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );


	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style intro") );

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select style music") );

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
	case SM_GoToPrevScreen:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
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
	m_sprPreview.SetGlow( D3DXCOLOR(1,1,1,0) );
	m_sprPreview.SetDiffuse( D3DXCOLOR(1,1,1,0) );

	m_sprPreview.BeginTweening( 0.25f );			// sleep

	m_sprPreview.BeginTweening( 0.2f );			// fade to white
	m_sprPreview.SetTweenGlow( D3DXCOLOR(1,1,1,1) );
	m_sprPreview.SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );

	m_sprPreview.BeginTweening( 0.01f );			// turn color on
	m_sprPreview.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );

	m_sprPreview.BeginTweening( 0.2f );			// fade to color
	m_sprPreview.SetTweenGlow( D3DXCOLOR(1,1,1,0) );
	m_sprPreview.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );


	// Tween Info
	m_sprInfo.Load( THEME->GetPathTo("Graphics",ssprintf("select style info game %d style %d",GAMESTATE->m_CurGame,m_iSelection)) );
	m_sprInfo.StopTweening();
	m_sprInfo.SetZoomY( 0 );
	m_sprInfo.BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprInfo.SetTweenZoomY( 1 );
}

void ScreenSelectStyle::MenuLeft( PlayerNumber p )
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


void ScreenSelectStyle::MenuRight( PlayerNumber p )
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

void ScreenSelectStyle::MenuStart( PlayerNumber p )
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

	CString sCurStyleName = GAMESTATE->GetCurrentStyleDef()->m_szName;
	sCurStyleName.MakeLower();
	if(	     -1!=sCurStyleName.Find("single") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment single") );
	else if( -1!=sCurStyleName.Find("versus") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment versus") );
	else if( -1!=sCurStyleName.Find("double") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment double") );
	else if( -1!=sCurStyleName.Find("couple") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment couple") );
	else if( -1!=sCurStyleName.Find("solo") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment solo") );

	m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );

	m_soundSelect.PlayRandom();

	m_Menu.StopTimer();

	TweenOffScreen();
}

void ScreenSelectStyle::MenuBack( PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );

//	m_Fade.CloseWipingLeft( SM_GoToPrevScreen );

//	TweenOffScreen();
}


void ScreenSelectStyle::TweenOnScreen() 
{
	for( int i=0; i<m_aPossibleStyles.GetSize(); i++ )
		m_sprIcon[i].FadeOn( (m_aPossibleStyles.GetSize()-i)*0.05f, "Left Accelerate", MENU_ELEMENTS_TWEEN_TIME );

	m_sprExplanation.FadeOn( 0, "Right Accelerate", MENU_ELEMENTS_TWEEN_TIME );

	// let AfterChange tween Preview and Info
}

void ScreenSelectStyle::TweenOffScreen()
{
	for( int i=0; i<m_aPossibleStyles.GetSize(); i++ )
		m_sprIcon[i].FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );

	m_sprExplanation.FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );

	m_sprPreview.FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );

	m_sprInfo.FadeOff( 0, "FoldY", MENU_ELEMENTS_TWEEN_TIME );
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
			m_sprIcon[i].SetDiffuse( D3DXCOLOR(1,1,1,1) );
		else
			m_sprIcon[i].SetDiffuse( D3DXCOLOR(0.5f,0.5f,0.5f,1) );
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

