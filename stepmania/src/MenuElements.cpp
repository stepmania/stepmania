#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: MenuElements.h

 Desc: Base class for menu Windows.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "MenuElements.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "WindowManager.h"
#include "WindowDancing.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"
#include "RageHelper.h"


const float HELP_X		=	CENTER_X;
const float HELP_Y		=	SCREEN_HEIGHT-25;

MenuElements::MenuElements()
{
	this->AddActor( &m_sprBG );
	this->AddActor( &m_sprTopEdge );
	this->AddActor( &m_sprBottomEdge );
	this->AddActor( &m_textHelp );

}

void MenuElements::Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText )
{
	HELPER.Log( "MenuElements::MenuElements()" );


	m_sprBG.Load( sBackgroundPath );
	m_sprBG.StretchTo( CRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) );
	m_sprBG.TurnShadowOff();

	m_sprTopEdge.Load( sTopEdgePath );
	m_sprTopEdge.TurnShadowOff();
	m_sprTopEdge.SetZ( -1 );
	m_sprTopEdge.SetY( SCREEN_TOP + m_sprTopEdge.GetZoomedHeight()/2.0f );

	m_sprBottomEdge.Load( THEME->GetPathTo(GRAPHIC_MENU_BOTTOM_EDGE) );
	m_sprBottomEdge.TurnShadowOff();
	m_sprBottomEdge.SetZ( -1 );

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

	m_sprTopEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*2, TWEEN_SPRING );
	m_sprTopEdge.SetTweenX( CENTER_X );

	float fOriginalZoomY = m_textHelp.GetZoomY();
	m_textHelp.SetZoomY( 0 );
	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textHelp.SetTweenZoomY( fOriginalZoomY );

	m_soundSwoosh.PlayRandom();
}

void MenuElements::TweenTopEdgeOffScreen()
{
	SetTopEdgeOnScreen();

	m_sprTopEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME*2, TWEEN_BIAS_END );
	m_sprTopEdge.SetTweenX( SCREEN_WIDTH*1.5f );


	m_textHelp.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_textHelp.SetTweenZoomY( 0 );

	m_soundSwoosh.PlayRandom();
}

void MenuElements::TweenBackgroundOnScreen()
{
	SetBackgroundOffScreen();

	m_sprBottomEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBottomEdge.SetTweenY( SCREEN_HEIGHT - m_sprBottomEdge.GetZoomedHeight()/2 );

	m_sprBG.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBG.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	m_soundSwoosh.PlayRandom();
}

void MenuElements::TweenBackgroundOffScreen()
{
	SetBackgroundOnScreen();

	m_sprBottomEdge.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBottomEdge.SetTweenY( SCREEN_HEIGHT + m_sprTopEdge.GetZoomedHeight() );

	m_sprBG.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_sprBG.BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
	m_sprBG.SetTweenDiffuseColor( D3DXCOLOR(0,0,0,1) );

	m_soundBack.PlayRandom();
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
	m_sprBottomEdge.SetXY( CENTER_X, SCREEN_HEIGHT - m_sprBottomEdge.GetZoomedHeight()/2 );
}

void MenuElements::SetBackgroundOffScreen()
{
	m_sprBottomEdge.SetXY( CENTER_X, SCREEN_HEIGHT + m_sprBottomEdge.GetZoomedHeight()/2 );
}

void MenuElements::SetTopEdgeOnScreen()
{
	m_sprTopEdge.SetXY( CENTER_X, m_sprTopEdge.GetZoomedHeight()/2 );
}

void MenuElements::SetTopEdgeOffScreen()
{
	m_sprTopEdge.SetXY( CENTER_X+SCREEN_WIDTH, m_sprTopEdge.GetZoomedHeight()/2 );
}


void MenuElements::RenderPrimitives()
{
	// do nothing.  Call DrawBottomLayer() and DrawTopLayer() instead.
}

void MenuElements::DrawTopLayer()
{
	m_sprTopEdge.Draw();
	m_sprBottomEdge.Draw();
	m_textHelp.Draw();
}

void MenuElements::DrawBottomLayer()
{
	m_sprBG.Draw();
}



