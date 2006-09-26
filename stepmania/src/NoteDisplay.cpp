#include "global.h"
#include "NoteDisplay.h"
#include "GameState.h"
#include "NoteSkinManager.h"
#include "ArrowEffects.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "ActorUtil.h"
#include "Game.h"
#include "PlayerState.h"
#include "Sprite.h"
#include "NoteTypes.h"
#include "LuaBinding.h"

const RString& NoteNotePartToString( NotePart i );
#define FOREACH_NotePart( i ) FOREACH_ENUM( NotePart, NUM_NotePart, i )

static const char *NotePartNames[] = {
	"TapNote",
	"TapAddition",
	"TapMine",
	"TapLift",
	"HoldHead",
	"HoldTopCap",
	"HoldBody",
	"HoldBottomCap",
	"HoldTail",
};
XToString( NotePart, NUM_NotePart );

static const RageVector2 g_emptyVector = RageVector2( 0, 0 );

// Don't require that NoteSkins have more than 8 colors.  Using 9 colors to display 192nd notes
// would double the number of texture memory needed for many NoteSkin graphics versus just having
// 8 colors.
static const NoteType MAX_DISPLAY_NOTE_TYPE = (NoteType)7;

// cache
struct NoteMetricCache_t
{
	bool m_bDrawHoldHeadForTapsOnSameRow;
	float m_fAnimationLengthInBeats[NUM_NotePart];
	bool m_bAnimationIsVivid[NUM_NotePart];
	RageVector2 m_fNoteColorTextureCoordSpacing[NUM_NotePart];

	bool m_bHoldHeadIsAboveWavyParts;
	bool m_bHoldTailIsAboveWavyParts;
	int m_iStartDrawingHoldBodyOffsetFromHead;
	int m_iStopDrawingHoldBodyOffsetFromTail;
	float m_fHoldLetGoGrayPercent;
	bool m_bTapNoteUseLighting;
	bool m_bTapAdditionUseLighting;
	bool m_bTapMineUseLighting;
	bool m_bTapLiftUseLighting;
	bool m_bHoldHeadUseLighting;
	bool m_bHoldTailUseLighting;
	bool m_bFlipHeadAndTailWhenReverse;

	void Load( const RString &sButton );
} *NoteMetricCache;

void NoteMetricCache_t::Load( const RString &sButton )
{
	m_bDrawHoldHeadForTapsOnSameRow = NOTESKIN->GetMetricB(sButton,"DrawHoldHeadForTapsOnSameRow");
	FOREACH_NotePart( p )
	{
		const RString &s = NotePartToString(p);
		m_fAnimationLengthInBeats[p] = NOTESKIN->GetMetricF(sButton,s+"AnimationLengthInBeats");
		m_bAnimationIsVivid[p] = NOTESKIN->GetMetricB(sButton,s+"AnimationIsVivid");
		m_fNoteColorTextureCoordSpacing[p].x = NOTESKIN->GetMetricF(sButton,s+"NoteColorTextureCoordSpacingX");
		m_fNoteColorTextureCoordSpacing[p].y = NOTESKIN->GetMetricF(sButton,s+"NoteColorTextureCoordSpacingY");
	}
	m_bHoldHeadIsAboveWavyParts =		NOTESKIN->GetMetricB(sButton,"HoldHeadIsAboveWavyParts");
	m_bHoldTailIsAboveWavyParts =		NOTESKIN->GetMetricB(sButton,"HoldTailIsAboveWavyParts");
	m_iStartDrawingHoldBodyOffsetFromHead =	NOTESKIN->GetMetricI(sButton,"StartDrawingHoldBodyOffsetFromHead");
	m_iStopDrawingHoldBodyOffsetFromTail =	NOTESKIN->GetMetricI(sButton,"StopDrawingHoldBodyOffsetFromTail");
	m_fHoldLetGoGrayPercent =		NOTESKIN->GetMetricF(sButton,"HoldLetGoGrayPercent");
	m_bTapNoteUseLighting =			NOTESKIN->GetMetricB(sButton,"TapNoteUseLighting");
	m_bTapAdditionUseLighting =		NOTESKIN->GetMetricB(sButton,"TapAdditionUseLighting");
	m_bTapMineUseLighting =			NOTESKIN->GetMetricB(sButton,"TapMineUseLighting");
	m_bTapLiftUseLighting =			NOTESKIN->GetMetricB(sButton,"TapLiftUseLighting");
	m_bHoldHeadUseLighting =		NOTESKIN->GetMetricB(sButton,"HoldHeadUseLighting");
	m_bHoldTailUseLighting =		NOTESKIN->GetMetricB(sButton,"HoldTailUseLighting");
	m_bFlipHeadAndTailWhenReverse =		NOTESKIN->GetMetricB(sButton,"FlipHeadAndTailWhenReverse");
}


struct NoteSkinAndPath
{
	NoteSkinAndPath( const RString sNoteSkin_, const RString sPath_ ) : sNoteSkin(sNoteSkin_), sPath(sPath_) { }
	RString sNoteSkin;
	RString sPath;
	bool operator<( const NoteSkinAndPath &other ) const
	{
		int cmp = strcmp(sNoteSkin, other.sNoteSkin);
		if( cmp < 0 )
			return true;
		else if( cmp == 0 )
			return sPath < other.sPath;
		else
			return false;
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

	const NoteSkinAndPath m_nsap; /* should be refcounted along with g_NoteResource[] */
	int m_iRefCount;
	Actor *m_pActor;
};

static map<NoteSkinAndPath, NoteResource *> g_NoteResource;

static NoteResource *MakeNoteResource( const RString &sButton, const RString &sElement, bool bSpriteOnly )
{
	RString sElementAndType = sElement;
	NoteSkinAndPath nsap( NOTESKIN->GetCurrentNoteSkin(), NOTESKIN->GetPath(sButton, sElementAndType) );

	map<NoteSkinAndPath, NoteResource *>::iterator it = g_NoteResource.find( nsap );
	if( it == g_NoteResource.end() )
	{
		NoteResource *pRes = new NoteResource( nsap );

		pRes->m_pActor = ActorUtil::MakeActor( nsap.sPath );
		ASSERT( pRes->m_pActor );

		/* Make sure pActor is a Sprite (or something derived from Sprite). */
		if( bSpriteOnly )
		{
			Lua *L = LUA->Get();
			pRes->m_pActor->PushSelf( L );
			Luna<Sprite>::check( L, lua_gettop(L) );
			lua_pop( L, 1 );
			LUA->Release( L );
		}

		g_NoteResource[nsap] = pRes;
		it = g_NoteResource.find( nsap );
	}

	NoteResource *pRet = it->second;
	++pRet->m_iRefCount;
	return pRet;
}

static NoteResource *FindNoteResource( const Actor *pActor )
{
	map<NoteSkinAndPath, NoteResource *>::iterator it;
	for( it = g_NoteResource.begin(); it != g_NoteResource.end(); ++it )
	{
		NoteResource *pRes = it->second;
		if( pRes->m_pActor == pActor )
			return pRes;
	}

	return NULL;
}

static void DeleteNoteResource( const Actor *pActor )
{
	if( pActor == NULL )
		return;

	NoteResource *pRes = FindNoteResource( pActor );
	ASSERT( pRes != NULL );

	ASSERT_M( pRes->m_iRefCount > 0, ssprintf("%i", pRes->m_iRefCount) );
	--pRes->m_iRefCount;
	if( pRes->m_iRefCount )
		return;

	g_NoteResource.erase( pRes->m_nsap );
	delete pRes;
}

Actor *MakeRefcountedActor( const RString &sButton, const RString &sElement )
{
	NoteResource *pRes = MakeNoteResource( sButton, sElement, false );
	return pRes->m_pActor;
}

Sprite *MakeRefcountedSprite( const RString &sButton, const RString &sElement )
{
	NoteResource *pRes = MakeNoteResource( sButton, sElement, true );
	return (Sprite *) pRes->m_pActor; /* XXX ick */
}

NoteColorActor::NoteColorActor()
{
	m_p = NULL;
}

NoteColorActor::~NoteColorActor()
{
	if( m_p )
		DeleteNoteResource( m_p );
}

void NoteColorActor::Load( const RString &sButton, const RString &sElement )
{
	m_p = MakeRefcountedActor( sButton, sElement );
}


NoteColorSprite::NoteColorSprite()
{
	m_p = NULL;
}

NoteColorSprite::~NoteColorSprite()
{
	if( m_p )
		DeleteNoteResource( m_p );
}

void NoteColorSprite::Load( const RString &sButton, const RString &sElement )
{
	m_p = MakeRefcountedSprite( sButton, sElement );
}


static const char *HoldTypeNames[] = {
	"hold",
	"roll",
};
XToString( HoldType, NUM_HoldType );

static const char *ActiveTypeNames[] = {
	"active",
	"inactive",
};
XToString( ActiveType, NUM_ActiveType );



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

	const RString &sButton = GAMESTATE->GetCurrentGame()->ColToButtonName( iColNum );

	cache->Load( sButton );

	m_TapNote.Load(		sButton, "tap note" );
	m_TapAddition.Load(	sButton, "tap addition" );
	m_TapMine.Load(		sButton, "tap mine" );
	m_TapLift.Load(		sButton, "tap lift" );
	
	FOREACH_HoldType( ht )
	{
		FOREACH_ActiveType( at )
		{
			m_HoldHead[ht][at].Load(	sButton, HoldTypeToString(ht)+" head "+ActiveTypeToString(at) );
			m_HoldTopCap[ht][at].Load(	sButton, HoldTypeToString(ht)+" topcap "+ActiveTypeToString(at) );
			m_HoldBody[ht][at].Load(	sButton, HoldTypeToString(ht)+" body "+ActiveTypeToString(at) );
			m_HoldBottomCap[ht][at].Load(	sButton, HoldTypeToString(ht)+" bottomcap "+ActiveTypeToString(at) );
			m_HoldTail[ht][at].Load(	sButton, HoldTypeToString(ht)+" tail "+ActiveTypeToString(at) );
		}
	}
}

bool NoteDisplay::DrawHoldHeadForTapsOnSameRow() const
{
	return cache->m_bDrawHoldHeadForTapsOnSameRow;
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

void NoteDisplay::SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLengthInBeats, bool bVivid )
{
	/* -inf ... inf */
	float fSongBeat = GAMESTATE->m_fSongBeat;
	/* -len ... +len */
	float fPercentIntoAnimation = fmodf( fSongBeat, fAnimationLengthInBeats );
	/* -1 ... 1 */
	fPercentIntoAnimation /= fAnimationLengthInBeats;

	if( bVivid )
	{
		float fNoteBeatFraction = fmodf( fNoteBeat, 1.0f );

		const float fInterval = 1.f / fAnimationLengthInBeats;
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
	
	SetActiveFrame( fNoteBeat, *pActorOut, cache->m_fAnimationLengthInBeats[part], cache->m_bAnimationIsVivid[part] );
	return pActorOut;
}

Actor *NoteDisplay::GetHoldActor( NoteColorActor nca[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld )
{
	return GetTapActor( nca[bIsRoll ? roll:hold][bIsBeingHeld ? active:inactive], part, fNoteBeat );
}

Sprite *NoteDisplay::GetHoldSprite( NoteColorSprite ncs[NUM_HoldType][NUM_ActiveType], NotePart part, float fNoteBeat, bool bIsRoll, bool bIsBeingHeld )
{
	Sprite *pSpriteOut = ncs[bIsRoll ? roll:hold][bIsBeingHeld ? active:inactive].Get();
	
	SetActiveFrame( fNoteBeat, *pSpriteOut, cache->m_fAnimationLengthInBeats[part], cache->m_bAnimationIsVivid[part] );
	return pSpriteOut;
}

static float ArrowGetAlphaOrGlow( bool bGlow, const PlayerState* pPlayerState, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels )
{
	if( bGlow )
		return ArrowEffects::GetGlow( pPlayerState, iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels );
	else
		return ArrowEffects::GetAlpha( pPlayerState, iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels );
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
		DISPLAY->DrawQuadStrip( buf, v-buf );
	}
	int Used() const { return v - buf; }
	int Free() const { return size - Used(); }
};

void NoteDisplay::DrawHoldTopCap( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset )
{
	//
	// Draw the top cap (always wavy)
	//
	StripBuffer queue;

	Sprite *pSprTopCap = GetHoldSprite( m_HoldTopCap, NotePart_HoldTopCap, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );

	pSprTopCap->SetZoom( ArrowEffects::GetZoom( m_pPlayerState ) );

	// draw manually in small segments
	RageTexture* pTexture = pSprTopCap->GetTexture();
	const RectF *pRect = pSprTopCap->GetCurrentTextureCoordRect();
	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( TextureUnit_1, pTexture->GetTexHandle() );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetCullMode( CULL_NONE );
	DISPLAY->SetTextureWrapping(false);

	const float fFrameWidth  = pSprTopCap->GetZoomedWidth();
	const float fFrameHeight = pSprTopCap->GetZoomedHeight();
	const float fYCapTop	 = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead-fFrameHeight;
	const float fYCapBottom  = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead;

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(iCol) > 0.5f;

	if( bGlow )
		fColorScale = 1;

	float fDrawYCapTop;
	float fDrawYCapBottom;
	{
		float fYStartPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fYStartOffset, m_fYReverseOffsetPixels );
		float fYEndPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fYEndOffset, m_fYReverseOffsetPixels );
		fDrawYCapTop = max( fYCapTop, bReverse ? fYEndPos : fYStartPos );
		fDrawYCapBottom = min( fYCapBottom, bReverse ? fYStartPos : fYEndPos );
	}

	// don't draw any part of the head that is after the middle of the tail
	fDrawYCapBottom = min( fYTail, fDrawYCapBottom );

	bool bAllAreTransparent = true;
	bool bLast = false;

	float fY = fDrawYCapTop;
	for( ; !bLast; fY+=fYStep )
	{
		if( fY >= fDrawYCapBottom )
		{
			fY = fDrawYCapBottom;
			bLast = true;
		}

		const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ			= ArrowEffects::GetZPos( m_pPlayerState, iCol, fYOffset );
		const float fX			= ArrowEffects::GetXPos( m_pPlayerState, iCol, fYOffset );
		const float fXLeft		= fX - fFrameWidth/2;
		const float fXRight		= fX + fFrameWidth/2;
		const float fTopDistFromHeadTop	= fY - fYCapTop;
		const float fTexCoordTop	= SCALE( fTopDistFromHeadTop,    0, fFrameHeight, pRect->top, pRect->bottom );
		const float fTexCoordLeft	= pRect->left;
		const float fTexCoordRight	= pRect->right;
		const float fAlpha		= ArrowGetAlphaOrGlow( bGlow, m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor color		= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		queue.v[0].p = RageVector3(fXLeft,  fY, fZ);	queue.v[0].c = color; queue.v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		queue.v[1].p = RageVector3(fXRight, fY, fZ);	queue.v[1].c = color; queue.v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		queue.v+=2;
		if( queue.Free() < 2 )
		{
			/* The queue is full.  Render it, clear the buffer, and move back a step to
			 * start off the quad strip again. */
			if( !bAllAreTransparent )
				queue.Draw();
			queue.Init();
			bAllAreTransparent = true;
			fY -= fYStep;
		}
	}
	if( !bAllAreTransparent )
		queue.Draw();
}


void NoteDisplay::DrawHoldBody( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow,
							   float fYStartOffset, float fYEndOffset )
{
	//
	// Draw the body (always wavy)
	//
	StripBuffer queue;

	Sprite *pSprBody = GetHoldSprite( m_HoldBody, NotePart_HoldBody, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );

	pSprBody->SetZoom( ArrowEffects::GetZoom( m_pPlayerState ) );

	// draw manually in small segments
	RageTexture* pTexture = pSprBody->GetTexture();
	const RectF *pRect = pSprBody->GetCurrentTextureCoordRect();
	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( TextureUnit_1, pTexture->GetTexHandle() );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetCullMode( CULL_NONE );
	DISPLAY->SetTextureWrapping( true );


	const float fFrameWidth  = pSprBody->GetZoomedWidth();
	const float fFrameHeight = pSprBody->GetZoomedHeight();
	const float fYBodyTop = fYHead + cache->m_iStartDrawingHoldBodyOffsetFromHead;
	const float fYBodyBottom = fYTail + cache->m_iStopDrawingHoldBodyOffsetFromTail;

	const bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(iCol) > 0.5f;
	bool bAnchorToBottom = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	if( bGlow )
		fColorScale = 1;

	/* Only draw the section that's within the range specified.  If a hold note is
	 * very long, don't process or draw the part outside of the range.  Don't change
	 * fYBodyTop or fYBodyBottom; they need to be left alone to calculate texture
	 * coordinates. */
	float fDrawYBodyTop;
	float fDrawYBodyBottom;
	{
		float fYStartPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fYStartOffset, m_fYReverseOffsetPixels );
		float fYEndPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fYEndOffset, m_fYReverseOffsetPixels );

		fDrawYBodyTop = max( fYBodyTop, bReverse ? fYEndPos : fYStartPos );
		fDrawYBodyBottom = min( fYBodyBottom, bReverse ? fYStartPos : fYEndPos );
	}

	// top to bottom
	bool bAllAreTransparent = true;
	bool bLast = false;
	float fVertTexCoordOffset = 0;
	for( float fY = fDrawYBodyTop; !bLast; fY += fYStep )
	{
		if( fY >= fDrawYBodyBottom )
		{
			fY = fDrawYBodyBottom;
			bLast = true;
		}

		const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ			= ArrowEffects::GetZPos( m_pPlayerState, iCol, fYOffset );
		const float fX			= ArrowEffects::GetXPos( m_pPlayerState, iCol, fYOffset );
		const float fXLeft		= fX - fFrameWidth/2;
		const float fXRight		= fX + fFrameWidth/2;
		const float fDistFromBodyBottom	= fYBodyBottom - fY;
		const float fDistFromBodyTop	= fY - fYBodyTop;
		float fTexCoordTop		= SCALE( bAnchorToBottom ? fDistFromBodyTop : fDistFromBodyBottom,    0, fFrameHeight, pRect->bottom, pRect->top );
		/* For very large hold notes, shift the texture coordinates to be near 0, so we
		 * don't send very large values to the renderer. */
		if( fY == fDrawYBodyTop ) // first
				fVertTexCoordOffset = floorf( fTexCoordTop );
		fTexCoordTop -= fVertTexCoordOffset;
		const float fTexCoordLeft	= pRect->left;
		const float fTexCoordRight	= pRect->right;
		const float	fAlpha		= ArrowGetAlphaOrGlow( bGlow, m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor color		= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		queue.v[0].p = RageVector3(fXLeft,  fY, fZ);	queue.v[0].c = color; queue.v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		queue.v[1].p = RageVector3(fXRight, fY, fZ);	queue.v[1].c = color; queue.v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		queue.v+=2;
		if( queue.Free() < 2 )
		{
			/* The queue is full.  Render it, clear the buffer, and move back a step to
			 * start off the quad strip again. */
			if( !bAllAreTransparent )
				queue.Draw();
			queue.Init();
			bAllAreTransparent = true;
			fY -= fYStep;
		}
	}

	if( !bAllAreTransparent )
		queue.Draw();
}

void NoteDisplay::DrawHoldBottomCap( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fYTail, int	fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset )
{
	//
	// Draw the bottom cap (always wavy)
	//
	StripBuffer queue;

	Sprite* pBottomCap = GetHoldSprite( m_HoldBottomCap, NotePart_HoldBottomCap, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );

	pBottomCap->SetZoom( ArrowEffects::GetZoom( m_pPlayerState ) );

	// draw manually in small segments
	RageTexture* pTexture = pBottomCap->GetTexture();
	const RectF *pRect = pBottomCap->GetCurrentTextureCoordRect();
	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( TextureUnit_1, pTexture->GetTexHandle() );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetCullMode( CULL_NONE );
	DISPLAY->SetTextureWrapping(false);

	const float fFrameWidth		= pBottomCap->GetZoomedWidth();
	const float fFrameHeight	= pBottomCap->GetZoomedHeight();
	const float fYCapTop		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail;
	const float fYCapBottom		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail+fFrameHeight;

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(iCol) > 0.5f;

	if( bGlow )
		fColorScale = 1;

	float fDrawYCapTop;
	float fDrawYCapBottom;
	{
		float fYStartPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fYStartOffset, m_fYReverseOffsetPixels );
		float fYEndPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fYEndOffset, m_fYReverseOffsetPixels );
		fDrawYCapTop = max( fYCapTop, bReverse ? fYEndPos : fYStartPos );
		fDrawYCapBottom = min( fYCapBottom, bReverse ? fYStartPos : fYEndPos );
	}

	bool bAllAreTransparent = true;
	bool bLast = false;
	// don't draw any part of the tail that is before the middle of the head
	float fY=max( fDrawYCapTop, fYHead );
	for( ; !bLast; fY += fYStep )
	{
		if( fY >= fDrawYCapBottom )
		{
			fY = fDrawYCapBottom;
			bLast = true;
		}

		const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ			= ArrowEffects::GetZPos( m_pPlayerState, iCol, fYOffset );
		const float fX			= ArrowEffects::GetXPos( m_pPlayerState, iCol, fYOffset );
		const float fXLeft		= fX - fFrameWidth/2;
		const float fXRight		= fX + fFrameWidth/2;
		const float fTopDistFromTail	= fY - fYCapTop;
		const float fTexCoordTop	= SCALE( fTopDistFromTail,    0, fFrameHeight, pRect->top, pRect->bottom );
		const float fTexCoordLeft	= pRect->left;
		const float fTexCoordRight	= pRect->right;
		const float	fAlpha		= ArrowGetAlphaOrGlow( bGlow, m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor color		= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		queue.v[0].p = RageVector3(fXLeft,  fY, fZ);	queue.v[0].c = color; queue.v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		queue.v[1].p = RageVector3(fXRight, fY, fZ);	queue.v[1].c = color; queue.v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		queue.v+=2;
		if( queue.Free() < 2 )
		{
			/* The queue is full.  Render it, clear the buffer, and move back a step to
			 * start off the quad strip again. */
			if( !bAllAreTransparent )
				queue.Draw();
			queue.Init();
			bAllAreTransparent = true;
			fY -= fYStep;
		}
	}
	if( !bAllAreTransparent )
		queue.Draw();
}

void NoteDisplay::DrawHoldTail( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYTail, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset )
{
	//
	// Draw the tail
	//
	Actor* pSprTail = GetHoldActor( m_HoldTail, NotePart_HoldTail, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );

	pSprTail->SetZoom( ArrowEffects::GetZoom( m_pPlayerState ) );

	const float fY			= fYTail;
	const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, iCol, fY, m_fYReverseOffsetPixels );
	if( fYOffset < fYStartOffset || fYOffset > fYEndOffset )
			return;
	const float fX			= ArrowEffects::GetXPos( m_pPlayerState, iCol, fYOffset );
	const float fZ			= ArrowEffects::GetZPos( m_pPlayerState, iCol, fYOffset );
	const float fAlpha		= ArrowEffects::GetAlpha( m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float fGlow		= ArrowEffects::GetGlow( m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const RageColor colorDiffuse	= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	const RageColor colorGlow	= RageColor(1,1,1,fGlow);

	pSprTail->SetXY( fX, fY );
	pSprTail->SetZ( fZ );
	
	if( cache->m_fNoteColorTextureCoordSpacing[NotePart_HoldTail] != g_emptyVector )
	{
		DISPLAY->TexturePushMatrix();
		NoteType nt = GetNoteType( iRow );
		ENUM_CLAMP( nt, (NoteType)0, MAX_DISPLAY_NOTE_TYPE );
		DISPLAY->TextureTranslate( cache->m_fNoteColorTextureCoordSpacing[NotePart_HoldTail]*(float)nt );
	}

	if( bGlow )
	{
		pSprTail->SetDiffuse( RageColor(1,1,1,0) );
		pSprTail->SetGlow( colorGlow );
	}
	else
	{
		pSprTail->SetDiffuse( colorDiffuse );
		pSprTail->SetGlow( RageColor(0,0,0,0) );
	}

	if( cache->m_bHoldTailUseLighting )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(1,1,1,1), 
			RageColor(1,1,1,1),
			RageColor(1,1,1,1),
			RageVector3(1, 0, +1) );
	}

	pSprTail->Draw();

	if( cache->m_bHoldTailUseLighting )
	{
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}

	if( cache->m_fNoteColorTextureCoordSpacing[NotePart_HoldTail] != g_emptyVector )
	{
		DISPLAY->TexturePopMatrix();
	}
}

void NoteDisplay::DrawHoldHead( const TapNote& tn, int iCol, int iRow, bool bIsBeingHeld, float fYHead, float fPercentFadeToFail, float fColorScale, bool bGlow, float fYStartOffset, float fYEndOffset )
{
	//
	// Draw the head
	//
	Actor *pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );

	pActor->SetZoom( ArrowEffects::GetZoom( m_pPlayerState ) );

	// draw with normal Sprite
	const float fY				= fYHead;
	const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, iCol, fY, m_fYReverseOffsetPixels );
	if( fYOffset < fYStartOffset || fYOffset > fYEndOffset )
			return;
	const float fX				= ArrowEffects::GetXPos( m_pPlayerState, iCol, fYOffset );
	const float fZ				= ArrowEffects::GetZPos( m_pPlayerState, iCol, fYOffset );
	const float	fAlpha			= ArrowEffects::GetAlpha( m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float	fGlow			= ArrowEffects::GetGlow( m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	const RageColor colorGlow	= RageColor(1,1,1,fGlow);

	pActor->SetRotationZ( 0 );
	pActor->SetXY( fX, fY );
	pActor->SetZ( fZ );

	if( cache->m_fNoteColorTextureCoordSpacing[NotePart_HoldHead] != g_emptyVector )
	{
		DISPLAY->TexturePushMatrix();
		NoteType nt = GetNoteType( iRow );
		ENUM_CLAMP( nt, (NoteType)0, MAX_DISPLAY_NOTE_TYPE );
		DISPLAY->TextureTranslate( cache->m_fNoteColorTextureCoordSpacing[NotePart_HoldHead]*(float)nt );
	}

	if( bGlow )
	{
		pActor->SetDiffuse( RageColor(1,1,1,0) );
		pActor->SetGlow( colorGlow );
	}
	else
	{
		pActor->SetDiffuse( colorDiffuse );
		pActor->SetGlow( RageColor(0,0,0,0) );
	}

	if( cache->m_bHoldHeadUseLighting )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(1,1,1,1), 
			RageColor(1,1,1,1),
			RageColor(1,1,1,1),
			RageVector3(1, 0, +1) );
	}

	pActor->Draw();

	if( cache->m_bHoldHeadUseLighting )
	{
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}

	if( cache->m_fNoteColorTextureCoordSpacing[NotePart_HoldHead] != g_emptyVector )
	{
		DISPLAY->TexturePopMatrix();
	}
}

void NoteDisplay::DrawHold( const TapNote &tn, int iCol, int iRow, bool bIsBeingHeld, bool bIsActive, const HoldNoteResult &Result, float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels, float fYStartOffset, float fYEndOffset )
{
	int iEndRow = iRow + tn.iDuration;

	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(iCol) > 0.5f;
	float fStartBeat = NoteRowToBeat( max(tn.HoldResult.iLastHeldRow, iRow) );
	float fThrowAway = 0;

	// HACK: If active, don't set YOffset to 0 so that it doesn't jiggle around the receptor.
	bool bStartIsPastPeak = true;
	float fStartYOffset	= 0;
	if( bIsActive )
		;	// use the default values filled in above
	else
		fStartYOffset = ArrowEffects::GetYOffset( m_pPlayerState, iCol, fStartBeat, fThrowAway, bStartIsPastPeak );
	
	float fStartYPos	= ArrowEffects::GetYPos( m_pPlayerState, iCol, fStartYOffset, fReverseOffsetPixels );
	float fEndPeakYOffset	= 0;
	bool bEndIsPastPeak = false;
	float fEndYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, iCol, NoteRowToBeat(iEndRow), fEndPeakYOffset, bEndIsPastPeak );

	// In boomerang, the arrows reverse direction at Y offset value fPeakAtYOffset.  
	// If fPeakAtYOffset lies inside of the hold we're drawing, then the we 
	// want to draw the tail at that max Y offset, or else the hold will appear 
	// to magically grow as the tail approaches the max Y offset.
	if( bStartIsPastPeak && !bEndIsPastPeak )
		fEndYOffset	= fEndPeakYOffset;	// use the calculated PeakYOffset so that long holds don't appear to grow
	
	float fEndYPos		= ArrowEffects::GetYPos( m_pPlayerState, iCol, fEndYOffset, fReverseOffsetPixels );

	const float fYHead = bReverse ? fEndYPos : fStartYPos;		// the center of the head
	const float fYTail = bReverse ? fStartYPos : fEndYPos;		// the center the tail

//	const bool  bWavy = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fEffects[PlayerOptions::EFFECT_DRUNK] > 0;
	const bool WavyPartsNeedZBuffer = ArrowEffects::NeedZBuffer( m_pPlayerState );
	/* Hack: Z effects need a finer grain step. */
	const int	fYStep = WavyPartsNeedZBuffer? 4: 16; //bWavy ? 16 : 128;	// use small steps only if wavy

	const float fColorScale		= tn.HoldResult.fLife + (1-tn.HoldResult.fLife)*cache->m_fHoldLetGoGrayPercent;

	bool bFlipHeadAndTail = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	/* The body and caps should have no overlap, so their order doesn't matter.
	 * Draw the head last, so it appears on top. */
	if( !cache->m_bHoldHeadIsAboveWavyParts )
		DrawHoldHead( tn, iCol, iRow, bIsBeingHeld, bFlipHeadAndTail ? fYTail : fYHead, fPercentFadeToFail, fColorScale, bDrawGlowOnly, fYStartOffset, fYEndOffset );
	if( !cache->m_bHoldTailIsAboveWavyParts )
		DrawHoldTail( tn, iCol, iRow, bIsBeingHeld, bFlipHeadAndTail ? fYHead : fYTail, fPercentFadeToFail, fColorScale, bDrawGlowOnly, fYStartOffset, fYEndOffset );

	if( bDrawGlowOnly )
		DISPLAY->SetTextureModeGlow();
	else
		DISPLAY->SetTextureModeModulate();
	DISPLAY->SetZTestMode( WavyPartsNeedZBuffer?ZTEST_WRITE_ON_PASS:ZTEST_OFF );
	DISPLAY->SetZWrite( WavyPartsNeedZBuffer );
	
	if( !bFlipHeadAndTail )
		DrawHoldBottomCap( tn, iCol, iRow, bIsBeingHeld, fYHead, fYTail, fYStep, fPercentFadeToFail, fColorScale, bDrawGlowOnly, fYStartOffset, fYEndOffset );
	DrawHoldBody( tn, iCol, iRow, bIsBeingHeld, fYHead, fYTail, fYStep, fPercentFadeToFail, fColorScale, bDrawGlowOnly, fYStartOffset, fYEndOffset );
	if( bFlipHeadAndTail )
		DrawHoldTopCap( tn, iCol, iRow, bIsBeingHeld, fYHead, fYTail, fYStep, fPercentFadeToFail, fColorScale, bDrawGlowOnly, fYStartOffset, fYEndOffset );

	/* These set the texture mode themselves. */
	if( cache->m_bHoldTailIsAboveWavyParts )
		DrawHoldTail( tn, iCol, iRow, bIsBeingHeld, bFlipHeadAndTail ? fYHead : fYTail, fPercentFadeToFail, fColorScale, bDrawGlowOnly, fYStartOffset, fYEndOffset );
	if( cache->m_bHoldHeadIsAboveWavyParts )
		DrawHoldHead( tn, iCol, iRow, bIsBeingHeld, bFlipHeadAndTail ? fYTail : fYHead, fPercentFadeToFail, fColorScale, bDrawGlowOnly, fYStartOffset, fYEndOffset );

	// now, draw the glow pass
	if( !bDrawGlowOnly )
		DrawHold( tn, iCol, iRow, bIsBeingHeld, bIsActive, Result, fPercentFadeToFail, true, fReverseOffsetPixels, fYStartOffset, fYEndOffset );
}

void NoteDisplay::DrawActor( Actor* pActor, int iCol, float fBeat, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels, bool bUseLighting, NotePart part )
{
	const float fYOffset			= ArrowEffects::GetYOffset(	m_pPlayerState, iCol, fBeat );
	const float fYPos			= ArrowEffects::GetYPos(	m_pPlayerState, iCol, fYOffset, fReverseOffsetPixels );
	const float fRotation			= ArrowEffects::GetRotation(	m_pPlayerState, fBeat );
	const float fXPos			= ArrowEffects::GetXPos(	m_pPlayerState, iCol, fYOffset );
	const float fZPos			= ArrowEffects::GetZPos(	m_pPlayerState, iCol, fYOffset );
	const float fAlpha			= ArrowEffects::GetAlpha(	m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float fGlow			= ArrowEffects::GetGlow(	m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float fColorScale			= ArrowEffects::GetBrightness(	m_pPlayerState, fBeat ) * SCALE(fLife,0,1,0.2f,1);
	const float fZoom			= ArrowEffects::GetZoom( m_pPlayerState );
	RageColor diffuse = RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	RageColor glow = RageColor(1,1,1,fGlow);

	pActor->SetRotationZ( fRotation );
	pActor->SetXY( fXPos, fYPos );
	pActor->SetZ( fZPos );
	pActor->SetDiffuse( diffuse );
	pActor->SetGlow( glow );
	pActor->SetZoom( fZoom );

	if( cache->m_fNoteColorTextureCoordSpacing[part] != g_emptyVector )
	{
		DISPLAY->TexturePushMatrix();
		NoteType nt = BeatToNoteType( fBeat );
		ENUM_CLAMP( nt, (NoteType)0, MAX_DISPLAY_NOTE_TYPE );
		DISPLAY->TextureTranslate( cache->m_fNoteColorTextureCoordSpacing[part]*(float)nt );
	}

	if( bUseLighting )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(1,1,1,1), 
			RageColor(1,1,1,1),
			RageColor(1,1,1,1),
			RageVector3(1, 0, +1) );
	}

	pActor->Draw();

	if( bUseLighting )
	{
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}

	if( cache->m_fNoteColorTextureCoordSpacing[part] != g_emptyVector )
	{
		DISPLAY->TexturePopMatrix();
	}
}

void NoteDisplay::DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, bool bIsLift, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels )
{
	Actor* pActor = NULL;
	bool bUseLighting = false;
	NotePart part = NotePart_Tap;
	
	if( bIsLift )
	{
		pActor = GetTapActor( m_TapLift, NotePart_Lift, fBeat );
		bUseLighting = cache->m_bTapLiftUseLighting;
		part = NotePart_Lift;
	}
	else if( bIsMine )
	{
		pActor = GetTapActor( m_TapMine, NotePart_Mine, fBeat );
		bUseLighting = cache->m_bTapMineUseLighting;
		part = NotePart_Mine;
	}
	else if( bIsAddition )
	{
		pActor = GetTapActor( m_TapAddition, NotePart_Addition, fBeat );
		bUseLighting = cache->m_bTapAdditionUseLighting;
	}
	else if( bOnSameRowAsHoldStart  &&  cache->m_bDrawHoldHeadForTapsOnSameRow )
	{
		pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, fBeat, false, false );
		bUseLighting = cache->m_bHoldHeadUseLighting;
	}
	else	
	{
		pActor = GetTapActor( m_TapNote, NotePart_Tap, fBeat );
		bUseLighting = cache->m_bTapNoteUseLighting;
	}

	DrawActor( pActor, iCol, fBeat, fPercentFadeToFail, fLife, fReverseOffsetPixels, bUseLighting, part );
}

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
