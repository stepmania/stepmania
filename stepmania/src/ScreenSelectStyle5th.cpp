#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle5th

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectStyle5th.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageMusic.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"

#define HELP_TEXT			THEME->GetMetric("ScreenSelectStyle5th","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenSelectStyle5th","TimerSeconds")
#define NEXT_SCREEN			THEME->GetMetric("ScreenSelectStyle5th","NextScreen")

const CString DANCE_STYLES[NUM_DANCE_STYLES] = {
	"single",
	"versus",
	"double",
	"couple",
	"solo",
};

const float PAD_X[NUM_STYLE_PADS] = {
	CENTER_X-250,
	CENTER_X-125,
	CENTER_X-20,
	CENTER_X+125,
	CENTER_X+250,
};
const float PAD_Y[NUM_STYLE_PADS] = {
	CENTER_Y+60,
	CENTER_Y-30,
	CENTER_Y+70,
	CENTER_Y-30,
	CENTER_Y+60,
};
const float DANCER_X[NUM_STYLE_DANCERS] = {
	PAD_X[0],
	PAD_X[1]-35,
	PAD_X[1]+35,
	PAD_X[2],
	PAD_X[3]-35,
	PAD_X[3]+35,
	PAD_X[4],
};
const float DANCER_Y[NUM_STYLE_DANCERS] = {
	PAD_Y[0],
	PAD_Y[1]+20,
	PAD_Y[1]-20,
	PAD_Y[2],
	PAD_Y[3]+20,
	PAD_Y[3]-20,
	PAD_Y[4],
};

CString TEXT_EXPLANATION1[NUM_DANCE_STYLES] = {
	"1 Player",
	"2 Players",
	"1 Player",
	"2 Players",
	"1 Player"
};
CString TEXT_EXPLANATION2[NUM_DANCE_STYLES] = {
	"Dance using 4 panels on one pad",
	"Each uses 4 panels on one pad",
	"Dance using 8 panels on two pads",
	"Each uses 4 panels on one pad",
	"Dance using 6 panels on one pad"
};

const float EXPLANATION1_ZOOM_X		=	0.8f;
const float EXPLANATION1_ZOOM_Y		=	1;
const float EXPLANATION2_ZOOM_X		=	0.5f;
const float EXPLANATION2_ZOOM_Y		=	0.7f;

const float ICON_X		= 120;
const float ICON_Y		= SCREEN_HEIGHT - 70;

const float EXPLANATION1_X		= ICON_X+110;
const float EXPLANATION1_Y		= ICON_Y-16;
const float EXPLANATION2_X		= EXPLANATION1_X;
const float EXPLANATION2_Y		= ICON_Y+16;

const float HELP_X		= CENTER_X;
const float HELP_Y		= SCREEN_HEIGHT-20;

const float TWEEN_TIME		= 0.35f;

const RageColor COLOR_P1_SELECTED = RageColor(0.4f,1.0f,0.8f,1);
const RageColor COLOR_P2_SELECTED = RageColor(1.0f,0.5f,0.2f,1);
const RageColor COLOR_P1_NOT_SELECTED = COLOR_P1_SELECTED*0.5f + RageColor(0,0,0,0.5f);
const RageColor COLOR_P2_NOT_SELECTED = COLOR_P2_SELECTED*0.5f + RageColor(0,0,0,0.5f);


const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_UpdateAnimations		=	ScreenMessage(SM_User + 3);
const ScreenMessage SM_TweenExplanation2	=	ScreenMessage(SM_User + 4);


ScreenSelectStyle5th::ScreenSelectStyle5th()
{
	LOG->Trace( "ScreenSelectStyle5th::ScreenSelectStyle5th()" );

	m_iSelection = 0;	// single

	int i;
	for( i=0; i<NUM_STYLE_PADS; i++ )
	{
		m_sprPad[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style pad game %d style %d",GAMESTATE->m_CurGame,i)) );
//		m_sprPad[i].SetXY( PAD_X[i], PAD_Y[i] );
//		m_sprPad[i].SetZoom( 1 );
		this->AddChild( &m_sprPad[i] );
	}


	for( i=0; i<NUM_STYLE_DANCERS; i++ )
	{
		m_sprDancer[i].Load( THEME->GetPathTo("Graphics",ssprintf("select style player game %d style %d",GAMESTATE->m_CurGame,i)) );
		m_sprDancer[i].SetVertAlign( Actor::align_top );
//		m_sprDancer[i].SetXY( DANCER_X[i], DANCER_Y[i] );
//		m_sprDancer[i].SetZoom( 2 );
		m_sprDancer[i].StopAnimating();
		this->AddChild( &m_sprDancer[i] );
	}


	m_sprStyleIcon.Load( THEME->GetPathTo("Graphics",ssprintf("select style icons game %d",GAMESTATE->m_CurGame)) );
	m_sprStyleIcon.TurnShadowOff();
	m_sprStyleIcon.StopAnimating();
	m_sprStyleIcon.SetXY( ICON_X, ICON_Y );
	this->AddChild( &m_sprStyleIcon );


	m_textExplanation1.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textExplanation1.SetDiffuse( RageColor(0,0.7f,0,1) );
	m_textExplanation1.SetXY( EXPLANATION1_X, EXPLANATION1_Y );
	m_textExplanation1.SetZ( 1 );
	m_textExplanation1.SetZoomX( EXPLANATION1_ZOOM_X );
	m_textExplanation1.SetZoomY( EXPLANATION1_ZOOM_Y );
	m_textExplanation1.SetHorizAlign( BitmapText::align_left );
	this->AddChild( &m_textExplanation1 );

	m_textExplanation2.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textExplanation2.SetDiffuse( RageColor(0,0.7f,0,1) );
	m_textExplanation2.SetXY( EXPLANATION2_X, EXPLANATION2_Y );
	m_textExplanation2.SetZ( 1 );
	m_textExplanation2.SetZoomX( EXPLANATION2_ZOOM_X );
	m_textExplanation2.SetZoomY( EXPLANATION2_ZOOM_Y );
	m_textExplanation2.SetHorizAlign( BitmapText::align_left );
	this->AddChild( &m_textExplanation2 );
	
	m_Menu.Load( 	
		THEME->GetPathTo("BGAnimations","select style"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo("Graphics","select style change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style intro") );

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select style music") );

	m_soundChange.PlayRandom();
	TweenOnScreen();
	m_Menu.TweenOnScreenFromBlack( SM_None );
}


ScreenSelectStyle5th::~ScreenSelectStyle5th()
{
	LOG->Trace( "ScreenSelectStyle5th::~ScreenSelectStyle5th()" );

}

void ScreenSelectStyle5th::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectStyle5th::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectStyle5th::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectStyle5th::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevScreen:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenSelectDifficulty" );
		break;
	case SM_TweenExplanation2:
		m_textExplanation2.BeginTweening( 0.7f );
		m_textExplanation2.SetTweenZoomX( EXPLANATION2_ZOOM_X );
		break;
	case SM_UpdateAnimations:
		for( int i=0; i<NUM_STYLE_DANCERS; i++ )
			m_sprDancer[i].StopAnimating();
		switch( m_iSelection )
		{
		case 0:		m_sprDancer[0].StartAnimating();										break;
		case 1:		m_sprDancer[1].StartAnimating();	m_sprDancer[2].StartAnimating();	break;
		case 2:		m_sprDancer[3].StartAnimating();										break;
		case 3:		m_sprDancer[4].StartAnimating();	m_sprDancer[5].StartAnimating();	break;
		case 4:		m_sprDancer[6].StartAnimating();										break;
		}
		break;
	}
}

void ScreenSelectStyle5th::MenuLeft( PlayerNumber pn )
{
	if( m_iSelection == 0 )		// can't go left any further
		return;

	BeforeChange();
	m_iSelection = m_iSelection - 1;
	m_soundChange.Stop();
	m_soundChange.PlayRandom();
	AfterChange();
}


void ScreenSelectStyle5th::MenuRight( PlayerNumber pn )
{
	if( m_iSelection == NUM_DANCE_STYLES-1 )		// can't go right any further
		return;

	BeforeChange();
	m_iSelection = m_iSelection + 1;
	m_soundChange.Stop();
	m_soundChange.PlayRandom();
	AfterChange();
}

void ScreenSelectStyle5th::MenuStart( PlayerNumber pn )
{
	GAMESTATE->m_CurStyle = Style( m_iSelection );
	GAMESTATE->m_MasterPlayerNumber = pn;

	CString sCurStyleName = GAMESTATE->GetCurrentStyleDef()->m_szName;
	sCurStyleName.MakeLower();
	if(	     -1!=sCurStyleName.Find("single") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment single") );
	else if( -1!=sCurStyleName.Find("versus") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment versus") );
	else if( -1!=sCurStyleName.Find("double") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment double") );
	else if( -1!=sCurStyleName.Find("couple") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment couple") );
	else if( -1!=sCurStyleName.Find("solo") )	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select style comment solo") );

	this->ClearMessageQueue();

	TweenOffScreen();
	m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
	m_soundSelect.PlayRandom();
}

void ScreenSelectStyle5th::MenuBack( PlayerNumber pn )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}

void ScreenSelectStyle5th::BeforeChange()
{
	switch( m_iSelection )
	{
	case 0:
		m_sprDancer[0].BeginTweening( TWEEN_TIME );
		m_sprDancer[0].SetTweenDiffuse( COLOR_P1_NOT_SELECTED );
		break;
	case 1:
		m_sprDancer[1].BeginTweening( TWEEN_TIME );
		m_sprDancer[1].SetTweenDiffuse( COLOR_P1_NOT_SELECTED );
		m_sprDancer[2].BeginTweening( TWEEN_TIME );
		m_sprDancer[2].SetTweenDiffuse( COLOR_P2_NOT_SELECTED );
		break;
	case 2:
		m_sprDancer[3].BeginTweening( TWEEN_TIME );
		m_sprDancer[3].SetTweenDiffuse( COLOR_P1_NOT_SELECTED );
		break;
	case 3:
		m_sprDancer[4].BeginTweening( TWEEN_TIME );
		m_sprDancer[4].SetTweenDiffuse( COLOR_P1_NOT_SELECTED );
		m_sprDancer[5].BeginTweening( TWEEN_TIME );
		m_sprDancer[5].SetTweenDiffuse( COLOR_P2_NOT_SELECTED );
		break;
	case 4:
		m_sprDancer[6].BeginTweening( TWEEN_TIME );
		m_sprDancer[6].SetTweenDiffuse( COLOR_P1_NOT_SELECTED );
		break;
	}


}

void ScreenSelectStyle5th::AfterChange()
{
	this->ClearMessageQueue();

	m_textExplanation1.SetText( TEXT_EXPLANATION1[m_iSelection] );
	m_textExplanation1.SetZoomX( 0 );
	m_textExplanation1.BeginTweening( 0.6f );
	m_textExplanation1.SetTweenZoomX( EXPLANATION1_ZOOM_X );

	m_textExplanation2.SetText( TEXT_EXPLANATION2[m_iSelection] );
	m_textExplanation2.StopTweening();
	m_textExplanation2.SetZoomX( 0 );

	switch( m_iSelection )
	{
	case 0:
		m_sprPad[0].BeginTweening( TWEEN_TIME );
		m_sprPad[0].SetTweenDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[0].BeginTweening( TWEEN_TIME );
		m_sprDancer[0].SetTweenDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[0].StartAnimating();
		m_sprStyleIcon.SetState( 0 );
		break;
	case 1:
		m_sprPad[1].BeginTweening( TWEEN_TIME );
		m_sprPad[1].SetTweenDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[1].BeginTweening( TWEEN_TIME );
		m_sprDancer[1].SetTweenDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[1].StartAnimating();
		m_sprDancer[2].BeginTweening( TWEEN_TIME );
		m_sprDancer[2].SetTweenDiffuse( COLOR_P2_SELECTED );
		m_sprDancer[2].StartAnimating();
		m_sprStyleIcon.SetState( 1 );
		break;
	case 2:
		m_sprPad[2].BeginTweening( TWEEN_TIME );
		m_sprPad[2].SetTweenDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[3].BeginTweening( TWEEN_TIME );
		m_sprDancer[3].SetTweenDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[3].StartAnimating();
		m_sprStyleIcon.SetState( 2 );
		break;
	case 3:
		m_sprPad[3].BeginTweening( TWEEN_TIME );
		m_sprPad[3].SetTweenDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[4].BeginTweening( TWEEN_TIME );
		m_sprDancer[4].SetTweenDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[4].StartAnimating();
		m_sprDancer[5].BeginTweening( TWEEN_TIME );
		m_sprDancer[5].SetTweenDiffuse( COLOR_P2_SELECTED );
		m_sprDancer[5].StartAnimating();
		m_sprStyleIcon.SetState( 3 );
		break;
	case 4:
		m_sprPad[4].BeginTweening( TWEEN_TIME );
		m_sprPad[4].SetTweenDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[6].BeginTweening( TWEEN_TIME );
		m_sprDancer[6].SetTweenDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[6].StartAnimating();
		m_sprStyleIcon.SetState( 4 );
		break;
	}

	this->SendScreenMessage( SM_TweenExplanation2, 1 );
	this->SendScreenMessage( SM_UpdateAnimations, TWEEN_TIME );
}

void ScreenSelectStyle5th::TweenOffScreen()
{
	int i;
	for( i=0; i<NUM_STYLE_DANCERS; i++ )
	{
		m_sprDancer[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprDancer[i].SetTweenZoomY( 0 );
	}

	for( i=0; i<NUM_STYLE_PADS; i++ )
	{
		m_sprPad[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprPad[i].SetTweenZoomY( 0 );
	}

	m_sprStyleIcon.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprStyleIcon.SetTweenZoomY( 0 );

	m_textExplanation1.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textExplanation1.SetTweenZoomX( 0 );

	m_textExplanation2.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textExplanation2.SetTweenZoomX( 0 );
}
  

void ScreenSelectStyle5th::TweenOnScreen() 
{
	int i;
	for(i=0; i<NUM_STYLE_DANCERS; i++ )
	{
		float fOrigDancerZoomY = m_sprDancer[i].GetZoomY();
		m_sprDancer[i].SetZoomY( 0 );
		m_sprDancer[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprDancer[i].SetTweenZoomY( fOrigDancerZoomY );
	}

	for( i=0; i<NUM_STYLE_PADS; i++ )
	{
		float fOrigPadZoomY = m_sprPad[i].GetZoomY();
		m_sprPad[i].SetZoomY( 0 );
		m_sprPad[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprPad[i].SetTweenZoomY( fOrigPadZoomY );
	}

	m_sprStyleIcon.SetZoomY( 0 );
	m_sprStyleIcon.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprStyleIcon.SetTweenZoomY( 1 );

	m_textExplanation1.SetZoomX( 0 );
	m_textExplanation1.BeginTweening( 0.6f );
	m_textExplanation1.SetTweenZoomX( EXPLANATION1_ZOOM_X );

	m_textExplanation2.SetZoomX( 0 );
	m_textExplanation2.SetZoomX( 0 );

	m_sprStyleIcon.SetState( 0 );
	m_textExplanation1.SetText( TEXT_EXPLANATION1[m_iSelection] );
	m_textExplanation2.SetText( TEXT_EXPLANATION2[m_iSelection] );

	this->SendScreenMessage( SM_TweenExplanation2, 1 );
	this->SendScreenMessage( SM_UpdateAnimations, TWEEN_TIME );

}
