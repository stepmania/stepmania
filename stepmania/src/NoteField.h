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
#include "NoteDataWithScoring.h"
#include "NoteDisplay.h"
#include "ArrowBackdrop.h"
#include "ReceptorArrowRow.h"
#include "GhostArrowRow.h"

class Song;

class NoteField : public NoteDataWithScoring, public ActorFrame
{
public:
	NoteField();
	~NoteField();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	
	virtual void Load( const NoteData* pNoteData, PlayerNumber pn, int iStartDrawingPixel, int iEndDrawingPixel, float fYReverseOffsetPixels );
	virtual void Unload();
	void RemoveTapNoteRow( int iIndex );

	map<RowTrack,bool> m_HeldHoldNotes;	// hack:  Need this to know when to "light up" the center of hold notes

	float	m_fBeginMarker, m_fEndMarker;	// only used with MODE_EDIT

	void FadeToFail();
	void CacheAllUsedNoteSkins();
	void CacheNoteSkin( CString skin );

	void Step( int iCol );
	void SetPressed( int iCol );
	void DidTapNote( int iCol, TapNoteScore score, bool bBright );
	void DidHoldNote( int iCol );
	void DidTapMine( int iCol, TapNoteScore score );


protected:
	void DrawBeatBar( const float fBeat );
	void DrawMarkerBar( const float fBeat );
	void DrawAreaHighlight( const float fStartBeat, const float fEndBeat );
	void DrawBPMText( const float fBeat, const float fBPM );
	void DrawFreezeText( const float fBeat, const float fBPM );
	void DrawBGChangeText( const float fBeat, const CString sNewBGName );
	float GetWidth();

	void RefreshBeatToNoteSkin();

	float	m_fPercentFadeToFail;	// -1 of not fading to fail

	PlayerNumber	m_PlayerNumber;
	int				m_iStartDrawingPixel;	// this should be a negative number
	int				m_iEndDrawingPixel;	// this should be a positive number
	float			m_fYReverseOffsetPixels;

	// color arrows
	struct NoteDisplayCols
	{
		NoteDisplay		display[MAX_NOTE_TRACKS];
		ReceptorArrowRow	m_ReceptorArrowRow;
		GhostArrowRow	m_GhostArrowRow;
	};

	/* All loaded note displays, mapped by their name. */
	map<CString, NoteDisplayCols *> m_NoteDisplays;

	int m_LastSeenBeatToNoteSkinRev;

	/* Map of beat->NoteDisplayCols.  This is updated whenever GAMESTATE-> changes. */
	typedef map<float, NoteDisplayCols *> NDMap;
	void SearchForBeat( NDMap::iterator &cur, NDMap::iterator &next, float Beat );
	NoteDisplayCols *SearchForBeat( float Beat );
	NoteDisplayCols *SearchForSongBeat();

	NDMap m_BeatToNoteDisplays;

	NoteDisplayCols *LastDisplay;

	// used in MODE_EDIT
	Sprite			m_sprBars;	// 4 frames: Measure, 4th, 8th, 16th
	BitmapText		m_textMeasureNumber;
	Quad			m_rectMarkerBar;
	Quad			m_rectAreaHighlight;
};

#endif
