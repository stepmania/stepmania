#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Judgement.h

 Desc: A graphic displayed in the Judgement during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Judgement.h"
#include "RageUtil.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"


const float JUDGEMENT_DISPLAY_TIME	=	0.6f;


Judgement::Judgement()
{
	m_fDisplayCountdown = 0;
	m_sprJudgement.Load( THEME->GetPathTo(GRAPHIC_JUDGEMENT) );
	m_sprJudgement.StopAnimating();
	this->AddActor( &m_sprJudgement );
}

void Judgement::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fDisplayCountdown -= fDeltaTime;
	if( m_fDisplayCountdown < 0 )
		m_fDisplayCountdown = 0;
}

void Judgement::RenderPrimitives()
{
	if( m_fDisplayCountdown > 0 )
	{
		ActorFrame::RenderPrimitives();
	}
}


void Judgement::SetJudgement( TapStepScore score )
{
	//RageLog( "Judgement::SetJudgement()" );

	switch( score )
	{
	case TSS_PERFECT:	m_sprJudgement.SetState( 0 );	break;
	case TSS_GREAT:		m_sprJudgement.SetState( 1 );	break;
	case TSS_GOOD:		m_sprJudgement.SetState( 2 );	break;
	case TSS_BOO:		m_sprJudgement.SetState( 3 );	break;
	case TSS_MISS:		m_sprJudgement.SetState( 4 );	break;
	default:	ASSERT( false );
	}

	m_fDisplayCountdown = JUDGEMENT_DISPLAY_TIME;

	if( score == TSS_MISS )
	{
		// falling down
		m_sprJudgement.SetY( -30 );
		m_sprJudgement.SetZoom( 1.0f );
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME );
		m_sprJudgement.SetTweenY( 30 );
	} 
	else
	{
		// zooming out
		m_sprJudgement.SetZoom( 1.35f );
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME/5.0f );
		m_sprJudgement.SetTweenZoom( 1.0f );
	}
}