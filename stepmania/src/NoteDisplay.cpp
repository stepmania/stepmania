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
#include "RageDisplay.h"
#include "NoteTypes.h"
#include "NoteFieldPositioning.h"
#include "ActorUtil.h"
#include "NoteDataWithScoring.h"

enum part
{
	PART_TAP,
	PART_ADDITION,
	PART_MINE,
	PART_HOLD_HEAD,
	PART_HOLD_TAIL,
	PART_HOLD_TOP_CAP,
	PART_HOLD_BODY,
	PART_HOLD_BOTTOM_CAP,
	NUM_PARTS
};


// cache
struct NoteMetricCache_t
{
	bool m_bDrawHoldHeadForTapsOnSameRow;
	float m_fAnimationLengthInBeats[NUM_PARTS];
	bool m_bAnimationIsVivid[NUM_PARTS];
	bool m_bAnimationIsNoteColor[NUM_PARTS];

	bool m_bHoldHeadIsAboveWavyParts;
	bool m_bHoldTailIsAboveWavyParts;
	int m_iStartDrawingHoldBodyOffsetFromHead;
	int m_iStopDrawingHoldBodyOffsetFromTail;
	float m_fHoldNGGrayPercent;
	bool m_bTapNoteUseLighting;
	bool m_bTapAdditionUseLighting;
	bool m_bTapMineUseLighting;
	bool m_bHoldHeadUseLighting;
	bool m_bHoldTailUseLighting;
	bool m_bFlipHeadAndTailWhenReverse;

	void Load(CString skin, const CString &name);
} *NoteMetricCache;

void NoteMetricCache_t::Load(CString skin, const CString &name)
{
	m_bDrawHoldHeadForTapsOnSameRow =			NOTESKIN->GetMetricB(skin,name,"DrawHoldHeadForTapsOnSameRow");
	m_fAnimationLengthInBeats[PART_TAP] =		NOTESKIN->GetMetricF(skin,name,"TapNoteAnimationLengthInBeats");
	m_fAnimationLengthInBeats[PART_ADDITION] =	NOTESKIN->GetMetricF(skin,name,"TapAdditionAnimationLengthInBeats");
	m_fAnimationLengthInBeats[PART_MINE] =		NOTESKIN->GetMetricF(skin,name,"TapMineAnimationLengthInBeats");
	m_fAnimationLengthInBeats[PART_HOLD_HEAD] =	NOTESKIN->GetMetricF(skin,name,"HoldHeadAnimationLengthInBeats");
	m_fAnimationLengthInBeats[PART_HOLD_TOP_CAP] =	NOTESKIN->GetMetricF(skin,name,"HoldTopCapAnimationLengthInBeats");
	m_fAnimationLengthInBeats[PART_HOLD_BODY] =	NOTESKIN->GetMetricF(skin,name,"HoldBodyAnimationLengthInBeats");
	m_fAnimationLengthInBeats[PART_HOLD_BOTTOM_CAP] =	NOTESKIN->GetMetricF(skin,name,"HoldBottomCapAnimationLengthInBeats");
	m_fAnimationLengthInBeats[PART_HOLD_TAIL] =	NOTESKIN->GetMetricF(skin,name,"HoldTailAnimationLengthInBeats");
	m_bAnimationIsVivid[PART_TAP] =				NOTESKIN->GetMetricB(skin,name,"TapNoteAnimationIsVivid");
	m_bAnimationIsVivid[PART_ADDITION] =		NOTESKIN->GetMetricB(skin,name,"TapAdditionAnimationIsVivid");
	m_bAnimationIsVivid[PART_MINE] =			NOTESKIN->GetMetricB(skin,name,"TapMineAnimationIsVivid");
	m_bAnimationIsVivid[PART_HOLD_HEAD] =		NOTESKIN->GetMetricB(skin,name,"HoldHeadAnimationIsVivid");
	m_bAnimationIsVivid[PART_HOLD_TOP_CAP] =	NOTESKIN->GetMetricB(skin,name,"HoldTopCapAnimationIsVivid");
	m_bAnimationIsVivid[PART_HOLD_BODY] =		NOTESKIN->GetMetricB(skin,name,"HoldBodyAnimationIsVivid");
	m_bAnimationIsVivid[PART_HOLD_BOTTOM_CAP] =	NOTESKIN->GetMetricB(skin,name,"HoldBottomCapAnimationIsVivid");
	m_bAnimationIsVivid[PART_HOLD_TAIL] =		NOTESKIN->GetMetricB(skin,name,"HoldTailAnimationIsVivid");

	m_bAnimationIsNoteColor[PART_TAP] =			NOTESKIN->GetMetricB(skin,name,"TapNoteAnimationIsNoteColor");
	m_bAnimationIsNoteColor[PART_ADDITION] =	NOTESKIN->GetMetricB(skin,name,"TapAdditionAnimationIsNoteColor");
	// m_bAnimationIsNoteColor[PART_MINE] =	NOTESKIN->GetMetricB(skin,name,"TapMineAnimationIsNoteColor");
	m_bAnimationIsNoteColor[PART_HOLD_HEAD] =	NOTESKIN->GetMetricB(skin,name,"HoldHeadAnimationIsNoteColor");
	m_bAnimationIsNoteColor[PART_HOLD_TOP_CAP] =NOTESKIN->GetMetricB(skin,name,"HoldTopCapAnimationIsNoteColor");
	m_bAnimationIsNoteColor[PART_HOLD_BODY] =	NOTESKIN->GetMetricB(skin,name,"HoldBodyAnimationIsNoteColor");
	m_bAnimationIsNoteColor[PART_HOLD_BOTTOM_CAP] =	NOTESKIN->GetMetricB(skin,name,"HoldBottomCapAnimationIsNoteColor");
	m_bAnimationIsNoteColor[PART_HOLD_TAIL] =	NOTESKIN->GetMetricB(skin,name,"HoldTailAnimationIsNoteColor");

	m_bHoldHeadIsAboveWavyParts =				NOTESKIN->GetMetricB(skin,name,"HoldHeadIsAboveWavyParts");
	m_bHoldTailIsAboveWavyParts =				NOTESKIN->GetMetricB(skin,name,"HoldTailIsAboveWavyParts");
	m_iStartDrawingHoldBodyOffsetFromHead =		NOTESKIN->GetMetricI(skin,name,"StartDrawingHoldBodyOffsetFromHead");
	m_iStopDrawingHoldBodyOffsetFromTail =		NOTESKIN->GetMetricI(skin,name,"StopDrawingHoldBodyOffsetFromTail");
	m_fHoldNGGrayPercent =						NOTESKIN->GetMetricF(skin,name,"HoldNGGrayPercent");
	m_bTapNoteUseLighting =						NOTESKIN->GetMetricB(skin,name,"TapNoteUseLighting");
	m_bTapAdditionUseLighting =					NOTESKIN->GetMetricB(skin,name,"TapAdditionUseLighting");
	m_bTapMineUseLighting =						NOTESKIN->GetMetricB(skin,name,"TapMineUseLighting");
	m_bHoldHeadUseLighting =					NOTESKIN->GetMetricB(skin,name,"HoldHeadUseLighting");
	m_bHoldTailUseLighting =					NOTESKIN->GetMetricB(skin,name,"HoldTailUseLighting");
	m_bFlipHeadAndTailWhenReverse =				NOTESKIN->GetMetricB(skin,name,"FlipHeadAndTailWhenReverse");
}

NoteDisplay::NoteDisplay()
{
	for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
	{
		m_pTapNote[i] = NULL;
		m_pTapAddition[i] = NULL;
		m_pHoldHeadActive[i] = NULL;
		m_pHoldHeadInactive[i] = NULL;
		m_pHoldTailActive[i] = NULL;
		m_pHoldTailInactive[i] = NULL;
	}
	m_pTapMine = NULL;

	cache = new NoteMetricCache_t;
}

NoteDisplay::~NoteDisplay()
{
	for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
	{
		delete m_pTapNote[i];
		delete m_pTapAddition[i];
		delete m_pHoldHeadActive[i];
		delete m_pHoldHeadInactive[i];
		delete m_pHoldTailActive[i];
		delete m_pHoldTailInactive[i];
	}
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


	if( cache->m_bAnimationIsNoteColor[PART_TAP] )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
			m_pTapNote[i] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "tap note "+sNoteType[i]) );
	}
	else
	{
		m_pTapNote[0] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "tap note") );
	}

	if( cache->m_bAnimationIsNoteColor[PART_ADDITION] )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
			m_pTapAddition[i] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "tap addition "+sNoteType[i]) );
	}
	else
	{
		m_pTapAddition[0] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "tap addition") );
	}

	m_pTapMine = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "tap mine") );

	if( cache->m_bAnimationIsNoteColor[PART_HOLD_HEAD] )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_pHoldHeadActive[i] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold head active "+sNoteType[i]) );
			m_pHoldHeadInactive[i] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold head inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_pHoldHeadActive[0] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold head active") );
		m_pHoldHeadInactive[0] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold head inactive") );
	}

	if( cache->m_bAnimationIsNoteColor[PART_HOLD_TOP_CAP] )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldTopCapActive[i].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold topcap active "+sNoteType[i]) );
			m_sprHoldTopCapInactive[i].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold topcap inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldTopCapActive[0].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold topcap active") );
		m_sprHoldTopCapInactive[0].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold topcap inactive") );
	}

	if( cache->m_bAnimationIsNoteColor[PART_HOLD_BODY] )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldBodyActive[i].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold body active "+sNoteType[i]) );
			m_sprHoldBodyInactive[i].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold body inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldBodyActive[0].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold body active") );
		m_sprHoldBodyInactive[0].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold body inactive") );
	}

	if( cache->m_bAnimationIsNoteColor[PART_HOLD_BOTTOM_CAP] )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_sprHoldBottomCapActive[i].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold bottomcap active "+sNoteType[i]) );
			m_sprHoldBottomCapInactive[i].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold bottomcap inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_sprHoldBottomCapActive[0].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold bottomcap active") );
		m_sprHoldBottomCapInactive[0].Load( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold bottomcap inactive") );
	}

	if( cache->m_bAnimationIsNoteColor[PART_HOLD_TAIL] )
	{
		for( int i=0; i<NOTE_COLOR_IMAGES; i++ )
		{
			m_pHoldTailActive[i] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold tail active "+sNoteType[i]) );
			m_pHoldTailInactive[i] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold tail inactive "+sNoteType[i]) );
		}
	}
	else
	{
		m_pHoldTailActive[0] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold tail active") );
		m_pHoldTailInactive[0] = MakeActor( NOTESKIN->GetPathToFromNoteSkinAndButton(NoteSkin, Button, "hold tail inactive") );
	}
}

void NoteDisplay::Update( float fDeltaTime )
{
	int i;

	for( i=0; i<NOTE_COLOR_IMAGES; i++ )	if(m_pTapNote[i])			m_pTapNote[i]->Update(fDeltaTime);
	for( i=0; i<NOTE_COLOR_IMAGES; i++ )	if(m_pTapAddition[i])		m_pTapAddition[i]->Update(fDeltaTime);
	m_pTapMine->Update(fDeltaTime);
	for( i=0; i<NOTE_COLOR_IMAGES; i++ )	if(m_pHoldHeadActive[i])	m_pHoldHeadActive[i]->Update(fDeltaTime);
	for( i=0; i<NOTE_COLOR_IMAGES; i++ )	if(m_pHoldHeadInactive[i])	m_pHoldHeadInactive[i]->Update(fDeltaTime);
	for( i=0; i<NOTE_COLOR_IMAGES; i++ )	if(m_pHoldTailActive[i])	m_pHoldTailActive[i]->Update(fDeltaTime);
	for( i=0; i<NOTE_COLOR_IMAGES; i++ )	if(m_pHoldTailInactive[i])	m_pHoldTailInactive[i]->Update(fDeltaTime);
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
	{
		// changed to deal with the minor complaint that the color cycling is
		// one tick off in general
		const float fFraction = fNoteBeatFraction - 0.25f/fAnimationLengthInBeats;
		const float fInterval = 1.f / fAnimationLengthInBeats;
		iFrameNo += int( froundf(fFraction,fInterval)*iNumFrames );
	}

	// just in case somehow we're majorly negative with the subtraction
	iFrameNo += (iNumFrames * 2);
	iFrameNo %= iNumFrames;

	ASSERT( iFrameNo>=0 && iFrameNo<iNumFrames );

	actorToSet.SetState( iFrameNo );
}

Actor * NoteDisplay::GetTapNoteActor( float fNoteBeat )
{
	NoteType nt = NoteType(0);
	if( cache->m_bAnimationIsNoteColor[PART_TAP] )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Actor *pActorOut = m_pTapNote[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pActorOut,
		cache->m_fAnimationLengthInBeats[PART_TAP],
		cache->m_bAnimationIsVivid[PART_TAP],
		cache->m_bAnimationIsNoteColor[PART_TAP] );

	return pActorOut;
}

Actor * NoteDisplay::GetTapAdditionActor( float fNoteBeat )
{
	NoteType nt = NoteType(0);
	if( cache->m_bAnimationIsNoteColor[PART_ADDITION] )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Actor *pActorOut = m_pTapAddition[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pActorOut,
		cache->m_fAnimationLengthInBeats[PART_ADDITION],
		cache->m_bAnimationIsVivid[PART_ADDITION],
		cache->m_bAnimationIsNoteColor[PART_ADDITION] );

	return pActorOut;
}

Actor * NoteDisplay::GetTapMineActor( float fNoteBeat )
{
	SetActiveFrame( 
		fNoteBeat, 
		*m_pTapMine,
		cache->m_fAnimationLengthInBeats[PART_MINE], 
		cache->m_bAnimationIsVivid[PART_MINE], 
		false );

	return m_pTapMine;
}

Sprite * NoteDisplay::GetHoldTopCapSprite( float fNoteBeat, bool bIsBeingHeld )
{
	NoteType nt = NoteType(0);
	if( cache->m_bAnimationIsNoteColor[PART_HOLD_TOP_CAP] )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );
	
	Sprite *pSpriteOut = bIsBeingHeld ? &m_sprHoldTopCapActive[nt] : &m_sprHoldTopCapInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fAnimationLengthInBeats[PART_HOLD_TOP_CAP],
		cache->m_bAnimationIsVivid[PART_HOLD_TOP_CAP],
		cache->m_bAnimationIsNoteColor[PART_HOLD_TOP_CAP] );

	return pSpriteOut;
}

Sprite * NoteDisplay::GetHoldBottomCapSprite( float fNoteBeat, bool bIsBeingHeld )
{
	NoteType nt = NoteType(0);
	if( cache->m_bAnimationIsNoteColor[PART_HOLD_BOTTOM_CAP] )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );
	
	Sprite *pSpriteOut = bIsBeingHeld ? &m_sprHoldBottomCapActive[nt] : &m_sprHoldBottomCapInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fAnimationLengthInBeats[PART_HOLD_BOTTOM_CAP],
		cache->m_bAnimationIsVivid[PART_HOLD_BOTTOM_CAP],
		cache->m_bAnimationIsNoteColor[PART_HOLD_BOTTOM_CAP] );

	return pSpriteOut;
}


Actor* NoteDisplay::GetHoldHeadActor( float fNoteBeat, bool bIsBeingHeld )
{
	NoteType nt = NoteType(0);
	if( cache->m_bAnimationIsNoteColor[PART_HOLD_HEAD] )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Actor *pActorOut = bIsBeingHeld ? m_pHoldHeadActive[nt] : m_pHoldHeadInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pActorOut, 
		cache->m_fAnimationLengthInBeats[PART_HOLD_HEAD],
		cache->m_bAnimationIsVivid[PART_HOLD_HEAD],
		cache->m_bAnimationIsNoteColor[PART_HOLD_HEAD] );

	return pActorOut;
}

Sprite *NoteDisplay::GetHoldBodySprite( float fNoteBeat, bool bIsBeingHeld )
{
	NoteType nt = NoteType(0);
	if( cache->m_bAnimationIsNoteColor[PART_HOLD_BODY] )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Sprite *pSpriteOut = bIsBeingHeld ? &m_sprHoldBodyActive[nt] : &m_sprHoldBodyInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pSpriteOut, 
		cache->m_fAnimationLengthInBeats[PART_HOLD_BODY],
		cache->m_bAnimationIsVivid[PART_HOLD_BODY],
		cache->m_bAnimationIsNoteColor[PART_HOLD_BODY] );

	return pSpriteOut;
}

Actor* NoteDisplay::GetHoldTailActor( float fNoteBeat, bool bIsBeingHeld )
{
	NoteType nt = NoteType(0);
	if( cache->m_bAnimationIsNoteColor[PART_HOLD_TAIL] )
		nt = BeatToNoteType( fNoteBeat );
//  NOTE_TYPE_INVALID is 192nds at this point.
	if( nt == NOTE_TYPE_INVALID )
		nt = NOTE_TYPE_192ND;
	nt = min( nt, (NoteType) (NOTE_COLOR_IMAGES-1) );

	Actor *pActorOut = bIsBeingHeld ? m_pHoldTailActive[nt] : m_pHoldTailInactive[nt];

	SetActiveFrame( 
		fNoteBeat, 
		*pActorOut, 
		cache->m_fAnimationLengthInBeats[PART_HOLD_TAIL], 
		cache->m_bAnimationIsVivid[PART_HOLD_TAIL], 
		cache->m_bAnimationIsNoteColor[PART_HOLD_TAIL] );

	return pActorOut;
}

static float ArrowGetAlphaOrGlow( bool bGlow, PlayerNumber pn, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels )
{
	if( bGlow )
		return ArrowGetGlow( pn, iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels );
	else
		return ArrowGetAlpha( pn, iCol, fYOffset, fPercentFadeToFail, fYReverseOffsetPixels );
}

void NoteDisplay::DrawHoldTopCap( const HoldNote& hn, const bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the top cap (always wavy)
	//
	const int size = 4096;
	static RageSpriteVertex queue[size];
	Sprite* pSprTopCap = GetHoldTopCapSprite( hn.GetStartBeat(), bIsBeingHeld );

	// draw manually in small segments
	RageSpriteVertex *v = &queue[0];
	RageTexture* pTexture = pSprTopCap->GetTexture();
	const RectF *pRect = pSprTopCap->GetCurrentTextureCoordRect();
	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( 0, pTexture );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetCullMode( CULL_NONE );
	DISPLAY->SetTextureWrapping(false);

	const float fFrameWidth  = pSprTopCap->GetUnzoomedWidth();
	const float fFrameHeight = pSprTopCap->GetUnzoomedHeight();
	const float fYCapTop	 = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead-fFrameHeight;
	const float fYCapBottom  = fYHead+cache->m_iStartDrawingHoldBodyOffsetFromHead;

	if( bGlow )
		fColorScale = 1;

	bool bAllAreTransparent = true;
	bool bLast = false;
	// don't draw any part of the head that is after the middle of the tail
	float fY = fYCapTop;
	float fYStop = min(fYTail,fYCapBottom);
	for( ; !bLast; fY+=fYStep )
	{
		if( fY >= fYStop )
		{
			fY = fYStop;
			bLast = true;
		}

		const float fYOffset				= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ						= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffset );
		const float fX						= ArrowGetXPos( m_PlayerNumber, iCol, fYOffset );
		const float fXLeft					= fX - fFrameWidth/2;
		const float fXRight					= fX + fFrameWidth/2;
		const float fTopDistFromHeadTop		= fY - fYCapTop;
		const float fTexCoordTop			= SCALE( fTopDistFromHeadTop,    0, fFrameHeight, pRect->top, pRect->bottom );
		const float fTexCoordLeft			= pRect->left;
		const float fTexCoordRight			= pRect->right;
		const float	fAlpha					= ArrowGetAlphaOrGlow( bGlow, m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor color				= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		v[0].p = RageVector3(fXLeft,  fY, fZ); v[0].c = color; v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
		v[1].p = RageVector3(fXRight, fY, fZ); v[1].c = color; v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v+=2;
		if( v-queue >= size )
			break;
	}
	if( !bAllAreTransparent )
		DISPLAY->DrawQuadStrip( queue, v-queue );
}

void NoteDisplay::DrawHoldBody( const HoldNote& hn, const bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the body (always wavy)
	//
	const int size = 4096;
	static RageSpriteVertex queue[size];

	Sprite* pSprBody = GetHoldBodySprite( hn.GetStartBeat(), bIsBeingHeld );

	// draw manually in small segments
	RageSpriteVertex *v = &queue[0];
	RageTexture* pTexture = pSprBody->GetTexture();
	const RectF *pRect = pSprBody->GetCurrentTextureCoordRect();
	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( 0, pTexture );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetCullMode( CULL_NONE );
	DISPLAY->SetTextureWrapping( true );


	const float fFrameWidth  = pSprBody->GetUnzoomedWidth();
	const float fFrameHeight = pSprBody->GetUnzoomedHeight();
	const float fYBodyTop = fYHead + cache->m_iStartDrawingHoldBodyOffsetFromHead;
	const float fYBodyBottom = fYTail + cache->m_iStopDrawingHoldBodyOffsetFromTail;
	
	const bool bReverse = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].GetReversePercentForColumn(iCol) > 0.5;
	bool bAnchorToBottom = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	if( bGlow )
		fColorScale = 1;

	// top to bottom
	bool bAllAreTransparent = true;
	bool bLast = false;
	for( float fY = fYBodyTop; !bLast; fY += fYStep )
	{
		if( fY >= fYBodyBottom )
		{
			fY = fYBodyBottom;
			bLast = true;
		}

		const float fYOffset			= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ					= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffset );
		const float fX					= ArrowGetXPos( m_PlayerNumber, iCol, fYOffset );
		const float fXLeft				= fX - fFrameWidth/2;
		const float fXRight				= fX + fFrameWidth/2;
		const float fDistFromBodyBottom	= fYBodyBottom - fY;
		const float fDistFromBodyTop	= fY - fYBodyTop;
		const float fTexCoordTop		= SCALE( bAnchorToBottom ? fDistFromBodyTop : fDistFromBodyBottom,    0, fFrameHeight, pRect->bottom, pRect->top );
		const float fTexCoordLeft		= pRect->left;
		const float fTexCoordRight		= pRect->right;
		const float	fAlpha				= ArrowGetAlphaOrGlow( bGlow, m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor color			= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		v[0].p = RageVector3(fXLeft,  fY, fZ);	v[0].c = color; v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop);
		v[1].p = RageVector3(fXRight, fY, fZ);	v[1].c = color; v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v+=2;
		if( v-queue >= size )
			break;
	}

	if( !bAllAreTransparent )
		DISPLAY->DrawQuadStrip( queue, v-queue );
}

void NoteDisplay::DrawHoldBottomCap( const HoldNote& hn, const bool bIsBeingHeld, float fYHead, float fYTail, int fYStep, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the bottom cap (always wavy)
	//
	const int size = 4096;
	static RageSpriteVertex queue[size];

	Sprite* pBottomCap = GetHoldBottomCapSprite( hn.GetStartBeat(), bIsBeingHeld );

	// draw manually in small segments
	RageSpriteVertex *v = &queue[0];
	RageTexture* pTexture = pBottomCap->GetTexture();
	const RectF *pRect = pBottomCap->GetCurrentTextureCoordRect();
	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( 0, pTexture );
	DISPLAY->SetBlendMode( BLEND_NORMAL );
	DISPLAY->SetCullMode( CULL_NONE );
	DISPLAY->SetTextureWrapping(false);

	const float fFrameWidth		= pBottomCap->GetUnzoomedWidth();
	const float fFrameHeight	= pBottomCap->GetUnzoomedHeight();
	const float fYCapTop		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail;
	const float fYCapBottom		= fYTail+cache->m_iStopDrawingHoldBodyOffsetFromTail+fFrameHeight;

	if( bGlow )
		fColorScale = 1;

	bool bAllAreTransparent = true;
	bool bLast = false;
	// don't draw any part of the tail that is before the middle of the head
	float fY=max( fYCapTop, fYHead );
	for( ; !bLast; fY += fYStep )
	{
		if( fY >= fYCapBottom )
		{
			fY = fYCapBottom;
			bLast = true;
		}

		const float fYOffset				= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fY, m_fYReverseOffsetPixels );
		const float fZ						= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffset );
		const float fX						= ArrowGetXPos( m_PlayerNumber, iCol, fYOffset );
		const float fXLeft					= fX - fFrameWidth/2;
		const float fXRight					= fX + fFrameWidth/2;
		const float fTopDistFromTail		= fY - fYCapTop;
		const float fTexCoordTop			= SCALE( fTopDistFromTail,    0, fFrameHeight, pRect->top, pRect->bottom );
		const float fTexCoordLeft			= pRect->left;
		const float fTexCoordRight			= pRect->right;
		const float	fAlpha					= ArrowGetAlphaOrGlow( bGlow, m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
		const RageColor color				= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);

		if( fAlpha > 0 )
			bAllAreTransparent = false;

		v[0].p = RageVector3(fXLeft,  fY, fZ);	v[0].c = color; v[0].t = RageVector2(fTexCoordLeft,  fTexCoordTop),
		v[1].p = RageVector3(fXRight, fY, fZ);	v[1].c = color; v[1].t = RageVector2(fTexCoordRight, fTexCoordTop);
		v+=2;
		if( v-queue >= size )
			break;
	}
	if( !bAllAreTransparent )
		DISPLAY->DrawQuadStrip( queue, v-queue );
}

void NoteDisplay::DrawHoldTail( const HoldNote& hn, bool bIsBeingHeld, float fYTail, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the tail
	//
	Actor* pSprTail = GetHoldTailActor( hn.GetStartBeat(), bIsBeingHeld );

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
}

void NoteDisplay::DrawHoldHead( const HoldNote& hn, bool bIsBeingHeld, float fYHead, int iCol, float fPercentFadeToFail, float fColorScale, bool bGlow )
{
	//
	// Draw the head
	//
	Actor* pActor = GetHoldHeadActor( hn.GetStartBeat(), bIsBeingHeld );

	// draw with normal Sprite
	const float fY				= fYHead;
	const float fYOffset		= ArrowGetYOffsetFromYPos( m_PlayerNumber, iCol, fY, m_fYReverseOffsetPixels );
	const float fX				= ArrowGetXPos( m_PlayerNumber, iCol, fYOffset );
	const float fZ				= ArrowGetZPos(	m_PlayerNumber, iCol, fYOffset );
	const float	fAlpha			= ArrowGetAlpha( m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const float	fGlow			= ArrowGetGlow( m_PlayerNumber, iCol, fYOffset, fPercentFadeToFail, m_fYReverseOffsetPixels );
	const RageColor colorDiffuse= RageColor(fColorScale,fColorScale,fColorScale,fAlpha);
	const RageColor colorGlow	= RageColor(1,1,1,fGlow);

	pActor->SetRotationZ( 0 );
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
}

void NoteDisplay::DrawHold( const HoldNote& hn, bool bIsBeingHeld, bool bIsActive, const HoldNoteResult &Result, float fPercentFadeToFail, bool bDrawGlowOnly, float fReverseOffsetPixels )
{
	// bDrawGlowOnly is a little hacky.  We need to draw the diffuse part and the glow part one pass at a time to minimize state changes

	int	iCol			= hn.iTrack;
	bool bReverse = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].GetReversePercentForColumn(iCol) > 0.5;
	float fStartYOffset	= ArrowGetYOffset( m_PlayerNumber, iCol, Result.GetLastHeldBeat() );
	
	// HACK: If active, don't allow the top of the hold to go above the receptor
	if( bIsActive )
		fStartYOffset = 0;

	float fStartYPos		= ArrowGetYPos(	   m_PlayerNumber, iCol, fStartYOffset, fReverseOffsetPixels );
	float fEndYOffset		= ArrowGetYOffset( m_PlayerNumber, iCol, hn.GetEndBeat() );
	float fEndYPos		= ArrowGetYPos(	   m_PlayerNumber, iCol, fEndYOffset, fReverseOffsetPixels );

	const float fYHead = bReverse ? fEndYPos : fStartYPos;		// the center of the head
	const float fYTail = bReverse ? fStartYPos : fEndYPos;		// the center the tail

//	const bool  bWavy = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fEffects[PlayerOptions::EFFECT_DRUNK] > 0;
	const bool WavyPartsNeedZBuffer = ArrowsNeedZBuffer( m_PlayerNumber );
	/* Hack: Z effects need a finer grain step. */
	const int	fYStep = WavyPartsNeedZBuffer? 4: 16; //bWavy ? 16 : 128;	// use small steps only if wavy

	const float fColorScale		= 1*Result.fLife + (1-Result.fLife)*cache->m_fHoldNGGrayPercent;

	bool bFlipHeadAndTail = bReverse && cache->m_bFlipHeadAndTailWhenReverse;

	/* The body and caps should have no overlap, so their order doesn't matter.
	 * Draw the head last, so it appears on top. */
	if( !cache->m_bHoldHeadIsAboveWavyParts )
		DrawHoldHead( hn, bIsBeingHeld, bFlipHeadAndTail ? fYTail : fYHead, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	if( !cache->m_bHoldTailIsAboveWavyParts )
		DrawHoldTail( hn, bIsBeingHeld, bFlipHeadAndTail ? fYHead : fYTail, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );

	if( bDrawGlowOnly )
		DISPLAY->SetTextureModeGlow();
	else
		DISPLAY->SetTextureModeModulate();
	DISPLAY->SetZBuffer( WavyPartsNeedZBuffer );
	
	if( !bFlipHeadAndTail )
		DrawHoldBottomCap( hn, bIsBeingHeld, fYHead, fYTail, fYStep, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	DrawHoldBody( hn, bIsBeingHeld, fYHead, fYTail, fYStep, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	if( bFlipHeadAndTail )
		DrawHoldTopCap( hn, bIsBeingHeld, fYHead, fYTail, fYStep, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );

	/* These set the texture mode themselves. */
	if( cache->m_bHoldTailIsAboveWavyParts )
		DrawHoldTail( hn, bIsBeingHeld, bFlipHeadAndTail ? fYHead : fYTail, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );
	if( cache->m_bHoldHeadIsAboveWavyParts )
		DrawHoldHead( hn, bIsBeingHeld, bFlipHeadAndTail ? fYTail : fYHead, iCol, fPercentFadeToFail, fColorScale, bDrawGlowOnly );

	// now, draw the glow pass
	if( !bDrawGlowOnly )
		DrawHold( hn, bIsBeingHeld, bIsActive, Result, fPercentFadeToFail, true, fReverseOffsetPixels );
}

void NoteDisplay::DrawActor( Actor* pActor, int iCol, float fBeat, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels, bool bUseLighting )
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
}

void NoteDisplay::DrawTap( int iCol, float fBeat, bool bOnSameRowAsHoldStart, bool bIsAddition, bool bIsMine, float fPercentFadeToFail, float fLife, float fReverseOffsetPixels )
{
	Actor* pActor = NULL;
	bool bUseLighting = false;
	if( bIsMine )
	{
		pActor = GetTapMineActor( fBeat );
		bUseLighting = cache->m_bTapMineUseLighting;
	}
	else if( bIsAddition )
	{
		pActor = GetTapAdditionActor( fBeat );
		bUseLighting = cache->m_bTapAdditionUseLighting;
	}
	else if( bOnSameRowAsHoldStart  &&  cache->m_bDrawHoldHeadForTapsOnSameRow )
	{
		pActor = GetHoldHeadActor( fBeat, false );
		bUseLighting = cache->m_bHoldHeadUseLighting;
	}
	else	
	{
		pActor = GetTapNoteActor( fBeat );
		bUseLighting = cache->m_bTapNoteUseLighting;
	}

	DrawActor( pActor, iCol, fBeat, fPercentFadeToFail, fLife, fReverseOffsetPixels, bUseLighting );
}
