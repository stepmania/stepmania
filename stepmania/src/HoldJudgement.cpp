#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: HoldJudgement.h

 Desc: A graphic displayed in the HoldJudgement during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "HoldJudgement.h"
#include "RageUtil.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"


const float JUDGEMENT_DISPLAY_TIME	=	0.6f;


HoldJudgement::HoldJudgement()
{
	m_fDisplayCountdown = 0;
	m_sprJudgement.Load( THEME->GetPathTo(GRAPHIC_JUDGEMENT) );
	m_sprJudgement.StopAnimating();
	this->AddActor( &m_sprJudgement );
}

void HoldJudgement::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fDisplayCountdown -= fDeltaTime;
	if( m_fDisplayCountdown < 0 )
		m_fDisplayCountdown = 0;
}

void HoldJudgement::RenderPrimitives()
{
	if( m_fDisplayCountdown > 0 )
	{
		ActorFrame::RenderPrimitives();
	}
}

void HoldJudgement::SetHoldJudgement( HoldNoteResult result )
{
	//HELPER.Log( "Judgement::SetJudgement()" );

	switch( result )
	{
	case HNR_NONE:	m_sprJudgement.SetState( 0 );	break;
	case HNR_OK:	m_sprJudgement.SetState( 7 );	break;
	case HNR_NG:	m_sprJudgement.SetState( 8 );	break;
	default:	ASSERT( false );
	}

	m_fDisplayCountdown = JUDGEMENT_DISPLAY_TIME;

	if( result == HNR_NG ) 
	{
		// falling down
		m_sprJudgement.SetY( -10 );
		m_sprJudgement.SetZoom( 1.0f );
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME );
		m_sprJudgement.SetTweenY( 10 );
	} 
	else if( result == HNR_OK ) 
	{		
		// zooming out
		m_sprJudgement.SetZoom( 1.5f );
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME/3.0f );
		m_sprJudgement.SetTweenZoom( 1.0f );
	}
}