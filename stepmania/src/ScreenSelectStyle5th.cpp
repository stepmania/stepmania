#include "global.h"
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
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"

const float MENU_ELEMENTS_TWEEN_TIME = 0.5;

#define HELP_TEXT			THEME->GetMetric("ScreenSelectStyle5th","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenSelectStyle5th","TimerSeconds")
#define NEXT_SCREEN			THEME->GetMetric("ScreenSelectStyle5th","NextScreen")

const int NUM_DANCE_STYLES = 5;

const CString DANCE_STYLES[NUM_DANCE_STYLES] = {
	"single",
	"versus",
	"double",
	"couple",
	"solo",
};
/*
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
*/
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


const ScreenMessage SM_UpdateAnimations		=	ScreenMessage(SM_User + 3);
const ScreenMessage SM_TweenExplanation2	=	ScreenMessage(SM_User + 4);

#define PAD_X( p )	THEME->GetMetricF("ScreenSelectStyle5th",ssprintf("PadX%d",p+1))
#define PAD_Y( p )	THEME->GetMetricF("ScreenSelectStyle5th",ssprintf("PadY%d",p+1))
#define PAD_ZOOM( p )	THEME->GetMetricF("ScreenSelectStyle5th",ssprintf("PadZoom%d",p+1))
#define DANCER_X( p )	THEME->GetMetricF("ScreenSelectStyle5th",ssprintf("DancerX%d",p+1))
#define DANCER_Y( p )	THEME->GetMetricF("ScreenSelectStyle5th",ssprintf("DancerY%d",p+1))
#define DANCER_ZOOM( p )	THEME->GetMetricF("ScreenSelectStyle5th",ssprintf("DancerZoom%d",p+1))

ScreenSelectStyle5th::ScreenSelectStyle5th() : Screen("ScreenSelectStyle5th")
{
	LOG->Trace( "ScreenSelectStyle5th::ScreenSelectStyle5th()" );

	m_iSelection = 0;	// single

	int i;
	for( i=0; i<NUM_STYLE_PADS; i++ )
	{
		m_sprPad[i].Load( THEME->GetPathToG(ssprintf("select style pad game %d style %d",GAMESTATE->m_CurGame,i)) );
//		m_sprPad[i].SetXY( PAD_X[i], PAD_Y[i] );
//		m_sprPad[i].SetZoom( 1 );
		m_sprPad[i].SetXY( PAD_X(i), PAD_Y(i));
		m_sprPad[i].SetZoom( PAD_ZOOM(i));
		this->AddChild( &m_sprPad[i] );
	}


	for( i=0; i<NUM_STYLE_DANCERS; i++ )
	{
		m_sprDancer[i].Load( THEME->GetPathToG(ssprintf("select style player game %d style %d",GAMESTATE->m_CurGame,i)) );
		m_sprDancer[i].SetVertAlign( Actor::align_top );
		m_sprDancer[i].SetXY( DANCER_X(i), DANCER_Y(i));
		m_sprDancer[i].SetZoom( DANCER_ZOOM(i));
//		m_sprDancer[i].SetXY( DANCER_X[i], DANCER_Y[i] );
//		m_sprDancer[i].SetZoom( 2 );
		m_sprDancer[i].StopAnimating();
		this->AddChild( &m_sprDancer[i] );
	}


	m_sprStyleIcon.Load( THEME->GetPathToG(ssprintf("select style icons game %d",GAMESTATE->m_CurGame)) );
	m_sprStyleIcon.EnableShadow( false );
	m_sprStyleIcon.StopAnimating();
	m_sprStyleIcon.SetXY( ICON_X, ICON_Y );
	this->AddChild( &m_sprStyleIcon );


	m_textExplanation1.LoadFromFont( THEME->GetPathToF("header1") );
	m_textExplanation1.SetDiffuse( RageColor(0,0.7f,0,1) );
	m_textExplanation1.SetXY( EXPLANATION1_X, EXPLANATION1_Y );
	m_textExplanation1.SetZ( 1 );
	m_textExplanation1.SetZoomX( EXPLANATION1_ZOOM_X );
	m_textExplanation1.SetZoomY( EXPLANATION1_ZOOM_Y );
	m_textExplanation1.SetHorizAlign( BitmapText::align_left );
	this->AddChild( &m_textExplanation1 );

	m_textExplanation2.LoadFromFont( THEME->GetPathToF("header1") );
	m_textExplanation2.SetDiffuse( RageColor(0,0.7f,0,1) );
	m_textExplanation2.SetXY( EXPLANATION2_X, EXPLANATION2_Y );
	m_textExplanation2.SetZ( 1 );
	m_textExplanation2.SetZoomX( EXPLANATION2_ZOOM_X );
	m_textExplanation2.SetZoomY( EXPLANATION2_ZOOM_Y );
	m_textExplanation2.SetHorizAlign( BitmapText::align_left );
	this->AddChild( &m_textExplanation2 );
	
	m_Menu.Load( "ScreenSelectStyle5th" );
	this->AddChild( &m_Menu );

	m_soundChange.Load( THEME->GetPathToG("ScreenSelectStyle5th change") );
	m_soundSelect.Load( THEME->GetPathToS("Common start") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenSelectStyle5th music") );

	m_soundChange.PlayRandom();
	TweenOnScreen();
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

	if( m_Menu.IsTransitioning() )
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
		SOUNDMAN->StopMusic();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenSelectDifficulty" );
		break;
	case SM_TweenExplanation2:
		m_textExplanation2.BeginTweening( 0.7f );
		m_textExplanation2.SetZoomX( EXPLANATION2_ZOOM_X );
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
	if(	     -1!=sCurStyleName.Find("single") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("ScreenSelectStyle5th comment single") );
	else if( -1!=sCurStyleName.Find("versus") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("ScreenSelectStyle5th comment versus") );
	else if( -1!=sCurStyleName.Find("double") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment double") );
	else if( -1!=sCurStyleName.Find("couple") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment couple") );
	else if( -1!=sCurStyleName.Find("solo") )	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select style comment solo") );

	this->ClearMessageQueue();

	TweenOffScreen();
	m_Menu.StartTransitioning( SM_GoToNextScreen );
	m_soundSelect.PlayRandom();
}

void ScreenSelectStyle5th::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.Back( SM_GoToPrevScreen );
}

void ScreenSelectStyle5th::BeforeChange()
{
	switch( m_iSelection )
	{
	case 0:
		m_sprDancer[0].BeginTweening( TWEEN_TIME );
		m_sprDancer[0].SetDiffuse( COLOR_P1_NOT_SELECTED );
		break;
	case 1:
		m_sprDancer[1].BeginTweening( TWEEN_TIME );
		m_sprDancer[1].SetDiffuse( COLOR_P1_NOT_SELECTED );
		m_sprDancer[2].BeginTweening( TWEEN_TIME );
		m_sprDancer[2].SetDiffuse( COLOR_P2_NOT_SELECTED );
		break;
	case 2:
		m_sprDancer[3].BeginTweening( TWEEN_TIME );
		m_sprDancer[3].SetDiffuse( COLOR_P1_NOT_SELECTED );
		break;
	case 3:
		m_sprDancer[4].BeginTweening( TWEEN_TIME );
		m_sprDancer[4].SetDiffuse( COLOR_P1_NOT_SELECTED );
		m_sprDancer[5].BeginTweening( TWEEN_TIME );
		m_sprDancer[5].SetDiffuse( COLOR_P2_NOT_SELECTED );
		break;
	case 4:
		m_sprDancer[6].BeginTweening( TWEEN_TIME );
		m_sprDancer[6].SetDiffuse( COLOR_P1_NOT_SELECTED );
		break;
	}


}

void ScreenSelectStyle5th::AfterChange()
{
	this->ClearMessageQueue();

	m_textExplanation1.SetText( TEXT_EXPLANATION1[m_iSelection] );
	m_textExplanation1.SetZoomX( 0 );
	m_textExplanation1.BeginTweening( 0.6f );
	m_textExplanation1.SetZoomX( EXPLANATION1_ZOOM_X );

	m_textExplanation2.SetText( TEXT_EXPLANATION2[m_iSelection] );
	m_textExplanation2.StopTweening();
	m_textExplanation2.SetZoomX( 0 );

	switch( m_iSelection )
	{
	case 0:
		m_sprPad[0].BeginTweening( TWEEN_TIME );
		m_sprPad[0].SetDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[0].BeginTweening( TWEEN_TIME );
		m_sprDancer[0].SetDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[0].StartAnimating();
		m_sprStyleIcon.SetState( 0 );
		break;
	case 1:
		m_sprPad[1].BeginTweening( TWEEN_TIME );
		m_sprPad[1].SetDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[1].BeginTweening( TWEEN_TIME );
		m_sprDancer[1].SetDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[1].StartAnimating();
		m_sprDancer[2].BeginTweening( TWEEN_TIME );
		m_sprDancer[2].SetDiffuse( COLOR_P2_SELECTED );
		m_sprDancer[2].StartAnimating();
		m_sprStyleIcon.SetState( 1 );
		break;
	case 2:
		m_sprPad[2].BeginTweening( TWEEN_TIME );
		m_sprPad[2].SetDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[3].BeginTweening( TWEEN_TIME );
		m_sprDancer[3].SetDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[3].StartAnimating();
		m_sprStyleIcon.SetState( 2 );
		break;
	case 3:
		m_sprPad[3].BeginTweening( TWEEN_TIME );
		m_sprPad[3].SetDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[4].BeginTweening( TWEEN_TIME );
		m_sprDancer[4].SetDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[4].StartAnimating();
		m_sprDancer[5].BeginTweening( TWEEN_TIME );
		m_sprDancer[5].SetDiffuse( COLOR_P2_SELECTED );
		m_sprDancer[5].StartAnimating();
		m_sprStyleIcon.SetState( 3 );
		break;
	case 4:
		m_sprPad[4].BeginTweening( TWEEN_TIME );
		m_sprPad[4].SetDiffuse( RageColor(1,1,1,1) );
		m_sprDancer[6].BeginTweening( TWEEN_TIME );
		m_sprDancer[6].SetDiffuse( COLOR_P1_SELECTED );
		m_sprDancer[6].StartAnimating();
		m_sprStyleIcon.SetState( 4 );
		break;
	}

	this->PostScreenMessage( SM_TweenExplanation2, 1 );
	this->PostScreenMessage( SM_UpdateAnimations, TWEEN_TIME );
}

void ScreenSelectStyle5th::TweenOffScreen()
{
	int i;
	for( i=0; i<NUM_STYLE_DANCERS; i++ )
	{
		m_sprDancer[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprDancer[i].SetZoomY( 0 );
	}

	for( i=0; i<NUM_STYLE_PADS; i++ )
	{
		m_sprPad[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprPad[i].SetZoomY( 0 );
	}

	m_sprStyleIcon.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprStyleIcon.SetZoomY( 0 );

	m_textExplanation1.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textExplanation1.SetZoomX( 0 );

	m_textExplanation2.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textExplanation2.SetZoomX( 0 );
}
  

void ScreenSelectStyle5th::TweenOnScreen() 
{
	int i;
	for(i=0; i<NUM_STYLE_DANCERS; i++ )
	{
		float fOrigDancerZoomY = m_sprDancer[i].GetZoomY();
		m_sprDancer[i].SetZoomY( 0 );
		m_sprDancer[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprDancer[i].SetZoomY( fOrigDancerZoomY );
	}

	for( i=0; i<NUM_STYLE_PADS; i++ )
	{
		float fOrigPadZoomY = m_sprPad[i].GetZoomY();
		m_sprPad[i].SetZoomY( 0 );
		m_sprPad[i].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprPad[i].SetZoomY( fOrigPadZoomY );
	}

	m_sprStyleIcon.SetZoomY( 0 );
	m_sprStyleIcon.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprStyleIcon.SetZoomY( 1 );

	m_textExplanation1.SetZoomX( 0 );
	m_textExplanation1.BeginTweening( 0.6f );
	m_textExplanation1.SetZoomX( EXPLANATION1_ZOOM_X );

	m_textExplanation2.SetZoomX( 0 );
	m_textExplanation2.SetZoomX( 0 );

	m_sprStyleIcon.SetState( 0 );
	m_textExplanation1.SetText( TEXT_EXPLANATION1[m_iSelection] );
	m_textExplanation2.SetText( TEXT_EXPLANATION2[m_iSelection] );

	this->PostScreenMessage( SM_TweenExplanation2, 1 );
	this->PostScreenMessage( SM_UpdateAnimations, TWEEN_TIME );

}
