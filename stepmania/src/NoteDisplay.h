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

	void DrawTap( const int iCol, const float fBeat, const bool bOnSameRowAsHoldStart, const float fPercentFadeToFail, const float fLife = 1 );
	void DrawHold( const HoldNote& hn, const bool bActive, const float fLife, const float fPercentFadeToFail, bool bDrawGlowOnly = false );

protected:
	void SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLengthInBeats, bool bVivid, bool bNoteColor );
	Actor *GetTapNoteActor( float fNoteBeat );
	Actor *GetHoldHeadActor( float fNoteBeat, bool bActive );
	Sprite *GetHoldTopCapSprite( float fNoteBeat, bool bActive );
	Sprite *GetHoldBodySprite( float fNoteBeat, bool bActive );
	Sprite *GetHoldBottomCapSprite( float fNoteBeat, bool bActive );
	Sprite *GetHoldTailSprite( float fNoteBeat, bool bActive );

	PlayerNumber m_PlayerNumber;	// to look up PlayerOptions

	struct NoteMetricCache_t *cache;

#define NOTE_COLOR_IMAGES 6

	Actor*		m_pTapNote[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldHeadActive[NOTE_COLOR_IMAGES];
	Actor*		m_pHoldHeadInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTopCapActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTopCapInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBodyActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBodyInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBottomCapActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBottomCapInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTailActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTailInactive[NOTE_COLOR_IMAGES];
};

#endif
