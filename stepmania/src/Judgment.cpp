#include "global.h"
#include "Judgment.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "ThemeManager.h"


static CachedThemeMetric	MARVELOUS_COMMAND		("Judgment","MarvelousCommand");
static CachedThemeMetric	PERFECT_COMMAND			("Judgment","PerfectCommand");
static CachedThemeMetric	GREAT_COMMAND			("Judgment","GreatCommand");
static CachedThemeMetric	GOOD_COMMAND			("Judgment","GoodCommand");
static CachedThemeMetric	BOO_COMMAND				("Judgment","BooCommand");
static CachedThemeMetric	MISS_COMMAND			("Judgment","MissCommand");

static CachedThemeMetric	MARVELOUS_ODD_COMMAND	("Judgment","MarvelousOddCommand");
static CachedThemeMetric	PERFECT_ODD_COMMAND		("Judgment","PerfectOddCommand");
static CachedThemeMetric	GREAT_ODD_COMMAND		("Judgment","GreatOddCommand");
static CachedThemeMetric	GOOD_ODD_COMMAND		("Judgment","GoodOddCommand");
static CachedThemeMetric	BOO_ODD_COMMAND			("Judgment","BooOddCommand");
static CachedThemeMetric	MISS_ODD_COMMAND		("Judgment","MissOddCommand");

static CachedThemeMetric	MARVELOUS_EVEN_COMMAND	("Judgment","MarvelousEvenCommand");
static CachedThemeMetric	PERFECT_EVEN_COMMAND	("Judgment","PerfectEvenCommand");
static CachedThemeMetric	GREAT_EVEN_COMMAND		("Judgment","GreatEvenCommand");
static CachedThemeMetric	GOOD_EVEN_COMMAND		("Judgment","GoodEvenCommand");
static CachedThemeMetric	BOO_EVEN_COMMAND		("Judgment","BooEvenCommand");
static CachedThemeMetric	MISS_EVEN_COMMAND		("Judgment","MissEvenCommand");


Judgment::Judgment()
{
	MARVELOUS_COMMAND.Refresh();
	PERFECT_COMMAND.Refresh();
	GREAT_COMMAND.Refresh();
	GOOD_COMMAND.Refresh();
	BOO_COMMAND.Refresh();
	MISS_COMMAND.Refresh();

	MARVELOUS_ODD_COMMAND.Refresh();
	PERFECT_ODD_COMMAND.Refresh();
	GREAT_ODD_COMMAND.Refresh();
	GOOD_ODD_COMMAND.Refresh();
	BOO_ODD_COMMAND.Refresh();
	MISS_ODD_COMMAND.Refresh();

	MARVELOUS_EVEN_COMMAND.Refresh();
	PERFECT_EVEN_COMMAND.Refresh();
	GREAT_EVEN_COMMAND.Refresh();
	GOOD_EVEN_COMMAND.Refresh();
	BOO_EVEN_COMMAND.Refresh();
	MISS_EVEN_COMMAND.Refresh();

	m_iCount = 0;

	m_sprJudgment.Load( THEME->GetPathToG("Judgment 1x6") );
	m_sprJudgment.StopAnimating();
	Reset();
	this->AddChild( &m_sprJudgment );
}


void Judgment::Reset()
{
	m_sprJudgment.FinishTweening();
	m_sprJudgment.SetXY( 0, 0 );
	m_sprJudgment.SetEffectNone();
	m_sprJudgment.SetHidden( true );
}

void Judgment::SetJudgment( TapNoteScore score )
{
	//LOG->Trace( "Judgment::SetJudgment()" );

	Reset();

	m_sprJudgment.SetHidden( false );

	switch( score )
	{
	case TNS_MARVELOUS:
		m_sprJudgment.SetState( 0 );
		m_sprJudgment.Command( (m_iCount%2) ? MARVELOUS_ODD_COMMAND : MARVELOUS_EVEN_COMMAND );
		m_sprJudgment.Command( MARVELOUS_COMMAND );
		break;
	case TNS_PERFECT:
		m_sprJudgment.SetState( 1 );
		m_sprJudgment.Command( (m_iCount%2) ? PERFECT_ODD_COMMAND : PERFECT_EVEN_COMMAND );
		m_sprJudgment.Command( PERFECT_COMMAND );
		break;
	case TNS_GREAT:
		m_sprJudgment.SetState( 2 );
		m_sprJudgment.Command( (m_iCount%2) ? GREAT_ODD_COMMAND : GREAT_EVEN_COMMAND );
		m_sprJudgment.Command( GREAT_COMMAND );
		break;
	case TNS_GOOD:
		m_sprJudgment.SetState( 3 );
		m_sprJudgment.Command( (m_iCount%2) ? GOOD_ODD_COMMAND : GOOD_EVEN_COMMAND );
		m_sprJudgment.Command( GOOD_COMMAND );
		break;
	case TNS_BOO:
		m_sprJudgment.SetState( 4 );
		m_sprJudgment.Command( (m_iCount%2) ? BOO_ODD_COMMAND : BOO_EVEN_COMMAND );
		m_sprJudgment.Command( BOO_COMMAND );
		break;
	case TNS_MISS:
		m_sprJudgment.SetState( 5 );
		m_sprJudgment.Command( (m_iCount%2) ? MISS_ODD_COMMAND : MISS_EVEN_COMMAND );
		m_sprJudgment.Command( MISS_COMMAND );
		break;
	default:
		ASSERT(0);
	}

	m_iCount++;
}

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
