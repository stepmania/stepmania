#include "global.h"
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
#include <math.h>


RageTimer	g_timerExpand;
float		g_fExpandSeconds = 0;

float ArrowGetYOffset( PlayerNumber pn, float fNoteBeat )
{
	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fBeatsUntilStep = fNoteBeat - fSongBeat;
	float fYOffset = fBeatsUntilStep * ARROW_GAP;

	// don't mess with the arrows after they've crossed 0
	if( fYOffset < 0 )
		return fYOffset;

	const float* fAccels = GAMESTATE->m_CurrentPlayerOptions[pn].m_fAccels;


	float fYAdjust = 0;	// fill this in depending on PlayerOptions

	if( fAccels[PlayerOptions::ACCEL_BOOST] > 0 )
	{
		float fNewYOffset = fYOffset * 1.5f / ((fYOffset+SCREEN_HEIGHT/1.2f)/SCREEN_HEIGHT); 
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_BOOST] * (fNewYOffset - fYOffset);
	}
	if( fAccels[PlayerOptions::ACCEL_LAND] > 0 )
	{
		float fNewYOffset = fYOffset * SCALE( fYOffset, 0.f, SCREEN_HEIGHT, 0.25f, 1.5f ); 
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_LAND] * (fNewYOffset - fYOffset);
	}
	if( fAccels[PlayerOptions::ACCEL_WAVE] > 0 )
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_WAVE] * 20.0f*sinf( fYOffset/38.0f );
	if( fAccels[PlayerOptions::ACCEL_EXPAND] > 0 )
	{
		if( !GAMESTATE->m_bFreeze )
			g_fExpandSeconds += g_timerExpand.GetDeltaTime();
		else
			g_timerExpand.GetDeltaTime();	// throw away
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_EXPAND] * (fYOffset * SCALE( cosf(g_fExpandSeconds*3), -1, 1, 0.5f, 1.5f ) - fYOffset); 
	}
	if( fAccels[PlayerOptions::ACCEL_BOOMERANG] > 0 )
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_BOOMERANG] * (fYOffset * SCALE( fYOffset, 0.f, SCREEN_HEIGHT, 1.5f, 0.5f )- fYOffset);

	return fYOffset + fYAdjust;
}

float ArrowGetXPos( PlayerNumber pn, int iColNum, float fYPos ) 
{
	float fPixelOffsetFromCenter = GAMESTATE->GetCurrentStyleDef()->m_ColumnInfo[pn][iColNum].fXOffset;
	
	const float* fEffects = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects;

	if( fEffects[PlayerOptions::EFFECT_TORNADO] > 0 )
	{
		const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
		float fMinX, fMaxX;
		pStyleDef->GetMinAndMaxColX( pn, fMinX, fMaxX );

		float fPositionBetween = SCALE( fPixelOffsetFromCenter, fMinX, fMaxX, -1, 1 );
		float fRads = acosf( fPositionBetween );
		fRads += fYPos * 6 / SCREEN_HEIGHT;
		
		fPixelOffsetFromCenter = SCALE( cosf(fRads), -1, 1, fMinX, fMaxX );
	}

	if( fEffects[PlayerOptions::EFFECT_DRUNK] > 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_DRUNK] * ( cosf( RageTimer::GetTimeSinceStart() + iColNum*0.2f + fYPos*10/SCREEN_HEIGHT) * ARROW_SIZE*0.5f );
	if( fEffects[PlayerOptions::EFFECT_FLIP] > 0 )
		fPixelOffsetFromCenter *= SCALE(fEffects[PlayerOptions::EFFECT_FLIP], 0.f, 1.f, 1.f, -1.f);

	return fPixelOffsetFromCenter;
}

float ArrowGetRotation( PlayerNumber pn, float fNoteBeat ) 
{
	if( GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects[PlayerOptions::EFFECT_DIZZY] > 0 )
	{
		float fSongBeat = GAMESTATE->m_fSongBeat;
		float fDizzyRotation = fNoteBeat - fSongBeat;
		fDizzyRotation = fmodf( fDizzyRotation, 2*PI );
		return fDizzyRotation * GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects[PlayerOptions::EFFECT_DIZZY];
	}
	else
		return 0;
}

float ArrowGetYPosWithoutReverse( PlayerNumber pn, float fYOffset )
{
	return fYOffset * GAMESTATE->m_CurrentPlayerOptions[pn].m_fScrollSpeed;
}

float ArrowGetYPos( PlayerNumber pn, float fYOffset )
{
	return ArrowGetYPosWithoutReverse(pn,fYOffset) * SCALE( GAMESTATE->m_CurrentPlayerOptions[pn].m_fReverseScroll, 0.f, 1.f, 1.f, -1.f );
}


const float fCenterLine = 160;	// from fYPos == 0
const float fFadeDist = 100;

// used by ArrowGetAlpha and ArrowGetGlow below
float ArrowGetPercentVisible( PlayerNumber pn, float fYPosWithoutReverse )
{
	const float fDistFromCenterLine = fYPosWithoutReverse - fCenterLine;

	if( fYPosWithoutReverse < 0 )	// past Gray Arrows
		return 1;	// totally visible


	const float* fAppearances = GAMESTATE->m_CurrentPlayerOptions[pn].m_fAppearances;

	float fVisibleAdjust = 0;

	if( fAppearances[PlayerOptions::APPEARANCE_HIDDEN] > 0 )
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_HIDDEN] * SCALE( fDistFromCenterLine, 0, fFadeDist, -1, 0 );
	if( fAppearances[PlayerOptions::APPEARANCE_SUDDEN] > 0 )
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_SUDDEN] * SCALE( fDistFromCenterLine, 0, -fFadeDist, -1, 0 );
	if( fAppearances[PlayerOptions::APPEARANCE_STEALTH] > 0 )
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_STEALTH] * -1;
	if( fAppearances[PlayerOptions::APPEARANCE_BLINK] > 0 )
		fVisibleAdjust += clamp( sinf(RageTimer::GetTimeSinceStart()*10)-1, 0, 1 );

	return clamp( 1+fVisibleAdjust, 0, 1 );
}

float ArrowGetAlpha( PlayerNumber pn, float fYPos, float fPercentFadeToFail )
{
	fYPos /= GAMESTATE->m_CurrentPlayerOptions[pn].m_fScrollSpeed;
	float fPercentVisible = ArrowGetPercentVisible(pn,fYPos);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	return (fPercentVisible>0.5f) ? 1.0f : 0.0f;
}

float ArrowGetGlow( PlayerNumber pn, float fYPos, float fPercentFadeToFail )
{
	fYPos /= GAMESTATE->m_CurrentPlayerOptions[pn].m_fScrollSpeed;
	float fPercentVisible = ArrowGetPercentVisible(pn,fYPos);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	const float fDistFromHalf = fabsf( fPercentVisible - 0.5f );
	return SCALE( fDistFromHalf, 0, 0.5f, 1.3f, 0 );
}

float ArrowGetBrightness( PlayerNumber pn, float fNoteBeat )
{
	if( GAMESTATE->m_bEditing )
		return 1;

	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fBeatsUntilStep = fNoteBeat - fSongBeat;

	float fBrightness = SCALE( fBeatsUntilStep, 0, -1, 1.f, 0.f );
	CLAMP( fBrightness, 0, 1 );
	return fBrightness;
}

