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
#include "GameManager.h"
#include "RageException.h"
#include "ArrowEffects.h"
#include "RageLog.h"
#include <math.h>
#include "RageDisplay.h"
#include "NoteTypes.h"


#define DRAW_HOLD_HEAD_FOR_TAPS_ON_SAME_ROW			GAMEMAN->GetMetricB("NoteDisplay","DrawHoldHeadForTapsOnSameRow")
#define TAP_NOTE_ANIMATION_LENGTH_IN_BEATS			GAMEMAN->GetMetricF("NoteDisplay","TapNoteAnimationLengthInBeats")
#define HOLD_HEAD_ANIMATION_LENGTH_IN_BEATS			GAMEMAN->GetMetricF("NoteDisplay","HoldHeadAnimationLengthInBeats")
#define HOLD_BODY_ANIMATION_LENGTH_IN_BEATS			GAMEMAN->GetMetricF("NoteDisplay","HoldBodyAnimationLengthInBeats")
#define HOLD_TAIL_ANIMATION_LENGTH_IN_BEATS			GAMEMAN->GetMetricF("NoteDisplay","HoldTailAnimationLengthInBeats")
#define TAP_NOTE_ANIMATION_IS_VIVID					GAMEMAN->GetMetricB("NoteDisplay","TapNoteAnimationIsVivid")
#define HOLD_HEAD_ANIMATION_IS_VIVID				GAMEMAN->GetMetricB("NoteDisplay","HoldHeadAnimationIsVivid")
#define HOLD_BODY_ANIMATION_IS_VIVID				GAMEMAN->GetMetricB("NoteDisplay","HoldBodyAnimationIsVivid")
#define HOLD_TAIL_ANIMATION_IS_VIVID				GAMEMAN->GetMetricB("NoteDisplay","HoldTailAnimationIsVivid")
#define TAP_NOTE_ANIMATION_IS_NOTE_COLOR			GAMEMAN->GetMetricB("NoteDisplay","TapNoteAnimationIsNoteColor")
#define HOLD_HEAD_ANIMATION_IS_NOTE_COLOR			GAMEMAN->GetMetricB("NoteDisplay","HoldHeadAnimationIsNoteColor")
#define HOLD_BODY_ANIMATION_IS_NOTE_COLOR			GAMEMAN->GetMetricB("NoteDisplay","HoldBodyAnimationIsNoteColor")
#define HOLD_TAIL_ANIMATION_IS_NOTE_COLOR			GAMEMAN->GetMetricB("NoteDisplay","HoldTailAnimationIsNoteColor")
#define START_DRAWING_HOLD_BODY_OFFSET_FROM_HEAD	GAMEMAN->GetMetricI("NoteDisplay","StartDrawingHoldBodyOffsetFromHead")
#define STOP_DRAWING_HOLD_BODY_OFFSET_FROM_TAIL		GAMEMAN->GetMetricI("NoteDisplay","StopDrawingHoldBodyOffsetFromTail")
#define HOLD_HEAD_IS_WAVY							GAMEMAN->GetMetricB("NoteDisplay","HoldHeadIsWavy")
#define HOLD_TAIL_IS_WAVY							GAMEMAN->GetMetricB("NoteDisplay","HoldTailIsWavy")
#define HOLD_NG_GRAY_PERCENT						GAMEMAN->GetMetricF("NoteDisplay","HoldNGGrayPercent")

// cache
bool g_bDrawHoldHeadForTapsOnSameRow;
float g_fTapNoteAnimationLengthInBeats;
float g_fHoldHeadAnimationLengthInBeats;
float g_fHoldBodyAnimationLengthInBeats;
float g_fHoldTailAnimationLengthInBeats;
bool g_bTapNoteAnimationIsVivid;
bool g_bHoldHeadAnimationIsVivid;
bool g_bHoldBodyAnimationIsVivid;
bool g_bHoldTailAnimationIsVivid;
bool g_bTapNoteAnimationIsNoteColor;
bool g_bHoldHeadAnimationIsNoteColor;
bool g_bHoldBodyAnimationIsNoteColor;
bool g_bHoldTailAnimationIsNoteColor;
int g_iStartDrawingHoldBodyOffsetFromHead;
int g_iStopDrawingHoldBodyOffsetFromTail;
bool g_bHoldHeadIsWavy;
bool g_bHoldTailIsWavy;
float g_fHoldNGGrayPercent;

NoteDisplay::NoteDisplay()
{
	g_bDrawHoldHeadForTapsOnSameRow = DRAW_HOLD_HEAD_FOR_TAPS_ON_SAME_ROW;
	g_fTapNoteAnimationLengthInBeats = TAP_NOTE_ANIMATION_LENGTH_IN_BEATS;
	g_fHoldHeadAnimationLengthInBeats = HOLD_HEAD_ANIMATION_LENGTH_IN_BEATS;
	g_fHoldBodyAnimationLengthInBeats = HOLD_BODY_ANIMATION_LENGTH_IN_BEATS;
	g_fHoldTailAnimationLengthInBeats = HOLD_TAIL_ANIMATION_LENGTH_IN_BEATS;
	g_bTapNoteAnimationIsVivid = TAP_NOTE_ANIMATION_IS_VIVID;
	g_bHoldHeadAnimationIsVivid = HOLD_HEAD_ANIMATION_IS_VIVID;
	g_bHoldBodyAnimationIsVivid = HOLD_BODY_ANIMATION_IS_VIVID;
	g_bHoldTailAnimationIsVivid = HOLD_TAIL_ANIMATION_IS_VIVID;
	g_bTapNoteAnimationIsNoteColor = TAP_NOTE_ANIMATION_IS_NOTE_COLOR;
	g_bHoldHeadAnimationIsNoteColor = HOLD_HEAD_ANIMATION_IS_NOTE_COLOR;
	g_bHoldBodyAnimationIsNoteColor = HOLD_BODY_ANIMATION_IS_NOTE_COLOR;
	g_bHoldTailAnimationIsNoteColor = HOLD_TAIL_ANIMATION_IS_NOTE_COLOR;
	g_iStartDrawingHoldBodyOffsetFromHead = START_DRAWING_HOLD_BODY_OFFSET_FROM_HEAD;
	g_iStopDrawingHoldBodyOffsetFromTail = STOP_DRAWING_HOLD_BODY_OFFSET_FROM_TAIL;
	g_bHoldHeadIsWavy = HOLD_HEAD_IS_WAVY;
	g_bHoldTailIsWavy = HOLD_TAIL_IS_WAVY;
	g_fHoldNGGrayPercent = HOLD_NG_GRAY_PERCENT;
}


void NoteDisplay::Load( int iColNum, PlayerNumber pn )
{
	m_PlayerNumber = pn;

	m_sprTapNote.Load(			GAMEMAN->GetPathTo(iColNum, "tap note") );
	m_sprHoldHeadActive.Load(	GAMEMAN->GetPathTo(iColNum, "hold head active") );
	m_sprHoldHeadInactive.Load( GAMEMAN->GetPathTo(iColNum, "hold head inactive") );
	m_sprHoldBodyActive.Load(	GAMEMAN->GetPathTo(iColNum, "hold body active") );
	m_sprHoldBodyInactive.Load( GAMEMAN->GetPathTo(iColNum, "hold body inactive") );
	m_sprHoldTailActive.Load(	GAMEMAN->GetPathTo(iColNum, "hold tail active") );
	m_sprHoldTailInactive.Load( GAMEMAN->GetPathTo(iColNum, "hold tail inactive") );
}


int NoteDisplay::GetFrameNo( float fNoteBeat, int iNumFrames, float fAnimationLengthInBeats, bool bVivid, bool bNoteColor )
{
	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fPercentIntoMeasure = fmodfp( fSongBeat, fAnimationLengthInBeats ) / fAnimationLengthInBeats;

	float fBeatFraction = fmodf( fNoteBeat, 1.0f );
	fBeatFraction = froundf( fBeatFraction, 0.25f );	// round to nearest 1/4th

	int iBeatFractionContrib = (int)roundf( fBeatFraction * iNumFrames );
	int iSongBeatFrameContrib = (int)roundf( fPercentIntoMeasure * iNumFrames );

	int iFrameNo;
	if( bVivid )
	{
		iFrameNo = (iSongBeatFrameContrib + iBeatFractionContrib) % iNumFrames;
	}
	else if( bNoteColor )
	{
		NoteType type = BeatToNoteType( fNoteBeat );
		int iBaseFrameNo;
		switch( type )
		{
		case NOTE_TYPE_4TH:		iBaseFrameNo = iNumFrames*0/6;	break;
		case NOTE_TYPE_8TH:		iBaseFrameNo = iNumFrames*1/6;	break;
		case NOTE_TYPE_12TH:	iBaseFrameNo = iNumFrames*2/6;	break;
		case NOTE_TYPE_16TH:	iBaseFrameNo = iNumFrames*3/6;	break;
		case NOTE_TYPE_24TH:	iBaseFrameNo = iNumFrames*4/6;	break;
		case NOTE_TYPE_32ND:	iBaseFrameNo = iNumFrames*5/6;	break;
		default:	ASSERT(0);	iBaseFrameNo=0;	// get rid of warning
		}

		iFrameNo = iBaseFrameNo + iBeatFractionContrib;
	}
	else	// flat
	{
		iFrameNo = (iSongBeatFrameContrib) % iNumFrames;
	}

	ASSERT( iFrameNo>=0 && iFrameNo<iNumFrames );
	return iFrameNo;
}

int NoteDisplay::GetTapNoteFrameNo( float fNoteBeat )
{
	return GetFrameNo( 
		fNoteBeat, 
		m_sprTapNote.GetNumStates(), 
		g_fTapNoteAnimationLengthInBeats, 
		g_bTapNoteAnimationIsVivid, 
		g_bTapNoteAnimationIsNoteColor );
}

int	NoteDisplay::GetHoldHeadFrameNo( float fNoteBeat, bool bActive )
{
	return GetFrameNo( 
		fNoteBeat, 
		(bActive?m_sprHoldHeadActive:m_sprHoldHeadInactive).GetNumStates(), 
		g_fHoldHeadAnimationLengthInBeats, 
		g_bHoldHeadAnimationIsVivid, 
		g_bHoldHeadAnimationIsNoteColor );
}

int	NoteDisplay::GetHoldBodyFrameNo( float fNoteBeat, bool bActive )
{
	return GetFrameNo( 
		fNoteBeat, 
		(bActive?m_sprHoldBodyActive:m_sprHoldBodyInactive).GetNumStates(), 
		g_fHoldBodyAnimationLengthInBeats, 
		g_bHoldBodyAnimationIsVivid, 
		g_bHoldBodyAnimationIsNoteColor );
}

int	NoteDisplay::GetHoldTailFrameNo( float fNoteBeat, bool bActive )
{
	return GetFrameNo( 
		fNoteBeat, 
		(bActive?m_sprHoldTailActive:m_sprHoldTailInactive).GetNumStates(), 
		g_fHoldTailAnimationLengthInBeats, 
		g_bHoldTailAnimationIsVivid, 
		g_bHoldTailAnimationIsNoteColor );
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

	const float fColorScale		= 1*fLife + (1-fLife)*g_fHoldNGGrayPercent;


	//
	// draw from tail to head (so head appears on top)
	//

	static RageVertex queue[4096];

	//
	// Draw the tail
	//
	{
		Sprite* pSprTail = bActive ? &m_sprHoldTailActive : &m_sprHoldTailInactive;
		int iFrameNo = GetHoldTailFrameNo( hn.m_fStartBeat, bActive );

		if( g_bHoldTailIsWavy )
		{
			// draw manually in small segments
			RageVertex *v = &queue[0];
			RageTexture* pTexture = pSprTail->GetTexture();
			const RectF* pRect = pTexture->GetTextureCoordRect( iFrameNo );
			DISPLAY->SetTexture( pTexture );
			DISPLAY->SetBlendModeNormal();
			if( bDrawGlowOnly )
				DISPLAY->SetTextureModeGlow();
			else
				DISPLAY->SetTextureModeModulate();
			DISPLAY->DisableTextureWrapping();


			const float fFrameWidth		= pSprTail->GetUnzoomedWidth();
			const float fFrameHeight	= pSprTail->GetUnzoomedHeight();
			const float fYTailTop		= fYTail-fFrameHeight/2;		
			const float fYTailBottom    = fYTail+fFrameHeight/2;	

			float fY=max( fYTailTop, fYHead );	// don't draw any part of the tail that is before the middle of the head
			for( ; fY<fYTailBottom; fY+=fYStep )	
			{
				const float fYTop					= fY;
				const float fYBottom				= min( fY+fYStep, fYTailBottom );
				const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
				const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
				const float fXTopLeft				= fXTop - fFrameWidth/2;
				const float fXTopRight				= fXTop + fFrameWidth/2;
				const float fXBottomLeft			= fXBottom - fFrameWidth/2;
				const float fXBottomRight			= fXBottom + fFrameWidth/2;
				const float fTopDistFromTailTop		= fYTop - fYTailTop;
				const float fBottomDistFromTailTop	= fYBottom - fYTailTop;
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
		else	//	!g_bHoldTailIsWavy
		{
			// draw with normal Sprite
			const float fY				= fYHead;
			const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fY );
			const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, fY, fPercentFadeToFail );
			const float	fGlow			= ArrowGetGlow( m_PlayerNumber, fY, fPercentFadeToFail );
			const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
			const RageColor colorGlow	= RageColor(1,1,1,fGlow);

			pSprTail->SetXY( fX, fY );
			pSprTail->SetState( iFrameNo );
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
	}

	//
	// Draw the body (always wavy)
	//
	{
		Sprite* pSprBody = bActive ? &m_sprHoldBodyActive : &m_sprHoldBodyInactive;
		int iFrameNo = GetHoldBodyFrameNo( hn.m_fStartBeat, bActive );
		
		// draw manually in small segments
		RageVertex *v = &queue[0];
		RageTexture* pTexture = pSprBody->GetTexture();
		const RectF* pRect = pTexture->GetTextureCoordRect( iFrameNo );
		DISPLAY->SetTexture( pTexture );
		DISPLAY->SetBlendModeNormal();
		if( bDrawGlowOnly )
			DISPLAY->SetTextureModeGlow();
		else
			DISPLAY->SetTextureModeModulate();
		DISPLAY->EnableTextureWrapping();


		const float fFrameWidth  = pSprBody->GetUnzoomedWidth();
		const float fFrameHeight = pSprBody->GetUnzoomedHeight();
		const float fYBodyTop = fYHead + g_iStartDrawingHoldBodyOffsetFromHead;
		const float fYBodyBottom = fYTail + g_iStopDrawingHoldBodyOffsetFromTail;
		
		// top to bottom
		for( float fY=fYBodyTop; fY<=fYBodyBottom+2; fY+=fYStep )	// HACK: +1 to fill gab between body and tail.
		{
			const float fYTop					= fY;
			const float fYBottom				= min( fY+fYStep, fYBodyBottom+1 );	// HACK: +1 to fill gab between body and tail.
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
	// Draw the head
	//
	{
		Sprite* pSprHead = bActive ? &m_sprHoldHeadActive : &m_sprHoldHeadInactive;
		int iFrameNo = GetHoldHeadFrameNo( hn.m_fStartBeat, bActive );

		if( g_bHoldHeadIsWavy )
		{
			// draw manually in small segments
			RageVertex *v = &queue[0];
			RageTexture* pTexture = pSprHead->GetTexture();
			const RectF* pRect = pTexture->GetTextureCoordRect( iFrameNo );
			DISPLAY->SetTexture( pTexture );
			DISPLAY->SetBlendModeNormal();
			if( bDrawGlowOnly )
				DISPLAY->SetTextureModeGlow();
			else
				DISPLAY->SetTextureModeModulate();
			DISPLAY->EnableTextureWrapping();

			const float fFrameWidth  = pSprHead->GetUnzoomedWidth();
			const float fFrameHeight = pSprHead->GetUnzoomedHeight();
			const float fYHeadTop		 = fYHead-fFrameHeight/2;		
			const float fYHeadBottom     = fYHead+fFrameHeight/2;	

			for( float fY=fYHeadTop; fY<fYHeadBottom; fY+=fYStep )	
			{
				const float fYTop					= fY;
				const float fYBottom				= fY+fYStep;
				const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYTop );
				const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYBottom );
				const float fXTopLeft				= fXTop - fFrameWidth/2;
				const float fXTopRight				= fXTop + fFrameWidth/2;
				const float fXBottomLeft			= fXBottom - fFrameWidth/2;
				const float fXBottomRight			= fXBottom + fFrameWidth/2;
				const float fTopDistFromHeadTop		= fYTop - fYHeadTop;
				const float fBottomDistFromHeadTop	= fYBottom - fYHeadTop;
				const float fTexCoordTop			= SCALE( fTopDistFromHeadTop,    0, fFrameHeight, pRect->top, pRect->bottom );
				const float fTexCoordBottom			= SCALE( fBottomDistFromHeadTop, 0, fFrameHeight, pRect->top, pRect->bottom );
				ASSERT( fTexCoordTop>=0 && fTexCoordBottom<=1 );
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
		else	//	!g_bHoldHeadIsWavy
		{
			// draw with normal Sprite
			const float fY				= fYHead;
			const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fY );
			const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, fY, fPercentFadeToFail );
			const float	fGlow			= ArrowGetGlow( m_PlayerNumber, fY, fPercentFadeToFail );
			const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
			const RageColor colorGlow	= RageColor(1,1,1,fGlow);

			pSprHead->SetXY( fX, fY );
			pSprHead->SetState( iFrameNo );
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

	if( GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_ColorType == PlayerOptions::COLOR_NOTE )
	{
		RageColor noteColor = GetNoteColorFromBeat(fBeat);
		diffuse.r *= noteColor.r;
		diffuse.g *= noteColor.g;
		diffuse.b *= noteColor.b;

		glow = RageColor(1,1,1,1)*fGlow + noteColor*(1-fGlow)*0.7f*fAlpha;
	}

	if( bOnSameRowAsHoldStart  &&  g_bDrawHoldHeadForTapsOnSameRow )
	{
		// draw hold head
		const int iHoldHeadFrameNo		= GetHoldHeadFrameNo( fBeat, false );
		m_sprHoldHeadInactive.SetXY( fXPos, fYPos );
		m_sprHoldHeadInactive.SetDiffuse( diffuse );
		m_sprHoldHeadInactive.SetGlow( glow );
		m_sprHoldHeadInactive.StopUsingCustomCoords();
		m_sprHoldHeadInactive.SetState( iHoldHeadFrameNo );
		m_sprHoldHeadInactive.Draw();
	}
	else	
	{
		const int iTapFrameNo		= GetTapNoteFrameNo( fBeat );
		m_sprTapNote.SetXY( fXPos, fYPos );
		m_sprTapNote.SetRotation( fRotation );
		m_sprTapNote.SetGlow( glow );
		m_sprTapNote.SetDiffuse( diffuse );
		m_sprTapNote.SetState( iTapFrameNo );
		m_sprTapNote.Draw();
	}
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
