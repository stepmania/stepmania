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
	"TapFake",
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

void NCSplineHandler::EvalForBeat(float song_beat, float note_beat, RageVector3& ret) const
{
	float t_value= BeatToTValue(song_beat, note_beat);
	m_spline.evaluate(t_value, ret);
}

void NCSplineHandler::EvalDerivForBeat(float song_beat, float note_beat, RageVector3& ret) const
{
	float t_value= BeatToTValue(song_beat, note_beat);
	m_spline.evaluate_derivative(t_value, ret);
}

void NCSplineHandler::EvalForReceptor(float song_beat, RageVector3& ret) const
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

void NoteColumnRenderArgs::spae_pos_for_beat(const PlayerState* player_state,
	float beat, float y_offset, float y_reverse_offset,
	RageVector3& sp_pos, RageVector3& ae_pos) const
{
	switch(pos_handler->m_spline_mode)
	{
		case NCSM_Disabled:
			ArrowEffects::GetXYZPos(player_state, column, y_offset, y_reverse_offset, ae_pos);
			break;
		case NCSM_Offset:
			ArrowEffects::GetXYZPos(player_state, column, y_offset, y_reverse_offset, ae_pos);
			pos_handler->EvalForBeat(song_beat, beat, sp_pos);
			break;
		case NCSM_Position:
			pos_handler->EvalForBeat(song_beat, beat, sp_pos);
			break;
	}
}
void NoteColumnRenderArgs::spae_zoom_for_beat(const PlayerState* state, float beat,
	RageVector3& sp_zoom, RageVector3& ae_zoom) const
{
	switch(zoom_handler->m_spline_mode)
	{
		case NCSM_Disabled:
			ae_zoom.x= ae_zoom.y= ae_zoom.z= ArrowEffects::GetZoom(state);
			break;
		case NCSM_Offset:
			ae_zoom.x= ae_zoom.y= ae_zoom.z= ArrowEffects::GetZoom(state);
			zoom_handler->EvalForBeat(song_beat, beat, sp_zoom);
			break;
		case NCSM_Position:
			zoom_handler->EvalForBeat(song_beat, beat, sp_zoom);
			break;
	}
}
void NoteColumnRenderArgs::SetPRZForActor(Actor* actor,
	const RageVector3& sp_pos, const RageVector3& ae_pos,
	const RageVector3& sp_rot, const RageVector3& ae_rot,
	const RageVector3& sp_zoom, const RageVector3& ae_zoom) const
{
	actor->SetX(sp_pos.x + ae_pos.x);
	actor->SetY(sp_pos.y + ae_pos.y);
	actor->SetZ(sp_pos.z + ae_pos.z);
	actor->SetRotationX(sp_rot.x * PI_180R + ae_rot.x);
	actor->SetRotationY(sp_rot.y * PI_180R + ae_rot.y);
	actor->SetRotationZ(sp_rot.z * PI_180R + ae_rot.z);
	actor->SetZoomX(sp_zoom.x + ae_zoom.x);
	actor->SetZoomY(sp_zoom.y + ae_zoom.y);
	actor->SetZoomZ(sp_zoom.z + ae_zoom.z);
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
			is_addition,
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
		DrawTap(tn, field_args, column_args,
			NoteRowToVisibleBeat(m_pPlayerState, tap_row),
			hold_begins_on_this_beat, roll_begins_on_this_beat,
			is_addition,
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
		return ArrowEffects::GetGlow(iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar);
	else
		return ArrowEffects::GetAlpha(iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar);
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

enum hold_part_type
{
	hpt_top,
	hpt_body,
	hpt_bottom,
};

void NoteDisplay::DrawHoldPart(vector<Sprite*> &vpSpr,
	const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args,
	const draw_hold_part_args& part_args, bool glow, int part_type)
{
	ASSERT(!vpSpr.empty());

	float ae_zoom= ArrowEffects::GetZoom(m_pPlayerState);
	Sprite *pSprite = vpSpr.front();

	// draw manually in small segments
	RectF rect = *pSprite->GetCurrentTextureCoordRect();
	if(part_args.flip_texture_vertically)
		swap(rect.top, rect.bottom);
	const float fFrameWidth		= pSprite->GetUnzoomedWidth();
	const float unzoomed_frame_height= pSprite->GetUnzoomedHeight();

	/* Only draw the section that's within the range specified.  If a hold note is
	 * very long, don't process or draw the part outside of the range.  Don't change
	 * part_args.y_top or part_args.y_bottom; they need to be left alone to calculate texture coordinates. */
	// If hold body, draw texture to the outside screen.(fix by A.C)
	float y_start_pos = (part_type == hpt_body) ? part_args.y_top : max(part_args.y_top, part_args.y_start_pos);
	if (part_args.y_top < part_args.y_start_pos - unzoomed_frame_height)
	{
		y_start_pos = fmod((y_start_pos - part_args.y_start_pos), unzoomed_frame_height) + part_args.y_start_pos;
	}
	float y_end_pos = min(part_args.y_bottom, part_args.y_end_pos);
	const float color_scale= glow ? 1 : part_args.color_scale;
	if(part_type == hpt_body)
	{
		// Overshoot to cover up the seam between body and bottom.
		// It's not fully gone, but the bg doesn't peak through.
		y_end_pos+= 1;
	}

	// top to bottom
	bool bAllAreTransparent = true;
	bool last_vert_set = false;
	bool first_vert_set= true;
	float add_to_tex_coord = 0;

	// The caps should always use the full texture.
	if(part_type == hpt_body)
	{
		if (!part_args.anchor_to_top)
		{
			float tex_coord_bottom= SCALE(part_args.y_bottom - part_args.y_top,
				0, unzoomed_frame_height, rect.top, rect.bottom);
			float want_tex_coord_bottom	= ceilf(tex_coord_bottom - 0.0001f);
			add_to_tex_coord = want_tex_coord_bottom - tex_coord_bottom;
		}

		if(part_args.wrapping)
		{
			/* For very large hold notes, shift the texture coordinates to be near 0, so we
			 * don't send very large values to the renderer. */
			const float fDistFromTop = y_start_pos - part_args.y_top;
			float fTexCoordTop = SCALE(fDistFromTop, 0, unzoomed_frame_height, rect.top, rect.bottom);
			fTexCoordTop += add_to_tex_coord;
			add_to_tex_coord -= floorf(fTexCoordTop);
		}
	}
	// The bottom caps mysteriously hate me and their texture coords need to be
	// shifted by one pixel or there is a seam. -Kyz
	if(part_type == hpt_bottom)
	{
		if (!part_args.anchor_to_top)
		{
			float offset = unzoomed_frame_height - (y_end_pos - y_start_pos) / ae_zoom;
			// ロングノート本体の長さがunzoomed_frame_height→0のときに、add_to_tex_coordを0→1にすればOK
			// つまり、offsetを0→unzoomed_frame_heightにすると理想通りの表示になる -A.C
			// Shift texture coord to fit hold length If hold length is less than
			// bottomcap frame height. (translated by hanubeki)
			if (offset>0){
				add_to_tex_coord = SCALE(offset, 0.0f, unzoomed_frame_height, 0.0f, 1.0f);
			}
			else{
				add_to_tex_coord = 0.0f;
			}
		}
	}

	DISPLAY->ClearAllTextures();

	const float fTexCoordLeft	= rect.left;
	const float fTexCoordRight	= rect.right;
	const float fTexCoordCenter	= (fTexCoordLeft+fTexCoordRight)/2;

	// pos_z_vec will be used later to orient the hold.  Read below. -Kyz
	static const RageVector3 pos_z_vec(0.0f, 0.0f, 1.0f);
	static const RageVector3 pos_y_vec(0.0f, 1.0f, 0.0f);
	StripBuffer queue;
	for(float fY = y_start_pos; !last_vert_set; fY += part_args.y_step)
	{
		if(fY >= y_end_pos)
		{
			fY = y_end_pos;
			last_vert_set = true;
		}

		const float fYOffset= ArrowEffects::GetYOffsetFromYPos(column_args.column, fY, m_fYReverseOffsetPixels);

		float cur_beat= part_args.top_beat;
		if(part_args.top_beat != part_args.bottom_beat)
		{
			cur_beat= SCALE(fY, part_args.y_top, part_args.y_bottom, part_args.top_beat, part_args.bottom_beat);
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

		RageVector3 sp_pos;
		RageVector3 sp_pos_forward;
		RageVector3 sp_rot;
		RageVector3 sp_zoom;
		RageVector3 ae_pos;
		RageVector3 ae_rot;

		// (step 1 of vector handling, part 1)
		// ArrowEffects only contributes to the Y component of the vector to
		// maintain the old behavior of how holds are drawn when they wave back
		// and forth. -Kyz
		RageVector3 render_forward(0.0f, 1.0f, 0.0f);
		column_args.spae_pos_for_beat(m_pPlayerState, cur_beat,
			fYOffset, m_fYReverseOffsetPixels, sp_pos, ae_pos);
		// fX and fZ are sp_pos.x + ae_pos.x and sp_pos.z + ae_pos.z. -Kyz
		// fY is the actual y position that should be used, not whatever spae
		// fetched from ArrowEffects. -Kyz
		switch(column_args.pos_handler->m_spline_mode)
		{
			case NCSM_Disabled:
				ae_pos.y= fY;
				break;
			case NCSM_Offset:
				ae_pos.y= fY;
				column_args.pos_handler->EvalDerivForBeat(column_args.song_beat, cur_beat, sp_pos_forward);
				RageVec3Normalize(&sp_pos_forward, &sp_pos_forward);
				break;
			case NCSM_Position:
				ae_pos.y= 0.0f;
				render_forward.y= 0.0f;
				column_args.pos_handler->EvalDerivForBeat(column_args.song_beat, cur_beat, sp_pos_forward);
				RageVec3Normalize(&sp_pos_forward, &sp_pos_forward);
				break;
		}

		render_forward.x+= sp_pos_forward.x;
		render_forward.y+= sp_pos_forward.y;
		render_forward.z+= sp_pos_forward.z;
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
				render_width= fFrameWidth * (ae_zoom + sp_zoom.x);
				break;
			case NCSM_Position:
				column_args.zoom_handler->EvalForBeat(column_args.song_beat, cur_beat, sp_zoom);
				render_width= fFrameWidth * sp_zoom.x;
				break;
		}

		const float fFrameWidthScale	= ArrowEffects::GetFrameWidthScale(m_pPlayerState, fYOffset, part_args.overlapped_time);
		const float fScaledFrameWidth	= render_width * fFrameWidthScale;

		// Can't use the same code as for taps because hold bodies can only rotate
		// around the y axis. -Kyz
		switch(column_args.rot_handler->m_spline_mode)
		{
			case NCSM_Disabled:
				// XXX: Actor rotations use degrees, Math uses radians. Convert here.
				ae_rot.y= ArrowEffects::GetRotationY(fYOffset) * PI_180;
				break;
			case NCSM_Offset:
				ae_rot.y= ArrowEffects::GetRotationY(fYOffset) * PI_180;
				column_args.rot_handler->EvalForBeat(column_args.song_beat, cur_beat, sp_rot);
				break;
			case NCSM_Position:
				column_args.rot_handler->EvalForBeat(column_args.song_beat, cur_beat, sp_rot);
				break;
		}

		RageVector3 center_vert(sp_pos.x + ae_pos.x,
			sp_pos.y + ae_pos.y, sp_pos.z + ae_pos.z);

		// Special case for hold caps, which have the same top and bottom beat.
		if(part_args.top_beat == part_args.bottom_beat && !first_vert_set)
		{
			center_vert.x+= render_forward.x;
			center_vert.y+= render_forward.y;
			center_vert.z+= render_forward.z;
		}

		const float render_roty= (sp_rot.y + ae_rot.y);

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

		const float fDistFromTop	= (fY - y_start_pos) / ae_zoom;
		float fTexCoordTop		= SCALE(fDistFromTop, 0, unzoomed_frame_height, rect.top, rect.bottom);
		fTexCoordTop += add_to_tex_coord;

		const float fAlpha		= ArrowGetAlphaOrGlow(glow, m_pPlayerState, column_args.column, fYOffset, part_args.percent_fade_to_fail, m_fYReverseOffsetPixels, field_args.draw_pixels_before_targets, field_args.fade_before_targets);
		const RageColor color= RageColor(
			column_args.diffuse.r * color_scale,
			column_args.diffuse.g * color_scale,
			column_args.diffuse.b * color_scale,
			column_args.diffuse.a * fAlpha);

		if(fAlpha > 0)
			bAllAreTransparent = false;

		queue.v[0].p = left_vert;  queue.v[0].c = color; queue.v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		queue.v[1].p = center_vert; queue.v[1].c = color; queue.v[1].t = RageVector2(fTexCoordCenter, fTexCoordTop);
		queue.v[2].p = right_vert;  queue.v[2].c = color; queue.v[2].t = RageVector2(fTexCoordRight, fTexCoordTop);
		queue.v+=3;

		if(queue.Free() < 3 || last_vert_set)
		{
			/* The queue is full.  Render it, clear the buffer, and move back a step to
			 * start off the strip again. */
			if(!bAllAreTransparent)
			{
				FOREACH(Sprite*, vpSpr, spr)
				{
					RageTexture* pTexture = (*spr)->GetTexture();
					DISPLAY->SetTexture(TextureUnit_1, pTexture->GetTexHandle());
					DISPLAY->SetBlendMode(spr == vpSpr.begin() ? BLEND_NORMAL : BLEND_ADD);
					DISPLAY->SetCullMode(CULL_NONE);
					DISPLAY->SetTextureWrapping(TextureUnit_1, part_args.wrapping);
					queue.Draw();
				}
			}
			queue.Init();
			bAllAreTransparent = true;
			fY -= part_args.y_step;
		}
		first_vert_set= false;
	}
}

void NoteDisplay::DrawHoldBodyInternal(vector<Sprite*>& sprite_top,
	vector<Sprite*>& sprite_body, vector<Sprite*>& sprite_bottom,
	const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args,
	draw_hold_part_args& part_args,
	const float head_minus_top, const float tail_plus_bottom,
	const float y_head, const float y_tail, const float top_beat,
	const float bottom_beat, bool glow)
{
	// Draw the top cap
	part_args.y_top= head_minus_top;
	part_args.y_bottom= y_head;
	part_args.top_beat= top_beat;
	part_args.bottom_beat= top_beat;
	part_args.wrapping= false;
	DrawHoldPart(sprite_top, field_args, column_args, part_args, glow, hpt_top);
	// Draw the body
	part_args.y_top= y_head;
	part_args.y_bottom= y_tail;
	part_args.bottom_beat= bottom_beat;
	part_args.wrapping= true;
	DrawHoldPart(sprite_body, field_args, column_args, part_args, glow, hpt_body);
	// Draw the bottom cap
	part_args.y_top= y_tail;
	part_args.y_bottom = tail_plus_bottom;
	part_args.top_beat = bottom_beat;
	part_args.y_start_pos = max(part_args.y_start_pos, y_head);
	part_args.wrapping= false;
	DrawHoldPart(sprite_bottom, field_args, column_args, part_args, glow, hpt_bottom);
}

void NoteDisplay::DrawHoldBody(const TapNote& tn,
	const NoteFieldRenderArgs& field_args,
	const NoteColumnRenderArgs& column_args, float beat,
	bool being_held, float y_head, float y_tail, float percent_fade_to_fail,
	float color_scale, float top_beat, float bottom_beat)
{
	draw_hold_part_args part_args;
	part_args.percent_fade_to_fail= percent_fade_to_fail;
	part_args.color_scale= color_scale;
	part_args.overlapped_time= tn.HoldResult.fOverlappedTime;
	vector<Sprite*> vpSprTop;
	Sprite *pSpriteTop = GetHoldSprite( m_HoldTopCap, NotePart_HoldTopCap, beat, tn.subType == TapNoteSubType_Roll, being_held && !cache->m_bHoldActiveIsAddLayer );
	vpSprTop.push_back( pSpriteTop );

	vector<Sprite*> vpSprBody;
	Sprite *pSpriteBody = GetHoldSprite( m_HoldBody, NotePart_HoldBody, beat, tn.subType == TapNoteSubType_Roll, being_held && !cache->m_bHoldActiveIsAddLayer );
	vpSprBody.push_back( pSpriteBody );

	vector<Sprite*> vpSprBottom;
	Sprite *pSpriteBottom = GetHoldSprite( m_HoldBottomCap, NotePart_HoldBottomCap, beat, tn.subType == TapNoteSubType_Roll, being_held && !cache->m_bHoldActiveIsAddLayer );
	vpSprBottom.push_back( pSpriteBottom );

	if(being_held && cache->m_bHoldActiveIsAddLayer)
	{
		Sprite *pSprTop = GetHoldSprite( m_HoldTopCap, NotePart_HoldTopCap, beat, tn.subType == TapNoteSubType_Roll, true );
		vpSprTop.push_back( pSprTop );
		Sprite *pSprBody = GetHoldSprite( m_HoldBody, NotePart_HoldBody, beat, tn.subType == TapNoteSubType_Roll, true );
		vpSprBody.push_back( pSprBody );
		Sprite *pSprBottom = GetHoldSprite( m_HoldBottomCap, NotePart_HoldBottomCap, beat, tn.subType == TapNoteSubType_Roll, true );
		vpSprBottom.push_back( pSprBottom );
	}

	const bool reverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(column_args.column) > 0.5f;
	part_args.flip_texture_vertically = reverse && cache->m_bFlipHoldBodyWhenReverse;
	if(part_args.flip_texture_vertically)
	{
		swap( vpSprTop, vpSprBottom );
		swap( pSpriteTop, pSpriteBottom );
	}

	const bool bWavyPartsNeedZBuffer = ArrowEffects::NeedZBuffer();
	DISPLAY->SetZTestMode( bWavyPartsNeedZBuffer?ZTEST_WRITE_ON_PASS:ZTEST_OFF );
	DISPLAY->SetZWrite( bWavyPartsNeedZBuffer );

	// Hack: Z effects need a finer grain step.
	part_args.y_step = bWavyPartsNeedZBuffer? 4: 16; // use small steps only if wavy

	if(part_args.flip_texture_vertically)
	{
		y_head -= cache->m_iStopDrawingHoldBodyOffsetFromTail;
		y_tail -= cache->m_iStartDrawingHoldBodyOffsetFromHead;
	}
	else
	{
		y_head += cache->m_iStartDrawingHoldBodyOffsetFromHead;
		y_tail += cache->m_iStopDrawingHoldBodyOffsetFromTail;
	}

	float ae_zoom= ArrowEffects::GetZoom(m_pPlayerState);
	const float frame_height_top= pSpriteTop->GetUnzoomedHeight() * ae_zoom;
	const float frame_height_bottom= pSpriteBottom->GetUnzoomedHeight() * ae_zoom;

	part_args.y_start_pos= ArrowEffects::GetYPos(column_args.column,
		field_args.draw_pixels_after_targets, m_fYReverseOffsetPixels);
	part_args.y_end_pos= ArrowEffects::GetYPos(column_args.column,
		field_args.draw_pixels_before_targets, m_fYReverseOffsetPixels);
	if(reverse)
	{
		swap(part_args.y_start_pos, part_args.y_end_pos);
	}
	// So that part_args.y_start_pos can be changed when drawing the bottom.
	const float original_y_start_pos= part_args.y_start_pos;
	const float head_minus_top= y_head - frame_height_top;
	const float tail_plus_bottom= y_tail + frame_height_bottom;

	part_args.anchor_to_top= reverse && cache->m_bTopHoldAnchorWhenReverse;

	DISPLAY->SetTextureMode(TextureUnit_1, TextureMode_Modulate);
	DrawHoldBodyInternal(vpSprTop, vpSprBody, vpSprBottom, field_args,
		column_args, part_args, head_minus_top,
		tail_plus_bottom, y_head, y_tail, top_beat, bottom_beat,
		false);
	part_args.y_start_pos= original_y_start_pos;
	DISPLAY->SetTextureMode(TextureUnit_1, TextureMode_Glow);
	DrawHoldBodyInternal(vpSprTop, vpSprBody, vpSprBottom, field_args,
		column_args, part_args, head_minus_top,
		tail_plus_bottom, y_head, y_tail, top_beat, bottom_beat,
		true);
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

	const float fYHead= ArrowEffects::GetYPos(column_args.column, fStartYOffset, m_fYReverseOffsetPixels);
	const float fYTail= ArrowEffects::GetYPos(column_args.column, fEndYOffset, m_fYReverseOffsetPixels);

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

	DrawHoldBody(tn, field_args, column_args, fBeat, bIsBeingHeld, fYHead, fYTail, fPercentFadeToFail, fColorScale, top_beat, bottom_beat);

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
	{
		return;
	}
	float spline_beat= fBeat;
	if(is_being_held) { spline_beat= column_args.song_beat; }

	const float fAlpha= ArrowEffects::GetAlpha(column_args.column, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, field_args.draw_pixels_before_targets, field_args.fade_before_targets);
	const float fGlow= ArrowEffects::GetGlow(column_args.column, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, field_args.draw_pixels_before_targets, field_args.fade_before_targets);
	const RageColor diffuse	= RageColor(
		column_args.diffuse.r * fColorScale,
		column_args.diffuse.g * fColorScale,
		column_args.diffuse.b * fColorScale,
		column_args.diffuse.a * fAlpha);
	const RageColor glow = RageColor(1, 1, 1, fGlow);
	// We can't actually use the glow color from the effect on the colum actor
	// because it's used by the stealth modifier. -Kyz
	/*
	const RageColor glow	= RageColor(
		column_args.glow.r * fColorScale,
		column_args.glow.g * fColorScale,
		column_args.glow.b * fColorScale,
		column_args.glow.a * fGlow);
	*/

	bool bIsHoldHead = tn.type == TapNoteType_HoldHead;
	bool bIsHoldCap = bIsHoldHead || tn.type == TapNoteType_HoldTail;

	// So, thie call to GetBrightness does nothing because fColorScale is not
	// used after this point.  If you read GetBrightness, it looks like it's
	// meant to fade the note to black, so a note that is one beat past the
	// receptors is black.  However, I looked through the github history and
	// it's been down here, disabled, since at least SM5 beta 1a.  I don't
	// know if we should bring that behavior back now. -Kyz
	if( tn.type != TapNoteType_HoldHead )
	{ fColorScale *= ArrowEffects::GetBrightness(m_pPlayerState, fBeat); }

	// same logical structure as in UpdateReceptorGhostStuff, I just haven't
	// figured out a good way to combine them. -Kyz
	RageVector3 sp_pos;
	RageVector3 sp_rot;
	RageVector3 sp_zoom;
	RageVector3 ae_pos;
	RageVector3 ae_rot;
	RageVector3 ae_zoom;
	column_args.spae_pos_for_beat(m_pPlayerState, spline_beat,
		fYOffset, m_fYReverseOffsetPixels, sp_pos, ae_pos);

	switch(column_args.rot_handler->m_spline_mode)
	{
		case NCSM_Disabled:
			if(!bIsHoldCap)
			{
				ae_rot.x= ArrowEffects::GetRotationX(fYOffset);
			}
			ae_rot.y= ArrowEffects::GetRotationY(fYOffset);
			ae_rot.z= ArrowEffects::GetRotationZ(m_pPlayerState, fBeat, bIsHoldHead);
			break;
		case NCSM_Offset:
			if(!bIsHoldCap)
			{
				ae_rot.x= ArrowEffects::GetRotationX(fYOffset);
			}
			ae_rot.y= ArrowEffects::GetRotationY(fYOffset);
			ae_rot.z= ArrowEffects::GetRotationZ(m_pPlayerState, fBeat, bIsHoldHead);
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
	else if( tn.type == TapNoteType_Fake )
	{
		pActor = GetTapActor( m_TapFake, NotePart_Fake, fBeat );
		part = NotePart_Fake;
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
	RageVector3 sp_pos;
	RageVector3 sp_rot;
	RageVector3 sp_zoom;
	RageVector3 ae_pos;
	RageVector3 ae_rot;
	RageVector3 ae_zoom;
	switch(NCR_current.m_pos_handler.m_spline_mode)
	{
		case NCSM_Disabled:
			ArrowEffects::GetXYZPos(player_state, m_column, 0, m_field_render_args->reverse_offset_pixels, ae_pos);
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
			ae_rot.z= ArrowEffects::ReceptorGetRotationZ(player_state);
			break;
		case NCSM_Offset:
			ae_rot.z= ArrowEffects::ReceptorGetRotationZ(player_state);
			NCR_current.m_rot_handler.EvalForReceptor(song_beat, sp_rot);
			break;
		case NCSM_Position:
			NCR_current.m_rot_handler.EvalForReceptor(song_beat, sp_rot);
			break;
	}
	switch(NCR_current.m_zoom_handler.m_spline_mode)
	{
		case NCSM_Disabled:
			ae_zoom.x= ae_zoom.y= ae_zoom.z= ArrowEffects::GetZoom(player_state);
			break;
		case NCSM_Offset:
			ae_zoom.x= ae_zoom.y= ae_zoom.z= ArrowEffects::GetZoom(player_state);
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
			case TapNoteType_Fake:
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
