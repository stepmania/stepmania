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

	void Load( int iColNum, PlayerNumber pn );

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

	Actor*		m_pTapNote[NOTE_COLOR_IMAGES];
	Actor*		m_pTapAddition;
	Actor*		m_pTapMine;
	Actor*		m_pHoldHeadActive[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldHeadInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTopCapActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTopCapInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBodyActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBodyInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBottomCapActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBottomCapInactive[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldTailActive[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldTailInactive[NOTE_COLOR_IMAGES];
};

#endif
