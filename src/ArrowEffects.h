#ifndef ARROWEFFECTS_H
#define ARROWEFFECTS_H

#include "RageTypes.h"

class PlayerState;
class PlayerOptions;
/** @brief Functions that return properties of arrows based on Style and PlayerOptions. */
class ArrowEffects
{
public:
	static void Update();
	// SetCurrentOptions and the hidden static variable it set exists so that
	// ArrowEffects doesn't have to reach through the PlayerState to check
	// every option.  Also, it will make it easier to implement per-column
	// mods later. -Kyz
	static void SetCurrentOptions(const PlayerOptions* options);

	// fYOffset is a vertical position in pixels relative to the center
	// (positive if has not yet been stepped on, negative if has already passed).
	// The ArrowEffect and ScrollSpeed is applied in this stage.
	static float GetYOffset( const PlayerState* pPlayerState, int iCol, float fNoteBeat, float &fPeakYOffsetOut, bool &bIsPastPeakYOffset, bool bAbsolute=false );
	static float GetYOffset( const PlayerState* pPlayerState, int iCol, float fNoteBeat, bool bAbsolute=false )
	{
		float fThrowAway;
		bool bThrowAway;
		return GetYOffset( pPlayerState, iCol, fNoteBeat, fThrowAway, bThrowAway, bAbsolute );
	}

	static void GetXYZPos(const PlayerState* player_state, int col, float y_offset, float y_reverse_offset, RageVector3& ret, bool with_reverse= true)
	{
		ret.x= GetXPos(player_state, col, y_offset);
		ret.y= GetYPos(col, y_offset, y_reverse_offset, with_reverse);
		ret.z= GetZPos(col, y_offset);
	}

	/**
	 * @brief Retrieve the actual display position.
	 *
	 * In this case, reverse and post-reverse-effects are factored in (fYOffset -> YPos). 
	 * @param pPlayerState the Player's state in question, including mods.
	 * @param iCol the specific arrow column.
	 * @param fYOffset the original display position.
	 * @param fYReverseOffsetPixels the amount offset due to reverse.
	 * @param WithReverse a flag to see if the Reverse mod is on.
	 * @return the actual display position. */
	static float GetYPos(int iCol, float fYOffset, float fYReverseOffsetPixels, bool WithReverse = true );

	// Inverse of ArrowGetYPos (YPos -> fYOffset).
	static float GetYOffsetFromYPos(int iCol, float YPos, float fYReverseOffsetPixels);

	// fRotation is Z rotation of an arrow.  This will depend on the column of 
	// the arrow and possibly the Arrow effect and the fYOffset (in the case of 
	// EFFECT_DIZZY).
	static float GetRotationZ(	const PlayerState* pPlayerState, float fNoteBeat, bool bIsHoldHead );
	static float ReceptorGetRotationZ(	const PlayerState* pPlayerState );

	// Due to the handling logic for holds on Twirl, we need to use an offset instead.
	// It's more intuitive for Roll to be based off offset, so use an offset there too.
	static float GetRotationX(float fYOffset);
	static float GetRotationY(float fYOffset);

	// fXPos is a horizontal position in pixels relative to the center of the field.
	// This depends on the column of the arrow and possibly the Arrow effect and
	// fYPos (in the case of EFFECT_DRUNK).
	static float GetXPos( const PlayerState* pPlayerState, int iCol, float fYOffset );

	/**
	 * @brief Retrieve the Z position.
	 *
	 * This is normally 0. This is only visible with perspective modes.
	 * @param pPlayerState the Player's state, including the mods.
	 * @param iCol the specific arrow column.
	 * @param fYPos the Y position of the arrow.
	 * @return the Z position. */
	static float GetZPos(int iCol, float fYPos);

	// Enable this if any ZPos effects are enabled.
	static bool NeedZBuffer();

	// fAlpha is the transparency of the arrow.  It depends on fYPos and the 
	// AppearanceType.
	static float GetAlpha(int iCol, float fYPos, float fPercentFadeToFail, float fYReverseOffsetPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar);

	// fAlpha is the transparency of the arrow.  It depends on fYPos and the 
	// AppearanceType.
	static float GetGlow(int iCol, float fYPos, float fPercentFadeToFail, float fYReverseOffsetPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar );

	/**
	 * @brief Retrieve the current brightness.
	 *
	 * Note that this depends on fYOffset.
	 * @param pPlayerState the present PlayerState.
	 * @param fNoteBeat the current beat.
	 * @return the current brightness. */
	static float GetBrightness( const PlayerState* pPlayerState, float fNoteBeat );

	// This is the zoom of the individual tracks, not of the whole Player.
	static float GetZoom( const PlayerState* pPlayerState );

	static float GetFrameWidthScale( const PlayerState* pPlayerState, float fYOffset, float fOverlappedTime );
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
