#include "global.h"
#include "ArrowEffects.h"
#include "Steps.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "NoteDisplay.h"
#include "song.h"
#include "RageMath.h"
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "GameState.h"
#include "Style.h"
#include <float.h>

const float ARROW_SPACING	= ARROW_SIZE;// + 2;

static float GetNoteFieldHeight( const PlayerState* pPlayerState )
{
	return SCREEN_HEIGHT + fabsf(pPlayerState->m_CurrentPlayerOptions.m_fPerspectiveTilt)*200;
}

/* For visibility testing: if bAbsolute is false, random modifiers must return the
 * minimum possible scroll speed. */
float ArrowEffects::GetYOffset( const PlayerState* pPlayerState, int iCol, float fNoteBeat, float &fPeakYOffsetOut, bool &bIsPastPeakOut, bool bAbsolute )
{
	// Default values that are returned if boomerang is off.
	fPeakYOffsetOut = FLT_MAX;
	bIsPastPeakOut = true;


	float fYOffset = 0;

	/* Usually, fTimeSpacing is 0 or 1, in which case we use entirely beat spacing or
	 * entirely time spacing (respectively).  Occasionally, we tween between them. */
	if( pPlayerState->m_CurrentPlayerOptions.m_fTimeSpacing != 1.0f )
	{
		float fSongBeat = GAMESTATE->m_fSongBeat;
		float fBeatsUntilStep = fNoteBeat - fSongBeat;
		float fYOffsetBeatSpacing = fBeatsUntilStep * ARROW_SPACING;
		fYOffset += fYOffsetBeatSpacing * (1-pPlayerState->m_CurrentPlayerOptions.m_fTimeSpacing);
	}

	if( pPlayerState->m_CurrentPlayerOptions.m_fTimeSpacing != 0.0f )
	{
		float fSongSeconds = GAMESTATE->m_fMusicSeconds;
		float fNoteSeconds = GAMESTATE->m_pCurSong->GetElapsedTimeFromBeat(fNoteBeat);
		float fSecondsUntilStep = fNoteSeconds - fSongSeconds;
		float fBPM = pPlayerState->m_CurrentPlayerOptions.m_fScrollBPM;
		float fBPS = fBPM/60.f;
		float fYOffsetTimeSpacing = fSecondsUntilStep * fBPS * ARROW_SPACING;
		fYOffset += fYOffsetTimeSpacing * pPlayerState->m_CurrentPlayerOptions.m_fTimeSpacing;
	}

	// don't mess with the arrows after they've crossed 0
	if( fYOffset < 0 )
		return fYOffset * pPlayerState->m_CurrentPlayerOptions.m_fScrollSpeed;

	const float* fAccels = pPlayerState->m_CurrentPlayerOptions.m_fAccels;
	//const float* fEffects = pPlayerState->m_CurrentPlayerOptions.m_fEffects;


	float fYAdjust = 0;	// fill this in depending on PlayerOptions

	if( fAccels[PlayerOptions::ACCEL_BOOST] != 0 )
	{
		float fEffectHeight = GetNoteFieldHeight(pPlayerState);
		float fNewYOffset = fYOffset * 1.5f / ((fYOffset+fEffectHeight/1.2f)/fEffectHeight); 
		float fAccelYAdjust =	fAccels[PlayerOptions::ACCEL_BOOST] * (fNewYOffset - fYOffset);
		// TRICKY:	Clamp this value, or else BOOST+BOOMERANG will draw a ton of arrows on the screen.
		CLAMP( fAccelYAdjust, -400.f, 400.f );
		fYAdjust += fAccelYAdjust;
	}
	if( fAccels[PlayerOptions::ACCEL_BRAKE] != 0 )
	{
		float fEffectHeight = GetNoteFieldHeight(pPlayerState);
		float fScale = SCALE( fYOffset, 0.f, fEffectHeight, 0, 1.f );
		float fNewYOffset = fYOffset * fScale; 
		float fBrakeYAdjust = fAccels[PlayerOptions::ACCEL_BRAKE] * (fNewYOffset - fYOffset);
		// TRICKY:	Clamp this value the same way as BOOST so that in BOOST+BRAKE, BRAKE doesn't overpower BOOST
		CLAMP( fBrakeYAdjust, -400.f, 400.f );
		fYAdjust += fBrakeYAdjust;
	}
	if( fAccels[PlayerOptions::ACCEL_WAVE] != 0 )
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_WAVE] * 20.0f*RageFastSin( fYOffset/38.0f );

	fYOffset += fYAdjust;

	//
	// Factor in boomerang
	//
	if( fAccels[PlayerOptions::ACCEL_BOOMERANG] != 0 )
	{
		float fOriginalYOffset = fYOffset;

		fYOffset = (-1*fOriginalYOffset*fOriginalYOffset/SCREEN_HEIGHT) + 1.5f*fOriginalYOffset;
		float fPeakAtYOffset = SCREEN_HEIGHT * 0.75f;	// zero point of function above
		fPeakYOffsetOut = (-1*fPeakAtYOffset*fPeakAtYOffset/SCREEN_HEIGHT) + 1.5f*fPeakAtYOffset;
		bIsPastPeakOut = fOriginalYOffset < fPeakAtYOffset;
	}

	//
	// Factor in scroll speed
	//
	float fScrollSpeed = pPlayerState->m_CurrentPlayerOptions.m_fScrollSpeed;
	if( pPlayerState->m_CurrentPlayerOptions.m_fRandomSpeed > 0 && !bAbsolute )
	{
		int seed = GAMESTATE->m_iStageSeed + ( BeatToNoteRow( fNoteBeat ) << 8 ) + (iCol * 100);

		/* Temporary hack: the first call to RandomFloat isn't "random"; it takes an extra
		 * call to get the RNG rolling. */
		RandomFloat( seed );
		float fRandom = RandomFloat( seed );

		/* Random speed always increases speed: a random speed of 10 indicates [1,11].
		 * This keeps it consistent with other mods: 0 means no effect. */
		fScrollSpeed *=
				SCALE( fRandom,
						0.0f, 1.0f,
						1.0f, pPlayerState->m_CurrentPlayerOptions.m_fRandomSpeed + 1.0f );
	}	


	if( fAccels[PlayerOptions::ACCEL_EXPAND] != 0 )
	{
		static float fExpandSeconds = 0;
		static float fLastTime = 0;
		float fTime = RageTimer::GetTimeSinceStartFast();
		if( !GAMESTATE->m_bFreeze )
		{
			fExpandSeconds += fTime - fLastTime;
			fExpandSeconds = fmodf( fExpandSeconds, PI*2 );
		}
		fLastTime = fTime;

		float fExpandMultiplier = SCALE( RageFastCos(fExpandSeconds*3), -1, 1, 0.75f, 1.75f );
		fScrollSpeed *=	SCALE( fAccels[PlayerOptions::ACCEL_EXPAND], 0.f, 1.f, 1.f, fExpandMultiplier );
	}

	fYOffset *= fScrollSpeed;
	fPeakYOffsetOut *= fScrollSpeed;

	return fYOffset;
}

void ArrowGetReverseShiftAndScale( const PlayerState* pPlayerState, int iCol, float fYReverseOffsetPixels, float &fShiftOut, float &fScaleOut )
{
	float fPercentReverse = pPlayerState->m_CurrentPlayerOptions.GetReversePercentForColumn(iCol);
	fShiftOut = SCALE( fPercentReverse, 0.f, 1.f, -fYReverseOffsetPixels/2, fYReverseOffsetPixels/2 );
	float fPercentCentered = pPlayerState->m_CurrentPlayerOptions.m_fScrolls[PlayerOptions::SCROLL_CENTERED];
	fShiftOut = SCALE( fPercentCentered, 0.f, 1.f, fShiftOut, 0.5f );

	fScaleOut = SCALE( fPercentReverse, 0.f, 1.f, 1.f, -1.f);
}

float ArrowEffects::GetYPos( const PlayerState* pPlayerState, int iCol, float fYOffset, float fYReverseOffsetPixels, bool WithReverse )
{
	float f = fYOffset;

	if( WithReverse )
	{
		float fShift, fScale;
		ArrowGetReverseShiftAndScale( pPlayerState, iCol, fYReverseOffsetPixels, fShift, fScale );

		f *= fScale;
		f += fShift;
	}

	const float* fEffects = pPlayerState->m_CurrentPlayerOptions.m_fEffects;

	if( fEffects[PlayerOptions::EFFECT_TIPSY] != 0 )
		f += fEffects[PlayerOptions::EFFECT_TIPSY] * ( RageFastCos( RageTimer::GetTimeSinceStartFast()*1.2f + iCol*1.8f) * ARROW_SIZE*0.4f );
	return f;
}

float ArrowEffects::GetYOffsetFromYPos( const PlayerState* pPlayerState, int iCol, float YPos, float fYReverseOffsetPixels )
{
	float f = YPos;

	const float* fEffects = pPlayerState->m_CurrentPlayerOptions.m_fEffects;
	if( fEffects[PlayerOptions::EFFECT_TIPSY] != 0 )
		f -= fEffects[PlayerOptions::EFFECT_TIPSY] * ( RageFastCos( RageTimer::GetTimeSinceStartFast()*1.2f + iCol*2.f) * ARROW_SIZE*0.4f );

	float fShift, fScale;
	ArrowGetReverseShiftAndScale( pPlayerState, iCol, fYReverseOffsetPixels, fShift, fScale );

	f -= fShift;
	if( fScale )
		f /= fScale;

	return f;
}

float ArrowEffects::GetXPos( const PlayerState* pPlayerState, int iColNum, float fYOffset ) 
{
	float fPixelOffsetFromCenter = 0;	// fill this in below
	
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	const float* fEffects = pPlayerState->m_CurrentPlayerOptions.m_fEffects;

	if( fEffects[PlayerOptions::EFFECT_TORNADO] != 0 )
	{
		// TRICKY: Tornado is very unplayable in doubles, so use a smaller
		// tornado width if there are many columns
		bool bWideField = pStyle->m_iColsPerPlayer > 4;
		int iTornadoWidth = bWideField ? 2 : 3;

		int iStartCol = iColNum - iTornadoWidth;
		int iEndCol = iColNum + iTornadoWidth;
		CLAMP( iStartCol, 0, pStyle->m_iColsPerPlayer-1 );
		CLAMP( iEndCol, 0, pStyle->m_iColsPerPlayer-1 );

		float fMinX = FLT_MAX;
		float fMaxX = FLT_MIN;
		
		// TODO: Don't index by PlayerNumber.
		PlayerNumber pn = pPlayerState->m_PlayerNumber;

		for( int i=iStartCol; i<=iEndCol; i++ )
		{
			fMinX = min( fMinX, pStyle->m_ColumnInfo[pn][i].fXOffset );
			fMaxX = max( fMaxX, pStyle->m_ColumnInfo[pn][i].fXOffset );
		}

		const float fRealPixelOffset = pStyle->m_ColumnInfo[pn][iColNum].fXOffset;
		const float fPositionBetween = SCALE( fRealPixelOffset, fMinX, fMaxX, -1, 1 );
		float fRads = acosf( fPositionBetween );
		fRads += fYOffset * 6 / SCREEN_HEIGHT;
		
		const float fAdjustedPixelOffset = SCALE( RageFastCos(fRads), -1, 1, fMinX, fMaxX );

		fPixelOffsetFromCenter += (fAdjustedPixelOffset - fRealPixelOffset) * fEffects[PlayerOptions::EFFECT_TORNADO];
	}

	if( fEffects[PlayerOptions::EFFECT_DRUNK] != 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_DRUNK] * ( RageFastCos( RageTimer::GetTimeSinceStartFast() + iColNum*0.2f + fYOffset*10/SCREEN_HEIGHT) * ARROW_SIZE*0.5f );
	if( fEffects[PlayerOptions::EFFECT_FLIP] != 0 )
	{
		// TODO: Don't index by PlayerNumber.
		PlayerNumber pn = pPlayerState->m_PlayerNumber;

		const int iNumCols = pStyle->m_iColsPerPlayer;
		int iFirstCol = 0;
		int iLastCol = iNumCols-1;
		const int iNewCol = SCALE( iColNum, iFirstCol, iLastCol, iLastCol, iFirstCol );
		const float fOldPixelOffset = pStyle->m_ColumnInfo[pn][iColNum].fXOffset;
		const float fNewPixelOffset = pStyle->m_ColumnInfo[pn][iNewCol].fXOffset;
		const float fDistance = fNewPixelOffset - fOldPixelOffset;
		fPixelOffsetFromCenter += fDistance * fEffects[PlayerOptions::EFFECT_FLIP];
	}
	if( fEffects[PlayerOptions::EFFECT_INVERT] != 0 )
	{
		// TODO: Don't index by PlayerNumber.
		PlayerNumber pn = pPlayerState->m_PlayerNumber;
		
		const int iNumCols = pStyle->m_iColsPerPlayer;
		const int iNumSides = pStyle->m_StyleType==ONE_PLAYER_TWO_SIDES ? 2 : 1;
		const int iNumColsPerSide = iNumCols / iNumSides;
		const int iSideIndex = iColNum / iNumColsPerSide;
		const int iColOnSide = iColNum % iNumColsPerSide;

		const int iColLeftOfMiddle = (iNumColsPerSide-1)/2;
		const int iColRightOfMidde = (iNumColsPerSide+1)/2;

		int iFirstColOnSide = -1;
		int iLastColOnSide = -1;
		if( iColOnSide <= iColLeftOfMiddle )
		{
			iFirstColOnSide = 0;
			iLastColOnSide = iColLeftOfMiddle;
		}
		else if( iColOnSide >= iColRightOfMidde )
		{
			iFirstColOnSide = iColRightOfMidde;
			iLastColOnSide = iNumColsPerSide-1;
		}
		else
		{
			iFirstColOnSide = iColOnSide/2;
			iLastColOnSide = iColOnSide/2;
		}

		// mirror
		const int iNewColOnSide = SCALE( iColOnSide, iFirstColOnSide, iLastColOnSide, iLastColOnSide, iFirstColOnSide );
		const int iNewCol = iSideIndex*iNumColsPerSide + iNewColOnSide;

		const float fOldPixelOffset = pStyle->m_ColumnInfo[pn][iColNum].fXOffset;
		const float fNewPixelOffset = pStyle->m_ColumnInfo[pn][iNewCol].fXOffset;
		const float fDistance = fNewPixelOffset - fOldPixelOffset;
		fPixelOffsetFromCenter += fDistance * fEffects[PlayerOptions::EFFECT_INVERT];
	}

	if( fEffects[PlayerOptions::EFFECT_BEAT] != 0 )
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

		const float fShift = 20.0f*fAmount*RageFastSin( fYOffset / 15.0f + PI/2.0f );
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_BEAT] * fShift;
	} while(0);

	return fPixelOffsetFromCenter;
}

float ArrowEffects::GetRotation( const PlayerState* pPlayerState, float fNoteBeat ) 
{
	if( pPlayerState->m_CurrentPlayerOptions.m_fEffects[PlayerOptions::EFFECT_DIZZY] != 0 )
	{
		const float fSongBeat = GAMESTATE->m_fSongBeat;
		float fDizzyRotation = fNoteBeat - fSongBeat;
		fDizzyRotation *= pPlayerState->m_CurrentPlayerOptions.m_fEffects[PlayerOptions::EFFECT_DIZZY];
		fDizzyRotation = fmodf( fDizzyRotation, 2*PI );
		fDizzyRotation *= 180/PI;
		return fDizzyRotation;
	}
	else
		return 0;
}


#define CENTER_LINE_Y 160	// from fYOffset == 0
#define FADE_DIST_Y 40

static float GetCenterLine( const PlayerState* pPlayerState )
{
	return CENTER_LINE_Y;
}

static float GetHiddenSudden( const PlayerState* pPlayerState ) 
{
	const float* fAppearances = pPlayerState->m_CurrentPlayerOptions.m_fAppearances;
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
static float GetHiddenEndLine( const PlayerState* pPlayerState )
{
	return GetCenterLine( pPlayerState ) + 
		FADE_DIST_Y * SCALE( GetHiddenSudden(pPlayerState), 0.f, 1.f, -1.0f, -1.25f ) + 
		GetCenterLine( pPlayerState ) * pPlayerState->m_CurrentPlayerOptions.m_fAppearances[PlayerOptions::APPEARANCE_HIDDEN_OFFSET];
}

static float GetHiddenStartLine( const PlayerState* pPlayerState )
{
	return GetCenterLine( pPlayerState ) + 
		FADE_DIST_Y * SCALE( GetHiddenSudden(pPlayerState), 0.f, 1.f, +0.0f, -0.25f ) + 
		GetCenterLine( pPlayerState ) * pPlayerState->m_CurrentPlayerOptions.m_fAppearances[PlayerOptions::APPEARANCE_HIDDEN_OFFSET];
}

static float GetSuddenEndLine( const PlayerState* pPlayerState )
{
	return GetCenterLine( pPlayerState ) + 
		FADE_DIST_Y * SCALE( GetHiddenSudden(pPlayerState), 0.f, 1.f, -0.0f, +0.25f ) + 
		GetCenterLine( pPlayerState ) * pPlayerState->m_CurrentPlayerOptions.m_fAppearances[PlayerOptions::APPEARANCE_SUDDEN_OFFSET];
}

static float GetSuddenStartLine( const PlayerState* pPlayerState )
{
	return GetCenterLine( pPlayerState ) + 
		FADE_DIST_Y * SCALE( GetHiddenSudden(pPlayerState), 0.f, 1.f, +1.0f, +1.25f ) + 
		GetCenterLine( pPlayerState ) * pPlayerState->m_CurrentPlayerOptions.m_fAppearances[PlayerOptions::APPEARANCE_SUDDEN_OFFSET];
}

// used by ArrowGetAlpha and ArrowGetGlow below
float ArrowGetPercentVisible( const PlayerState* pPlayerState, int iCol, float fYOffset, float fYReverseOffsetPixels )
{
	/* Get the YPos without reverse (that is, factor in EFFECT_TIPSY). */
	float fYPos = ArrowEffects::GetYPos( pPlayerState, iCol, fYOffset, fYReverseOffsetPixels, false );

	const float fDistFromCenterLine = fYPos - GetCenterLine( pPlayerState );

	if( fYPos < 0 )	// past Gray Arrows
		return 1;	// totally visible

	const float* fAppearances = pPlayerState->m_CurrentPlayerOptions.m_fAppearances;

	float fVisibleAdjust = 0;

	if( fAppearances[PlayerOptions::APPEARANCE_HIDDEN] != 0 )
	{
		float fHiddenVisibleAdjust = SCALE( fYPos, GetHiddenStartLine(pPlayerState), GetHiddenEndLine(pPlayerState), 0, -1 );
		CLAMP( fHiddenVisibleAdjust, -1, 0 );
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_HIDDEN] * fHiddenVisibleAdjust;
	}
	if( fAppearances[PlayerOptions::APPEARANCE_SUDDEN] != 0 )
	{
		float fSuddenVisibleAdjust = SCALE( fYPos, GetSuddenStartLine(pPlayerState), GetSuddenEndLine(pPlayerState), -1, 0 );
		CLAMP( fSuddenVisibleAdjust, -1, 0 );
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_SUDDEN] * fSuddenVisibleAdjust;
	}

	if( fAppearances[PlayerOptions::APPEARANCE_STEALTH] != 0 )
		fVisibleAdjust -= fAppearances[PlayerOptions::APPEARANCE_STEALTH];
	if( fAppearances[PlayerOptions::APPEARANCE_BLINK] != 0 )
	{
		float f = RageFastSin(RageTimer::GetTimeSinceStartFast()*10);
		f = Quantize( f, 0.3333f );
		fVisibleAdjust += SCALE( f, 0, 1, -1, 0 );
	}
	if( fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH] != 0 )
	{
		const float fRealFadeDist = 80;
		fVisibleAdjust += SCALE( fabsf(fDistFromCenterLine), fRealFadeDist, 2*fRealFadeDist, -1, 0 )
			* fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH];
	}

	return clamp( 1+fVisibleAdjust, 0, 1 );
}

float ArrowEffects::GetAlpha( const PlayerState* pPlayerState, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels )
{
	float fPercentVisible = ArrowGetPercentVisible(pPlayerState,iCol,fYOffset,fYReverseOffsetPixels);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	return (fPercentVisible>0.5f) ? 1.0f : 0.0f;
}

float ArrowEffects::GetGlow( const PlayerState* pPlayerState, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels )
{
	float fPercentVisible = ArrowGetPercentVisible(pPlayerState,iCol,fYOffset,fYReverseOffsetPixels);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	const float fDistFromHalf = fabsf( fPercentVisible - 0.5f );
	return SCALE( fDistFromHalf, 0, 0.5f, 1.3f, 0 );
}

float ArrowEffects::GetBrightness( const PlayerState* pPlayerState, float fNoteBeat )
{
	if( GAMESTATE->IsEditing() )
		return 1;

	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fBeatsUntilStep = fNoteBeat - fSongBeat;

	float fBrightness = SCALE( fBeatsUntilStep, 0, -1, 1.f, 0.f );
	CLAMP( fBrightness, 0, 1 );
	return fBrightness;
}


float ArrowEffects::GetZPos( const PlayerState* pPlayerState, int iCol, float fYOffset )
{
	float fZPos=0;
	const float* fEffects = pPlayerState->m_CurrentPlayerOptions.m_fEffects;

	if( fEffects[PlayerOptions::EFFECT_BUMPY] != 0 )
		fZPos += fEffects[PlayerOptions::EFFECT_BUMPY] * 40*RageFastSin( fYOffset/16.0f );

	return fZPos;
}

bool ArrowEffects::NeedZBuffer( const PlayerState* pPlayerState )
{
	const float* fEffects = pPlayerState->m_CurrentPlayerOptions.m_fEffects;
	if( fEffects[PlayerOptions::EFFECT_BUMPY] != 0 )
		return true;

	return false;
}

float ArrowEffects::GetZoom( const PlayerState* pPlayerState )
{
	float fZoom = 1.0f;
	// FIXME: Move the zoom values into Style
	if( GAMESTATE->m_pCurStyle->m_bNeedsZoomOutWith2Players &&
		(GAMESTATE->GetNumSidesJoined()==2 || GAMESTATE->AnyPlayersAreCpu()) )
		fZoom *= 0.6f;

	float fMiniPercent = pPlayerState->m_CurrentPlayerOptions.m_fEffects[PlayerOptions::EFFECT_MINI];
	if( fMiniPercent != 0 )
	{
		fMiniPercent = powf( 0.5f, fMiniPercent );
		fZoom *= fMiniPercent;
	}
	return fZoom;
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
