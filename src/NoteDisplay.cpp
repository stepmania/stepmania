#include "global.h"
#include "NoteDisplay.h"
#include "GameState.h"
#include "GhostArrowRow.h"
#include "NoteData.h"
#include "NoteSkinManager.h"
#include "ArrowEffects.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "ReceptorArrowRow.h"
#include "ActorUtil.h"
#include "Style.h"
#include "PlayerState.h"
#include "Sprite.h"
#include "NoteTypes.h"
#include "LuaBinding.h"
#include "Foreach.h"
#include "RageMath.h"

static const double PI_180= PI / 180.0;
static const double PI_180R= 180.0 / PI;

const RString& NoteNotePartToString( NotePart i );
/** @brief A foreach loop going through the different NoteParts. */
#define FOREACH_NotePart( i ) FOREACH_ENUM( NotePart, i )

static const char *NotePartNames[] = {
	"TapNote",
	"TapMine",
	"TapLift",
	"HoldHead",
	"HoldTail",
	"HoldTopCap",
	"HoldBody",
	"HoldBottomCap",
};
XToString( NotePart );
LuaXType( NotePart );

static const char *NoteColorTypeNames[] = {
	"Denominator",
	"Progress",
};
XToString( NoteColorType );
StringToX( NoteColorType );
LuaXType( NoteColorType );

static const char* NoteColumnSplineModeNames[] = {
	"Disabled",
	"Offset",
	"Position",
};
XToString(NoteColumnSplineMode);
StringToX(NoteColumnSplineMode);
LuaXType(NoteColumnSplineMode);

static bool IsVectorZero( const RageVector2 &v )
{
	return v.x == 0  &&  v.y == 0;
}

// Don't require that NoteSkins have more than 8 colors.  Using 9 colors to display 192nd notes
// would double the number of texture memory needed for many NoteSkin graphics versus just having
// 8 colors.
static const NoteType MAX_DISPLAY_NOTE_TYPE = (NoteType)7;

// cache
struct NoteMetricCache_t
{
	bool m_bDrawHoldHeadForTapsOnSameRow;
	bool m_bDrawRollHeadForTapsOnSameRow;
	bool m_bTapHoldRollOnRowMeansHold;
	float m_fAnimationLength[NUM_NotePart];
	bool m_bAnimationIsVivid[NUM_NotePart];
	RageVector2 m_fAdditionTextureCoordOffset[NUM_NotePart];
	RageVector2 m_fNoteColorTextureCoordSpacing[NUM_NotePart];

	int m_iNoteColorCount[NUM_NotePart];
	NoteColorType m_NoteColorType[NUM_NotePart];

	//For animation based on beats or seconds -DaisuMaster
	bool m_bAnimationBasedOnBeats;
	bool m_bHoldHeadIsAboveWavyParts;
	bool m_bHoldTailIsAboveWavyParts;
	int m_iStartDrawingHoldBodyOffsetFromHead;
	int m_iStopDrawingHoldBodyOffsetFromTail;
	float m_fHoldLetGoGrayPercent;
	bool m_bFlipHeadAndTailWhenReverse;
	bool m_bFlipHoldBodyWhenReverse;
	bool m_bTopHoldAnchorWhenReverse;
	bool m_bHoldActiveIsAddLayer;

	void Load( const RString &sButton );
} *NoteMetricCache;

void NoteMetricCache_t::Load( const RString &sButton )
{
	m_bDrawHoldHeadForTapsOnSameRow = NOTESKIN->GetMetricB(sButton,"DrawHoldHeadForTapsOnSameRow");
	m_bDrawRollHeadForTapsOnSameRow = NOTESKIN->GetMetricB(sButton,"DrawRollHeadForTapsOnSameRow");
	m_bTapHoldRollOnRowMeansHold = NOTESKIN->GetMetricB(sButton,"TapHoldRollOnRowMeansHold");
	FOREACH_NotePart( p )
	{
		const RString &s = NotePartToString(p);
		m_fAnimationLength[p] = NOTESKIN->GetMetricF(sButton,s+"AnimationLength");
		m_bAnimationIsVivid[p] = NOTESKIN->GetMetricB(sButton,s+"AnimationIsVivid");
		m_fAdditionTextureCoordOffset[p].x = NOTESKIN->GetMetricF(sButton,s+"AdditionTextureCoordOffsetX");
		m_fAdditionTextureCoordOffset[p].y = NOTESKIN->GetMetricF(sButton,s+"AdditionTextureCoordOffsetY");
		m_fNoteColorTextureCoordSpacing[p].x = NOTESKIN->GetMetricF(sButton,s+"NoteColorTextureCoordSpacingX");
		m_fNoteColorTextureCoordSpacing[p].y = NOTESKIN->GetMetricF(sButton,s+"NoteColorTextureCoordSpacingY");
		m_iNoteColorCount[p] = NOTESKIN->GetMetricI(sButton,s+"NoteColorCount");

		RString ct = NOTESKIN->GetMetric(sButton,s+"NoteColorType");
		m_NoteColorType[p] = StringToNoteColorType(ct);
	}
	//I was here -DaisuMaster
	m_bAnimationBasedOnBeats = NOTESKIN->GetMetricB(sButton,"AnimationIsBeatBased");
	m_bHoldHeadIsAboveWavyParts =		NOTESKIN->GetMetricB(sButton,"HoldHeadIsAboveWavyParts");
	m_bHoldTailIsAboveWavyParts =		NOTESKIN->GetMetricB(sButton,"HoldTailIsAboveWavyParts");
	m_iStartDrawingHoldBodyOffsetFromHead =	NOTESKIN->GetMetricI(sButton,"StartDrawingHoldBodyOffsetFromHead");
	m_iStopDrawingHoldBodyOffsetFromTail =	NOTESKIN->GetMetricI(sButton,"StopDrawingHoldBodyOffsetFromTail");
	m_fHoldLetGoGrayPercent =		NOTESKIN->GetMetricF(sButton,"HoldLetGoGrayPercent");
	m_bFlipHeadAndTailWhenReverse =		NOTESKIN->GetMetricB(sButton,"FlipHeadAndTailWhenReverse");
	m_bFlipHoldBodyWhenReverse =		NOTESKIN->GetMetricB(sButton,"FlipHoldBodyWhenReverse");
	m_bTopHoldAnchorWhenReverse =		NOTESKIN->GetMetricB(sButton,"TopHoldAnchorWhenReverse");
	m_bHoldActiveIsAddLayer =		NOTESKIN->GetMetricB(sButton,"HoldActiveIsAddLayer");
}


struct NoteSkinAndPath
{
	NoteSkinAndPath( const RString sNoteSkin_, const RString sPath_, const PlayerNumber pn_, const GameController gc_ ) : sNoteSkin(sNoteSkin_), sPath(sPath_), pn(pn_), gc(gc_) { }
	RString sNoteSkin;
	RString sPath;
	PlayerNumber pn;
	GameController gc;
	bool operator<( const NoteSkinAndPath &other ) const
	{
		int cmp = strcmp(sNoteSkin, other.sNoteSkin);

		if( cmp < 0 )
		{
			return true;
		}
		else if( cmp == 0 )
		{
			if( sPath < other.sPath )
			{
				return true;
			}
			else if( sPath == other.sPath )
			{
				if ( pn < other.pn )
					return true;
				else if ( pn == other.pn )
					return gc < other.gc;
				else
					return false;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
};

struct NoteResource
{
	NoteResource( const NoteSkinAndPath &nsap ): m_nsap(nsap)
	{
		m_iRefCount = 0;
		m_pActor = NULL;
	}

	~NoteResource()
	{
		delete m_pActor;
	}

	const NoteSkinAndPath m_nsap; // should be refcounted along with g_NoteResource[]
	int m_iRefCount;
	Actor *m_pActor; // todo: AutoActor me? -aj
};

static map<NoteSkinAndPath, NoteResource *> g_NoteResource;

static NoteResource *MakeNoteResource( const RString &sButton, const RString &sElement, PlayerNumber pn, GameController gc, bool bSpriteOnly )
{
	RString sElementAndType = ssprintf( "%s, %s", sButton.c_str(), sElement.c_str() );
	NoteSkinAndPath nsap( NOTESKIN->GetCurrentNoteSkin(), sElementAndType, pn, gc );

	map<NoteSkinAndPath, NoteResource *>::iterator it = g_NoteResource.find( nsap );
	if( it == g_NoteResource.end() )
	{
		NoteResource *pRes = new NoteResource( nsap );

		NOTESKIN->SetPlayerNumber( pn );
		NOTESKIN->SetGameController( gc );

		pRes->m_pActor = NOTESKIN->LoadActor( sButton, sElement, NULL, bSpriteOnly );
		ASSERT( pRes->m_pActor != NULL );

		g_NoteResource[nsap] = pRes;
		it = g_NoteResource.find( nsap );
	}

	NoteResource *pRet = it->second;
	++pRet->m_iRefCount;
	return pRet;
}

static void DeleteNoteResource( NoteResource *pRes )
{
	ASSERT( pRes != NULL );

	ASSERT_M( pRes->m_iRefCount > 0, ssprintf("RefCount %i > 0", pRes->m_iRefCount) );
	--pRes->m_iRefCount;
	if( pRes->m_iRefCount )
		return;

	g_NoteResource.erase( pRes->m_nsap );
	delete pRes;
}

/* NoteColorActor */

NoteColorActor::NoteColorActor()
{
	m_p = NULL;
}

NoteColorActor::~NoteColorActor()
{
	if( m_p )
		DeleteNoteResource( m_p );
}

void NoteColorActor::Load( const RString &sButton, const RString &sElement, PlayerNumber pn, GameController gc )
{
	m_p = MakeNoteResource( sButton, sElement, pn, gc, false );
}


Actor *NoteColorActor::Get()
{
	return m_p->m_pActor;
}

/* NoteColorSprite */

NoteColorSprite::NoteColorSprite()
{
	m_p = NULL;
}

NoteColorSprite::~NoteColorSprite()
{
	if( m_p )
		DeleteNoteResource( m_p );
}

void NoteColorSprite::Load( const RString &sButton, const RString &sElement, PlayerNumber pn, GameController gc )
{
	m_p = MakeNoteResource( sButton, sElement, pn, gc, true );
}

Sprite *NoteColorSprite::Get()
{
	return dynamic_cast<Sprite *>( m_p->m_pActor );
}

static const char *HoldTypeNames[] = {
	"Hold",
	"Roll",
	//"Minefield",
};
XToString( HoldType );

static const char *ActiveTypeNames[] = {
	"Active",
	"Inactive",
};
XToString( ActiveType );



float NCSplineHandler::BeatToTValue(float song_beat, float note_beat) const
{
	float relative_beat= note_beat;
	// This allows someone to do something really fancy like having a spline
	// that extends the length of the song.  Think of arrows tracing a path
	// as the song progresses. -Kyz
	if(m_subtract_song_beat_from_curr)
	{
		relative_beat-= song_beat;
		return (relative_beat / m_beats_per_t) - m_receptor_t;
	}
	return relative_beat / m_beats_per_t;
}

void NCSplineHandler::EvalForBeat(float song_beat, float note_beat, vector<float>& ret) const
{
	float t_value= BeatToTValue(song_beat, note_beat);
	m_spline.evaluate(t_value, ret);
}

void NCSplineHandler::EvalDerivForBeat(float song_beat, float note_beat, vector<float>& ret) const
{
	float t_value= BeatToTValue(song_beat, note_beat);
	m_spline.evaluate_derivative(t_value, ret);
}

void NCSplineHandler::EvalForReceptor(float song_beat, vector<float>& ret) const
{
	float t_value= m_receptor_t;
	if(!m_subtract_song_beat_from_curr)
	{
		t_value= song_beat / m_beats_per_t;
	}
	m_spline.evaluate(t_value, ret);
}

void NCSplineHandler::MakeWeightedAverage(NCSplineHandler& out,
		const NCSplineHandler& from, const NCSplineHandler& to, float between)
{
#define BOOLS_FROM_CLOSEST(closest) \
	out.m_spline_mode= closest.m_spline_mode; \
	out.m_subtract_song_beat_from_curr= closest.m_subtract_song_beat_from_curr;
	if(between >= 0.5f)
	{
		BOOLS_FROM_CLOSEST(to);
	}
	else
	{
		BOOLS_FROM_CLOSEST(from);
	}
#undef BOOLS_FROM_CLOSEST
	CubicSplineN::weighted_average(out.m_spline, from.m_spline, to.m_spline,
		between);
}

void NoteColumnRenderArgs::spae_pos_for_beat(const PlayerState* state,
	float beat, float y_offset, float y_reverse_offset,
	vector<float>& sp_pos, vector<float>& ae_pos) const
{
	switch(pos_handler->m_spline_mode)
	{
		case NCSM_Disabled:
			ArrowEffects::GetXYZPos(state, column, y_offset, y_reverse_offset, ae_pos);
			sp_pos.resize(3);
			// Sure, resize is supposed to call the default constructor, and for
			// numbers the default constructor is supposed to set it to zero, but
			// I got bit for relying on that once. -Kyz
			sp_pos[0]= sp_pos[1]= sp_pos[2]= 0.0f;
			break;
		case NCSM_Offset:
			ArrowEffects::GetXYZPos(state, column, y_offset, y_reverse_offset, ae_pos);
			pos_handler->EvalForBeat(song_beat, beat, sp_pos);
			break;
		case NCSM_Position:
			pos_handler->EvalForBeat(song_beat, beat, sp_pos);
			break;
	}
}
void NoteColumnRenderArgs::spae_zoom_for_beat(const PlayerState* state, float beat,
	vector<float>& sp_zoom, vector<float>& ae_zoom) const
{
	switch(zoom_handler->m_spline_mode)
	{
		case NCSM_Disabled:
			ae_zoom[0]= ae_zoom[1]= ae_zoom[2]= ArrowEffects::GetZoom(state);
			sp_zoom.resize(3);
			sp_zoom[0]= sp_zoom[1]= sp_zoom[2]= 0.0f;
			break;
		case NCSM_Offset:
			ae_zoom[0]= ae_zoom[1]= ae_zoom[2]= ArrowEffects::GetZoom(state);
			zoom_handler->EvalForBeat(song_beat, beat, sp_zoom);
			break;
		case NCSM_Position:
			zoom_handler->EvalForBeat(song_beat, beat, sp_zoom);
			break;
	}
}
void NoteColumnRenderArgs::SetPRZForActor(Actor* actor,
	const vector<float>& sp_pos, const vector<float>& ae_pos,
	const vector<float>& sp_rot, const vector<float>& ae_rot,
	const vector<float>& sp_zoom, const vector<float>& ae_zoom) const
{
	actor->SetX(sp_pos[0] + ae_pos[0]);
	actor->SetY(sp_pos[1] + ae_pos[1]);
	actor->SetZ(sp_pos[2] + ae_pos[2]);
	actor->SetRotationX(sp_rot[0] * PI_180R + ae_rot[0]);
	actor->SetRotationY(sp_rot[1] * PI_180R + ae_rot[1]);
	actor->SetRotationZ(sp_rot[2] * PI_180R + ae_rot[2]);
	actor->SetZoomX(sp_zoom[0] + ae_zoom[0]);
	actor->SetZoomY(sp_zoom[1] + ae_zoom[1]);
	actor->SetZoomZ(sp_zoom[2] + ae_zoom[2]);
}


NoteDisplay::NoteDisplay()
{
	cache = new NoteMetricCache_t;
}

NoteDisplay::~NoteDisplay()
{
	delete cache;
}

void NoteDisplay::Load( int iColNum, const PlayerState* pPlayerState, float fYReverseOffsetPixels )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffsetPixels;

	const PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	vector<GameInput> GameI;
	GAMESTATE->GetCurrentStyle(pPlayerState->m_PlayerNumber)->StyleInputToGameInput( iColNum, pn, GameI );

	const RString &sButton = GAMESTATE->GetCurrentStyle(pPlayerState->m_PlayerNumber)->ColToButtonName( iColNum );

	cache->Load( sButton );

	// "normal" note types
	m_TapNote.Load(		sButton, "Tap Note", pn, GameI[0].controller );
	//m_TapAdd.Load(		sButton, "Tap Addition", pn, GameI.controller );
	m_TapMine.Load(		sButton, "Tap Mine", pn, GameI[0].controller );
	m_TapLift.Load(		sButton, "Tap Lift", pn, GameI[0].controller );
	m_TapFake.Load(		sButton, "Tap Fake", pn, GameI[0].controller );

	// hold types
	FOREACH_HoldType( ht )
	{
		FOREACH_ActiveType( at )
		{
			m_HoldHead[ht][at].Load(	sButton, HoldTypeToString(ht)+" Head "+ActiveTypeToString(at), pn, GameI[0].controller );
			m_HoldTopCap[ht][at].Load(	sButton, HoldTypeToString(ht)+" Topcap "+ActiveTypeToString(at), pn, GameI[0].controller );
			m_HoldBody[ht][at].Load(	sButton, HoldTypeToString(ht)+" Body "+ActiveTypeToString(at), pn, GameI[0].controller );
			m_HoldBottomCap[ht][at].Load(	sButton, HoldTypeToString(ht)+" Bottomcap "+ActiveTypeToString(at), pn, GameI[0].controller );
			m_HoldTail[ht][at].Load(	sButton, HoldTypeToString(ht)+" Tail "+ActiveTypeToString(at), pn, GameI[0].controller );
		}
	}
}

inline float NoteRowToVisibleBeat( const PlayerState *pPlayerState, int iRow )
{
	return NoteRowToBeat(iRow);
}

bool NoteDisplay::IsOnScreen( float fBeat, int iCol, int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels ) const
{
	// IMPORTANT:  Do not modify this function without also modifying the
	// version that is in NoteField.cpp or coming up with a good way to
	// merge them. -Kyz
	// TRICKY: If boomerang is on, then ones in the range
	// [iFirstRowToDraw,iLastRowToDraw] aren't necessarily visible.
	// Test to see if this beat is visible before drawing.
	float fYOffset = ArrowEffects::GetYOffset( m_pPlayerState, iCol, fBeat );
	if( fYOffset > iDrawDistanceBeforeTargetsPixels )	// off screen
		return false;
	if( fYOffset < iDrawDistanceAfterTargetsPixels )	// off screen
		return false;

	return true;
}

bool NoteDisplay::DrawHoldsInRange(const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args,
	const vector<NoteData::TrackMap::const_iterator>& tap_set)
{
	bool any_upcoming = false;
	for(vector<NoteData::TrackMap::const_iterator>::const_iterator tapit=
		tap_set.begin(); tapit != tap_set.end(); ++tapit)
	{
		const TapNote& tn= (*tapit)->second;
		const HoldNoteResult& result= tn.HoldResult;
		int start_row= (*tapit)->first;
		int end_row = start_row + tn.iDuration;

		// TRICKY: If boomerang is on, then all notes in the range
		// [first_row,last_row] aren't necessarily visible.
		// Test every note to make sure it's on screen before drawing
		float throw_away;
		bool start_past_peak = false;
		bool end_past_peak = false;
		float start_y	= ArrowEffects::GetYOffset(m_pPlayerState, column_args.column,
			NoteRowToVisibleBeat(m_pPlayerState, start_row), throw_away,
			start_past_peak);
		float end_y	= ArrowEffects::GetYOffset(m_pPlayerState, column_args.column,
			NoteRowToVisibleBeat(m_pPlayerState, end_row), throw_away,
			end_past_peak);
		bool tail_visible = field_args.draw_pixels_after_targets <= end_y &&
			end_y <= field_args.draw_pixels_before_targets;
		bool head_visible = field_args.draw_pixels_after_targets <= start_y  &&
			start_y <= field_args.draw_pixels_before_targets;
		bool straddling_visible = start_y <= field_args.draw_pixels_after_targets &&
			field_args.draw_pixels_before_targets <= end_y;
		bool straddling_peak = start_past_peak && !end_past_peak;
		if(!(tail_visible || head_visible || straddling_visible || straddling_peak))
		{
			//LOG->Trace( "skip drawing this hold." );
			continue;	// skip
		}

		bool is_addition = (tn.source == TapNoteSource_Addition);
		bool hopo_possible = (tn.bHopoPossible);
		bool use_addition_coloring = is_addition || hopo_possible;
		const bool hold_ghost_showing = tn.HoldResult.bActive  &&  tn.HoldResult.fLife > 0;
		const bool is_holding = tn.HoldResult.bHeld;
		if(hold_ghost_showing)
		{
			field_args.ghost_row->SetHoldShowing(column_args.column, tn);
		}

		ASSERT_M(NoteRowToBeat(start_row) > -2000, ssprintf("%i %i %i", start_row, end_row, column_args.column));

		bool in_selection_range = false;
		if(*field_args.selection_begin_marker != -1 && *field_args.selection_end_marker != -1)
		{
			in_selection_range = (*field_args.selection_begin_marker <= start_row &&
				end_row < *field_args.selection_end_marker);
		}

		DrawHold(tn, field_args, column_args, start_row, is_holding, result,
			use_addition_coloring,
			in_selection_range ? field_args.selection_glow : field_args.fail_fade);

		bool note_upcoming = NoteRowToBeat(start_row) >
			m_pPlayerState->GetDisplayedPosition().m_fSongBeat;
		any_upcoming |= note_upcoming;
	}
	return any_upcoming;
}

bool NoteDisplay::DrawTapsInRange(const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args,
	const vector<NoteData::TrackMap::const_iterator>& tap_set)
{
	bool any_upcoming= false;
	// draw notes from furthest to closest
	for(vector<NoteData::TrackMap::const_iterator>::const_iterator tapit=
		tap_set.begin(); tapit != tap_set.end(); ++tapit)
	{
		int tap_row= (*tapit)->first;
		const TapNote& tn= (*tapit)->second;

		// TRICKY: If boomerang is on, then all notes in the range
		// [first_row,last_row] aren't necessarily visible.
		// Test every note to make sure it's on screen before drawing.
		if(!IsOnScreen(NoteRowToBeat(tap_row), column_args.column,
				field_args.draw_pixels_after_targets, field_args.draw_pixels_before_targets))
		{
			continue; // skip
		}

		// Hm, this assert used to pass the first and last rows to draw, when it
		// was in NoteField, but those aren't available here.
		// Well, anyone who has to investigate hitting it can use a debugger to
		// discover the values, hopefully. -Kyz
		ASSERT_M(NoteRowToBeat(tap_row) > -2000,
			ssprintf("Invalid tap_row: %i, %f %f",
				tap_row,
				m_pPlayerState->GetDisplayedPosition().m_fSongBeat,
				m_pPlayerState->GetDisplayedPosition().m_fMusicSeconds));

		// See if there is a hold step that begins on this index.
		// Only do this if the noteskin cares.
		bool hold_begins_on_this_beat = false;
		if(DrawHoldHeadForTapsOnSameRow())
		{
			for(int c2= 0; c2 < field_args.note_data->GetNumTracks(); ++c2)
			{
				const TapNote &tmp = field_args.note_data->GetTapNote(c2, tap_row);
				if(tmp.type == TapNoteType_HoldHead &&
					tmp.subType == TapNoteSubType_Hold)
				{
					hold_begins_on_this_beat = true;
					break;
				}
			}
		}

		// do the same for a roll.
		bool roll_begins_on_this_beat = false;
		if(DrawRollHeadForTapsOnSameRow())
		{
			for(int c2= 0; c2 < field_args.note_data->GetNumTracks(); ++c2)
			{
				const TapNote &tmp = field_args.note_data->GetTapNote(c2, tap_row);
				if(tmp.type == TapNoteType_HoldHead &&
					tmp.subType == TapNoteSubType_Roll)
				{
					roll_begins_on_this_beat = true;
					break;
				}
			}
		}

		bool in_selection_range = false;
		if(*field_args.selection_begin_marker != -1 && *field_args.selection_end_marker != -1)
		{
			in_selection_range = *field_args.selection_begin_marker <= tap_row &&
				tap_row < *field_args.selection_end_marker;
		}

		bool is_addition = (tn.source == TapNoteSource_Addition);
		bool hopo_possible = (tn.bHopoPossible);
		bool use_addition_coloring = is_addition || hopo_possible;
		DrawTap(tn, field_args, column_args,
			NoteRowToVisibleBeat(m_pPlayerState, tap_row),
			hold_begins_on_this_beat, roll_begins_on_this_beat,
			use_addition_coloring,
			in_selection_range ? field_args.selection_glow : field_args.fail_fade);

		any_upcoming |= NoteRowToBeat(tap_row) >
			m_pPlayerState->GetDisplayedPosition().m_fSongBeat;

		if(!PREFSMAN->m_FastNoteRendering)
		{
			DISPLAY->ClearZBuffer();
		}
	}
	return any_upcoming;
}

bool NoteDisplay::DrawHoldHeadForTapsOnSameRow() const
{
	return cache->m_bDrawHoldHeadForTapsOnSameRow;
}

bool NoteDisplay::DrawRollHeadForTapsOnSameRow() const
{
	return cache->m_bDrawRollHeadForTapsOnSameRow;
}

void NoteDisplay::Update( float fDeltaTime )
{
	/* This function is static: it's called once per game loop, not once per
	 * NoteDisplay.  Update each cached item exactly once. */
	map<NoteSkinAndPath, NoteResource *>::iterator it;
	for( it = g_NoteResource.begin(); it != g_NoteResource.end(); ++it )
	{
		NoteResource *pRes = it->second;
		pRes->m_pActor->Update( fDeltaTime );
	}
}

void NoteDisplay::SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLength, bool bVivid )
{
	/* -inf ... inf */
	float fBeatOrSecond = cache->m_bAnimationBasedOnBeats ? m_pPlayerState->m_Position.m_fSongBeat : m_pPlayerState->m_Position.m_fMusicSeconds;
	/* -len ... +len */
	float fPercentIntoAnimation = fmodf( fBeatOrSecond, fAnimationLength );
	/* -1 ... 1 */
	fPercentIntoAnimation /= fAnimationLength;

	if( bVivid )
	{
		float fNoteBeatFraction = fmodf( fNoteBeat, 1.0f );

		const float fInterval = 1.f / fAnimationLength;
		fPercentIntoAnimation += QuantizeDown( fNoteBeatFraction, fInterval );

		// just in case somehow we're majorly negative with the subtraction
		wrap( fPercentIntoAnimation, 1.f );
	}
	else
	{
		/* 0 ... 1, wrapped */
		if( fPercentIntoAnimation < 0 )
			fPercentIntoAnimation += 1.0f;
	}

	float fLengthSeconds = actorToSet.GetAnimationLengthSeconds();
	actorToSet.SetSecondsIntoAnimation( fPercentIntoAnimation*fLengthSeconds );
}

Actor *NoteDisplay::GetTapActor( NoteColorActor &nca, NotePart part, float fNoteBeat )
{
	Actor *pActorOut = nca.Get();

	SetActiveFrame( fNoteBeat, *pActorOut, cache->m_fAnimationLength[part], cache->m_bAnimationIsVivid[part] );
	return pActorOut;
}

Actor *NoteDisplay::GetHoldActor( NoteColorActor nca[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld )
{
	return GetTapActor( nca[bIsRoll ? roll:hold][bIsBeingHeld ? active:inactive], part, fNoteBeat );
}

Sprite *NoteDisplay::GetHoldSprite( NoteColorSprite ncs[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld )
{
	Sprite *pSpriteOut = ncs[bIsRoll ? roll:hold][bIsBeingHeld ? active:inactive].Get();

	SetActiveFrame( fNoteBeat, *pSpriteOut, cache->m_fAnimationLength[part], cache->m_bAnimationIsVivid[part] );
	return pSpriteOut;
}

static float ArrowGetAlphaOrGlow( bool bGlow, const PlayerState* pPlayerState, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar )
{
	if( bGlow )
		return ArrowEffects::GetGlow( pPlayerState, iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	else
		return ArrowEffects::GetAlpha( pPlayerState, iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar  );
}

struct StripBuffer
{
	enum { size = 512 };
	RageSpriteVertex *buf;
	RageSpriteVertex *v;
	StripBuffer()
	{
		buf = (RageSpriteVertex *) malloc( size * sizeof(RageSpriteVertex) );
		Init();
	}
	~StripBuffer()
	{
		free( buf );
	}

	void Init()
	{
		v = buf;
	}
	void Draw()
	{
		DISPLAY->DrawSymmetricQuadStrip( buf, v-buf );
	}
	int Used() const { return v - buf; }
	int Free() const { return size - Used(); }
};

void NoteDisplay::DrawHoldPart(vector<Sprite*> &vpSpr,
	const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args, int fYStep,
	float fPercentFadeToFail, float fColorScale, bool bGlow,
	float fOverlappedTime, float fYTop, float fYBottom, float fYStartPos,
	float fYEndPos, bool bWrapping, bool bAnchorToTop,
	bool bFlipTextureVertically, float top_beat, float bottom_beat)
{
	ASSERT( !vpSpr.empty() );

	float ae_zoom= ArrowEffects::GetZoom(m_pPlayerState);
	Sprite *pSprite = vpSpr.front();

	// draw manually in small segments
	RectF rect = *pSprite->GetCurrentTextureCoordRect();
	if( bFlipTextureVertically )
		swap( rect.top, rect.bottom );
	const float fFrameWidth		= pSprite->GetUnzoomedWidth();
	const float fFrameHeight	= pSprite->GetUnzoomedHeight() * ae_zoom;

	/* Only draw the section that's within the range specified.  If a hold note is
	 * very long, don't process or draw the part outside of the range.  Don't change
	 * fYTop or fYBottom; they need to be left alone to calculate texture coordinates. */
	fYStartPos = max( fYTop, fYStartPos );
	fYEndPos = min( fYBottom, fYEndPos );

	if( bGlow )
		fColorScale = 1;

	// top to bottom
	bool bAllAreTransparent = true;
	bool bLast = false;
	float fAddToTexCoord = 0;

	if( !bAnchorToTop )
	{
		float fTexCoordBottom		= SCALE( fYBottom - fYTop, 0, fFrameHeight, rect.top, rect.bottom );
		float fWantTexCoordBottom	= ceilf( fTexCoordBottom - 0.0001f );
		fAddToTexCoord = fWantTexCoordBottom - fTexCoordBottom;
	}

	if( bWrapping )
	{
		/* For very large hold notes, shift the texture coordinates to be near 0, so we
		 * don't send very large values to the renderer. */
		const float fDistFromTop	= fYStartPos - fYTop;
		float fTexCoordTop		= SCALE( fDistFromTop, 0, fFrameHeight, rect.top, rect.bottom );
		fTexCoordTop += fAddToTexCoord;
		fAddToTexCoord -= floorf( fTexCoordTop );
	}

	DISPLAY->ClearAllTextures();

	const float fTexCoordLeft	= rect.left;
	const float fTexCoordRight	= rect.right;
	const float fTexCoordCenter	= (fTexCoordLeft+fTexCoordRight)/2;

	// pos_z_vec will be used later to orient the hold.  Read below. -Kyz
	static const RageVector3 pos_z_vec(0.0f, 0.0f, 1.0f);
	static const RageVector3 pos_y_vec(0.0f, 1.0f, 0.0f);

	StripBuffer queue;
	for( float fY = fYStartPos; !bLast; fY += fYStep )
	{
		if( fY >= fYEndPos )
		{
			fY = fYEndPos;
			bLast = true;
		}

		const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, column_args.column, fY, m_fYReverseOffsetPixels );

		float cur_beat= top_beat;
		if(top_beat != bottom_beat)
		{
			cur_beat= SCALE(fY, fYTop, fYBottom, top_beat, bottom_beat);
		}

		// Fun times ahead with vector math.  If the notes are being moved by the
		// position spline, the vectors used to position the edges of the strip
		// need to be adjusted or the hold will vanish when the notes move
		// horizontally.
		// To accomplish this, we use the derivative at the current point from
		// AE and the position spline.  That gives us the forward vector for the
		// strip, pointing to where the next center vert will be. (step 1)
		// The vectors pointing left and right to the edges of the strip are
		// obtained from the cross product of the forward vector and pos_z_vec.
		// (unless the forward vec is too close to pos_z_vec or -pos_z_vec, in
		// which case pos_y_vec is used)  The result of a cross product is a
		// vector perpendicular to both, so forward crossed with pos_z_vec gives
		// us the left vector.  Right is of course -left. (step 2)
		// After that step, the left and right vectors need to be rotated around
		// the forward vector axis by the y rotation value, to allow the hold to
		// twist. (step 3)
		// Steps will be labeled where they occur below. -Kyz
		// TODO:  Figure out whether it's worth the time investment to figure out
		// a way to skip the complex vector handling if the spline is disabled.

		vector<float> sp_pos;
		vector<float> sp_pos_forward;
		vector<float> sp_rot;
		vector<float> sp_zoom;
		vector<float> ae_pos(3, 0.0f);
		vector<float> ae_rot(3, 0.0f);

		// (step 1 of vector handling, part 1)
		// ArrowEffects only contributes to the Y component of the vector to
		// maintain the old behavior of how holds are drawn when they wave back
		// and forth. -Kyz
		RageVector3 render_forward(0.0f, 1.0f, 0.0f);
		column_args.spae_pos_for_beat(m_pPlayerState, cur_beat,
			fYOffset, m_fYReverseOffsetPixels, sp_pos, ae_pos);
		// fX and fZ are sp_pos[0] + ae_pos[0] and sp_pos[2] + ae_pos[2]. -Kyz
		// fY is the actual y position that should be used, not whatever spae
		// fetched from ArrowEffects. -Kyz
		switch(column_args.pos_handler->m_spline_mode)
		{
			case NCSM_Disabled:
				ae_pos[1]= fY;
				sp_pos_forward.resize(3);
				sp_pos_forward[0]= sp_pos_forward[1]= sp_pos_forward[2]= 0.0f;
				break;
			case NCSM_Offset:
				ae_pos[1]= fY;
				column_args.pos_handler->EvalDerivForBeat(column_args.song_beat, cur_beat, sp_pos_forward);
				VectorFloatNormalize(sp_pos_forward);
				break;
			case NCSM_Position:
				ae_pos[1]= 0.0f;
				render_forward.y= 0.0f;
				column_args.pos_handler->EvalDerivForBeat(column_args.song_beat, cur_beat, sp_pos_forward);
				VectorFloatNormalize(sp_pos_forward);
				break;
		}

		render_forward.x+= sp_pos_forward[0];
		render_forward.y+= sp_pos_forward[1];
		render_forward.z+= sp_pos_forward[2];
		// Normalize the vector so it'll be easy to test when determining whether
		// to use pos_z_vec or pos_y_vec for the cross product in step 2.
		RageVec3Normalize(&render_forward, &render_forward);

		// Holds are only affected by the x axis of the zoom spline because they
		// are flat sprites. -Kyz
		float render_width= fFrameWidth;
		switch(column_args.zoom_handler->m_spline_mode)
		{
			case NCSM_Disabled:
				render_width= fFrameWidth * ae_zoom;
				break;
			case NCSM_Offset:
				column_args.zoom_handler->EvalForBeat(column_args.song_beat, cur_beat, sp_zoom);
				render_width= fFrameWidth * (ae_zoom + sp_zoom[0]);
				break;
			case NCSM_Position:
				column_args.zoom_handler->EvalForBeat(column_args.song_beat, cur_beat, sp_zoom);
				render_width= fFrameWidth * sp_zoom[0];
				break;
		}

		const float fFrameWidthScale	= ArrowEffects::GetFrameWidthScale( m_pPlayerState, fYOffset, fOverlappedTime );
		const float fScaledFrameWidth	= render_width * fFrameWidthScale;

		// Can't use the same code as for taps because hold bodies can only rotate
		// around the y axis. -Kyz
		switch(column_args.rot_handler->m_spline_mode)
		{
			case NCSM_Disabled:
				// XXX: Actor rotations use degrees, Math uses radians. Convert here.
				ae_rot[1]= ArrowEffects::GetRotationY(m_pPlayerState, fYOffset) * PI_180;
				sp_rot.resize(3);
				sp_rot[0]= sp_rot[1]= sp_rot[2]= 0.0f;
				break;
			case NCSM_Offset:
				ae_rot[1]= ArrowEffects::GetRotationY(m_pPlayerState, fYOffset) * PI_180;
				column_args.rot_handler->EvalForBeat(column_args.song_beat, cur_beat, sp_rot);
				break;
			case NCSM_Position:
				column_args.rot_handler->EvalForBeat(column_args.song_beat, cur_beat, sp_rot);
				break;
		}

		RageVector3 center_vert(sp_pos[0] + ae_pos[0],
			sp_pos[1] + ae_pos[1], sp_pos[2] + ae_pos[2]);

		// Special case for hold caps, which have the same top and bottom beat.
		if(top_beat == bottom_beat && fY != fYStartPos)
		{
			center_vert.x+= render_forward.x;
			center_vert.y+= render_forward.y;
			center_vert.z+= render_forward.z;
		}

		const float render_roty= (sp_rot[1] + ae_rot[1]);

		// (step 2 of vector handling)
		RageVector3 render_left;
		if(abs(render_forward.z) > 0.9f) // 0.9 arbitrariliy picked.
		{
			RageVec3Cross(&render_left, &pos_y_vec, &render_forward);
		}
		else
		{
			RageVec3Cross(&render_left, &pos_z_vec, &render_forward);
		}
		RageAARotate(&render_left, &render_forward, render_roty);
		const float half_width= fScaledFrameWidth * .5f;
		render_left.x*= half_width;
		render_left.y*= half_width;
		render_left.z*= half_width;

		const RageVector3 left_vert(center_vert.x + render_left.x,
			center_vert.y + render_left.y, center_vert.z + render_left.z);
		const RageVector3 right_vert(center_vert.x - render_left.x,
			center_vert.y - render_left.y, center_vert.z - render_left.z);

		const float fDistFromTop	= fY - fYTop;
		float fTexCoordTop		= SCALE( fDistFromTop, 0, fFrameHeight, rect.top, rect.bottom );
		fTexCoordTop += fAddToTexCoord;

		const float fAlpha		= ArrowGetAlphaOrGlow( bGlow, m_pPlayerState, column_args.column, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, field_args.draw_pixels_before_targets, field_args.fade_before_targets );
		const RageColor color= RageColor(
			column_args.diffuse.r * fColorScale,
			column_args.diffuse.g * fColorScale,
			column_args.diffuse.b * fColorScale,
			column_args.diffuse.a * fAlpha);

		// Holds don't get a glow pass because rendering them is already
		// painfully slow. -Kyz

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		queue.v[0].p = left_vert;  queue.v[0].c = color; queue.v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		queue.v[1].p = center_vert; queue.v[1].c = color; queue.v[1].t = RageVector2(fTexCoordCenter, fTexCoordTop);
		queue.v[2].p = right_vert;  queue.v[2].c = color; queue.v[2].t = RageVector2(fTexCoordRight, fTexCoordTop);
		queue.v+=3;

		if( queue.Free() < 3 || bLast )
		{
			/* The queue is full.  Render it, clear the buffer, and move back a step to
			 * start off the strip again. */
			if( !bAllAreTransparent )
			{
				FOREACH( Sprite*, vpSpr, spr )
				{
					RageTexture* pTexture = (*spr)->GetTexture();
					DISPLAY->SetTexture( TextureUnit_1, pTexture->GetTexHandle() );
					DISPLAY->SetBlendMode( spr == vpSpr.begin() ? BLEND_NORMAL : BLEND_ADD );
					DISPLAY->SetCullMode( CULL_NONE );
					DISPLAY->SetTextureWrapping( TextureUnit_1, bWrapping );
					queue.Draw();
				}
			}
			queue.Init();
			bAllAreTransparent = true;
			fY -= fYStep;
		}
	}
}

void NoteDisplay::DrawHoldBody(const TapNote& tn,
	const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args, float fBeat,
	bool bIsBeingHeld, float fYHead, float fYTail, bool bIsAddition,
	float fPercentFadeToFail, float fColorScale, bool bGlow,
	float top_beat, float bottom_beat)
{
	vector<Sprite*> vpSprTop;
	Sprite *pSpriteTop = GetHoldSprite( m_HoldTopCap, NotePart_HoldTopCap, fBeat, tn.subType == TapNoteSubType_Roll, bIsBeingHeld && !cache->m_bHoldActiveIsAddLayer );
	vpSprTop.push_back( pSpriteTop );

	vector<Sprite*> vpSprBody;
	Sprite *pSpriteBody = GetHoldSprite( m_HoldBody, NotePart_HoldBody, fBeat, tn.subType == TapNoteSubType_Roll, bIsBeingHeld && !cache->m_bHoldActiveIsAddLayer );
	vpSprBody.push_back( pSpriteBody );

	vector<Sprite*> vpSprBottom;
	Sprite *pSpriteBottom = GetHoldSprite( m_HoldBottomCap, NotePart_HoldBottomCap, fBeat, tn.subType == TapNoteSubType_Roll, bIsBeingHeld && !cache->m_bHoldActiveIsAddLayer );
	vpSprBottom.push_back( pSpriteBottom );

	if( bIsBeingHeld && cache->m_bHoldActiveIsAddLayer )
	{
		Sprite *pSprTop = GetHoldSprite( m_HoldTopCap, NotePart_HoldTopCap, fBeat, tn.subType == TapNoteSubType_Roll, true );
		vpSprTop.push_back( pSprTop );
		Sprite *pSprBody = GetHoldSprite( m_HoldBody, NotePart_HoldBody, fBeat, tn.subType == TapNoteSubType_Roll, true );
		vpSprBody.push_back( pSprBody );
		Sprite *pSprBottom = GetHoldSprite( m_HoldBottomCap, NotePart_HoldBottomCap, fBeat, tn.subType == TapNoteSubType_Roll, true );
		vpSprBottom.push_back( pSprBottom );
	}

	const bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(column_args.column) > 0.5f;
	bool bFlipHoldBody = bReverse && cache->m_bFlipHoldBodyWhenReverse;
	if( bFlipHoldBody )
	{
		swap( vpSprTop, vpSprBottom );
		swap( pSpriteTop, pSpriteBottom );
	}

	if( bGlow )
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Glow );
	else
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Modulate );

	const bool bWavyPartsNeedZBuffer = ArrowEffects::NeedZBuffer( m_pPlayerState );
	DISPLAY->SetZTestMode( bWavyPartsNeedZBuffer?ZTEST_WRITE_ON_PASS:ZTEST_OFF );
	DISPLAY->SetZWrite( bWavyPartsNeedZBuffer );

	// Hack: Z effects need a finer grain step.
	const int fYStep = bWavyPartsNeedZBuffer? 4: 16; // use small steps only if wavy

	if( bFlipHoldBody )
	{
		fYHead -= cache->m_iStopDrawingHoldBodyOffsetFromTail;
		fYTail -= cache->m_iStartDrawingHoldBodyOffsetFromHead;
	}
	else
	{
		fYHead += cache->m_iStartDrawingHoldBodyOffsetFromHead;
		fYTail += cache->m_iStopDrawingHoldBodyOffsetFromTail;
	}

	const float fFrameHeightTop	= pSpriteTop->GetUnzoomedHeight();
	const float fFrameHeightBottom	= pSpriteBottom->GetUnzoomedHeight();

	float fYStartPos = ArrowEffects::GetYPos( m_pPlayerState, column_args.column,
		field_args.draw_pixels_after_targets, m_fYReverseOffsetPixels );
	float fYEndPos = ArrowEffects::GetYPos( m_pPlayerState, column_args.column,
		field_args.draw_pixels_before_targets, m_fYReverseOffsetPixels );
	if( bReverse )
		swap( fYStartPos, fYEndPos );

	bool bTopAnchor = bReverse && cache->m_bTopHoldAnchorWhenReverse;

	// Draw the top cap
	DrawHoldPart(vpSprTop, field_args, column_args, fYStep, fPercentFadeToFail,
		fColorScale, bGlow,
		tn.HoldResult.fOverlappedTime,
		fYHead-fFrameHeightTop, fYHead,
		fYStartPos, fYEndPos,
		false, bTopAnchor, bFlipHoldBody, top_beat, top_beat);

	// Draw the body
	DrawHoldPart(vpSprBody, field_args, column_args, fYStep, fPercentFadeToFail,
		fColorScale, bGlow,
		tn.HoldResult.fOverlappedTime,
		fYHead, fYTail,
		fYStartPos, fYEndPos,
		true, bTopAnchor, bFlipHoldBody, top_beat, bottom_beat);

	// Draw the bottom cap
	DrawHoldPart(vpSprBottom, field_args, column_args, fYStep, fPercentFadeToFail,
		fColorScale, bGlow,
		tn.HoldResult.fOverlappedTime,
		fYTail, fYTail+fFrameHeightBottom,
		max(fYStartPos, fYHead), fYEndPos,
		false, bTopAnchor, bFlipHoldBody, bottom_beat, bottom_beat);
}

void NoteDisplay::DrawHold(const TapNote& tn,
	const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args, int iRow, bool bIsBeingHeld,
	const HoldNoteResult &Result, bool bIsAddition, float fPercentFadeToFail)
{
	int iEndRow = iRow + tn.iDuration;
	float top_beat= NoteRowToVisibleBeat(m_pPlayerState, iRow);
	float bottom_beat= NoteRowToVisibleBeat(m_pPlayerState, iEndRow);
	if(bIsBeingHeld)
	{
		top_beat= column_args.song_beat;
	}

	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(column_args.column) > 0.5f;
	float fStartBeat = NoteRowToBeat( max(tn.HoldResult.iLastHeldRow, iRow) );
	float fThrowAway = 0;

	// HACK: If life > 0, don't set YOffset to 0 so that it doesn't jiggle around the receptor.
	bool bStartIsPastPeak = true;
	float fStartYOffset	= 0;
	if( tn.HoldResult.bActive  &&  tn.HoldResult.fLife > 0 )
		;	// use the default values filled in above
	else
		fStartYOffset = ArrowEffects::GetYOffset( m_pPlayerState, column_args.column, fStartBeat, fThrowAway, bStartIsPastPeak );
	
	float fEndPeakYOffset	= 0;
	bool bEndIsPastPeak = false;
	float fEndYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, column_args.column, NoteRowToBeat(iEndRow), fEndPeakYOffset, bEndIsPastPeak );

	// In boomerang, the arrows reverse direction at Y offset value fPeakAtYOffset.  
	// If fPeakAtYOffset lies inside of the hold we're drawing, then the we 
	// want to draw the tail at that max Y offset, or else the hold will appear 
	// to magically grow as the tail approaches the max Y offset.
	if( bStartIsPastPeak && !bEndIsPastPeak )
		fEndYOffset	= fEndPeakYOffset;	// use the calculated PeakYOffset so that long holds don't appear to grow
	
	// Swap in reverse, so fStartYOffset is always the offset higher on the screen.
	if( bReverse )
		swap( fStartYOffset, fEndYOffset );

	const float fYHead		= ArrowEffects::GetYPos( m_pPlayerState, column_args.column, fStartYOffset, m_fYReverseOffsetPixels );
	const float fYTail		= ArrowEffects::GetYPos( m_pPlayerState, column_args.column, fEndYOffset, m_fYReverseOffsetPixels );

	const float fColorScale		= SCALE( tn.HoldResult.fLife, 0.0f, 1.0f, cache->m_fHoldLetGoGrayPercent, 1.0f );

	bool bFlipHeadAndTail = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	/* The body and caps should have no overlap, so their order doesn't matter.
	 * Draw the head last, so it appears on top. */
	float fBeat = NoteRowToBeat(iRow);
	// Side note:  I don't know why these two checks were commented out and I
	// didn't bother to update them when rewriting the arguments that are
	// passed to the note drawing functions. -Kyz
	/*
	if( !cache->m_bHoldHeadIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, NoteRowToBeat(iRow), tn.subType == TapNoteSubType_Roll, bIsBeingHeld );
		DrawActor( tn, pActor, NotePart_HoldHead, iCol, bFlipHeadAndTail ? fEndYOffset : fStartYOffset, fBeat, bIsAddition, fPercentFadeToFail, fReverseOffsetPixels, fColorScale, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	}
	if( !cache->m_bHoldTailIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldTail, NotePart_HoldTail, NoteRowToBeat(iRow), tn.subType == TapNoteSubType_Roll, bIsBeingHeld );
		DrawActor( tn, pActor, NotePart_HoldTail, iCol, bFlipHeadAndTail ? fStartYOffset : fEndYOffset, fBeat, bIsAddition, fPercentFadeToFail, fReverseOffsetPixels, fColorScale, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	}
	*/

	DrawHoldBody(tn, field_args, column_args, fBeat, bIsBeingHeld, fYHead, fYTail, bIsAddition, fPercentFadeToFail, fColorScale, false, top_beat, bottom_beat);
	DrawHoldBody(tn, field_args, column_args, fBeat, bIsBeingHeld, fYHead, fYTail, bIsAddition, fPercentFadeToFail, fColorScale, true, top_beat, bottom_beat);

	/* These set the texture mode themselves. */
	// this part was modified in pumpmania, where it flips the draw order
	// of the head and tail. Perhaps make this a theme/noteskin metric? -aj
	if( cache->m_bHoldTailIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldTail, NotePart_HoldTail, NoteRowToBeat(iRow), tn.subType == TapNoteSubType_Roll, bIsBeingHeld );
		DrawActor(tn, pActor, NotePart_HoldTail, field_args, column_args, bFlipHeadAndTail ? fStartYOffset : fEndYOffset, fBeat, bIsAddition, fPercentFadeToFail, fColorScale, false);
	}
	if( cache->m_bHoldHeadIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, NoteRowToBeat(iRow), tn.subType == TapNoteSubType_Roll, bIsBeingHeld );
		DrawActor(tn, pActor, NotePart_HoldHead, field_args, column_args, bFlipHeadAndTail ? fEndYOffset : fStartYOffset, fBeat, bIsAddition, fPercentFadeToFail, fColorScale, bIsBeingHeld);
	}
}

void NoteDisplay::DrawActor(const TapNote& tn, Actor* pActor, NotePart part,
	const NoteFieldRenderArgs& field_args, const NoteColumnRenderArgs& column_args, float fYOffset, float fBeat,
	bool bIsAddition, float fPercentFadeToFail, float fColorScale,
	bool is_being_held)
{
	if (tn.type == TapNoteType_AutoKeysound && !GAMESTATE->m_bInStepEditor) return;
	if(fYOffset < field_args.draw_pixels_after_targets ||
		fYOffset > field_args.draw_pixels_before_targets)
		return;
	float spline_beat= fBeat;
	if(is_being_held) { spline_beat= column_args.song_beat; }

	const float fAlpha	= ArrowEffects::GetAlpha(	m_pPlayerState, column_args.column, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, field_args.draw_pixels_before_targets, field_args.fade_before_targets );
	const float fGlow	= ArrowEffects::GetGlow(	m_pPlayerState, column_args.column, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, field_args.draw_pixels_before_targets, field_args.fade_before_targets );
	const RageColor diffuse	= RageColor(
		column_args.diffuse.r * fColorScale,
		column_args.diffuse.g * fColorScale,
		column_args.diffuse.b * fColorScale,
		column_args.diffuse.a * fAlpha);
	const RageColor glow	= RageColor(
		column_args.glow.r * fColorScale,
		column_args.glow.g * fColorScale,
		column_args.glow.b * fColorScale,
		column_args.glow.a * fGlow);

	bool bIsHoldHead = tn.type == TapNoteType_HoldHead;
	bool bIsHoldCap = bIsHoldHead || tn.type == TapNoteType_HoldTail;
	
	if( tn.type != TapNoteType_HoldHead )
		fColorScale		*= ArrowEffects::GetBrightness(	m_pPlayerState, fBeat );

	// same logical structure as in UpdateReceptorGhostStuff, I just haven't
	// figured out a good way to combine them. -Kyz
	vector<float> sp_pos;
	vector<float> sp_rot;
	vector<float> sp_zoom;
	vector<float> ae_pos(3, 0.0f);
	vector<float> ae_rot(3, 0.0f);
	vector<float> ae_zoom(3, 0.0f);
	column_args.spae_pos_for_beat(m_pPlayerState, spline_beat,
		fYOffset, m_fYReverseOffsetPixels, sp_pos, ae_pos);

	switch(column_args.rot_handler->m_spline_mode)
	{
		case NCSM_Disabled:
			if(!bIsHoldCap)
			{
				ae_rot[0]= ArrowEffects::GetRotationX(m_pPlayerState, fYOffset);
			}
			ae_rot[1]= ArrowEffects::GetRotationY(m_pPlayerState, fYOffset);
			ae_rot[2]= ArrowEffects::GetRotationZ(m_pPlayerState, fBeat, bIsHoldHead);
			sp_rot.resize(3);
			sp_rot[0]= sp_rot[1]= sp_rot[2]= 0.0f;
			break;
		case NCSM_Offset:
			if(!bIsHoldCap)
			{
				ae_rot[0]= ArrowEffects::GetRotationX(m_pPlayerState, fYOffset);
			}
			ae_rot[1]= ArrowEffects::GetRotationY(m_pPlayerState, fYOffset);
			ae_rot[2]= ArrowEffects::GetRotationZ(m_pPlayerState, fBeat, bIsHoldHead);
			column_args.rot_handler->EvalForBeat(column_args.song_beat, spline_beat, sp_rot);
			break;
		case NCSM_Position:
			column_args.rot_handler->EvalForBeat(column_args.song_beat, spline_beat, sp_rot);
			break;
	}
	column_args.spae_zoom_for_beat(m_pPlayerState, spline_beat, sp_zoom, ae_zoom);
	column_args.SetPRZForActor(pActor, sp_pos, ae_pos, sp_rot, ae_rot, sp_zoom, ae_zoom);
	// [AJ] this two lines (and how they're handled) piss off many people:
	pActor->SetDiffuse( diffuse );
	pActor->SetGlow( glow );

	bool bNeedsTranslate = (bIsAddition && !IsVectorZero(cache->m_fAdditionTextureCoordOffset[part])) || !IsVectorZero(cache->m_fNoteColorTextureCoordSpacing[part]);
	if( bNeedsTranslate )
	{
		DISPLAY->TexturePushMatrix();
		float color = 0.0f;
		switch( cache->m_NoteColorType[part] )
		{
		case NoteColorType_Denominator:
			color = float( BeatToNoteType( fBeat ) );
			color = clamp( color, 0, (cache->m_iNoteColorCount[part]-1) );
			break;
		case NoteColorType_Progress:
			color = fmodf( ceilf( fBeat * cache->m_iNoteColorCount[part] ), (float)cache->m_iNoteColorCount[part] );
			break;
		default:
			FAIL_M(ssprintf("Invalid NoteColorType: %i", cache->m_NoteColorType[part]));
		}
		DISPLAY->TextureTranslate( (bIsAddition ? cache->m_fAdditionTextureCoordOffset[part] : RageVector2(0,0)) + cache->m_fNoteColorTextureCoordSpacing[part]*color );
	}

	pActor->Draw();

	if( bNeedsTranslate )
	{
		DISPLAY->TexturePopMatrix();
	}
}

void NoteDisplay::DrawTap(const TapNote& tn,
	const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args, float fBeat,
	bool bOnSameRowAsHoldStart, bool bOnSameRowAsRollStart,
	bool bIsAddition, float fPercentFadeToFail)
{
	Actor* pActor = NULL;
	NotePart part = NotePart_Tap;
	/*
	if( tn.source == TapNoteSource_Addition )
	{
		pActor = GetTapActor( m_TapAddition, NotePart_Addition, fBeat );
		part = NotePart_Addition;
	}
	*/
	if( tn.type == TapNoteType_Lift )
	{
		pActor = GetTapActor( m_TapLift, NotePart_Lift, fBeat );
		part = NotePart_Lift;
	}
	else if( tn.type == TapNoteType_Mine )
	{
		pActor = GetTapActor( m_TapMine, NotePart_Mine, fBeat );
		part = NotePart_Mine;
	}
	// TODO: Simplify all of the below.
	else if (bOnSameRowAsHoldStart && bOnSameRowAsRollStart)
	{
		if (cache->m_bDrawHoldHeadForTapsOnSameRow && cache->m_bDrawRollHeadForTapsOnSameRow)
		{
			if (cache->m_bTapHoldRollOnRowMeansHold) // another new metric?
			{
				pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, fBeat, false, false );
			}
			else
			{
				pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, fBeat, true, false );
			}
		}
		else if (cache->m_bDrawHoldHeadForTapsOnSameRow)
		{
			pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, fBeat, false, false );
		}
		else if (cache->m_bDrawRollHeadForTapsOnSameRow)
		{
			pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, fBeat, true, false );
		}
	}
	
	else if( bOnSameRowAsHoldStart  &&  cache->m_bDrawHoldHeadForTapsOnSameRow )
	{
		pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, fBeat, false, false );
	}
	
	else if( bOnSameRowAsRollStart  &&  cache->m_bDrawRollHeadForTapsOnSameRow )
	{
		pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, fBeat, true, false );
	}
	
	else
	{
		pActor = GetTapActor( m_TapNote, NotePart_Tap, fBeat );
	}

	if( tn.type == TapNoteType_Attack )
	{
		Message msg( "SetAttack" );
		msg.SetParam( "Modifiers", tn.sAttackModifiers );
		pActor->HandleMessage( msg );
	}

	const float fYOffset = ArrowEffects::GetYOffset( m_pPlayerState, column_args.column, fBeat );
	// this is the line that forces the (1,1,1,x) part of the noteskin diffuse -aj
	DrawActor(tn, pActor, part, field_args, column_args, fYOffset, fBeat, bIsAddition, fPercentFadeToFail, 1.0f, false);

	if( tn.type == TapNoteType_Attack )
		pActor->PlayCommand( "UnsetAttack" );
}

void NoteColumnRenderer::UpdateReceptorGhostStuff(Actor* receptor) const
{
	const PlayerState* player_state= m_field_render_args->player_state;
	float song_beat= player_state->GetDisplayedPosition().m_fSongBeatVisible;
	// sp_* will be filled with the settings from the splines.
	// ae_* will be filled with the settings from ArrowEffects.
	// The two together will be applied to the actor.
	// sp_* will be zeroes in NCSM_Disabled, and ae_* will be zeroes in
	// NCSM_Position, so the setting step won't have to check the mode. -Kyz
	// sp_* are sized by the spline evaluate function.
	vector<float> sp_pos;
	vector<float> sp_rot;
	vector<float> sp_zoom;
	vector<float> ae_pos(3, 0.0f);
	vector<float> ae_rot(3, 0.0f);
	vector<float> ae_zoom(3, 0.0f);
	switch(NCR_current.m_pos_handler.m_spline_mode)
	{
		case NCSM_Disabled:
			ArrowEffects::GetXYZPos(player_state, m_column, 0, m_field_render_args->reverse_offset_pixels, ae_pos);
			sp_pos.resize(3);
			// Sure, resize is supposed to call the default constructor, and for
			// numbers the default constructor is supposed to set it to zero, but
			// I got bit for relying on that once. -Kyz
			sp_pos[0]= sp_pos[1]= sp_pos[2]= 0.0f;
			break;
		case NCSM_Offset:
			ArrowEffects::GetXYZPos(player_state, m_column, 0, m_field_render_args->reverse_offset_pixels, ae_pos);
			NCR_current.m_pos_handler.EvalForReceptor(song_beat, sp_pos);
			break;
		case NCSM_Position:
			NCR_current.m_pos_handler.EvalForReceptor(song_beat, sp_pos);
			break;
	}
	switch(NCR_current.m_rot_handler.m_spline_mode)
	{
		case NCSM_Disabled:
			ae_rot[2]= ArrowEffects::ReceptorGetRotationZ(player_state);
			sp_rot.resize(3);
			sp_rot[0]= sp_rot[1]= sp_rot[2]= 0.0f;
			break;
		case NCSM_Offset:
			ae_rot[2]= ArrowEffects::ReceptorGetRotationZ(player_state);
			NCR_current.m_rot_handler.EvalForReceptor(song_beat, sp_rot);
			break;
		case NCSM_Position:
			NCR_current.m_rot_handler.EvalForReceptor(song_beat, sp_rot);
			break;
	}
	switch(NCR_current.m_zoom_handler.m_spline_mode)
	{
		case NCSM_Disabled:
			ae_zoom[0]= ae_zoom[1]= ae_zoom[2]= ArrowEffects::GetZoom(player_state);
			sp_zoom.resize(3);
			sp_zoom[0]= sp_zoom[1]= sp_zoom[2]= 0.0f;
			break;
		case NCSM_Offset:
			ae_zoom[0]= ae_zoom[1]= ae_zoom[2]= ArrowEffects::GetZoom(player_state);
			NCR_current.m_zoom_handler.EvalForReceptor(song_beat, sp_zoom);
			break;
		case NCSM_Position:
			NCR_current.m_zoom_handler.EvalForReceptor(song_beat, sp_zoom);
			break;
	}
	m_column_render_args.SetPRZForActor(receptor, sp_pos, ae_pos, sp_rot, ae_rot, sp_zoom, ae_zoom);
	receptor->SetInternalDiffuse(m_column_render_args.diffuse);
	receptor->SetInternalGlow(m_column_render_args.glow);
}

void NoteColumnRenderer::DrawPrimitives()
{
	m_column_render_args.song_beat= m_field_render_args->player_state->GetDisplayedPosition().m_fSongBeatVisible;
	m_column_render_args.pos_handler= &NCR_current.m_pos_handler;
	m_column_render_args.rot_handler= &NCR_current.m_rot_handler;
	m_column_render_args.zoom_handler= &NCR_current.m_zoom_handler;
	m_column_render_args.diffuse= m_pTempState->diffuse[0];
	m_column_render_args.glow= m_pTempState->glow;
	m_field_render_args->first_beat= NoteRowToBeat(m_field_render_args->first_row);
	m_field_render_args->last_beat= NoteRowToBeat(m_field_render_args->last_row);
	bool any_upcoming= false;
	// Build lists of holds and taps for each player number, then pass those
	// lists to the displays to draw.
	// The vector in the NUM_PlayerNumber slot should stay empty, not worth
	// optimizing it out. -Kyz
	vector<vector<NoteData::TrackMap::const_iterator> > holds(PLAYER_INVALID+1);
	vector<vector<NoteData::TrackMap::const_iterator> > taps(PLAYER_INVALID+1);
	NoteData::TrackMap::const_iterator begin, end;
	m_field_render_args->note_data->GetTapNoteRangeInclusive(m_column,
		m_field_render_args->first_row, m_field_render_args->last_row+1, begin, end);
	for(; begin != end; ++begin)
	{
		const TapNote& tn= begin->second;
		switch(tn.type)
		{
			case TapNoteType_Empty:
				continue;
			case TapNoteType_Tap:
			case TapNoteType_HoldTail:
			case TapNoteType_Mine:
			case TapNoteType_Lift:
			case TapNoteType_Attack:
			case TapNoteType_AutoKeysound:
				if(!tn.result.bHidden)
				{
					taps[tn.pn].push_back(begin);
				}
				break;
			case TapNoteType_HoldHead:
				if(tn.HoldResult.hns != HNS_Held)
				{
					holds[tn.pn].push_back(begin);
				}
				break;
		}
	}
#define DTS_INNER(pn, tap_set, draw_func, disp) \
	if(!tap_set[pn].empty()) \
	{ \
		any_upcoming|= disp->draw_func(*m_field_render_args, m_column_render_args, tap_set[pn]); \
	}
#define DRAW_TAP_SET(tap_set, draw_func) \
	FOREACH_PlayerNumber(pn) \
	{ \
		DTS_INNER(pn, tap_set, draw_func, m_displays[pn]); \
	}
	DRAW_TAP_SET(holds, DrawHoldsInRange);
	DTS_INNER(PLAYER_INVALID, holds, DrawHoldsInRange, m_displays[PLAYER_INVALID]);
	DRAW_TAP_SET(taps, DrawTapsInRange);
	DTS_INNER(PLAYER_INVALID, taps, DrawTapsInRange, m_displays[PLAYER_INVALID]);
#undef DTS_INNER
#undef DRAW_TAP_SET
	m_field_render_args->receptor_row->SetNoteUpcoming(m_column, any_upcoming);
}

void NoteColumnRenderer::SetCurrentTweenStart()
{
	NCR_start= NCR_current;
}

void NoteColumnRenderer::EraseHeadTween()
{
	NCR_current= NCR_Tweens[0];
	NCR_Tweens.erase(NCR_Tweens.begin());
}

void NoteColumnRenderer::UpdatePercentThroughTween(float between)
{
	NCR_TweenState::MakeWeightedAverage(NCR_current, NCR_start, NCR_Tweens[0],
		between);
}

void NoteColumnRenderer::BeginTweening(float time, ITween* interp)
{
	Actor::BeginTweening(time, interp);
	if(!NCR_Tweens.empty())
	{
		NCR_Tweens.push_back(NCR_Tweens.back());
	}
	else
	{
		NCR_Tweens.push_back(NCR_current);
	}
}

void NoteColumnRenderer::StopTweening()
{
	NCR_Tweens.clear();
	Actor::StopTweening();
}

void NoteColumnRenderer::FinishTweening()
{
	if(!NCR_Tweens.empty())
	{
		NCR_current= NCR_DestTweenState();
	}
	Actor::FinishTweening();
}

NoteColumnRenderer::NCR_TweenState::NCR_TweenState()
{
	m_rot_handler.m_spline.set_spatial_extent(0, PI*2.0f);
	m_rot_handler.m_spline.set_spatial_extent(1, PI*2.0f);
	m_rot_handler.m_spline.set_spatial_extent(2, PI*2.0f);
}

void NoteColumnRenderer::NCR_TweenState::MakeWeightedAverage(
	NCR_TweenState& out, const NCR_TweenState& from, const NCR_TweenState& to,
	float between)
{
#define WEIGHT_FOR_ME(me) \
	NCSplineHandler::MakeWeightedAverage(out.me, from.me, to.me, between);
	WEIGHT_FOR_ME(m_pos_handler);
	WEIGHT_FOR_ME(m_rot_handler);
	WEIGHT_FOR_ME(m_zoom_handler);
#undef WEIGHT_FOR_ME
}


#include "LuaBinding.h"

struct LunaNCSplineHandler : Luna<NCSplineHandler>
{
	static int get_spline(T* p, lua_State* L)
	{
		p->m_spline.PushSelf(L);
		return 1;
	}
	DEFINE_METHOD(get_receptor_t, m_receptor_t);
	DEFINE_METHOD(get_beats_per_t, m_beats_per_t);
#define SET_T(member, name) \
	static int name(T* p, lua_State* L) \
	{ \
		p->member= FArg(1); \
		COMMON_RETURN_SELF; \
	}
#define SET_B(member, name) \
	static int name(T* p, lua_State* L) \
	{ \
		p->member= BArg(1); \
		COMMON_RETURN_SELF; \
	}
	SET_T(m_receptor_t, set_receptor_t);
	SET_T(m_beats_per_t, set_beats_per_t);
	SET_B(m_subtract_song_beat_from_curr, set_subtract_song_beat);
#undef SET_T
#undef SET_B
	static int set_spline_mode(T* p, lua_State* L)
	{
		p->m_spline_mode= Enum::Check<NoteColumnSplineMode>(L, 1);
		COMMON_RETURN_SELF;
	}
	DEFINE_METHOD(get_spline_mode, m_spline_mode);
	DEFINE_METHOD(get_subtract_song_beat, m_subtract_song_beat_from_curr);

	LunaNCSplineHandler()
	{
		ADD_METHOD(get_spline);
		ADD_METHOD(get_beats_per_t);
		ADD_METHOD(set_beats_per_t);
		ADD_METHOD(get_receptor_t);
		ADD_METHOD(set_receptor_t);
		ADD_METHOD(get_spline_mode);
		ADD_METHOD(set_spline_mode);
		ADD_METHOD(get_subtract_song_beat);
		ADD_METHOD(set_subtract_song_beat);
	}
};

LUA_REGISTER_CLASS(NCSplineHandler);

struct LunaNoteColumnRenderer : Luna<NoteColumnRenderer>
{
#define GET_HANDLER(member, name) \
	static int name(T* p, lua_State* L) \
	{ \
		p->member->PushSelf(L); \
		return 1; \
	}
	GET_HANDLER(GetPosHandler(), get_pos_handler);
	GET_HANDLER(GetRotHandler(), get_rot_handler);
	GET_HANDLER(GetZoomHandler(), get_zoom_handler);
#undef GET_HANDLER

	LunaNoteColumnRenderer()
	{
		ADD_METHOD(get_pos_handler);
		ADD_METHOD(get_rot_handler);
		ADD_METHOD(get_zoom_handler);
	}
};

LUA_REGISTER_DERIVED_CLASS(NoteColumnRenderer, Actor)

/*
 * NoteColumnRenderer and associated spline stuff (c) Eric Reese 2014-2015
 * (c) 2001-2006 Brian Bugh, Ben Nordstrom, Chris Danford, Steve Checkoway
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
