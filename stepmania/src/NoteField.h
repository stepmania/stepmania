#pragma once
/*
-----------------------------------------------------------------------------
 Class: NoteField

 Desc: A stream of ColorNotes that scrolls past Y==0.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "ActorFrame.h"
#include "Song.h"
#include "BitmapText.h"
#include "ThemeManager.h"
#include "StyleDef.h"
#include "ColorNote.h"
#include "BitmapText.h"
#include "RectangleActor.h"
#include "ArrowEffects.h"




class NoteField : public NoteData, public ActorFrame
{
public:
	NoteField();
	virtual void Update( float fDeltaTime, float fSongBeat );
	virtual void RenderPrimitives();

	enum NoteFieldMode {
		MODE_DANCING,
		MODE_EDITING,
	};
	
	void Load( NoteData* pNoteData, PlayerOptions po, float fNumArrowsToDrawBehind, float fNumArrowsToDrawAhead, NoteFieldMode mode );
	void RemoveTapNoteRow( int iIndex );
	void SetHoldNoteLife( int iIndex, float fLife );

	float	m_HoldNoteLife[MAX_HOLD_NOTE_ELEMENTS];		// 1.0 = full life, 0 = dead

	float	m_fBeginMarker, m_fEndMarker;	// only used with MODE_EDIT

protected:
	inline void CreateTapNoteInstance( ColorNoteInstance &cni, const int iCol, const float fIndex, const D3DXCOLOR color = D3DXCOLOR(-1,-1,-1,-1) );
	inline void CreateHoldNoteInstance( ColorNoteInstance &cni, const bool bActive, const float fIndex, const HoldNote &hn, const float fHoldNoteLife );
	inline void DrawMeasureBar( const int iIndex, const int iMeasureNo );
	inline void DrawMarkerBar( const int iIndex );

	PlayerOptions	m_PlayerOptions;

	float			m_fNumBeatsToDrawBehind;
	float			m_fNumBeatsToDrawAhead;

	float			m_fSongBeat;


	NoteFieldMode m_Mode;

	// color arrows
	ColorNote		m_ColorNote[MAX_NOTE_TRACKS];
	
	// used in MODE_EDIT
	RectangleActor	m_rectMeasureBar;
	BitmapText		m_textMeasureNumber;
	RectangleActor	m_rectMarkerBar;
};
