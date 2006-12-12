/* NoteField - An Actor that renders a NoteData. */

#ifndef NOTE_FIELD_H
#define NOTE_FIELD_H

#include "Sprite.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"
#include "NoteDisplay.h"
#include "ReceptorArrowRow.h"
#include "GhostArrowRow.h"

struct Attack;
class NoteData;

class NoteField : public ActorFrame
{
public:
	NoteField();
	~NoteField();
	virtual void Update( float fDeltaTime );
	virtual void ProcessMessages( float fDeltaTime );
	virtual void DrawPrimitives();
	
	virtual void Init( const PlayerState* pPlayerState, float fYReverseOffsetPixels );
	virtual void Load( 
		const NoteData* pNoteData, 
		int iDrawDistanceAfterTargetsPixels, 
		int iDrawDistanceBeforeTargetsPixels );
	virtual void Unload();

	virtual void HandleMessage( const Message &msg );

	// This is done automatically by Init(), but can be re-called explicitly if the
	// note skin changes.
	void CacheAllUsedNoteSkins();
	void FadeToFail();

	void Step( int iCol, TapNoteScore score );
	void SetPressed( int iCol );
	void DidTapNote( int iCol, TapNoteScore score, bool bBright );
	void DidHoldNote( int iCol, HoldNoteScore score, bool bBright );

	const PlayerState *GetPlayerState() const { return m_pPlayerState; }

	int	m_iBeginMarker, m_iEndMarker;	// only used with MODE_EDIT

protected:
	void CacheNoteSkin( const RString &sNoteSkin );
	void UncacheNoteSkin( const RString &sNoteSkin );

	bool IsOnScreen( float fBeat, int iCol, int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels ) const;

	void DrawBoard( int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels );
	void DrawBeatBar( const float fBeat );
	void DrawMarkerBar( int fBeat );
	void DrawAreaHighlight( int iStartBeat, int iEndBeat );
	void DrawBPMText( const float fBeat, const float fBPM );
	void DrawFreezeText( const float fBeat, const float fBPM );
	void DrawAttackText( const float fBeat, const Attack &attack );
	void DrawBGChangeText( const float fBeat, const RString sNewBGName );
	float GetWidth() const;

	const NoteData *m_pNoteData;

	float			m_fPercentFadeToFail;	// -1 if not fading to fail

	const PlayerState*	m_pPlayerState;
	int			m_iDrawDistanceAfterTargetsPixels;	// this should be a negative number
	int			m_iDrawDistanceBeforeTargetsPixels;	// this should be a positive number
	float			m_fYReverseOffsetPixels;

	// color arrows
	struct NoteDisplayCols
	{
		NoteDisplay		*display;
		ReceptorArrowRow	m_ReceptorArrowRow;
		GhostArrowRow		m_GhostArrowRow;
		NoteDisplayCols( int iNumCols ) { display = new NoteDisplay[iNumCols]; }
		~NoteDisplayCols() { delete [] display; }
	};

	/* All loaded note displays, mapped by their name. */
	map<RString, NoteDisplayCols *> m_NoteDisplays;
	NoteDisplayCols		*m_pCurDisplay;
	NoteDisplayCols		*m_pDisplays[NUM_PlayerNumber];

	// decorations, mostly used in MODE_EDIT
	AutoActor	m_sprBoard;
	float		m_fBoardOffsetPixels;
	float		m_fCurrentBeatLastUpdate;	// -1 on first update
	float		m_fYPosCurrentBeatLastUpdate;	// -1 on first update

	Sprite		m_sprBeatBars;	// 4 frames: Measure, 4th, 8th, 16th
	BitmapText	m_textMeasureNumber;
	Quad		m_rectMarkerBar;
	Quad		m_rectAreaHighlight;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
