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


const float HOLD_NOTE_BITS_PER_BEAT	= 6;
const float HOLD_NOTE_BITS_PER_ROW	= HOLD_NOTE_BITS_PER_BEAT / ELEMENTS_PER_BEAT;
const float ROWS_BETWEEN_HOLD_BITS	= 1 / HOLD_NOTE_BITS_PER_ROW;	

NoteField::NoteField()
{
	m_rectMeasureBar.TurnShadowOff();

	m_textMeasureNumber.Load( THEME->GetPathTo(FONT_NORMAL) );
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

	NoteDataWithScoring::Init();

	for( int i=0; i<MAX_HOLD_NOTES; i++ )
		m_bIsHoldingHoldNote[i] = false;

	StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();

	this->CopyAll( pNoteData );

	// init arrow rotations and X positions
	for( int c=0; c<m_iNumTracks; c++ ) 
	{
		CArray<D3DXCOLOR,D3DXCOLOR>	arrayTweenColors;
		GAMEMAN->GetTweenColors( c, arrayTweenColors );

		m_ColorNote[c].Load( c, pn );
	}


	for( i=0; i<MAX_HOLD_NOTES; i++ )
		m_fHoldNoteLife[i] = 1;		// start with full life


	ASSERT( m_iNumTracks == GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer );
}

void NoteField::Update( float fDeltaTime )
{
	m_rectMarkerBar.Update( fDeltaTime );

	if( m_fPercentFadeToFail >= 0 )
		m_fPercentFadeToFail = min( m_fPercentFadeToFail + fDeltaTime/3, 1 );	// take 3 seconds to totally fade
}




void NoteField::CreateTapNoteInstance( ColorNoteInstance &cni, const int iCol, const float fIndex, const D3DXCOLOR color )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, fIndex );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );
	const float fRotation	= ArrowGetRotation(	m_PlayerNumber, iCol, fYOffset );
	const float fXPos		= ArrowGetXPos(		m_PlayerNumber, iCol, fYOffset );
	      float fAlpha		= ArrowGetAlpha(	m_PlayerNumber, fYPos );

	if( m_fPercentFadeToFail != -1 )
		fAlpha = 1-m_fPercentFadeToFail;

	D3DXCOLOR colorLeading, colorTrailing;	// of the color part.  Alpha here be overwritten with fAlpha!
	if( color.a == -1 )	// indicated "NULL"
		m_ColorNote[iCol].GetEdgeColorsFromIndexAndBeat( roundf(fIndex), colorLeading, colorTrailing );
	else
		colorLeading = colorTrailing = color;

	float fAddAlpha = m_ColorNote[iCol].GetAddAlphaFromDiffuseAlpha( fAlpha );
	int iGrayPartFrameNo = m_ColorNote[iCol].GetGrayPartFrameNoFromIndexAndBeat( roundf(fIndex), GAMESTATE->m_fSongBeat );


	ColorNoteInstance instance = { fXPos, fYPos, fRotation, fAlpha, colorLeading, colorTrailing, fAddAlpha, iGrayPartFrameNo };
	cni = instance;
}

void NoteField::CreateHoldNoteInstance( ColorNoteInstance &cni, const bool bActive, const float fIndex, const HoldNote &hn, const float fHoldNoteLife )
{
	const int iCol = hn.m_iTrack;

	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, fIndex );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );
	const float fRotation	= ArrowGetRotation(	m_PlayerNumber, iCol, fYOffset );
	const float fXPos		= ArrowGetXPos(		m_PlayerNumber, iCol, fYOffset );
	      float fAlpha		= ArrowGetAlpha(	m_PlayerNumber, fYPos );

	if( m_fPercentFadeToFail != -1 )
		fAlpha = 1-m_fPercentFadeToFail;

	int iGrayPartFrameNo;
	if( bActive )
		iGrayPartFrameNo = m_ColorNote[iCol].GetGrayPartFrameNoFull();
	else
		iGrayPartFrameNo = m_ColorNote[iCol].GetGrayPartFrameNoClear();

	const float fPercentIntoHold = (fIndex-hn.m_iStartIndex)/(hn.m_iEndIndex-hn.m_iStartIndex);
	D3DXCOLOR colorLeading( fPercentIntoHold, 1, 0, 1 ); // color shifts from green to yellow
	colorLeading *= fHoldNoteLife;
	colorLeading.a = 1;
	D3DXCOLOR colorTrailing = colorLeading;

	float fAddAlpha = m_ColorNote[iCol].GetAddAlphaFromDiffuseAlpha( fAlpha );

	ColorNoteInstance instance = { fXPos, fYPos, fRotation, fAlpha, colorLeading, colorTrailing, fAddAlpha, iGrayPartFrameNo };
	cni = instance;
}

void NoteField::DrawMeasureBar( const int iIndex, const int iMeasureNo )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, (float)iIndex );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_rectMeasureBar.SetXY( 0, fYPos );
	m_rectMeasureBar.SetWidth( (float)(m_iNumTracks+1) * ARROW_SIZE );
	m_rectMeasureBar.SetHeight( 20 );
	m_rectMeasureBar.SetDiffuseColor( D3DXCOLOR(0,0,0,0.5f) );
	m_rectMeasureBar.Draw();

	m_textMeasureNumber.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_textMeasureNumber.SetAddColor( D3DXCOLOR(1,1,1,0) );
	m_textMeasureNumber.SetText( ssprintf("%d", iMeasureNo) );
	m_textMeasureNumber.SetXY( -m_rectMeasureBar.GetZoomedWidth()/2 + 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawMarkerBar( const int iIndex )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, (float)iIndex );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_rectMarkerBar.SetXY( 0, fYPos );
	m_rectMarkerBar.SetWidth( (float)(m_iNumTracks+1) * ARROW_SIZE );
	m_rectMarkerBar.SetHeight( 20 );
	m_rectMarkerBar.SetDiffuseColor( D3DXCOLOR(0,0,0,0.5f) );
	m_rectMarkerBar.Draw();
}

void NoteField::DrawBPMText( const int iIndex, const float fBPM )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, (float)iIndex );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_textMeasureNumber.SetDiffuseColor( D3DXCOLOR(1,0,0,1) );
	m_textMeasureNumber.SetAddColor( D3DXCOLOR(1,1,1,cosf(TIMER->GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fBPM) );
	m_textMeasureNumber.SetXY( -m_rectMeasureBar.GetZoomedWidth()/2 - 60, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawFreezeText( const int iIndex, const float fSecs )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerNumber, (float)iIndex );
	const float fYPos		= ArrowGetYPos(		m_PlayerNumber, fYOffset );

	m_textMeasureNumber.SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,0,1) );
	m_textMeasureNumber.SetAddColor( D3DXCOLOR(1,1,1,cosf(TIMER->GetTimeSinceStart()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fSecs) );
	m_textMeasureNumber.SetXY( -m_rectMeasureBar.GetZoomedWidth()/2 - 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawPrimitives()
{
	//LOG->Trace( "NoteField::DrawPrimitives()" );

	float fSongBeat = max( 0, GAMESTATE->m_fSongBeat );

	int iBaseFrameNo = (int)(fSongBeat*2.5) % NUM_FRAMES_IN_COLOR_ARROW_SPRITE;	// 2.5 is a "fudge number" :-)  This should be based on BPM
	
	const float fBeatsToDrawBehind = m_iPixelsToDrawBehind * (1/(float)ARROW_SIZE) * (1/GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fArrowScrollSpeed);
	const float fBeatsToDrawAhead  = m_iPixelsToDrawAhead  * (1/(float)ARROW_SIZE) * (1/GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fArrowScrollSpeed);
	const int iIndexFirstArrowToDraw = max( 0, BeatToNoteRow( fSongBeat - fBeatsToDrawBehind ) );
	const int iIndexLastArrowToDraw  = BeatToNoteRow( fSongBeat + fBeatsToDrawAhead );

	//LOG->Trace( "Drawing elements %d through %d", iIndexFirstArrowToDraw, iIndexLastArrowToDraw );

	if( GAMESTATE->m_bEditing )
	{
		//
		// Draw measure bars
		//
		for( int i=iIndexFirstArrowToDraw; i<=iIndexLastArrowToDraw; i++ )
		{
			if( i % ELEMENTS_PER_MEASURE == 0 )
				DrawMeasureBar( i, i / ELEMENTS_PER_MEASURE + 1 );
		}

		//
		// BPM text
		//
		CArray<BPMSegment,BPMSegment&> &aBPMSegments = GAMESTATE->m_pCurSong->m_BPMSegments;
		for( i=0; i<aBPMSegments.GetSize(); i++ )
		{
			DrawBPMText( BeatToNoteRow(aBPMSegments[i].m_fStartBeat), aBPMSegments[i].m_fBPM );
		}

		//
		// Freeze text
		//
		CArray<StopSegment,StopSegment&> &aStopSegments = GAMESTATE->m_pCurSong->m_StopSegments;
		for( i=0; i<aStopSegments.GetSize(); i++ )
		{
			DrawFreezeText( BeatToNoteRow(aStopSegments[i].m_fStartBeat), aStopSegments[i].m_fStopSeconds );
		}

		//
		// Draw marker bars
		//
		if( m_fBeginMarker != -1 )
		{
			DrawMarkerBar( BeatToNoteRow(m_fBeginMarker) );
		}
		if( m_fEndMarker != -1 )
		{
			DrawMarkerBar( BeatToNoteRow(m_fEndMarker) );
		}

	}


	//
	// Optimization is very important here because there are so many arrows to draw.  We're going 
	// to draw the arrows in order of column.  This will let us fill up a vertex buffer of arrows 
	// so we can draw them in one swoop without texture or state changes.
	//


	for( int c=0; c<m_iNumTracks; c++ )	// for each arrow column
	{
		const int MAX_COLOR_NOTE_INSTANCES = 300;
		ColorNoteInstance instances[MAX_COLOR_NOTE_INSTANCES];
		int iCount = 0;		// number of valid elements in the instances array




		/////////////////////////////////
		// Draw all HoldNotes in this column (so that they appear under the tap notes)
		/////////////////////////////////
		for( int i=0; i<m_iNumHoldNotes; i++ )
		{
			HoldNote &hn = m_HoldNotes[i];
			HoldNoteScore &hns = m_HoldNoteScores[i];
			
			if( hns == HNS_OK )	// if this HoldNote was completed
				continue;	// don't draw anything

			const float fLife = m_fHoldNoteLife[i];

			if( hn.m_iTrack != c )	// this HoldNote doesn't belong to this column
				continue;

			// If no part of this HoldNote is on the screen, skip it
			if( !( iIndexFirstArrowToDraw <= hn.m_iEndIndex && hn.m_iEndIndex <= iIndexLastArrowToDraw  ||
				iIndexFirstArrowToDraw <= hn.m_iStartIndex  && hn.m_iStartIndex <= iIndexLastArrowToDraw  ||
				hn.m_iStartIndex < iIndexFirstArrowToDraw && hn.m_iEndIndex > iIndexLastArrowToDraw ) )
			{
				continue;	// skip
			}


			// If this note was in the past and has life > 0, then it was completed and don't draw it!
			if( hn.m_iEndIndex < BeatToNoteRow(fSongBeat)  &&  fLife > 0  &&  m_bIsHoldingHoldNote[i] )
				continue;	// skip


			const int iCol = hn.m_iTrack;
			const float fHoldNoteLife = m_fHoldNoteLife[i];
			const bool bActive = m_bIsHoldingHoldNote[i];	// hack: added -1 because hn.m_iStartIndex changes as note is held

			// parts of the hold
			const float fStartDrawingAtBeat = froundf( (float)hn.m_iStartIndex, ROWS_BETWEEN_HOLD_BITS/GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fArrowScrollSpeed );
			for( float j=fStartDrawingAtBeat; 
				 j<=hn.m_iEndIndex; 
				 j+=ROWS_BETWEEN_HOLD_BITS/GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_fArrowScrollSpeed )	// for each bit of the hold
			{
 				// check if this arrow is off the the screen
				if( j < iIndexFirstArrowToDraw || iIndexLastArrowToDraw < j)
					continue;	// skip this arrow

				if( bActive  &&  NoteRowToBeat(j) < fSongBeat )
					continue;

				CreateHoldNoteInstance( instances[iCount++], bActive, (float)j, hn, fHoldNoteLife );
			}

		}
		
		const bool bDrawAddPass = GAMESTATE->m_PlayerOptions[m_PlayerNumber].m_AppearanceType != PlayerOptions::APPEARANCE_VISIBLE;
		if( iCount > 0 )
			m_ColorNote[c].DrawList( iCount, instances, bDrawAddPass );



		iCount = 0;		// reset count

		///////////////////////////////////
		// Draw all TapNotes in this column
		///////////////////////////////////
		for( i=iIndexFirstArrowToDraw; i<=iIndexLastArrowToDraw; i++ )	//	 for each row
		{	
			if( m_TapNotes[c][i] == '0' )
				continue;	// no note here
			
			// See if there is a hold step that begins on this beat.
			bool bHoldNoteOnThisBeat = false;
			float fHoldLife = -1;
			for( int j=0; j<m_iNumHoldNotes; j++ )
			{
				if( m_HoldNotes[j].m_iStartIndex == i )
				{
					bHoldNoteOnThisBeat = true;
					fHoldLife = m_fHoldNoteLife[j];
					break;
				}
			}

			if( bHoldNoteOnThisBeat )
			{
				D3DXCOLOR color(0,1,0,1);
				color.r *= fHoldLife;
				color.g *= fHoldLife;
				color.b *= fHoldLife;
				CreateTapNoteInstance( instances[iCount++], c, (float)i, color );
			}
			else
				CreateTapNoteInstance( instances[iCount++], c, (float)i );
		}

		if( iCount > 0 )
			m_ColorNote[c].DrawList( iCount, instances, bDrawAddPass );
	}

}




void NoteField::RemoveTapNoteRow( int iIndex )
{
	for( int c=0; c<m_iNumTracks; c++ )
		m_TapNotes[c][iIndex] = '0';
}

void NoteField::FadeToFail()
{
	m_fPercentFadeToFail = max( 0.0f, m_fPercentFadeToFail );	// this will slowly increase every Update()
		// don't fade all over again if this is called twice
}