#include "stdafx.h"
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
#include "PrefsManager.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "RageLog.h"


const float HOLD_NOTE_BITS_PER_BEAT	= 6;
const float HOLD_NOTE_BITS_PER_ROW	= HOLD_NOTE_BITS_PER_BEAT / ROWS_PER_BEAT;
const float ROWS_BETWEEN_HOLD_BITS	= 1 / HOLD_NOTE_BITS_PER_ROW;	

NoteField::NoteField()
{
	m_rectMeasureBar.TurnShadowOff();

	m_textMeasureNumber.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textMeasureNumber.SetZoom( 1.0f );

	m_rectMarkerBar.TurnShadowOff();
	m_rectMarkerBar.SetEffectGlowing();

	m_fBeginMarker = m_fEndMarker = -1;

	m_fPercentFadeToFail = -1;
}


void NoteField::Load( NoteData* pNoteData, PlayerNumber pn, int iPixelsToDrawBehind, int iPixelsToDrawAhead )
{
	m_PlayerNumber = pn;
	m_iPixelsToDrawBehind = iPixelsToDrawBehind;
	m_iPixelsToDrawAhead = iPixelsToDrawAhead;

	m_fPercentFadeToFail = -1;

	NoteDataWithScoring::Init(pNoteData->GetNumTapNotes(), pNoteData->GetNumHoldNotes());

	for( int i=0; i<MAX_HOLD_NOTES; i++ )
		m_bIsHoldingHoldNote[i] = false;

	this->CopyAll( pNoteData );

	// init note displays
	for( int c=0; c<m_iNumTracks; c++ ) 
		m_NoteDisplay[c].Load( c, pn );

	ASSERT( m_iNumTracks == GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer );
}

void NoteField::Update( float fDeltaTime )
{
	m_rectMarkerBar.Update( fDeltaTime );

	if( m_fPercentFadeToFail >= 0 )
		m_fPercentFadeToFail = min( m_fPercentFadeToFail + fDeltaTime/1.5f, 1 );	// take 1.5 seconds to totally fade
}

void NoteField::DrawMeasureBar( int iMeasureIndex )
{
	const int iMeasureNoDisplay = iMeasureIndex+1;
	const float fBeat = float(iMeasureIndex * BEATS_PER_MEASURE);

	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, fBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_rectMeasureBar.SetXY( 0, fYPos );
	m_rectMeasureBar.SetZoomX( (float)(m_iNumTracks+1) * ARROW_SIZE );
	m_rectMeasureBar.SetZoomY( 20 );
	m_rectMeasureBar.SetDiffuse( RageColor(0,0,0,0.5f) );
	m_rectMeasureBar.Draw();

	m_textMeasureNumber.SetDiffuse( RageColor(1,1,1,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,0) );
	m_textMeasureNumber.SetText( ssprintf("%d", iMeasureNoDisplay) );
	m_textMeasureNumber.SetXY( -m_rectMeasureBar.GetZoomedWidth()/2 + 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawMarkerBar( const float fBeat )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, fBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_rectMarkerBar.SetXY( 0, fYPos );
	m_rectMarkerBar.SetZoomX( (float)(m_iNumTracks+1) * ARROW_SIZE );
	m_rectMarkerBar.SetZoomY( 20 );
	m_rectMarkerBar.SetDiffuse( RageColor(0,0,0,0.5f) );
	m_rectMarkerBar.Draw();
}

void NoteField::DrawBPMText( const float fBeat, const float fBPM )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, fBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_textMeasureNumber.SetDiffuse( RageColor(1,0,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,cosf(TIMER->GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fBPM) );
	m_textMeasureNumber.SetXY( -m_rectMeasureBar.GetZoomedWidth()/2 - 60, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawFreezeText( const float fBeat, const float fSecs )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, fBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_textMeasureNumber.SetDiffuse( RageColor(0.8f,0.8f,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,cosf(TIMER->GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fSecs) );
	m_textMeasureNumber.SetXY( -m_rectMeasureBar.GetZoomedWidth()/2 - 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawBGChangeText( const float fBeat, const CString sNewBGName )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, fBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_textMeasureNumber.SetDiffuse( RageColor(0,1,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,cosf(TIMER->GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( sNewBGName );
	m_textMeasureNumber.SetXY( +m_rectMeasureBar.GetZoomedWidth()/2 + 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawPrimitives()
{
	//LOG->Trace( "NoteField::DrawPrimitives()" );

	const float fSongBeat = GAMESTATE->m_fSongBeat;
	
	const float fBeatsToDrawBehind = m_iPixelsToDrawBehind * (1/(float)ARROW_SIZE) * (1/GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fArrowScrollSpeed);
	const float fBeatsToDrawAhead  = m_iPixelsToDrawAhead  * (1/(float)ARROW_SIZE) * (1/GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fArrowScrollSpeed);
	const float fFirstBeatToDraw = max( 0, fSongBeat - fBeatsToDrawBehind );
	/* We actually want to draw up to the last row, which is after MAX_BEATS-1 but
	 * before MAX_BEATS.  Rely on GetTapNote to cap it for us correctly. */
	const float fLastBeatToDraw  = min( MAX_BEATS, fSongBeat + fBeatsToDrawAhead );
	const int iFirstIndexToDraw  = BeatToNoteRow(fFirstBeatToDraw);
	const int iLastIndexToDraw   = BeatToNoteRow(fLastBeatToDraw);

//	LOG->Trace( "Drawing elements %d through %d", iFirstIndexToDraw, iLastIndexToDraw );

	if( GAMESTATE->m_bEditing )
	{
		unsigned i;

		//
		// Draw measure bars
		//
		unsigned iFirstMeasureToDraw = int(fFirstBeatToDraw)/BEATS_PER_MEASURE;
		unsigned iLastMeasureToDraw = (int(fLastBeatToDraw)/BEATS_PER_MEASURE)+1;
		for( i=iFirstMeasureToDraw; i<=iLastMeasureToDraw; i++ )
			DrawMeasureBar( i );

		//
		// BPM text
		//
		CArray<BPMSegment,BPMSegment&> &aBPMSegments = GAMESTATE->m_pCurSong->m_BPMSegments;
		for( i=0; i<aBPMSegments.size(); i++ )
			DrawBPMText( aBPMSegments[i].m_fStartBeat, aBPMSegments[i].m_fBPM );

		//
		// Freeze text
		//
		CArray<StopSegment,StopSegment&> &aStopSegments = GAMESTATE->m_pCurSong->m_StopSegments;
		for( i=0; i<aStopSegments.size(); i++ )
			DrawFreezeText( aStopSegments[i].m_fStartBeat, aStopSegments[i].m_fStopSeconds );

		//
		// BGChange text
		//
		CArray<BackgroundChange,BackgroundChange&> &aBackgroundChanges = GAMESTATE->m_pCurSong->m_BackgroundChanges;
		for( i=0; i<aBackgroundChanges.size(); i++ )
			DrawBGChangeText( aBackgroundChanges[i].m_fStartBeat, aBackgroundChanges[i].m_sBGName );

		//
		// Draw marker bars
		//
		if( m_fBeginMarker != -1 )
			DrawMarkerBar( m_fBeginMarker );
		if( m_fEndMarker != -1 )
			DrawMarkerBar( m_fEndMarker );

	}


	//
	// Optimization is very important here because there are so many arrows to draw.  
	// Draw the arrows in order of column.  This minimize texture switches and let us
	// draw in big batches.
	//

	for( int c=0; c<m_iNumTracks; c++ )	// for each arrow column
	{
		/////////////////////////////////
		// Draw all HoldNotes in this column (so that they appear under the tap notes)
		/////////////////////////////////
		for( int i=0; i < GetNumHoldNotes(); i++ )
		{
			const HoldNote &hn = GetHoldNote(i);
			HoldNoteScore &hns = m_HoldNoteScores[i];
			const float fLife = m_fHoldNoteLife[i];
			const bool bIsHoldingNote = m_bIsHoldingHoldNote[i];	// hack: added -1 because hn.m_iStartIndex changes as note is held
			
			if( hns == HNS_OK )	// if this HoldNote was completed
				continue;	// don't draw anything

			if( hn.m_iTrack != c )	// this HoldNote doesn't belong to this column
				continue;

			// If no part of this HoldNote is on the screen, skip it
			if( !( fFirstBeatToDraw <= hn.m_fEndBeat && hn.m_fEndBeat <= fLastBeatToDraw  ||
				fFirstBeatToDraw <= hn.m_fStartBeat  && hn.m_fStartBeat <= fLastBeatToDraw  ||
				hn.m_fStartBeat < fFirstBeatToDraw   && hn.m_fEndBeat > fLastBeatToDraw ) )
			{
				continue;	// skip
			}


			m_NoteDisplay[c].DrawHold( hn, bIsHoldingNote, fLife, m_fPercentFadeToFail );
		}
		

		///////////////////////////////////
		// Draw all TapNotes in this column
		///////////////////////////////////
		for( i=iFirstIndexToDraw; i<=iLastIndexToDraw; i++ )	//	 for each row
		{	
			if( GetTapNote(c, i) == TAP_EMPTY )	// no note here
				continue;	// skip
			
			if( GetTapNote(c, i) == TAP_HOLD_HEAD )	// this is a HoldNote begin marker.  Grade it, but don't draw
				continue;	// skip

			// See if there is a hold step that begins on this index.
			bool bHoldNoteBeginsOnThisBeat = false;
			for( int c2=0; c2<m_iNumTracks; c2++ )
			{
				if( GetTapNote(c2, i) == TAP_HOLD_HEAD )
				{
					bHoldNoteBeginsOnThisBeat = true;
					break;
				}
			}

			m_NoteDisplay[c].DrawTap( c, NoteRowToBeat(i), bHoldNoteBeginsOnThisBeat, m_fPercentFadeToFail );
		}
	}

}


void NoteField::RemoveTapNoteRow( int iIndex )
{
	for( int c=0; c<m_iNumTracks; c++ )
		SetTapNote(c, iIndex, TAP_EMPTY);
}

void NoteField::FadeToFail()
{
	m_fPercentFadeToFail = max( 0.0f, m_fPercentFadeToFail );	// this will slowly increase every Update()
		// don't fade all over again if this is called twice
}