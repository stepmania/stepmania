#pragma once
/*
-----------------------------------------------------------------------------
 Class: NoteField

 Desc: A stream of ColorNotes that scrolls past Y==0.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "ActorFrame.h"
#include "Song.h"
#include "BitmapText.h"
#include "PrefsManager.h"
#include "StyleDef.h"
#include "ColorNote.h"
#include "BitmapText.h"
#include "Quad.h"
#include "ArrowEffects.h"
#include "NoteDataWithScoring.h"


class NoteField : public NoteDataWithScoring, public ActorFrame
{
public:
	NoteField();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	
	void Load( NoteData* pNoteData, PlayerNumber pn, int iPixelsToDrawBehind, int iPixelsToDrawAhead );
	void RemoveTapNoteRow( int iIndex );

	bool m_bIsHoldingHoldNote[MAX_HOLD_NOTES];	// hack:  Need this to prevent hold note jitter

	float	m_fBeginMarker, m_fEndMarker;	// only used with MODE_EDIT

	float  m_fOverrideAdd;	// -1 of not overriding

protected:
	inline void CreateTapNoteInstance( ColorNoteInstance &cni, const int iCol, const float fIndex, const D3DXCOLOR color = D3DXCOLOR(-1,-1,-1,-1) );
	inline void CreateHoldNoteInstance( ColorNoteInstance &cni, const bool bActive, const float fIndex, const HoldNote &hn, const float fHoldNoteLife );
	inline void DrawMeasureBar( const int iIndex, const int iMeasureNo );
	inline void DrawMarkerBar( const int iIndex );
	inline void DrawBPMText( const int iIndex, const float fBPM );
	inline void DrawFreezeText( const int iIndex, const float fBPM );

	PlayerNumber	m_PlayerNumber;
	int				m_iPixelsToDrawBehind;
	int				m_iPixelsToDrawAhead;

	// color arrows
	ColorNote		m_ColorNote[MAX_NOTE_TRACKS];
	
	// used in MODE_EDIT
	Quad			m_rectMeasureBar;
	BitmapText		m_textMeasureNumber;
	Quad			m_rectMarkerBar;
};
