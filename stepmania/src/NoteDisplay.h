/* NoteDisplay - Draws TapNotes and HoldNotes. */

#ifndef NOTEDISPLAY_H
#define NOTEDISPLAY_H

#include "Sprite.h"
class Model;
#include "NoteTypes.h"

struct HoldNoteResult;
struct NoteMetricCache_t;
class PlayerState;

enum NotePart
{
	NotePart_Tap,
	NotePart_Addition,
	NotePart_Mine,
	NotePart_HoldHead,
	NotePart_HoldTail,
	NotePart_HoldTopCap,
	NotePart_HoldBody,
	NotePart_HoldBottomCap,
	NUM_NotePart
};

#define NOTE_COLOR_IMAGES 8

struct NoteColorActor
{
	NoteColorActor();
	~NoteColorActor();
	void Load( const CString &sButton, const CString &sElement );
	Actor* Get( NoteType nt );
private:
	Actor* m_p[NOTE_COLOR_IMAGES];
	bool m_bIsNoteColor;
};

struct NoteColorSprite
{
	NoteColorSprite();
	~NoteColorSprite();
	void Load( const CString &sButton, const CString &sElement );
	Sprite* Get( NoteType nt );
private:
	Sprite*	m_p[NOTE_COLOR_IMAGES];
	bool m_bIsNoteColor;
};

enum HoldType { hold, roll, NUM_HOLD_TYPES };
#define FOREACH_HoldType( i ) FOREACH_ENUM( HoldType, NUM_HOLD_TYPES, i )
const CString &HoldTypeToString( HoldType ht );

enum ActiveType { active, inactive, NUM_ACTIVE_TYPES };
#define FOREACH_ActiveType( i ) FOREACH_ENUM( ActiveType, NUM_ACTIVE_TYPES, i )
const CString &ActiveTypeToString( ActiveType at );


class NoteDisplay
{
public:
	NoteDisplay();
	~NoteDisplay();

	void Load( int iColNum, const PlayerState* pPlayerState, float fYReverseOffsetPixels );

	static void Update( float fDeltaTime );

	void DrawActor( Actor* pActor, int iCol, float fBeat, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels, bool bUseLighting, NotePart part );
	void DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels );
	void DrawHold( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, bool bIsActive, const HoldNoteResult &Result, float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels, float fYStartOffset, float fYEndOffset );
	
	bool DrawHoldHeadForTapsOnSameRow() const;

protected:
	void SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLengthInBeats, bool bVivid );
	Actor *GetTapNoteActor( float fNoteBeat );
	Actor *GetTapAdditionActor( float fNoteBeat );
	Actor *GetTapMineActor( float fNoteBeat );
	Actor *GetHoldHeadActor( float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );
	Actor *GetHoldTailActor( float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );
	Sprite *GetHoldTopCapSprite( float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );
	Sprite *GetHoldBodySprite( float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );
	Sprite *GetHoldBottomCapSprite( float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );

	void DrawHoldBottomCap( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int	fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldTopCap( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldBody( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldTail( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYTail, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldHead( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );

	const PlayerState* m_pPlayerState;	// to look up PlayerOptions

	struct NoteMetricCache_t *cache;

	NoteColorActor		m_TapNote;
	NoteColorActor		m_TapAddition;
	NoteColorActor		m_TapMine;
	NoteColorActor		m_HoldHead[NUM_HOLD_TYPES][NUM_ACTIVE_TYPES];
	NoteColorSprite		m_HoldTopCap[NUM_HOLD_TYPES][NUM_ACTIVE_TYPES];
	NoteColorSprite		m_HoldBody[NUM_HOLD_TYPES][NUM_ACTIVE_TYPES];
	NoteColorSprite		m_HoldBottomCap[NUM_HOLD_TYPES][NUM_ACTIVE_TYPES];
	NoteColorActor		m_HoldTail[NUM_HOLD_TYPES][NUM_ACTIVE_TYPES];
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
