#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ArrowEffects.cpp

 Desc: Functions that return properties of arrows based on StyleDef and PlayerOptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "ArrowEffects.h"
#include "Notes.h"
#include "ColorNote.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"


float ArrowGetYOffset( const PlayerOptions& po, float fStepIndex, float fSongBeat )
{
	float fBeatsUntilStep = NoteRowToBeat( fStepIndex ) - fSongBeat;
	float fYOffset = fBeatsUntilStep * ARROW_GAP;
	switch( po.m_EffectType )
	{
	case PlayerOptions::EFFECT_BOOST:
		fYOffset *= 1.4f / ((fYOffset+SCREEN_HEIGHT/1.6f)/SCREEN_HEIGHT); 
		break;
	case PlayerOptions::EFFECT_WAVE:
		fYOffset += 15.0f*sinf( fYOffset/38.0f ); 
		break;
	}
	return fYOffset;
}

float ArrowGetXPos( const PlayerOptions& po, int iColNum, float fYOffset, float fSongBeat ) 
{
	float fColOffsetFromCenter = iColNum - (GAMEMAN->GetCurrentStyleDef()->m_iColsPerPlayer-1)/2.0f;
	float fPixelOffsetFromCenter = fColOffsetFromCenter * ARROW_SIZE;
	
	// BUG OR FEATURE??? THIS IS WHERE THE REAL COLUMN PLACEMENT HAPPENS!!!
	// GAMEMANAGER SITS AROUND ON ITS ASS DOING NOTHING
	// 
	// As I know very little about this system, and as this is the only place I can think of that I
	// can possibly change my arrow placements, Ez2dancer column setups will be here until
	// somebody makes some other system. In the meantime, if it works, i'm using it.

	if ( GAMEMAN->m_CurGame == GAME_EZ2 )
	{
		fPixelOffsetFromCenter = fColOffsetFromCenter * ARROW_SIZE / 1.3f;
		if ( GAMEMAN->m_CurStyle == STYLE_EZ2_REAL || GAMEMAN->m_CurStyle == STYLE_EZ2_REAL_VERSUS ) // real gets MEGA squashed
		{
			fPixelOffsetFromCenter = fColOffsetFromCenter * ARROW_SIZE / 1.6f;
		}
		else if ( GAMEMAN->m_CurStyle == STYLE_EZ2_DOUBLE && GAMEMAN->m_sMasterPlayerNumber == PLAYER_1)
		{
			fPixelOffsetFromCenter = (fColOffsetFromCenter + 2.9f) * ARROW_SIZE / 1.3f;
		}
		else if ( GAMEMAN->m_CurStyle == STYLE_EZ2_DOUBLE && GAMEMAN->m_sMasterPlayerNumber == PLAYER_2)
		{
			fPixelOffsetFromCenter = (fColOffsetFromCenter - 3.1f) * ARROW_SIZE / 1.3f;
		}
	}
	
	switch( po.m_EffectType )
	{
	case PlayerOptions::EFFECT_DRUNK:
		fPixelOffsetFromCenter += cosf( TIMER->GetTimeSinceStart()/4 + iColNum*0.4f + fYOffset/SCREEN_HEIGHT*4) * ARROW_SIZE/3; 
		break;
	}
	return fPixelOffsetFromCenter;
}

float ArrowGetRotation( const PlayerOptions& po, int iColNum, float fYOffset ) 
{
	float fRotation = 0; //StyleDef.m_ColumnToRotation[iColNum];

	switch( po.m_EffectType )
	{
	case PlayerOptions::EFFECT_DIZZY:
		fRotation += fYOffset/SCREEN_HEIGHT*6; 
		break;
	}
	
	return fRotation;
}

float ArrowGetYPos( const PlayerOptions& po, float fYOffset )
{
	return fYOffset * po.m_fArrowScrollSpeed * (po.m_bReverseScroll ? -1 : 1 );
}

float ArrowGetAlpha( const PlayerOptions& po, float fYPos )
{
	float fAlpha;
	switch( po.m_AppearanceType )
	{ 
	case PlayerOptions::APPEARANCE_VISIBLE:
		fAlpha = 1;
		break;
	case PlayerOptions::APPEARANCE_HIDDEN:
		fAlpha = (fYPos-100)/200;
		break;
	case PlayerOptions::APPEARANCE_SUDDEN:
		fAlpha = ((SCREEN_HEIGHT-fYPos)-260)/200;
		break;
	case PlayerOptions::APPEARANCE_STEALTH:
		fAlpha = 0;
		break;
	default:
		ASSERT( false );
		fAlpha = 0;
	};
	if( fYPos < 0 )
		fAlpha = 1;

	return fAlpha;
};


