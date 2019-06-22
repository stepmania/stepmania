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
	m_pNoteData = nullptr;
	m_pCurDisplay = nullptr;
	m_drawing_board_primitive= false;

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

	// I decided to do it this way because I don't want to dig through
	// ScreenEdit to change all the places it touches the markers. -Kyz
	m_FieldRenderArgs.selection_begin_marker= &m_iBeginMarker;
	m_FieldRenderArgs.selection_end_marker= &m_iEndMarker;
	m_iBeginMarker = m_iEndMarker = -1;

	m_FieldRenderArgs.fail_fade = -1;

	m_StepCallback.SetFromNil();
	m_SetPressedCallback.SetFromNil();
	m_DidTapNoteCallback.SetFromNil();
	m_DidHoldNoteCallback.SetFromNil();
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
	m_pCurDisplay = nullptr;
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
	NoteDisplayCols *nd = new NoteDisplayCols( GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer );

	for( int c=0; c<GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer; c++ )
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
	if( GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_StyleType == StyleType_TwoPlayersSharedSides )
	{
		FOREACH_EnabledPlayer( pn )
			GAMESTATE->ApplyStageModifiers( pn, ROUTINE_NOTESKIN.GetValue(pn) );
	}

	/* Cache all note skins that we might need for the whole song, course or battle
	 * play, so we don't have to load them later (such as between course songs). */
	vector<RString> asSkinsLower;
	GAMESTATE->GetAllUsedNoteSkins( asSkinsLower );
	asSkinsLower.push_back( m_pPlayerState->m_PlayerOptions.GetStage().m_sNoteSkin );
	for (RString &s : asSkinsLower)
	{
		NOTESKIN->ValidateNoteSkinName(s);
		s.MakeLower();
	}

	for( unsigned i=0; i < asSkinsLower.size(); ++i )
		CacheNoteSkin( asSkinsLower[i] );

	/* If we're changing note skins in the editor, we can have old note skins lying
	 * around.  Remove them so they don't accumulate. */
	set<RString> setNoteSkinsToUnload;
	for (std::pair<RString const &, NoteDisplayCols *> d : m_NoteDisplays)
	{
		bool unused = find(asSkinsLower.begin(), asSkinsLower.end(), d.first) == asSkinsLower.end();
		if( unused )
			setNoteSkinsToUnload.insert( d.first );
	}
	for (RString const & skin : setNoteSkinsToUnload)
		UncacheNoteSkin( skin );

	RString sCurrentNoteSkinLower = m_pPlayerState->m_PlayerOptions.GetCurrent().m_sNoteSkin;
	NOTESKIN->ValidateNoteSkinName(sCurrentNoteSkinLower);
	sCurrentNoteSkinLower.MakeLower();

	map<RString, NoteDisplayCols *>::iterator it = m_NoteDisplays.find( sCurrentNoteSkinLower );
	ASSERT_M( it != m_NoteDisplays.end(), sCurrentNoteSkinLower );
	m_pCurDisplay = it->second;
	memset( m_pDisplays, 0, sizeof(m_pDisplays) );

	FOREACH_EnabledPlayer( pn )
	{
		RString sNoteSkinLower = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_sNoteSkin;
		NOTESKIN->ValidateNoteSkinName(sNoteSkinLower);
		sNoteSkinLower.MakeLower();
		it = m_NoteDisplays.find( sNoteSkinLower );
		ASSERT_M( it != m_NoteDisplays.end(), sNoteSkinLower );
		m_pDisplays[pn] = it->second;
	}

	InitColumnRenderers();
}

void NoteField::Init( const PlayerState* pPlayerState, float fYReverseOffsetPixels, bool use_states_zoom )
{
	m_pPlayerState = pPlayerState;
	m_fYReverseOffsetPixels = fYReverseOffsetPixels;
	CacheAllUsedNoteSkins();
	// Design change:  Instead of having a flag in the style that toggles a
	// fixed zoom that is only applied to the columns, ScreenGameplay now
	// calculates a zoom factor to apply to the notefield and puts it in the
	// PlayerState. -Kyz
	// use_states_zoom flag exists because edit mode has to set its own special
	// zoom factor. -Kyz
	if(use_states_zoom)
	{
		SetZoom(pPlayerState->m_NotefieldZoom);
	}
	// Pass the player state info down to children so that they can set
	// per-player things.  For example, if a screen filter is in the notefield
	// board, this tells it what player it's for. -Kyz
	Message msg("PlayerStateSet");
	msg.SetParam("PlayerNumber", pPlayerState->m_PlayerNumber);
	HandleMessage(msg);
}

void NoteField::Load( 
	const NoteData *pNoteData,
	int iDrawDistanceAfterTargetsPixels,
	int iDrawDistanceBeforeTargetsPixels )
{
	ASSERT( pNoteData != nullptr );
	m_pNoteData = pNoteData;
	m_iDrawDistanceAfterTargetsPixels = iDrawDistanceAfterTargetsPixels;
	m_iDrawDistanceBeforeTargetsPixels = iDrawDistanceBeforeTargetsPixels;
	ASSERT( m_iDrawDistanceBeforeTargetsPixels >= m_iDrawDistanceAfterTargetsPixels );

	m_FieldRenderArgs.fail_fade = -1;

	//int i1 = m_pNoteData->GetNumTracks();
	//int i2 = GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer;
	ASSERT_M(m_pNoteData->GetNumTracks() == GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer,
		 ssprintf("NumTracks %d = ColsPerPlayer %d",m_pNoteData->GetNumTracks(),
			  GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer));

	// If we're in routine mode, the noteskin is forcibly set to the routine
	// noteskin metrics (which is bad in its own way).  The noteskin set in the
	// options is ignored and probably already set anyway. -Kyz
	if(GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_StyleType != StyleType_TwoPlayersSharedSides)
	{
		ensure_note_displays_have_skin();
	}
	InitColumnRenderers();
}

void NoteField::ensure_note_displays_have_skin()
{
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

void NoteField::InitColumnRenderers()
{
	m_FieldRenderArgs.player_state= m_pPlayerState;
	m_FieldRenderArgs.reverse_offset_pixels= m_fYReverseOffsetPixels;
	m_FieldRenderArgs.receptor_row= &(m_pCurDisplay->m_ReceptorArrowRow);
	m_FieldRenderArgs.ghost_row= &(m_pCurDisplay->m_GhostArrowRow);
	m_FieldRenderArgs.note_data= m_pNoteData;
	m_ColumnRenderers.resize(GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer);
	for(size_t ncr= 0; ncr < m_ColumnRenderers.size(); ++ncr)
	{
		FOREACH_EnabledPlayer(pn)
		{
			m_ColumnRenderers[ncr].m_displays[pn]= &(m_pDisplays[pn]->display[ncr]);
		}
		m_ColumnRenderers[ncr].m_displays[PLAYER_INVALID]= &(m_pCurDisplay->display[ncr]);
		m_ColumnRenderers[ncr].m_column= ncr;
		m_ColumnRenderers[ncr].m_column_render_args.column= ncr;
		m_ColumnRenderers[ncr].m_field_render_args= &m_FieldRenderArgs;
	}
	m_pCurDisplay->m_ReceptorArrowRow.SetColumnRenderers(m_ColumnRenderers);
	m_pCurDisplay->m_GhostArrowRow.SetColumnRenderers(m_ColumnRenderers);
}

void NoteField::Update( float fDeltaTime )
{
	if( m_bFirstUpdate )
	{
		m_pCurDisplay->m_ReceptorArrowRow.PlayCommand( "On" );
	}

	ActorFrame::Update( fDeltaTime );
	ArrowEffects::SetCurrentOptions(&m_pPlayerState->m_PlayerOptions.GetCurrent());

	for(size_t c= 0; c < m_ColumnRenderers.size(); ++c)
	{
		m_ColumnRenderers[c].Update(fDeltaTime);
	}

	// update m_fBoardOffsetPixels, m_fCurrentBeatLastUpdate, m_fYPosCurrentBeatLastUpdate
	const float fCurrentBeat = m_pPlayerState->GetDisplayedPosition().m_fSongBeat;
	bool bTweeningOn = m_sprBoard->GetCurrentDiffuseAlpha() >= 0.98  &&  m_sprBoard->GetCurrentDiffuseAlpha() < 1.00;	// HACK
	if( !bTweeningOn  &&  m_fCurrentBeatLastUpdate != -1 )
	{
		const float fYOffsetLast	= ArrowEffects::GetYOffset(m_pPlayerState, 0, m_fCurrentBeatLastUpdate);
		const float fYPosLast= ArrowEffects::GetYPos(m_pPlayerState, 0, fYOffsetLast, m_fYReverseOffsetPixels);
		const float fPixelDifference = fYPosLast - m_fYPosCurrentBeatLastUpdate;

		//LOG->Trace( "speed = %f, %f, %f, %f, %f, %f", fSpeed, fYOffsetAtCurrent, fYOffsetAtNext, fSecondsAtCurrent, fSecondsAtNext, fPixelDifference, fSecondsDifference );

		m_fBoardOffsetPixels += fPixelDifference;
		wrap( m_fBoardOffsetPixels, m_sprBoard->GetUnzoomedHeight() );
	}
	m_fCurrentBeatLastUpdate = fCurrentBeat;
	const float fYOffsetCurrent	= ArrowEffects::GetYOffset( m_pPlayerState, 0, m_fCurrentBeatLastUpdate );
	m_fYPosCurrentBeatLastUpdate= ArrowEffects::GetYPos(m_pPlayerState, 0, fYOffsetCurrent, m_fYReverseOffsetPixels);

	m_rectMarkerBar.Update( fDeltaTime );

	NoteDisplayCols *cur = m_pCurDisplay;

	cur->m_ReceptorArrowRow.Update( fDeltaTime );
	cur->m_GhostArrowRow.Update( fDeltaTime );

	if( m_FieldRenderArgs.fail_fade >= 0 )
		m_FieldRenderArgs.fail_fade = min( m_FieldRenderArgs.fail_fade + fDeltaTime/FADE_FAIL_TIME, 1 );

	// Update fade to failed
	m_pCurDisplay->m_ReceptorArrowRow.SetFadeToFailPercent( m_FieldRenderArgs.fail_fade );

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
	const Style* pStyle = GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber);
	float fMinX, fMaxX;
	// TODO: Remove use of PlayerNumber.
	pStyle->GetMinAndMaxColX( m_pPlayerState->m_PlayerNumber, fMinX, fMaxX );

	const float fYZoom	= ArrowEffects::GetZoom( m_pPlayerState, 0, 0 );
	return (fMaxX - fMinX + ARROW_SIZE) * fYZoom;
}

void NoteField::DrawBeatBar( const float fBeat, BeatBarType type, int iMeasureIndex )
{
	bool bIsMeasure = type == measure;

	const float fYOffset	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fBeat );
	const float fYPos= ArrowEffects::GetYPos(m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels);

	float fAlpha;
	int iState;

	if( bIsMeasure )
	{
		fAlpha = BAR_MEASURE_ALPHA;
		iState = 0;
	}
	else
	{
		PlayerOptions const& curr_ops= m_pPlayerState->m_PlayerOptions.GetCurrent();
		float fScrollSpeed = curr_ops.m_fScrollSpeed;
		if(curr_ops.m_fTimeSpacing > 0)
		{
			fScrollSpeed = 4;
		}
		else if(curr_ops.m_fMaxScrollBPM != 0)
		{
			fScrollSpeed= curr_ops.m_fMaxScrollBPM / m_pPlayerState->m_fReadBPM;
		}
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
	// todo: make this an AutoActor instead? -aj
	Sprite *pSprite = dynamic_cast<Sprite *>( (Actor*)m_sprBoard );
	if( pSprite == nullptr )
	{
		m_sprBoard->Draw();
	}
	else
	{
		// Draw the board centered on fYPosAt0 so that the board doesn't slide as
		// the draw distance changes with modifiers.
		const float fYPosAt0= ArrowEffects::GetYPos(m_pPlayerState, 0, 0, m_fYReverseOffsetPixels);

		RectF rect = *pSprite->GetCurrentTextureCoordRect();
		const float fBoardGraphicHeightPixels = pSprite->GetUnzoomedHeight();
		float fTexCoordOffset = m_fBoardOffsetPixels / fBoardGraphicHeightPixels;

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
	const float fYPos	= ArrowEffects::GetYPos(m_pPlayerState, 0, fYOffset, m_fYReverseOffsetPixels);

	m_rectMarkerBar.StretchTo( RectF(-GetWidth()/2, fYPos-ARROW_SIZE/2, GetWidth()/2, fYPos+ARROW_SIZE/2) );
	m_rectMarkerBar.Draw();
}

static ThemeMetric<RageColor> AREA_HIGHLIGHT_COLOR("NoteField", "AreaHighlightColor");
void NoteField::DrawAreaHighlight( int iStartBeat, int iEndBeat )
{
	float fStartBeat = NoteRowToBeat( iStartBeat );
	float fEndBeat = NoteRowToBeat( iEndBeat );
	float fDrawDistanceAfterTargetsPixels	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fStartBeat );
	float fYStartPos	= ArrowEffects::GetYPos(m_pPlayerState, 0, fDrawDistanceAfterTargetsPixels, m_fYReverseOffsetPixels);
	float fDrawDistanceBeforeTargetsPixels	= ArrowEffects::GetYOffset( m_pPlayerState, 0, fEndBeat );
	float fYEndPos= ArrowEffects::GetYPos(m_pPlayerState, 0, fDrawDistanceBeforeTargetsPixels, m_fYReverseOffsetPixels);

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
static ThemeMetric<RageColor> TIME_SIG_COLOR ( "NoteField", "TimeSignatureColor" );
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
static ThemeMetric<bool> TIME_SIG_IS_LEFT_SIDE ( "NoteField", "TimeSignatureIsLeftSide" );
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
static ThemeMetric<float> TIME_SIG_OFFSETX ( "NoteField", "TimeSignatureOffsetX" );
static ThemeMetric<float> TICKCOUNT_OFFSETX ( "NoteField", "TickcountOffsetX" );
static ThemeMetric<float> COMBO_OFFSETX ( "NoteField", "ComboOffsetX" );
static ThemeMetric<float> LABEL_OFFSETX ( "NoteField", "LabelOffsetX" );
static ThemeMetric<float> SPEED_OFFSETX ( "NoteField", "SpeedOffsetX" );
static ThemeMetric<float> SCROLL_OFFSETX ( "NoteField", "ScrollOffsetX" );
static ThemeMetric<float> FAKE_OFFSETX ( "NoteField", "FakeOffsetX" );

void NoteField::set_text_measure_number_for_draw(
	const float beat, const float side_sign, float x_offset,
	const float horiz_align, const RageColor& color, const RageColor& glow)
{
	const float y_offset= ArrowEffects::GetYOffset(m_pPlayerState, 0, beat);
	const float y_pos= ArrowEffects::GetYPos(m_pPlayerState, 0, y_offset, m_fYReverseOffsetPixels);
	const float zoom= ArrowEffects::GetZoom(m_pPlayerState, y_offset, 0);
	const float x_base= GetWidth() * .5f;
	x_offset*= zoom;

	m_textMeasureNumber.SetZoom(zoom);
	m_textMeasureNumber.SetHorizAlign(horiz_align);
	m_textMeasureNumber.SetDiffuse(color);
	m_textMeasureNumber.SetGlow(glow);
	m_textMeasureNumber.SetXY((x_offset + x_base) * side_sign, y_pos);
}

void NoteField::draw_timing_segment_text(const RString& text,
	const float beat, const float side_sign, float x_offset,
	const float horiz_align, const RageColor& color, const RageColor& glow)
{
	set_text_measure_number_for_draw(beat, side_sign, x_offset, horiz_align,
		color, glow);
	m_textMeasureNumber.SetText(text);
	m_textMeasureNumber.Draw();
}

void NoteField::DrawAttackText(const float beat, const Attack &attack,
	const RageColor& glow)
{
	set_text_measure_number_for_draw(beat, 1, 10, align_left,
		RageColor(0,0.8f,0.8f,1), glow);
	m_textMeasureNumber.SetText( attack.GetTextDescription() );
	m_textMeasureNumber.Draw();
}

void NoteField::DrawBGChangeText(const float beat, const RString new_bg_name,
	const RageColor& glow)
{
	set_text_measure_number_for_draw(beat, 1, 0, align_left, RageColor(0,1,0,1),
		glow);
	m_textMeasureNumber.SetText(new_bg_name);
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

bool NoteField::IsOnScreen( float fBeat, int iCol, int iDrawDistanceAfterTargetsPixels, int iDrawDistanceBeforeTargetsPixels ) const
{
	// IMPORTANT:  Do not modify this function without also modifying the
	// version that is in NoteDisplay.cpp or coming up with a good way to
	// merge them. -Kyz
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

void NoteField::CalcPixelsBeforeAndAfterTargets()
{
	const PlayerOptions& curr_options= m_pPlayerState->m_PlayerOptions.GetCurrent();
	// Adjust draw range depending on some effects
	m_FieldRenderArgs.draw_pixels_after_targets= m_iDrawDistanceAfterTargetsPixels * (1.f + curr_options.m_fDrawSizeBack);
	// HACK: If boomerang and centered are on, then we want to draw much 
	// earlier so that the notes don't pop on screen.
	float centered_times_boomerang=
		curr_options.m_fScrolls[PlayerOptions::SCROLL_CENTERED] *
		curr_options.m_fAccels[PlayerOptions::ACCEL_BOOMERANG];
	m_FieldRenderArgs.draw_pixels_after_targets +=
		int(SCALE(centered_times_boomerang, 0.f, 1.f, 0.f, -SCREEN_HEIGHT/2));
	m_FieldRenderArgs.draw_pixels_before_targets =
		m_iDrawDistanceBeforeTargetsPixels * (1.f + curr_options.m_fDrawSize);

	float draw_scale= 1;
	draw_scale*= 1 + 0.5f * fabsf(curr_options.m_fPerspectiveTilt);
	draw_scale*= 1 + fabsf(curr_options.m_fEffects[PlayerOptions::EFFECT_MINI]);

	m_FieldRenderArgs.draw_pixels_after_targets=
		(int)(m_FieldRenderArgs.draw_pixels_after_targets * draw_scale);
	m_FieldRenderArgs.draw_pixels_before_targets=
		(int)(m_FieldRenderArgs.draw_pixels_before_targets * draw_scale);
}

void NoteField::DrawPrimitives()
{
	//LOG->Trace( "NoteField::DrawPrimitives()" );

	// This should be filled in on the first update.
	ASSERT( m_pCurDisplay != nullptr );

	// ArrowEffects::Update call moved because having it happen once per
	// NoteField (which means twice in two player) seemed wasteful. -Kyz

	if(m_drawing_board_primitive)
	{
		CalcPixelsBeforeAndAfterTargets();
		DrawBoard(m_FieldRenderArgs.draw_pixels_after_targets,
			m_FieldRenderArgs.draw_pixels_before_targets);
		return;
	}
	// Some might prefer an else block, instead of returning from the if, but I
	// don't want to bump the indent on the entire remaining section. -Kyz
	ArrowEffects::SetCurrentOptions(&m_pPlayerState->m_PlayerOptions.GetCurrent());

	CalcPixelsBeforeAndAfterTargets();
	NoteDisplayCols *cur = m_pCurDisplay;
	// Probe for first and last notes on the screen
	float first_beat_to_draw= FindFirstDisplayedBeat(
		m_pPlayerState, m_FieldRenderArgs.draw_pixels_after_targets);
	float last_beat_to_draw= FindLastDisplayedBeat(
		m_pPlayerState, m_FieldRenderArgs.draw_pixels_before_targets);

	m_pPlayerState->m_fLastDrawnBeat = last_beat_to_draw;

	m_FieldRenderArgs.first_row  = BeatToNoteRow(first_beat_to_draw);
	m_FieldRenderArgs.last_row   = BeatToNoteRow(last_beat_to_draw);

	//LOG->Trace( "start = %f.1, end = %f.1", first_beat_to_draw-fSongBeat, last_beat_to_draw-fSongBeat );
	//LOG->Trace( "Drawing elements %d through %d", m_FieldRenderArgs.first_row, m_FieldRenderArgs.last_row );

#define IS_ON_SCREEN(fBeat)  (first_beat_to_draw <= (fBeat) && (fBeat) <= last_beat_to_draw && IsOnScreen(fBeat, 0, m_FieldRenderArgs.draw_pixels_after_targets, m_FieldRenderArgs.draw_pixels_before_targets))

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
	if( ( GAMESTATE->IsEditing() || SHOW_BEAT_BARS ) && pTiming != nullptr )
	{
		const vector<TimingSegment *> &tSigs = *segs[SEGMENT_TIME_SIG];
		int iMeasureIndex = 0;
		for (i = 0; i < tSigs.size(); i++)
		{
			const TimeSignatureSegment *ts = ToTimeSignature(tSigs[i]);
			int iSegmentEndRow = (i + 1 == tSigs.size()) ? m_FieldRenderArgs.last_row : tSigs[i+1]->GetRow();

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

	if( GAMESTATE->IsEditing() && pTiming != nullptr )
	{
		ASSERT(GAMESTATE->m_pCurSong != nullptr);

		const TimingData &timing = *pTiming;
		const RageColor text_glow= RageColor(1,1,1,RageFastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f);

		float horiz_align= align_right;
		float side_sign= 1;
#define draw_all_segments(str_exp, name, caps_name)	\
		horiz_align= caps_name##_IS_LEFT_SIDE ? align_right : align_left; \
		side_sign= caps_name##_IS_LEFT_SIDE ? -1 : 1; \
		for(unsigned int i= 0; i < segs[SEGMENT_##caps_name]->size(); ++i) \
		{ \
			const name##Segment* seg= To##name((*segs[SEGMENT_##caps_name])[i]); \
			if(seg->GetRow() >= m_FieldRenderArgs.first_row && \
				seg->GetRow() <= m_FieldRenderArgs.last_row && \
				IS_ON_SCREEN(seg->GetBeat())) \
			{ \
				draw_timing_segment_text(str_exp, seg->GetBeat(), side_sign, \
					caps_name##_OFFSETX, horiz_align, caps_name##_COLOR, text_glow); \
			} \
		}

		draw_all_segments(std::to_string(seg->GetRatio()), Scroll, SCROLL);
		draw_all_segments(std::to_string(seg->GetBPM()), BPM, BPM);
		draw_all_segments(std::to_string(seg->GetPause()), Stop, STOP);
		draw_all_segments(std::to_string(seg->GetPause()), Delay, DELAY);
		draw_all_segments(std::to_string(seg->GetLength()), Warp, WARP);
		draw_all_segments(ssprintf("%d\n--\n%d", seg->GetNum(), seg->GetDen()),
			TimeSignature, TIME_SIG);
		draw_all_segments(ssprintf("%d", seg->GetTicks()), Tickcount, TICKCOUNT);
		draw_all_segments(
			ssprintf("%d/%d", seg->GetCombo(), seg->GetMissCombo()), Combo, COMBO);
		draw_all_segments(seg->GetLabel(), Label, LABEL);
		draw_all_segments(ssprintf("%s\n%s\n%s",
				std::to_string(seg->GetRatio()).c_str(),
				(seg->GetUnit() == 1 ? "S" : "B"),
				std::to_string(seg->GetDelay()).c_str()), Speed, SPEED);
		draw_all_segments(std::to_string(seg->GetLength()), Fake, FAKE);
#undef draw_all_segments

		// Course mods text
		const Course *pCourse = GAMESTATE->m_pCurCourse;
		if( pCourse )
		{
			ASSERT_M( GAMESTATE->m_iEditCourseEntryIndex >= 0  &&  GAMESTATE->m_iEditCourseEntryIndex < (int)pCourse->m_vEntries.size(), 
				ssprintf("%i",GAMESTATE->m_iEditCourseEntryIndex.Get()) );
			const CourseEntry &ce = pCourse->m_vEntries[GAMESTATE->m_iEditCourseEntryIndex];
			for (Attack const &a : ce.attacks)
			{
				float fSecond = a.fStartSecond;
				float fBeat = timing.GetBeatFromElapsedTime( fSecond );

				if( BeatToNoteRow(fBeat) >= m_FieldRenderArgs.first_row &&
					BeatToNoteRow(fBeat) <= m_FieldRenderArgs.last_row &&
					IS_ON_SCREEN(fBeat))
				{
					DrawAttackText(fBeat, a, text_glow);
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
				for (Attack const &a : attacks)
				{
					float fBeat = timing.GetBeatFromElapsedTime(a.fStartSecond);
					if (BeatToNoteRow(fBeat) >= m_FieldRenderArgs.first_row &&
						BeatToNoteRow(fBeat) <= m_FieldRenderArgs.last_row &&
						IS_ON_SCREEN(fBeat))
					{
						this->DrawAttackText(fBeat, a, text_glow);
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

						for(;;)
						{
							float fLowestBeat = FLT_MAX;
							vector<BackgroundLayer> viLowestIndex;

							FOREACH_BackgroundLayer( j )
							{
								if( iter[j] == GAMESTATE->m_pCurSong->GetBackgroundChanges(j).end() )
								{
									continue;
								}
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
								{
									ASSERT( iter[j] == GAMESTATE->m_pCurSong->GetBackgroundChanges(j).end() );
								}
								break;
							}
	
							if( IS_ON_SCREEN(fLowestBeat) )
							{
								vector<RString> vsBGChanges;
								for (BackgroundLayer const &bl : viLowestIndex)
								{
									ASSERT( iter[bl] != GAMESTATE->m_pCurSong->GetBackgroundChanges(bl).end() );
									const BackgroundChange& change = *iter[bl];
									RString s = change.GetTextDescription();
									if( bl!=0 )
									{
										s = ssprintf("%d: ",bl) + s;
									}
									vsBGChanges.push_back( s );
								}
								DrawBGChangeText(fLowestBeat, join("\n",vsBGChanges), text_glow);
							}
							for (BackgroundLayer const &bl : viLowestIndex)
							{
								iter[bl]++;
							}
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
			CLAMP( iBegin, m_FieldRenderArgs.first_row, m_FieldRenderArgs.last_row );
			CLAMP( iEnd, m_FieldRenderArgs.first_row, m_FieldRenderArgs.last_row );
			DrawAreaHighlight( iBegin, iEnd );
		}
		else if( m_iBeginMarker != -1 )
		{
			if( m_iBeginMarker >= m_FieldRenderArgs.first_row &&
				m_iBeginMarker <= m_FieldRenderArgs.last_row )
				DrawMarkerBar( m_iBeginMarker );
		}
		else if( m_iEndMarker != -1 )
		{
			if( m_iEndMarker >= m_FieldRenderArgs.first_row &&
				m_iEndMarker <= m_FieldRenderArgs.last_row )
			DrawMarkerBar( m_iEndMarker );
		}
	}

	// Optimization is very important here because there are so many arrows to draw.
	// Draw the arrows in order of column. This minimizes texture switches and
	// lets us draw in big batches.

	const Style* pStyle = GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber);
	ASSERT_M(m_pNoteData->GetNumTracks() == GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer, 
		ssprintf("NumTracks %d != ColsPerPlayer %d",m_pNoteData->GetNumTracks(), 
			GAMESTATE->GetCurrentStyle(m_pPlayerState->m_PlayerNumber)->m_iColsPerPlayer));

	if(*m_FieldRenderArgs.selection_begin_marker != -1 &&
		*m_FieldRenderArgs.selection_end_marker != -1)
	{
		m_FieldRenderArgs.selection_glow= SCALE(
			RageFastCos(RageTimer::GetTimeSinceStartFast()*2), -1, 1, 0.1f, 0.3f);
	}
	m_FieldRenderArgs.fade_before_targets= FADE_BEFORE_TARGETS_PERCENT;

	for( int j=0; j<m_pNoteData->GetNumTracks(); j++ )	// for each arrow column
	{
		const int c = pStyle->m_iColumnDrawOrder[j];
		m_ColumnRenderers[c].Draw();
	}

	cur->m_GhostArrowRow.Draw();
}

void NoteField::DrawBoardPrimitive()
{
	if(!SHOW_BOARD)
	{
		return;
	}
	m_drawing_board_primitive= true;
	Draw();
	m_drawing_board_primitive= false;
}

void NoteField::FadeToFail()
{
	m_FieldRenderArgs.fail_fade = max( 0.0f, m_FieldRenderArgs.fail_fade );	// this will slowly increase every Update()
		// don't fade all over again if this is called twice
}

// A few functions and macros to take care of processing the callback
// return values, since the code would be identical in all of them. -Kyz

#define OPEN_CALLBACK_BLOCK(member_name) \
	if(!from_lua && !member_name.IsNil()) \
	{ \
		Lua* L= LUA->Get(); \
		member_name.PushSelf(L);

#define OPEN_RUN_BLOCK(arg_count) \
	RString error= "Error running callback: "; \
	if(LuaHelpers::RunScriptOnStack(L, error, arg_count, arg_count, true)) \
	{

#define CLOSE_RUN_AND_CALLBACK_BLOCKS  } lua_settop(L, 0);  LUA->Release(L); }
#define PUSH_COLUMN lua_pushnumber(L, col+1)

static void get_returned_column(Lua* L, PlayerNumber pn, int index, int& col)
{
	if(lua_isnumber(L, index))
	{
		// 1-indexed columns in lua
		int tmpcol= lua_tonumber(L, index) - 1;
		if(tmpcol < 0 || tmpcol >= GAMESTATE->GetCurrentStyle(pn)->m_iColsPerPlayer)
		{
			LuaHelpers::ReportScriptErrorFmt(
				"Column returned by callback must be between 1 and %d "
				"(GAMESTATE:GetCurrentStyle():ColumnsPerPlayer()).",
				GAMESTATE->GetCurrentStyle(pn)->m_iColsPerPlayer);
		}
		else
		{
			col= tmpcol;
		}
	}
}

// Templated so it can be used for TNS and HNS. -Kyz
template<class T> static void get_returned_score(Lua* L, int index, T& score)
{
	T maybe_score= Enum::Check<T>(L, index, true, true);
	if(maybe_score != EnumTraits<T>::Invalid)
	{
		score= maybe_score;
	}
}

static void get_returned_bright(Lua* L, int index, bool& bright)
{
	if(lua_isboolean(L, index))
	{
		bright= lua_toboolean(L, index);
	}
}

void NoteField::Step(int col, TapNoteScore score, bool from_lua)
{
	OPEN_CALLBACK_BLOCK(m_StepCallback);
	PUSH_COLUMN;
	Enum::Push(L, score);
	OPEN_RUN_BLOCK(2);
	get_returned_column(L, m_pPlayerState->m_PlayerNumber, 1, col);
	get_returned_score(L, 2, score);
	CLOSE_RUN_AND_CALLBACK_BLOCKS;
	m_pCurDisplay->m_ReceptorArrowRow.Step(col, score);
}
void NoteField::SetPressed(int col, bool from_lua)
{
	OPEN_CALLBACK_BLOCK(m_SetPressedCallback);
	PUSH_COLUMN;
	OPEN_RUN_BLOCK(1);
	get_returned_column(L, m_pPlayerState->m_PlayerNumber, 1, col);
	CLOSE_RUN_AND_CALLBACK_BLOCKS;
	m_pCurDisplay->m_ReceptorArrowRow.SetPressed(col);
}
void NoteField::DidTapNote(int col, TapNoteScore score, bool bright, bool from_lua)
{
	OPEN_CALLBACK_BLOCK(m_DidTapNoteCallback);
	PUSH_COLUMN;
	Enum::Push(L, score);
	lua_pushboolean(L, bright);
	OPEN_RUN_BLOCK(3);
	get_returned_column(L, m_pPlayerState->m_PlayerNumber, 1, col);
	get_returned_score(L, 2, score);
	get_returned_bright(L, 3, bright);
	CLOSE_RUN_AND_CALLBACK_BLOCKS;
	m_pCurDisplay->m_GhostArrowRow.DidTapNote(col, score, bright);
}
void NoteField::DidHoldNote(int col, HoldNoteScore score, bool bright, bool from_lua)
{
	OPEN_CALLBACK_BLOCK(m_DidHoldNoteCallback);
	PUSH_COLUMN;
	Enum::Push(L, score);
	lua_pushboolean(L, bright);
	OPEN_RUN_BLOCK(3);
	get_returned_column(L, m_pPlayerState->m_PlayerNumber, 1, col);
	get_returned_score(L, 2, score);
	get_returned_bright(L, 3, bright);
	CLOSE_RUN_AND_CALLBACK_BLOCKS;
	m_pCurDisplay->m_GhostArrowRow.DidHoldNote(col, score, bright);
}

#undef OPEN_CALLBACK_BLOCK
#undef OPEN_RUN_BLOCK
#undef CLOSE_RUN_AND_CALLBACK_BLOCKS
#undef PUSH_COLUMN

void NoteField::HandleMessage( const Message &msg )
{
	if( msg == Message_CurrentSongChanged )
	{
		m_fCurrentBeatLastUpdate = -1;
		m_fYPosCurrentBeatLastUpdate = -1;
	}

	ActorFrame::HandleMessage( msg );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Notefield. */
class LunaNoteField: public Luna<NoteField>
{
public:
#define SET_CALLBACK_GENERIC(callback_name, member_name) \
	static int callback_name(T* p, lua_State* L) \
	{ \
		if(lua_isnoneornil(L, 1)) \
		{ \
			p->member_name.SetFromNil(); \
		} \
		else if(lua_isfunction(L, 1)) \
		{ \
			p->member_name.SetFromStack(L); \
		} \
		else \
		{ \
			luaL_error(L, #callback_name "Callback argument must be nil (to clear the callback) or a function (to set the callback)."); \
		} \
		return 0; \
	}
	SET_CALLBACK_GENERIC(set_step_callback, m_StepCallback);
	SET_CALLBACK_GENERIC(set_set_pressed_callback, m_SetPressedCallback);
	SET_CALLBACK_GENERIC(set_did_tap_note_callback, m_DidTapNoteCallback);
	SET_CALLBACK_GENERIC(set_did_hold_note_callback, m_DidHoldNoteCallback);
#undef SET_CALLBACK_GENERIC

	static int check_column(lua_State* L, int index, PlayerNumber pn)
	{
		// 1-indexed columns in lua
		int col= IArg(1)-1;
		if(col < 0 || col >= GAMESTATE->GetCurrentStyle(pn)->m_iColsPerPlayer)
		{
			luaL_error(L, "Column must be between 1 and %d "
				"(GAMESTATE:GetCurrentStyle(pn):ColumnsPerPlayer()).",
				GAMESTATE->GetCurrentStyle(pn)->m_iColsPerPlayer);
		}
		return col;
	}

	static int step(T* p, lua_State* L)
	{
		int col= check_column(L, 1, p->GetPlayerState()->m_PlayerNumber);
		TapNoteScore tns= Enum::Check<TapNoteScore>(L, 2);
		p->Step(col, tns, true);
		return 0;
	}

	static int set_pressed(T* p, lua_State* L)
	{
		int col= check_column(L, 1, p->GetPlayerState()->m_PlayerNumber);
		p->SetPressed(col, true);
		return 0;
	}

	static int did_tap_note(T* p, lua_State* L)
	{
		int col= check_column(L, 1, p->GetPlayerState()->m_PlayerNumber);
		TapNoteScore tns= Enum::Check<TapNoteScore>(L, 2);
		bool bright= BArg(3);
		p->DidTapNote(col, tns, bright, true);
		return 0;
	}

	static int did_hold_note(T* p, lua_State* L)
	{
		int col= check_column(L, 1, p->GetPlayerState()->m_PlayerNumber);
		HoldNoteScore hns= Enum::Check<HoldNoteScore>(L, 2);
		bool bright= BArg(3);
		p->DidHoldNote(col, hns, bright, true);
		return 0;
	}

	static int get_column_actors(T* p, lua_State* L)
	{
		lua_createtable(L, p->m_ColumnRenderers.size(), 0);
		for(size_t i= 0; i < p->m_ColumnRenderers.size(); ++i)
		{
			p->m_ColumnRenderers[i].PushSelf(L);
			lua_rawseti(L, -2, i+1);
		}
		return 1;
	}

	LunaNoteField()
	{
		ADD_METHOD(set_step_callback);
		ADD_METHOD(set_set_pressed_callback);
		ADD_METHOD(set_did_tap_note_callback);
		ADD_METHOD(set_did_hold_note_callback);
		ADD_METHOD(step);
		ADD_METHOD(set_pressed);
		ADD_METHOD(did_tap_note);
		ADD_METHOD(did_hold_note);
		ADD_METHOD(get_column_actors);
	}
};

LUA_REGISTER_DERIVED_CLASS(NoteField, ActorFrame)
// lua end

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
