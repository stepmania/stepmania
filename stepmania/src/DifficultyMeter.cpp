#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: DifficultyMeter

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "DifficultyMeter.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "Course.h"
#include "SongManager.h"


#define NUM_FEET_IN_METER						THEME->GetMetricI("DifficultyMeter","NumFeetInMeter")
#define MAX_FEET_IN_METER						THEME->GetMetricI("DifficultyMeter","MaxFeetInMeter")
#define GLOW_IF_METER_GREATER_THAN				THEME->GetMetricI("DifficultyMeter","GlowIfMeterGreaterThan")


DifficultyMeter::DifficultyMeter()
{
	BitmapText::LoadFromTextureAndChars( THEME->GetPathToG("DifficultyMeter bar 2x1"), "10" );

	Unset();
}

void DifficultyMeter::SetFromNotes( const Steps* pNotes )
{
	if( pNotes == NULL )
	{
		Unset();
		return;
	}

	SetMeter( pNotes->GetMeter() );
	SetDiffuse( SONGMAN->GetDifficultyColor(pNotes->GetDifficulty()) );
}

void DifficultyMeter::SetFromCourse( const Course* pCourse )
{
	if( pCourse == NULL )
	{
		Unset();
		return;
	}

	const int meter = (int) roundf(pCourse->GetMeter());
	SetMeter( meter );
	
	// XXX
	RageColor c;
	if( meter <= 1 )
		c = SONGMAN->GetDifficultyColor( DIFFICULTY_BEGINNER );
	else if( meter <= 2 )
		c = SONGMAN->GetDifficultyColor( DIFFICULTY_EASY );
	else if( meter <= 5 )
		c = SONGMAN->GetDifficultyColor( DIFFICULTY_MEDIUM );
	else if( meter <= 7 )
		c = SONGMAN->GetDifficultyColor( DIFFICULTY_HARD );
	else
		c = SONGMAN->GetDifficultyColor( DIFFICULTY_CHALLENGE );
	SetDiffuse( c );
}

void DifficultyMeter::Unset()
{
	SetEffectNone();
	SetDiffuse( RageColor(0.8f,0.8f,0.8f,1) );
	SetMeter( 0 );
}

void DifficultyMeter::SetFromGameState( PlayerNumber pn )
{
	if( GAMESTATE->m_pCurNotes[pn] )
		SetFromNotes( GAMESTATE->m_pCurNotes[pn] );
	else if( GAMESTATE->m_pCurCourse )
		SetFromCourse( GAMESTATE->m_pCurCourse );
	else
		Unset();
}

void DifficultyMeter::SetMeter( int iMeter )
{
	CString sNewText;
	int f;
	for( f=0; f<NUM_FEET_IN_METER; f++ )
		sNewText += (f<iMeter) ? "1" : "0";
	for( f=NUM_FEET_IN_METER; f<MAX_FEET_IN_METER; f++ )
		if( f<iMeter )
			sNewText += "1";

	SetText( sNewText );

	if( iMeter > GLOW_IF_METER_GREATER_THAN )
		SetEffectGlowShift();
	else
		SetEffectNone();
}
