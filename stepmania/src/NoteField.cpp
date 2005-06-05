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
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "Style.h"
#include "CommonMetrics.h"
#include <float.h>
#include "BackgroundUtil.h"

NoteField::NoteField()
{	
	m_pNoteData = NULL;

	m_textMeasureNumber.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textMeasureNumber.SetZoom( 1.0f );
	m_textMeasureNumber.SetShadowLength( 2 );
	m_textMeasureNumber.SetWrapWidthPixels( 300 );

	m_rectMarkerBar.SetEffectDiffuseShift( 2, RageColor(1,1,1,0.5f), RageColor(0.5f,0.5f,0.5f,0.5f) );

	m_sprBars.Load( THEME->GetPathG("NoteField","bars") );
	m_sprBars.StopAnimating();

	m_iBeginMarker = m_iEndMarker = -1;

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

void NoteField::CacheNoteSkin( const CString &sNoteSkin_ )
{
	CString sNoteSkin = sNoteSkin_;
	sNoteSkin.ToLower();

	if( m_NoteDisplays.find(sNoteSkin) != m_NoteDisplays.end() )
		return;

	LockNoteSkin l( sNoteSkin );
		
	LOG->Trace("NoteField::CacheNoteSkin: cache %s", sNoteSkin.c_str() );
	NoteDisplayCols *nd = new NoteDisplayCols( GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer );
	for( int c=0; c<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; c++ ) 
		nd->display[c].Load( c, m_pPlayerState, m_fYReverseOffsetPixels );
	nd->m_ReceptorArrowRow.Load( m_pPlayerState, m_fYReverseOffsetPixels );
	nd->m_GhostArrowRow.Load( m_pPlayerState, m_fYReverseOffsetPixels );

	m_NoteDisplays[ sNoteSkin ] = nd;
}

void NoteField::CacheAllUsedNoteSkins()
{
	/* Cache all note skins that we might need for the whole song, course or battle
	 * play, so we don't have to load them later (such as between course songs). */
	vector<CString> skins;
	GAMESTATE->GetAllUsedNoteSkins( skins );
	for( unsigned i=0; i < skins.size(); ++i )
		CacheNoteSkin( skins[i] );
}

void NoteField::Init( const PlayerState* pPlayerState, float fYReverseOffsetPixels )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffsetPixels;
	CacheAllUsedNoteSkins();
}

void NoteField::Load( 
	const NoteData *pNoteData,
	int iFirstPixelToDraw, 
	int iLastPixelToDraw )
{
	m_pNoteData = pNoteData;
	m_iStartDrawingPixel = iFirstPixelToDraw;
	m_iEndDrawingPixel = iLastPixelToDraw;

	m_fPercentFadeToFail = -1;
	m_LastSeenBeatToNoteSkinRev = -1;

	ASSERT( m_pNoteData->GetNumTracks() == GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer );

	RefreshBeatToNoteSkin();
}


void NoteField::RefreshBeatToNoteSkin()
{
	if( GAMESTATE->m_BeatToNoteSkinRev == m_LastSeenBeatToNoteSkinRev )
		return;
	m_LastSeenBeatToNoteSkinRev = GAMESTATE->m_BeatToNoteSkinRev;

	/* Set by GameState::ResetNoteSkins(): */
	ASSERT( !m_pPlayerState->m_BeatToNoteSkin.empty() );

	m_BeatToNoteDisplays.clear();

	/* GAMESTATE->m_BeatToNoteSkin[pn] maps from song beats to note skins.  Maintain
	 * m_BeatToNoteDisplays, to map from song beats to NoteDisplay*s, so we don't
	 * have to do it while rendering. */
	
	for( map<float,CString>::const_iterator it = m_pPlayerState->m_BeatToNoteSkin.begin(); 
		 it != m_pPlayerState->m_BeatToNoteSkin.end(); 
		 ++it )
	{
		const float Beat = it->first;
		const CString &Skin = it->second;

		map<CString, NoteDisplayCols *>::iterator display = m_NoteDisplays.find( Skin );
		if( display == m_NoteDisplays.end() )
		{
			/* Skins should always be loaded by CacheAllUsedNoteSkins. */
			LOG->Warn( "NoteField::RefreshBeatToNoteSkin: need note skin \"%s\" which should have been loaded alraedy", Skin.c_str() );
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

	cur->m_ReceptorArrowRow.Update( fDeltaTime );

	if( m_fPercentFadeToFail >= 0 )
		m_fPercentFadeToFail = min( m_fPercentFadeToFail + fDeltaTime/1.5f, 1 );	// take 1.5 seconds to totally fade


	// Update fade to failed
	FOREACHM( CString, NoteDisplayCols*, m_NoteDisplays, iter )
	{
		iter->second->m_ReceptorArrowRow.SetFadeToFailPercent( m_fPercentFadeToFail );
	}


	RefreshBeatToNoteSkin();

	/*
	 * Update all NoteDisplays.  Hack: We need to call this once per frame, not
	 * once per player.
	 */
	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	if( pn == GAMESTATE->m_MasterPlayerNumber )
		NoteDisplay::Update( fDeltaTime );
}

float NoteField::GetWidth()
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	float fMinX, fMaxX;
	// TODO: Remove use of PlayerNumber.
	pStyle->GetMinAndMaxColX( m_pPlayerState->m_PlayerNumber, fMinX, fMaxX );

	return fMaxX - fMinX + ARROW_SIZE;
}

void NoteField::DrawBeatBar( const float fBeat )
{
	bool bIsMeasure = fmodf( fBeat, (float)BEATS_PER_MEASURE ) == 0;
	int iMeasureIndex = (int)fBeat / BEATS_PER_MEASURE;
	int iMeasureNoDisplay = iMeasureIndex+1;

	NoteType nt = BeatToNoteType( fBeat );

	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos		= ArrowEffects::GetYPos(	m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );

	float fAlpha;
	int iState;
	
	if( bIsMeasure )
	{
		fAlpha = 1;
		iState = 0;
	}
	else
	{
		float fScrollSpeed = m_pPlayerState->m_CurrentPlayerOptions.m_fScrollSpeed;
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

void NoteField::DrawMarkerBar( int iBeat )
{
	float fBeat = NoteRowToBeat( iBeat );
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos		= ArrowEffects::GetYPos(	m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );


	m_rectMarkerBar.StretchTo( RectF(-GetWidth()/2, fYPos-ARROW_SIZE/2, GetWidth()/2, fYPos+ARROW_SIZE/2) );
	m_rectMarkerBar.Draw();
}

void NoteField::DrawAreaHighlight( int iStartBeat, int iEndBeat )
{
	float fStartBeat = NoteRowToBeat( iStartBeat );
	float fEndBeat = NoteRowToBeat( iEndBeat );
	float fYStartOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fStartBeat );
	float fYStartPos	= ArrowEffects::GetYPos(	m_pPlayerState, 0, fYStartOffset, m_fYReverseOffsetPixels );
	float fYEndOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fEndBeat );
	float fYEndPos		= ArrowEffects::GetYPos(	m_pPlayerState, 0, fYEndOffset, m_fYReverseOffsetPixels );

	// The caller should have clamped these to reasonable values
	ASSERT( fYStartPos > -1000 );
	ASSERT( fYEndPos < +5000 );

	m_rectAreaHighlight.StretchTo( RectF(-GetWidth()/2, fYStartPos, GetWidth()/2, fYEndPos) );
	m_rectAreaHighlight.SetDiffuse( RageColor(1,0,0,0.3f) );
	m_rectAreaHighlight.Draw();
}



void NoteField::DrawBPMText( const float fBeat, const float fBPM )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos		= ArrowEffects::GetYPos(	m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );

	m_textMeasureNumber.SetHorizAlign( Actor::align_right );
	m_textMeasureNumber.SetDiffuse( RageColor(1,0,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fBPM) );
	m_textMeasureNumber.SetXY( -GetWidth()/2.f - 60, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawFreezeText( const float fBeat, const float fSecs )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos		= ArrowEffects::GetYPos(	m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );

	m_textMeasureNumber.SetHorizAlign( Actor::align_right );
	m_textMeasureNumber.SetDiffuse( RageColor(0.8f,0.8f,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.2f", fSecs) );
	m_textMeasureNumber.SetXY( -GetWidth()/2.f - 10, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawBGChangeText( const float fBeat, const CString sNewBGName )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos		= ArrowEffects::GetYPos(	m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );

	m_textMeasureNumber.SetHorizAlign( Actor::align_left );
	m_textMeasureNumber.SetDiffuse( RageColor(0,1,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
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
float FindFirstDisplayedBeat( const PlayerState* pPlayerState, int iFirstPixelToDraw )
{
	float fFirstBeatToDraw = GAMESTATE->m_fSongBeat-4;	// Adjust to balance off performance and showing enough notes.

	while( fFirstBeatToDraw < GAMESTATE->m_fSongBeat )
	{
		float fYOffset = ArrowEffects::GetYOffset( pPlayerState, 0, fFirstBeatToDraw, true );
		if( fYOffset < iFirstPixelToDraw )	// off screen
			fFirstBeatToDraw += 0.1f;	// move toward fSongBeat
		else	// on screen
			break;	// stop probing
	}
	fFirstBeatToDraw -= 0.1f;	// rewind if we intentionally overshot
	return fFirstBeatToDraw;
}

float FindLastDisplayedBeat( const PlayerState* pPlayerState, int iLastPixelToDraw )
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
		float fYOffset = ArrowEffects::GetYOffset( pPlayerState, 0, fLastBeatToDraw, true );

		if( fYOffset > iLastPixelToDraw )	// off screen
			fLastBeatToDraw -= fSearchDistance;
		else	// on screen
			fLastBeatToDraw += fSearchDistance;

		fSearchDistance /= 2;
	}

	return fLastBeatToDraw;
}

bool NoteField::IsOnScreen( float fBeat, int iFirstPixelToDraw, int iLastPixelToDraw )
{
	// TRICKY: If boomerang is on, then ones in the range 
	// [iFirstIndexToDraw,iLastIndexToDraw] aren't necessarily visible.
	// Test to see if this beat is visible before drawing.
	float fYOffset = ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	if( fYOffset > iLastPixelToDraw )	// off screen
		return false;
	if( fYOffset < iFirstPixelToDraw )	// off screen
		return false;

	return true;
}

void NoteField::DrawPrimitives()
{
	//LOG->Trace( "NoteField::DrawPrimitives()" );

	/* This should be filled in on the first update. */
	ASSERT( !m_BeatToNoteDisplays.empty() );

	NoteDisplayCols *cur = SearchForSongBeat();
	cur->m_ReceptorArrowRow.Draw();

	const PlayerOptions &current_po = m_pPlayerState->m_CurrentPlayerOptions;

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
	
	iFirstPixelToDraw = (int)(iFirstPixelToDraw * fDrawScale);
	iLastPixelToDraw = (int)(iLastPixelToDraw * fDrawScale);


	// Probe for first and last notes on the screen
	float fFirstBeatToDraw = FindFirstDisplayedBeat( m_pPlayerState, iFirstPixelToDraw );
	float fLastBeatToDraw = FindLastDisplayedBeat( m_pPlayerState, iLastPixelToDraw );

	m_pPlayerState->m_fLastDrawnBeat = fLastBeatToDraw;

	const int iFirstIndexToDraw  = BeatToNoteRow(fFirstBeatToDraw);
	const int iLastIndexToDraw   = BeatToNoteRow(fLastBeatToDraw);

//	LOG->Trace( "start = %f.1, end = %f.1", fFirstBeatToDraw-fSongBeat, fLastBeatToDraw-fSongBeat );
//	LOG->Trace( "Drawing elements %d through %d", iFirstIndexToDraw, iLastIndexToDraw );

#define IS_ON_SCREEN( fBeat )  IsOnScreen( fBeat, iFirstPixelToDraw, iLastPixelToDraw )

	if( GAMESTATE->m_bEditing )
	{
		ASSERT(GAMESTATE->m_pCurSong);

		//
		// Draw beat bars
		//
		{
			float fStartDrawingMeasureBars = max( 0, Quantize(fFirstBeatToDraw-0.25f,0.25f) );
			for( float f=fStartDrawingMeasureBars; f<fLastBeatToDraw; f+=0.25f )
			{
				if( IS_ON_SCREEN(f) )
					DrawBeatBar( f );
			}
		}

		//
		// BPM text
		//
		vector<BPMSegment> &aBPMSegments = GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments;
		for( unsigned i=0; i<aBPMSegments.size(); i++ )
		{
			if( aBPMSegments[i].m_iStartIndex >= iFirstIndexToDraw &&
			    aBPMSegments[i].m_iStartIndex <= iLastIndexToDraw)
			{
				float fBeat = NoteRowToBeat(aBPMSegments[i].m_iStartIndex);
				if( IS_ON_SCREEN(fBeat) )
					DrawBPMText( fBeat, aBPMSegments[i].GetBPM() );
			}
		}
		//
		// Freeze text
		//
		vector<StopSegment> &aStopSegments = GAMESTATE->m_pCurSong->m_Timing.m_StopSegments;
		for( unsigned i=0; i<aStopSegments.size(); i++ )
		{
			if( aStopSegments[i].m_iStartRow >= iFirstIndexToDraw &&
			    aStopSegments[i].m_iStartRow <= iLastIndexToDraw)
			{
				float fBeat = NoteRowToBeat(aStopSegments[i].m_iStartRow);
				if( IS_ON_SCREEN(fBeat) )
					DrawFreezeText( fBeat, aStopSegments[i].m_fStopSeconds );
			}
		}

		//
		// BGChange text
		//
		switch( EDIT_MODE.GetValue() )
		{
		case EDIT_MODE_HOME:
		case EDIT_MODE_PRACTICE:
			break;
		case EDIT_MODE_FULL:
			{
				vector<BackgroundChange>::iterator iter[NUM_BackgroundLayer];
				FOREACH_BackgroundLayer( i )
					iter[i] = GAMESTATE->m_pCurSong->GetBackgroundChanges(i).begin();

				while( 1 )
				{
					float fLowestBeat = FLT_MAX;
					vector<BackgroundLayer> viLowestIndex;

					FOREACH_BackgroundLayer( i )
					{
						if( iter[i] == GAMESTATE->m_pCurSong->GetBackgroundChanges(i).end() )
							continue;

						float fBeat = iter[i]->m_fStartBeat;
						if( fBeat < fLowestBeat )
						{
							fLowestBeat = fBeat;
							viLowestIndex.clear();
							viLowestIndex.push_back( i );
						}
						else if( fBeat == fLowestBeat )
						{
							viLowestIndex.push_back( i );
						}
					}

					if( viLowestIndex.empty() )
					{
						FOREACH_BackgroundLayer( i )
							ASSERT( iter[i] == GAMESTATE->m_pCurSong->GetBackgroundChanges(i).end() );
						break;
					}

					if( IS_ON_SCREEN(fLowestBeat) )
					{
						vector<CString> vsBGChanges;
						FOREACH_CONST( BackgroundLayer, viLowestIndex, i )
						{
							ASSERT( iter[*i] != GAMESTATE->m_pCurSong->GetBackgroundChanges(*i).end() );

							const BackgroundChange& change = *iter[*i];
							vector<CString> vsParts;
							if( *i!=0 )								vsParts.push_back( ssprintf("%d: ",*i) );
							if( !change.m_def.m_sFile1.empty() )	vsParts.push_back( change.m_def.m_sFile1 );
							if( !change.m_def.m_sFile2.empty() )	vsParts.push_back( change.m_def.m_sFile2 );
							if( change.m_fRate!=1.0f )				vsParts.push_back( ssprintf("%.2f%%",change.m_fRate*100) );
							if( !change.m_sTransition.empty() )		vsParts.push_back( change.m_sTransition );
							if( !change.m_def.m_sEffect.empty() )	vsParts.push_back( change.m_def.m_sEffect );
							if( !change.m_def.m_sColor1.empty() )	vsParts.push_back( change.m_def.m_sColor1 );
							if( !change.m_def.m_sColor2.empty() )	vsParts.push_back( change.m_def.m_sColor2 );

							vsBGChanges.push_back( join("\n",vsParts) );
						}
						DrawBGChangeText( fLowestBeat, join("\n",vsBGChanges) );
					}
					FOREACH_CONST( BackgroundLayer, viLowestIndex, i )
						iter[*i]++;
				}
			}
			break;
		default:
			ASSERT(0);
		}

		//
		// Draw marker bars
		//
		if( m_iBeginMarker != -1  &&  m_iEndMarker != -1 )
		{
			int iBegin = m_iBeginMarker;
			int iEnd = m_iEndMarker;
			CLAMP( iBegin, iFirstIndexToDraw, iLastIndexToDraw );
			CLAMP( iEnd, iFirstIndexToDraw, iLastIndexToDraw );
			DrawAreaHighlight( iBegin, iEnd );
		}
		else if( m_iBeginMarker != -1 )
		{
			if( m_iBeginMarker >= iFirstIndexToDraw &&
				m_iBeginMarker <= iLastIndexToDraw )
				DrawMarkerBar( m_iBeginMarker );
		}
		else if( m_iEndMarker != -1 )
		{
			if( m_iEndMarker >= iFirstIndexToDraw &&
				m_iEndMarker <= iLastIndexToDraw )
			DrawMarkerBar( m_iEndMarker );
		}
	}



	//
	// Optimization is very important here because there are so many arrows to draw.  
	// Draw the arrows in order of column.  This minimize texture switches and let us
	// draw in big batches.
	//

	float fSelectedRangeGlow = SCALE( RageFastCos(RageTimer::GetTimeSinceStartFast()*2), -1, 1, 0.1f, 0.3f );

	for( int c=0; c<m_pNoteData->GetNumTracks(); c++ )	// for each arrow column
	{
		// TODO: Remove use of PlayerNumber.
		PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
		NoteFieldMode::BeginDrawTrack( pn, c );

		//
		// Draw all HoldNotes in this column (so that they appear under the tap notes)
		//	
		NDMap::iterator CurDisplay = m_BeatToNoteDisplays.begin();
		ASSERT( CurDisplay != m_BeatToNoteDisplays.end() );
		NDMap::iterator NextDisplay = CurDisplay; ++NextDisplay;

		{
			NoteData::TrackMap::const_iterator begin, end;
			m_pNoteData->GetTapNoteRangeInclusive( c, iFirstIndexToDraw, iLastIndexToDraw+1, begin, end );

			for( ; begin != end; ++begin )
			{	
				const TapNote &tn = begin->second; //m_pNoteData->GetTapNote(c, i);
				if( tn.type != TapNote::hold_head )
					continue;	// skip

				const HoldNoteResult &Result = tn.HoldResult;
				if( Result.hns == HNS_OK )	// if this HoldNote was completed
					continue;	// don't draw anything

				int iStartRow = begin->first;
				int iEndRow = iStartRow + tn.iDuration;

				// TRICKY: If boomerang is on, then all notes in the range 
				// [iFirstIndexToDraw,iLastIndexToDraw] aren't necessarily visible.
				// Test every note to make sure it's on screen before drawing
				float fThrowAway;
				bool bStartIsPastPeak = false;
				bool bEndIsPastPeak = false;
				float fStartYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, c, NoteRowToBeat(iStartRow), fThrowAway, bStartIsPastPeak );
				float fEndYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, c, NoteRowToBeat(iEndRow), fThrowAway, bEndIsPastPeak );

				bool bTailIsOnVisible = iFirstPixelToDraw <= fEndYOffset && fEndYOffset <= iLastPixelToDraw;
				bool bHeadIsVisible = iFirstPixelToDraw <= fStartYOffset  && fStartYOffset <= iLastPixelToDraw;
				bool bStraddlingVisible = fStartYOffset <= iFirstPixelToDraw && iLastPixelToDraw <= fEndYOffset;
				bool bStaddlingPeak = bStartIsPastPeak && !bEndIsPastPeak;
				if( !(bTailIsOnVisible || bHeadIsVisible || bStraddlingVisible || bStaddlingPeak) )
				{
					//LOG->Trace( "skip drawing this hold." );
					continue;	// skip
				}

				const bool bIsActive = tn.HoldResult.bActive;
				const bool bIsHoldingNote = tn.HoldResult.bHeld;
				if( bIsActive )
					SearchForSongBeat()->m_GhostArrowRow.SetHoldIsActive( c );
				
				ASSERT_M( NoteRowToBeat(iStartRow) > -2000, ssprintf("%i %i %i", iStartRow, iEndRow, c) );
				SearchForBeat( CurDisplay, NextDisplay, NoteRowToBeat(iStartRow) );

				bool bIsInSelectionRange = false;
				if( m_iBeginMarker!=-1 && m_iEndMarker!=-1 )
					bIsInSelectionRange = (m_iBeginMarker <= iStartRow && iEndRow < m_iEndMarker);

				NoteDisplayCols *nd = CurDisplay->second;
				nd->display[c].DrawHold( tn, c, iStartRow, bIsHoldingNote, bIsActive, Result, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, false, m_fYReverseOffsetPixels, (float) iFirstPixelToDraw, (float) iLastPixelToDraw );
			}

		}
		

		//
		// Draw all TapNotes in this column
		//
		CurDisplay = m_BeatToNoteDisplays.begin();
		NextDisplay = CurDisplay; ++NextDisplay;

		// draw notes from furthest to closest

		NoteData::TrackMap::const_iterator begin, end;
		m_pNoteData->GetTapNoteRange( c, iFirstIndexToDraw, iLastIndexToDraw+1, begin, end );
		for( ; begin != end; ++begin )
		{	
			int i = begin->first;
			const TapNote &tn = begin->second; //m_pNoteData->GetTapNote(c, i);
			switch( tn.type )
			{
			case TapNote::empty: // no note here
			case TapNote::hold_head:
				continue;	// skip
			}

			/* Don't draw hidden (fully judged) steps. */
			if( tn.result.bHidden )
				continue;


			// TRICKY: If boomerang is on, then all notes in the range 
			// [iFirstIndexToDraw,iLastIndexToDraw] aren't necessarily visible.
			// Test every note to make sure it's on screen before drawing
			if( !IS_ON_SCREEN(NoteRowToBeat(i)) )
				continue;	// skip

			ASSERT_M( NoteRowToBeat(i) > -2000, ssprintf("%i %i %i, %f %f", i, iLastIndexToDraw, iFirstIndexToDraw, GAMESTATE->m_fSongBeat, GAMESTATE->m_fMusicSeconds) );
			SearchForBeat( CurDisplay, NextDisplay, NoteRowToBeat(i) );
			NoteDisplayCols *nd = CurDisplay->second;

			// See if there is a hold step that begins on this index.  Only do this
			// if the note skin cares.
			bool bHoldNoteBeginsOnThisBeat = false;
			if( nd->display[c].DrawHoldHeadForTapsOnSameRow() )
			{
				for( int c2=0; c2<m_pNoteData->GetNumTracks(); c2++ )
				{
					if( m_pNoteData->GetTapNote(c2, i).type == TapNote::hold_head)
					{
						bHoldNoteBeginsOnThisBeat = true;
						break;
					}
				}
			}

			bool bIsInSelectionRange = false;
			if( m_iBeginMarker!=-1 && m_iEndMarker!=-1 )
				bIsInSelectionRange = m_iBeginMarker<=i && i<m_iEndMarker;

			bool bIsAddition = (tn.source == TapNote::addition);
			bool bIsMine = (tn.type == TapNote::mine);
			bool bIsAttack = (tn.type == TapNote::attack);

			if( bIsAttack )
			{
				Sprite sprite;
				sprite.Load( THEME->GetPathG("NoteField","attack "+tn.sAttackModifiers) );
				float fBeat = NoteRowToBeat(i);
				nd->display[c].DrawActor( &sprite, c, fBeat, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels, false );
			}
			else
			{
				nd->display[c].DrawTap( c, NoteRowToBeat(i), bHoldNoteBeginsOnThisBeat, bIsAddition, bIsMine, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels );
			}
		}


		NoteFieldMode::EndDrawTrack( c );
	}

	cur->m_GhostArrowRow.Draw();
}

void NoteField::FadeToFail()
{
	m_fPercentFadeToFail = max( 0.0f, m_fPercentFadeToFail );	// this will slowly increase every Update()
		// don't fade all over again if this is called twice
}

void NoteField::Step( int iCol, TapNoteScore score ) { SearchForSongBeat()->m_ReceptorArrowRow.Step( iCol, score ); }
void NoteField::SetPressed( int iCol ) { SearchForSongBeat()->m_ReceptorArrowRow.SetPressed( iCol ); }
void NoteField::DidTapNote( int iCol, TapNoteScore score, bool bBright ) { SearchForSongBeat()->m_GhostArrowRow.DidTapNote( iCol, score, bBright ); }
void NoteField::DidHoldNote( int iCol, HoldNoteScore score, bool bBright ) { SearchForSongBeat()->m_GhostArrowRow.DidHoldNote( iCol, score, bBright ); }

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
