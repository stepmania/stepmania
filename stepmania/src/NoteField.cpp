#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: NoteField

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteField.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ArrowEffects.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageMath.h"
#include <math.h>
#include "ThemeManager.h"
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"
#include "song.h"

NoteField::NoteField()
{	
	m_textMeasureNumber.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textMeasureNumber.SetZoom( 1.0f );

	m_rectMarkerBar.EnableShadow( false );
	m_rectMarkerBar.SetEffectDiffuseShift( 2, RageColor(1,1,1,0.5f), RageColor(0.5f,0.5f,0.5f,0.5f) );

	m_sprBars.Load( THEME->GetPathToG("NoteField bars") );
	m_sprBars.StopAnimating();

	m_fBeginMarker = m_fEndMarker = -1;

	m_fPercentFadeToFail = -1;
}

NoteField::~NoteField()
{
	Unload();
}

void NoteField::Unload()
{
	for( map<CString, NoteDisplayCols *>::iterator it = m_NoteDisplays.begin();
		it != m_NoteDisplays.end(); ++it )
		delete it->second;
	m_NoteDisplays.clear();
}

void NoteField::CacheNoteSkin( CString skin )
{
	if( m_NoteDisplays.find(skin) != m_NoteDisplays.end() )
		return;

	LOG->Trace("NoteField::CacheNoteSkin: cache %s", skin.c_str() );
	NoteDisplayCols *nd = new NoteDisplayCols;
	for( int c=0; c<GetNumTracks(); c++ ) 
		nd->display[c].Load( c, m_PlayerNumber, skin, m_fYReverseOffsetPixels );
	m_NoteDisplays[ skin ] = nd;
}

void NoteField::CacheAllUsedNoteSkins()
{
	/* Cache note skins. */
	vector<CString> skins;
	GAMESTATE->GetAllUsedNoteSkins( skins );
	for( unsigned i=0; i < skins.size(); ++i )
		CacheNoteSkin( skins[i] );
}

void NoteField::Load( const NoteData* pNoteData, PlayerNumber pn, int iFirstPixelToDraw, int iLastPixelToDraw, float fYReverseOffsetPixels )
{
	Unload();

	m_PlayerNumber = pn;
	m_iStartDrawingPixel = iFirstPixelToDraw;
	m_iEndDrawingPixel = iLastPixelToDraw;
	m_fYReverseOffsetPixels = fYReverseOffsetPixels;

	m_fPercentFadeToFail = -1;
	m_LastSeenBeatToNoteSkinRev = -1;

	NoteDataWithScoring::Init();

	m_bIsHoldingHoldNote.clear();
	m_bIsHoldingHoldNote.insert(m_bIsHoldingHoldNote.end(), pNoteData->GetNumTapNotes(), false);

	this->CopyAll( pNoteData );
	ASSERT( GetNumTracks() == GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer );

	CacheAllUsedNoteSkins();
	RefreshBeatToNoteSkin();
}


void NoteField::RefreshBeatToNoteSkin()
{
	if( GAMESTATE->m_BeatToNoteSkinRev == m_LastSeenBeatToNoteSkinRev )
		return;
	m_LastSeenBeatToNoteSkinRev = GAMESTATE->m_BeatToNoteSkinRev;

	/* Set by GameState::ResetNoteSkins(): */
	ASSERT( !GAMESTATE->m_BeatToNoteSkin[m_PlayerNumber].empty() );

	m_BeatToNoteDisplays.clear();

	/* GAMESTATE->m_BeatToNoteSkin[pn] maps from song beats to note skins.  Maintain
	 * m_BeatToNoteDisplays, to map from song beats to NoteDisplay*s, so we don't
	 * have to do it while rendering. */
	map<float,CString>::iterator it;
	for( it = GAMESTATE->m_BeatToNoteSkin[m_PlayerNumber].begin(); 
		 it != GAMESTATE->m_BeatToNoteSkin[m_PlayerNumber].end(); ++it )
	{
		const float Beat = it->first;
		const CString &Skin = it->second;

		map<CString, NoteDisplayCols *>::iterator display = m_NoteDisplays.find( Skin );
		if( display == m_NoteDisplays.end() )
		{
			this->CacheNoteSkin( Skin );
			display = m_NoteDisplays.find( Skin );
		}

		RAGE_ASSERT_M( display != m_NoteDisplays.end(), ssprintf("Couldn't find %s", Skin.c_str()) );

		NoteDisplayCols *cols = display->second;
		m_BeatToNoteDisplays[Beat] = cols;
	}
}

void NoteField::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_rectMarkerBar.Update( fDeltaTime );

	if( m_fPercentFadeToFail >= 0 )
		m_fPercentFadeToFail = min( m_fPercentFadeToFail + fDeltaTime/1.5f, 1 );	// take 1.5 seconds to totally fade

	RefreshBeatToNoteSkin();
}

float NoteField::GetWidth()
{
	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	float fMinX, fMaxX;
	pStyleDef->GetMinAndMaxColX( m_PlayerNumber, fMinX, fMaxX );

	return fMaxX - fMinX + ARROW_SIZE;
}

void NoteField::DrawBeatBar( const float fBeat )
{
	bool bIsMeasure = fmodf( fBeat, (float)BEATS_PER_MEASURE ) == 0;
	int iMeasureIndex = (int)fBeat / BEATS_PER_MEASURE;
	int iMeasureNoDisplay = iMeasureIndex+1;

	NoteType nt = BeatToNoteType( fBeat );

	const float fYOffset	= ArrowGetYOffset( m_PlayerNumber, 0, fBeat );
	const float fYPos		= ArrowGetYPos(	m_PlayerNumber, 0, fYOffset, m_fYReverseOffsetPixels );

	float fAlpha;
	int iState;
	
	if( bIsMeasure )
	{
		fAlpha = 1;
		iState = 0;
	}
	else
	{
		float fScrollSpeed = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fScrollSpeed;
		switch( nt )
		{
		default:	ASSERT(0);
		case NOTE_TYPE_4TH:	fAlpha = 1;										iState = 1;	break;
		case NOTE_TYPE_8TH:	fAlpha = SCALE(fScrollSpeed,1.f,2.f,0.f,1.f);	iState = 2;	break;
		case NOTE_TYPE_16TH:fAlpha = SCALE(fScrollSpeed,2.f,4.f,0.f,1.f);	iState = 3;	break;
		}
		CLAMP( fAlpha, 0, 1 );
	}

	float fWidth = GetWidth();
	float fFrameWidth = m_sprBars.GetUnzoomedWidth();

	m_sprBars.SetX( 0 );
	m_sprBars.SetY( fYPos );
	m_sprBars.SetDiffuse( RageColor(1,1,1,fAlpha) );
	m_sprBars.SetState( iState );
	m_sprBars.SetCustomTextureRect( RectF(0,SCALE(iState,0.f,4.f,0.f,1.f), fWidth/fFrameWidth, SCALE(iState+1,0.f,4.f,0.f,1.f)) );
	m_sprBars.SetZoomX( fWidth/m_sprBars.GetUnzoomedWidth() );
	m_sprBars.Draw();


	if( bIsMeasure )
	{
		m_textMeasureNumber.SetDiffuse( RageColor(1,1,1,1) );
		m_textMeasureNumber.SetGlow( RageColor(1,1,1,0) );
		m_textMeasureNumber.SetHorizAlign( Actor::align_right );
		m_textMeasureNumber.SetText( ssprintf("%d", iMeasureNoDisplay) );
		m_textMeasureNumber.SetXY( -fWidth/2, fYPos );
		m_textMeasureNumber.Draw();
	}
}

void NoteField::DrawMarkerBar( const float fBeat )
{
	const float fYOffset	= ArrowGetYOffset( m_PlayerNumber, 0, fBeat );
	const float fYPos		= ArrowGetYPos(	m_PlayerNumber, 0, fYOffset, m_fYReverseOffsetPixels );


	m_rectMarkerBar.StretchTo( RectF(-GetWidth()/2, fYPos-ARROW_SIZE/2, GetWidth()/2, fYPos+ARROW_SIZE/2) );
	m_rectMarkerBar.Draw();
}

void NoteField::DrawAreaHighlight( const float fStartBeat, const float fEndBeat )
{
	float fYStartOffset	= ArrowGetYOffset( m_PlayerNumber, 0, fStartBeat );
	float fYStartPos	= ArrowGetYPos(	m_PlayerNumber, 0, fYStartOffset, m_fYReverseOffsetPixels );
	float fYEndOffset	= ArrowGetYOffset( m_PlayerNumber, 0, fEndBeat );
	float fYEndPos		= ArrowGetYPos(	m_PlayerNumber, 0, fYEndOffset, m_fYReverseOffsetPixels );

	// Something in OpenGL crashes if this is values are too large.  Strange.  -Chris
	fYStartPos = max( fYStartPos, -1000 );	
	fYEndPos = min( fYEndPos, +5000 );	

	m_rectAreaHighlight.StretchTo( RectF(-GetWidth()/2, fYStartPos-ARROW_SIZE/2, GetWidth()/2, fYEndPos+ARROW_SIZE/2) );
	m_rectAreaHighlight.SetDiffuse( RageColor(1,0,0,0.3f) );
	m_rectAreaHighlight.Draw();
}



void NoteField::DrawBPMText( const float fBeat, const float fBPM )
{
	const float fYOffset	= ArrowGetYOffset( m_PlayerNumber, 0, fBeat );
	const float fYPos		= ArrowGetYPos(	m_PlayerNumber, 0, fYOffset, m_fYReverseOffsetPixels );

	m_textMeasureNumber.SetHorizAlign( Actor::align_right );
	m_textMeasureNumber.SetDiffuse( RageColor(1,0,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,cosf(RageTimer::GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fBPM) );
	m_textMeasureNumber.SetXY( -GetWidth()/2.f - 60, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawFreezeText( const float fBeat, const float fSecs )
{
	const float fYOffset	= ArrowGetYOffset(			m_PlayerNumber, 0, fBeat );
 	const float fYPos		= ArrowGetYPos(	m_PlayerNumber, 0, fYOffset, m_fYReverseOffsetPixels );

	m_textMeasureNumber.SetHorizAlign( Actor::align_right );
	m_textMeasureNumber.SetDiffuse( RageColor(0.8f,0.8f,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,cosf(RageTimer::GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fSecs) );
	m_textMeasureNumber.SetXY( -GetWidth()/2.f - 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawBGChangeText( const float fBeat, const CString sNewBGName )
{
	const float fYOffset	= ArrowGetYOffset(			m_PlayerNumber, 0, fBeat );
	const float fYPos		= ArrowGetYPos(	m_PlayerNumber, 0, fYOffset, m_fYReverseOffsetPixels );

	m_textMeasureNumber.SetHorizAlign( Actor::align_left );
	m_textMeasureNumber.SetDiffuse( RageColor(0,1,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,cosf(RageTimer::GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( sNewBGName );
	m_textMeasureNumber.SetXY( +GetWidth()/2.f, fYPos );
	m_textMeasureNumber.Draw();
}

/* cur is an iterator within m_NoteDisplays.  next is ++cur.  Advance or rewind cur to
 * point to the block that contains beat.  We maintain next as an optimization, as this
 * is called for every tap. */
void NoteField::SearchForBeat( NDMap::iterator &cur, NDMap::iterator &next, float Beat )
{
	/* cur is too far ahead: */
	while( cur != m_BeatToNoteDisplays.begin() && cur->first > Beat )
	{
		next = cur;
		--cur;
	}

	/* cur is too far behind: */
	while( next != m_BeatToNoteDisplays.end() && next->first < Beat )
	{
		cur = next;
		++next;
	}
}


// CPU OPTIMIZATION OPPORTUNITY:
// change this probing to binary search
float FindFirstDisplayedBeat( PlayerNumber pn, int iFirstPixelToDraw )
{
	float fFirstBeatToDraw = GAMESTATE->m_fSongBeat-4;	// Adjust to balance off performance and showing enough notes.

	while( fFirstBeatToDraw < GAMESTATE->m_fSongBeat )
	{
		float fYOffset = ArrowGetYOffset(pn, 0, fFirstBeatToDraw);
		if( fYOffset < iFirstPixelToDraw )	// off screen
			fFirstBeatToDraw += 0.1f;	// move toward fSongBeat
		else	// on screen
			break;	// stop probing
	}
	fFirstBeatToDraw -= 0.1f;	// rewind if we intentionally overshot
	return fFirstBeatToDraw;
}

float FindLastDisplayedBeat( PlayerNumber pn, int iLastPixelToDraw )
{
	//
	// Probe for last note to draw.
	// worst case is 0.25x + boost.  Adjust search distance to 
	// so that notes don't pop onto the screen.
	//
	float fSearchDistance = 10;
	float fLastBeatToDraw = GAMESTATE->m_fSongBeat+fSearchDistance;	

	const int NUM_ITERATIONS = 20;

	for( int i=0; i<NUM_ITERATIONS; i++ )
	{
		float fYOffset = ArrowGetYOffset( pn, 0, fLastBeatToDraw );

		if( fYOffset > iLastPixelToDraw )	// off screen
			fLastBeatToDraw -= fSearchDistance;
		else	// on screen
			fLastBeatToDraw += fSearchDistance;

		fSearchDistance /= 2;
	}

	return fLastBeatToDraw;
}


void NoteField::DrawPrimitives()
{
	//LOG->Trace( "NoteField::DrawPrimitives()" );

	/* This should be filled in on the first update. */
	ASSERT( !m_BeatToNoteDisplays.empty() );

	//
	// Adjust draw range depending on some effects
	//
	int iFirstPixelToDraw = m_iStartDrawingPixel;
	int iLastPixelToDraw = m_iEndDrawingPixel;
	
	float fDrawScale = 1;
	fDrawScale *= 1 + 0.5f * fabsf( GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fPerspectiveTilt );
	fDrawScale *= 1 + fabsf( GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber].m_fEffects[PlayerOptions::EFFECT_MINI] );
	
	float fFirstDrawScale = g_NoteFieldMode[m_PlayerNumber].m_fFirstPixelToDrawScale;
	float fLastDrawScale = g_NoteFieldMode[m_PlayerNumber].m_fLastPixelToDrawScale;

	iFirstPixelToDraw = (int)(iFirstPixelToDraw * fFirstDrawScale * fDrawScale);
	iLastPixelToDraw = (int)(iLastPixelToDraw * fLastDrawScale * fDrawScale);


	// probe for first and last notes on the screen
	float fFirstBeatToDraw = FindFirstDisplayedBeat( m_PlayerNumber, iFirstPixelToDraw );
	float fLastBeatToDraw = FindLastDisplayedBeat( m_PlayerNumber, iLastPixelToDraw );

	GAMESTATE->m_fLastDrawnBeat[m_PlayerNumber] = fLastBeatToDraw;

	const int iFirstIndexToDraw  = BeatToNoteRow(fFirstBeatToDraw);
	const int iLastIndexToDraw   = BeatToNoteRow(fLastBeatToDraw);

//	LOG->Trace( "start = %f.1, end = %f.1", fFirstBeatToDraw-fSongBeat, fLastBeatToDraw-fSongBeat );
//	LOG->Trace( "Drawing elements %d through %d", iFirstIndexToDraw, iLastIndexToDraw );

	if( GAMESTATE->m_bEditing )
	{
		ASSERT(GAMESTATE->m_pCurSong);

		unsigned i;

		//
		// Draw beat bars
		//
		{
			float fStartDrawingMeasureBars = max( 0, froundf(fFirstBeatToDraw-0.25f,0.25f) );
			for( float f=fStartDrawingMeasureBars; f<fLastBeatToDraw; f+=0.25f )
				DrawBeatBar( f );
		}

		//
		// BPM text
		//
		vector<BPMSegment> &aBPMSegments = GAMESTATE->m_pCurSong->m_BPMSegments;
		for( i=0; i<aBPMSegments.size(); i++ )
		{
			if(aBPMSegments[i].m_fStartBeat >= fFirstBeatToDraw &&
			   aBPMSegments[i].m_fStartBeat <= fLastBeatToDraw)
				DrawBPMText( aBPMSegments[i].m_fStartBeat, aBPMSegments[i].m_fBPM );
		}
		//
		// Freeze text
		//
		vector<StopSegment> &aStopSegments = GAMESTATE->m_pCurSong->m_StopSegments;
		for( i=0; i<aStopSegments.size(); i++ )
		{
			if(aStopSegments[i].m_fStartBeat >= fFirstBeatToDraw &&
			   aStopSegments[i].m_fStartBeat <= fLastBeatToDraw)
			DrawFreezeText( aStopSegments[i].m_fStartBeat, aStopSegments[i].m_fStopSeconds );
		}

		//
		// BGChange text
		//
		vector<BackgroundChange> &aBackgroundChanges = GAMESTATE->m_pCurSong->m_BackgroundChanges;
		for( i=0; i<aBackgroundChanges.size(); i++ )
		{
			if(aBackgroundChanges[i].m_fStartBeat >= fFirstBeatToDraw &&
			   aBackgroundChanges[i].m_fStartBeat <= fLastBeatToDraw)
			{
				const BackgroundChange& change = aBackgroundChanges[i];
				CString sChangeText = ssprintf("%s\n%.0f%%%s%s%s",
					change.m_sBGName.c_str(),
					change.m_fRate*100,
					change.m_bFadeLast ? " Fade" : "",
					change.m_bRewindMovie ? " Rewind" : "",
					change.m_bLoop ? " Loop" : "" );

				DrawBGChangeText( change.m_fStartBeat, sChangeText );
			}
		}

		//
		// Draw marker bars
		//
		if( m_fBeginMarker != -1  &&  m_fEndMarker != -1 )
			DrawAreaHighlight( m_fBeginMarker, m_fEndMarker );
		else if( m_fBeginMarker != -1 )
			DrawMarkerBar( m_fBeginMarker );
		else if( m_fEndMarker != -1 )
			DrawMarkerBar( m_fEndMarker );

	}


	//
	// Optimization is very important here because there are so many arrows to draw.  
	// Draw the arrows in order of column.  This minimize texture switches and let us
	// draw in big batches.
	//

	float fSelectedRangeGlow = SCALE( cosf(RageTimer::GetTimeSinceStart()*2), -1, 1, 0.1f, 0.3f );

	for( int c=0; c<GetNumTracks(); c++ )	// for each arrow column
	{
		g_NoteFieldMode[m_PlayerNumber].BeginDrawTrack(c);

		//
		// Draw all HoldNotes in this column (so that they appear under the tap notes)
		//
		int i;

		NDMap::iterator CurDisplay = m_BeatToNoteDisplays.begin();
		ASSERT( CurDisplay != m_BeatToNoteDisplays.end() );
		NDMap::iterator NextDisplay = CurDisplay; ++NextDisplay;
		for( i=0; i < GetNumHoldNotes(); i++ )
		{
			const HoldNote &hn = GetHoldNote(i);
			const HoldNoteScore hns = GetHoldNoteScore(i);
			const float fLife = GetHoldNoteLife(i);
			const bool bIsHoldingNote = (i < int(m_bIsHoldingHoldNote.size()))?
				m_bIsHoldingHoldNote[i]: false;
			
			if( hns == HNS_OK )	// if this HoldNote was completed
				continue;	// don't draw anything

			if( hn.iTrack != c )	// this HoldNote doesn't belong to this column
				continue;

			// If no part of this HoldNote is on the screen, skip it
			if( !( fFirstBeatToDraw <= hn.fEndBeat && hn.fEndBeat <= fLastBeatToDraw  ||
				fFirstBeatToDraw <= hn.fStartBeat  && hn.fStartBeat <= fLastBeatToDraw  ||
				hn.fStartBeat < fFirstBeatToDraw   && hn.fEndBeat > fLastBeatToDraw ) )
			{
				continue;	// skip
			}

			SearchForBeat( CurDisplay, NextDisplay, hn.fStartBeat );

			bool bIsInSelectionRange = false;
			if( m_fBeginMarker!=-1 && m_fEndMarker!=-1 )
				bIsInSelectionRange = m_fBeginMarker<=hn.fStartBeat && hn.fStartBeat<=m_fEndMarker && m_fBeginMarker<=hn.fEndBeat && hn.fEndBeat<=m_fEndMarker;

			NoteDisplayCols *nd = CurDisplay->second;

			nd->display[c].DrawHold( hn, bIsHoldingNote, fLife, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, false, m_fYReverseOffsetPixels );
		}
		

		//
		// Draw all TapNotes in this column
		//
		CurDisplay = m_BeatToNoteDisplays.begin();
		NextDisplay = CurDisplay; ++NextDisplay;

		// draw notes from furthest to closest
		for( i=iLastIndexToDraw; i>=iFirstIndexToDraw; --i )	//	 for each row
		{	
			TapNote tn = GetTapNote(c, i);
			if( tn == TAP_EMPTY )	// no note here
				continue;	// skip
			
			if( tn == TAP_HOLD_HEAD )	// this is a HoldNote begin marker.  Grade it, but don't draw
				continue;	// skip

			// See if there is a hold step that begins on this index.
			bool bHoldNoteBeginsOnThisBeat = false;
			for( int c2=0; c2<GetNumTracks(); c2++ )
			{
				if( GetTapNote(c2, i) == TAP_HOLD_HEAD )
				{
					bHoldNoteBeginsOnThisBeat = true;
					break;
				}
			}

			bool bIsInSelectionRange = false;
			if( m_fBeginMarker!=-1 && m_fEndMarker!=-1 )
			{
				float fBeat = NoteRowToBeat(i);
				bIsInSelectionRange = m_fBeginMarker<=fBeat && fBeat<=m_fEndMarker;
			}

			bool bIsAddition = (tn == TAP_ADDITION);
			bool bIsMine = (tn == TAP_MINE);
			bool bIsAttack = IsTapAttack(tn);

			SearchForBeat( CurDisplay, NextDisplay, NoteRowToBeat(i) );
			NoteDisplayCols *nd = CurDisplay->second;
			if( bIsAttack )
			{
				const Attack& attack = GetAttackAt( c, i );
				Sprite sprite;
				sprite.Load( THEME->GetPathToG("NoteField attack "+attack.sModifier) );
				float fBeat = NoteRowToBeat(i);
				SearchForBeat( CurDisplay, NextDisplay, fBeat );
				NoteDisplayCols *nd = CurDisplay->second;
				nd->display[c].DrawActor( &sprite, c, fBeat, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels );
			}
			else
			{
				nd->display[c].DrawTap( c, NoteRowToBeat(i), bHoldNoteBeginsOnThisBeat, bIsAddition, bIsMine, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels );
			}
		}


		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}

}

void NoteField::RemoveTapNoteRow( int iIndex )
{
	for( int c=0; c<GetNumTracks(); c++ )
		SetTapNote(c, iIndex, TAP_EMPTY);
}

void NoteField::FadeToFail()
{
	m_fPercentFadeToFail = max( 0.0f, m_fPercentFadeToFail );	// this will slowly increase every Update()
		// don't fade all over again if this is called twice
}
