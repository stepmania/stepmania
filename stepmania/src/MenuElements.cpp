#include "global.h"
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
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "ThemeManager.h"


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
	this->AddChild( &m_Background );
	this->AddChild( &m_quadBrightness );
	this->AddChild( &m_textHelp );

	m_KeepAlive.SetOpened();
	this->AddChild( &m_KeepAlive );

	m_Wipe.SetOpened();
	this->AddChild( &m_Wipe );

	this->AddChild( &m_Invisible );
}

void MenuElements::StealthTimer( int iActive )
{

	m_MenuTimer.StealthTimer( iActive ); // go a bit deeper... get rid of the sound...

	if (iActive == 0) // if we wanna hide the timer... 
	{
		m_MenuTimer.SetXY( TIMER_X, TIMER_Y ); // set it off-screen
	}
	else if (iActive == 1) // we wanna hide the timer
	{
		m_MenuTimer.SetXY( 999.0f, 999.0f ); // otherwise position it off-screen
	}
	// else... take no action :)
}

void MenuElements::Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText, bool bShowStyleIcon, bool bTimerEnabled, int iTimerSeconds )
{
	LOG->Trace( "MenuElements::MenuElements()" );


	m_Background.LoadFromAniDir( sBackgroundPath );

	m_quadBrightness.SetDiffuse( RageColor(0,0,0,0) );
	m_quadBrightness.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );

	m_sprTopEdge.Load( sTopEdgePath );
	m_sprTopEdge.SetXY( TOP_EDGE_X, TOP_EDGE_Y );
	
	m_sprStyleIcon.Load( THEME->GetPathTo("Graphics",ssprintf("menu style icons %s",GAMESTATE->GetCurrentGameDef()->m_szName)) );
	m_sprStyleIcon.StopAnimating();
	m_sprStyleIcon.SetXY( STYLE_ICON_X, STYLE_ICON_Y );
	if( GAMESTATE->m_CurStyle == STYLE_INVALID  ||  !bShowStyleIcon )
		m_sprStyleIcon.SetDiffuse( RageColor(1,1,1,0) );
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

void MenuElements::TweenTopLayerOnScreen(float tm)
{
	if(tm == -1)
		tm = MENU_ELEMENTS_TWEEN_TIME;

	vector<Actor*> apActorsInTopFrame;
	apActorsInTopFrame.push_back( &m_sprTopEdge );
	apActorsInTopFrame.push_back( &m_sprStyleIcon );
	apActorsInTopFrame.push_back( &m_MenuTimer );
	for( unsigned i=0; i<apActorsInTopFrame.size(); i++ )
	{
		float fOriginalX = apActorsInTopFrame[i]->GetX();
		apActorsInTopFrame[i]->SetX( fOriginalX+SCREEN_WIDTH );
		apActorsInTopFrame[i]->BeginTweening( tm, TWEEN_SPRING );
		apActorsInTopFrame[i]->SetTweenX( fOriginalX );
	}

	float fOriginalZoom = m_textHelp.GetZoomY();
	m_textHelp.SetZoomY( 0 );
	m_textHelp.BeginTweening( tm/2 );
	m_textHelp.SetTweenZoomY( fOriginalZoom );
}

void MenuElements::TweenOnScreenFromMenu( ScreenMessage smSendWhenDone, bool bLeaveKeepAliveOn )
{
	TweenTopLayerOnScreen();
	if( !bLeaveKeepAliveOn )
		m_KeepAlive.OpenWipingRight( smSendWhenDone );
	else
		m_KeepAlive.SetClosed();
	m_soundSwoosh.Play();
}

void MenuElements::TweenTopLayerOffScreen(float tm)
{
/*
 This trick is neat, but there's a problem: fOriginalX may not be the settled
 position--we might still be tweening, and if it's a bounce tween, it might be
 left of center, which means fOriginalX+SCREEN_WIDTH won't actually take it
 completely off-screen.  fOriginalX+SCREEN_WIDTH*2 would, but that'd make the
 bounce faster.  SCREEN_WIDTH+SCREEN_WIDTH/2 would, but ignoring fOriginalX
 will make each component tween off at a different rate ...
 -glenn
	vector<Actor*> apActorsInTopFrame;
	apActorsInTopFrame.push_back( &m_sprTopEdge );
	apActorsInTopFrame.push_back( &m_sprStyleIcon );
	apActorsInTopFrame.push_back( &m_MenuTimer );
	for( unsigned i=0; i<apActorsInTopFrame.size(); i++ )
	{
		float fOriginalX = apActorsInTopFrame[i]->GetX();
		apActorsInTopFrame[i]->BeginTweening( MENU_ELEMENTS_TWEEN_TIME, TWEEN_BOUNCE_BEGIN );
		apActorsInTopFrame[i]->SetTweenX( fOriginalX+SCREEN_WIDTH );
	}
	*/

	if(tm == -1)
		tm = MENU_ELEMENTS_TWEEN_TIME;

	m_sprTopEdge.StopTweening();
	m_sprTopEdge.BeginTweening( tm, TWEEN_BOUNCE_BEGIN );
	m_sprTopEdge.SetTweenX( TOP_EDGE_X+SCREEN_WIDTH );

	m_sprStyleIcon.StopTweening();
	m_sprStyleIcon.BeginTweening( tm, TWEEN_BOUNCE_BEGIN );
	m_sprStyleIcon.SetTweenX( STYLE_ICON_X+SCREEN_WIDTH );

	m_MenuTimer.StopTweening();
	m_MenuTimer.BeginTweening( tm, TWEEN_BOUNCE_BEGIN );
	m_MenuTimer.SetTweenX( TIMER_X+SCREEN_WIDTH );

	m_textHelp.StopTweening();
	m_textHelp.BeginTweening( tm/2 );
	m_textHelp.SetTweenZoomY( 0 );
}

void MenuElements::TweenOffScreenToMenu( ScreenMessage smSendWhenDone )
{
	m_MenuTimer.StopTimer();
	TweenTopLayerOffScreen();
	if( !m_KeepAlive.IsClosed() )
		m_KeepAlive.CloseWipingRight( smSendWhenDone );
	else
		SCREENMAN->SendMessageToTopScreen( smSendWhenDone, m_KeepAlive.GetTransitionTime() );
	m_soundSwoosh.Play();
}

void MenuElements::ImmedOnScreenFromMenu( bool bLeaveKeepAliveOn )
{
	TweenTopLayerOnScreen(0);
	Update(0);

	if( !bLeaveKeepAliveOn )
		m_KeepAlive.SetOpened();
	else
		m_KeepAlive.SetClosed();
}

void MenuElements::ImmedOffScreenToMenu()
{
	m_MenuTimer.StopTimer();
	m_KeepAlive.SetClosed();

	/* Remove the top layer immediately. */
	TweenTopLayerOffScreen(0);

	/* We need to do a null update after doing null tweens (tweens with zero time),
	 * or they'll show up in their default positions for a frame (since screens
	 * draw once before they're updated for the first time). */
	Update(0);
}

void MenuElements::TweenBottomLayerOnScreen()
{
	float fOriginalY = m_sprBottomEdge.GetY();
	m_sprBottomEdge.SetY( fOriginalY + 100 );
	m_sprBottomEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
	m_sprBottomEdge.SetTweenY( fOriginalY );

	m_quadBrightness.SetDiffuse( RageColor(0,0,0,1) );
	m_quadBrightness.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
	m_quadBrightness.SetTweenDiffuse( RageColor(0,0,0,0) );
}

void MenuElements::TweenBottomLayerOffScreen()
{
	float fOriginalY = m_sprBottomEdge.GetY();
	m_sprBottomEdge.StopTweening();
	m_sprBottomEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );
	m_sprBottomEdge.SetTweenY( fOriginalY + 100 );

	m_quadBrightness.SetDiffuse( RageColor(0,0,0,0) );
	m_quadBrightness.StopTweening();
	m_quadBrightness.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*3/2.0f );	// sleep
	m_quadBrightness.BeginTweening( MENU_ELEMENTS_TWEEN_TIME/2 );	// fade
	m_quadBrightness.SetTweenDiffuse( RageColor(0,0,0,1) );
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

	m_Background.Draw();
	m_quadBrightness.Draw();

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

