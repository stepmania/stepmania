#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: MenuElements.h

 Desc: Base class for menu Screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MenuElements.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "ScreenGameplay.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"


const float HELP_X		=	CENTER_X;
const float HELP_Y		=	SCREEN_BOTTOM-28;

const float STYLE_ICON_LOCAL_X	=	130;
const float STYLE_ICON_LOCAL_Y	=	6;

const float TIMER_LOCAL_X	=	270;
const float TIMER_LOCAL_Y	=	0;

const float CREDIT_X[NUM_PLAYERS]	=	{160,480};
const float CREDIT_Y[NUM_PLAYERS]	=	{SCREEN_BOTTOM-8, SCREEN_BOTTOM-8};


MenuElements::MenuElements()
{
	m_frameTopBar.AddSubActor( &m_sprTopEdge );
	m_frameTopBar.AddSubActor( &m_sprStyleIcon );
	m_frameTopBar.AddSubActor( &m_MenuTimer );

	m_frameBottomBar.AddSubActor( &m_sprBottomEdge );

	this->AddSubActor( &m_sprBG );
	this->AddSubActor( &m_frameTopBar );
	this->AddSubActor( &m_frameBottomBar );
	this->AddSubActor( &m_textHelp );
	for( int p=0; p<NUM_PLAYERS; p++ )
		this->AddSubActor( &m_textCreditInfo[p] );

	m_KeepAlive.SetZ( -2 );
	m_KeepAlive.SetOpened();
	this->AddSubActor( &m_KeepAlive );

	m_Wipe.SetZ( -2 );
	m_Wipe.SetOpened();
	this->AddSubActor( &m_Wipe );

	this->AddSubActor( &m_Invisible );
}

void MenuElements::Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText, bool bShowStyleIcon, bool bTimerEnabled, int iTimerSeconds )
{
	LOG->WriteLine( "MenuElements::MenuElements()" );


	m_sprBG.Load( sBackgroundPath );
	m_sprBG.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	m_sprBG.TurnShadowOff();

	m_frameTopBar.SetZ( -1 );

	m_sprTopEdge.Load( sTopEdgePath );
	m_sprTopEdge.TurnShadowOff();
	
	m_sprStyleIcon.Load( THEME->GetPathTo(GRAPHIC_MENU_STYLE_ICONS) );
	m_sprStyleIcon.StopAnimating();
	m_sprStyleIcon.SetXY( STYLE_ICON_LOCAL_X, STYLE_ICON_LOCAL_Y );
	m_sprStyleIcon.SetZ( -1 );
	if( GAMESTATE->m_CurStyle == STYLE_NONE  ||  !bShowStyleIcon )
		m_sprStyleIcon.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	else
		m_sprStyleIcon.SetState( GAMESTATE->m_CurStyle );

	m_MenuTimer.SetXY( TIMER_LOCAL_X, TIMER_LOCAL_Y );
	m_MenuTimer.SetZ( -1 );
	if( !bTimerEnabled  ||  !PREFSMAN->m_bMenuTimer )
	{
		m_MenuTimer.SetTimer( 99 );
		m_MenuTimer.Update( 0 );
		m_MenuTimer.StopTimer();
	}
	else
		m_MenuTimer.SetTimer( iTimerSeconds );

	m_frameBottomBar.SetZ( -1 );

	m_sprBottomEdge.Load( THEME->GetPathTo(GRAPHIC_MENU_BOTTOM_EDGE) );
	m_sprBottomEdge.TurnShadowOff();

	m_textHelp.SetXY( HELP_X, HELP_Y );
//	m_textHelp.SetZ( -1 );
	CStringArray asHelpTips;
	split( sHelpText, "\n", asHelpTips );
	m_textHelp.SetTips( asHelpTips );
	//m_textHelp.SetText( sHelpText );
	m_textHelp.SetZoom( 0.5f );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_textCreditInfo[p].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textCreditInfo[p].SetXY( CREDIT_X[p], CREDIT_Y[p] );
		m_textCreditInfo[p].SetZ( -1 );

		if( GAMESTATE->m_CurStyle == STYLE_NONE )
			m_textCreditInfo[p].SetText( "PRESS START" );
		else if( GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
			m_textCreditInfo[p].SetText( "" );
		else	// not enabled
			m_textCreditInfo[p].SetText( "NOT PRESENT" );


		m_textCreditInfo[p].SetZoom( 0.5f );
		m_textCreditInfo[p].SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
		m_textCreditInfo[p].TurnShadowOff();
	}
	
	m_soundSwoosh.Load( THEME->GetPathTo(SOUND_MENU_SWOOSH) );
	m_soundBack.Load( THEME->GetPathTo(SOUND_MENU_BACK) );



	m_frameTopBar.SetXY( CENTER_X, m_sprTopEdge.GetZoomedHeight()/2 );

	m_frameBottomBar.SetXY( CENTER_X, SCREEN_HEIGHT - m_sprBottomEdge.GetZoomedHeight()/2 );
}

void MenuElements::TweenTopLayerOnScreen()
{
	m_frameTopBar.SetXY( CENTER_X+SCREEN_WIDTH, m_sprTopEdge.GetZoomedHeight()/2 );
	m_frameTopBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*2, TWEEN_SPRING );
	m_frameTopBar.SetTweenX( CENTER_X );

	float fOriginalZoom = m_textHelp.GetZoomY();
	m_textHelp.SetZoomY( 0 );
	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textHelp.SetTweenZoomY( fOriginalZoom );
}

void MenuElements::TweenOnScreenFromMenu( ScreenMessage smSendWhenDone )
{
	TweenTopLayerOnScreen();
	m_KeepAlive.OpenWipingRight( smSendWhenDone );
	m_soundSwoosh.Play();
}

void MenuElements::TweenTopLayerOffScreen()
{
	m_frameTopBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*2, TWEEN_BIAS_END );
	m_frameTopBar.SetTweenX( SCREEN_WIDTH*1.5f );

	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textHelp.SetTweenZoomY( 0 );
}

void MenuElements::TweenOffScreenToMenu( ScreenMessage smSendWhenDone )
{
	TweenTopLayerOffScreen();
	m_KeepAlive.CloseWipingRight( smSendWhenDone );
	m_soundSwoosh.Play();
}


void MenuElements::TweenBottomLayerOnScreen()
{
	m_frameBottomBar.SetXY( CENTER_X, SCREEN_HEIGHT + m_sprBottomEdge.GetZoomedHeight()/2 );
	m_frameBottomBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_frameBottomBar.SetTweenY( SCREEN_HEIGHT - m_sprBottomEdge.GetZoomedHeight()/2 );

	m_sprBG.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBG.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

}

void MenuElements::TweenBottomLayerOffScreen()
{
	m_frameBottomBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_frameBottomBar.SetTweenY( SCREEN_HEIGHT + m_sprTopEdge.GetZoomedHeight() );

	m_sprBG.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBG.SetTweenDiffuseColor( D3DXCOLOR(0,0,0,1) );
}

void MenuElements::TweenOnScreenFromBlack( ScreenMessage smSendWhenDone )
{
	TweenTopLayerOnScreen();
	TweenBottomLayerOnScreen();
	//m_Wipe.OpenWipingRight( smSendWhenDone );
	m_soundSwoosh.Play();
}

void MenuElements::TweenOffScreenToBlack( ScreenMessage smSendWhenDone, bool bPlayBackSound )
{
	if( !bPlayBackSound )
	{
		TweenTopLayerOffScreen();
		TweenBottomLayerOffScreen();
		m_Invisible.CloseWipingRight( smSendWhenDone );
	}
	else
	{
		m_soundBack.Play();
		m_Wipe.CloseWipingLeft( smSendWhenDone );
	}
}

void MenuElements::DrawPrimitives()
{
	// do nothing.  Call DrawBottomLayer() and DrawTopLayer() instead.
}

void MenuElements::DrawTopLayer()
{
	m_frameTopBar.Draw();
	m_frameBottomBar.Draw();
	m_textHelp.Draw();
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_textCreditInfo[p].Draw();

	m_KeepAlive.Draw();
	m_Wipe.Draw();
}

void MenuElements::DrawBottomLayer()
{
	m_sprBG.Draw();
}

void MenuElements::StopTimer()
{
	m_MenuTimer.StopTimer();
}



