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
	int			GetTapFrameNo( const float fNoteBeat );

	PlayerNumber m_PlayerNumber;	// to look up PlayerOptions

	Sprite		m_sprTap;
	Sprite		m_sprHoldParts;
};

#endif
