/* ArrowEffects - Functions that return properties of arrows based on Style and PlayerOptions. */

#ifndef ARROWEFFECTS_H
#define ARROWEFFECTS_H

class PlayerState;

class ArrowEffects
{
public:
	static void Update();
	
	//	fYOffset is a vertical position in pixels relative to the center.
	//	(positive if has not yet been stepped on, negative if has already passed).
	//	The ArrowEffect and ScrollSpeed is applied in this stage.
	static float GetYOffset( const PlayerState* pPlayerState, int iCol, float fNoteBeat, float &fPeakYOffsetOut, bool &bIsPastPeakYOffset, bool bAbsolute=false );
	static float GetYOffset( const PlayerState* pPlayerState, int iCol, float fNoteBeat, bool bAbsolute=false )
	{
		float fThrowAway;
		bool bThrowAway;
		return GetYOffset( pPlayerState, iCol, fNoteBeat, fThrowAway, bThrowAway, bAbsolute );
	}

	/* Actual display position, with reverse and post-reverse-effects factored in
	 * (fYOffset -> YPos). */
	static float GetYPos(	const PlayerState* pPlayerState, int iCol, float fYOffset, float fYReverseOffsetPixels, bool WithReverse = true );

	// Inverse of ArrowGetYPos (YPos -> fYOffset).
	static float GetYOffsetFromYPos( const PlayerState* pPlayerState, int iCol, float YPos, float fYReverseOffsetPixels );


	//	fRotation is Z rotation of an arrow.  This will depend on the column of 
	//	the arrow and possibly the Arrow effect and the fYOffset (in the case of 
	//	EFFECT_DIZZY).
	static float GetRotation(	const PlayerState* pPlayerState, float fNoteBeat );


	//	fXPos is a horizontal position in pixels relative to the center of the field.
	//	This depends on the column of the arrow and possibly the Arrow effect and
	//	fYPos (in the case of EFFECT_DRUNK).
	static float GetXPos( const PlayerState* pPlayerState, int iCol, float fYOffset );

	//  Z position; normally 0.  Only visible in perspective modes.
	static float GetZPos( const PlayerState* pPlayerState, int iCol, float fYPos );

	// Enable this if any ZPos effects are enabled.
	static bool NeedZBuffer( const PlayerState* pPlayerState );

	//	fAlpha is the transparency of the arrow.  It depends on fYPos and the 
	//	AppearanceType.
	static float GetAlpha( const PlayerState* pPlayerState, int iCol, float fYPos, float fPercentFadeToFail, float fYReverseOffsetPixels );


	//	fAlpha is the transparency of the arrow.  It depends on fYPos and the 
	//	AppearanceType.
	static float GetGlow( const PlayerState* pPlayerState, int iCol, float fYPos, float fPercentFadeToFail, float fYReverseOffsetPixels );


	//	Depends on fYOffset.
	static float GetBrightness( const PlayerState* pPlayerState, float fNoteBeat );

	// This is the zoom of the individual tracks, not of the whole Player.
	static float GetZoom( const PlayerState* pPlayerState );
};

#endif

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
