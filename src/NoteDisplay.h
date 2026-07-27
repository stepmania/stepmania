#ifndef NOTE_DISPLAY_H
#define NOTE_DISPLAY_H

#include "ActorFrame.h"
#include "CubicSpline.h"
#include "NoteData.h"
#include "PlayerNumber.h"
#include "GameInput.h"

#include <vector>


class Sprite;
class Model;
class PlayerState;
class GhostArrowRow;
class ReceptorArrowRow;
struct TapNote;
struct HoldNoteResult;
struct NoteMetricCache_t;
/** @brief the various parts of a Note. */
enum NotePart
{
	NotePart_Tap, /**< The part representing a traditional TapNote. */
	NotePart_Mine, /**< The part representing a mine. */
	NotePart_Lift, /**< The part representing a lift note. */
	NotePart_Fake, /**< The part representing a fake note. */
	NotePart_HoldHead, /**< The part representing a hold head. */
	NotePart_HoldTail, /**< The part representing a hold tail. */
	NotePart_HoldTopCap, /**< The part representing a hold's top cap. */
	NotePart_HoldBody, /**< The part representing a hold's body. */
	NotePart_HoldBottomCap, /**< The part representing a hold's bottom cap. */
	NUM_NotePart,
	NotePart_Invalid
};

/** @brief the color type of a Note. */
enum NoteColorType
{
	NoteColorType_Denominator, /**< Color by note type. */
	NoteColorType_Progress, /**< Color by progress. */
	NoteColorType_ProgressAlternate, /**< Color by progress, except the frame boundaries are slightly later. */
	NUM_NoteColorType,
	NoteColorType_Invalid
};
const RString& NoteColorTypeToString( NoteColorType nct );
NoteColorType StringToNoteColorType( const RString& s );

struct NoteResource;

struct NoteColorActor
{
	NoteColorActor();
	~NoteColorActor();
	void Load( const RString &sButton, const RString &sElement, PlayerNumber, GameController );
	Actor *Get();
private:
	NoteResource *m_p;
};

struct NoteColorSprite
{
	NoteColorSprite();
	~NoteColorSprite();
	void Load( const RString &sButton, const RString &sElement, PlayerNumber, GameController );
	Sprite *Get();
private:
	NoteResource *m_p;
};
/** @brief What types of holds are there? */
enum HoldType
{
	hold, /**< Merely keep your foot held on the body for it to count. */
	roll, /**< Keep hitting the hold body for it to stay alive. */
	// minefield,
	NUM_HoldType,
	HoldType_Invalid
};
/** @brief Loop through each HoldType. */
#define FOREACH_HoldType( i ) FOREACH_ENUM( HoldType, i )
const RString &HoldTypeToString( HoldType ht );

enum ActiveType
{
	active,
	inactive,
	NUM_ActiveType,
	ActiveType_Invalid
};
/** @brief Loop through each ActiveType. */
#define FOREACH_ActiveType( i ) FOREACH_ENUM( ActiveType, i )
const RString &ActiveTypeToString( ActiveType at );

enum NoteColumnSplineMode
{
	NCSM_Disabled,
	NCSM_Offset,
	NCSM_Position,
	NUM_NoteColumnSplineMode,
	NoteColumnSplineMode_Invalid
};

const RString& NoteColumnSplineModeToString(NoteColumnSplineMode ncsm);
LuaDeclareType(NoteColumnSplineMode);

// A little pod struct to carry the data the NoteField needs to pass to the
// NoteDisplay during rendering.
struct NoteFieldRenderArgs
{
	const PlayerState* player_state; // to look up PlayerOptions
	float reverse_offset_pixels;
	ReceptorArrowRow* receptor_row;
	GhostArrowRow* ghost_row;
	const NoteData* note_data;
	float first_beat;
	float last_beat;
	int first_row;
	int last_row;
	float draw_pixels_before_targets;
	float draw_pixels_after_targets;
	int* selection_begin_marker;
	int* selection_end_marker;
	float selection_glow;
	float fail_fade;
	float fade_before_targets;
};

// NCSplineHandler exists to allow NoteColumnRenderer to have separate
// splines for position, rotation, and zoom, while concisely presenting the
// same interface for all three.
struct NCSplineHandler
{
	NCSplineHandler()
	{
		m_spline.redimension(3);
		m_spline.m_owned_by_actor= true;
		m_spline_mode= NCSM_Disabled;
		m_receptor_t= 0.0f;
		m_beats_per_t= 1.0f;
		m_subtract_song_beat_from_curr= true;
	}
	float BeatToTValue(float song_beat, float note_beat) const;
	void EvalForBeat(float song_beat, float note_beat, RageVector3& ret) const;
	void EvalDerivForBeat(float song_beat, float note_beat, RageVector3& ret) const;
	void EvalForReceptor(float song_beat, RageVector3& ret) const;
	static void MakeWeightedAverage(NCSplineHandler& out,
		const NCSplineHandler& from, const NCSplineHandler& to, float between);

	CubicSplineN m_spline;
	NoteColumnSplineMode m_spline_mode;
	float m_receptor_t;
	float m_beats_per_t;
	bool m_subtract_song_beat_from_curr;

	void PushSelf(lua_State* L);
};

struct NoteColumnRenderArgs
{
	void spae_pos_for_beat(const PlayerState* player_state,
		float beat, float y_offset, float y_reverse_offset,
		RageVector3& sp_pos, RageVector3& ae_pos) const;
	void spae_zoom_for_beat(const PlayerState* state, float beat,
		RageVector3& sp_zoom, RageVector3& ae_zoom, int col_num, float y_offset) const;
	void SetPRZForActor(Actor* actor,
		const RageVector3& sp_pos, const RageVector3& ae_pos,
		const RageVector3& sp_rot, const RageVector3& ae_rot,
		const RageVector3& sp_zoom, const RageVector3& ae_zoom) const;
	const NCSplineHandler* pos_handler;
	const NCSplineHandler* rot_handler;
	const NCSplineHandler* zoom_handler;
	RageColor diffuse;
	RageColor glow;
	float song_beat;
	int column;
};

/** @brief Draws TapNotes and HoldNotes. */
class NoteDisplay
{
public:
	NoteDisplay();
	~NoteDisplay();

	void Load( int iColNum, const PlayerState* pPlayerState, float fYReverseOffsetPixels );

	static void Update( float fDeltaTime );

	bool IsOnScreen( float fBeat, int iCol, int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels ) const;

	bool DrawHoldsInRange(const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args,
		const std::vector<NoteData::TrackMap::const_iterator>& tap_set);
	bool DrawTapsInRange(const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args,
		const std::vector<NoteData::TrackMap::const_iterator>& tap_set);
	/**
	 * @brief Draw the TapNote onto the NoteField.
	 * @param tn the TapNote in question.
	 * @param iCol the column.
	 * @param float fBeat the beat to draw them on.
	 * @param bOnSameRowAsHoldStart a flag to see if a hold is on the same beat.
	 * @param bOnSameRowAsRollStart a flag to see if a roll is on the same beat.
	 * @param bIsAddition a flag to see if this note was added via mods.
	 * @param fPercentFadeToFail at what point do the notes fade on failure?
	 * @param fReverseOffsetPixels How are the notes adjusted on Reverse?
	 * @param fDrawDistanceAfterTargetsPixels how much to draw after the receptors.
	 * @param fDrawDistanceBeforeTargetsPixels how much ot draw before the receptors.
	 * @param fFadeInPercentOfDrawFar when to start fading in. */
	void DrawTap(const TapNote& tn, const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args, float fBeat,
		bool bOnSameRowAsHoldStart,
		bool bOnSameRowAsRollBeat, bool bIsAddition, float fPercentFadeToFail);
	void DrawHold(const TapNote& tn, const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args, int iRow, bool bIsBeingHeld,
		const HoldNoteResult &Result,
		bool bIsAddition, float fPercentFadeToFail);

	bool DrawHoldHeadForTapsOnSameRow() const;
	bool DrawRollHeadForTapsOnSameRow() const;

private:
	void SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLength, bool bVivid );
	Actor *GetTapActor( NoteColorActor &nca, NotePart part, float fNoteBeat );
	Actor *GetHoldActor( NoteColorActor nca[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );
	Sprite *GetHoldSprite( NoteColorSprite ncs[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld );

	struct draw_hold_part_args
	{
		int y_step;
		float percent_fade_to_fail;
		float color_scale;
		float overlapped_time;
		float y_top;
		float y_bottom;
		float y_start_pos;
		float y_end_pos;
		float top_beat;
		float bottom_beat;
		bool wrapping;
		bool anchor_to_top;
		bool flip_texture_vertically;
	};

	void DrawActor(const TapNote& tn, Actor* pActor, NotePart part,
		const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args, float fYOffset, float fBeat,
		bool bIsAddition, float fPercentFadeToFail, float fColorScale,
		bool is_being_held);
	void DrawHoldPart(std::vector<Sprite*> &vpSpr,
		const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args,
		const draw_hold_part_args& part_args, bool glow, int part_type);
	void DrawHoldBodyInternal(std::vector<Sprite*>& sprite_top,
		std::vector<Sprite*>& sprite_body, std::vector<Sprite*>& sprite_bottom,
		const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args,
		draw_hold_part_args& part_args,
		const float head_minus_top, const float tail_plus_bottom,
		const float y_head, const float y_tail, const float top_beat,
		const float bottom_beat, bool glow);
	void DrawHoldBody(const TapNote& tn, const NoteFieldRenderArgs& field_args,
		const NoteColumnRenderArgs& column_args, float beat, bool being_held,
		float y_head, float y_tail, float percent_fade_to_fail,
		float color_scale, float top_beat, float bottom_beat);

	const PlayerState	*m_pPlayerState;	// to look up PlayerOptions
	NoteMetricCache_t	*cache;

	NoteColorActor		m_TapNote;
	NoteColorActor		m_TapMine;
	NoteColorActor		m_TapLift;
	NoteColorActor		m_TapFake;
	NoteColorActor		m_HoldHead[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldTopCap[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldBody[NUM_HoldType][NUM_ActiveType];
	NoteColorSprite		m_HoldBottomCap[NUM_HoldType][NUM_ActiveType];
	NoteColorActor		m_HoldTail[NUM_HoldType][NUM_ActiveType];
	float			m_fYReverseOffsetPixels;
};

// So, this is a bit screwy, and it's partly because routine forces rendering
// notes from different noteskins in the same column.
// NoteColumnRenderer exists to hold all the data needed for rendering a
// column and apply any transforms from that column's actor to the
// NoteDisplays that render the notes.
// NoteColumnRenderer is also used as a fake parent for the receptor and ghost
// actors so they can move with the rest of the column.  I didn't use
// ActorProxy because the receptor/ghost actors need to pull in the parent
// state of their rows and the parent state of the column. -Kyz

struct NoteColumnRenderer : public Actor
{
	NoteDisplay* m_displays[PLAYER_INVALID+1];
	NoteFieldRenderArgs* m_field_render_args;
	NoteColumnRenderArgs m_column_render_args;
	int m_column;

	// UpdateReceptorGhostStuff takes care of the logic for making the ghost
	// and receptor positions follow the splines.  It's called by their row
	// update functions. -Kyz
	void UpdateReceptorGhostStuff(Actor* receptor) const;
	virtual void DrawPrimitives() override;
	virtual void PushSelf(lua_State* L) override;

	struct NCR_TweenState
	{
		NCR_TweenState();
		NCSplineHandler m_pos_handler;
		NCSplineHandler m_rot_handler;
		NCSplineHandler m_zoom_handler;
		static void MakeWeightedAverage(NCR_TweenState& out,
			const NCR_TweenState& from, const NCR_TweenState& to, float between);
		bool operator==(const NCR_TweenState& other) const;
		bool operator!=(const NCR_TweenState& other) const { return !operator==(other); }
	};

	NCR_TweenState& NCR_DestTweenState()
	{
		if(NCR_Tweens.empty())
		{ return NCR_current; }
		else
		{ return NCR_Tweens.back(); }
	}
	const NCR_TweenState& NCR_DestTweenState() const { return const_cast<NoteColumnRenderer*>(this)->NCR_DestTweenState(); }

	virtual void SetCurrentTweenStart() override;
	virtual void EraseHeadTween() override;
	virtual void UpdatePercentThroughTween(float between) override;
	virtual void BeginTweening(float time, ITween* interp) override;
	virtual void StopTweening() override;
	virtual void FinishTweening() override;

	NCSplineHandler* GetPosHandler() { return &NCR_DestTweenState().m_pos_handler; }
	NCSplineHandler* GetRotHandler() { return &NCR_DestTweenState().m_rot_handler; }
	NCSplineHandler* GetZoomHandler() { return &NCR_DestTweenState().m_zoom_handler; }

	private:
	std::vector<NCR_TweenState> NCR_Tweens;
	NCR_TweenState NCR_current;
	NCR_TweenState NCR_start;
};

#endif

/**
 * NoteColumnRenderer and associated spline stuff (c) Eric Reese 2014-2015
 * @file
 * @author Brian Bugh, Ben Nordstrom, Chris Danford, Steve Checkoway (c) 2001-2006
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
