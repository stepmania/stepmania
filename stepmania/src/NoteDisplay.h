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
#include "NoteTypes.h"


class NoteDisplay
{
public:
	NoteDisplay();

	void Load( int iColNum, PlayerNumber pn );

	void DrawTap( const int iCol, const float fBeat, const bool bOnSameRowAsHoldStart, const float fPercentFadeToFail, const float fLife = 1 );
	void DrawHold( const HoldNote& hn, const bool bActive, const float fLife, const float fPercentFadeToFail, bool bDrawGlowOnly = false );

protected:
	int GetFrameNo( float fNoteBeat, int iNumFrames, float fAnimationLengthInBeats, bool bVivid, bool bNoteColor  );
	void	GetTapNoteSpriteAndFrameNo( float fNoteBeat, Sprite*& pSpriteOut, int& iFrameNoOut );
	void	GetHoldHeadSpriteAndFrameNo( float fNoteBeat, bool bActive, Sprite*& pSpriteOut, int& iFrameNoOut );
	void	GetHoldBodySpriteAndFrameNo( float fNoteBeat, bool bActive, Sprite*& pSpriteOut, int& iFrameNoOut );
	void	GetHoldTailSpriteAndFrameNo( float fNoteBeat, bool bActive, Sprite*& pSpriteOut, int& iFrameNoOut );

	PlayerNumber m_PlayerNumber;	// to look up PlayerOptions

#define NOTE_COLOR_IMAGES 6

	Sprite		m_sprTapNote[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldHeadActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldHeadInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBodyActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldBodyInactive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTailActive[NOTE_COLOR_IMAGES];
	Sprite		m_sprHoldTailInactive[NOTE_COLOR_IMAGES];
};

#endif
