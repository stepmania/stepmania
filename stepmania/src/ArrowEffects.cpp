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
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "NoteDisplay.h"


float ArrowGetYOffset( PlayerNumber pn, float fNoteBeat )
{
	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fBeatsUntilStep = fNoteBeat - fSongBeat;
	float fYOffset = fBeatsUntilStep * ARROW_GAP;

	/* With both boost and wave enabled, the effect is that the
	 * notes "bufferr" at the bottom of the screen and shoot out
	 * at high speed.
	 *
	 * With wave first, the boost appears halfway down the screen.
	 * With boost first, it appears about 1/4 down the screen.
	 *
	 * I'm not sure which is better. - glenn
	 */
	int EffectType = GAMESTATE->m_PlayerOptions[pn].m_EffectType;
	if ( EffectType & PlayerOptions::EFFECT_BOOST )
		fYOffset *= 1.4f / ((fYOffset+SCREEN_HEIGHT/1.6f)/SCREEN_HEIGHT); 
	if ( EffectType & PlayerOptions::EFFECT_WAVE )
		fYOffset += 15.0f*sinf( fYOffset/38.0f ); 

	return fYOffset;
}

float ArrowGetXPos( PlayerNumber pn, int iColNum, float fYPos ) 
{
	float fPixelOffsetFromCenter = GAMESTATE->GetCurrentStyleDef()->m_ColumnInfo[pn][iColNum].fXOffset;
	
	if( GAMESTATE->m_PlayerOptions[pn].m_EffectType & PlayerOptions::EFFECT_DRUNK )
		fPixelOffsetFromCenter += cosf( TIMER->GetTimeSinceStart() + iColNum*0.2f + fYPos*6/SCREEN_HEIGHT) * ARROW_SIZE*0.5f; 
	if( GAMESTATE->m_PlayerOptions[pn].m_EffectType & PlayerOptions::EFFECT_FLIP )
		fPixelOffsetFromCenter = -fPixelOffsetFromCenter; 
	if( GAMESTATE->m_PlayerOptions[pn].m_EffectType & PlayerOptions::EFFECT_TORNADO )
	{
		const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
		float fMaxX = -100000, fMinX = +100000;
		for( int i=0; i<pStyleDef->m_iColsPerPlayer; i++ )
		{
			fMaxX = max( fMaxX, pStyleDef->m_ColumnInfo[pn][i].fXOffset );
			fMinX = min( fMinX, pStyleDef->m_ColumnInfo[pn][i].fXOffset );
		}

		float fPositionBetween = SCALE( fPixelOffsetFromCenter, fMinX, fMaxX, -1, 1 );
		float fRads = acosf( fPositionBetween );
		fRads += fYPos * 6 / SCREEN_HEIGHT;
		
		fPixelOffsetFromCenter = SCALE( cosf(fRads), -1, 1, fMinX, fMaxX );
	}

	return fPixelOffsetFromCenter;
}

float ArrowGetRotation( PlayerNumber pn, int iColNum, float fYOffset ) 
{
	float fRotation = 0; //StyleDef.m_ColumnToRotation[iColNum];

	if( GAMESTATE->m_PlayerOptions[pn].m_EffectType & PlayerOptions::EFFECT_DIZZY)
		fRotation += fYOffset/SCREEN_HEIGHT*6; 
	
	return fRotation;
}

float ArrowGetYPos( PlayerNumber pn, float fYOffset )
{
	return fYOffset * GAMESTATE->m_PlayerOptions[pn].m_fArrowScrollSpeed * (GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ? -1 : 1 );
}


const float fCenterLine = 160;	// from fYPos == 0
const float fFadeDist = 100;

// used by ArrowGetAlpha and ArrowGetGlow below
float ArrowGetPercentVisible( PlayerNumber pn, float fYPos )
{
	const bool bReverse = GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll;
	const float fCorrectedYPos = bReverse ? -fYPos : fYPos;
	const float fDistFromCenterLine = fCorrectedYPos - fCenterLine;
	float fAlpha;

	switch( GAMESTATE->m_PlayerOptions[pn].m_AppearanceType )
	{ 
	case PlayerOptions::APPEARANCE_VISIBLE:
		fAlpha = 1;
		break;
	case PlayerOptions::APPEARANCE_HIDDEN:
		fAlpha = SCALE( fDistFromCenterLine, 0, fFadeDist, 0, 1 );
		break;
	case PlayerOptions::APPEARANCE_SUDDEN:
		fAlpha = SCALE( fDistFromCenterLine, 0, -fFadeDist, 0, 1 );
		break;
	case PlayerOptions::APPEARANCE_STEALTH:
		fAlpha = 0;
		break;
	case PlayerOptions::APPEARANCE_BLINK: // this is an Ez2dancer Appearance Mode
		fAlpha = sinf( TIMER->GetTimeSinceStart()*12 );
		fAlpha = froundf( fAlpha, 0.3333f );
		break;
	default:
		ASSERT(0);
		fAlpha = 1;
	}

	if( fCorrectedYPos < 0 )	// past Gray Arrows
		fAlpha = 1;

	return clamp( fAlpha, 0, 1 );
}

float ArrowGetAlpha( PlayerNumber pn, float fYPos, float fPercentFadeToFail )
{
	float fPercentVisible = ArrowGetPercentVisible(pn,fYPos);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	return (fPercentVisible>0.5f) ? 1.0f : 0.0f;
}

float ArrowGetGlow( PlayerNumber pn, float fYPos, float fPercentFadeToFail )
{
	float fPercentVisible = ArrowGetPercentVisible(pn,fYPos);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	const float fDistFromHalf = fabsf( fPercentVisible - 0.5f );
	return SCALE( fDistFromHalf, 0, 0.5f, 1.3f, 0 );
}
