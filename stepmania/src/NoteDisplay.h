#pragma once
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

	void DrawTap( const int iCol, const float fBeat, const bool bUseHoldColor, const float fPercentFadeToFail );
	void DrawHold( const HoldNote& hn, const bool bActive, const float fLife, const float fPercentFadeToFail );

protected:
	int			GetTapGrayFrameNo( const float fNoteBeat );
	int			GetTapColorFrameNo( const float fNoteBeat );
	void		GetTapEdgeColors( const float fNoteBeat, D3DXCOLOR &colorLeadingOut, D3DXCOLOR &colorTrailingOut );
//	D3DXCOLOR	GetHoldColor( const float fY, const float fYTop, const float fYBottom );
	float		GetAddAlpha( float DiffuseAlpha, const float fPercentFadeToFail );

	PlayerNumber m_PlayerNumber;	// to look up PlayerOptions

	Sprite		m_sprTapParts;		// for now, must be an even number of frames
	Sprite		m_sprHoldParts;		// for now, must be 16 frames

	CArray<D3DXCOLOR,D3DXCOLOR> m_colorTapTweens;
//	CArray<D3DXCOLOR,D3DXCOLOR> m_colorHoldTweens;
};
