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


static CachedThemeMetric	MARVELOUS_COMMAND	("Judgment","MarvelousCommand");
static CachedThemeMetric	PERFECT_COMMAND		("Judgment","PerfectCommand");
static CachedThemeMetric	GREAT_COMMAND		("Judgment","GreatCommand");
static CachedThemeMetric	GOOD_COMMAND		("Judgment","GoodCommand");
static CachedThemeMetric	BOO_COMMAND			("Judgment","BooCommand");
static CachedThemeMetric	MISS_COMMAND		("Judgment","MissCommand");


Judgment::Judgment()
{
	MARVELOUS_COMMAND.Refresh();
	PERFECT_COMMAND.Refresh();
	GREAT_COMMAND.Refresh();
	GOOD_COMMAND.Refresh();
	BOO_COMMAND.Refresh();
	MISS_COMMAND.Refresh();

	m_sprJudgment.Load( THEME->GetPathToG("Judgment 1x6") );
	m_sprJudgment.StopAnimating();
	Reset();
	this->AddChild( &m_sprJudgment );
}


void Judgment::Reset()
{
	m_sprJudgment.SetDiffuse( RageColor(1,1,1,0) );
	m_sprJudgment.SetXY( 0, 0 );
	m_sprJudgment.StopTweening();
	m_sprJudgment.SetEffectNone();
}

void Judgment::SetJudgment( TapNoteScore score )
{
	//LOG->Trace( "Judgment::SetJudgment()" );

	Reset();

	switch( score )
	{
	case TNS_MARVELOUS:
		m_sprJudgment.SetState( 0 );
		m_sprJudgment.Command( MARVELOUS_COMMAND );
		break;
	case TNS_PERFECT:
		m_sprJudgment.SetState( 1 );
		m_sprJudgment.Command( PERFECT_COMMAND );
		break;
	case TNS_GREAT:
		m_sprJudgment.SetState( 2 );
		m_sprJudgment.Command( GREAT_COMMAND );
		break;
	case TNS_GOOD:
		m_sprJudgment.SetState( 3 );
		m_sprJudgment.Command( GOOD_COMMAND );
		break;
	case TNS_BOO:
		m_sprJudgment.SetState( 4 );
		m_sprJudgment.Command( BOO_COMMAND );
		break;
	case TNS_MISS:
		m_sprJudgment.SetState( 5 );
		m_sprJudgment.Command( MISS_COMMAND );
		break;
	default:
		ASSERT(0);
	}
}
