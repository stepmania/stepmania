#include "global.h"
#include "Judgment.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "ThemeMetric.h"

static ThemeMetric<apActorCommands>	W1_COMMAND		("Judgment","W1Command");
static ThemeMetric<apActorCommands>	W2_COMMAND		("Judgment","W2Command");
static ThemeMetric<apActorCommands>	W3_COMMAND		("Judgment","W3Command");
static ThemeMetric<apActorCommands>	W4_COMMAND		("Judgment","W4Command");
static ThemeMetric<apActorCommands>	W5_COMMAND		("Judgment","W5Command");
static ThemeMetric<apActorCommands>	MISS_COMMAND	("Judgment","MissCommand");

Judgment::Judgment()
{
}

void Judgment::Load( bool bBeginner )
{
	m_sprJudgment.Load( THEME->GetPathG("Judgment",bBeginner?"BeginnerLabel":"label") );
	ASSERT( m_sprJudgment.GetNumStates() == 6  ||  m_sprJudgment.GetNumStates() == 12 );
	m_sprJudgment.StopAnimating();
	Reset();
	this->AddChild( &m_sprJudgment );
}


void Judgment::Reset()
{
	m_sprJudgment.FinishTweening();
	m_sprJudgment.SetXY( 0, 0 );
	m_sprJudgment.StopEffect();
	m_sprJudgment.SetHidden( true );
}

void Judgment::SetJudgment( TapNoteScore score, bool bEarly )
{
	//LOG->Trace( "Judgment::SetJudgment()" );

	Reset();

	m_sprJudgment.SetHidden( false );

	int iStateMult = (m_sprJudgment.GetNumStates()==12) ? 2 : 1;
	int iStateAdd = ( bEarly || ( iStateMult == 1 ) ) ? 0 : 1;

	switch( score )
	{
	case TNS_W1:
		m_sprJudgment.SetState( 0 * iStateMult + iStateAdd );
		m_sprJudgment.RunCommands( W1_COMMAND );
		break;
	case TNS_W2:
		m_sprJudgment.SetState( 1 * iStateMult + iStateAdd );
		m_sprJudgment.RunCommands( W2_COMMAND );
		break;
	case TNS_W3:
		m_sprJudgment.SetState( 2 * iStateMult + iStateAdd );
		m_sprJudgment.RunCommands( W3_COMMAND );
		break;
	case TNS_W4:
		m_sprJudgment.SetState( 3 * iStateMult + iStateAdd );
		m_sprJudgment.RunCommands( W4_COMMAND );
		break;
	case TNS_W5:
		m_sprJudgment.SetState( 4 * iStateMult + iStateAdd );
		m_sprJudgment.RunCommands( W5_COMMAND );
		break;
	case TNS_Miss:
		m_sprJudgment.SetState( 5 * iStateMult + iStateAdd );
		m_sprJudgment.RunCommands( MISS_COMMAND );
		break;
	default:
		ASSERT(0);
	}
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
