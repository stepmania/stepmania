#ifndef NOTEFIELD_H
#define NOTEFIELD_H
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
#include "BitmapText.h"
#include "PrefsManager.h"
#include "StyleDef.h"
#include "BitmapText.h"
#include "Quad.h"
#include "ArrowEffects.h"
#include "NoteDataWithScoring.h"
#include "NoteDisplay.h"
#include "ArrowBackdrop.h"

class Song;

class NoteField : public NoteDataWithScoring, public ActorFrame
{
public:
	NoteField();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	
	virtual void Load( NoteData* pNoteData, PlayerNumber pn, int iStartDrawingPixel, int iEndDrawingPixel, int iYReverseOffsetPixels );
	void RemoveTapNoteRow( int iIndex );

	vector<bool> m_bIsHoldingHoldNote;	// hack:  Need this to know when to "light up" the center of hold notes

	float	m_fBeginMarker, m_fEndMarker;	// only used with MODE_EDIT

	void FadeToFail();
	void ReloadNoteSkin();


protected:
	void DrawBeatBar( const float fBeat );
	void DrawMarkerBar( const float fBeat );
	void DrawAreaHighlight( const float fStartBeat, const float fEndBeat );
	void DrawBPMText( const float fBeat, const float fBPM );
	void DrawFreezeText( const float fBeat, const float fBPM );
	void DrawBGChangeText( const float fBeat, const CString sNewBGName );
	float GetWidth();

	float	m_fPercentFadeToFail;	// -1 of not fading to fail

	PlayerNumber	m_PlayerNumber;
	int				m_iStartDrawingPixel;	// this should be a negative number
	int				m_iEndDrawingPixel;	// this should be a positive number
	int				m_iYPosNormal;
	int				m_iYReverseOffsetPixels;

	// color arrows
	NoteDisplay		m_NoteDisplay[MAX_NOTE_TRACKS];
	
	// used in MODE_EDIT
	Sprite			m_sprBars;	// 4 frames: Measure, 4th, 8th, 16th
	BitmapText		m_textMeasureNumber;
	Quad			m_rectMarkerBar;
	Quad			m_rectAreaHighlight;
};

#endif
