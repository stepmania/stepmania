#ifndef NOTEDISPLAY_H
#define NOTEDISPLAY_H
/*
-----------------------------------------------------------------------------
 Class: NoteDisplay

 Desc: Draws TapNotes and HoldNotes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Brian Bugh
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "Model.h"
#include "NoteTypes.h"
#include "PlayerNumber.h"


struct NoteMetricCache_t;

class NoteDisplay
{
public:
	NoteDisplay();
	~NoteDisplay();

	void Load( int iColNum, PlayerNumber pn, float fYReverseOffsetPixels  );

	void DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels );
	void DrawHold( const HoldNote& hn, bool bActive, float fLife, float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels );

protected:
	void SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLengthInBeats, bool bVivid, bool bNoteColor );
	Actor *GetTapNoteActor( float fNoteBeat );
	Actor *GetTapAdditionActor( float fNoteBeat );
	Actor *GetTapMineActor( float fNoteBeat );
	Actor *GetHoldHeadActor( float fNoteBeat, bool bActive );
	Actor* GetHoldTailActor( float fNoteBeat, bool bActive );
	Sprite *GetHoldTopCapSprite( float fNoteBeat, bool bActive );
	Sprite *GetHoldBodySprite( float fNoteBeat, bool bActive );
	Sprite *GetHoldBottomCapSprite( float fNoteBeat, bool bActive );

	void DrawHoldBottomCap( const HoldNote& hn, const bool bActive, float fYHead, float fYTail, int	fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldTopCap( const HoldNote& hn, const bool bActive, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldBody( const HoldNote& hn, const bool bActive, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldTail( const HoldNote& hn, const bool bActive, float fYTail, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );
	void DrawHoldHead( const HoldNote& hn, const bool bActive, float fYHead, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow );

	PlayerNumber m_PlayerNumber;	// to look up PlayerOptions

	struct NoteMetricCache_t *cache;

#define NOTE_COLOR_IMAGES 6

	float		m_fYReverseOffsetPixels;
};

#endif
