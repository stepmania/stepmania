#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Judgment

 Desc: See header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Judgment.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"


CachedThemeMetric	WORD_X			("Judgment","WordX");
CachedThemeMetric	WORD_Y			("Judgment","WordY");
CachedThemeMetric	MARVELOUS_ZOOM	("Judgment","MarvelousZoom");
CachedThemeMetric	PERFECT_ZOOM	("Judgment","PerfectZoom");
CachedThemeMetric	GREAT_ZOOM		("Judgment","GreatZoom");
CachedThemeMetric	GOOD_ZOOM		("Judgment","GoodZoom");
CachedThemeMetric	BOO_ZOOM		("Judgment","BooZoom");
CachedThemeMetric	J_TWEEN_SECONDS	("Judgment","TweenSeconds");
CachedThemeMetric	J_SHOW_SECONDS	("Judgment","ShowSeconds");


Judgment::Judgment()
{
	WORD_X.Refresh();
	WORD_Y.Refresh();
	MARVELOUS_ZOOM.Refresh();
	PERFECT_ZOOM.Refresh();
	GREAT_ZOOM.Refresh();
	GOOD_ZOOM.Refresh();
	BOO_ZOOM.Refresh();
	J_TWEEN_SECONDS.Refresh();
	J_SHOW_SECONDS.Refresh();

	m_fShowCountdown = 0;
	m_sprJudgment.Load( THEME->GetPathTo("Graphics","gameplay Judgment 1x6") );
	m_sprJudgment.SetXY( WORD_X, WORD_Y );
	m_sprJudgment.StopAnimating();
	m_sprJudgment.TurnShadowOn();
	this->AddChild( &m_sprJudgment );
}

void Judgment::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_fShowCountdown -= fDeltaTime;
	if( m_fShowCountdown < 0 )
		m_fShowCountdown = 0;
}

void Judgment::DrawPrimitives()
{
	if( m_fShowCountdown > 0 )
	{
		ActorFrame::DrawPrimitives();
	}
}


void Judgment::SetJudgment( TapNoteScore score )
{
	//LOG->Trace( "Judgment::SetJudgment()" );

	switch( score )
	{
	case TNS_MARVELOUS:	m_sprJudgment.SetState( 0 );	break;
	case TNS_PERFECT:	m_sprJudgment.SetState( 1 );	break;
	case TNS_GREAT:		m_sprJudgment.SetState( 2 );	break;
	case TNS_GOOD:		m_sprJudgment.SetState( 3 );	break;
	case TNS_BOO:		m_sprJudgment.SetState( 4 );	break;
	case TNS_MISS:		m_sprJudgment.SetState( 5 );	break;
	default:	ASSERT(0);
	}

	m_fShowCountdown = J_SHOW_SECONDS;

	m_sprJudgment.SetEffectNone();
	m_sprJudgment.StopTweening();

	if( score == TNS_MISS )
	{
		// falling down
		m_sprJudgment.SetY( -20 );
		m_sprJudgment.BeginTweening( J_SHOW_SECONDS );
		m_sprJudgment.SetTweenY( 20 );
	} 
	else if( score == TNS_BOO )
	{
		// vibrate
		m_sprJudgment.SetY( 0 );
		m_sprJudgment.SetZoom( 1.0f );
		m_sprJudgment.SetEffectVibrating();
	}
	else
	{
		// zooming out
		float fZoom;
		switch( score )
		{
		case TNS_MARVELOUS:	fZoom = MARVELOUS_ZOOM;	break;
		case TNS_PERFECT:	fZoom = PERFECT_ZOOM;	break;
		case TNS_GREAT:		fZoom = GREAT_ZOOM;		break;
		case TNS_GOOD:		fZoom = GOOD_ZOOM;		break;
		default: ASSERT(0);	fZoom = 1;				break;
		}
		m_sprJudgment.SetY( 0 );
		m_sprJudgment.SetZoom( fZoom );
		m_sprJudgment.BeginTweening( J_TWEEN_SECONDS );
		m_sprJudgment.SetTweenZoom( 1 );
	}

	if( score == TNS_MARVELOUS )
		m_sprJudgment.SetEffectGlowBlinking(0.05f, RageColor(1,1,1,0), RageColor(1,1,1,0.5f));
	else
		m_sprJudgment.SetEffectNone();
}
