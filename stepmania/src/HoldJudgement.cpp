#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: HoldJudgement.h

 Desc: A graphic displayed in the HoldJudgement during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "HoldJudgement.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"


const float JUDGEMENT_DISPLAY_TIME	=	0.6f;


HoldJudgement::HoldJudgement()
{
	m_fDisplayCountdown = 0;
	m_sprJudgement.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_JUDGEMENT) );
	m_sprJudgement.StopAnimating();
	m_sprJudgement.TurnShadowOn();
	this->AddSubActor( &m_sprJudgement );
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
	case HNS_OK:	m_sprJudgement.SetState( 7 );	break;
	case HNS_NG:	m_sprJudgement.SetState( 8 );	break;
	default:	ASSERT( false );
	}

	m_fDisplayCountdown = JUDGEMENT_DISPLAY_TIME;

	if( hns == HNS_NG ) 
	{
		// falling down
		m_sprJudgement.SetY( -10 );
		m_sprJudgement.SetZoom( 1.0f );
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME );
		m_sprJudgement.SetTweenY( 10 );
	} 
	else // hns == HNS_OK
	{		
		// zooming out
		m_sprJudgement.SetZoom( 1.5f );
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME/3.0f );
		m_sprJudgement.SetTweenZoom( 1.0f );
	}
}