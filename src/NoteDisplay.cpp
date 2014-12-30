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
	const GameInput GameI = GAMESTATE->GetCurrentStyle()->StyleInputToGameInput( iColNum, pn );

	const RString &sButton = GAMESTATE->GetCurrentStyle()->ColToButtonName( iColNum );

	cache->Load( sButton );

	// "normal" note types
	m_TapNote.Load(		sButton, "Tap Note", pn, GameI.controller );
	//m_TapAdd.Load(		sButton, "Tap Addition", pn, GameI.controller );
	m_TapMine.Load(		sButton, "Tap Mine", pn, GameI.controller );
	m_TapLift.Load(		sButton, "Tap Lift", pn, GameI.controller );
	m_TapFake.Load(		sButton, "Tap Fake", pn, GameI.controller );

	// hold types
	FOREACH_HoldType( ht )
	{
		FOREACH_ActiveType( at )
		{
			m_HoldHead[ht][at].Load(	sButton, HoldTypeToString(ht)+" Head "+ActiveTypeToString(at), pn, GameI.controller );
			m_HoldTopCap[ht][at].Load(	sButton, HoldTypeToString(ht)+" Topcap "+ActiveTypeToString(at), pn, GameI.controller );
			m_HoldBody[ht][at].Load(	sButton, HoldTypeToString(ht)+" Body "+ActiveTypeToString(at), pn, GameI.controller );
			m_HoldBottomCap[ht][at].Load(	sButton, HoldTypeToString(ht)+" Bottomcap "+ActiveTypeToString(at), pn, GameI.controller );
			m_HoldTail[ht][at].Load(	sButton, HoldTypeToString(ht)+" Tail "+ActiveTypeToString(at), pn, GameI.controller );
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

float NoteDisplay::BeatToTValue(CommonColumnRenderArgs const& args, float beat) const
{
	float song_beat= m_pPlayerState->GetDisplayedPosition().m_fSongBeatVisible;
	float relative_beat= beat - song_beat;
	return (relative_beat / args.beats_per_t) - args.receptor_t;
}

bool NoteDisplay::DrawHoldsInRange(CommonColumnRenderArgs const& args,
	int column, vector<NoteData::TrackMap::const_iterator> const& tap_set)
{
	bool any_upcoming = false;
	for(vector<NoteData::TrackMap::const_iterator>::const_iterator tapit=
		tap_set.begin(); tapit != tap_set.end(); ++tapit)
	{
		TapNote const& tn= (*tapit)->second;
		HoldNoteResult const& result= tn.HoldResult;
		int start_row= (*tapit)->first;
		int end_row = start_row + tn.iDuration;

		// TRICKY: If boomerang is on, then all notes in the range
		// [first_row,last_row] aren't necessarily visible.
		// Test every note to make sure it's on screen before drawing
		float throw_away;
		bool start_past_peak = false;
		bool end_past_peak = false;
		float start_y	= ArrowEffects::GetYOffset(m_pPlayerState, column,
			NoteRowToVisibleBeat(m_pPlayerState, start_row), throw_away,
			start_past_peak);
		float end_y	= ArrowEffects::GetYOffset(m_pPlayerState, column,
			NoteRowToVisibleBeat(m_pPlayerState, end_row), throw_away,
			end_past_peak);
		bool tail_visible = args.draw_pixels_after_targets <= end_y &&
			end_y <= args.draw_pixels_before_targets;
		bool head_visible = args.draw_pixels_after_targets <= start_y  &&
			start_y <= args.draw_pixels_before_targets;
		bool straddling_visible = start_y <= args.draw_pixels_after_targets &&
			args.draw_pixels_before_targets <= end_y;
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
			args.ghost_row->SetHoldShowing(column, tn);
		}

		ASSERT_M(NoteRowToBeat(start_row) > -2000, ssprintf("%i %i %i", start_row, end_row, column));

		bool in_selection_range = false;
		if(*args.selection_begin_marker != -1 && *args.selection_end_marker != -1)
		{
			in_selection_range = (*args.selection_begin_marker <= start_row &&
				end_row < *args.selection_end_marker);
		}

		DrawHold(tn, args, column, start_row, is_holding, result,
			use_addition_coloring,
			in_selection_range ? args.selection_glow : args.fail_fade);

		bool note_upcoming = NoteRowToBeat(start_row) >
			m_pPlayerState->GetDisplayedPosition().m_fSongBeat;
		any_upcoming |= note_upcoming;
	}
	return any_upcoming;
}

bool NoteDisplay::DrawTapsInRange(CommonColumnRenderArgs const& args,
	int column, vector<NoteData::TrackMap::const_iterator> const& tap_set)
{
	bool any_upcoming= false;
	// draw notes from furthest to closest
	for(vector<NoteData::TrackMap::const_iterator>::const_iterator tapit=
		tap_set.begin(); tapit != tap_set.end(); ++tapit)
	{
		int tap_row= (*tapit)->first;
		TapNote const& tn= (*tapit)->second;

		// TRICKY: If boomerang is on, then all notes in the range
		// [first_row,last_row] aren't necessarily visible.
		// Test every note to make sure it's on screen before drawing.
		if(!IsOnScreen(NoteRowToBeat(tap_row), column,
				args.draw_pixels_after_targets, args.draw_pixels_before_targets))
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
			for(int c2= 0; c2 < args.note_data->GetNumTracks(); ++c2)
			{
				const TapNote &tmp = args.note_data->GetTapNote(c2, tap_row);
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
			for(int c2= 0; c2 < args.note_data->GetNumTracks(); ++c2)
			{
				const TapNote &tmp = args.note_data->GetTapNote(c2, tap_row);
				if(tmp.type == TapNoteType_HoldHead &&
					tmp.subType == TapNoteSubType_Roll)
				{
					roll_begins_on_this_beat = true;
					break;
				}
			}
		}

		bool in_selection_range = false;
		if(*args.selection_begin_marker != -1 && *args.selection_end_marker != -1)
		{
			in_selection_range = *args.selection_begin_marker <= tap_row &&
				tap_row < *args.selection_end_marker;
		}

		bool is_addition = (tn.source == TapNoteSource_Addition);
		bool hopo_possible = (tn.bHopoPossible);
		bool use_addition_coloring = is_addition || hopo_possible;
		DrawTap(tn, args, column, NoteRowToVisibleBeat(m_pPlayerState, tap_row),
			hold_begins_on_this_beat, roll_begins_on_this_beat,
			use_addition_coloring,
			in_selection_range ? args.selection_glow : args.fail_fade);

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
	CommonColumnRenderArgs const& args, int iCol, int fYStep,
	float fPercentFadeToFail, float fColorScale, bool bGlow,
	float fOverlappedTime, float fYTop, float fYBottom, float fYStartPos,
	float fYEndPos, bool bWrapping, bool bAnchorToTop,
	bool bFlipTextureVertically, float top_t, float bottom_t)
{
	ASSERT( !vpSpr.empty() );

	Sprite *pSprite = vpSpr.front();
	FOREACH( Sprite *, vpSpr, s )
	{
		(*s)->SetZoom( ArrowEffects::GetZoom(m_pPlayerState) );
		ASSERT( (*s)->GetUnzoomedWidth() == pSprite->GetUnzoomedWidth() );
		ASSERT( (*s)->GetUnzoomedHeight() == pSprite->GetUnzoomedHeight() );
	}

	// draw manually in small segments
	RectF rect = *pSprite->GetCurrentTextureCoordRect();
	if( bFlipTextureVertically )
		swap( rect.top, rect.bottom );
	const float fFrameWidth		= pSprite->GetZoomedWidth();
	const float fFrameHeight	= pSprite->GetZoomedHeight();

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

	StripBuffer queue;
	for( float fY = fYStartPos; !bLast; fY += fYStep )
	{
		if( fY >= fYEndPos )
		{
			fY = fYEndPos;
			bLast = true;
		}

		float curtsy= top_t;
		if(top_t != bottom_t)
		{
			curtsy= SCALE(fY, fYTop, fYBottom, top_t, bottom_t);
		}
		vector<float> spline_pos;
		args.spline->evaluate(curtsy, spline_pos);

		const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ			= ArrowEffects::GetZPos( m_pPlayerState, iCol, fYOffset ) + spline_pos[2];
		const float fFrameWidthScale	= ArrowEffects::GetFrameWidthScale( m_pPlayerState, fYOffset, fOverlappedTime );
		const float fScaledFrameWidth	= fFrameWidth * fFrameWidthScale;

		float fX			= ArrowEffects::GetXPos( m_pPlayerState, iCol, fYOffset ) + spline_pos[0];

		// XXX: Actor rotations use degrees, RageFastCos/Sin use radians. Convert here.
		const float fRotationY		= ArrowEffects::GetRotationY( m_pPlayerState, fYOffset ) * PI/180;

		// if we're rotating, we need to modify the X and Z coords for the outer edges.
		const float fRotOffsetX		= (fScaledFrameWidth/2) * RageFastCos(fRotationY);
		const float fRotOffsetZ		= (fScaledFrameWidth/2) * RageFastSin(fRotationY);

		//const float fXLeft			= fX - (fScaledFrameWidth/2);
		const float fXLeft			= fX - fRotOffsetX;
		const float fXCenter		= fX;
		//const float fXRight		= fX + (fScaledFrameWidth/2);
		const float fXRight		= fX + fRotOffsetX;
		const float fZLeft			= fZ - fRotOffsetZ;
		const float fZCenter	= fZ;
		const float fZRight		= fZ + fRotOffsetZ;

		const float fDistFromTop	= fY - fYTop;
		float fTexCoordTop		= SCALE( fDistFromTop, 0, fFrameHeight, rect.top, rect.bottom );
		fTexCoordTop += fAddToTexCoord;

		const float fAlpha		= ArrowGetAlphaOrGlow( bGlow, m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, args.draw_pixels_before_targets, args.fade_before_targets );
		const RageColor color		= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		queue.v[0].p = RageVector3(fXLeft,  fY + spline_pos[1], fZLeft);  queue.v[0].c = color; queue.v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		queue.v[1].p = RageVector3(fXCenter, fY + spline_pos[1], fZCenter); queue.v[1].c = color; queue.v[1].t = RageVector2(fTexCoordCenter, fTexCoordTop);
		queue.v[2].p = RageVector3(fXRight, fY + spline_pos[1], fZRight);  queue.v[2].c = color; queue.v[2].t = RageVector2(fTexCoordRight, fTexCoordTop);
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
	CommonColumnRenderArgs const& args, int iCol, float fBeat,
	bool bIsBeingHeld, float fYHead, float fYTail, bool bIsAddition,
	float fPercentFadeToFail, float fColorScale, bool bGlow,
	float top_t, float bottom_t)
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

	const bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(iCol) > 0.5f;
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

	float fYStartPos = ArrowEffects::GetYPos( m_pPlayerState, iCol,
		args.draw_pixels_after_targets, m_fYReverseOffsetPixels );
	float fYEndPos = ArrowEffects::GetYPos( m_pPlayerState, iCol,
		args.draw_pixels_before_targets, m_fYReverseOffsetPixels );
	if( bReverse )
		swap( fYStartPos, fYEndPos );

	bool bTopAnchor = bReverse && cache->m_bTopHoldAnchorWhenReverse;

	// Draw the top cap
	DrawHoldPart(vpSprTop, args, iCol, fYStep, fPercentFadeToFail,
		fColorScale, bGlow,
		tn.HoldResult.fOverlappedTime,
		fYHead-fFrameHeightTop, fYHead,
		fYStartPos, fYEndPos,
		false, bTopAnchor, bFlipHoldBody, top_t, top_t);

	// Draw the body
	DrawHoldPart(vpSprBody, args, iCol, fYStep, fPercentFadeToFail,
		fColorScale, bGlow,
		tn.HoldResult.fOverlappedTime,
		fYHead, fYTail,
		fYStartPos, fYEndPos,
		true, bTopAnchor, bFlipHoldBody, top_t, bottom_t);

	// Draw the bottom cap
	DrawHoldPart(vpSprBottom, args, iCol, fYStep, fPercentFadeToFail,
		fColorScale, bGlow,
		tn.HoldResult.fOverlappedTime,
		fYTail, fYTail+fFrameHeightBottom,
		max(fYStartPos, fYHead), fYEndPos,
		false, bTopAnchor, bFlipHoldBody, bottom_t, bottom_t);
}

void NoteDisplay::DrawHold(const TapNote& tn,
	CommonColumnRenderArgs const& args, int iCol, int iRow, bool bIsBeingHeld,
	const HoldNoteResult &Result, bool bIsAddition, float fPercentFadeToFail)
{
	int iEndRow = iRow + tn.iDuration;
	float top_t= BeatToTValue(args, NoteRowToVisibleBeat(m_pPlayerState, iRow));
	float bottom_t= BeatToTValue(args, NoteRowToVisibleBeat(m_pPlayerState, iEndRow));
	if(bIsBeingHeld)
	{
		top_t= args.receptor_t;
	}

	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(iCol) > 0.5f;
	float fStartBeat = NoteRowToBeat( max(tn.HoldResult.iLastHeldRow, iRow) );
	float fThrowAway = 0;

	// HACK: If life > 0, don't set YOffset to 0 so that it doesn't jiggle around the receptor.
	bool bStartIsPastPeak = true;
	float fStartYOffset	= 0;
	if( tn.HoldResult.bActive  &&  tn.HoldResult.fLife > 0 )
		;	// use the default values filled in above
	else
		fStartYOffset = ArrowEffects::GetYOffset( m_pPlayerState, iCol, fStartBeat, fThrowAway, bStartIsPastPeak );
	
	float fEndPeakYOffset	= 0;
	bool bEndIsPastPeak = false;
	float fEndYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, iCol, NoteRowToBeat(iEndRow), fEndPeakYOffset, bEndIsPastPeak );

	// In boomerang, the arrows reverse direction at Y offset value fPeakAtYOffset.  
	// If fPeakAtYOffset lies inside of the hold we're drawing, then the we 
	// want to draw the tail at that max Y offset, or else the hold will appear 
	// to magically grow as the tail approaches the max Y offset.
	if( bStartIsPastPeak && !bEndIsPastPeak )
		fEndYOffset	= fEndPeakYOffset;	// use the calculated PeakYOffset so that long holds don't appear to grow
	
	// Swap in reverse, so fStartYOffset is always the offset higher on the screen.
	if( bReverse )
		swap( fStartYOffset, fEndYOffset );

	const float fYHead		= ArrowEffects::GetYPos( m_pPlayerState, iCol, fStartYOffset, m_fYReverseOffsetPixels );
	const float fYTail		= ArrowEffects::GetYPos( m_pPlayerState, iCol, fEndYOffset, m_fYReverseOffsetPixels );

	const float fColorScale		= SCALE( tn.HoldResult.fLife, 0.0f, 1.0f, cache->m_fHoldLetGoGrayPercent, 1.0f );

	bool bFlipHeadAndTail = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	/* The body and caps should have no overlap, so their order doesn't matter.
	 * Draw the head last, so it appears on top. */
	float fBeat = NoteRowToBeat(iRow);
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

	DrawHoldBody(tn, args, iCol, fBeat, bIsBeingHeld, fYHead, fYTail, bIsAddition, fPercentFadeToFail, fColorScale, false, top_t, bottom_t);
	DrawHoldBody(tn, args, iCol, fBeat, bIsBeingHeld, fYHead, fYTail, bIsAddition, fPercentFadeToFail, fColorScale, true, top_t, bottom_t);

	/* These set the texture mode themselves. */
	// this part was modified in pumpmania, where it flips the draw order
	// of the head and tail. Perhaps make this a theme/noteskin metric? -aj
	if( cache->m_bHoldTailIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldTail, NotePart_HoldTail, NoteRowToBeat(iRow), tn.subType == TapNoteSubType_Roll, bIsBeingHeld );
		DrawActor(tn, pActor, NotePart_HoldTail, args, iCol, bFlipHeadAndTail ? fStartYOffset : fEndYOffset, fBeat, bIsAddition, fPercentFadeToFail, fColorScale, false);
	}
	if( cache->m_bHoldHeadIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, NoteRowToBeat(iRow), tn.subType == TapNoteSubType_Roll, bIsBeingHeld );
		DrawActor(tn, pActor, NotePart_HoldHead, args, iCol, bFlipHeadAndTail ? fEndYOffset : fStartYOffset, fBeat, bIsAddition, fPercentFadeToFail, fColorScale, bIsBeingHeld);
	}
}

void NoteDisplay::DrawActor(const TapNote& tn, Actor* pActor, NotePart part,
	CommonColumnRenderArgs const& args, int iCol, float fYOffset, float fBeat,
	bool bIsAddition, float fPercentFadeToFail, float fColorScale,
	bool is_being_held)
{
	if (tn.type == TapNoteType_AutoKeysound && !GAMESTATE->m_bInStepEditor) return;
	if(fYOffset < args.draw_pixels_after_targets ||
		fYOffset > args.draw_pixels_before_targets)
		return;
	float spline_t= BeatToTValue(args, fBeat);
	if(is_being_held) { spline_t= args.receptor_t; }
	vector<float> spline_pos;
	args.spline->evaluate(spline_t, spline_pos);
	const float fY		= ArrowEffects::GetYPos(	m_pPlayerState, iCol, fYOffset, m_fYReverseOffsetPixels ) + spline_pos[1];
	const float fX		= ArrowEffects::GetXPos(	m_pPlayerState, iCol, fYOffset ) + spline_pos[0];
	const float fZ		= ArrowEffects::GetZPos(	m_pPlayerState, iCol, fYOffset ) + spline_pos[2];
	const float fAlpha	= ArrowEffects::GetAlpha(	m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, args.draw_pixels_before_targets, args.fade_before_targets );
	const float fGlow	= ArrowEffects::GetGlow(	m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, args.draw_pixels_before_targets, args.fade_before_targets );
	const RageColor diffuse	= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	const RageColor glow	= RageColor(1,1,1,fGlow);
	float fRotationX	= 0, fRotationZ	= 0;
	const float fRotationY = ArrowEffects::GetRotationY( m_pPlayerState, fYOffset );

	bool bIsHoldHead = tn.type == TapNoteType_HoldHead;
	bool bIsHoldCap = bIsHoldHead || tn.type == TapNoteType_HoldTail;
	
	fRotationZ = ArrowEffects::GetRotationZ( m_pPlayerState, fBeat, bIsHoldHead );
	if( !bIsHoldCap )
	{
		fRotationX = ArrowEffects::GetRotationX( m_pPlayerState, fYOffset );
	}

	if( tn.type != TapNoteType_HoldHead )
		fColorScale		*= ArrowEffects::GetBrightness(	m_pPlayerState, fBeat );

	pActor->SetRotationX( fRotationX );
	pActor->SetRotationY( fRotationY );
	pActor->SetRotationZ( fRotationZ );
	pActor->SetXY( fX, fY );
	pActor->SetZ( fZ );
	pActor->SetZoom( ArrowEffects::GetZoom(m_pPlayerState) );
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
	CommonColumnRenderArgs const& args, int iCol, float fBeat,
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

	const float fYOffset = ArrowEffects::GetYOffset( m_pPlayerState, iCol, fBeat );
	// this is the line that forces the (1,1,1,x) part of the noteskin diffuse -aj
	DrawActor(tn, pActor, part, args, iCol, fYOffset, fBeat, bIsAddition, fPercentFadeToFail, 1.0f, false);

	if( tn.type == TapNoteType_Attack )
		pActor->PlayCommand( "UnsetAttack" );
}

void NoteColumnRenderer::DrawPrimitives()
{
	m_render_args->spline= &m_spline;
	m_render_args->receptor_t= m_receptor_t;
	m_render_args->beats_per_t= m_beats_per_t;
	m_render_args->first_beat= NoteRowToBeat(m_render_args->first_row);
	m_render_args->last_beat= NoteRowToBeat(m_render_args->last_row);
	bool any_upcoming= false;
	// Build lists of holds and taps for each player number, then pass those
	// lists to the displays to draw.
	// The vector in the NUM_PlayerNumber slot should stay empty, not worth
	// optimizing it out. -Kyz
	vector<vector<NoteData::TrackMap::const_iterator> > holds(PLAYER_INVALID+1);
	vector<vector<NoteData::TrackMap::const_iterator> > taps(PLAYER_INVALID+1);
	NoteData::TrackMap::const_iterator begin, end;
	m_render_args->note_data->GetTapNoteRangeInclusive(m_column,
		m_render_args->first_row, m_render_args->last_row+1, begin, end);
	for(; begin != end; ++begin)
	{
		TapNote const& tn= begin->second;
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
		any_upcoming|= disp->draw_func(*m_render_args, m_column, tap_set[pn]); \
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
	m_render_args->receptor_row->SetNoteUpcoming(m_column, any_upcoming);
}

#include "LuaBinding.h"

struct LunaNoteColumnRenderer : Luna<NoteColumnRenderer>
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
	SET_T(m_receptor_t, set_receptor_t);
	SET_T(m_beats_per_t, set_beats_per_t);

	LunaNoteColumnRenderer()
	{
		ADD_METHOD(get_spline);
		ADD_METHOD(get_receptor_t);
		ADD_METHOD(get_beats_per_t);
		ADD_METHOD(set_receptor_t);
		ADD_METHOD(set_beats_per_t);
	}
};

LUA_REGISTER_DERIVED_CLASS(NoteColumnRenderer, Actor)

/*
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
