#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: HoldJudgment

 Desc: A graphic displayed in the HoldJudgment during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "HoldJudgment.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


CachedThemeMetric	OK_ZOOM_X		("HoldJudgment","OKZoomX");
CachedThemeMetric	OK_ZOOM_Y		("HoldJudgment","OKZoomY");
CachedThemeMetric	TWEEN_SECONDS	("HoldJudgment","TweenSeconds");
CachedThemeMetric	SHOW_SECONDS	("HoldJudgment","ShowSeconds");


HoldJudgment::HoldJudgment()
{
	OK_ZOOM_X.Refresh();
	OK_ZOOM_Y.Refresh();
	TWEEN_SECONDS.Refresh();
	SHOW_SECONDS.Refresh();

	m_fShowCountdown = 0;
	m_sprJudgment.Load( THEME->GetPathTo("Graphics","gameplay hold Judgment 1x2") );
	m_sprJudgment.StopAnimating();
	m_sprJudgment.TurnShadowOn();
	this->AddChild( &m_sprJudgment );
}

void HoldJudgment::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fShowCountdown -= fDeltaTime;
	if( m_fShowCountdown < 0 )
		m_fShowCountdown = 0;
}

void HoldJudgment::DrawPrimitives()
{
	if( m_fShowCountdown > 0 )
	{
		ActorFrame::DrawPrimitives();
	}
}

void HoldJudgment::SetHoldJudgment( HoldNoteScore hns )
{
	//LOG->Trace( "Judgment::SetJudgment()" );

	switch( hns )
	{
	case HNS_NONE:	m_sprJudgment.SetState( 0 );	break;
	case HNS_OK:	m_sprJudgment.SetState( 0 );	break;
	case HNS_NG:	m_sprJudgment.SetState( 1 );	break;
	default:	ASSERT( false );
	}

	m_fShowCountdown = SHOW_SECONDS;

	if( hns == HNS_NG ) 
	{
		// falling down
		m_sprJudgment.StopTweening();
		m_sprJudgment.SetY( -10 );
		m_sprJudgment.SetZoom( 1.0f );
		m_sprJudgment.BeginTweening( SHOW_SECONDS );
		m_sprJudgment.SetTweenY( 10 );
	} 
	else // hns == HNS_OK
	{		
		// zooming out
		m_sprJudgment.StopTweening();
		m_sprJudgment.SetZoomX( OK_ZOOM_X );
		m_sprJudgment.SetZoomY( OK_ZOOM_Y );
		m_sprJudgment.BeginTweening( TWEEN_SECONDS );
		m_sprJudgment.SetTweenZoom( 1.0f );
	}
}
