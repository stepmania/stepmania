#include "global.h"
#include "DifficultyMeter.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "Course.h"
#include "SongManager.h"
#include "ActorUtil.h"
#include "Style.h"

// lua start
LUA_REGISTER_CLASS( DifficultyMeter )
// lua end
REGISTER_ACTOR_CLASS( DifficultyMeter );


DifficultyMeter::DifficultyMeter()
{
}

/* sID experiment:
 *
 * Names of an actor, "Foo":
 * [Foo]
 * Metric=abc
 *
 * [ScreenSomething]
 * FooP1X=20
 * FooP2Y=30
 *
 * Graphics\Foo under p1
 *
 * We want to call it different things in different contexts: we may only want one
 * set of internal metrics for a given use, but separate metrics for each player at
 * the screen level, and we may or may not want separate names at the asset level.
 *
 * As is, we tend to end up having to either duplicate [Foo] to [FooP1] and [FooP2]
 * or not use m_sName for [Foo], which limits its use.  Let's try using a separate
 * name for internal metrics.  I'm not sure if this will cause more confusion than good,
 * so I'm trying it first in only this object.
 */

void DifficultyMeter::Load( const CString &sType )
{
	m_sType = sType;

	/* We can't use global ThemeMetric<CString>s, because we can have multiple
	 * DifficultyMeters on screen at once, with different names. */
	m_iNumFeetInMeter = THEME->GetMetricI(m_sType,"NumFeetInMeter");
	m_iMaxFeetInMeter = THEME->GetMetricI(m_sType,"MaxFeetInMeter");
	m_iGlowIfMeterGreaterThan = THEME->GetMetricI(m_sType,"GlowIfMeterGreaterThan");
	m_bShowFeet = THEME->GetMetricB(m_sType,"ShowFeet");
	/* "easy", "hard" */
	m_bShowDifficulty = THEME->GetMetricB(m_sType,"ShowDifficulty");
	/* 3, 9 */
	m_bShowMeter = THEME->GetMetricB(m_sType,"ShowMeter");
	m_bFeetIsDifficultyColor = THEME->GetMetricB(m_sType,"FeetIsDifficultyColor");
	m_bFeetPerDifficulty = THEME->GetMetricB(m_sType,"FeetPerDifficulty");

	if( m_bShowFeet )
	{
		m_textFeet.SetName( "Feet" );
		CString Feet;
		if( m_bFeetPerDifficulty )
		{
			for( unsigned i = 0; i < NUM_DIFFICULTIES; ++i )
				Feet += char(i + '0'); // 01234
			Feet += 'X'; // Off
		}
		else
			Feet = "0X";
		m_textFeet.LoadFromTextureAndChars( THEME->GetPathG(m_sType,"bar"), Feet );
		ActorUtil::SetXYAndOnCommand( m_textFeet, m_sType );
		this->AddChild( &m_textFeet );
	}

	if( m_bShowDifficulty )
	{
		m_Difficulty.Load( THEME->GetPathG(m_sType,"difficulty") );
		m_Difficulty->SetName( "Difficulty" );
		ActorUtil::SetXYAndOnCommand( m_Difficulty, m_sType );
		this->AddChild( m_Difficulty );
	}

	if( m_bShowMeter )
	{
		m_textMeter.SetName( "Meter" );
		m_textMeter.LoadFromFont( THEME->GetPathF(m_sType,"meter") );
		ActorUtil::SetXYAndOnCommand( m_textMeter, m_sType );
		this->AddChild( &m_textMeter );
	}

	Unset();
}

void DifficultyMeter::SetFromGameState( PlayerNumber pn )
{
	if( GAMESTATE->IsCourseMode() )
	{
		const Trail* pTrail = GAMESTATE->m_pCurTrail[pn];
		if( pTrail )
			SetFromTrail( pTrail );
		else
			SetFromCourseDifficulty( GAMESTATE->m_PreferredCourseDifficulty[pn] );
	}
	else
	{
		const Steps* pSteps = GAMESTATE->m_pCurSteps[pn];
		if( pSteps )
			SetFromSteps( pSteps );
		else
			SetFromDifficulty( GAMESTATE->m_PreferredDifficulty[pn] );
	}
}

void DifficultyMeter::SetFromSteps( const Steps* pSteps )
{
	if( pSteps == NULL )
	{
		Unset();
		return;
	}

	SetFromMeterAndDifficulty( pSteps->GetMeter(), pSteps->GetDifficulty() );
	SetDifficulty( DifficultyToString( pSteps->GetDifficulty() ) );
}

void DifficultyMeter::SetFromTrail( const Trail* pTrail )
{
	if( pTrail == NULL )
	{
		Unset();
		return;
	}

	SetFromMeterAndDifficulty( pTrail->GetMeter(), pTrail->m_CourseDifficulty );
	SetDifficulty( CourseDifficultyToString(pTrail->m_CourseDifficulty) + "Course" );
}

void DifficultyMeter::Unset()
{
	SetFromMeterAndDifficulty( 0, DIFFICULTY_BEGINNER );
	SetDifficulty( "None" );
}

void DifficultyMeter::SetFromDifficulty( Difficulty dc )
{
	SetFromMeterAndDifficulty( 0, DIFFICULTY_BEGINNER );
	SetDifficulty( DifficultyToString( dc ) );
}

void DifficultyMeter::SetFromCourseDifficulty( CourseDifficulty cd )
{
	SetFromMeterAndDifficulty( 0, DIFFICULTY_BEGINNER );
	SetDifficulty( CourseDifficultyToString( cd ) + "Course" );
}

void DifficultyMeter::SetFromMeterAndDifficulty( int iMeter, Difficulty dc )
{
	if( m_bShowFeet )
	{
		char on = '0';
		char off = 'X';
		if( m_bFeetPerDifficulty )
			on = char(dc + '0');

		CString sNewText;
		int iNumOn = min( m_iMaxFeetInMeter, iMeter );
		sNewText.insert( sNewText.end(), iNumOn, on );
		int iNumOff = max( 0, m_iNumFeetInMeter-iNumOn );
		sNewText.insert( sNewText.end(), iNumOff, off );

		m_textFeet.SetText( sNewText );

		if( iMeter > m_iGlowIfMeterGreaterThan )
			m_textFeet.SetEffectGlowShift();
		else
			m_textFeet.SetEffectNone();

		if( m_bFeetIsDifficultyColor )
			m_textFeet.SetDiffuse( SONGMAN->GetDifficultyColor( dc ) );
	}

	if( m_bShowMeter )
	{
		if( iMeter == 0 )	// Unset calls with this
		{
			m_textMeter.SetText( "x" );
		}
		else
		{
			const CString sMeter = ssprintf( "%i", iMeter );
			m_textMeter.SetText( sMeter );
		}
	}
}

void DifficultyMeter::SetDifficulty( CString diff )
{
	if( m_CurDifficulty == diff )
		return;
	m_CurDifficulty = diff;

	if( m_bShowDifficulty )
		ActorUtil::RunCommand( m_Difficulty, m_sType, "Set" + Capitalize(diff) );
	if( m_bShowMeter )
		ActorUtil::RunCommand( m_textMeter, m_sType, "Set" + Capitalize(diff) );
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
