/* NoteDisplay - Draws TapNotes and HoldNotes. */

#ifndef NOTEDISPLAY_H
#define NOTEDISPLAY_H

#include "Sprite.h"
class Model;
#include "NoteTypes.h"
#include "PlayerNumber.h"

struct HoldNoteResult;
struct NoteMetricCache_t;

class NoteDisplay
{
public:
	NoteDisplay();
	~NoteDisplay();

	void Load( int iColNum, PlayerNumber pn, CString NoteSkin, float fYReverseOffsetPixels );

	static void Update( float fDeltaTime );

	void DrawActor( Actor* pActor, int iCol, float fBeat, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels, bool bUseLighting );
	void DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels );
	void DrawHold( const HoldNote& hn, bool bIsBeingHeld, bool bIsActive, const HoldNoteResult &Result, float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels );

protected:
	void SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLengthInBeats, bool bVivid, bool bNoteColor );
	Actor *GetTapNoteActor( float fNoteBeat );
	Actor *GetTapAdditionActor( float fNoteBeat );
	Actor *GetTapMineActor( float fNoteBeat );
	Actor *GetHoldHeadActor( float fNoteBeat, bool bIsBeingHeld );
	Actor* GetHoldTailActor( float fNoteBeat, bool bIsBeingHeld );
	Sprite *GetHoldTopCapSprite( float fNoteBeat, bool bIsBeingHeld );
	Sprite *GetHoldBodySprite( float fNoteBeat, bool bIsBeingHeld );
	Sprite *GetHoldBottomCapSprite( float fNoteBeat, bool bIsBeingHeld );

	void DrawHoldBottomCap( const HoldNote& hn, const bool bIsBeingHeld, float fYHead, float fYTail, int	fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldTopCap( const HoldNote& hn, const bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldBody( const HoldNote& hn, const bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldTail( const HoldNote& hn, const bool bIsBeingHeld, float fYTail, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldHead( const HoldNote& hn, const bool bIsBeingHeld, float fYHead, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );

	PlayerNumber m_PlayerNumber;	// to look up PlayerOptions

	struct NoteMetricCache_t *cache;

#define NOTE_COLOR_IMAGES 8

	Actor*		m_pTapNote[NOTE_COLOR_IMAGES];
	Actor*		m_pTapAddition[NOTE_COLOR_IMAGES];
	Actor*		m_pTapMine[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldHeadActive[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldHeadInactive[NOTE_COLOR_IMAGES];
	Sprite*		m_pHoldTopCapActive[NOTE_COLOR_IMAGES];
	Sprite*		m_pHoldTopCapInactive[NOTE_COLOR_IMAGES];
	Sprite*		m_pHoldBodyActive[NOTE_COLOR_IMAGES];
	Sprite*		m_pHoldBodyInactive[NOTE_COLOR_IMAGES];
	Sprite*		m_pHoldBottomCapActive[NOTE_COLOR_IMAGES];
	Sprite*		m_pHoldBottomCapInactive[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldTailActive[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldTailInactive[NOTE_COLOR_IMAGES];
	float		m_fYReverseOffsetPixels;
};

#endif

/*
 * (c) 2001-2004 Brian Bugh, Ben Nordstrom, Chris Danford
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
