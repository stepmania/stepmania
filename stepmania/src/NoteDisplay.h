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

	void Load( int iColNum, PlayerNumber pn, CString NoteSkin, float fYReverseOffsetPixels );

	void Update( float fDeltaTime );

	void DrawActor( Actor* pActor, int iCol, float fBeat, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels, bool bUseLighting );
	void DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels );
	void DrawHold( const HoldNote& hn, bool bIsBeingHeld, bool bIsActive, float fLife, float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels );

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
	float		m_fYReverseOffsetPixels;
};

#endif
