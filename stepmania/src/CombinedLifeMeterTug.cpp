#include "global.h"
/*
-----------------------------------------------------------------------------
 File: CombinedLifeMeterTug.h

 Desc: The song's CombinedLifeMeterTug displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "CombinedLifeMeterTug.h"
#include "ThemeManager.h"
#include "GameState.h"


const float METER_WIDTH = 550;

const float FACE_X[NUM_PLAYERS] = { -300, +300 };
const float FACE_Y[NUM_PLAYERS] = { 0, 0 };


CombinedLifeMeterTug::CombinedLifeMeterTug() 
{
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_Stream[p].Load( THEME->GetPathToG(ssprintf("CombinedLifeMeterTug stream p%d",p+1)), METER_WIDTH );
		this->AddChild( &m_Stream[p] );
	}
	m_Stream[PLAYER_2].SetZoomX( -1 );

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

	ActorFrame::Update( fDelta );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, TapNoteScore score )
{
	float fPercentToMove;
	switch( score )
	{
	case TNS_MARVELOUS:		fPercentToMove = +0.010f;	break;
	case TNS_PERFECT:		fPercentToMove = +0.010f;	break;
	case TNS_GREAT:			fPercentToMove = +0.005f;	break;
	case TNS_GOOD:			fPercentToMove = +0.000f;	break;
	case TNS_BOO:			fPercentToMove = -0.010f;	break;
	case TNS_MISS:			fPercentToMove = -0.020f;	break;
	default:	ASSERT(0);	fPercentToMove = +0.000f;	break;
	}

	switch( pn )
	{
	case PLAYER_1:	GAMESTATE->m_fTugLifePercentP1 += fPercentToMove;	break;
	case PLAYER_2:	GAMESTATE->m_fTugLifePercentP1 -= fPercentToMove;	break;
	default:	ASSERT(0);
	}

	CLAMP( GAMESTATE->m_fTugLifePercentP1, 0, 1 );
}

void CombinedLifeMeterTug::ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore )
{

}
