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
#include "ThemeManager.h"
#include "ArrowEffects.h"
#include "PrefsManager.h"
#include "GameManager.h"

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

	m_Mode = MODE_DANCING;

	m_fBeginMarker = m_fEndMarker = -1;

	m_fOverrideAlpha = -1;
}


void NoteField::Load( NoteData* pNoteData, PlayerNumber p, StyleDef* pStyleDef, PlayerOptions po, float fNumBeatsToDrawBehind, float fNumBeatsToDrawAhead, NoteFieldMode mode )
{
	m_PlayerOptions = po;
	m_fNumBeatsToDrawBehind = fNumBeatsToDrawBehind;
	m_fNumBeatsToDrawAhead = fNumBeatsToDrawAhead;
	m_Mode = mode;

	this->CopyAll( pNoteData );

	// init arrow rotations and X positions
	for( int c=0; c<m_iNumTracks; c++ ) 
	{
		CArray<D3DXCOLOR,D3DXCOLOR>	arrayTweenColors;
		GAMEMAN->GetTweenColors( p, c, arrayTweenColors );

		m_ColorNote[c].m_sprColorPart.Load( GAMEMAN->GetPathToGraphic( p, c, GRAPHIC_NOTE_COLOR_PART) );
		m_ColorNote[c].m_sprGrayPart.Load( GAMEMAN->GetPathToGraphic( p, c, GRAPHIC_NOTE_GRAY_PART) );
	}


	for( int i=0; i<MAX_HOLD_NOTE_ELEMENTS; i++ )
		m_HoldNoteLife[i] = 1;		// start with full life


	ASSERT( m_iNumTracks == GAMEMAN->GetCurrentStyleDef()->m_iColsPerPlayer );
}

void NoteField::Update( float fDeltaTime, float fSongBeat )
{
	m_fSongBeat = fSongBeat;
	m_rectMarkerBar.Update( fDeltaTime );
}




void NoteField::CreateTapNoteInstance( ColorNoteInstance &cni, const int iCol, const float fIndex, const D3DXCOLOR color )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerOptions, fIndex, m_fSongBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerOptions, fYOffset );
	const float fRotation	= ArrowGetRotation(	m_PlayerOptions, iCol, fYOffset );
	const float fXPos		= ArrowGetXPos(		m_PlayerOptions, iCol, fYOffset, m_fSongBeat );
	float fAlpha			= ArrowGetAlpha(	m_PlayerOptions, fYPos );
	if( m_fOverrideAlpha != -1 )
		fAlpha = m_fOverrideAlpha;

	D3DXCOLOR colorLeading, colorTrailing;	// of the color part.  Alpha here be overwritten with fAlpha!
	if( color.a == -1 )	// indicated "NULL"
		m_ColorNote[iCol].GetEdgeColorsFromIndexAndBeat( roundf(fIndex), m_fSongBeat, m_PlayerOptions.m_ColorType, colorLeading, colorTrailing );
	else
		colorLeading = colorTrailing = color;

	const float fAddAlpha = m_ColorNote[iCol].GetAddAlphaFromDiffuseAlpha( fAlpha );
	int iGrayPartFrameNo = m_ColorNote[iCol].GetGrayPartFrameNoFromIndexAndBeat( roundf(fIndex), m_fSongBeat );


	ColorNoteInstance instance = { fXPos, fYPos, fRotation, fAlpha, colorLeading, colorTrailing, fAddAlpha, iGrayPartFrameNo };
	cni = instance;
}

void NoteField::CreateHoldNoteInstance( ColorNoteInstance &cni, const bool bActive, const float fIndex, const HoldNote &hn, const float fHoldNoteLife )
{
	const int iCol = hn.m_iTrack;

	const float fYOffset	= ArrowGetYOffset(	m_PlayerOptions, fIndex, m_fSongBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerOptions, fYOffset );
	const float fRotation	= ArrowGetRotation(	m_PlayerOptions, iCol, fYOffset );
	const float fXPos		= ArrowGetXPos(		m_PlayerOptions, iCol, fYOffset, m_fSongBeat );
	float fAlpha			= ArrowGetAlpha(	m_PlayerOptions, fYPos );
	if( m_fOverrideAlpha != -1 )
		fAlpha = m_fOverrideAlpha;

	int iGrayPartFrameNo;
	if( bActive  &&  m_Mode == MODE_DANCING )
		iGrayPartFrameNo = m_ColorNote[iCol].GetGrayPartFrameNoFull();
	else
		iGrayPartFrameNo = m_ColorNote[iCol].GetGrayPartFrameNoClear();

	const float fPercentIntoHold = (fIndex-hn.m_iStartIndex)/(hn.m_iEndIndex-hn.m_iStartIndex);
	D3DXCOLOR colorLeading( fPercentIntoHold, 1, 0, 1 ); // color shifts from green to yellow
	colorLeading *= fHoldNoteLife;
	colorLeading.a = 1;
	D3DXCOLOR colorTrailing = colorLeading;

	const float fAddAlpha = m_ColorNote[iCol].GetAddAlphaFromDiffuseAlpha( fAlpha );

	ColorNoteInstance instance = { fXPos, fYPos, fRotation, fAlpha, colorLeading, colorTrailing, fAddAlpha, iGrayPartFrameNo };
	cni = instance;
}

void NoteField::DrawMeasureBar( const int iIndex, const int iMeasureNo )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerOptions, (float)iIndex, m_fSongBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerOptions, fYOffset );

	m_rectMeasureBar.SetXY( 0, fYPos );
	m_rectMeasureBar.SetWidth( (float)(m_iNumTracks+1) * ARROW_SIZE );
	m_rectMeasureBar.SetHeight( 20 );
	m_rectMeasureBar.SetDiffuseColor( D3DXCOLOR(0,0,0,0.5f) );
	m_rectMeasureBar.Draw();

	m_textMeasureNumber.SetText( ssprintf("%d", iMeasureNo) );
	m_textMeasureNumber.SetXY( -m_rectMeasureBar.GetZoomedWidth()/2 + 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawMarkerBar( const int iIndex )
{
	const float fYOffset	= ArrowGetYOffset(	m_PlayerOptions, (float)iIndex, m_fSongBeat );
	const float fYPos		= ArrowGetYPos(		m_PlayerOptions, fYOffset );

	m_rectMarkerBar.SetXY( 0, fYPos );
	m_rectMarkerBar.SetWidth( (float)(m_iNumTracks+1) * ARROW_SIZE );
	m_rectMarkerBar.SetHeight( 20 );
	m_rectMarkerBar.SetDiffuseColor( D3DXCOLOR(0,0,0,0.5f) );
	m_rectMarkerBar.Draw();
}

void NoteField::DrawPrimitives()
{
	//LOG->WriteLine( "NoteField::DrawPrimitives()" );

	if( m_fSongBeat < 0 )
		m_fSongBeat = 0;

	int iBaseFrameNo = (int)(m_fSongBeat*2.5) % NUM_FRAMES_IN_COLOR_ARROW_SPRITE;	// 2.5 is a "fudge number" :-)  This should be based on BPM
	
	int iIndexFirstArrowToDraw = BeatToNoteRow( m_fSongBeat - m_fNumBeatsToDrawBehind );	// 2 beats earlier
	if( iIndexFirstArrowToDraw < 0 ) iIndexFirstArrowToDraw = 0;
	int iIndexLastArrowToDraw  = BeatToNoteRow( m_fSongBeat + m_fNumBeatsToDrawAhead );	// 7 beats later

	//LOG->WriteLine( "Drawing elements %d through %d", iIndexFirstArrowToDraw, iIndexLastArrowToDraw );

	//
	// Draw measure bars
	//
	if( m_Mode == MODE_EDITING )
	{
		for( int i=iIndexFirstArrowToDraw; i<=iIndexLastArrowToDraw; i++ )
		{
			if( i % ELEMENTS_PER_MEASURE == 0 )
				DrawMeasureBar( i, i / ELEMENTS_PER_MEASURE );
		}
	}

	//
	// Draw marker bars
	//
	if( m_Mode == MODE_EDITING )
	{
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
			if( hn.m_iEndIndex < BeatToNoteRow(m_fSongBeat)  &&  m_HoldNoteLife[i] > 0 )
				continue;	// skip


			const int iCol = hn.m_iTrack;
			const float fHoldNoteLife = m_HoldNoteLife[i];
			const bool bActive = NoteRowToBeat(hn.m_iStartIndex-1) <= m_fSongBeat  &&  m_fSongBeat <= NoteRowToBeat(hn.m_iEndIndex);	// hack: added -1 because hn.m_iStartIndex changes as note is held

			// parts of the hold
			const float fStartDrawingAtBeat = froundf( (float)hn.m_iStartIndex, ROWS_BETWEEN_HOLD_BITS );
			for( float j=fStartDrawingAtBeat; 
				 j<=hn.m_iEndIndex; 
				 j+=ROWS_BETWEEN_HOLD_BITS/m_PlayerOptions.m_fArrowScrollSpeed )	// for each bit of the hold
			{
 				// check if this arrow is off the the screen
				if( j < iIndexFirstArrowToDraw || iIndexLastArrowToDraw < j)
					continue;	// skip this arrow

				CreateHoldNoteInstance( instances[iCount++], bActive, (float)j, hn, fHoldNoteLife );
			}

		}
		
		const bool bDrawAddPass = m_PlayerOptions.m_AppearanceType != PlayerOptions::APPEARANCE_VISIBLE;
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
			for( int j=0; j<m_iNumHoldNotes; j++ )
			{
				if( m_HoldNotes[j].m_iStartIndex == i )
				{
					bHoldNoteOnThisBeat = true;
					break;
				}
			}

			if( bHoldNoteOnThisBeat )
				CreateTapNoteInstance( instances[iCount++], c, (float)i, D3DXCOLOR(0,1,0,1) );
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

void NoteField::SetHoldNoteLife( int iIndex, float fLife )
{
	m_HoldNoteLife[iIndex] = fLife;
}
