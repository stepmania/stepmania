#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: TransitionOniFade

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "TransitionOniFade.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"


#define BANNER_WIDTH		THEME->GetMetricF("TransitionOniFade","BannerWidth")
#define BANNER_HEIGHT		THEME->GetMetricF("TransitionOniFade","BannerHeight")


TransitionOniFade::TransitionOniFade()
{
	SetDiffuse( RageColor(1,1,1,1) );	// white

	m_quadBackground.StretchTo( RectI(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );

	m_quadStrip.StretchTo( RectI(SCREEN_LEFT, int(CENTER_Y-30), SCREEN_RIGHT, int(CENTER_Y+30)) );
	
	m_textSongInfo.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongInfo.TurnShadowOff();
	m_textSongInfo.SetZoom( 0.5f );
	m_textSongInfo.SetXY( CENTER_X, CENTER_Y );

	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	m_Banner.SetXY( CENTER_X, CENTER_Y );
}

TransitionOniFade::~TransitionOniFade()
{

}

void TransitionOniFade::DrawPrimitives()
{
	if( m_TransitionState == opened ) 
		return;

	if( m_TransitionState == closed ) 
	{
		UpdateSongText();
	}

	m_quadBackground.SetDiffuse( RageColor(1,1,1,SCALE(GetPercentageClosed(),0,1,-1,1)) );
	m_quadBackground.Draw();

	if( m_TransitionState == closed  ||  m_TransitionState == opening_right )
	{
		m_quadStrip.SetDiffuse( RageColor(0,0,0,SCALE(GetPercentageClosed(),0,1,0,2)) );
//		m_quadStrip.Draw();

		m_textSongInfo.SetDiffuse( RageColor(1,1,1,SCALE(GetPercentageClosed(),0,1,0,2)) );
//		m_textSongInfo.Draw();

		m_Banner.SetDiffuse( RageColor(1,1,1,SCALE(GetPercentageClosed(),0,1,0,2)) );
		m_Banner.Draw();
	}
}


void TransitionOniFade::OpenWipingRight( ScreenMessage send_when_done )
{
	SetTransitionTime( 2.5f );
	Transition::OpenWipingRight( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::OpenWipingLeft(  ScreenMessage send_when_done )
{
	SetTransitionTime( 2.5f );
	Transition::OpenWipingLeft( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::CloseWipingRight(ScreenMessage send_when_done )
{
	SetTransitionTime( 1 );
	Transition::CloseWipingRight( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::CloseWipingLeft( ScreenMessage send_when_done )
{
	SetTransitionTime( 1 );
	Transition::CloseWipingLeft( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::UpdateSongText()
{
	Song* pSong = GAMESTATE->m_pCurSong;
	ASSERT( pSong );
	m_textSongInfo.SetText( pSong->GetFullDisplayTitle() + "\n" + pSong->m_sArtist + "\n");
	m_Banner.LoadFromSong( pSong );
}
