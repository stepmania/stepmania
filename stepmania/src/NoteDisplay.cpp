#include "global.h"
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
#include "Steps.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "NoteSkinManager.h"
#include "RageException.h"
#include "ArrowEffects.h"
#include "RageLog.h"
#include <math.h>
#include "RageDisplay.h"
#include "NoteTypes.h"
#include "NoteFieldPositioning.h"
#include "ActorUtil.h"



#define DRAW_HOLD_HEAD_FOR_TAPS_ON_SAME_ROW			NOTESKIN->GetMetricB(skin,name,"DrawHoldHeadForTapsOnSameRow")
#define TAP_NOTE_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(skin,name,"TapNoteAnimationLengthInBeats")
#define TAP_ADDITION_ANIMATION_LENGTH_IN_BEATS		NOTESKIN->GetMetricF(skin,name,"TapAdditionAnimationLengthInBeats")
#define TAP_MINE_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(skin,name,"TapMineAnimationLengthInBeats")
#define HOLD_HEAD_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(skin,name,"HoldHeadAnimationLengthInBeats")
#define HOLD_TOPCAP_ANIMATION_LENGTH_IN_BEATS		NOTESKIN->GetMetricF(skin,name,"HoldTopCapAnimationLengthInBeats")
#define HOLD_BODY_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(skin,name,"HoldBodyAnimationLengthInBeats")
#define HOLD_BOTTOMCAP_ANIMATION_LENGTH_IN_BEATS	NOTESKIN->GetMetricF(skin,name,"HoldBottomCapAnimationLengthInBeats")
#define HOLD_TAIL_ANIMATION_LENGTH_IN_BEATS			NOTESKIN->GetMetricF(skin,name,"HoldTailAnimationLengthInBeats")
#define TAP_NOTE_ANIMATION_IS_VIVID					NOTESKIN->GetMetricB(skin,name,"TapNoteAnimationIsVivid")
#define TAP_ADDITION_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(skin,name,"TapAdditionAnimationIsVivid")
#define TAP_MINE_ANIMATION_IS_VIVID					NOTESKIN->GetMetricB(skin,name,"TapMineAnimationIsVivid")
#define HOLD_HEAD_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(skin,name,"HoldHeadAnimationIsVivid")
#define HOLD_TOPCAP_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(skin,name,"HoldTopCapAnimationIsVivid")
#define HOLD_BODY_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(skin,name,"HoldBodyAnimationIsVivid")
#define HOLD_BOTTOMCAP_ANIMATION_IS_VIVID			NOTESKIN->GetMetricB(skin,name,"HoldBottomCapAnimationIsVivid")
#define HOLD_TAIL_ANIMATION_IS_VIVID				NOTESKIN->GetMetricB(skin,name,"HoldTailAnimationIsVivid")
#define TAP_NOTE_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(skin,name,"TapNoteAnimationIsNoteColor")
#define HOLD_HEAD_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(skin,name,"HoldHeadAnimationIsNoteColor")
#define HOLD_TOPCAP_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(skin,name,"HoldTopCapAnimationIsNoteColor")
#define HOLD_BODY_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(skin,name,"HoldBodyAnimationIsNoteColor")
#define HOLD_BOTTOMCAP_ANIMATION_IS_NOTE_COLOR		NOTESKIN->GetMetricB(skin,name,"HoldBottomCapAnimationIsNoteColor")
#define HOLD_TAIL_ANIMATION_IS_NOTE_COLOR			NOTESKIN->GetMetricB(skin,name,"HoldTailAnimationIsNoteColor")
#define HOLD_HEAD_IS_ABOVE_WAVY_PARTS				NOTESKIN->GetMetricB(skin,name,"HoldHeadIsAboveWavyParts")
#define HOLD_TAIL_IS_ABOVE_WAVY_PARTS				NOTESKIN->GetMetricB(skin,name,"HoldTailIsAboveWavyParts")
#define START_DRAWING_HOLD_BODY_OFFSET_FROM_HEAD	NOTESKIN->GetMetricI(skin,name,"StartDrawingHoldBodyOffsetFromHead")
#define STOP_DRAWING_HOLD_BODY_OFFSET_FROM_TAIL		NOTESKIN->GetMetricI(skin,name,"StopDrawingHoldBodyOffsetFromTail")
#define HOLD_NG_GRAY_PERCENT						NOTESKIN->GetMetricF(skin,name,"HoldNGGrayPercent")
#define USE_LIGHTING								NOTESKIN->GetMetricB(skin,name,"UseLighting")
#define FLIP_HEAD_AND_TAIL_WHEN_REVERSE				NOTESKIN->GetMetricB(skin,name,"FlipHeadAndTailWhenReverse")

// cache
struct NoteMetricCache_t {
	bool m_bDrawHoldHeadForTapsOnSameRow;
	float m_fTapNoteAnimationLengthInBeats;
	float m_fTapAdditionAnimationLengthInBeats;
	float m_fTapMineAnimationLengthInBeats;
	float m_fHoldHeadAnimationLengthInBeats;
	float m_fHoldTopCapAnimationLengthInBeats;
	float m_fHoldBodyAnimationLengthInBeats;
	float m_fHoldBottomCapAnimationLengthInBeats;
	float m_fHoldTailAnimationLengthInBeats;
	bool m_bTapNoteAnimationIsVivid;
	bool m_bTapAdditionAnimationIsVivid;
	bool m_bTapMineAnimationIsVivid;
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
	bool m_bHoldHeadIsAboveWavyParts;
	bool m_bHoldTailIsAboveWavyParts;
	int m_iStartDrawingHoldBodyOffsetFromHead;
	int m_iStopDrawingHoldBodyOffsetFromTail;
	float m_fHoldNGGrayPercent;
	bool m_bUseLighting;
	bool m_bFlipHeadAndTailWhenReverse;

	void Load(CString skin, const CString &name);
} *NoteMetricCache;

void NoteMetricCache_t::Load(CString skin, const CString &name)
{
	m_bDrawHoldHeadForTapsOnSameRow = DRAW_HOLD_HEAD_FOR_TAPS_ON_SAME_ROW;
	m_fTapNoteAnimationLengthInBeats = TAP_NOTE_ANIMATION_LENGTH_IN_BEATS;
	m_fTapAdditionAnimationLengthInBeats = TAP_ADDITION_ANIMATION_LENGTH_IN_BEATS;
	m_fTapMineAnimationLengthInBeats = TAP_MINE_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldHeadAnimationLengthInBeats = HOLD_HEAD_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldTopCapAnimationLengthInBeats = HOLD_TOPCAP_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldBodyAnimationLengthInBeats = HOLD_BODY_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldBottomCapAnimationLengthInBeats = HOLD_BOTTOMCAP_ANIMATION_LENGTH_IN_BEATS;
	m_fHoldTailAnimationLengthInBeats = HOLD_TAIL_ANIMATION_LENGTH_IN_BEATS;
	m_bTapNoteAnimationIsVivid = TAP_NOTE_ANIMATION_IS_VIVID;
	m_bTapAdditionAnimationIsVivid = TAP_ADDITION_ANIMATION_IS_VIVID;
	m_bTapMineAnimationIsVivid = TAP_MINE_ANIMATION_IS_VIVID;
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
	m_bHoldHeadIsAboveWavyParts = HOLD_HEAD_IS_ABOVE_WAVY_PARTS;
	m_bHoldTailIsAboveWavyParts = HOLD_TAIL_IS_ABOVE_WAVY_PARTS;
	m_iStartDrawingHoldBodyOffsetFromHead = START_DRAWING_HOLD_BODY_OFFSET_FROM_HEAD;
	m_iStopDrawingHoldBodyOffsetFromTail = STOP_DRAWING_HOLD_BODY_OFFSET_FROM_TAIL;
	m_fHoldNGGrayPercent = HOLD_NG_GRAY_PERCENT;
	m_bUseLighting = USE_LIGHTING;
	m_bFlipHeadAndTailWhenReverse = FLIP_HEAD_AND_TAIL_WHEN_REVERSE;
}

NoteDisplay::NoteDisplay()
{
	for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
	{
		m_pTapNote[i] = NULL;
		m_pHoldHeadActive[i] = NULL;
		m_pHoldHeadInactive[i] = NULL;
		m_pHoldTailActive[i] = NULL;
		m_pHoldTailInactive[i] = NULL;
	}
	m_pTapAddition = NULL;
	m_pTapMine = NULL;

	cache = new NoteMetricCache_t;
}

NoteDisplay::~NoteDisplay()
{
	for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
	{
		delete m_pTapNote[i];
		delete m_pHoldHeadActive[i];
		delete m_pHoldHeadInactive[i];
		delete m_pHoldTailActive[i];
		delete m_pHoldTailInactive[i];
	}
	delete m_pTapAddition;
	delete m_pTapMine;

	delete cache;
}

void NoteDisplay::Load( int iColNum, PlayerNumber pn, CString NoteSkin, float fYReverseOffsetPixels )
{
	m_PlayerNumber = pn;
	m_fYReverseOffsetPixels = fYReverseOffsetPixels;

	/* Normally, this is empty and we use the style table entry via ColToButtonName. */
	CString Button = g_NoteFieldMode[m_PlayerNumber].NoteButtonNames[iColNum];
	if(Button == "")
		Button = NoteSkinManager::ColToButtonName(iColNum);

	cache->Load( NoteSkin, Button );

	// Look up note names once and store them here.
	CString sNoteType[ NOTE_COLOR_IMAGES ];
	for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		sNoteType[i] = NoteTypeToString( (NoteType)i );


	if( cache->m_bTapNoteAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
			m_pTapNote[i] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "tap note "+sNoteType[i]) );
	}
	else
	{
		m_pTapNote[0] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "tap note") );
	}

	m_pTapAddition = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "tap addition") );

	m_pTapMine = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "tap mine") );

	if( cache->m_bHoldHeadAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_pHoldHeadActive[i] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold head active "+sNoteType[i]) );
			m_pHoldHeadInactive[i] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold head inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_pHoldHeadActive[0] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold head active") );
		m_pHoldHeadInactive[0] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold head inactive") );
	}

	if( cache->m_bHoldTopCapAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldTopCapActive[i].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold topcap active "+sNoteType[i]) );
			m_sprHoldTopCapInactive[i].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold topcap inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldTopCapActive[0].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold topcap active") );
		m_sprHoldTopCapInactive[0].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold topcap inactive") );
	}

	if( cache->m_bHoldBodyAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldBodyActive[i].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold body active "+sNoteType[i]) );
			m_sprHoldBodyInactive[i].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold body inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldBodyActive[0].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold body active") );
		m_sprHoldBodyInactive[0].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold body inactive") );
	}

	if( cache->m_bHoldBottomCapAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldBottomCapActive[i].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold bottomcap active "+sNoteType[i]) );
			m_sprHoldBottomCapInactive[i].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold bottomcap inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldBottomCapActive[0].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold bottomcap active") );
		m_sprHoldBottomCapInactive[0].Load( NOTESKIN->GetPathTo(NoteSkin, Button, "hold bottomcap inactive") );
	}

	if( cache->m_bHoldTailAnimationIsNoteColor )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_pHoldTailActive[i] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold tail active "+sNoteType[i]) );
			m_pHoldTailInactive[i] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold tail inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_pHoldTailActive[0] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold tail active") );
		m_pHoldTailInactive[0] = MakeActor( NOTESKIN->GetPathTo(NoteSkin, Button, "hold tail inactive") );
	}
}


void NoteDisplay::SetActiveFrame( float fNoteBeat, Actor &actorToSet, float fAnimationLengthInBeats, bool bVivid, bool bNoteColor )
{
	const int iNumFrames = actorToSet.GetNumStates();
	if( iNumFrames == 0 )	// Model with no textures
		return;

	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fPrecentIntoAnimation = fmodf(fSongBeat,fAnimationLengthInBeats) / fAnimationLengthInBeats;
	float fNoteBeatFraction = fmodf( fNoteBeat, 1.0f );

	int iFrameNo = (int)(fPrecentIntoAnimation*iNumFrames);
	if( bVivid )
		iFrameNo += (int)( froundf(fNoteBeatFraction,1.f/fAnimationLengthInBeats)*iNumFrames );

	iFrameNo += iNumFrames;
	iFrameNo %= iNumFrames;

	ASSERT( iFrameNo>=0 && iFrameNo<iNumFrames );

	actorToSet.SetState( iFrameNo );
}

Actor * NoteDisplay::GetTapNoteActor( float fNoteBeat )
{
	NoteType nt = NoteType(0);
	if( cache->m_bTapNoteAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Actor *pActorOut = m_pTapNote[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pActorOut,
		cache->m_fTapNoteAnimationLengthInBeats, 
		cache->m_bTapNoteAnimationIsVivid, 
		cache->m_bTapNoteAnimationIsNoteColor );

	return pActorOut;
}

Actor * NoteDisplay::GetTapAdditionActor( float fNoteBeat )
{
	SetActiveFrame( 
		fNoteBeat, 
		*m_pTapAddition,
		cache->m_fTapAdditionAnimationLengthInBeats, 
		cache->m_bTapAdditionAnimationIsVivid, 
		false );

	return m_pTapAddition;
}

Actor * NoteDisplay::GetTapMineActor( float fNoteBeat )
{
	SetActiveFrame( 
		fNoteBeat, 
		*m_pTapMine,
		cache->m_fTapMineAnimationLengthInBeats, 
		cache->m_bTapMineAnimationIsVivid, 
		false );

	return m_pTapMine;
}

Sprite * NoteDisplay::GetHoldTopCapSprite( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldTopCapAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );
	
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
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );
	
	Sprite *pSpriteOut = bActive ? &m_sprHoldBottomCapActive[nt] : &m_sprHoldBottomCapInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fHoldBottomCapAnimationLengthInBeats, 
		cache->m_bHoldBottomCapAnimationIsVivid, 
		cache->m_bHoldBottomCapAnimationIsNoteColor );

	return pSpriteOut;
}


Actor* NoteDisplay::GetHoldHeadActor( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldHeadAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Actor *pActorOut = bActive ? m_pHoldHeadActive[nt] : m_pHoldHeadInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pActorOut, 
		cache->m_fHoldHeadAnimationLengthInBeats, 
		cache->m_bHoldHeadAnimationIsVivid, 
		cache->m_bHoldHeadAnimationIsNoteColor );

	return pActorOut;
}

Sprite *NoteDisplay::GetHoldBodySprite( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldBodyAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Sprite *pSpriteOut = bActive ? &m_sprHoldBodyActive[nt] : &m_sprHoldBodyInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fHoldBodyAnimationLengthInBeats, 
		cache->m_bHoldBodyAnimationIsVivid, 
		cache->m_bHoldBodyAnimationIsNoteColor );

	return pSpriteOut;
}

Actor* NoteDisplay::GetHoldTailActor( float fNoteBeat, bool bActive )
{
	NoteType nt = NoteType(0);
	if( cache->m_bHoldTailAnimationIsNoteColor )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Actor *pActorOut = bActive ? m_pHoldTailActive[nt] : m_pHoldTailInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pActorOut, 
		cache->m_fHoldTailAnimationLengthInBeats, 
		cache->m_bHoldTailAnimationIsVivid, 
		cache->m_bHoldTailAnimationIsNoteColor );

	return pActorOut;
}


void NoteDisplay::DrawHoldTopCap( const HoldNote& hn, const bool bActive, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the top cap (always wavy)
	//
	const int size = 4096;
	static RageSpriteVertex queue[size];
	Sprite* pSprTopCap = GetHoldTopCapSprite( hn.GetStartBeat(), bActive );

	// draw manually in small segments
	RageSpriteVertex *v = &queue[0];
	RageTexture* pTexture = pSprTopCap->GetTexture();
	const RectF *pRect = pSprTopCap->GetCurrentTextureCoordRect();
	DISPLAY->SetTexture( pTexture );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetBackfaceCull( false );
	DISPLAY->SetTextureWrapping(false);

	const float fFrameWidth  = pSprTopCap->GetUnzoomedWidth();
	const float fFrameHeight = pSprTopCap->GetUnzoomedHeight();
	const float fYCapTop	 = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead-fFrameHeight;
	const float fYCapBottom  = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead;

	for( float fY=fYCapTop; fY<fYCapBottom; fY+=fYStep )
	{
		const float fYTop					= fY;
		const float fYOffsetTop				= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fYTop, m_fYReverseOffsetPixels );
		const float fStepHeight				= min( fYStep, fFrameHeight );
		const float fYBottom				= min( fY+fStepHeight, fYCapBottom );
		const float fYOffsetBottom			= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fYBottom, m_fYReverseOffsetPixels );
		const float fZTop					= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffsetTop );
		const float fZBottom				= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffsetBottom );
		const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYOffsetTop );
		const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYOffsetBottom );
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
		const float	fAlphaTop				= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffsetTop, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fAlphaBottom			= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffsetBottom, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fGlowTop				= ArrowGetGlow( m_PlayerNumber, iCol, fYOffsetTop, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fGlowBottom				= ArrowGetGlow( m_PlayerNumber, iCol, fYOffsetBottom, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
		const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

		if( bGlow && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bGlow && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		v[0].p = RageVector3(fXTopLeft,    fYTop,   fZTop);		v[0].c = bGlow? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
		v[1].p = RageVector3(fXTopRight,   fYTop,   fZTop);		v[1].c = bGlow? colorGlowTop    : colorDiffuseTop;    	v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v[2].p = RageVector3(fXBottomRight,fYBottom,fZBottom);	v[2].c = bGlow? colorGlowBottom : colorDiffuseBottom; 	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
		v[3].p = RageVector3(fXBottomLeft, fYBottom,fZBottom);	v[3].c = bGlow? colorGlowBottom : colorDiffuseBottom; 	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
		v+=4;
		if( v-queue >= size )
			break;
	}
	DISPLAY->DrawQuads( queue, v-queue );
}

void NoteDisplay::DrawHoldBody( const HoldNote& hn, const bool bActive, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the body (always wavy)
	//
	const int size = 4096;
	static RageSpriteVertex queue[size];

	Sprite* pSprBody = GetHoldBodySprite( hn.GetStartBeat(), bActive );

	// draw manually in small segments
	RageSpriteVertex *v = &queue[0];
	RageTexture* pTexture = pSprBody->GetTexture();
	const RectF *pRect = pSprBody->GetCurrentTextureCoordRect();
	DISPLAY->SetTexture( pTexture );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetBackfaceCull( false );
	DISPLAY->SetTextureWrapping( true );


	const float fFrameWidth  = pSprBody->GetUnzoomedWidth();
	const float fFrameHeight = pSprBody->GetUnzoomedHeight();
	const float fYBodyTop = fYHead + cache->m_iStartDrawingHoldBodyOffsetFromHead;
	const float fYBodyBottom = fYTail + cache->m_iStopDrawingHoldBodyOffsetFromTail;
	
	// top to bottom
	for( float fY=fYBodyTop; fY<=fYBodyBottom; fY+=fYStep )
	{
		const float fYTop					= fY;
		const float fYOffsetTop				= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fYTop, m_fYReverseOffsetPixels );
		const float fYBottom				= min( fY+fYStep, fYBodyBottom );
		const float fYOffsetBottom			= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fYBottom, m_fYReverseOffsetPixels );
		const float fZTop					= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffsetTop );
		const float fZBottom				= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffsetBottom );
		const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYOffsetTop );
		const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYOffsetBottom );
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
		const float	fAlphaTop				= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffsetTop, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fAlphaBottom			= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffsetBottom, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fGlowTop				= ArrowGetGlow( m_PlayerNumber, iCol, fYOffsetTop, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fGlowBottom				= ArrowGetGlow( m_PlayerNumber, iCol, fYOffsetBottom, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
		const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

		if( bGlow && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bGlow && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		v[0].p = RageVector3(fXTopLeft,    fYTop,   fZTop);		v[0].c = bGlow? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		v[1].p = RageVector3(fXTopRight,   fYTop,   fZTop);		v[1].c = bGlow? colorGlowTop    : colorDiffuseTop;		v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v[2].p = RageVector3(fXBottomRight,fYBottom,fZBottom);	v[2].c = bGlow? colorGlowBottom : colorDiffuseBottom;	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
		v[3].p = RageVector3(fXBottomLeft, fYBottom,fZBottom);	v[3].c = bGlow? colorGlowBottom : colorDiffuseBottom;	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
		v+=4;
		if( v-queue >= size )
			break;
	}	
	DISPLAY->DrawQuads( queue, v-queue );
}

void NoteDisplay::DrawHoldBottomCap( const HoldNote& hn, const bool bActive, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the bottom cap (always wavy)
	//
	const int size = 4096;
	static RageSpriteVertex queue[size];

	Sprite* pBottomCap = GetHoldBottomCapSprite( hn.GetStartBeat(), bActive );

	// draw manually in small segments
	RageSpriteVertex *v = &queue[0];
	RageTexture* pTexture = pBottomCap->GetTexture();
	const RectF *pRect = pBottomCap->GetCurrentTextureCoordRect();
	DISPLAY->SetTexture( pTexture );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetBackfaceCull( false );
	DISPLAY->SetTextureWrapping(false);

	const float fFrameWidth		= pBottomCap->GetUnzoomedWidth();
	const float fFrameHeight	= pBottomCap->GetUnzoomedHeight();
	const float fYCapTop		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail;
	const float fYCapBottom		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail+fFrameHeight;

	// don't draw any part of the tail that is before the middle of the head
	float fY=max( fYCapTop, fYHead );
	for( ; fY<fYCapBottom; fY+=fYStep )	
	{
		const float fYTop					= fY;
		const float fYOffsetTop				= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fYTop, m_fYReverseOffsetPixels );
		const float fStepHeight				= min( fYStep, fFrameHeight );
		const float fYBottom				= min( fY+fStepHeight, fYCapBottom );
		const float fYOffsetBottom			= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fYBottom, m_fYReverseOffsetPixels );
		const float fZTop					= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffsetTop );
		const float fZBottom				= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffsetBottom );
		const float fXTop					= ArrowGetXPos( m_PlayerNumber, iCol, fYOffsetTop );
		const float fXBottom				= ArrowGetXPos( m_PlayerNumber, iCol, fYOffsetBottom );
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
		const float	fAlphaTop				= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffsetTop, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fAlphaBottom			= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffsetBottom, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fGlowTop				= ArrowGetGlow( m_PlayerNumber, iCol, fYOffsetTop, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const float	fGlowBottom				= ArrowGetGlow( m_PlayerNumber, iCol, fYOffsetBottom, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor colorDiffuseTop		= RageColor(fColorScale,fColorScale,fColorScale,fAlphaTop);
		const RageColor colorDiffuseBottom	= RageColor(fColorScale,fColorScale,fColorScale,fAlphaBottom);
		const RageColor colorGlowTop		= RageColor(1,1,1,fGlowTop);
		const RageColor colorGlowBottom		= RageColor(1,1,1,fGlowBottom);

		if( bGlow && colorGlowTop.a==0 && colorGlowBottom.a==0 )
			continue;
		if( !bGlow && colorDiffuseTop.a==0 && colorDiffuseBottom.a==0 )
			continue;

		v[0].p = RageVector3(fXTopLeft,    fYTop,   fZTop);		v[0].c = bGlow ? colorGlowTop    : colorDiffuseTop;		v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
		v[1].p = RageVector3(fXTopRight,   fYTop,   fZTop);		v[1].c = bGlow ? colorGlowTop    : colorDiffuseTop;    	v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v[2].p = RageVector3(fXBottomRight,fYBottom,fZBottom);	v[2].c = bGlow ? colorGlowBottom : colorDiffuseBottom; 	v[2].t = RageVector2(fTexCoordRight, fTexCoordBottom);
		v[3].p = RageVector3(fXBottomLeft, fYBottom,fZBottom);	v[3].c = bGlow ? colorGlowBottom : colorDiffuseBottom; 	v[3].t = RageVector2(fTexCoordLeft,  fTexCoordBottom);
		v+=4;
		if( v-queue >= size )
			break;
	}
	DISPLAY->DrawQuads( queue, v-queue );
}

void NoteDisplay::DrawHoldTail( const HoldNote& hn, bool bActive, float fYTail, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the tail
	//
	Actor* pSprTail = GetHoldTailActor( hn.GetStartBeat(), bActive );

	const float fY				= fYTail;
	const float fYOffset		= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fY, m_fYReverseOffsetPixels );
	const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fYOffset );
	const float fZ				= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffset );
	const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float	fGlow			= ArrowGetGlow( m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	const RageColor colorGlow	= RageColor(1,1,1,fGlow);

	pSprTail->SetXY( fX, fY );
	pSprTail->SetZ( fZ );
	
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

	if( cache->m_bUseLighting )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(0.1f,0.1f,0.1f,1), 
			RageColor(1,1,1,1),
			RageColor(1,1,1,1),
			RageVector3(1, 0, +1) );
	}

	pSprTail->Draw();

	if( cache->m_bUseLighting )
	{
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}
}

void NoteDisplay::DrawHoldHead( const HoldNote& hn, bool bActive, float fYHead, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the head
	//
	Actor* pActor = GetHoldHeadActor( hn.GetStartBeat(), bActive );

	// draw with normal Sprite
	const float fY				= fYHead;
	const float fYOffset		= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fY, m_fYReverseOffsetPixels );
	const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fYOffset );
	const float fZ				= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffset );
	const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float	fGlow			= ArrowGetGlow( m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	const RageColor colorGlow	= RageColor(1,1,1,fGlow);

	pActor->SetXY( fX, fY );
	pActor->SetZ( fZ );

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

	if( cache->m_bUseLighting )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(0.1f,0.1f,0.1f,1), 
			RageColor(1,1,1,1),
			RageColor(1,1,1,1),
			RageVector3(1, 0, +1) );
	}

	pActor->Draw();

	if( cache->m_bUseLighting )
	{
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}
}

void NoteDisplay::DrawHold( const HoldNote& hn, const bool bActive, const float fLife, const float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels )
{
	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	const int	iCol			= hn.iTrack;
	const bool bReverse = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].GetReversePercentForColumn(iCol) > 0.5;
	const float fStartYOffset	= ArrowGetYOffset( m_PlayerNumber, iCol, hn.GetStartBeat() );
	const float fStartYPos		= ArrowGetYPos(	   m_PlayerNumber, iCol, fStartYOffset, fReverseOffsetPixels );
	const float fEndYOffset		= ArrowGetYOffset( m_PlayerNumber, iCol, hn.GetEndBeat() );
	const float fEndYPos		= ArrowGetYPos(	   m_PlayerNumber, iCol, fEndYOffset, fReverseOffsetPixels );

	const float fYHead = bReverse ? fEndYPos : fStartYPos;		// the center of the head
	const float fYTail = bReverse ? fStartYPos : fEndYPos;		// the center the tail

//	const bool  bWavy = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fEffects[PlayerOptions::EFFECT_DRUNK] > 0;
	const bool WavyPartsNeedZBuffer = ArrowsNeedZBuffer( m_PlayerNumber );
	/* Hack: Z effects need a finer grain step. */
	const int	fYStep = WavyPartsNeedZBuffer? 4: 16; //bWavy ? 16 : 128;	// use small steps only if wavy

	const float fColorScale		= 1*fLife + (1-fLife)*cache->m_fHoldNGGrayPercent;

	bool bFlipHeadAndTail = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	/* The body and caps should have no overlap, so their order doesn't matter.
	 * Draw the head last, so it appears on top. */
	if( !cache->m_bHoldHeadIsAboveWavyParts )
		DrawHoldHead( hn, bActive, bFlipHeadAndTail ? fYTail : fYHead, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	if( !cache->m_bHoldTailIsAboveWavyParts )
		DrawHoldTail( hn, bActive, bFlipHeadAndTail ? fYHead : fYTail, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );

	if( bDrawGlowOnly )
		DISPLAY->SetTextureModeGlow();
	else
		DISPLAY->SetTextureModeModulate();
	DISPLAY->SetZBuffer( WavyPartsNeedZBuffer );
	DrawHoldBottomCap( hn, bActive, fYHead, fYTail, fYStep, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	DrawHoldBody( hn, bActive, fYHead, fYTail, fYStep, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	DrawHoldTopCap( hn, bActive, fYHead, fYTail, fYStep, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );

	/* These set the texture mode themselves. */
	if( cache->m_bHoldTailIsAboveWavyParts )
		DrawHoldTail( hn, bActive, bFlipHeadAndTail ? fYHead : fYTail, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	if( cache->m_bHoldHeadIsAboveWavyParts )
		DrawHoldHead( hn, bActive, bFlipHeadAndTail ? fYTail : fYHead, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );

	// now, draw the glow pass
	if( !bDrawGlowOnly )
		DrawHold( hn, bActive, fLife, fPercentFadeToFail, true, fReverseOffsetPixels );
}

void NoteDisplay::DrawActor( Actor* pActor, int iCol, float fBeat, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels )
{
	const float fYOffset		= ArrowGetYOffset(	m_PlayerNumber, iCol, fBeat );
	const float fYPos			= ArrowGetYPos(	m_PlayerNumber, iCol, fYOffset, fReverseOffsetPixels );
	const float fRotation		= ArrowGetRotation(	m_PlayerNumber, fBeat );
	const float fXPos			= ArrowGetXPos(		m_PlayerNumber, iCol, fYOffset );
	const float fZPos			= ArrowGetZPos(	   m_PlayerNumber, iCol, fYOffset );
	const float fAlpha			= ArrowGetAlpha(	m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float fGlow			= ArrowGetGlow(		m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float fColorScale		= ArrowGetBrightness( m_PlayerNumber, fBeat ) * SCALE(fLife,0,1,0.2f,1);
	RageColor diffuse = RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	RageColor glow = RageColor(1,1,1,fGlow);

	pActor->SetRotationZ( fRotation );
	pActor->SetXY( fXPos, fYPos );
	pActor->SetZ( fZPos );
	pActor->SetDiffuse( diffuse );
	pActor->SetGlow( glow );

	if( cache->m_bUseLighting )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(0.1f,0.1f,0.1f,1), 
			RageColor(1,1,1,1),
			RageColor(1,1,1,1),
			RageVector3(1, 0, +1) );
	}

	pActor->Draw();

	if( cache->m_bUseLighting )
	{
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}
}

void NoteDisplay::DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels )
{
	Actor* pActor = NULL;
	if( bIsMine )
		pActor = GetTapMineActor( fBeat );
	else if( bIsAddition )
		pActor = GetTapAdditionActor( fBeat );
	else if( bOnSameRowAsHoldStart  &&  cache->m_bDrawHoldHeadForTapsOnSameRow )
		pActor = GetHoldHeadActor( fBeat, false );
	else	
		pActor = GetTapNoteActor( fBeat );

	DrawActor( pActor, iCol, fBeat, fPercentFadeToFail, fLife, fReverseOffsetPixels );
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
