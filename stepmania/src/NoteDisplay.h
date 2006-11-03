/* NoteDisplay - Draws TapNotes and HoldNotes. */

#ifndef NOTE_DISPLAY_H
#define NOTE_DISPLAY_H

class Actor;
class Sprite;
class Model;
class PlayerState;
struct TapNote;
struct HoldNoteResult;
struct NoteMetricCache_t;

enum NotePart
{
	NotePart_Tap,
	NotePart_Addition,
	NotePart_Mine,
	NotePart_Lift,
	NotePart_HoldHead,
	NotePart_HoldTail,
	NotePart_HoldTopCap,
	NotePart_HoldBody,
	NotePart_HoldBottomCap,
	NUM_NotePart
};

struct NoteResource;

struct NoteColorActor
{
	NoteColorActor();
	~NoteColorActor();
	void Load( const RString &sButton, const RString &sElement );
	Actor *Get();
private:
	NoteResource *m_p;
};

struct NoteColorSprite
{
	NoteColorSprite();
	~NoteColorSprite();
	void Load( const RString &sButton, const RString &sElement );
	Sprite *Get();
private:
	NoteResource *m_p;
};

enum HoldType 
{
	hold, 
	roll, 
	NUM_HoldType,
	HoldType_Invalid
};
#define FOREACH_HoldType( i ) FOREACH_ENUM( HoldType, i )
const RString &HoldTypeToString( HoldType ht );

enum ActiveType
{
	active,
	inactive,
	NUM_ActiveType,
	ActiveType_Invalid
};
#define FOREACH_ActiveType( i ) FOREACH_ENUM( ActiveType, i )
const RString &ActiveTypeToString( ActiveType at );


class NoteDisplay
{
public:
	NoteDisplay();
	~NoteDisplay();

	void Load( int iColNum, const PlayerState* pPlayerState, float fYReverseOffsetPixels );

	static void Update( float fDeltaTime );

	void DrawActor( Actor* pActor, int iCol, float fBeat, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels, bool bUseLighting, NotePart part );
	void DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, bool bIsLift, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels );
	void DrawHold( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, bool bIsActive, const HoldNoteResult &Result, float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels, float fYStartOffset, float fYEndOffset );
	
	bool DrawHoldHeadForTapsOnSameRow() const;

private:
	void SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLengthInBeats, bool bVivid );
	Actor *GetTapActor( NoteColorActor &nca, NotePart part, float fNoteBeat );
	Actor *GetHoldActor( NoteColorActor nca[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );
	Sprite *GetHoldSprite( NoteColorSprite ncs[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );

	void DrawHoldBottomCap( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, 
	                        float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldTopCap( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int fYStep,
			     float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldBody( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, float fPercentFadeToFail, 
			   float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldTail( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYTail, float fPercentFadeToFail, float fColorScale, 
			   bool bGlow, float fYStartOffset, float fYEndOffset );
	void DrawHoldHead( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fPercentFadeToFail, float fColorScale, 
			   bool bGlow, float fYStartOffset, float fYEndOffset );

	const PlayerState	*m_pPlayerState;	// to look up PlayerOptions
	NoteMetricCache_t	*cache;

	NoteColorActor		m_TapNote;
	NoteColorActor		m_TapAddition;
	NoteColorActor		m_TapMine;
	NoteColorActor		m_TapLift;
	NoteColorActor		m_HoldHead[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldTopCap[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldBody[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldBottomCap[NUM_HoldType][NUM_ActiveType];
	NoteColorActor		m_HoldTail[NUM_HoldType][NUM_ActiveType];
	float			m_fYReverseOffsetPixels;
};

#endif

/*
 * (c) 2001-2006 Brian Bugh, Ben Nordstrom, Chris Danford, Steve Checkoway
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
