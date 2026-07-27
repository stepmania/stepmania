#ifndef NOTE_FIELD_H
#define NOTE_FIELD_H

#include "TimingData.h"
#include "SongPosition.h"
#include "Sprite.h"
#include "ActorFrame.h"
#include "BitmapText.h"
#include "Quad.h"
#include "NoteDisplay.h"
#include "ReceptorArrowRow.h"
#include "GhostArrowRow.h"

#include <vector>


struct Attack;
class NoteData;
/** @brief An Actor that renders NoteData. */
class NoteField : public ActorFrame
{
public:
	NoteField();
	~NoteField();
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	void CalcPixelsBeforeAndAfterTargets();
	void DrawBoardPrimitive();

	virtual void Init( const PlayerState* pPlayerState, float fYReverseOffsetPixels, bool use_states_zoom= true );
	virtual void Load(
		const NoteData* pNoteData,
		int iDrawDistanceAfterTargetsPixels,
		int iDrawDistanceBeforeTargetsPixels );
	virtual void Unload();

	void ensure_note_displays_have_skin();
	void InitColumnRenderers();

	virtual void HandleMessage( const Message &msg );

	// This is done automatically by Init(), but can be re-called explicitly if the
	// note skin changes.
	void CacheAllUsedNoteSkins();
	void FadeToFail();

	void Step(int col, TapNoteScore score, bool from_lua= false);
	void SetPressed(int col, bool from_lua= false);
	void DidTapNote(int col, TapNoteScore score, bool bright, bool from_lua= false);
	void DidHoldNote(int col, HoldNoteScore score, bool bright, bool from_lua= false);

	virtual void PushSelf( lua_State *L );

	// Allows the theme to modify the parameters to Step, SetPressed,
	// DidTapNote, and DidHoldNote before they pass on to the ghost arrows or
	// receptors. -Kyz
	LuaReference m_StepCallback;
	LuaReference m_SetPressedCallback;
	LuaReference m_DidTapNoteCallback;
	LuaReference m_DidHoldNoteCallback;

	const PlayerState *GetPlayerState() const { return m_pPlayerState; }

	int	m_iBeginMarker, m_iEndMarker;	// only used with MODE_EDIT

	// m_ColumnRenderers belongs in the protected section, but it's here in
	// public so that the Lua API can access it. -Kyz
	std::vector<NoteColumnRenderer> m_ColumnRenderers;

	void SetBeatBars(bool active);
	bool GetBeatBars();
	void SetBeatBarsAlpha(float measure, float fourth, float eighth, float sixteenth);

protected:
	void CacheNoteSkin( const RString &sNoteSkin );
	void UncacheNoteSkin( const RString &sNoteSkin );

	bool IsOnScreen( float fBeat, int iCol, int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels ) const;

	void DrawBoard( int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels );

	enum BeatBarType { measure, beat, half_beat, quarter_beat };
	void DrawBeatBar( const float fBeat, BeatBarType type, int iMeasureIndex );
	void DrawMarkerBar( int fBeat );
	void DrawAreaHighlight( int iStartBeat, int iEndBeat );
	void set_text_measure_number_for_draw(
		const float beat, const float side_sign, float x_offset,
		const float horiz_align, const RageColor& color, const RageColor& glow);
	void draw_timing_segment_text(const RString& text,
		const float beat, const float side_sign, float x_offset,
		const float horiz_align, const RageColor& color, const RageColor& glow);
	void DrawAttackText(const float beat, const Attack &attack, const RageColor& glow);
	void DrawBGChangeText(const float beat, const RString new_bg_name, const RageColor& glow);
	float GetWidth() const;

	const NoteData *m_pNoteData;

	const PlayerState*	m_pPlayerState;
	int			m_iDrawDistanceAfterTargetsPixels;	// this should be a negative number
	int			m_iDrawDistanceBeforeTargetsPixels;	// this should be a positive number
	float		m_fYReverseOffsetPixels;

	// This exists so that the board can be drawn underneath combo/judge. -Kyz
	bool m_drawing_board_primitive;

	// color arrows
	struct NoteDisplayCols
	{
		NoteDisplay		*display;
		ReceptorArrowRow	m_ReceptorArrowRow;
		GhostArrowRow		m_GhostArrowRow;
		NoteDisplayCols( int iNumCols ) { display = new NoteDisplay[iNumCols]; }
		~NoteDisplayCols() { delete [] display; }
	};

	NoteFieldRenderArgs m_FieldRenderArgs;

	/* All loaded note displays, mapped by their name. */
	std::map<RString, NoteDisplayCols *> m_NoteDisplays;
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

	bool m_bShowBeatBars;
	float m_fBarMeasureAlpha;
	float m_fBar4thAlpha;
	float m_fBar8thAlpha;
	float m_fBar16thAlpha;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
