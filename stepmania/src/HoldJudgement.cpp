#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: HoldJudgement

 Desc: A graphic displayed in the HoldJudgement during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "HoldJudgement.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


//
// Important!!!!  Do not use these macros during gameplay.  They return very slowly.  Cache them in a member.
//
#define JUDGEMENT_DISPLAY_TIME			THEME->GetMetricF("HoldJudgement","DisplayTime")


HoldJudgement::HoldJudgement()
{
	m_fDisplayTime = JUDGEMENT_DISPLAY_TIME;
	m_fDisplayCountdown = 0;
	m_sprJudgement.Load( THEME->GetPathTo("Graphics","gameplay hold judgement") );
	m_sprJudgement.StopAnimating();
	m_sprJudgement.TurnShadowOn();
	this->AddChild( &m_sprJudgement );
}

void HoldJudgement::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fDisplayCountdown -= fDeltaTime;
	if( m_fDisplayCountdown < 0 )
		m_fDisplayCountdown = 0;
}

void HoldJudgement::DrawPrimitives()
{
	if( m_fDisplayCountdown > 0 )
	{
		ActorFrame::DrawPrimitives();
	}
}

void HoldJudgement::SetHoldJudgement( HoldNoteScore hns )
{
	//LOG->Trace( "Judgement::SetJudgement()" );

	switch( hns )
	{
	case HNS_NONE:	m_sprJudgement.SetState( 0 );	break;
	case HNS_OK:	m_sprJudgement.SetState( 0 );	break;
	case HNS_NG:	m_sprJudgement.SetState( 1 );	break;
	default:	ASSERT( false );
	}

	m_fDisplayCountdown = m_fDisplayTime;

	if( hns == HNS_NG ) 
	{
		// falling down
		m_sprJudgement.StopTweening();
		m_sprJudgement.SetY( -10 );
		m_sprJudgement.SetZoom( 1.0f );
		m_sprJudgement.BeginTweening( m_fDisplayTime );
		m_sprJudgement.SetTweenY( 10 );
	} 
	else // hns == HNS_OK
	{		
		// zooming out
		m_sprJudgement.StopTweening();
		m_sprJudgement.SetZoom( 1.5f );
		m_sprJudgement.BeginTweening( m_fDisplayTime/3.0f );
		m_sprJudgement.SetTweenZoom( 1.0f );
	}
}
