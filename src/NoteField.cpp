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
#include "Song.h"
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "Style.h"
#include "CommonMetrics.h"
#include <float.h>
#include "BackgroundUtil.h"
#include "Course.h"
#include "NoteData.h"
#include "RageDisplay.h"

float FindFirstDisplayedBeat( const PlayerState* pPlayerState, int iDrawDistanceAfterTargetsPixels );
float FindLastDisplayedBeat( const PlayerState* pPlayerState, int iDrawDistanceBeforeTargetsPixels );

static ThemeMetric<bool> SHOW_BOARD( "NoteField", "ShowBoard" );
static ThemeMetric<bool> SHOW_BEAT_BARS( "NoteField", "ShowBeatBars" );
static ThemeMetric<float> FADE_BEFORE_TARGETS_PERCENT( "NoteField", "FadeBeforeTargetsPercent" );
static ThemeMetric<float> BAR_MEASURE_ALPHA( "NoteField", "BarMeasureAlpha" );
static ThemeMetric<float> BAR_4TH_ALPHA( "NoteField", "Bar4thAlpha" );
static ThemeMetric<float> BAR_8TH_ALPHA( "NoteField", "Bar8thAlpha" );
static ThemeMetric<float> BAR_16TH_ALPHA( "NoteField", "Bar16thAlpha" );
static ThemeMetric<float> FADE_FAIL_TIME( "NoteField", "FadeFailTime" );

static RString RoutineNoteSkinName( size_t i ) { return ssprintf("RoutineNoteSkinP%i",int(i+1)); }
static ThemeMetric1D<RString> ROUTINE_NOTESKIN( "NoteField", RoutineNoteSkinName, NUM_PLAYERS );

NoteField::NoteField()
{
	m_pNoteData = NULL;
	m_pCurDisplay = NULL;

	m_textMeasureNumber.LoadFromFont( THEME->GetPathF("NoteField","MeasureNumber") );
	m_textMeasureNumber.SetZoom( 1.0f );
	m_textMeasureNumber.SetShadowLength( 2 );
	m_textMeasureNumber.SetWrapWidthPixels( 300 );

	m_rectMarkerBar.SetEffectDiffuseShift( 2, RageColor(1,1,1,0.5f), RageColor(0.5f,0.5f,0.5f,0.5f) );

	m_sprBoard.Load( THEME->GetPathG("NoteField","board") );
	m_sprBoard->SetName("Board");
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
	RString sNoteSkinLower = sNoteSkin_;
	sNoteSkinLower.MakeLower();

	if( m_NoteDisplays.find(sNoteSkinLower) != m_NoteDisplays.end() )
		return;

	LockNoteSkin l( sNoteSkinLower );

	LOG->Trace("NoteField::CacheNoteSkin: cache %s", sNoteSkinLower.c_str() );
	NoteDisplayCols *nd = new NoteDisplayCols( GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer );

	for( int c=0; c<GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer; c++ ) 
		nd->display[c].Load( c, m_pPlayerState, m_fYReverseOffsetPixels );
	nd->m_ReceptorArrowRow.Load( m_pPlayerState, m_fYReverseOffsetPixels );
	nd->m_GhostArrowRow.Load( m_pPlayerState, m_fYReverseOffsetPixels );

	m_NoteDisplays[ sNoteSkinLower ] = nd;
}

void NoteField::UncacheNoteSkin( const RString &sNoteSkin_ )
{
	RString sNoteSkinLower = sNoteSkin_;
	sNoteSkinLower.MakeLower();

	LOG->Trace("NoteField::CacheNoteSkin: release %s", sNoteSkinLower.c_str() );
	ASSERT_M( m_NoteDisplays.find(sNoteSkinLower) != m_NoteDisplays.end(), sNoteSkinLower );
	delete m_NoteDisplays[sNoteSkinLower];
	m_NoteDisplays.erase( sNoteSkinLower );
}

void NoteField::CacheAllUsedNoteSkins()
{
	// If we're in Routine mode, apply our per-player noteskins.
	if( GAMESTATE->GetCurrentStyle()->m_StyleType == StyleType_TwoPlayersSharedSides )
	{
		FOREACH_EnabledPlayer( pn )
			GAMESTATE->ApplyStageModifiers( pn, ROUTINE_NOTESKIN.GetValue(pn) );
	}

	/* Cache all note skins that we might need for the whole song, course or battle
	 * play, so we don't have to load them later (such as between course songs). */
	vector<RString> asSkinsLower;
	GAMESTATE->GetAllUsedNoteSkins( asSkinsLower );
	asSkinsLower.push_back( m_pPlayerState->m_PlayerOptions.GetStage().m_sNoteSkin );
	FOREACH( RString, asSkinsLower, s )
		s->MakeLower();

	for( unsigned i=0; i < asSkinsLower.size(); ++i )
		CacheNoteSkin( asSkinsLower[i] );

	/* If we're changing note skins in the editor, we can have old note skins lying
	 * around.  Remove them so they don't accumulate. */
	set<RString> setNoteSkinsToUnload;
	FOREACHM( RString, NoteDisplayCols *, m_NoteDisplays, d )
	{
		bool unused = find(asSkinsLower.begin(), asSkinsLower.end(), d->first) == asSkinsLower.end();
		if( unused )
			setNoteSkinsToUnload.insert( d->first );
	}
	FOREACHS( RString, setNoteSkinsToUnload, s )
		UncacheNoteSkin( *s );

	RString sCurrentNoteSkinLower = m_pPlayerState->m_PlayerOptions.GetCurrent().m_sNoteSkin;
	sCurrentNoteSkinLower.MakeLower();

	map<RString, NoteDisplayCols *>::iterator it = m_NoteDisplays.find( sCurrentNoteSkinLower );
	ASSERT_M( it != m_NoteDisplays.end(), sCurrentNoteSkinLower );
	m_pCurDisplay = it->second;
	memset( m_pDisplays, 0, sizeof(m_pDisplays) );

	FOREACH_EnabledPlayer( pn )
	{
		RString sNoteSkinLower = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_sNoteSkin;
		sNoteSkinLower.MakeLower();
		it = m_NoteDisplays.find( sNoteSkinLower );
		ASSERT_M( it != m_NoteDisplays.end(), sNoteSkinLower );
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
	ASSERT( pNoteData != NULL );
	m_pNoteData = pNoteData;
	m_iDrawDistanceAfterTargetsPixels = iDrawDistanceAfterTargetsPixels;
	m_iDrawDistanceBeforeTargetsPixels = iDrawDistanceBeforeTargetsPixels;
	ASSERT( m_iDrawDistanceBeforeTargetsPixels >= m_iDrawDistanceAfterTargetsPixels );

	m_fPercentFadeToFail = -1;

	//int i1 = m_pNoteData->GetNumTracks();
	//int i2 = GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer;
	ASSERT_M(m_pNoteData->GetNumTracks() == GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer, 
		 ssprintf("NumTracks %d = ColsPerPlayer %d",m_pNoteData->GetNumTracks(), 
			  GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer));
	
	// The NoteSkin may have changed at the beginning of a new course song.
	RString sNoteSkinLower = m_pPlayerState->m_PlayerOptions.GetCurrent().m_sNoteSkin;

	/* XXX: Combination of good idea and bad idea to ensure courses load
	 * regardless of noteskin content. This may take a while to fix. */
	NoteDisplayCols *badIdea = m_pCurDisplay;

	if(sNoteSkinLower.empty())
	{
		sNoteSkinLower = m_pPlayerState->m_PlayerOptions.GetPreferred().m_sNoteSkin;

		if(sNoteSkinLower.empty())
		{
			sNoteSkinLower = "default";
		}
		m_NoteDisplays.insert(pair<RString, NoteDisplayCols *> (sNoteSkinLower, badIdea));
	}

	sNoteSkinLower.MakeLower();
	map<RString, NoteDisplayCols *>::iterator it = m_NoteDisplays.find( sNoteSkinLower );
	ASSERT_M( it != m_NoteDisplays.end(), ssprintf("iterator != m_NoteDisplays.end() [sNoteSkinLower = %s]",sNoteSkinLower.c_str()) );
	memset( m_pDisplays, 0, sizeof(m_pDisplays) );
	FOREACH_EnabledPlayer( pn )
	{
		sNoteSkinLower = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_sNoteSkin;

		// XXX: Re-setup sNoteSkinLower. Unsure if inserting the skin again is needed.
		if(sNoteSkinLower.empty())
		{
			sNoteSkinLower = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetPreferred().m_sNoteSkin;

			if(sNoteSkinLower.empty())
			{
				sNoteSkinLower = "default";
			}
			m_NoteDisplays.insert(pair<RString, NoteDisplayCols *> (sNoteSkinLower, badIdea));
		}

		sNoteSkinLower.MakeLower();
		it = m_NoteDisplays.find( sNoteSkinLower );
		ASSERT_M( it != m_NoteDisplays.end(), sNoteSkinLower );
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
	const float fCurrentBeat = m_pPlayerState->GetDisplayedPosition().m_fSongBeat;
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
		m_fPercentFadeToFail = min( m_fPercentFadeToFail + fDeltaTime/FADE_FAIL_TIME, 1 );

	// Update fade to failed
	m_pCurDisplay->m_ReceptorArrowRow.SetFadeToFailPercent( m_fPercentFadeToFail );

	NoteDisplay::Update( fDeltaTime );
	/* Update all NoteDisplays. Hack: We need to call this once per frame, not
	 * once per player. */
	// TODO: Remove use of PlayerNumber.

	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	if( pn == GAMESTATE->GetMasterPlayerNumber() )
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

void NoteField::DrawBeatBar( const float fBeat, BeatBarType type, int iMeasureIndex )
{
	bool bIsMeasure = type == measure;

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
		switch( type )
		{
			DEFAULT_FAIL( type );
			case measure: // handled above
			case beat: // fall through
				fAlpha = BAR_4TH_ALPHA;
				iState = 1;
				break;
			case half_beat:
				fAlpha = SCALE(fScrollSpeed,1.0f,2.0f,0.0f,BAR_8TH_ALPHA);
				iState = 2;
				break;
			case quarter_beat:
				fAlpha = SCALE(fScrollSpeed,2.0f,4.0f,0.0f,BAR_16TH_ALPHA);
				iState = 3;
				break;
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
		int iMeasureNoDisplay = iMeasureIndex;

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
	// Draw the board centered on fYPosAt0 so that the board doesn't slide as
	// the draw distance changes with modifiers.
	const float fYPosAt0 = ArrowEffects::GetYPos( m_pPlayerState, 0, 0, m_fYReverseOffsetPixels );

	// todo: make this an AutoActor instead? -aj
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

		// handle tex coord offset and fade
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

static ThemeMetric<RageColor> AREA_HIGHLIGHT_COLOR("NoteField", "AreaHighlightColor");
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
	m_rectAreaHighlight.SetDiffuse( AREA_HIGHLIGHT_COLOR );
	m_rectAreaHighlight.Draw();
}

// todo: add DrawWarpAreaBG? -aj

static ThemeMetric<RageColor> BPM_COLOR ( "NoteField", "BPMColor" );
static ThemeMetric<RageColor> STOP_COLOR ( "NoteField", "StopColor" );
static ThemeMetric<RageColor> DELAY_COLOR ( "NoteField", "DelayColor" );
static ThemeMetric<RageColor> WARP_COLOR ( "NoteField", "WarpColor" );
static ThemeMetric<RageColor> TIME_SIGNATURE_COLOR ( "NoteField", "TimeSignatureColor" );
static ThemeMetric<RageColor> TICKCOUNT_COLOR ( "NoteField", "TickcountColor" );
static ThemeMetric<RageColor> COMBO_COLOR ( "NoteField", "ComboColor" );
static ThemeMetric<RageColor> LABEL_COLOR ( "NoteField", "LabelColor" );
static ThemeMetric<RageColor> SPEED_COLOR ( "NoteField", "SpeedColor" );
static ThemeMetric<RageColor> SCROLL_COLOR ( "NoteField", "ScrollColor" );
static ThemeMetric<RageColor> FAKE_COLOR ("NoteField", "FakeColor" );
static ThemeMetric<bool> BPM_IS_LEFT_SIDE ( "NoteField", "BPMIsLeftSide" );
static ThemeMetric<bool> STOP_IS_LEFT_SIDE ( "NoteField", "StopIsLeftSide" );
static ThemeMetric<bool> DELAY_IS_LEFT_SIDE ( "NoteField", "DelayIsLeftSide" );
static ThemeMetric<bool> WARP_IS_LEFT_SIDE ( "NoteField", "WarpIsLeftSide" );
static ThemeMetric<bool> TIME_SIGNATURE_IS_LEFT_SIDE ( "NoteField", "TimeSignatureIsLeftSide" );
static ThemeMetric<bool> TICKCOUNT_IS_LEFT_SIDE ( "NoteField", "TickcountIsLeftSide" );
static ThemeMetric<bool> COMBO_IS_LEFT_SIDE ( "NoteField", "ComboIsLeftSide" );
static ThemeMetric<bool> LABEL_IS_LEFT_SIDE ( "NoteField", "LabelIsLeftSide" );
static ThemeMetric<bool> SPEED_IS_LEFT_SIDE ( "NoteField", "SpeedIsLeftSide" );
static ThemeMetric<bool> SCROLL_IS_LEFT_SIDE ( "NoteField", "ScrollIsLeftSide" );
static ThemeMetric<bool> FAKE_IS_LEFT_SIDE ( "NoteField", "FakeIsLeftSide" );
static ThemeMetric<float> BPM_OFFSETX ( "NoteField", "BPMOffsetX" );
static ThemeMetric<float> STOP_OFFSETX ( "NoteField", "StopOffsetX" );
static ThemeMetric<float> DELAY_OFFSETX ( "NoteField", "DelayOffsetX" );
static ThemeMetric<float> WARP_OFFSETX ( "NoteField", "WarpOffsetX" );
static ThemeMetric<float> TIME_SIGNATURE_OFFSETX ( "NoteField", "TimeSignatureOffsetX" );
static ThemeMetric<float> TICKCOUNT_OFFSETX ( "NoteField", "TickcountOffsetX" );
static ThemeMetric<float> COMBO_OFFSETX ( "NoteField", "ComboOffsetX" );
static ThemeMetric<float> LABEL_OFFSETX ( "NoteField", "LabelOffsetX" );
static ThemeMetric<float> SPEED_OFFSETX ( "NoteField", "SpeedOffsetX" );
static ThemeMetric<float> SCROLL_OFFSETX ( "NoteField", "ScrollOffsetX" );
static ThemeMetric<float> FAKE_OFFSETX ( "NoteField", "FakeOffsetX" );

void NoteField::DrawBPMText( const float fBeat, const float fBPM )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= BPM_OFFSETX * fZoom;

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( BPM_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( BPM_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( FloatToString(fBPM) );
	m_textMeasureNumber.SetXY( (BPM_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawFreezeText( const float fBeat, const float fSecs )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= STOP_OFFSETX * fZoom;

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( STOP_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( STOP_COLOR );
	m_textMeasureNumber.SetXY( (STOP_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( FloatToString(fSecs) );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawDelayText( const float fBeat, const float fSecs )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= DELAY_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( DELAY_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( DELAY_COLOR );
	m_textMeasureNumber.SetXY( (DELAY_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( FloatToString(fSecs) );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawWarpText( const float fBeat, const float fNewBeat )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= WARP_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( WARP_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( WARP_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( FloatToString(fNewBeat) );
	m_textMeasureNumber.SetXY( (WARP_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawTimeSignatureText( const float fBeat, int iNumerator, int iDenominator )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= TIME_SIGNATURE_OFFSETX * fZoom;

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( TIME_SIGNATURE_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( TIME_SIGNATURE_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%d\n--\n%d", iNumerator, iDenominator) );
	m_textMeasureNumber.SetXY( (TIME_SIGNATURE_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawTickcountText( const float fBeat, int iTicks )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= TICKCOUNT_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( TICKCOUNT_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( TICKCOUNT_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%d", iTicks) );
	m_textMeasureNumber.SetXY( (TICKCOUNT_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawComboText( const float fBeat, int iCombo, int iMiss )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= COMBO_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( COMBO_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( COMBO_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%d/%d", iCombo, iMiss) );
	m_textMeasureNumber.SetXY( (COMBO_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawLabelText( const float fBeat, RString sLabel )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= LABEL_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( LABEL_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( LABEL_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( sLabel.c_str() );
	m_textMeasureNumber.SetXY( (LABEL_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawSpeedText( const float fBeat, float fPercent, float fWait, int iMode )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= SPEED_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( SPEED_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( SPEED_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( ssprintf("%s\n%s\n%s", FloatToString(fPercent).c_str(), (iMode == 1 ? "S" : "B"), FloatToString(fWait).c_str()) );
	m_textMeasureNumber.SetXY( (SPEED_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawScrollText( const float fBeat, float fPercent )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
 	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= SCROLL_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( SCROLL_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( SCROLL_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( FloatToString(fPercent) );
	m_textMeasureNumber.SetXY( (SCROLL_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawFakeText( const float fBeat, const float fNewBeat )
{
	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos	= ArrowEffects::GetYPos(    m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom(    m_pPlayerState );
	const float xBase	= GetWidth()/2.f;
	const float xOffset	= FAKE_OFFSETX * fZoom;
	
	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( FAKE_IS_LEFT_SIDE ? align_right : align_left );
	m_textMeasureNumber.SetDiffuse( FAKE_COLOR );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( FloatToString(fNewBeat) );
	m_textMeasureNumber.SetXY( (FAKE_IS_LEFT_SIDE ? -xBase - xOffset : xBase + xOffset), fYPos );
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
	const float fYPos	= ArrowEffects::GetYPos( m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels );
	const float fZoom	= ArrowEffects::GetZoom( m_pPlayerState );

	m_textMeasureNumber.SetZoom( fZoom );
	m_textMeasureNumber.SetHorizAlign( align_left );
	m_textMeasureNumber.SetDiffuse( RageColor(0,1,0,1) );
	m_textMeasureNumber.SetGlow( RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f) );
	m_textMeasureNumber.SetText( sNewBGName );
	m_textMeasureNumber.SetXY( +GetWidth()/2.f, fYPos );
	m_textMeasureNumber.Draw();
}

static CacheNoteStat GetNumNotesFromBeginning( const PlayerState *pPlayerState, float beat )
{
	// XXX: I realized that I have copied and pasted my binary search code 3 times already.
	//      how can we abstract this?
	const vector<CacheNoteStat> &data = pPlayerState->m_CacheNoteStat;
	int max = data.size() - 1;
	int l = 0, r = max;
	while( l <= r )
	{
		int m = ( l + r ) / 2;
		if( ( m == 0 || data[m].beat <= beat ) && ( m == max || beat < data[m + 1].beat ) )
		{
			return data[m];
		}
		else if( data[m].beat <= beat )
		{
			l = m + 1;
		}
		else
		{
			r = m - 1;
		}
	}
	CacheNoteStat dummy = { 0, 0, 0 };
	return dummy;
}

static int GetNumNotesRange( const PlayerState* pPlayerState, float fLow, float fHigh )
{
	CacheNoteStat low  = GetNumNotesFromBeginning( pPlayerState, fLow );
	CacheNoteStat high = GetNumNotesFromBeginning( pPlayerState, fHigh );
	return high.notesUpper - low.notesLower;
}

float FindFirstDisplayedBeat( const PlayerState* pPlayerState, int iDrawDistanceAfterTargetsPixels )
{
	
	float fLow = 0, fHigh = pPlayerState->GetDisplayedPosition().m_fSongBeat;
	
	bool bHasCache = pPlayerState->m_CacheNoteStat.size() > 0;
	
	if( !bHasCache )
	{
		fLow = fHigh - 4.0f;
	}
	
	const int NUM_ITERATIONS = 24;
	const int MAX_NOTES_AFTER = 64;
	
	float fFirstBeatToDraw = fLow;
	
	for( int i = 0; i < NUM_ITERATIONS; i ++ )
	{
	
		float fMid = (fLow + fHigh) / 2.0f;
		
		bool bIsPastPeakYOffset;
		float fPeakYOffset;
		float fYOffset = ArrowEffects::GetYOffset( pPlayerState, 0, fMid, fPeakYOffset, bIsPastPeakYOffset, true );

		if( fYOffset < iDrawDistanceAfterTargetsPixels || ( bHasCache && GetNumNotesRange( pPlayerState, fMid, pPlayerState->GetDisplayedPosition().m_fSongBeat ) > MAX_NOTES_AFTER ) ) // off screen / too many notes
		{
			fFirstBeatToDraw = fMid; // move towards fSongBeat
			fLow = fMid;
		}
		else // on screen, move away!!
		{
			fHigh = fMid;
		}
		
	}

	return fFirstBeatToDraw;

}

float FindLastDisplayedBeat( const PlayerState* pPlayerState, int iDrawDistanceBeforeTargetsPixels )
{
	// Probe for last note to draw. Worst case is 0.25x + boost.
	// Adjust search distance so that notes don't pop onto the screen.
	float fSearchDistance = 10;
	float fLastBeatToDraw = pPlayerState->GetDisplayedPosition().m_fSongBeat+fSearchDistance;
	float fSpeedMultiplier = pPlayerState->GetDisplayedTiming().GetDisplayedSpeedPercent(pPlayerState->GetDisplayedPosition().m_fSongBeatVisible, pPlayerState->GetDisplayedPosition().m_fMusicSecondsVisible);

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
		else if( fYOffset > iDrawDistanceBeforeTargetsPixels ) // off screen
			fLastBeatToDraw -= fSearchDistance;
		else // on screen
			fLastBeatToDraw += fSearchDistance;

		fSearchDistance /= 2;
	}

	if( fSpeedMultiplier < 0.75 )
	{
		fLastBeatToDraw = min(fLastBeatToDraw, pPlayerState->GetDisplayedPosition().m_fSongBeat + 16);
	}

	return fLastBeatToDraw;
}

inline float NoteRowToVisibleBeat( const PlayerState *pPlayerState, int iRow )
{
	return NoteRowToBeat(iRow);
}

bool NoteField::IsOnScreen( float fBeat, int iCol, int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels ) const
{
	// TRICKY: If boomerang is on, then ones in the range 
	// [iFirstRowToDraw,iLastRowToDraw] aren't necessarily visible.
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

	// This should be filled in on the first update.
	ASSERT( m_pCurDisplay != NULL );

	ArrowEffects::Update();

	NoteDisplayCols *cur = m_pCurDisplay;

	const PlayerOptions &current_po = m_pPlayerState->m_PlayerOptions.GetCurrent();

	// Adjust draw range depending on some effects
	int iDrawDistanceAfterTargetsPixels = m_iDrawDistanceAfterTargetsPixels;
	// HACK: If boomerang and centered are on, then we want to draw much 
	// earlier so that the notes don't pop on screen.
	float fCenteredTimesBoomerang = 
		current_po.m_fScrolls[PlayerOptions::SCROLL_CENTERED] * 
		current_po.m_fAccels[PlayerOptions::ACCEL_BOOMERANG];
	iDrawDistanceAfterTargetsPixels += int(SCALE( fCenteredTimesBoomerang, 0.f, 1.f, 0.f, -SCREEN_HEIGHT/2 ));
	int iDrawDistanceBeforeTargetsPixels = m_iDrawDistanceBeforeTargetsPixels;

	float fDrawScale = 1;
	fDrawScale *= 1 + 0.5f * fabsf( current_po.m_fPerspectiveTilt );
	fDrawScale *= 1 + fabsf( current_po.m_fEffects[PlayerOptions::EFFECT_MINI] );

	iDrawDistanceAfterTargetsPixels = (int)(iDrawDistanceAfterTargetsPixels * fDrawScale);
	iDrawDistanceBeforeTargetsPixels = (int)(iDrawDistanceBeforeTargetsPixels * fDrawScale);


	// Probe for first and last notes on the screen
	float fFirstBeatToDraw = FindFirstDisplayedBeat( m_pPlayerState, iDrawDistanceAfterTargetsPixels );
	float fLastBeatToDraw = FindLastDisplayedBeat( m_pPlayerState, iDrawDistanceBeforeTargetsPixels );

	m_pPlayerState->m_fLastDrawnBeat = fLastBeatToDraw;

	const int iFirstRowToDraw  = BeatToNoteRow(fFirstBeatToDraw);
	const int iLastRowToDraw   = BeatToNoteRow(fLastBeatToDraw);

	//LOG->Trace( "start = %f.1, end = %f.1", fFirstBeatToDraw-fSongBeat, fLastBeatToDraw-fSongBeat );
	//LOG->Trace( "Drawing elements %d through %d", iFirstRowToDraw, iLastRowToDraw );

#define IS_ON_SCREEN( fBeat )  ( fFirstBeatToDraw <= (fBeat) && (fBeat) <= fLastBeatToDraw && IsOnScreen( fBeat, 0, iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels ) )

	// Draw board
	if( SHOW_BOARD )
	{
		DrawBoard( iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels );
	}

	// Draw Receptors
	{
		cur->m_ReceptorArrowRow.Draw();
	}

	const TimingData *pTiming = &m_pPlayerState->GetDisplayedTiming();
	const vector<TimingSegment*>* segs[NUM_TimingSegmentType];

	FOREACH_TimingSegmentType( tst )
		segs[tst] = &(pTiming->GetTimingSegments(tst));

	unsigned i = 0;
	// Draw beat bars
	if( ( GAMESTATE->IsEditing() || SHOW_BEAT_BARS ) && pTiming != NULL )
	{
		const vector<TimingSegment *> &tSigs = *segs[SEGMENT_TIME_SIG];
		int iMeasureIndex = 0;
		for (i = 0; i < tSigs.size(); i++)
		{
			const TimeSignatureSegment *ts = ToTimeSignature(tSigs[i]);
			int iSegmentEndRow = (i + 1 == tSigs.size()) ? iLastRowToDraw : tSigs[i+1]->GetRow();

			// beat bars every 16th note
			int iDrawBeatBarsEveryRows = BeatToNoteRow( ((float)ts->GetDen()) / 4 ) / 4;

			// In 4/4, every 16th beat bar is a measure
			int iMeasureBarFrequency =  ts->GetNum() * 4;
			int iBeatBarsDrawn = 0;

			for( int j=ts->GetRow(); j < iSegmentEndRow; j += iDrawBeatBarsEveryRows )
			{
				bool bMeasureBar = iBeatBarsDrawn % iMeasureBarFrequency == 0;
				BeatBarType type = quarter_beat;
				if( bMeasureBar )
					type = measure;
				else if( iBeatBarsDrawn % 4 == 0 )
					type = beat;
				else if( iBeatBarsDrawn % 2 == 0 )
					type = half_beat;
				float fBeat = NoteRowToBeat(j);

				if( IS_ON_SCREEN(fBeat) )
				{
					DrawBeatBar( fBeat, type, iMeasureIndex );
				}

				iBeatBarsDrawn++;
				if( bMeasureBar )
					iMeasureIndex++;
			}
		}
	}

	if( GAMESTATE->IsEditing() && pTiming != NULL )
	{
		ASSERT(GAMESTATE->m_pCurSong != NULL);

		const TimingData &timing = *pTiming;

		// Scroll text
		if( GAMESTATE->m_bIsUsingStepTiming )
		{
			for (i = 0; i < segs[SEGMENT_SCROLL]->size(); i++)
			{
				ScrollSegment *seg = ToScroll( segs[SEGMENT_SCROLL]->at(i) );
				if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
				{
					float fBeat = seg->GetBeat();
					if( IS_ON_SCREEN(fBeat) )
						DrawScrollText( fBeat, seg->GetRatio() );
				}
			}
		}

		// BPM text
		for (i = 0; i < segs[SEGMENT_BPM]->size(); i++)
		{
			const BPMSegment *seg = ToBPM( segs[SEGMENT_BPM]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawBPMText( fBeat, seg->GetBPM() );
			}
		}

		// Freeze text
		for (i = 0; i < segs[SEGMENT_STOP]->size(); i++)
		{
			const StopSegment *seg = ToStop( segs[SEGMENT_STOP]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawFreezeText( fBeat, seg->GetPause() );
			}
		}

		// Delay text
		for (i = 0; i < segs[SEGMENT_DELAY]->size(); i++)
		{
			const DelaySegment *seg = ToDelay( segs[SEGMENT_DELAY]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawDelayText( fBeat, seg->GetPause() );
			}
		}

		// Warp text
		for (i = 0; i < segs[SEGMENT_WARP]->size(); i++)
		{
			const WarpSegment *seg = ToWarp( segs[SEGMENT_WARP]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawWarpText( fBeat, seg->GetLength() );
			}
		}

		// Time Signature text
		for (i = 0; i < segs[SEGMENT_TIME_SIG]->size(); i++)
		{
			const TimeSignatureSegment *seg = ToTimeSignature( segs[SEGMENT_TIME_SIG]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawTimeSignatureText( fBeat, seg->GetNum(), seg->GetDen() );
			}
		}

		// Tickcount text
		for (i = 0; i < segs[SEGMENT_TICKCOUNT]->size(); i++)
		{
			const TickcountSegment *seg = ToTickcount( segs[SEGMENT_TICKCOUNT]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawTickcountText( fBeat, seg->GetTicks() );
			}
		}

		// Combo text
		for (i = 0; i < segs[SEGMENT_COMBO]->size(); i++)
		{
			const ComboSegment *seg = ToCombo( segs[SEGMENT_COMBO]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawComboText( fBeat, seg->GetCombo(), seg->GetMissCombo() );
			}
		}

		// Label text
		for (i = 0; i < segs[SEGMENT_LABEL]->size(); i++)
		{
			const LabelSegment *seg = ToLabel( segs[SEGMENT_LABEL]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawLabelText( fBeat, seg->GetLabel() );
			}
		}

		// Speed text
		for (i = 0; i < segs[SEGMENT_SPEED]->size(); i++)
		{
			const SpeedSegment *seg = ToSpeed( segs[SEGMENT_SPEED]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawSpeedText(fBeat, seg->GetRatio(), 
							  seg->GetDelay(), seg->GetUnit() );
			}
		}

		// Fake text
		for (i = 0; i < segs[SEGMENT_FAKE]->size(); i++)
		{
			const FakeSegment *seg = ToFake( segs[SEGMENT_FAKE]->at(i) );
			if( seg->GetRow() >= iFirstRowToDraw && seg->GetRow() <= iLastRowToDraw )
			{
				float fBeat = seg->GetBeat();
				if( IS_ON_SCREEN(fBeat) )
					DrawFakeText( fBeat, seg->GetLength() );
			}
		}

		// Course mods text
		const Course *pCourse = GAMESTATE->m_pCurCourse;
		if( pCourse )
		{
			ASSERT_M( GAMESTATE->m_iEditCourseEntryIndex >= 0  &&  GAMESTATE->m_iEditCourseEntryIndex < (int)pCourse->m_vEntries.size(), 
				ssprintf("%i",GAMESTATE->m_iEditCourseEntryIndex.Get()) );
			const CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
			FOREACH_CONST( Attack, ce.attacks, a )
			{
				float fSecond = a->fStartSecond;
				float fBeat = timing.GetBeatFromElapsedTime( fSecond );

				if( BeatToNoteRow(fBeat) >= iFirstRowToDraw &&
					BeatToNoteRow(fBeat) <= iLastRowToDraw)
				{
					if( IS_ON_SCREEN(fBeat) )
						DrawAttackText( fBeat, *a );
				}
			}
		}
		else
		{
			AttackArray &attacks = GAMESTATE->m_bIsUsingStepTiming ?
				GAMESTATE->m_pCurSteps[PLAYER_1]->m_Attacks :
				GAMESTATE->m_pCurSong->m_Attacks;
			// XXX: We're somehow getting here when attacks is null. Find the actual cause later.
			if (&attacks)
			{
				FOREACH_CONST(Attack, attacks, a)
				{
					float fBeat = timing.GetBeatFromElapsedTime(a->fStartSecond);
					if (BeatToNoteRow(fBeat) >= iFirstRowToDraw &&
						BeatToNoteRow(fBeat) <= iLastRowToDraw &&
						IS_ON_SCREEN(fBeat))
					{
						this->DrawAttackText(fBeat, *a);
					}
				}
			}
		}

		if( !GAMESTATE->m_bIsUsingStepTiming )
		{
			// BGChange text
			EditMode mode = GAMESTATE->m_EditMode;
			switch( mode )
			{
				case EditMode_Home:
				case EditMode_CourseMods:
				case EditMode_Practice:
					break;
				case EditMode_Full:
					{
						vector<BackgroundChange>::iterator iter[NUM_BackgroundLayer];
						FOREACH_BackgroundLayer( j )
							iter[j] = GAMESTATE->m_pCurSong->GetBackgroundChanges(j).begin();

						while( 1 )
						{
							float fLowestBeat = FLT_MAX;
							vector<BackgroundLayer> viLowestIndex;

							FOREACH_BackgroundLayer( j )
							{
								if( iter[j] == GAMESTATE->m_pCurSong->GetBackgroundChanges(j).end() )
									continue;

								float fBeat = iter[j]->m_fStartBeat;
								if( fBeat < fLowestBeat )
								{
									fLowestBeat = fBeat;
									viLowestIndex.clear();
									viLowestIndex.push_back( j );
								}
								else if( fBeat == fLowestBeat )
								{
									viLowestIndex.push_back( j );
								}
							}
		
							if( viLowestIndex.empty() )
							{
								FOREACH_BackgroundLayer( j )
									ASSERT( iter[j] == GAMESTATE->m_pCurSong->GetBackgroundChanges(j).end() );
								break;
							}
	
							if( IS_ON_SCREEN(fLowestBeat) )
							{
								vector<RString> vsBGChanges;
								FOREACH_CONST( BackgroundLayer, viLowestIndex, bl )
								{
									ASSERT( iter[*bl] != GAMESTATE->m_pCurSong->GetBackgroundChanges(*bl).end() );
									const BackgroundChange& change = *iter[*bl];
									RString s = change.GetTextDescription();
									if( *bl!=0 )
										s = ssprintf("%d: ",*bl) + s;
									vsBGChanges.push_back( s );
								}
								DrawBGChangeText( fLowestBeat, join("\n",vsBGChanges) );
							}
							FOREACH_CONST( BackgroundLayer, viLowestIndex, bl )
								iter[*bl]++;
						}
					}
					break;
				default:
					FAIL_M(ssprintf("Invalid edit mode: %i", mode));
			}
		}

		// Draw marker bars
		if( m_iBeginMarker != -1  &&  m_iEndMarker != -1 )
		{
			int iBegin = m_iBeginMarker;
			int iEnd = m_iEndMarker;
			CLAMP( iBegin, iFirstRowToDraw, iLastRowToDraw );
			CLAMP( iEnd, iFirstRowToDraw, iLastRowToDraw );
			DrawAreaHighlight( iBegin, iEnd );
		}
		else if( m_iBeginMarker != -1 )
		{
			if( m_iBeginMarker >= iFirstRowToDraw &&
				m_iBeginMarker <= iLastRowToDraw )
				DrawMarkerBar( m_iBeginMarker );
		}
		else if( m_iEndMarker != -1 )
		{
			if( m_iEndMarker >= iFirstRowToDraw &&
				m_iEndMarker <= iLastRowToDraw )
			DrawMarkerBar( m_iEndMarker );
		}
	}

	// Optimization is very important here because there are so many arrows to draw.
	// Draw the arrows in order of column. This minimizes texture switches and
	// lets us draw in big batches.

	float fSelectedRangeGlow = SCALE( RageFastCos(RageTimer::GetTimeSinceStartFast()*2), -1, 1, 0.1f, 0.3f );

	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	ASSERT_M(m_pNoteData->GetNumTracks() == GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer, 
		 ssprintf("NumTracks %d != ColsPerPlayer %d",m_pNoteData->GetNumTracks(), 
			  GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer));

	for( int j=0; j<m_pNoteData->GetNumTracks(); j++ )	// for each arrow column
	{
		const int c = pStyle->m_iColumnDrawOrder[j];

		bool bAnyUpcomingInThisCol = false;

		// Draw all HoldNotes in this column (so that they appear under the tap notes)
		{
			NoteData::TrackMap::const_iterator begin, end;
			m_pNoteData->GetTapNoteRangeInclusive( c, iFirstRowToDraw, iLastRowToDraw+1, begin, end );

			for( ; begin != end; ++begin )
			{
				const TapNote &tn = begin->second; //m_pNoteData->GetTapNote(c, j);
				if( tn.type != TapNote::hold_head )
					continue; // skip

				const HoldNoteResult &Result = tn.HoldResult;
				if( Result.hns == HNS_Held ) // if this HoldNote was completed
					continue; // don't draw anything

				int iStartRow = begin->first;
				int iEndRow = iStartRow + tn.iDuration;

				// TRICKY: If boomerang is on, then all notes in the range 
				// [iFirstRowToDraw,iLastRowToDraw] aren't necessarily visible.
				// Test every note to make sure it's on screen before drawing
				float fThrowAway;
				bool bStartIsPastPeak = false;
				bool bEndIsPastPeak = false;
				float fStartYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, c, NoteRowToVisibleBeat(m_pPlayerState, iStartRow), fThrowAway, bStartIsPastPeak );
				float fEndYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, c, NoteRowToVisibleBeat(m_pPlayerState, iEndRow), fThrowAway, bEndIsPastPeak );

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
				const bool bHoldGhostShowing = tn.HoldResult.bActive  &&  tn.HoldResult.fLife > 0;
				const bool bIsHoldingNote = tn.HoldResult.bHeld;
				if( bHoldGhostShowing )
					m_pCurDisplay->m_GhostArrowRow.SetHoldShowing( c, tn );

				ASSERT_M( NoteRowToBeat(iStartRow) > -2000, ssprintf("%i %i %i", iStartRow, iEndRow, c) );

				bool bIsInSelectionRange = false;
				if( m_iBeginMarker!=-1 && m_iEndMarker!=-1 )
					bIsInSelectionRange = (m_iBeginMarker <= iStartRow && iEndRow < m_iEndMarker);

				NoteDisplayCols *displayCols = tn.pn == PLAYER_INVALID ? m_pCurDisplay : m_pDisplays[tn.pn];
				displayCols->display[c].DrawHold( tn, c, iStartRow, bIsHoldingNote, Result, bUseAdditionColoring, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 
					m_fYReverseOffsetPixels, (float) iDrawDistanceAfterTargetsPixels, (float) iDrawDistanceBeforeTargetsPixels, iDrawDistanceBeforeTargetsPixels, FADE_BEFORE_TARGETS_PERCENT );

				bool bNoteIsUpcoming = NoteRowToBeat(iStartRow) > m_pPlayerState->GetDisplayedPosition().m_fSongBeat;
				bAnyUpcomingInThisCol |= bNoteIsUpcoming;
			}
		}

		// Draw all TapNotes in this column

		// draw notes from furthest to closest
		NoteData::TrackMap::const_iterator begin, end;
		m_pNoteData->GetTapNoteRange( c, iFirstRowToDraw, iLastRowToDraw+1, begin, end );
		for( ; begin != end; ++begin )
		{
			int q = begin->first;
			const TapNote &tn = begin->second; //m_pNoteData->GetTapNote(c, q);

			// Switch modified by Wolfman2000, tested by Saturn2888
			// Fixes hold head overlapping issue, but not the rolls.
			switch( tn.type )
			{
				case TapNote::empty: // no note here
				{
					continue;
				}
				case TapNote::hold_head:
				{
					//if (tn.subType == TapNote::hold_head_roll)
						continue; // skip
				}
				default: break;
			}

			// Don't draw hidden (fully judged) steps.
			if( tn.result.bHidden )
				continue;

			// TRICKY: If boomerang is on, then all notes in the range 
			// [iFirstRowToDraw,iLastRowToDraw] aren't necessarily visible.
			// Test every note to make sure it's on screen before drawing.
			if( !IsOnScreen( NoteRowToBeat(q), c, iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels ) )
				continue; // skip

			ASSERT_M( NoteRowToBeat(q) > -2000, ssprintf("%i %i %i, %f %f", q, iLastRowToDraw, 
							iFirstRowToDraw, m_pPlayerState->GetDisplayedPosition().m_fSongBeat, m_pPlayerState->GetDisplayedPosition().m_fMusicSeconds) );

			// See if there is a hold step that begins on this index.
			// Only do this if the noteskin cares.
			bool bHoldNoteBeginsOnThisBeat = false;
			if( m_pCurDisplay->display[c].DrawHoldHeadForTapsOnSameRow() )
			{
				for( int c2=0; c2<m_pNoteData->GetNumTracks(); c2++ )
				{
					const TapNote &tmp = m_pNoteData->GetTapNote(c2, q);
					if(tmp.type == TapNote::hold_head &&
					   tmp.subType == TapNote::hold_head_hold)
					{
						bHoldNoteBeginsOnThisBeat = true;
						break;
					}
				}
			}

			// do the same for a roll.
			bool bRollNoteBeginsOnThisBeat = false;
			if (m_pCurDisplay->display[c].DrawRollHeadForTapsOnSameRow() )
			{
				for( int c2=0; c2<m_pNoteData->GetNumTracks(); c2++ )
				{
					const TapNote &tmp = m_pNoteData->GetTapNote(c2, q);
					if(tmp.type == TapNote::hold_head &&
					   tmp.subType == TapNote::hold_head_roll)
					{
						bRollNoteBeginsOnThisBeat = true;
						break;
					}
				}
			}

			bool bIsInSelectionRange = false;
			if( m_iBeginMarker!=-1 && m_iEndMarker!=-1 )
				bIsInSelectionRange = m_iBeginMarker<=q && q<m_iEndMarker;

			bool bIsAddition = (tn.source == TapNote::addition);
			bool bIsHopoPossible = (tn.bHopoPossible);
			bool bUseAdditionColoring = bIsAddition || bIsHopoPossible;
			NoteDisplayCols *displayCols = tn.pn == PLAYER_INVALID ? m_pCurDisplay : m_pDisplays[tn.pn];
			displayCols->display[c].DrawTap(tn, c, NoteRowToVisibleBeat(m_pPlayerState, q),
							bHoldNoteBeginsOnThisBeat, bRollNoteBeginsOnThisBeat,
					bUseAdditionColoring, bIsInSelectionRange ? fSelectedRangeGlow : m_fPercentFadeToFail, 
					m_fYReverseOffsetPixels, iDrawDistanceAfterTargetsPixels, iDrawDistanceBeforeTargetsPixels, 
					FADE_BEFORE_TARGETS_PERCENT );

			bool bNoteIsUpcoming = NoteRowToBeat(q) > m_pPlayerState->GetDisplayedPosition().m_fSongBeat;
			bAnyUpcomingInThisCol |= bNoteIsUpcoming;

			DISPLAY->ClearZBuffer();
		}

		cur->m_ReceptorArrowRow.SetNoteUpcoming( c, bAnyUpcomingInThisCol );
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
