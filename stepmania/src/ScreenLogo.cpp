#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenLogo

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenLogo.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "GameDef.h"
#include "RageLog.h"
#include "SongManager.h"

#define LOGO_X							THEME->GetMetricF("ScreenLogo","LogoX")
#define LOGO_Y							THEME->GetMetricF("ScreenLogo","LogoY")
#define VERSION_X						THEME->GetMetricF("ScreenLogo","VersionX")
#define VERSION_Y						THEME->GetMetricF("ScreenLogo","VersionY")
#define SONGS_X							THEME->GetMetricF("ScreenLogo","SongsX")
#define SONGS_Y							THEME->GetMetricF("ScreenLogo","SongsY")


ScreenLogo::ScreenLogo() : ScreenAttract("ScreenLogo","logo")
{
	m_sprLogo.Load( THEME->GetPathTo("Graphics",ssprintf("logo %s",GAMESTATE->GetCurrentGameDef()->m_szName)) );
	m_sprLogo.SetXY( LOGO_X, LOGO_Y );
	m_sprLogo.SetGlow( RageColor(1,1,1,1) );
	m_sprLogo.SetZoomY( 0 );
	m_sprLogo.StopTweening();
	m_sprLogo.BeginTweening( 0.5f );	// sleep
	m_sprLogo.BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
	m_sprLogo.SetEffectGlowing(1, RageColor(1,1,1,0.1f), RageColor(1,1,1,0.3f) );
	m_sprLogo.SetTweenZoom( 1 );
	this->AddChild( &m_sprLogo );

	m_textVersion.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textVersion.SetText( "CVS" );
	m_textVersion.SetDiffuse( RageColor(0.6f,0.6f,0.6f,1) );	// light gray
	m_textVersion.SetXY( VERSION_X, VERSION_Y );
	m_textVersion.SetZoom( 0.5f );
	m_textVersion.SetShadowLength( 2 );
	this->AddChild( &m_textVersion );


	m_textSongs.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textSongs.SetHorizAlign( Actor::align_left );
	m_textSongs.SetText( ssprintf("%d songs in %d groups", SONGMAN->GetNumSongs(), SONGMAN->GetNumGroups()) );
	m_textSongs.SetDiffuse( RageColor(0.6f,0.6f,0.6f,1) );	// light gray
	m_textSongs.SetXY( SONGS_X, SONGS_Y );
	m_textSongs.SetZoom( 0.5f );
	m_textSongs.SetShadowLength( 2 );
	this->AddChild( &m_textSongs );

	this->MoveToTail( &(ScreenAttract::m_Fade) );	// put it in the back so it covers up the stuff we just added
}
