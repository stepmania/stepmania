#include "global.h"
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
#include "ThemeManager.h"
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"
#include "song.h"

NoteField::NoteField()
{	
	m_textMeasureNumber.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textMeasureNumber.SetZoom( 1.0f );

	m_rectMarkerBar.SetShadowLength( 0 );
	m_rectMarkerBar.SetEffectDiffuseShift( 2, RageColor(1,1,1,0.5f), RageColor(0.5f,0.5f,0.5f,0.5f) );

	m_sprBars.Load( THEME->GetPathToG("NoteField bars") );
	m_sprBars.StopAnimating();

	m_fBeginMarker = m_fEndMarker = -1;

	m_fPercentFadeToFail = -1;
	LastDisplay = NULL;
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
	LastDisplay = NULL;
}

void NoteField::CacheNoteSkin( CString skin )
{
	if( m_NoteDisplays.find(skin) != m_NoteDisplays.end() )
		return;

	LOG->Trace("NoteField::CacheNoteSkin: cache %s", skin.c_str() );
	NoteDisplayCols *nd = new NoteDisplayCols( GetNumTracks() );
	for( int c=0; c<GetNumTracks(); c++ ) 
		nd->display[c].Load( c, m_PlayerNumber, skin, m_fYReverseOffsetPixels );
	nd->m_ReceptorArrowRow.Load( m_PlayerNumber, skin, m_fYReverseOffsetPixels );
	nd->m_GhostArrowRow.Load( m_PlayerNumber, skin, m_fYReverseOffsetPixels );

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

	m_HeldHoldNotes.clear();
	m_ActiveHoldNotes.clear();

	this->CopyAll( pNoteData );
	ASSERT( GetNumTracks() == GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer );

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

		ASSERT_M( display != m_NoteDisplays.end(), ssprintf("Couldn't find %s", Skin.c_str()) );

		NoteDisplayCols *cols = display->second;
		m_BeatToNoteDisplays[Beat] = cols;
	}
}

void NoteField::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_rectMarkerBar.Update( fDeltaTime );

	NoteDisplayCols *cur = SearchForSongBeat();

	if( cur != LastDisplay )
	{
		/* The display has changed.  We might be in the middle of a step; copy any
		 * tweens. */
		if( LastDisplay )
		{
			cur->m_GhostArrowRow.CopyTweening( LastDisplay->m_GhostArrowRow );
			cur->m_ReceptorArrowRow.CopyTweening( LastDisplay->m_ReceptorArrowRow );
		}

		LastDisplay = cur;
	}

	cur->m_ReceptorArrowRow.Update( fDeltaTime );
	cur->m_GhostArrowRow.Update( fDeltaTime );

	if( m_fPercentFadeToFail >= 0 )
		m_fPercentFadeToFail = min( m_fPercentFadeToFail + fDeltaTime/1.5f, 1 );	// take 1.5 seconds to totally fade

	RefreshBeatToNoteSkin();


	//
	// update all NoteDisplays
	//

	/*
	 * Update all NoteDisplays.  Hack: We need to call this once per frame, not
	 * once per player.
	 */
	if( m_PlayerNumber == GAMESTATE->m_MasterPlayerNumber )
		NoteDisplay::Update( fDeltaTime );
}

float NoteField::GetWidth()
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	float fMinX, fMaxX;
	pStyle->GetMinAndMaxColX( m_PlayerNumber, fMinX, fMaxX );

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

NoteField::NoteDisplayCols *NoteField::SearchForSongBeat()
{
	return SearchForBeat( GAMESTATE->m_fSongBeat );
}

NoteField::NoteDisplayCols *NoteField::SearchForBeat( float Beat )
{
	NDMap::iterator it = m_BeatToNoteDisplays.lower_bound( Beat );
	/* The first entry should always be lower than any Beat we might receive. */
	// This assert is firing with Beat = -7408. -Chris
	// Again with Beat = -7254 and GAMESTATE->m_fMusicSeconds = -3043.61
	// Again with Beat = -9806 and GAMESTATE->m_fMusicSeconds = -3017.22
	// Again with Beat = -9806 and GAMESTATE->m_fMusicSeconds = -3017.22
	// Again in the middle of Remember December in Hardcore Galore:
	//    Beat = -9373.56 and GAMESTATE->m_fMusicSeconds = -2923.48
 	ASSERT_M( it != m_BeatToNoteDisplays.begin(), ssprintf("%f",Beat) );
	--it;
	ASSERT_M( it != m_BeatToNoteDisplays.end(), ssprintf("%f",Beat) );

	return it->second;
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

	NoteDisplayCols *cur = SearchForSongBeat();
	cur->m_ReceptorArrowRow.Draw();

	const PlayerOptions &current_po = GAMESTATE->m_CurrentPlayerOptions[m_PlayerNumber];

	//
	// Adjust draw range depending on some effects
	//
	int iFirstPixelToDraw = m_iStartDrawingPixel;
	// HACK: if boomerang and centered are on, then we want to draw much 
	// earlier to that the notes don't pop on screen.
	float fCenteredTimesBoomerang = 
		current_po.m_fScrolls[PlayerOptions::SCROLL_CENTERED] * 
		current_po.m_fAccels[PlayerOptions::ACCEL_BOOMERANG];
	iFirstPixelToDraw += int(SCALE( fCenteredTimesBoomerang, 0.f, 1.f, 0.f, -SCREEN_HEIGHT/2 ));
	int iLastPixelToDraw = m_iEndDrawingPixel;
	
	float fDrawScale = 1;
	fDrawScale *= 1 + 0.5f * fabsf( current_po.m_fPerspectiveTilt );
	fDrawScale *= 1 + fabsf( current_po.m_fEffects[PlayerOptions::EFFECT_MINI] );
	
	float fFirstDrawScale = g_NoteFieldMode[m_PlayerNumber].m_fFirstPixelToDrawScale;
	float fLastDrawScale = g_NoteFieldMode[m_PlayerNumber].m_fLastPixelToDrawScale;

	iFirstPixelToDraw = (int)(iFirstPixelToDraw * fFirstDrawScale * fDrawScale);
	iLastPixelToDraw = (int)(iLastPixelToDraw * fLastDrawScale * fDrawScale);


	// Probe for first and last notes on the screen
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
		vector<BPMSegment> &aBPMSegments = GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments;
		for( i=0; i<aBPMSegments.size(); i++ )
		{
			if(aBPMSegments[i].m_fStartBeat >= fFirstBeatToDraw &&
			   aBPMSegments[i].m_fStartBeat <= fLastBeatToDraw)
				DrawBPMText( aBPMSegments[i].m_fStartBeat, aBPMSegments[i].m_fBPM );
		}
		//
		// Freeze text
		//
		vector<StopSegment> &aStopSegments = GAMESTATE->m_pCurSong->m_Timing.m_StopSegments;
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
			if( hn.iTrack != c )	// this HoldNote doesn't belong to this column
				continue;

			const HoldNoteResult Result = GetHoldNoteResult( hn );
			if( Result.hns == HNS_OK )	// if this HoldNote was completed
				continue;	// don't draw anything

			// If no part of this HoldNote is on the screen, skip it
			if( !hn.RangeOverlaps(iFirstIndexToDraw, iLastIndexToDraw) )
				continue;	// skip

			// TRICKY: If boomerang is on, then all notes in the range 
			// [iFirstIndexToDraw,iLastIndexToDraw] aren't necessarily visible.
			// Test every note to make sure it's on screen before drawing
			float fYStartOffset = ArrowGetYOffset( m_PlayerNumber, 0, NoteRowToBeat(hn.iStartRow) );
			float fYEndOffset = ArrowGetYOffset( m_PlayerNumber, 0, NoteRowToBeat(hn.iEndRow) );
			if( !( iFirstPixelToDraw <= fYEndOffset && fYEndOffset <= iLastPixelToDraw  ||
				iFirstPixelToDraw <= fYStartOffset  && fYStartOffset <= iLastPixelToDraw  ||
				fYStartOffset < iFirstPixelToDraw   && fYEndOffset > iLastPixelToDraw ) )
			{
				continue;	// skip
			}

			const bool bIsActive = m_ActiveHoldNotes[hn];
			const bool bIsHoldingNote = m_HeldHoldNotes[hn];
			if( bIsActive )
				SearchForSongBeat()->m_GhostArrowRow.SetHoldIsActive( hn.iTrack );
			
			ASSERT_M( NoteRowToBeat(hn.iStartRow) > -2000, ssprintf("%i %i %i", hn.iStartRow, hn.iEndRow, hn.iTrack) );
			SearchForBeat( CurDisplay, NextDisplay, NoteRowToBeat(hn.iStartRow) );

			bool bIsInSelectionRange = false;
			if( m_fBeginMarker!=-1 && m_fEndMarker!=-1 )
				bIsInSelectionRange = hn.ContainedByRange( BeatToNoteRow( m_fBeginMarker ), BeatToNoteRow( m_fEndMarker ) );

			NoteDisplayCols *nd = CurDisplay->second;
			nd->display[c].DrawHold( hn, bIsHoldingNote, bIsActive, Result, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, false, m_fYReverseOffsetPixels );
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
			if( tn.type == TapNote::empty )	// no note here
				continue;	// skip
			
			if( tn.type == TapNote::hold_head )	// this is a HoldNote begin marker.  Grade it, but don't draw
				continue;	// skip

			// TRICKY: If boomerang is on, then all notes in the range 
			// [iFirstIndexToDraw,iLastIndexToDraw] aren't necessarily visible.
			// Test every note to make sure it's on screen before drawing
			float fYOffset = ArrowGetYOffset( m_PlayerNumber, 0, NoteRowToBeat(i) );
			if( fYOffset > iLastPixelToDraw )	// off screen
				continue;	// skip
			if( fYOffset < iFirstPixelToDraw )	// off screen
				continue;	// skip

			// See if there is a hold step that begins on this index.
			bool bHoldNoteBeginsOnThisBeat = false;
			for( int c2=0; c2<GetNumTracks(); c2++ )
			{
				if( GetTapNote(c2, i).type == TapNote::hold_head)
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

			bool bIsAddition = (tn.source == TapNote::addition);
			bool bIsMine = (tn.type == TapNote::mine);
			bool bIsAttack = (tn.type == TapNote::attack);

			ASSERT_M( NoteRowToBeat(i) > -2000, ssprintf("%i %i %i, %f %f", i, iLastIndexToDraw, iFirstIndexToDraw, GAMESTATE->m_fSongBeat, GAMESTATE->m_fMusicSeconds) );
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
				nd->display[c].DrawActor( &sprite, c, fBeat, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels, false );
			}
			else
			{
				nd->display[c].DrawTap( c, NoteRowToBeat(i), bHoldNoteBeginsOnThisBeat, bIsAddition, bIsMine, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels );
			}
		}


		g_NoteFieldMode[m_PlayerNumber].EndDrawTrack(c);
	}

	cur->m_GhostArrowRow.Draw();
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

void NoteField::Step( int iCol, TapNoteScore score ) { SearchForSongBeat()->m_ReceptorArrowRow.Step( iCol, score ); }
void NoteField::SetPressed( int iCol ) { SearchForSongBeat()->m_ReceptorArrowRow.SetPressed( iCol ); }
void NoteField::DidTapNote( int iCol, TapNoteScore score, bool bBright ) { SearchForSongBeat()->m_GhostArrowRow.DidTapNote( iCol, score, bBright ); }
void NoteField::DidHoldNote( int iCol ) { /*SearchForSongBeat()->m_GhostArrowRow.DidHoldNote( iCol );*/ }

/*
 * (c) 2001-2004 Chris Danford
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
