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
#include "ThemeManager.h"
#include "RageLog.h"


const float HELP_X		=	CENTER_X;
const float HELP_Y		=	SCREEN_HEIGHT-28;

const float TIMER_LOCAL_X	=	270;
const float TIMER_LOCAL_Y	=	0;

MenuElements::MenuElements()
{
	m_frameTopBar.AddActor( &m_sprTopEdge );
	m_frameTopBar.AddActor( &m_MenuTimer );

	m_frameBottomBar.AddActor( &m_sprBottomEdge );

	this->AddActor( &m_sprBG );
	this->AddActor( &m_frameTopBar );
	this->AddActor( &m_frameBottomBar );
	this->AddActor( &m_textHelp );
}

void MenuElements::Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText )
{
	LOG->WriteLine( "MenuElements::MenuElements()" );


	m_sprBG.Load( sBackgroundPath );
	m_sprBG.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	m_sprBG.TurnShadowOff();

	m_frameTopBar.SetZ( -1 );

	m_sprTopEdge.Load( sTopEdgePath );
	m_sprTopEdge.TurnShadowOff();

	m_MenuTimer.SetXY( TIMER_LOCAL_X, TIMER_LOCAL_Y );
	m_MenuTimer.SetZ( -1 );

	m_frameBottomBar.SetZ( -1 );

	m_sprBottomEdge.Load( THEME->GetPathTo(GRAPHIC_MENU_BOTTOM_EDGE) );
	m_sprBottomEdge.TurnShadowOff();

	m_textHelp.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textHelp.SetXY( HELP_X, HELP_Y );
	m_textHelp.SetZ( -1 );
	m_textHelp.SetText( sHelpText );
	m_textHelp.SetZoom( 0.5f );
	m_textHelp.SetEffectBlinking();
	m_textHelp.SetShadowLength( 2 );

	
	m_soundSwoosh.Load( THEME->GetPathTo(SOUND_MENU_SWOOSH) );
	m_soundBack.Load( THEME->GetPathTo(SOUND_MENU_BACK) );


	SetTopEdgeOnScreen();
	SetBackgroundOnScreen();
}


void MenuElements::TweenTopEdgeOnScreen()
{
	SetTopEdgeOffScreen();

	m_frameTopBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*2, TWEEN_SPRING );
	m_frameTopBar.SetTweenX( CENTER_X );

	float fOriginalZoomY = m_textHelp.GetZoomY();
	m_textHelp.SetZoomY( 0 );
	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textHelp.SetTweenZoomY( fOriginalZoomY );

	m_soundSwoosh.Play();
}

void MenuElements::TweenTopEdgeOffScreen()
{
	m_frameTopBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*2, TWEEN_BIAS_END );
	m_frameTopBar.SetTweenX( SCREEN_WIDTH*1.5f );


	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textHelp.SetTweenZoomY( 0 );

	m_soundSwoosh.Play();
}

void MenuElements::TweenBackgroundOnScreen()
{
	SetBackgroundOffScreen();

	m_frameBottomBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_frameBottomBar.SetTweenY( SCREEN_HEIGHT - m_sprBottomEdge.GetZoomedHeight()/2 );

	m_sprBG.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBG.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_soundSwoosh.Play();
}

void MenuElements::TweenBackgroundOffScreen()
{
	m_frameBottomBar.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_frameBottomBar.SetTweenY( SCREEN_HEIGHT + m_sprTopEdge.GetZoomedHeight() );

	m_sprBG.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBG.SetTweenDiffuseColor( D3DXCOLOR(0,0,0,1) );

	m_soundBack.Play();
}

void MenuElements::TweenAllOnScreen()
{
	TweenTopEdgeOnScreen();
	TweenBackgroundOnScreen();
}

void MenuElements::TweenAllOffScreen()
{
	TweenTopEdgeOffScreen();
	TweenBackgroundOffScreen();
}

void MenuElements::SetBackgroundOnScreen()
{
	m_frameBottomBar.SetXY( CENTER_X, SCREEN_HEIGHT - m_sprBottomEdge.GetZoomedHeight()/2 );
}

void MenuElements::SetBackgroundOffScreen()
{
	m_frameBottomBar.SetXY( CENTER_X, SCREEN_HEIGHT + m_sprBottomEdge.GetZoomedHeight()/2 );
}

void MenuElements::SetTopEdgeOnScreen()
{
	m_frameTopBar.SetXY( CENTER_X, m_sprTopEdge.GetZoomedHeight()/2 );
}

void MenuElements::SetTopEdgeOffScreen()
{
	m_frameTopBar.SetXY( CENTER_X+SCREEN_WIDTH, m_sprTopEdge.GetZoomedHeight()/2 );
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
}

void MenuElements::DrawBottomLayer()
{
	m_sprBG.Draw();
}



