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
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"


#define TOP_EDGE_X			THEME->GetMetricF("MenuElements","TopEdgeX")
#define TOP_EDGE_Y			THEME->GetMetricF("MenuElements","TopEdgeY")
#define BOTTOM_EDGE_X		THEME->GetMetricF("MenuElements","BottomEdgeX")
#define BOTTOM_EDGE_Y		THEME->GetMetricF("MenuElements","BottomEdgeY")
#define STYLE_ICON_X		THEME->GetMetricF("MenuElements","StyleIconX")
#define STYLE_ICON_Y		THEME->GetMetricF("MenuElements","StyleIconY")
#define TIMER_X				THEME->GetMetricF("MenuElements","TimerX")
#define TIMER_Y				THEME->GetMetricF("MenuElements","TimerY")
#define HELP_X				THEME->GetMetricF("MenuElements","HelpX")
#define HELP_Y				THEME->GetMetricF("MenuElements","HelpY")


MenuElements::MenuElements()
{
	this->AddChild( &m_sprTopEdge );
	this->AddChild( &m_sprStyleIcon );
	this->AddChild( &m_MenuTimer );
	this->AddChild( &m_sprBottomEdge );
	this->AddChild( &m_sprBG );
	this->AddChild( &m_textHelp );

	m_KeepAlive.SetOpened();
	this->AddChild( &m_KeepAlive );

	m_Wipe.SetOpened();
	this->AddChild( &m_Wipe );

	this->AddChild( &m_Invisible );
}

void MenuElements::Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText, bool bShowStyleIcon, bool bTimerEnabled, int iTimerSeconds )
{
	LOG->Trace( "MenuElements::MenuElements()" );


	m_sprBG.Load( sBackgroundPath );
	m_sprBG.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	m_sprBG.TurnShadowOff();

	m_sprTopEdge.Load( sTopEdgePath );
	m_sprTopEdge.SetXY( TOP_EDGE_X, TOP_EDGE_Y );
	
	m_sprStyleIcon.Load( THEME->GetPathTo("Graphics",ssprintf("menu style icons game %d",GAMESTATE->m_CurGame)) );
	m_sprStyleIcon.StopAnimating();
	m_sprStyleIcon.SetXY( STYLE_ICON_X, STYLE_ICON_Y );
	if( GAMESTATE->m_CurStyle == STYLE_NONE  ||  !bShowStyleIcon )
		m_sprStyleIcon.SetDiffuse( D3DXCOLOR(1,1,1,0) );
	else
	{
		int iRowNum = GetStyleIndexRelativeToGame( GAMESTATE->m_CurGame, GAMESTATE->m_CurStyle );
		m_sprStyleIcon.SetState( iRowNum*2+GAMESTATE->m_MasterPlayerNumber );
	}

	m_MenuTimer.SetXY( TIMER_X, TIMER_Y );
	if( !bTimerEnabled  ||  !PREFSMAN->m_bMenuTimer )
	{
		m_MenuTimer.SetTimer( 99 );
		m_MenuTimer.Update( 0 );
		m_MenuTimer.StopTimer();
	}
	else
		m_MenuTimer.SetTimer( iTimerSeconds );

	m_sprBottomEdge.Load( THEME->GetPathTo("Graphics","menu bottom edge") );
	m_sprBottomEdge.SetXY( BOTTOM_EDGE_X, BOTTOM_EDGE_Y );

	m_textHelp.SetXY( HELP_X, HELP_Y );
	CStringArray asHelpTips;
	split( sHelpText, "\n", asHelpTips );
	m_textHelp.SetTips( asHelpTips );
	m_textHelp.SetZoom( 0.5f );


	m_soundSwoosh.Load( THEME->GetPathTo("Sounds","menu swoosh") );
	m_soundBack.Load( THEME->GetPathTo("Sounds","menu back") );
}

void MenuElements::TweenTopLayerOnScreen()
{
	CArray<Actor*,Actor*> apActorsInTopFrame;
	apActorsInTopFrame.Add( &m_sprTopEdge );
	apActorsInTopFrame.Add( &m_sprStyleIcon );
	apActorsInTopFrame.Add( &m_MenuTimer );
	for( int i=0; i<apActorsInTopFrame.GetSize(); i++ )
	{
		float fOriginalX = apActorsInTopFrame[i]->GetX();
		apActorsInTopFrame[i]->SetX( fOriginalX+SCREEN_WIDTH );
		apActorsInTopFrame[i]->BeginTweening( MENU_ELEMENTS_TWEEN_TIME, TWEEN_SPRING );
		apActorsInTopFrame[i]->SetTweenX( fOriginalX );
	}

	float fOriginalZoom = m_textHelp.GetZoomY();
	m_textHelp.SetZoomY( 0 );
	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
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
/*
 This trick is neat, but there's a problem: fOriginalX may not be the settled
 position--we might still be tweening, and if it's a bounce tween, it might be
 left of center, which means fOriginalX+SCREEN_WIDTH won't actually take it
 completely off-screen.  fOriginalX+SCREEN_WIDTH*2 would, but that'd make the
 bounce faster.  SCREEN_WIDTH+SCREEN_WIDTH/2 would, but ignoring fOriginalX
 will make each component tween off at a different rate ...
 -glenn
	CArray<Actor*,Actor*> apActorsInTopFrame;
	apActorsInTopFrame.Add( &m_sprTopEdge );
	apActorsInTopFrame.Add( &m_sprStyleIcon );
	apActorsInTopFrame.Add( &m_MenuTimer );
	for( int i=0; i<apActorsInTopFrame.GetSize(); i++ )
	{
		float fOriginalX = apActorsInTopFrame[i]->GetX();
		apActorsInTopFrame[i]->BeginTweening( MENU_ELEMENTS_TWEEN_TIME, TWEEN_BOUNCE_BEGIN );
		apActorsInTopFrame[i]->SetTweenX( fOriginalX+SCREEN_WIDTH );
	}
	*/

	m_sprTopEdge.StopTweening();
	m_sprTopEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME, TWEEN_BOUNCE_BEGIN );
	m_sprTopEdge.SetTweenX( TOP_EDGE_X+SCREEN_WIDTH );

	m_sprStyleIcon.StopTweening();
	m_sprStyleIcon.BeginTweening( MENU_ELEMENTS_TWEEN_TIME, TWEEN_BOUNCE_BEGIN );
	m_sprStyleIcon.SetTweenX( STYLE_ICON_X+SCREEN_WIDTH );

	m_MenuTimer.StopTweening();
	m_MenuTimer.BeginTweening( MENU_ELEMENTS_TWEEN_TIME, TWEEN_BOUNCE_BEGIN );
	m_MenuTimer.SetTweenX( TIMER_X+SCREEN_WIDTH );

	m_textHelp.StopTweening();
	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
	m_textHelp.SetTweenZoomY( 0 );
}

void MenuElements::TweenOffScreenToMenu( ScreenMessage smSendWhenDone )
{
	m_MenuTimer.StopTimer();
	TweenTopLayerOffScreen();
	m_KeepAlive.CloseWipingRight( smSendWhenDone );
	m_soundSwoosh.Play();
}

void MenuElements::ImmedOffScreenToMenu()
{
	m_MenuTimer.StopTimer();
	m_KeepAlive.SetClosed();

	/* XXX: This is an ugly paste from TweenTopLayerOffScreen.
	 * We need some way to use the same code for tweening and
	 * immediates ... */
	m_sprTopEdge.StopTweening();
	m_sprTopEdge.SetX( TOP_EDGE_X+SCREEN_WIDTH );

	m_sprStyleIcon.StopTweening();
	m_sprStyleIcon.SetX( STYLE_ICON_X+SCREEN_WIDTH );

	m_MenuTimer.StopTweening();
	m_MenuTimer.SetX( TIMER_X+SCREEN_WIDTH );

	m_textHelp.StopTweening();
	m_textHelp.SetZoomY( 0 );
}

void MenuElements::TweenBottomLayerOnScreen()
{
	float fOriginalY = m_sprBottomEdge.GetY();
	m_sprBottomEdge.SetY( fOriginalY + 100 );
	m_sprBottomEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
	m_sprBottomEdge.SetTweenY( fOriginalY );

	m_sprBG.SetDiffuse( D3DXCOLOR(0,0,0,1) );
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
	m_sprBG.SetTweenDiffuse( D3DXCOLOR(1,1,1,1) );
}

void MenuElements::TweenBottomLayerOffScreen()
{
	float fOriginalY = m_sprBottomEdge.GetY();
	m_sprBottomEdge.StopTweening();
	m_sprBottomEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
	m_sprBottomEdge.SetTweenY( fOriginalY + 100 );

	m_sprBG.SetDiffuse( D3DXCOLOR(1,1,1,1) );
	m_sprBG.StopTweening();
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*3/2.0f );	// sleep
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );	// fade
	m_sprBG.SetTweenDiffuse( D3DXCOLOR(0,0,0,1) );
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
	m_MenuTimer.StopTimer();

	if( !bPlayBackSound )
	{
		TweenTopLayerOffScreen();
		TweenBottomLayerOffScreen();
		m_Invisible.SetTransitionTime( MENU_ELEMENTS_TWEEN_TIME*2 );
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
	BeginDraw();

	m_sprTopEdge.Draw();
	m_sprStyleIcon.Draw();
	m_MenuTimer.Draw();
	m_sprBottomEdge.Draw();
	m_textHelp.Draw();
	m_KeepAlive.Draw();
	m_Wipe.Draw();

	EndDraw();
}

void MenuElements::DrawBottomLayer()
{
	BeginDraw();

	m_sprBG.Draw();

	EndDraw();
}

void MenuElements::SetTimer( int iTimerSeconds )
{
	m_MenuTimer.SetTimer( iTimerSeconds );
}

void MenuElements::StartTimer()
{
	m_MenuTimer.StartTimer();
}

void MenuElements::StopTimer()
{
	m_MenuTimer.StopTimer();
}

void MenuElements::StallTimer()
{
	m_MenuTimer.StallTimer();
}

