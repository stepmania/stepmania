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


TransitionOniFade::TransitionOniFade()
{
	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );	// white

	m_quadBackground.StretchTo( CRect(SCREEN_LEFT, SCREEN_TOP, SCREEN_RIGHT, SCREEN_BOTTOM) );

	m_quadStrip.StretchTo( CRect(SCREEN_LEFT, int(CENTER_Y-30), SCREEN_RIGHT, int(CENTER_Y+30)) );
	
	m_textSongInfo.Load( THEME->GetPathTo(FONT_NORMAL) );
	m_textSongInfo.TurnShadowOff();
	m_textSongInfo.SetZoom( 0.5f );
	m_textSongInfo.SetXY( CENTER_X, CENTER_Y );
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

	m_quadBackground.SetDiffuseColor( D3DXCOLOR(1,1,1,SCALE(GetPercentageClosed(),0,1,-1,1)) );
	m_quadBackground.Draw();

	if( m_TransitionState == closed  ||  m_TransitionState == opening_right )
	{
		m_quadStrip.SetDiffuseColor( D3DXCOLOR(0,0,0,SCALE(GetPercentageClosed(),0,1,0,2)) );
		m_quadStrip.Draw();

		m_textSongInfo.SetDiffuseColor( D3DXCOLOR(1,1,1,SCALE(GetPercentageClosed(),0,1,0,2)) );
		m_textSongInfo.Draw();
	}
}


void TransitionOniFade::OpenWipingRight( ScreenMessage send_when_done )
{
	SetTransitionTime( 4 );
	Transition::OpenWipingRight( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::OpenWipingLeft(  ScreenMessage send_when_done )
{
	SetTransitionTime( 4 );
	Transition::OpenWipingLeft( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::CloseWipingRight(ScreenMessage send_when_done )
{
	SetTransitionTime( 2 );
	Transition::CloseWipingRight( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::CloseWipingLeft( ScreenMessage send_when_done )
{
	SetTransitionTime( 2 );
	Transition::CloseWipingLeft( send_when_done );
	UpdateSongText();
}

void TransitionOniFade::UpdateSongText()
{
	Song* pSong = GAMESTATE->m_pCurSong;
	ASSERT( pSong );
	m_textSongInfo.SetText( pSong->GetFullTitle() + "\n" + pSong->m_sArtist + "\n");
}
