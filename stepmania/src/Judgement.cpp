#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Judgement

 Desc: See header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Judgement.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"

const float JUDGEMENT_DISPLAY_TIME	=	0.8f;


Judgement::Judgement()
{
	m_fDisplayCountdown = 0;
	m_sprJudgement.Load( THEME->GetPathTo("Graphics","gameplay judgement 1x6") );
	m_sprJudgement.StopAnimating();
	m_sprJudgement.TurnShadowOn();
	this->AddChild( &m_sprJudgement );
}

void Judgement::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fDisplayCountdown -= fDeltaTime;
	if( m_fDisplayCountdown < 0 )
		m_fDisplayCountdown = 0;
}

void Judgement::DrawPrimitives()
{
	if( m_fDisplayCountdown > 0 )
	{
		ActorFrame::DrawPrimitives();
	}
}


void Judgement::SetJudgement( TapNoteScore score )
{
	//LOG->Trace( "Judgement::SetJudgement()" );

	switch( score )
	{
	case TNS_MARVELOUS:	m_sprJudgement.SetState( 0 );	break;
	case TNS_PERFECT:	m_sprJudgement.SetState( 1 );	break;
	case TNS_GREAT:		m_sprJudgement.SetState( 2 );	break;
	case TNS_GOOD:		m_sprJudgement.SetState( 3 );	break;
	case TNS_BOO:		m_sprJudgement.SetState( 4 );	break;
	case TNS_MISS:		m_sprJudgement.SetState( 5 );	break;
	default:	ASSERT(0);
	}

	m_fDisplayCountdown = JUDGEMENT_DISPLAY_TIME;

	m_sprJudgement.SetEffectNone();
	m_sprJudgement.StopTweening();

	if( score == TNS_MISS )
	{
		// falling down
		m_sprJudgement.SetY( -20 );
		m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME );
		m_sprJudgement.SetTweenY( 20 );
	} 
	else if( score == TNS_BOO )
	{
		// vibrate
		m_sprJudgement.SetY( 0 );
		m_sprJudgement.SetZoom( 1.0f );
		m_sprJudgement.SetEffectVibrating();
	}
	else
	{
		// zooming out
		float fMagnitudeX, fMagnitudeY;
		switch( score )
		{
		case TNS_MARVELOUS:	fMagnitudeX = 1.50f;	fMagnitudeY = 2.00f;	break;
		case TNS_PERFECT:	fMagnitudeX = 1.50f;	fMagnitudeY = 2.00f;	break;
		case TNS_GREAT:		fMagnitudeX = 1.30f;	fMagnitudeY = 1.50f;	break;
		case TNS_GOOD:		fMagnitudeX = 1.10f;	fMagnitudeY = 1.25f;	break;
		default:	ASSERT(false);	// invalid score value
		}
		m_sprJudgement.SetY( 0 );
	//	m_sprJudgement.SetZoomY( fMagnitudeY );
	//	m_sprJudgement.SetZoomX( fMagnitudeX );
	//	m_sprJudgement.BeginTweening( JUDGEMENT_DISPLAY_TIME/5.0f );
	//	m_sprJudgement.SetTweenZoom( 1 );
	}
}
