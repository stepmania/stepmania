#ifndef NOTE_DISPLAY_H
#define NOTE_DISPLAY_H

class Actor;
class Sprite;
class Model;
class PlayerState;
struct TapNote;
struct HoldNoteResult;
struct NoteMetricCache_t;
/** @brief the various parts of a Note. */
enum NotePart
{
	NotePart_Tap, /**< The part representing a traditional TapNote. */
	NotePart_Mine, /**< The part representing a mine. */
	NotePart_Lift, /**< The part representing a lift note. */
	NotePart_Fake, /**< The part representing a fake note. */
	NotePart_HoldHead, /**< The part representing a hold head. */
	NotePart_HoldTail, /**< The part representing a hold tail. */
	NotePart_HoldTopCap, /**< The part representing a hold's top cap. */
	NotePart_HoldBody, /**< The part representing a hold's body. */
	NotePart_HoldBottomCap, /**< The part representing a hold's bottom cap. */
	NUM_NotePart,
	NotePart_Invalid
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
/** @brief What types of holds are there? */
enum HoldType 
{
	hold, /**< Merely keep your foot held on the body for it to count. */
	roll, /**< Keep hitting the hold body for it to stay alive. */
	// minefield,
	NUM_HoldType,
	HoldType_Invalid
};
/** @brief Loop through each HoldType. */
#define FOREACH_HoldType( i ) FOREACH_ENUM( HoldType, i )
const RString &HoldTypeToString( HoldType ht );

enum ActiveType
{
	active,
	inactive,
	NUM_ActiveType,
	ActiveType_Invalid
};
/** @brief Loop through each ActiveType. */
#define FOREACH_ActiveType( i ) FOREACH_ENUM( ActiveType, i )
const RString &ActiveTypeToString( ActiveType at );

/** @brief Draws TapNotes and HoldNotes. */
class NoteDisplay
{
public:
	NoteDisplay();
	~NoteDisplay();

	void Load( int iColNum, const PlayerState* pPlayerState, float fYReverseOffsetPixels );

	static void Update( float fDeltaTime );

	/**
	 * @brief Draw the TapNote onto the NoteField.
	 * @param tn the TapNote in question.
	 * @param iCol the column.
	 * @param float fBeat the beat to draw them on.
	 * @param bOnSameRowAsHoldStart a flag to see if a hold is on the same beat.
	 * @param bOnSameRowAsRollStart a flag to see if a roll is on the same beat.
	 * @param bIsAddition a flag to see if this note was added via mods.
	 * @param fPercentFadeToFail at what point do the notes fade on failure?
	 * @param fReverseOffsetPixels How are the notes adjusted on Reverse? 
	 * @param fDrawDistanceAfterTargetsPixels how much to draw after the receptors.
	 * @param fDrawDistanceBeforeTargetsPixels how much ot draw before the receptors.
	 * @param fFadeInPercentOfDrawFar when to start fading in. */
	void DrawTap(const TapNote& tn, int iCol, float fBeat, 
		     bool bOnSameRowAsHoldStart, bool bOnSameRowAsRollBeat,
		     bool bIsAddition, float fPercentFadeToFail,
		     float fReverseOffsetPixels,
		     float fDrawDistanceAfterTargetsPixels,
		     float fDrawDistanceBeforeTargetsPixels,
		     float fFadeInPercentOfDrawFar );
	void DrawHold( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, const HoldNoteResult &Result, 
		bool bIsAddition, float fPercentFadeToFail, float fReverseOffsetPixels, float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, 
		float fDrawDistanceBeforeTargetsPixels2, float fFadeInPercentOfDrawFar );
	
	bool DrawHoldHeadForTapsOnSameRow() const;
	
	bool DrawRollHeadForTapsOnSameRow() const;

private:
	void SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLength, bool bVivid );
	Actor *GetTapActor( NoteColorActor &nca, NotePart part, float fNoteBeat );
	Actor *GetHoldActor( NoteColorActor nca[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );
	Sprite *GetHoldSprite( NoteColorSprite ncs[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );

	void DrawActor( const TapNote& tn, Actor* pActor, NotePart part, int iCol, float fYOffset, float fBeat, bool bIsAddition, float fPercentFadeToFail,
			float fReverseOffsetPixels, float fColorScale, float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar );
	void DrawHoldBody( const TapNote& tn, int iCol, float fBeat, bool bIsBeingHeld, float fYHead, float fYTail, bool bIsAddition, float fPercentFadeToFail, 
			   float fColorScale, 
			   bool bGlow, float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar );
	void DrawHoldPart( vector<Sprite*> &vpSpr, int iCol, int fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow,
			   float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar, float fOverlappedTime,
			   float fYTop, float fYBottom, float fYStartPos, float fYEndPos, bool bWrapping, bool bAnchorToTop, bool bFlipTextureVertically );

	const PlayerState	*m_pPlayerState;	// to look up PlayerOptions
	NoteMetricCache_t	*cache;

	NoteColorActor		m_TapNote;
	NoteColorActor		m_TapMine;
	NoteColorActor		m_TapLift;
	NoteColorActor		m_TapFake;
	NoteColorActor		m_HoldHead[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldTopCap[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldBody[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldBottomCap[NUM_HoldType][NUM_ActiveType];
	NoteColorActor		m_HoldTail[NUM_HoldType][NUM_ActiveType];
	float			m_fYReverseOffsetPixels;
};

#endif

/**
 * @file
 * @author Brian Bugh, Ben Nordstrom, Chris Danford, Steve Checkoway (c) 2001-2006
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
 */
