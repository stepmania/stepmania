#include "global.h"
#include "ArrowEffects.h"
#include "Steps.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "NoteDisplay.h"
#include "song.h"
#include "RageMath.h"

const float ARROW_SPACING	= ARROW_SIZE;// + 2;

float		g_fExpandSeconds = 0;

static float GetNoteFieldHeight( PlayerNumber pn )
{
	return SCREEN_HEIGHT + fabsf(GAMESTATE->m_CurrentPlayerOptions[pn].m_fPerspectiveTilt)*200;
}

float ArrowGetYOffset( PlayerNumber pn, int iCol, float fNoteBeat )
{
	float fYOffset = 0;

	/* Usually, fTimeSpacing is 0 or 1, in which case we use entirely beat spacing or
	 * entirely time spacing (respectively).  Occasionally, we tween between them. */
	if( GAMESTATE->m_CurrentPlayerOptions[pn].m_fTimeSpacing != 1.0f )
	{
		float fSongBeat = GAMESTATE->m_fSongBeat;
		float fBeatsUntilStep = fNoteBeat - fSongBeat;
		float fYOffsetBeatSpacing = fBeatsUntilStep * ARROW_SPACING;
		fYOffset += fYOffsetBeatSpacing * (1-GAMESTATE->m_CurrentPlayerOptions[pn].m_fTimeSpacing);
	}

	if( GAMESTATE->m_CurrentPlayerOptions[pn].m_fTimeSpacing != 0.0f )
	{
		float fSongSeconds = GAMESTATE->m_fMusicSeconds;
		float fNoteSeconds = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(fNoteBeat);
		float fSecondsUntilStep = fNoteSeconds - fSongSeconds;
		float fBPM = GAMESTATE->m_CurrentPlayerOptions[pn].m_fScrollBPM;
		float fBPS = fBPM/60.f;
		float fYOffsetTimeSpacing = fSecondsUntilStep * fBPS * ARROW_SPACING;
		fYOffset += fYOffsetTimeSpacing * GAMESTATE->m_CurrentPlayerOptions[pn].m_fTimeSpacing;
	}

	// don't mess with the arrows after they've crossed 0
	if( fYOffset < 0 )
		return fYOffset * GAMESTATE->m_CurrentPlayerOptions[pn].m_fScrollSpeed;

	const float* fAccels = GAMESTATE->m_CurrentPlayerOptions[pn].m_fAccels;
	//const float* fEffects = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects;


	float fYAdjust = 0;	// fill this in depending on PlayerOptions

	if( fAccels[PlayerOptions::ACCEL_BOOST] > 0 )
	{
		float fEffectHeight = GetNoteFieldHeight(pn);
		float fNewYOffset = fYOffset * 1.5f / ((fYOffset+fEffectHeight/1.2f)/fEffectHeight); 
		float fAccelYAdjust =	fAccels[PlayerOptions::ACCEL_BOOST] * (fNewYOffset - fYOffset);
		// TRICKY:	Clamp this value, or else BOOST+BOOMERANG will draw a ton of arrows on the screen.
		CLAMP( fAccelYAdjust, -400.f, 400.f );
		fYAdjust += fAccelYAdjust;
	}
	if( fAccels[PlayerOptions::ACCEL_BRAKE] > 0 )
	{
		float fEffectHeight = GetNoteFieldHeight(pn);
		float fScale = SCALE( fYOffset, 0.f, fEffectHeight, 0, 1.f );
		float fNewYOffset = fYOffset * fScale; 
		float fBrakeYAdjust = fAccels[PlayerOptions::ACCEL_BRAKE] * (fNewYOffset - fYOffset);
		// TRICKY:	Clamp this value the same way as BOOST so that in BOOST+BRAKE, BRAKE doesn't overpower BOOST
		CLAMP( fBrakeYAdjust, -400.f, 400.f );
		fYAdjust += fBrakeYAdjust;
	}
	if( fAccels[PlayerOptions::ACCEL_WAVE] > 0 )
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_WAVE] * 20.0f*sinf( fYOffset/38.0f );

	fYOffset += fYAdjust;

	if( fAccels[PlayerOptions::ACCEL_BOOMERANG] > 0 )
		fYOffset +=	fAccels[PlayerOptions::ACCEL_BOOMERANG] * (fYOffset * SCALE( fYOffset, 0.f, SCREEN_HEIGHT, 1.5f, 0.5f )- fYOffset);

	float fScrollSpeed = GAMESTATE->m_CurrentPlayerOptions[pn].m_fScrollSpeed;

	if( fAccels[PlayerOptions::ACCEL_EXPAND] > 0 )
	{
		/* Timers can't be global, since they'll be initialized before SDL. */
		static RageTimer timerExpand;
		if( !GAMESTATE->m_bFreeze )
			g_fExpandSeconds += timerExpand.GetDeltaTime();
		else
			timerExpand.GetDeltaTime();	// throw away
		float fExpandMultiplier = SCALE( cosf(g_fExpandSeconds*3), -1, 1, 0.5f, 1.5f );
		fScrollSpeed *=	SCALE( fAccels[PlayerOptions::ACCEL_EXPAND], 0.f, 1.f, 1.f, fExpandMultiplier );
	}

	fYOffset *= fScrollSpeed;

	return fYOffset;
}


static void ArrowGetReverseShiftAndScale( PlayerNumber pn, int iCol, float fYReverseOffsetPixels, float &fShiftOut, float &fScaleOut )
{
	/* XXX: Hack: we need to scale the reverse shift by the zoom. */
	float fMiniPercent = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects[PlayerOptions::EFFECT_MINI];
	float fZoom = 1 - fMiniPercent*0.5f;

	float fPercentReverse = GAMESTATE->m_CurrentPlayerOptions[pn].GetReversePercentForColumn(iCol);
	fShiftOut = SCALE( fPercentReverse, 0.f, 1.f, -fYReverseOffsetPixels/fZoom/2, fYReverseOffsetPixels/fZoom/2 );
	float fPercentCentered = GAMESTATE->m_CurrentPlayerOptions[pn].m_fScrolls[PlayerOptions::SCROLL_CENTERED];
	fShiftOut = SCALE( fPercentCentered, 0.f, 1.f, fShiftOut, 0.5f );

	fScaleOut = SCALE( fPercentReverse, 0.f, 1.f, 1.f, -1.f);
}

float ArrowGetYPos( PlayerNumber pn, int iCol, float fYOffset, float fYReverseOffsetPixels, bool WithReverse )
{
	float f = fYOffset;

	if( WithReverse )
	{
		float fShift, fScale;
		ArrowGetReverseShiftAndScale( pn, iCol, fYReverseOffsetPixels, fShift, fScale );

		f *= fScale;
		f += fShift;
	}

	const float* fEffects = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects;

	if( fEffects[PlayerOptions::EFFECT_TIPSY] > 0 )
		f += fEffects[PlayerOptions::EFFECT_TIPSY] * ( cosf( RageTimer::GetTimeSinceStart()*1.2f + iCol*1.8f) * ARROW_SIZE*0.4f );
	return f;
}

float ArrowGetYOffsetFromYPos( PlayerNumber pn, int iCol, float YPos, float fYReverseOffsetPixels )
{
	float f = YPos;

	const float* fEffects = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects;
	if( fEffects[PlayerOptions::EFFECT_TIPSY] > 0 )
		f -= fEffects[PlayerOptions::EFFECT_TIPSY] * ( cosf( RageTimer::GetTimeSinceStart()*1.2f + iCol*2.f) * ARROW_SIZE*0.4f );

	float fShift, fScale;
	ArrowGetReverseShiftAndScale( pn, iCol, fYReverseOffsetPixels, fShift, fScale );

	f -= fShift;
	if( fScale )
		f /= fScale;

	return f;
}

float ArrowGetXPos( PlayerNumber pn, int iColNum, float fYOffset ) 
{
	float fPixelOffsetFromCenter = 0;
	
	const float* fEffects = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects;

	if( fEffects[PlayerOptions::EFFECT_TORNADO] > 0 )
	{
		const Style* pStyle = GAMESTATE->GetCurrentStyle();

		// TRICKY: Tornado is very unplayable in doubles, so use a smaller
		// tornado width if there are many columns
		bool bWideField = pStyle->m_iColsPerPlayer > 4;
		int iTornadoWidth = bWideField ? 2 : 3;

		int iStartCol = iColNum - iTornadoWidth;
		int iEndCol = iColNum + iTornadoWidth;
		CLAMP( iStartCol, 0, pStyle->m_iColsPerPlayer-1 );
		CLAMP( iEndCol, 0, pStyle->m_iColsPerPlayer-1 );

		float fMinX = +100000;
		float fMaxX = -100000;
		for( int i=iStartCol; i<=iEndCol; i++ )
		{
			fMinX = min( fMinX, pStyle->m_ColumnInfo[pn][i].fXOffset );
			fMaxX = max( fMaxX, pStyle->m_ColumnInfo[pn][i].fXOffset );
		}

		const float fRealPixelOffset = GAMESTATE->GetCurrentStyle()->m_ColumnInfo[pn][iColNum].fXOffset;
		const float fPositionBetween = SCALE( fRealPixelOffset, fMinX, fMaxX, -1, 1 );
		float fRads = acosf( fPositionBetween );
		fRads += fYOffset * 6 / SCREEN_HEIGHT;
		
		const float fAdjustedPixelOffset = SCALE( cosf(fRads), -1, 1, fMinX, fMaxX );

		fPixelOffsetFromCenter += (fAdjustedPixelOffset - fRealPixelOffset) * fEffects[PlayerOptions::EFFECT_TORNADO];
	}

	if( fEffects[PlayerOptions::EFFECT_DRUNK] > 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_DRUNK] * ( cosf( RageTimer::GetTimeSinceStart() + iColNum*0.2f + fYOffset*10/SCREEN_HEIGHT) * ARROW_SIZE*0.5f );
	if( fEffects[PlayerOptions::EFFECT_FLIP] > 0 )
	{
//		fPixelOffsetFromCenter *= SCALE(fEffects[PlayerOptions::EFFECT_FLIP], 0.f, 1.f, 1.f, -1.f);
		const float fRealPixelOffset = GAMESTATE->GetCurrentStyle()->m_ColumnInfo[pn][iColNum].fXOffset;
		const float fDistance = -fRealPixelOffset * 2;
		fPixelOffsetFromCenter += fDistance * fEffects[PlayerOptions::EFFECT_FLIP];
	}

	if( fEffects[PlayerOptions::EFFECT_BEAT] > 0 )
	do {
		float fAccelTime = 0.2f, fTotalTime = 0.5f;
		
		/* If the song is really fast, slow down the rate, but speed up the
		 * acceleration to compensate or it'll look weird. */
		const float fBPM = GAMESTATE->m_fCurBPS * 60;
		const float fDiv = max(1.0f, truncf( fBPM / 150.0f ));
		fAccelTime /= fDiv;
		fTotalTime /= fDiv;

		float fBeat = GAMESTATE->m_fSongBeat + fAccelTime;
		fBeat /= fDiv;

		const bool bEvenBeat = ( int(fBeat) % 2 ) != 0;

		/* -100.2 -> -0.2 -> 0.2 */
		if( fBeat < 0 )
			break;

		fBeat -= truncf( fBeat );
		fBeat += 1;
		fBeat -= truncf( fBeat );

		if( fBeat >= fTotalTime )
			break;

		float fAmount;
		if( fBeat < fAccelTime )
		{
			fAmount = SCALE( fBeat, 0.0f, fAccelTime, 0.0f, 1.0f);
			fAmount *= fAmount;
		} else /* fBeat < fTotalTime */ {
			fAmount = SCALE( fBeat, fAccelTime, fTotalTime, 1.0f, 0.0f);
			fAmount = 1 - (1-fAmount) * (1-fAmount);
		}

		if( bEvenBeat )
			fAmount *= -1;

		const float fShift = 20.0f*fAmount*sinf( fYOffset / 15.0f + PI/2.0f );
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_BEAT] * fShift;
	} while(0);

	return fPixelOffsetFromCenter;
}

float ArrowGetRotation( PlayerNumber pn, float fNoteBeat ) 
{
	if( GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects[PlayerOptions::EFFECT_DIZZY] > 0 )
	{
		const float fSongBeat = GAMESTATE->m_fSongBeat;
		float fDizzyRotation = fNoteBeat - fSongBeat;
		fDizzyRotation *= GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects[PlayerOptions::EFFECT_DIZZY];
		fDizzyRotation = fmodf( fDizzyRotation, 2*PI );
		fDizzyRotation *= 180/PI;
		return fDizzyRotation;
	}
	else
		return 0;
}


#define CENTER_LINE_Y 160	// from fYOffset == 0
#define FADE_DIST_Y 40

static float GetCenterLine( PlayerNumber pn )
{
	const float fMiniPercent = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects[PlayerOptions::EFFECT_MINI];
	const float fZoom = 1 - fMiniPercent*0.5f;

	/* Another mini hack: if EFFECT_MINI is on, then our center line is at eg. 320, 
	 * not 160. */
	return CENTER_LINE_Y / fZoom;
}

static float GetHiddenSudden( PlayerNumber pn ) 
{
	const float* fAppearances = GAMESTATE->m_CurrentPlayerOptions[pn].m_fAppearances;
	return fAppearances[PlayerOptions::APPEARANCE_HIDDEN] *
		fAppearances[PlayerOptions::APPEARANCE_SUDDEN];
}

//
//  -gray arrows-
// 
//  ...invisible...
//  -hidden end line-
//  -hidden start line-
//  ...visible...
//  -sudden end line-
//  -sudden start line-
//  ...invisible...
//
// TRICKY:  We fudge hidden and sudden to be farther apart if they're both on.
static float GetHiddenEndLine( PlayerNumber pn )
{
	return GetCenterLine( pn ) + FADE_DIST_Y * SCALE( GetHiddenSudden(pn), 0.f, 1.f, -1.0f, -1.25f );
}

static float GetHiddenStartLine( PlayerNumber pn )
{
	return GetCenterLine( pn ) + FADE_DIST_Y * SCALE( GetHiddenSudden(pn), 0.f, 1.f, +0.0f, -0.25f );
}

static float GetSuddenEndLine( PlayerNumber pn )
{
	return GetCenterLine( pn ) + FADE_DIST_Y * SCALE( GetHiddenSudden(pn), 0.f, 1.f, -0.0f, +0.25f );
}

static float GetSuddenStartLine( PlayerNumber pn )
{
	return GetCenterLine( pn ) + FADE_DIST_Y * SCALE( GetHiddenSudden(pn), 0.f, 1.f, +1.0f, +1.25f );
}

// used by ArrowGetAlpha and ArrowGetGlow below
static float ArrowGetPercentVisible( PlayerNumber pn, int iCol, float fYOffset, float fYReverseOffsetPixels )
{
	/* Get the YPos without reverse (that is, factor in EFFECT_TIPSY). */
	float fYPos = ArrowGetYPos( pn, iCol, fYOffset, fYReverseOffsetPixels, false );

	const float fDistFromCenterLine = fYPos - GetCenterLine( pn );

	if( fYPos < 0 )	// past Gray Arrows
		return 1;	// totally visible

	const float* fAppearances = GAMESTATE->m_CurrentPlayerOptions[pn].m_fAppearances;

	float fVisibleAdjust = 0;

	if( fAppearances[PlayerOptions::APPEARANCE_HIDDEN] > 0 )
	{
		float fHiddenVisibleAdjust = SCALE( fYPos, GetHiddenStartLine(pn), GetHiddenEndLine(pn), 0, -1 );
		CLAMP( fHiddenVisibleAdjust, -1, 0 );
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_HIDDEN] * fHiddenVisibleAdjust;
	}
	if( fAppearances[PlayerOptions::APPEARANCE_SUDDEN] > 0 )
	{
		float fSuddenVisibleAdjust = SCALE( fYPos, GetSuddenStartLine(pn), GetSuddenEndLine(pn), -1, 0 );
		CLAMP( fSuddenVisibleAdjust, -1, 0 );
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_SUDDEN] * fSuddenVisibleAdjust;
	}

	if( fAppearances[PlayerOptions::APPEARANCE_STEALTH] > 0 )
		fVisibleAdjust -= fAppearances[PlayerOptions::APPEARANCE_STEALTH];
	if( fAppearances[PlayerOptions::APPEARANCE_BLINK] > 0 )
	{
		float f = sinf(RageTimer::GetTimeSinceStart()*10);
        f = froundf( f, 0.3333f );
		fVisibleAdjust += SCALE( f, 0, 1, -1, 0 );
	}
	if( fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH] > 0)
	{
		const float fRealFadeDist = 80;
		fVisibleAdjust += SCALE( fabsf(fDistFromCenterLine), fRealFadeDist, 2*fRealFadeDist, -1, 0 )
			* fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH];
	}

	return clamp( 1+fVisibleAdjust, 0, 1 );
}

float ArrowGetAlpha( PlayerNumber pn, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels )
{
	float fPercentVisible = ArrowGetPercentVisible(pn,iCol,fYOffset,fYReverseOffsetPixels);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	return (fPercentVisible>0.5f) ? 1.0f : 0.0f;
}

float ArrowGetGlow( PlayerNumber pn, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels )
{
	float fPercentVisible = ArrowGetPercentVisible(pn,iCol,fYOffset,fYReverseOffsetPixels);

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


float ArrowGetZPos( PlayerNumber pn, int iCol, float fYOffset )
{
	float fZPos=0;
	const float* fEffects = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects;

	if( fEffects[PlayerOptions::EFFECT_BUMPY] > 0 )
		fZPos += fEffects[PlayerOptions::EFFECT_BUMPY] * 40*sinf( fYOffset/16.0f );

	return fZPos;
}

bool ArrowsNeedZBuffer( PlayerNumber pn )
{
	const float* fEffects = GAMESTATE->m_CurrentPlayerOptions[pn].m_fEffects;
	if( fEffects[PlayerOptions::EFFECT_BUMPY] > 0 )
		return true;

	return false;
}

float ArrowGetZoom( PlayerNumber pn )
{
	// FIXME: Move the zoom values into Style
	if( GAMESTATE->m_pCurStyle->m_bNeedsZoomOutWith2Players &&
		(GAMESTATE->GetNumSidesJoined()==2 || GAMESTATE->AnyPlayersAreCpu()) )
		return 0.6f;
	return 1.0f;
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
