#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: NoteDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Brian Bugh
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "NoteDisplay.h"
#include "Notes.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "NoteSkinManager.h"
#include "RageException.h"
#include "ArrowEffects.h"
#include "RageLog.h"
#include <math.h>
#include "RageDisplay.h"
#include "NoteTypes.h"


#define DRAW_HOLD_HEAD_FOR_TAPS_ON_SAME_ROW			NOTESKIN->GetMetricB(name,"DrawHoldHeadForTapsOnSameRow")
#define TAP_NOTE_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(name,"TapNoteAnimationLengthInBeats")
#define HOLD_HEAD_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(name,"HoldHeadAnimationLengthInBeats")
#define HOLD_TOPCAP_ANIMATION_LENGTH_IN_BEATS		NOTESKIN->GetMetricF(name,"HoldTopCapAnimationLengthInBeats")
#define HOLD_BODY_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(name,"HoldBodyAnimationLengthInBeats")
#define HOLD_BOTTOMCAP_ANIMATION_LENGTH_IN_BEATS	NOTESKIN->GetMetricF(name,"HoldBottomCapAnimationLengthInBeats")
#define HOLD_TAIL_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(name,"HoldTailAnimationLengthInBeats")
#define TAP_NOTE_ANIMATION_IS_VIVID					NOTESKIN->GetMetricB(name,"TapNoteAnimationIsVivid")
#define HOLD_HEAD_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(name,"HoldHeadAnimationIsVivid")
#define HOLD_TOPCAP_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(name,"HoldTopCapAnimationIsVivid")
#define HOLD_BODY_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(name,"HoldBodyAnimationIsVivid")
#define HOLD_BOTTOMCAP_ANIMATION_IS_VIVID			NOTESKIN->GetMetricB(name,"HoldBottomCapAnimationIsVivid")
#define HOLD_TAIL_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(name,"HoldTailAnimationIsVivid")
#define TAP_NOTE_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(name,"TapNoteAnimationIsNoteColor")
#define HOLD_HEAD_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(name,"HoldHeadAnimationIsNoteColor")
#define HOLD_TOPCAP_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(name,"HoldTopCapAnimationIsNoteColor")
#define HOLD_BODY_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(name,"HoldBodyAnimationIsNoteColor")
#define HOLD_BOTTOMCAP_ANIMATION_IS_NOTE_COLOR		NOTESKIN->GetMetricB(name,"HoldBottomCapAnimationIsNoteColor")
#define HOLD_TAIL_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(name,"HoldTailAnimationIsNoteColor")
#define START_DRAWING_HOLD_BODY_OFFSET_FROM_HEAD	NOTESKIN->GetMetricI(name,"StartDrawingHoldBodyOffsetFromHead")
#define STOP_DRAWING_HOLD_BODY_OFFSET_FROM_TAIL		NOTESKIN->GetMetricI(name,"StopDrawingHoldBodyOffsetFromTail")
#define HOLD_NG_GRAY_PERCENT						NOTESKIN->GetMetricF(name,"HoldNGGrayPercent")

// cache
struct NoteMetricCache_t {
	bool m_bDrawHoldHeadForTapsOnSameRow;
	float m_fTapNoteAnimationLengthInBeats;
	float m_fHoldHeadAnimationLengthInBeats;
	float m_fHoldTopCapAnimationLengthInBeats;
	float m_fHoldBodyAnimationLengthInBeats;
	float m_fHoldBottomCapAnimationLengthInBeats;
	float m_fHoldTailAnimationLengthInBeats;
	bool m_bTapNoteAnimationIsVivid;
	bool m_bHoldHeadAnimationIsVivid;
	bool m_bHoldTopCapAnimationIsVivid;
	bool m_bHoldBodyAnimationIsVivid;
	bool m_bHoldBottomCapAnimationIsVivid;
	bool m_bHoldTailAnimationIsVivid;
	bool m_bTapNoteAnimationIsNoteColor;
	bool m_bHoldHeadAnimationIsNoteColor;
	bool m_bHoldTopCapAnimationIsNoteColor;
	bool m_bHoldBodyAnimationIsNoteColor;
	bool m_bHoldBottomCapAnimationIsNoteColor;
	bool m_bHoldTailAnimationIsNoteColor;
	int m_iStartDrawingHoldBodyOffsetFromHead;
	int m_iStopDrawingHoldBodyOffsetFromTail;
	float m_fHoldNGGrayPercent;

	void Load(const CString &name);
} *NoteMetricCache;

void NoteMetricCache_t::Load(const CString &name)
{
	m_bDrawHoldHeadForTapsOnSameRow = DRAW_HOLD_HEAD_FOR_TAPS_ON_SAME_ROW;
	m_fTapNoteAnimationLengthInBeats = TAP_NOTE_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldHeadAnimationLengthInBeats = HOLD_HEAD_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldTopCapAnimationLengthInBeats = HOLD_TOPCAP_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldBodyAnimationLengthInBeats = HOLD_BODY_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldBottomCapAnimationLengthInBeats = HOLD_BOTTOMCAP_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldTailAnimationLengthInBeats = HOLD_TAIL_ANIMATION_LENGTH_IN_BEATS;
	m_bTapNoteAnimationIsVivid = TAP_NOTE_ANIMATION_IS_VIVID;
	m_bHoldHeadAnimationIsVivid = HOLD_HEAD_ANIMATION_IS_VIVID;
	m_bHoldTopCapAnimationIsVivid = HOLD_TOPCAP_ANIMATION_IS_VIVID;
	m_bHoldBodyAnimationIsVivid = HOLD_BODY_ANIMATION_IS_VIVID;
	m_bHoldBottomCapAnimationIsVivid = HOLD_BOTTOMCAP_ANIMATION_IS_VIVID;
	m_bHoldTailAnimationIsVivid = HOLD_TAIL_ANIMATION_IS_VIVID;
	m_bTapNoteAnimationIsNoteColor = TAP_NOTE_ANIMATION_IS_NOTE_COLOR;
	m_bHoldHeadAnimationIsNoteColor = HOLD_HEAD_ANIMATION_IS_NOTE_COLOR;
	m_bHoldTopCapAnimationIsNoteColor = HOLD_TOPCAP_ANIMATION_IS_NOTE_COLOR;
	m_bHoldBodyAnimationIsNoteColor = HOLD_BODY_ANIMATION_IS_NOTE_COLOR;
	m_bHoldBottomCapAnimationIsNoteColor = HOLD_BOTTOMCAP_ANIMATION_IS_NOTE_COLOR;
	m_bHoldTailAnimationIsNoteColor = HOLD_TAIL_ANIMATION_IS_NOTE_COLOR;
	m_iStartDrawingHoldBodyOffsetFromHead = START_DRAWING_HOLD_BODY_OFFSET_FROM_HEAD;
	m_iStopDrawingHoldBodyOffsetFromTail = STOP_DRAWING_HOLD_BODY_OFFSET_FROM_TAIL;
	m_fHoldNGGrayPercent = HOLD_NG_GRAY_PERCENT;
}

NoteDisplay::NoteDisplay()
{
	cache = new NoteMetricCache_t;
}

NoteDisplay::~NoteDisplay()
{
	delete cache;
}

void NoteDisplay::Load( int iColNum, PlayerNumber pn )
{
	m_PlayerNumber = pn;

	cache->Load(NoteSkinManager::ColToButtonName(iColNum));

	// Look up note names once and store them here.
	CString sNoteType[ NOTE_COLOR_IMAGES ];
	for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		sNoteType[i] = NoteTypeToString( (NoteType)i );


	if( cache->m_bTapNoteAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
			m_sprTapNote[i].Load( NOTESKIN->GetPathTo(iColNum, "tap note "+sNoteType[i]) );
	}
	else
	{
		m_sprTapNote[0].Load( NOTESKIN->GetPathTo(iColNum, "tap note") );
	}

	if( cache->m_bHoldHeadAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldHeadActive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold head active "+sNoteType[i]) );
			m_sprHoldHeadInactive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold head inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldHeadActive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold head active") );
		m_sprHoldHeadInactive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold head inactive") );
	}

	if( cache->m_bHoldTopCapAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldTopCapActive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold topcap active "+sNoteType[i]) );
			m_sprHoldTopCapInactive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold topcap inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldTopCapActive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold topcap active") );
		m_sprHoldTopCapInactive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold topcap inactive") );
	}

	if( cache->m_bHoldBodyAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldBodyActive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold body active "+sNoteType[i]) );
			m_sprHoldBodyInactive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold body inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldBodyActive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold body active") );
		m_sprHoldBodyInactive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold body inactive") );
	}

	if( cache->m_bHoldBottomCapAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldBottomCapActive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold bottomcap active "+sNoteType[i]) );
			m_sprHoldBottomCapInactive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold bottomcap inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldBottomCapActive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold bottomcap active") );
		m_sprHoldBottomCapInactive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold bottomcap inactive") );
	}

	if( cache->m_bHoldTailAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldTailActive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold tail active "+sNoteType[i]) );
			m_sprHoldTailInactive[i].Load( NOTESKIN->GetPathTo(iColNum, "hold tail inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldTailActive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold tail active") );
		m_sprHoldTailInactive[0].Load( NOTESKIN->GetPathTo(iColNum, "hold tail inactive") );
	}
}


void NoteDisplay::SetActiveFrame( float fNoteBeat, Sprite &Spr, float fAnimationLengthInBeats, bool bVivid, bool bNoteColor )
{
	const int iNumFrames = Spr.GetNumStates();
	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fPrecentIntoAnimation = fmodf(fSongBeat,fAnimationLengthInBeats) / fAnimationLengthInBeats;
	float fNoteBeatFraction = fmodf( fNoteBeat, 1.0f );

	int iFrameNo = (int)(fPrecentIntoAnimation*iNumFrames);
	if( bVivid )
		iFrameNo += (int)( froundf(fNoteBeatFraction,1.f/fAnimationLengthInBeats)*iNumFrames );

	iFrameNo += iNumFrames;
	iFrameNo %= iNumFrames;

	ASSERT( iFrameNo>=0 && iFrameNo<iNumFrames );

	Spr.SetState( iFrameNo );
}

Sprite * NoteDisplay::GetTapNoteSprite( float fNoteBeat )
{
	NoteType nt = NoteType(0);
	if( cache->m_bTapNoteAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_32ND;
	Sprite *pSpriteOut = &m_sprTapNote[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut,
		cache->m_fTapNoteAnimationLengthInBeats, 
		cache->m_bTapNoteAnimationIsVivid, 
		cache->m_bTapNoteAnimationIsNoteColor );

	return pSpriteOut;
}

Sprite * NoteDisplay::GetHoldTopCapSprite( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldTopCapAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_32ND;
	
	Sprite *pSpriteOut = bActive ? &m_sprHoldTopCapActive[nt] : &m_sprHoldTopCapInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fHoldTopCapAnimationLengthInBeats, 
		cache->m_bHoldTopCapAnimationIsVivid, 
		cache->m_bHoldTopCapAnimationIsNoteColor );

	return pSpriteOut;
}

Sprite * NoteDisplay::GetHoldBottomCapSprite( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldBottomCapAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_32ND;
	
	Sprite *pSpriteOut = bActive ? &m_sprHoldBottomCapActive[nt] : &m_sprHoldBottomCapInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fHoldBottomCapAnimationLengthInBeats, 
		cache->m_bHoldBottomCapAnimationIsVivid, 
		cache->m_bHoldBottomCapAnimationIsNoteColor );

	return pSpriteOut;
}


Sprite * NoteDisplay::GetHoldHeadSprite( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldHeadAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_32ND;

	Sprite *pSpriteOut = bActive ? &m_sprHoldHeadActive[nt] : &m_sprHoldHeadInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fHoldHeadAnimationLengthInBeats, 
		cache->m_bHoldHeadAnimationIsVivid, 
		cache->m_bHoldHeadAnimationIsNoteColor );

	return pSpriteOut;
}

Sprite *NoteDisplay::GetHoldBodySprite( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldBodyAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_32ND;

	Sprite *pSpriteOut = bActive ? &m_sprHoldBodyActive[nt] : &m_sprHoldBodyInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fHoldBodyAnimationLengthInBeats, 
		cache->m_bHoldBodyAnimationIsVivid, 
		cache->m_bHoldBodyAnimationIsNoteColor );

	return pSpriteOut;
}

Sprite * NoteDisplay::GetHoldTailSprite( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldTailAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_32ND;

	Sprite *pSpriteOut = bActive ? &m_sprHoldTailActive[nt] : &m_sprHoldTailInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fHoldTailAnimationLengthInBeats, 
		cache->m_bHoldTailAnimationIsVivid, 
		cache->m_bHoldTailAnimationIsNoteColor );

	return pSpriteOut;
}


void NoteDisplay::DrawHold( const HoldNote& hn, const bool bActive, const float fLife, const float fPercentFadeToFail, bool bDrawGlowOnly )
{
	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	const bool bReverse = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_bReverseScroll;

	const int	iCol			= hn.m_iTrack;
	const float fStartYOffset	= ArrowGetYOffset( m_PlayerNumber, hn.m_fStartBeat );
	const float fStartYPos		= ArrowGetYPos(	   m_PlayerNumber, fStartYOffset );
	const float fEndYOffset		= ArrowGetYOffset( m_PlayerNumber, hn.m_fEndBeat );
	const float fEndYPos		= ArrowGetYPos(	   m_PlayerNumber, fEndYOffset );

	const float fYHead = bReverse ? fEndYPos : fStartYPos;		// the center of the head
	const float fYTail = bReverse ? fStartYPos : fEndYPos;		// the center the tail

	const int	fYStep = 16;	// 16 logical pixels per segment

	const float fColorScale		= 1*fLife + (1-fLife)*cache->m_fHoldNGGrayPercent;


	//
	// draw from tail to head (so head appears on top)
	//

	/* The body and caps should have no overlap, so their order doesn't matter.
	 * Draw the head last, so it appears on top. */
	static RageVertex queue[4096];

	//
	// Draw the bottom cap (always wavy)
	//
	{
		Sprite* pBottomCap = GetHoldBottomCapSprite( hn.m_fStartBeat, bActive );

		// draw manually in small segments
		RageVertex *v = &queue[0];
		RageTexture* pTexture = pBottomCap->GetTexture();
		const RectF *pRect = pBottomCap->GetCurrentTextureCoordRect();
		DISPLAY->SetTexture( pTexture );
		DISPLAY->SetBlendModeNormal();
		if( bDrawGlowOnly )
			DISPLAY->SetTextureModeGlow();
		else
			DISPLAY->SetTextureModeModulate();
		DISPLAY->DisableTextureWrapping();

		const float fFrameWidth		= pBottomCap->GetUnzoomedWidth();
		const float fFrameHeight	= pBottomCap->GetUnzoomedHeight();
		const float fYCapTop		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail;
		const float fYCapBottom		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail+fFrameHeight;

		// don't draw any part of the tail that is before the middle of the head
		float fY=max( fYCapTop, fYHead );
		for( ; fY<fYCapBottom; fY+=fYStep )	
		{
			const float fYTop					= fY;
			const float fStepHeight				= min( fYStep, fFrameHeight );
			const float fYBottom				= min( fY+fStepHeight, fYCapBottom );
			const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
			const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
			const float fXTopLeft				= fXTop - fFrameWidth/2;
			const float fXTopRight				= fXTop + fFrameWidth/2;
			const float fXBottomLeft			= fXBottom - fFrameWidth/2;
			const float fXBottomRight			= fXBottom + fFrameWidth/2;
			const float fTopDistFromTailTop		= fYTop - fYCapTop;
			const float fBottomDistFromTailTop	= fYBottom - fYCapTop;
			const float fTexCoordTop			= SCALE( fTopDistFromTailTop,    0, fFrameHeight, pRect->top, pRect->bottom );
			const float fTexCoordBottom			= SCALE( fBottomDistFromTailTop, 0, fFrameHeight, pRect->top, pRect->bottom );
			const float fTexCoordLeft			= pRect->left;
			const float fTexCoordRight			= pRect->right;
			const float	fAlphaTop				= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fAlphaBottom			= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const float	fGlowTop				= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fGlowBottom				= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
			const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
			const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
			const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

			if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
				continue;
			if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
				continue;

			v[0].p = RageVector3(fXTopLeft,    fYTop,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
			v[1].p = RageVector3(fXTopRight,   fYTop,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;    	v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
			v[2].p = RageVector3(fXBottomRight,fYBottom,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
			v[3].p = RageVector3(fXBottomLeft, fYBottom,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
			v+=4;
		}
		DISPLAY->DrawQuads( queue, v-queue );
	}

	//
	// Draw the body (always wavy)
	//
	{
		Sprite* pSprBody = GetHoldBodySprite( hn.m_fStartBeat, bActive );

		// draw manually in small segments
		RageVertex *v = &queue[0];
		RageTexture* pTexture = pSprBody->GetTexture();
		const RectF *pRect = pSprBody->GetCurrentTextureCoordRect();
		DISPLAY->SetTexture( pTexture );
		DISPLAY->SetBlendModeNormal();
		if( bDrawGlowOnly )
			DISPLAY->SetTextureModeGlow();
		else
			DISPLAY->SetTextureModeModulate();
		DISPLAY->EnableTextureWrapping();


		const float fFrameWidth  = pSprBody->GetUnzoomedWidth();
		const float fFrameHeight = pSprBody->GetUnzoomedHeight();
		const float fYBodyTop = fYHead + cache->m_iStartDrawingHoldBodyOffsetFromHead;
		const float fYBodyBottom = fYTail + cache->m_iStopDrawingHoldBodyOffsetFromTail;
		
		// top to bottom
		for( float fY=fYBodyTop; fY<=fYBodyBottom; fY+=fYStep )
		{
			const float fYTop					= fY;
			const float fYBottom				= min( fY+fYStep, fYBodyBottom );
			const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
			const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
			const float fXTopLeft				= fXTop - fFrameWidth/2;
			const float fXTopRight				= fXTop + fFrameWidth/2;
			const float fXBottomLeft			= fXBottom - fFrameWidth/2;
			const float fXBottomRight			= fXBottom + fFrameWidth/2;
			const float fTopDistFromBodyBottom		= fYBodyBottom - fYTop;
			const float fBottomDistFromBodyBottom	= fYBodyBottom - fYBottom;
				  float fTexCoordTop			= SCALE( fTopDistFromBodyBottom,    0, fFrameHeight, pRect->bottom, pRect->top );
				  float fTexCoordBottom			= SCALE( fBottomDistFromBodyBottom, 0, fFrameHeight, pRect->bottom, pRect->top );
			if( fTexCoordTop < 0 )
			{
				int iToSubtract = (int)fTexCoordTop - 1;
				fTexCoordTop -= iToSubtract;
				fTexCoordBottom -= iToSubtract;
			}
			else if( fTexCoordBottom > 2 )
			{
				int iToSubtract = (int)fTexCoordBottom;
				fTexCoordTop -= iToSubtract;
				fTexCoordBottom -= iToSubtract;
			}
			ASSERT( fTexCoordTop>=0 && fTexCoordBottom>=0 );
			ASSERT( fTexCoordTop<=2 && fTexCoordBottom<=2 );
			const float fTexCoordLeft			= pRect->left;
			const float fTexCoordRight			= pRect->right;
			const float	fAlphaTop				= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fAlphaBottom			= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const float	fGlowTop				= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fGlowBottom				= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
			const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
			const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
			const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

			if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
				continue;
			if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
				continue;

			v[0].p = RageVector3(fXTopLeft,    fYTop,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
			v[1].p = RageVector3(fXTopRight,   fYTop,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
			v[2].p = RageVector3(fXBottomRight,fYBottom,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
			v[3].p = RageVector3(fXBottomLeft, fYBottom,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom;	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
			v+=4;
		}	
		DISPLAY->DrawQuads( queue, v-queue );
	}

	//
	// Draw the top cap (always wavy)
	//
	{
		Sprite* pSprTopCap = GetHoldTopCapSprite( hn.m_fStartBeat, bActive );

		// draw manually in small segments
		RageVertex *v = &queue[0];
		RageTexture* pTexture = pSprTopCap->GetTexture();
		const RectF *pRect = pSprTopCap->GetCurrentTextureCoordRect();
		DISPLAY->SetTexture( pTexture );
		DISPLAY->SetBlendModeNormal();
		if( bDrawGlowOnly )
			DISPLAY->SetTextureModeGlow();
		else
			DISPLAY->SetTextureModeModulate();
		DISPLAY->DisableTextureWrapping();

		const float fFrameWidth  = pSprTopCap->GetUnzoomedWidth();
		const float fFrameHeight = pSprTopCap->GetUnzoomedHeight();
		const float fYCapTop	 = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead-fFrameHeight;
		const float fYCapBottom  = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead;

		for( float fY=fYCapTop; fY<fYCapBottom; fY+=fYStep )
		{
			const float fYTop					= fY;
			const float fStepHeight				= min( fYStep, fFrameHeight );
			const float fYBottom				= min( fY+fStepHeight, fYCapBottom );
			const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
			const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
			const float fXTopLeft				= fXTop - fFrameWidth/2;
			const float fXTopRight				= fXTop + fFrameWidth/2;
			const float fXBottomLeft			= fXBottom - fFrameWidth/2;
			const float fXBottomRight			= fXBottom + fFrameWidth/2;
			const float fTopDistFromHeadTop		= fYTop - fYCapTop;
			const float fBottomDistFromHeadTop	= fYBottom - fYCapTop;
			const float fTexCoordTop			= SCALE( fTopDistFromHeadTop,    0, fFrameHeight, pRect->top, pRect->bottom );
			const float fTexCoordBottom			= SCALE( fBottomDistFromHeadTop, 0, fFrameHeight, pRect->top, pRect->bottom );
			ASSERT( fTexCoordTop>=-0.0001 && fTexCoordBottom<=1.0001 ); /* allow for rounding error */
			const float fTexCoordLeft			= pRect->left;
			const float fTexCoordRight			= pRect->right;
			const float	fAlphaTop				= ArrowGetAlpha( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fAlphaBottom			= ArrowGetAlpha( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const float	fGlowTop				= ArrowGetGlow( m_PlayerNumber, fYTop, fPercentFadeToFail );
			const float	fGlowBottom				= ArrowGetGlow( m_PlayerNumber, fYBottom, fPercentFadeToFail );
			const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
			const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
			const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
			const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

			if( bDrawGlowOnly && colorGlowTop.a==0 && colorGlowBottom.a==0 )
				continue;
			if( !bDrawGlowOnly && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
				continue;

			v[0].p = RageVector3(fXTopLeft,    fYTop,   0);	v[0].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
			v[1].p = RageVector3(fXTopRight,   fYTop,   0);	v[1].c = bDrawGlowOnly ? colorGlowTop    : colorDiffuseTop;    	v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
			v[2].p = RageVector3(fXBottomRight,fYBottom,0);	v[2].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
			v[3].p = RageVector3(fXBottomLeft, fYBottom,0);	v[3].c = bDrawGlowOnly ? colorGlowBottom : colorDiffuseBottom; 	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
			v+=4;
		}
		DISPLAY->DrawQuads( queue, v-queue );
	}

	//
	// Draw the tail
	//
	{
		Sprite* pSprTail = GetHoldTailSprite( hn.m_fStartBeat, bActive );

		const float fY				= fYTail;
		const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fY );
		const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, fY, fPercentFadeToFail );
		const float	fGlow			= ArrowGetGlow( m_PlayerNumber, fY, fPercentFadeToFail );
		const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
		const RageColor colorGlow	= RageColor(1,1,1,fGlow);

		pSprTail->SetXY( fX, fY );
		if( bDrawGlowOnly )
		{
			pSprTail->SetDiffuse( RageColor(1,1,1,0) );
			pSprTail->SetGlow( colorGlow );
		}
		else
		{
			pSprTail->SetDiffuse( colorDiffuse );
			pSprTail->SetGlow( RageColor(0,0,0,0) );
		}
		pSprTail->Draw();
	}

	//
	// Draw the head
	//
	{
		Sprite* pSprHead = GetHoldHeadSprite( hn.m_fStartBeat, bActive );

		// draw with normal Sprite
		const float fY				= fYHead;
		const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fY );
		const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, fY, fPercentFadeToFail );
		const float	fGlow			= ArrowGetGlow( m_PlayerNumber, fY, fPercentFadeToFail );
		const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
		const RageColor colorGlow	= RageColor(1,1,1,fGlow);

		pSprHead->SetXY( fX, fY );
		if( bDrawGlowOnly )
		{
			pSprHead->SetDiffuse( RageColor(1,1,1,0) );
			pSprHead->SetGlow( colorGlow );
		}
		else
		{
			pSprHead->SetDiffuse( colorDiffuse );
			pSprHead->SetGlow( RageColor(0,0,0,0) );
		}
		pSprHead->Draw();
	}

	// now, draw the glow pass
	if( !bDrawGlowOnly )
		DrawHold( hn, bActive, fLife, fPercentFadeToFail, true );
}

void NoteDisplay::DrawTap( const int iCol, const float fBeat, const bool bOnSameRowAsHoldStart, const float fPercentFadeToFail, const float fLife )
{
	const float fYOffset		= ArrowGetYOffset(			m_PlayerNumber, fBeat );
	const float fYPos			= ArrowGetYPos(	m_PlayerNumber, fYOffset );
	const float fRotation		= ArrowGetRotation(			m_PlayerNumber, fBeat );
	const float fXPos			= ArrowGetXPos(				m_PlayerNumber, iCol, fYPos );
	const float fAlpha			= ArrowGetAlpha(			m_PlayerNumber, fYPos, fPercentFadeToFail );
	const float fGlow			= ArrowGetGlow(				m_PlayerNumber, fYPos, fPercentFadeToFail );
	const float fColorScale		= SCALE(fLife,0,1,0.2f,1);
	RageColor diffuse = RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	RageColor glow = RageColor(1,1,1,fGlow);

	/*
	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_ColorType == PlayerOptions::COLOR_NOTE )
	{
		RageColor noteColor = GetNoteColorFromBeat(fBeat);
		diffuse.r *= noteColor.r;
		diffuse.g *= noteColor.g;
		diffuse.b *= noteColor.b;

		glow = RageColor(1,1,1,1)*fGlow + noteColor*(1-fGlow)*0.7f*fAlpha;
	}
	*/

	Sprite* pSprite;
	if( bOnSameRowAsHoldStart  &&  cache->m_bDrawHoldHeadForTapsOnSameRow )
	{
		// draw hold head
		pSprite = GetHoldHeadSprite( fBeat, false );
		pSprite->StopUsingCustomCoords();
	}
	else	
	{
		// draw tap
		pSprite = GetTapNoteSprite( fBeat );
		pSprite->SetRotation( fRotation );
	}

	pSprite->SetXY( fXPos, fYPos );
	pSprite->SetDiffuse( diffuse );
	pSprite->SetGlow( glow );
	pSprite->Draw();
}


//	if( ct == PlayerOptions::COLOR_NOTE )
//	{
//		RageColor color = GetNoteColorFromBeat( fNoteBeat );
//		colorLeadingOut = color;
//		colorTrailingOut = color;
//
//		// add a little bit of white so the note doesn't look so plain
//		colorLeadingOut.r += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
//		colorLeadingOut.g += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
//		colorLeadingOut.b += 0.3f * fabsf( fPercentThroughColorsLeading - 0.5f );
//		colorTrailingOut.r += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
//		colorTrailingOut.g += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
//		colorTrailingOut.b += 0.3f * fabsf( fPercentThroughColorsTrailing - 0.5f );
//		return;
//	}
