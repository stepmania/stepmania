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


#define NUM_FEET_IN_METER						THEME->GetMetricI(m_sName,"NumFeetInMeter")
#define MAX_FEET_IN_METER						THEME->GetMetricI(m_sName,"MaxFeetInMeter")
#define GLOW_IF_METER_GREATER_THAN				THEME->GetMetricI(m_sName,"GlowIfMeterGreaterThan")
#define SHOW_FEET								THEME->GetMetricB(m_sName,"ShowFeet")
/* "easy", "hard" */
#define SHOW_DIFFICULTY							THEME->GetMetricB(m_sName,"ShowDifficulty")
/* 3, 9 */
#define SHOW_METER								THEME->GetMetricB(m_sName,"ShowMeter")
#define FEET_IS_DIFFICULTY_COLOR				THEME->GetMetricB(m_sName,"FeetIsDifficultyColor")
#define FEET_PER_DIFFICULTY						THEME->GetMetricB(m_sName,"FeetPerDifficulty")

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

void DifficultyMeter::Load()
{
	if( SHOW_FEET )
	{
		m_textFeet.SetName( "Feet" );
		CString Feet;
		if( FEET_PER_DIFFICULTY )
		{
			for( unsigned i = 0; i < NUM_DIFFICULTIES; ++i )
				Feet += char(i + '0'); // 01234
			Feet += 'X'; // Off
		}
		else
			Feet = "0X";
		m_textFeet.LoadFromTextureAndChars( THEME->GetPathG(m_sName,"bar"), Feet );
		SET_XY_AND_ON_COMMAND( &m_textFeet );
		this->AddChild( &m_textFeet );
	}

	if( SHOW_DIFFICULTY )
	{
		m_Difficulty.Load( THEME->GetPathG(m_sName,"difficulty") );
		m_Difficulty->SetName( "Difficulty" );
		SET_XY_AND_ON_COMMAND( m_Difficulty );
		this->AddChild( m_Difficulty );
	}

	if( SHOW_METER )
	{
		m_textMeter.SetName( "Meter" );
		m_textMeter.LoadFromFont( THEME->GetPathF(m_sName,"meter") );
		SET_XY_AND_ON_COMMAND( m_textMeter );
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
	if( SHOW_FEET )
	{
		CString on = "0";
		CString off = "X";
		if( FEET_PER_DIFFICULTY )
			on = char(dc + '0');

		CString sNewText;
		int f;
		for( f=0; f<NUM_FEET_IN_METER; f++ )
			sNewText += (f<iMeter) ? on : off;
		for( f=NUM_FEET_IN_METER; f<MAX_FEET_IN_METER && f<iMeter; f++ )
			sNewText += on;

		m_textFeet.SetText( sNewText );

		if( iMeter > GLOW_IF_METER_GREATER_THAN )
			m_textFeet.SetEffectGlowShift();
		else
			m_textFeet.SetEffectNone();

		if( FEET_IS_DIFFICULTY_COLOR )
			m_textFeet.SetDiffuse( SONGMAN->GetDifficultyColor( dc ) );
	}

	if( SHOW_METER )
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

	if( SHOW_DIFFICULTY )
		COMMAND( m_Difficulty, "Set" + Capitalize(diff) );
	if( SHOW_METER )
		COMMAND( m_textMeter, "Set" + Capitalize(diff) );
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
