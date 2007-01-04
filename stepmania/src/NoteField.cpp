#include "global.h"
#include "NoteField.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ArrowEffects.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageMath.h"
#include "ThemeManager.h"
#include "NoteSkinManager.h"
#include "song.h"
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "Style.h"
#include "CommonMetrics.h"
#include <float.h>
#include "BackgroundUtil.h"
#include "Course.h"
#include "NoteData.h"

static ThemeMetric<bool> SHOW_BOARD( "NoteField", "ShowBoard" );
static ThemeMetric<bool> SHOW_BEAT_BARS( "NoteField", "ShowBeatBars" );
static ThemeMetric<float> FADE_BEFORE_TARGETS_PERCENT( "NoteField", "FadeBeforeTargetsPercent" );
static ThemeMetric<float> BAR_MEASURE_ALPHA( "NoteField", "BarMeasureAlpha" );
static ThemeMetric<float> BAR_4TH_ALPHA( "NoteField", "Bar4thAlpha" );
static ThemeMetric<float> BAR_8TH_ALPHA( "NoteField", "Bar8thAlpha" );
static ThemeMetric<float> BAR_16TH_ALPHA( "NoteField", "Bar16thAlpha" );

NoteField::NoteField()
{	
	m_pNoteData = NULL;
	m_pCurDisplay = NULL;

	m_textMeasureNumber.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textMeasureNumber.SetZoom( 1.0f );
	m_textMeasureNumber.SetShadowLength( 2 );
	m_textMeasureNumber.SetWrapWidthPixels( 300 );

	m_rectMarkerBar.SetEffectDiffuseShift( 2, RageColor(1,1,1,0.5f), RageColor(0.5f,0.5f,0.5f,0.5f) );

	m_sprBoard.Load( THEME->GetPathG("NoteField","board") );
	m_sprBoard->PlayCommand( "On" );
	this->AddChild( m_sprBoard );

	m_fBoardOffsetPixels = 0;
	m_fCurrentBeatLastUpdate = -1;
	m_fYPosCurrentBeatLastUpdate = -1;
	this->SubscribeToMessage( Message_CurrentSongChanged );

	m_sprBeatBars.Load( THEME->GetPathG("NoteField","bars") );
	m_sprBeatBars.StopAnimating();

	m_iBeginMarker = m_iEndMarker = -1;

	m_fPercentFadeToFail = -1;
}

NoteField::~NoteField()
{
	Unload();
}

void NoteField::Unload()
{
	for( map<RString, NoteDisplayCols *>::iterator it = m_NoteDisplays.begin();
		it != m_NoteDisplays.end(); ++it )
		delete it->second;
	m_NoteDisplays.clear();
	m_pCurDisplay = NULL;
	memset( m_pDisplays, 0, sizeof(m_pDisplays) );
}

void NoteField::CacheNoteSkin( const RString &sNoteSkin_ )
{
	RString sNoteSkin = sNoteSkin_;
	sNoteSkin.MakeLower();

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

void NoteField::UncacheNoteSkin( const RString &sNoteSkin_ )
{
	RString sNoteSkin( sNoteSkin_ );
	sNoteSkin.MakeLower();

	LOG->Trace("NoteField::CacheNoteSkin: release %s", sNoteSkin.c_str() );
	ASSERT_M( m_NoteDisplays.find(sNoteSkin) != m_NoteDisplays.end(), sNoteSkin );
	delete m_NoteDisplays[sNoteSkin];
	m_NoteDisplays.erase( sNoteSkin );
}

void NoteField::CacheAllUsedNoteSkins()
{
	/* Cache all note skins that we might need for the whole song, course or battle
	 * play, so we don't have to load them later (such as between course songs). */
	vector<RString> asSkins;
	GAMESTATE->GetAllUsedNoteSkins( asSkins );
	asSkins.push_back( m_pPlayerState->m_PlayerOptions.GetStage().m_sNoteSkin );

	for( unsigned i=0; i < asSkins.size(); ++i )
		CacheNoteSkin( asSkins[i] );

	/* If we're changing note skins in the editor, we can have old note skins lying
	 * around.  Remove them so they don't accumulate. */
	set<RString> setNoteSkinsToUnload;
	FOREACHM( RString, NoteDisplayCols *, m_NoteDisplays, d )
	{
		if( find(asSkins.begin(), asSkins.end(), d->first) == asSkins.end() )
			setNoteSkinsToUnload.insert( d->first );
	}
	FOREACHS( RString, setNoteSkinsToUnload, s )
		UncacheNoteSkin( *s );

	map<RString, NoteDisplayCols *>::iterator it = m_NoteDisplays.find( m_pPlayerState->m_PlayerOptions.GetCurrent().m_sNoteSkin );
	ASSERT_M( it != m_NoteDisplays.end(), m_pPlayerState->m_PlayerOptions.GetCurrent().m_sNoteSkin );
	m_pCurDisplay = it->second;
	memset( m_pDisplays, 0, sizeof(m_pDisplays) );
	FOREACH_EnabledPlayer( pn )
	{
		const RString& sNoteSkin = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_sNoteSkin;
		it = m_NoteDisplays.find( sNoteSkin );
		ASSERT_M( it != m_NoteDisplays.end(), sNoteSkin );
		m_pDisplays[pn] = it->second;
	}
}

void NoteField::Init( const PlayerState* pPlayerState, float fYReverseOffsetPixels )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffsetPixels;
	CacheAllUsedNoteSkins();
}

void NoteField::Load( 
	const NoteData *pNoteData,
	int iDrawDistanceAfterTargetsPixels, 
	int iDrawDistanceBeforeTargetsPixels )
{
	ASSERT( pNoteData );
	m_pNoteData = pNoteData;
	m_iDrawDistanceAfterTargetsPixels = iDrawDistanceAfterTargetsPixels;
	m_iDrawDistanceBeforeTargetsPixels = iDrawDistanceBeforeTargetsPixels;
	ASSERT( m_iDrawDistanceBeforeTargetsPixels >= m_iDrawDistanceAfterTargetsPixels );

	m_fPercentFadeToFail = -1;

	//int i1 = m_pNoteData->GetNumTracks();
	//int i2 = GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer;

	ASSERT_M( m_pNoteData->GetNumTracks() == GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer, 
		ssprintf("%d = %d",m_pNoteData->GetNumTracks(), GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer) );

	// The note skin may have changed at the beginning of a new course song.
	map<RString, NoteDisplayCols *>::iterator it = m_NoteDisplays.find( m_pPlayerState->m_PlayerOptions.GetCurrent().m_sNoteSkin );
	ASSERT_M( it != m_NoteDisplays.end(), m_pPlayerState->m_PlayerOptions.GetCurrent().m_sNoteSkin );
	m_pCurDisplay = it->second;
	memset( m_pDisplays, 0, sizeof(m_pDisplays) );
	FOREACH_EnabledPlayer( pn )
	{
		const RString& sNoteSkin = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_sNoteSkin;
		it = m_NoteDisplays.find( sNoteSkin );
		ASSERT_M( it != m_NoteDisplays.end(), sNoteSkin );
		m_pDisplays[pn] = it->second;
	}
}

void NoteField::Update( float fDeltaTime )
{
	if( m_bFirstUpdate )
	{
		m_pCurDisplay->m_ReceptorArrowRow.PlayCommand( "On" );
	}

	ActorFrame::Update( fDeltaTime );

	// update m_fBoardOffsetPixels, m_fCurrentBeatLastUpdate, m_fYPosCurrentBeatLastUpdate
	const float fCurrentBeat = GAMESTATE->m_fSongBeat;
	bool bTweeningOn = m_sprBoard->GetCurrentDiffuseAlpha() >= 0.98  &&  m_sprBoard->GetCurrentDiffuseAlpha() < 1.00;	// HACK
	if( !bTweeningOn  &&  m_fCurrentBeatLastUpdate != -1 )
	{
		const float fYOffsetLast	= ArrowEffects::GetYOffset( m_pPlayerState, 0, m_fCurrentBeatLastUpdate );
		const float fYPosLast		= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffsetLast, m_fYReverseOffsetPixels );
		const float fPixelDifference = fYPosLast - m_fYPosCurrentBeatLastUpdate;
	
		//LOG->Trace( "speed = %f, %f, %f, %f, %f, %f", fSpeed, fYOffsetAtCurrent, fYOffsetAtNext, fSecondsAtCurrent, fSecondsAtNext, fPixelDifference, fSecondsDifference );

		m_fBoardOffsetPixels += fPixelDifference;
		wrap( m_fBoardOffsetPixels, m_sprBoard->GetUnzoomedHeight() );
	}
	m_fCurrentBeatLastUpdate = fCurrentBeat;
	const float fYOffsetCurrent	= ArrowEffects::GetYOffset( m_pPlayerState, 0, m_fCurrentBeatLastUpdate );
	m_fYPosCurrentBeatLastUpdate	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffsetCurrent, m_fYReverseOffsetPixels );


	m_rectMarkerBar.Update( fDeltaTime );

	NoteDisplayCols *cur = m_pCurDisplay;

	cur->m_ReceptorArrowRow.Update( fDeltaTime );
	cur->m_GhostArrowRow.Update( fDeltaTime );

	if( m_fPercentFadeToFail >= 0 )
		m_fPercentFadeToFail = min( m_fPercentFadeToFail + fDeltaTime/1.5f, 1 );	// take 1.5 seconds to totally fade


	// Update fade to failed
	m_pCurDisplay->m_ReceptorArrowRow.SetFadeToFailPercent( m_fPercentFadeToFail );


	/*
	 * Update all NoteDisplays.  Hack: We need to call this once per frame, not
	 * once per player.
	 */
	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	if( pn == GAMESTATE->m_MasterPlayerNumber )
		NoteDisplay::Update( fDeltaTime );
}

float NoteField::GetWidth() const
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	float fMinX, fMaxX;
	// TODO: Remove use of PlayerNumber.
	pStyle->GetMinAndMaxColX( m_pPlayerState->m_PlayerNumber, fMinX, fMaxX );

	const float fYZoom	= ArrowEffects::GetZoom( m_pPlayerState );
	return (fMaxX - fMinX + ARROW_SIZE) * fYZoom;
}

void NoteField::DrawBeatBar( const float fBeat )
{
	bool bIsMeasure = fmodf( fBeat, (float)BEATS_PER_MEASURE ) == 0;
	int iMeasureIndex = (int)fBeat / BEATS_PER_MEASURE;
	int iMeasureNoDisplay = iMeasureIndex+1;

	NoteType nt = BeatToNoteType( fBeat );

	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );

	float fAlpha;
	int iState;
	
	if( bIsMeasure )
	{
		fAlpha = BAR_MEASURE_ALPHA;
		iState = 0;
	}
	else
	{
		float fScrollSpeed = m_pPlayerState->m_PlayerOptions.GetCurrent().m_fScrollSpeed;
		if( m_pPlayerState->m_PlayerOptions.GetCurrent().m_fTimeSpacing > 0 )
			fScrollSpeed = 4;
		switch( nt )
		{
		default:	ASSERT(0);
		case NOTE_TYPE_4TH:	fAlpha = BAR_4TH_ALPHA;						iState = 1;	break;
		case NOTE_TYPE_8TH:	fAlpha = SCALE(fScrollSpeed,1.0f,2.0f,0.0f,BAR_8TH_ALPHA);	iState = 2;	break;
		case NOTE_TYPE_16TH:	fAlpha = SCALE(fScrollSpeed,2.0f,4.0f,0.0f,BAR_16TH_ALPHA);	iState = 3;	break;
		}
		CLAMP( fAlpha, 0, 1 );
	}

	float fWidth = GetWidth();
	float fFrameWidth = m_sprBeatBars.GetUnzoomedWidth();

	m_sprBeatBars.SetX( 0 );
	m_sprBeatBars.SetY( fYPos );
	m_sprBeatBars.SetDiffuse( RageColor(1,1,1,fAlpha) );
	m_sprBeatBars.SetState( iState );
	m_sprBeatBars.SetCustomTextureRect( RectF(0,SCALE(iState,0.f,4.f,0.f,1.f), fWidth/fFrameWidth, SCALE(iState+1,0.f,4.f,0.f,1.f)) );
	m_sprBeatBars.SetZoomX( fWidth/m_sprBeatBars.GetUnzoomedWidth() );
	m_sprBeatBars.Draw();


	if( GAMESTATE->IsEditing()  &&  bIsMeasure )
	{
		m_textMeasureNumber.SetDiffuse( RageColor(1,1,1,1) );
		m_textMeasureNumber.SetGlow( RageColor(1,1,1,0) );
		m_textMeasureNumber.SetHorizAlign( align_right );
		m_textMeasureNumber.SetText( ssprintf("%d", iMeasureNoDisplay) );
		m_textMeasureNumber.SetXY( -fWidth/2, fYPos );
		m_textMeasureNumber.Draw();
	}
}

void NoteField::DrawBoard( int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels )
{
	const float fYPosAt0		= ArrowEffects::GetYPos(    m_pPlayerState, 0, 0, m_fYReverseOffsetPixels );	

	// Draw the board centered on fYPosAt0 so that the board doesn't slide as the draw distance changes with modifiers.
	
	Sprite *pSprite = dynamic_cast<Sprite *>( (Actor*)m_sprBoard );
	if( pSprite == NULL )
		RageException::Throw( "Board must be a Sprite" );


	RectF rect = *pSprite->GetCurrentTextureCoordRect();
	const float fBoardGraphicHeightPixels = pSprite->GetUnzoomedHeight();
	float fTexCoordOffset = m_fBoardOffsetPixels / fBoardGraphicHeightPixels;
	{
		// top half
		const float fHeight = iDrawDistanceBeforeTargetsPixels - iDrawDistanceAfterTargetsPixels;
		const float fY = fYPosAt0 - ((iDrawDistanceBeforeTargetsPixels + iDrawDistanceAfterTargetsPixels) / 2.0f);

		pSprite->ZoomToHeight( fHeight );
		pSprite->SetY( fY );

		rect.top = -fTexCoordOffset-(iDrawDistanceBeforeTargetsPixels/fBoardGraphicHeightPixels);
		rect.bottom = -fTexCoordOffset+(-iDrawDistanceAfterTargetsPixels/fBoardGraphicHeightPixels);
		pSprite->SetCustomTextureRect( rect );
		float fFadeTop = FADE_BEFORE_TARGETS_PERCENT * iDrawDistanceBeforeTargetsPixels / (iDrawDistanceBeforeTargetsPixels-iDrawDistanceAfterTargetsPixels);
		pSprite->SetFadeTop( fFadeTop );
		pSprite->Draw();
	}
}

void NoteField::DrawMarkerBar( int iBeat )
{
	float fBeat = NoteRowToBeat( iBeat );
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );


	m_rectMarkerBar.StretchTo( RectF(-GetWidth()/2, fYPos-ARROW_SIZE/2, GetWidth()/2, fYPos+ARROW_SIZE/2) );
	m_rectMarkerBar.Draw();
}

void NoteField::DrawAreaHighlight( int iStartBeat, int iEndBeat )
{
	float fStartBeat = NoteRowToBeat( iStartBeat );
	float fEndBeat = NoteRowToBeat( iEndBeat );
	float fDrawDistanceAfterTargetsPixels	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fStartBeat );
	float fYStartPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fDrawDistanceAfterTargetsPixels, m_fYReverseOffsetPixels );
	float fDrawDistanceBeforeTargetsPixels	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fEndBeat );
	float fYEndPos		= ArrowEffects::GetYPos(    m_pPlayerState, 0, fDrawDistanceBeforeTargetsPixels, m_fYReverseOffsetPixels );

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
	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( align_right );
	m_textMeasureNumber.SetDiffuse( RageColor(1,0,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.3f", fBPM) );
	m_textMeasureNumber.SetXY( -GetWidth()/2 - 60*fZoom, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawFreezeText( const float fBeat, const float fSecs )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( align_right );
	m_textMeasureNumber.SetDiffuse( RageColor(0.8f,0.8f,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%.3f", fSecs) );
	m_textMeasureNumber.SetXY( -GetWidth()/2.f - 10*fZoom, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawAttackText( const float fBeat, const Attack &attack )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( align_left );
	m_textMeasureNumber.SetDiffuse( RageColor(0,0.8f,0.8f,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( attack.GetTextDescription() );
	m_textMeasureNumber.SetXY( +GetWidth()/2.f + 10*fZoom, fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawBGChangeText( const float fBeat, const RString sNewBGName )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( align_left );
	m_textMeasureNumber.SetDiffuse( RageColor(0,1,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( sNewBGName );
	m_textMeasureNumber.SetXY( +GetWidth()/2.f, fYPos );
	m_textMeasureNumber.Draw();
}

// CPU OPTIMIZATION OPPORTUNITY:
// change this probing to binary search
float FindFirstDisplayedBeat( const PlayerState* pPlayerState, int iDrawDistanceAfterTargetsPixels )
{
	float fFirstBeatToDraw = GAMESTATE->m_fSongBeat-4;	// Adjust to balance off performance and showing enough notes.

	/* In Boomerang, we'll usually have two sections of notes: before and after
	 * the peak.  We always start drawing before the peak, and end after it, or
	 * we may falsely detect the off-screen portion as the end (or beginning)
	 * of the stream. */
	bool bBoomerang;
	{
		const float* fAccels = pPlayerState->m_PlayerOptions.GetCurrent().m_fAccels;
		bBoomerang = (fAccels[PlayerOptions::ACCEL_BOOMERANG] != 0);
	}

	while( fFirstBeatToDraw < GAMESTATE->m_fSongBeat )
	{
		bool bIsPastPeakYOffset;
		float fPeakYOffset;
		float fYOffset = ArrowEffects::GetYOffset( pPlayerState, 0, fFirstBeatToDraw, fPeakYOffset, bIsPastPeakYOffset, true );

		if( bBoomerang && bIsPastPeakYOffset )
			break;	// stop probing
		else if( fYOffset < iDrawDistanceAfterTargetsPixels )	// off screen
			fFirstBeatToDraw += 0.1f;	// move toward fSongBeat
		else	// on screen
			break;	// stop probing
	}
	fFirstBeatToDraw -= 0.1f;	// rewind if we intentionally overshot
	return fFirstBeatToDraw;
}

float FindLastDisplayedBeat( const PlayerState* pPlayerState, int iDrawDistanceBeforeTargetsPixels )
{
	//
	// Probe for last note to draw.
	// worst case is 0.25x + boost.  Adjust search distance to 
	// so that notes don't pop onto the screen.
	//
	float fSearchDistance = 10;
	float fLastBeatToDraw = GAMESTATE->m_fSongBeat+fSearchDistance;	

	const int NUM_ITERATIONS = 20;

	bool bBoomerang;
	{
		const float* fAccels = pPlayerState->m_PlayerOptions.GetCurrent().m_fAccels;
		bBoomerang = (fAccels[PlayerOptions::ACCEL_BOOMERANG] != 0);
	}

	for( int i=0; i<NUM_ITERATIONS; i++ )
	{
		bool bIsPastPeakYOffset;
		float fPeakYOffset;
		float fYOffset = ArrowEffects::GetYOffset( pPlayerState, 0, fLastBeatToDraw, fPeakYOffset, bIsPastPeakYOffset, true );

		if( bBoomerang && !bIsPastPeakYOffset )
			fLastBeatToDraw += fSearchDistance;
		else if( fYOffset > iDrawDistanceBeforeTargetsPixels )	// off screen
			fLastBeatToDraw -= fSearchDistance;
		else	// on screen
			fLastBeatToDraw += fSearchDistance;

		fSearchDistance /= 2;
	}

	return fLastBeatToDraw;
}

bool NoteField::IsOnScreen( float fBeat, int iCol, int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels ) const
{
	// TRICKY: If boomerang is on, then ones in the range 
	// [iFirstIndexToDraw,iLastIndexToDraw] aren't necessarily visible.
	// Test to see if this beat is visible before drawing.
	float fYOffset = ArrowEffects::GetYOffset( m_pPlayerState, iCol, fBeat );
	if( fYOffset > iDrawDistanceBeforeTargetsPixels )	// off screen
		return false;
	if( fYOffset < iDrawDistanceAfterTargetsPixels )	// off screen
		return false;

	return true;
}

void NoteField::DrawPrimitives()
{
	//LOG->Trace( "NoteField::DrawPrimitives()" );

	/* This should be filled in on the first update. */
	ASSERT( m_pCurDisplay != NULL );

	ArrowEffects::Update();

	NoteDisplayCols *cur = m_pCurDisplay;

	const PlayerOptions &current_po = m_pPlayerState->m_PlayerOptions.GetCurrent();

	//
	// Adjust draw range depending on some effects
	//
	int iDrawDistanceAfterTargetsPixels = m_iDrawDistanceAfterTargetsPixels;
	// HACK: if boomerang and centered are on, then we want to draw much 
	// earlier to that the notes don't pop on screen.
	float fCenteredTimesBoomerang = 
		current_po.m_fScrolls[PlayerOptions::SCROLL_CENTERED] * 
		current_po.m_fAccels[PlayerOptions::ACCEL_BOOMERANG];
	iDrawDistanceAfterTargetsPixels += int(SCALE( fCenteredTimesBoomerang, 0.f, 1.f, 0.f, -SCREEN_HEIGHT/2 ));
	int iDrawDistanceBeforeTargetsPixels = m_iDrawDistanceBeforeTargetsPixels;
	
	float fDrawScale = 1;
	fDrawScale *= 1 + 0.5f * fabsf( current_po.m_fPerspectiveTilt );
	fDrawScale *= 1 + fabsf( current_po.m_fEffects[PlayerOptions::EFFECT_TINY] );
	
	iDrawDistanceAfterTargetsPixels = (int)(iDrawDistanceAfterTargetsPixels * fDrawScale);
	iDrawDistanceBeforeTargetsPixels = (int)(iDrawDistanceBeforeTargetsPixels * fDrawScale);


	// Probe for first and last notes on the screen
	float fFirstBeatToDraw = FindFirstDisplayedBeat( m_pPlayerState, iDrawDistanceAfterTargetsPixels );
	float fLastBeatToDraw = FindLastDisplayedBeat( m_pPlayerState, iDrawDistanceBeforeTargetsPixels );

	m_pPlayerState->m_fLastDrawnBeat = fLastBeatToDraw;

	const int iFirstIndexToDraw  = BeatToNoteRow(fFirstBeatToDraw);
	const int iLastIndexToDraw   = BeatToNoteRow(fLastBeatToDraw);

//	LOG->Trace( "start = %f.1, end = %f.1", fFirstBeatToDraw-fSongBeat, fLastBeatToDraw-fSongBeat );
//	LOG->Trace( "Drawing elements %d through %d", iFirstIndexToDraw, iLastIndexToDraw );

#define IS_ON_SCREEN( fBeat )  IsOnScreen( fBeat, 0, iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels )

	//
	// Draw board
	//
	if( SHOW_BOARD )
	{
		DrawBoard( iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels );
	}

	//
	// Draw Receptors
	//
	{
		cur->m_ReceptorArrowRow.Draw();
	}

	//
	// Draw beat bars
	//
	if( GAMESTATE->IsEditing() || SHOW_BEAT_BARS )
	{
		float fStartDrawingMeasureBars = max( 0, Quantize(fFirstBeatToDraw-0.25f,0.25f) );
		for( float f=fStartDrawingMeasureBars; f<fLastBeatToDraw; f+=0.25f )
		{
			if( IS_ON_SCREEN(f) )
				DrawBeatBar( f );
		}
	}

	if( GAMESTATE->IsEditing() )
	{
		ASSERT(GAMESTATE->m_pCurSong);

		//
		// BPM text
		//
		const vector<BPMSegment> &aBPMSegments = GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments;
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
		const vector<StopSegment> &aStopSegments = GAMESTATE->m_pCurSong->m_Timing.m_StopSegments;
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
		// Course mods text
		//
		const Course *pCourse = GAMESTATE->m_pCurCourse;
		if( pCourse )
		{
			ASSERT_M( GAMESTATE->m_iEditCourseEntryIndex > 0  &&  GAMESTATE->m_iEditCourseEntryIndex < (int)pCourse->m_vEntries.size(), 
				ssprintf("%i",GAMESTATE->m_iEditCourseEntryIndex.Get()) );
			const CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
			FOREACH_CONST( Attack, ce.attacks, a )
			{
				float fSecond = a->fStartSecond;
				float fBeat = GAMESTATE->m_pCurSong->m_Timing.GetBeatFromElapsedTime( fSecond );

				if( BeatToNoteRow(fBeat) >= iFirstIndexToDraw &&
					BeatToNoteRow(fBeat) <= iLastIndexToDraw)
				{
					if( IS_ON_SCREEN(fBeat) )
						DrawAttackText( fBeat, *a );
				}
			}
		}

		//
		// BGChange text
		//
		switch( GAMESTATE->m_EditMode )
		{
		case EditMode_Home:
		case EditMode_CourseMods:
		case EditMode_Practice:
			break;
		case EditMode_Full:
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
						vector<RString> vsBGChanges;
						FOREACH_CONST( BackgroundLayer, viLowestIndex, i )
						{
							ASSERT( iter[*i] != GAMESTATE->m_pCurSong->GetBackgroundChanges(*i).end() );
							const BackgroundChange& change = *iter[*i];
							RString s = change.GetTextDescription();
							if( *i!=0 )
								s = ssprintf("%d: ",*i) + s;
							vsBGChanges.push_back( s );
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
		//
		// Draw all HoldNotes in this column (so that they appear under the tap notes)
		//	
		{
			NoteData::TrackMap::const_iterator begin, end;
			m_pNoteData->GetTapNoteRangeInclusive( c, iFirstIndexToDraw, iLastIndexToDraw+1, begin, end );

			for( ; begin != end; ++begin )
			{	
				const TapNote &tn = begin->second; //m_pNoteData->GetTapNote(c, i);
				if( tn.type != TapNote::hold_head )
					continue;	// skip

				const HoldNoteResult &Result = tn.HoldResult;
				if( Result.hns == HNS_Held )	// if this HoldNote was completed
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

				bool bTailIsOnVisible = iDrawDistanceAfterTargetsPixels <= fEndYOffset && fEndYOffset <= iDrawDistanceBeforeTargetsPixels;
				bool bHeadIsVisible = iDrawDistanceAfterTargetsPixels <= fStartYOffset  && fStartYOffset <= iDrawDistanceBeforeTargetsPixels;
				bool bStraddlingVisible = fStartYOffset <= iDrawDistanceAfterTargetsPixels && iDrawDistanceBeforeTargetsPixels <= fEndYOffset;
				bool bStaddlingPeak = bStartIsPastPeak && !bEndIsPastPeak;
				if( !(bTailIsOnVisible || bHeadIsVisible || bStraddlingVisible || bStaddlingPeak) )
				{
					//LOG->Trace( "skip drawing this hold." );
					continue;	// skip
				}

				bool bIsAddition = (tn.source == TapNote::addition);
				bool bIsHopoPossible = (tn.bHopoPossible);
				bool bUseAdditionColoring = bIsAddition || bIsHopoPossible;
				const bool bIsActive = tn.HoldResult.bActive;
				const bool bIsHoldingNote = tn.HoldResult.bHeld;
				if( bIsActive )
					m_pCurDisplay->m_GhostArrowRow.SetHoldIsActive( c );
				
				ASSERT_M( NoteRowToBeat(iStartRow) > -2000, ssprintf("%i %i %i", iStartRow, iEndRow, c) );

				bool bIsInSelectionRange = false;
				if( m_iBeginMarker!=-1 && m_iEndMarker!=-1 )
					bIsInSelectionRange = (m_iBeginMarker <= iStartRow && iEndRow < m_iEndMarker);
				
				NoteDisplayCols *displayCols = tn.pn == PLAYER_INVALID ? m_pCurDisplay : m_pDisplays[tn.pn];
				displayCols->display[c].DrawHold( tn, c, iStartRow, bIsHoldingNote, bIsActive, Result, bUseAdditionColoring, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 
					false, m_fYReverseOffsetPixels, (float) iDrawDistanceAfterTargetsPixels, (float) iDrawDistanceBeforeTargetsPixels, iDrawDistanceBeforeTargetsPixels, FADE_BEFORE_TARGETS_PERCENT );
			}

		}
		

		//
		// Draw all TapNotes in this column
		//

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
			if( !IsOnScreen( NoteRowToBeat(i), c, iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels ) )
				continue;	// skip

			ASSERT_M( NoteRowToBeat(i) > -2000, ssprintf("%i %i %i, %f %f", i, iLastIndexToDraw, iFirstIndexToDraw, GAMESTATE->m_fSongBeat, GAMESTATE->m_fMusicSeconds) );

			// See if there is a hold step that begins on this index.  Only do this
			// if the note skin cares.
			bool bHoldNoteBeginsOnThisBeat = false;
			if( m_pCurDisplay->display[c].DrawHoldHeadForTapsOnSameRow() )
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
			bool bIsHopoPossible = (tn.bHopoPossible);
			bool bUseAdditionColoring = bIsAddition || bIsHopoPossible;
			bool bIsAttack = (tn.type == TapNote::attack);
			NoteDisplayCols *displayCols = tn.pn == PLAYER_INVALID ? m_pCurDisplay : m_pDisplays[tn.pn];
			
			if( bIsAttack )
			{
				Sprite sprite;
				sprite.Load( THEME->GetPathG("NoteField","attack "+tn.sAttackModifiers) );
				float fBeat = NoteRowToBeat(i);
				displayCols->display[c].DrawActor( tn, &sprite, c, fBeat, bUseAdditionColoring, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels, false, NotePart_Tap, iDrawDistanceBeforeTargetsPixels, FADE_BEFORE_TARGETS_PERCENT );
			}
			else
			{
				displayCols->display[c].DrawTap( tn, c, NoteRowToBeat(i), bHoldNoteBeginsOnThisBeat, bUseAdditionColoring, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 1, m_fYReverseOffsetPixels, iDrawDistanceBeforeTargetsPixels, FADE_BEFORE_TARGETS_PERCENT );
			}
		}
	}

	cur->m_GhostArrowRow.Draw();
}

void NoteField::FadeToFail()
{
	m_fPercentFadeToFail = max( 0.0f, m_fPercentFadeToFail );	// this will slowly increase every Update()
		// don't fade all over again if this is called twice
}
void NoteField::Step( int iCol, TapNoteScore score ) { m_pCurDisplay->m_ReceptorArrowRow.Step( iCol, score ); }
void NoteField::SetPressed( int iCol ) { m_pCurDisplay->m_ReceptorArrowRow.SetPressed( iCol ); }
void NoteField::DidTapNote( int iCol, TapNoteScore score, bool bBright ) { m_pCurDisplay->m_GhostArrowRow.DidTapNote( iCol, score, bBright ); }
void NoteField::DidHoldNote( int iCol, HoldNoteScore score, bool bBright ) { m_pCurDisplay->m_GhostArrowRow.DidHoldNote( iCol, score, bBright ); }

void NoteField::HandleMessage( const Message &msg )
{
	if( msg == Message_CurrentSongChanged )
	{
		m_fCurrentBeatLastUpdate = -1;
		m_fYPosCurrentBeatLastUpdate = -1;
	}

	ActorFrame::HandleMessage( msg );
}

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
