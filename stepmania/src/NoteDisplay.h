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
	int	GetTapNoteFrameNo( float fNoteBeat );
	int	GetHoldHeadFrameNo( float fNoteBeat, bool bActive );
	int	GetHoldBodyFrameNo( float fNoteBeat, bool bActive );
	int	GetHoldTailFrameNo( float fNoteBeat, bool bActive );

	PlayerNumber m_PlayerNumber;	// to look up PlayerOptions

	Sprite		m_sprTapNote;
	Sprite		m_sprHoldHeadActive;
	Sprite		m_sprHoldHeadInactive;
	Sprite		m_sprHoldBodyActive;
	Sprite		m_sprHoldBodyInactive;
	Sprite		m_sprHoldTailActive;
	Sprite		m_sprHoldTailInactive;
};

#endif
