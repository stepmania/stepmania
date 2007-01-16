#include "global.h"
#include "NoteDisplay.h"
#include "GameState.h"
#include "NoteSkinManager.h"
#include "ArrowEffects.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "ActorUtil.h"
#include "Style.h"
#include "PlayerState.h"
#include "Sprite.h"
#include "NoteTypes.h"
#include "LuaBinding.h"

const RString& NoteNotePartToString( NotePart i );
#define FOREACH_NotePart( i ) FOREACH_ENUM( NotePart, i )

static const char *NotePartNames[] = {
	"TapNote",
	"TapMine",
	"TapLift",
	"HoldHead",
	"HoldTopCap",
	"HoldBody",
	"HoldBottomCap",
	"HoldTail",
};
XToString( NotePart );

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
	float m_fAnimationLengthInBeats[NUM_NotePart];
	bool m_bAnimationIsVivid[NUM_NotePart];
	RageVector2 m_fAdditionTextureCoordOffset[NUM_NotePart];
	RageVector2 m_fNoteColorTextureCoordSpacing[NUM_NotePart];

	bool m_bHoldHeadIsAboveWavyParts;
	bool m_bHoldTailIsAboveWavyParts;
	int m_iStartDrawingHoldBodyOffsetFromHead;
	int m_iStopDrawingHoldBodyOffsetFromTail;
	float m_fHoldLetGoGrayPercent;
	bool m_bTapNoteUseLighting;
	bool m_bTapMineUseLighting;
	bool m_bTapLiftUseLighting;
	bool m_bHoldHeadUseLighting;
	bool m_bHoldTailUseLighting;
	bool m_bFlipHeadAndTailWhenReverse;
	bool m_bFlipHoldBodyWhenReverse;
	bool m_bTopHoldAnchorWhenReverse;
	bool m_bHoldActiveIsAddLayer;


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
		m_fAdditionTextureCoordOffset[p].x = NOTESKIN->GetMetricF(sButton,s+"AdditionTextureCoordOffsetX");
		m_fAdditionTextureCoordOffset[p].y = NOTESKIN->GetMetricF(sButton,s+"AdditionTextureCoordOffsetY");
		m_fNoteColorTextureCoordSpacing[p].x = NOTESKIN->GetMetricF(sButton,s+"NoteColorTextureCoordSpacingX");
		m_fNoteColorTextureCoordSpacing[p].y = NOTESKIN->GetMetricF(sButton,s+"NoteColorTextureCoordSpacingY");
	}
	m_bHoldHeadIsAboveWavyParts =		NOTESKIN->GetMetricB(sButton,"HoldHeadIsAboveWavyParts");
	m_bHoldTailIsAboveWavyParts =		NOTESKIN->GetMetricB(sButton,"HoldTailIsAboveWavyParts");
	m_iStartDrawingHoldBodyOffsetFromHead =	NOTESKIN->GetMetricI(sButton,"StartDrawingHoldBodyOffsetFromHead");
	m_iStopDrawingHoldBodyOffsetFromTail =	NOTESKIN->GetMetricI(sButton,"StopDrawingHoldBodyOffsetFromTail");
	m_fHoldLetGoGrayPercent =		NOTESKIN->GetMetricF(sButton,"HoldLetGoGrayPercent");
	m_bTapNoteUseLighting =			NOTESKIN->GetMetricB(sButton,"TapNoteUseLighting");
	m_bTapMineUseLighting =			NOTESKIN->GetMetricB(sButton,"TapMineUseLighting");
	m_bTapLiftUseLighting =			NOTESKIN->GetMetricB(sButton,"TapLiftUseLighting");
	m_bHoldHeadUseLighting =		NOTESKIN->GetMetricB(sButton,"HoldHeadUseLighting");
	m_bHoldTailUseLighting =		NOTESKIN->GetMetricB(sButton,"HoldTailUseLighting");
	m_bFlipHeadAndTailWhenReverse =		NOTESKIN->GetMetricB(sButton,"FlipHeadAndTailWhenReverse");
	m_bFlipHoldBodyWhenReverse =		NOTESKIN->GetMetricB(sButton,"FlipHoldBodyWhenReverse");
	m_bTopHoldAnchorWhenReverse =		NOTESKIN->GetMetricB(sButton,"TopHoldAnchorWhenReverse");
	m_bHoldActiveIsAddLayer =		NOTESKIN->GetMetricB(sButton,"HoldActiveIsAddLayer");
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
			Sprite *pSprite = dynamic_cast<Sprite *>( pRes->m_pActor );
			if( pSprite == NULL )
				RageException::Throw( "%s: must be a Sprite", nsap.sPath.c_str() );
		}

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

	ASSERT_M( pRes->m_iRefCount > 0, ssprintf("%i", pRes->m_iRefCount) );
	--pRes->m_iRefCount;
	if( pRes->m_iRefCount )
		return;

	g_NoteResource.erase( pRes->m_nsap );
	delete pRes;
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
	m_p = MakeNoteResource( sButton, sElement, false );
}


Actor *NoteColorActor::Get()
{
	return m_p->m_pActor;
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
	m_p = MakeNoteResource( sButton, sElement, true );
}

Sprite *NoteColorSprite::Get()
{
	return dynamic_cast<Sprite *>( m_p->m_pActor );
}

static const char *HoldTypeNames[] = {
	"hold",
	"roll",
};
XToString( HoldType );

static const char *ActiveTypeNames[] = {
	"active",
	"inactive",
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

	const RString &sButton = GAMESTATE->GetCurrentStyle()->ColToButtonName( iColNum );

	cache->Load( sButton );

	m_TapNote.Load(		sButton, "tap note" );
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

void NoteDisplay::DrawHoldPart( vector<Sprite*> &vpSpr, int iCol, int fYStep, float fPercentFadeToFail, float fColorScale, bool bGlow,
				float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar,
				float fOverlappedTime,
				float fYTop, float fYBottom,
				float fYStartPos, float fYEndPos,
				bool bWrapping, bool bAnchorToTop, bool bFlipTextureVertically )
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

		const float fYOffset		= ArrowEffects::GetYOffsetFromYPos( m_pPlayerState, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ			= ArrowEffects::GetZPos( m_pPlayerState, iCol, fYOffset );
		const float fFrameWidthScale	= ArrowEffects::GetFrameWidthScale( m_pPlayerState, fYOffset, fOverlappedTime );
		const float fScaledFrameWidth	= fFrameWidth * fFrameWidthScale;

		const float fX			= ArrowEffects::GetXPos( m_pPlayerState, iCol, fYOffset );
		const float fXLeft		= fX - fScaledFrameWidth/2;
		const float fXCenter		= fX;
		const float fXRight		= fX + fScaledFrameWidth/2;
		const float fDistFromTop	= fY - fYTop;
		float fTexCoordTop		= SCALE( fDistFromTop, 0, fFrameHeight, rect.top, rect.bottom );
		fTexCoordTop += fAddToTexCoord;

		const float fAlpha		= ArrowGetAlphaOrGlow( bGlow, m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
		const RageColor color		= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		queue.v[0].p = RageVector3(fXLeft,  fY, fZ);  queue.v[0].c = color; queue.v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		queue.v[1].p = RageVector3(fXCenter, fY, fZ); queue.v[1].c = color; queue.v[1].t = RageVector2(fTexCoordCenter, fTexCoordTop);
		queue.v[2].p = RageVector3(fXRight, fY, fZ);  queue.v[2].c = color; queue.v[2].t = RageVector2(fTexCoordRight, fTexCoordTop);
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
					DISPLAY->SetTextureWrapping( bWrapping );
					queue.Draw();
				}
			}
			queue.Init();
			bAllAreTransparent = true;
			fY -= fYStep;
		}
	}
}

void NoteDisplay::DrawHoldBody( const TapNote& tn, int iCol, float fBeat, bool bIsBeingHeld, float fYHead, float fYTail, bool bIsAddition, float fPercentFadeToFail, float fColorScale, bool bGlow,
			   float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar )
{
	vector<Sprite*> vpSprTop;
	Sprite *pSpriteTop = GetHoldSprite( m_HoldTopCap, NotePart_HoldTopCap, fBeat, tn.subType == TapNote::hold_head_roll, bIsBeingHeld && !cache->m_bHoldActiveIsAddLayer );
	vpSprTop.push_back( pSpriteTop );

	vector<Sprite*> vpSprBody;
	Sprite *pSpriteBody = GetHoldSprite( m_HoldBody, NotePart_HoldBody, fBeat, tn.subType == TapNote::hold_head_roll, bIsBeingHeld && !cache->m_bHoldActiveIsAddLayer );
	vpSprBody.push_back( pSpriteBody );

	vector<Sprite*> vpSprBottom;
	Sprite *pSpriteBottom = GetHoldSprite( m_HoldBottomCap, NotePart_HoldBottomCap, fBeat, tn.subType == TapNote::hold_head_roll, bIsBeingHeld && !cache->m_bHoldActiveIsAddLayer );
	vpSprBottom.push_back( pSpriteBottom );

	if( bIsBeingHeld && cache->m_bHoldActiveIsAddLayer )
	{
		Sprite *pSprTop = GetHoldSprite( m_HoldTopCap, NotePart_HoldTopCap, fBeat, tn.subType == TapNote::hold_head_roll, true );
		vpSprTop.push_back( pSprTop );
		Sprite *pSprBody = GetHoldSprite( m_HoldBody, NotePart_HoldBody, fBeat, tn.subType == TapNote::hold_head_roll, true );
		vpSprBody.push_back( pSprBody );
		Sprite *pSprBottom = GetHoldSprite( m_HoldBottomCap, NotePart_HoldBottomCap, fBeat, tn.subType == TapNote::hold_head_roll, true );
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
		DISPLAY->SetTextureModeGlow();
	else
		DISPLAY->SetTextureModeModulate();

	const bool bWavyPartsNeedZBuffer = ArrowEffects::NeedZBuffer( m_pPlayerState );
	DISPLAY->SetZTestMode( bWavyPartsNeedZBuffer?ZTEST_WRITE_ON_PASS:ZTEST_OFF );
	DISPLAY->SetZWrite( bWavyPartsNeedZBuffer );

	/* Hack: Z effects need a finer grain step. */
	const int fYStep = bWavyPartsNeedZBuffer? 4: 16;		// use small steps only if wavy

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

	const float fFrameHeightTop	= pSpriteTop->GetZoomedHeight();
	const float fFrameHeightBottom	= pSpriteBottom->GetZoomedHeight();

	float fYStartPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fDrawDistanceAfterTargetsPixels, m_fYReverseOffsetPixels );
	float fYEndPos = ArrowEffects::GetYPos( m_pPlayerState, iCol, fDrawDistanceBeforeTargetsPixels, m_fYReverseOffsetPixels );
	if( bReverse )
		swap( fYStartPos, fYEndPos );

	bool bTopAnchor = bReverse && cache->m_bTopHoldAnchorWhenReverse;

	// Draw the top cap
	DrawHoldPart(
		vpSprTop,
		iCol, fYStep, fPercentFadeToFail, fColorScale, bGlow,
		fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar,
		tn.HoldResult.fOverlappedTime,
		fYHead-fFrameHeightTop, fYHead,
		fYStartPos, min(fYEndPos, fYTail),
		false, bTopAnchor, bFlipHoldBody );

	// Draw the body
	DrawHoldPart(
		vpSprBody,
		iCol, fYStep, fPercentFadeToFail, fColorScale, bGlow,
		fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar,
		tn.HoldResult.fOverlappedTime,
		fYHead, fYTail,
		fYStartPos, fYEndPos,
		true, bTopAnchor, bFlipHoldBody );

	// Draw the bottom cap
	DrawHoldPart(
		vpSprBottom,
		iCol, fYStep, fPercentFadeToFail, fColorScale, bGlow,
		fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar,
		tn.HoldResult.fOverlappedTime,
		fYTail, fYTail+fFrameHeightBottom,
		max(fYStartPos, fYHead), fYEndPos,
		false, bTopAnchor, bFlipHoldBody );
}

void NoteDisplay::DrawHold( const TapNote &tn, int iCol, int iRow, bool bIsBeingHeld, const HoldNoteResult &Result, bool bIsAddition, float fPercentFadeToFail, 
			   float fReverseOffsetPixels, float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fDrawDistanceBeforeTargetsPixels2, float fFadeInPercentOfDrawFar )
{
	int iEndRow = iRow + tn.iDuration;

	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	bool bReverse = m_pPlayerState->m_PlayerOptions.GetCurrent().GetReversePercentForColumn(iCol) > 0.5f;
	float fStartBeat = NoteRowToBeat( max(tn.HoldResult.iLastHeldRow, iRow) );
	float fThrowAway = 0;

	// HACK: If active, don't set YOffset to 0 so that it doesn't jiggle around the receptor.
	bool bStartIsPastPeak = true;
	float fStartYOffset	= 0;
	if( tn.HoldResult.bActive )
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

	const float fYHead		= ArrowEffects::GetYPos( m_pPlayerState, iCol, fStartYOffset, fReverseOffsetPixels );
	const float fYTail		= ArrowEffects::GetYPos( m_pPlayerState, iCol, fEndYOffset, fReverseOffsetPixels );

	const float fColorScale		= SCALE( tn.HoldResult.fLife, 0.0f, 1.0f, cache->m_fHoldLetGoGrayPercent, 1.0f );

	bool bFlipHeadAndTail = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	/* The body and caps should have no overlap, so their order doesn't matter.
	 * Draw the head last, so it appears on top. */
	float fBeat = NoteRowToBeat(iRow);
	if( !cache->m_bHoldHeadIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );
		DrawActor( tn, pActor, NotePart_HoldHead, iCol, bFlipHeadAndTail ? fEndYOffset : fStartYOffset, fBeat, bIsAddition, fPercentFadeToFail, fReverseOffsetPixels, fColorScale, cache->m_bHoldHeadUseLighting, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	}
	if( !cache->m_bHoldTailIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldTail, NotePart_HoldTail, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );
		DrawActor( tn, pActor, NotePart_HoldTail, iCol, bFlipHeadAndTail ? fStartYOffset : fEndYOffset, fBeat, bIsAddition, fPercentFadeToFail, fReverseOffsetPixels, fColorScale, cache->m_bHoldTailUseLighting, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	}

	DrawHoldBody( tn, iCol, fBeat, bIsBeingHeld, fYHead, fYTail, bIsAddition, fPercentFadeToFail, fColorScale, false, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	DrawHoldBody( tn, iCol, fBeat, bIsBeingHeld, fYHead, fYTail, bIsAddition, fPercentFadeToFail, fColorScale, true, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );

	/* These set the texture mode themselves. */
	if( cache->m_bHoldTailIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldTail, NotePart_HoldTail, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );
		DrawActor( tn, pActor, NotePart_HoldTail, iCol, bFlipHeadAndTail ? fStartYOffset : fEndYOffset, fBeat, bIsAddition, fPercentFadeToFail, fReverseOffsetPixels, fColorScale, cache->m_bHoldTailUseLighting, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	}
	if( cache->m_bHoldHeadIsAboveWavyParts )
	{
		Actor *pActor = GetHoldActor( m_HoldHead, NotePart_HoldHead, NoteRowToBeat(iRow), tn.subType == TapNote::hold_head_roll, bIsBeingHeld );
		DrawActor( tn, pActor, NotePart_HoldHead, iCol, bFlipHeadAndTail ? fEndYOffset : fStartYOffset, fBeat, bIsAddition, fPercentFadeToFail, fReverseOffsetPixels, fColorScale, cache->m_bHoldHeadUseLighting, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	}
}

void NoteDisplay::DrawActor( const TapNote& tn, Actor* pActor, NotePart part, int iCol, float fYOffset, float fBeat, bool bIsAddition, float fPercentFadeToFail, float fReverseOffsetPixels, float fColorScale, bool bUseLighting, float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar )
{
	if( fYOffset < fDrawDistanceAfterTargetsPixels || fYOffset > fDrawDistanceBeforeTargetsPixels )
		return;
	const float fY			= ArrowEffects::GetYPos(	m_pPlayerState, iCol, fYOffset, fReverseOffsetPixels );
	const float fX			= ArrowEffects::GetXPos(	m_pPlayerState, iCol, fYOffset );
	const float fZ			= ArrowEffects::GetZPos(	m_pPlayerState, iCol, fYOffset );
	const float fAlpha		= ArrowEffects::GetAlpha(	m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	const float fGlow		= ArrowEffects::GetGlow(	m_pPlayerState, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );
	const RageColor diffuse		= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	const RageColor glow		= RageColor(1,1,1,fGlow);
	float fRotation			= 0;
	if( tn.type != TapNote::hold_head )
	{
		fRotation		= ArrowEffects::GetRotation(	m_pPlayerState, fBeat );
		fColorScale		*= ArrowEffects::GetBrightness(	m_pPlayerState, fBeat );
	}

	pActor->SetRotationZ( fRotation );
	pActor->SetXY( fX, fY );
	pActor->SetZ( fZ );
	pActor->SetDiffuse( diffuse );
	pActor->SetGlow( glow );
	pActor->SetZoom( ArrowEffects::GetZoom(m_pPlayerState) );

	bool bNeedsTranslate = (bIsAddition && !IsVectorZero(cache->m_fAdditionTextureCoordOffset[part])) || !IsVectorZero(cache->m_fNoteColorTextureCoordSpacing[part]);
	if( bNeedsTranslate )
	{
		DISPLAY->TexturePushMatrix();
		NoteType nt = BeatToNoteType( fBeat );
		ENUM_CLAMP( nt, (NoteType)0, MAX_DISPLAY_NOTE_TYPE );
		DISPLAY->TextureTranslate( (bIsAddition ? cache->m_fAdditionTextureCoordOffset[part] : RageVector2(0,0)) + cache->m_fNoteColorTextureCoordSpacing[part]*(float)nt );
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

	if( bNeedsTranslate )
	{
		DISPLAY->TexturePopMatrix();
	}
}

void NoteDisplay::DrawTap( const TapNote& tn, int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, float fPercentFadeToFail, float fReverseOffsetPixels, float fDrawDistanceAfterTargetsPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar )
{
	Actor* pActor = NULL;
	bool bUseLighting = false;
	NotePart part = NotePart_Tap;
	
	if( tn.type == TapNote::lift )
	{
		pActor = GetTapActor( m_TapLift, NotePart_Lift, fBeat );
		bUseLighting = cache->m_bTapLiftUseLighting;
		part = NotePart_Lift;
	}
	else if( tn.type == TapNote::mine )
	{
		pActor = GetTapActor( m_TapMine, NotePart_Mine, fBeat );
		bUseLighting = cache->m_bTapMineUseLighting;
		part = NotePart_Mine;
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

	if( tn.type == TapNote::attack )
	{
		Lua *L = LUA->Get();
		LuaTable tab;
		LuaHelpers::Push( L, tn.sAttackModifiers );
		tab.Set( L, "Modifiers" );
		pActor->PlayCommand( "SetAttack", &tab );
		LUA->Release( L );
	}

	const float fYOffset = ArrowEffects::GetYOffset( m_pPlayerState, iCol, fBeat );
	DrawActor( tn, pActor, part, iCol, fYOffset, fBeat, bIsAddition, fPercentFadeToFail, fReverseOffsetPixels, 1.0f, bUseLighting, fDrawDistanceAfterTargetsPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar );

	if( tn.type == TapNote::attack )
		pActor->PlayCommand( "UnsetAttack" );
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
