#include "global.h"
/*
-----------------------------------------------------------------------------
 File: CombinedLifeMeterTug.h

 Desc: Dance Magic-like tug-o-war life meter.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "CombinedLifeMeterTug.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "PrefsManager.h"


CachedThemeMetricF METER_WIDTH		("CombinedLifeMeterTug","MeterWidth");

const float FACE_X[NUM_PLAYERS] = { -300, +300 };
const float FACE_Y[NUM_PLAYERS] = { 0, 0 };


CombinedLifeMeterTug::CombinedLifeMeterTug() 
{
	METER_WIDTH.Refresh();

	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		CString sStreamPath = THEME->GetPathToG(ssprintf("CombinedLifeMeterTug stream p%d",p+1));
		CString sTipPath = THEME->GetPathToG(ssprintf("CombinedLifeMeterTug tip p%d",p+1));
		m_Stream[p].Load( sStreamPath, METER_WIDTH, sTipPath );
		this->AddChild( &m_Stream[p] );
	}
	m_Stream[PLAYER_2].SetZoomX( -1 );

	m_sprSeparator.Load( THEME->GetPathToG(ssprintf("CombinedLifeMeterTug separator")) );
	this->AddChild( &m_sprSeparator );

	m_sprFrame.Load( THEME->GetPathToG(ssprintf("CombinedLifeMeterTug frame")) );
	this->AddChild( &m_sprFrame );
	
	
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		Character* pCharacter = GAMESTATE->m_pCurCharacters[p];
		ASSERT( pCharacter );

		m_Head[p].LoadFromCharacter( pCharacter );
		m_Head[p].SetXY( FACE_X[p], FACE_Y[p] );
		this->AddChild( &m_Head[p] );
	}
}

void CombinedLifeMeterTug::Update( float fDelta )
{
	m_Stream[PLAYER_1].SetPercent( GAMESTATE->m_fTugLifePercentP1 );
	m_Stream[PLAYER_2].SetPercent( 1-GAMESTATE->m_fTugLifePercentP1 );

	float fSeparatorX = SCALE( GAMESTATE->m_fTugLifePercentP1, 0.f, 1.f, -METER_WIDTH/2.f, +METER_WIDTH/2.f );

	m_sprSeparator.SetX( fSeparatorX );

	ActorFrame::Update( fDelta );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, TapNoteScore score )
{
	float fPercentToMove = 0;
	switch( score )
	{
	case TNS_MARVELOUS:		fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeMarvelous;	break;
	case TNS_PERFECT:		fPercentToMove = PREFSMAN->m_fTugMeterPercentChangePerfect;	break;
	case TNS_GREAT:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeGreat;		break;
	case TNS_GOOD:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeGood;		break;
	case TNS_BOO:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeBoo;		break;
	case TNS_MISS:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeMiss;		break;
	default:	ASSERT(0);	break;
	}

	ChangeLife( pn, fPercentToMove );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore )
{
	/* The initial tap note score (which we happen to have in have in
	 * tscore) has already been reported to the above function.  If the
	 * hold end result was an NG, count it as a miss; if the end result
	 * was an OK, count a perfect.  (Remember, this is just life meter
	 * computation, not scoring.) */
	float fPercentToMove = 0;
	switch( score )
	{
	case HNS_OK:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeOK;	break;
	case HNS_NG:			fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeNG;	break;
	default:	ASSERT(0);	break;
	}

	ChangeLife( pn, fPercentToMove );
}

void CombinedLifeMeterTug::ChangeLifeMine( PlayerNumber pn )
{
	float fPercentToMove = PREFSMAN->m_fTugMeterPercentChangeHitMine;

	ChangeLife( pn, fPercentToMove );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, float fPercentToMove )
{
	if( PREFSMAN->m_bMercifulDrain  &&  fPercentToMove < 0 )
	{
		float fLifePercentage = 0;
		switch( pn )
		{
		case PLAYER_1:	fLifePercentage = GAMESTATE->m_fTugLifePercentP1;		break;
		case PLAYER_2:	fLifePercentage = 1 - GAMESTATE->m_fTugLifePercentP1;	break;
		default:	ASSERT(0);
		}
		fPercentToMove *= SCALE( fLifePercentage, 0.f, 1.f, 0.2f, 1.f);
	}

	switch( pn )
	{
	case PLAYER_1:	GAMESTATE->m_fTugLifePercentP1 += fPercentToMove;	break;
	case PLAYER_2:	GAMESTATE->m_fTugLifePercentP1 -= fPercentToMove;	break;
	default:	ASSERT(0);
	}
	CLAMP( GAMESTATE->m_fTugLifePercentP1, 0, 1 );
}
