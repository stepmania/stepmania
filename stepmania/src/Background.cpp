#include "global.h"
#include "Background.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageException.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageTextureManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "NoteTypes.h"
#include "Steps.h"
#include "DancingCharacters.h"
#include "arch/arch.h"
#include "BeginnerHelper.h"
#include "StageStats.h"

#include <set>


const float FADE_SECONDS = 1.0f;

#define LEFT_EDGE			THEME->GetMetricF("Background","LeftEdge")
#define TOP_EDGE			THEME->GetMetricF("Background","TopEdge")
#define RIGHT_EDGE			THEME->GetMetricF("Background","RightEdge")
#define BOTTOM_EDGE			THEME->GetMetricF("Background","BottomEdge")
CachedThemeMetricB BLINK_DANGER_ALL("Background","BlinkDangerAll");
CachedThemeMetricB DANGER_ALL_IS_OPAQUE("Background","DangerAllIsOpaque");
#define BRIGHTNESS_FADE_COMMAND THEME->GetMetric("Background","BrightnessFadeCommand")
#define RECT_BACKGROUND RectF(LEFT_EDGE,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE)

static float g_fBackgroundCenterWidth = 40;
const CString STATIC_BACKGROUND = "static background";

Background::Background()
{
	BLINK_DANGER_ALL.Refresh();
	DANGER_ALL_IS_OPAQUE.Refresh();

	m_iCurBGChangeIndex = -1;
	m_pCurrentBGA = NULL;
	m_pFadingBGA = NULL;
	m_fSecsLeftInFade = 0;

	m_DangerAll.LoadFromAniDir( THEME->GetPathToB("ScreenGameplay danger all") );
	FOREACH_PlayerNumber( p )
		m_DangerPlayer[p].LoadFromAniDir( THEME->GetPathToB(ssprintf("ScreenGameplay danger p%d",p+1)) );
	FOREACH_PlayerNumber( p )
		m_DeadPlayer[p].LoadFromAniDir( THEME->GetPathToB(ssprintf("ScreenGameplay dead p%d",p+1)) );

	bool bOneOrMoreChars = false;
	bool bShowingBeginnerHelper = false;
	FOREACH_PlayerNumber( p )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

		bOneOrMoreChars = true;
		// Disable dancing characters if BH will be showing.
		if( PREFSMAN->m_bShowBeginnerHelper && BeginnerHelper::CanUse() && 
			GAMESTATE->m_pCurSteps[p] && GAMESTATE->m_pCurSteps[p]->GetDifficulty() == DIFFICULTY_BEGINNER )
			bShowingBeginnerHelper = true;
	}

	if( bOneOrMoreChars && !bShowingBeginnerHelper )
		m_pDancingCharacters = new DancingCharacters;
	else
		m_pDancingCharacters = NULL;

	m_quadBorder[0].StretchTo( RectF(SCREEN_LEFT,SCREEN_TOP,LEFT_EDGE,SCREEN_BOTTOM) );
	m_quadBorder[0].SetDiffuse( RageColor(0,0,0,1) );
	m_quadBorder[1].StretchTo( RectF(LEFT_EDGE,SCREEN_TOP,RIGHT_EDGE,TOP_EDGE) );
	m_quadBorder[1].SetDiffuse( RageColor(0,0,0,1) );
	m_quadBorder[2].StretchTo( RectF(RIGHT_EDGE,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_quadBorder[2].SetDiffuse( RageColor(0,0,0,1) );
	m_quadBorder[3].StretchTo( RectF(LEFT_EDGE,BOTTOM_EDGE,RIGHT_EDGE,SCREEN_BOTTOM) );
	m_quadBorder[3].SetDiffuse( RageColor(0,0,0,1) );

	this->AddChild( &m_quadBorder[0] );
	this->AddChild( &m_quadBorder[1] );
	this->AddChild( &m_quadBorder[2] );
	this->AddChild( &m_quadBorder[3] );

	this->AddChild( &m_Brightness );
}

Background::~Background()
{
	Unload();
	delete m_pDancingCharacters;
}

void Background::Unload()
{
    for( map<CString,BGAnimation*>::iterator iter = m_BGAnimations.begin();
		 iter != m_BGAnimations.end();
		 iter++ )
		delete iter->second;
	m_BGAnimations.clear();
	m_RandomBGAnimations.clear();
	m_aBGChanges.clear();

	m_pCurrentBGA = NULL;
	m_pFadingBGA = NULL;
	m_pSong = NULL;
	m_fSecsLeftInFade = 0;
	m_iCurBGChangeIndex = -1;
	m_fLastMusicSeconds	= -9999;
}

void Background::LoadFromAniDir( CString sAniDir )
{
	Unload();

	if( PREFSMAN->m_fBGBrightness == 0 )
		return;

	BGAnimation* pTempBGA;
	pTempBGA = new BGAnimation;
	pTempBGA->LoadFromAniDir( sAniDir );
	m_BGAnimations[STATIC_BACKGROUND] = pTempBGA;
	m_aBGChanges.push_back( BackgroundChange(-1000, STATIC_BACKGROUND) );
}

BGAnimation *Background::CreateSongBGA( CString sBGName ) const
{
	BGAnimation *pTempBGA;

	// Using aniseg.m_sBGName, search for the corresponding animation.
	// Look in this order:  movies in song dir, BGAnims in song dir
	//  movies in RandomMovies dir, BGAnims in BGAnimsDir.
	CStringArray asFiles;

	// Look for BGAnims in the song dir
	GetDirListing( m_pSong->GetSongDir()+sBGName, asFiles, true, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromAniDir( asFiles[0] );
		return pTempBGA;
	}
	// Look for BG movies or static graphics in the song dir
	GetDirListing( m_pSong->GetSongDir()+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		const CString sExt = GetExtension( asFiles[0]) ;
		if( sExt.CompareNoCase("avi")==0 ||
			sExt.CompareNoCase("mpg")==0 ||
			sExt.CompareNoCase("mpeg")==0 )
			pTempBGA->LoadFromMovie( asFiles[0] );
		else
			pTempBGA->LoadFromStaticGraphic( asFiles[0] );
		return pTempBGA;
	}
	// Look for movies in the RandomMovies dir
	GetDirListing( RANDOMMOVIES_DIR+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromMovie( asFiles[0] );
		return pTempBGA;
	}

	// Look for BGAnims in the BGAnims dir
	GetDirListing( BG_ANIMS_DIR+sBGName, asFiles, true, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromAniDir( asFiles[0] );
		return pTempBGA;
	}

	// Look for BGAnims in the BGAnims dir
	GetDirListing( VISUALIZATIONS_DIR+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromVisualization( asFiles[0] );
		return pTempBGA;
	}

	// There is no background by this name.  
	return NULL;
}

CString Background::CreateRandomBGA()
{
	if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_OFF )
		return "";

	/* If we already have enough random BGAs loaded, use them round-robin. */
	if( (int) m_RandomBGAnimations.size() >= PREFSMAN->m_iNumBackgrounds )
	{
		/* XXX: every time we fully loop, shuffle, so we don't play the same sequence
		 * over and over; and nudge the shuffle so the next one won't be a repeat */
		const CString first = m_RandomBGAnimations.front();
		m_RandomBGAnimations.push_back( m_RandomBGAnimations.front() );
		m_RandomBGAnimations.pop_front();
		return first;
	}

	CStringArray arrayPaths;
	switch( PREFSMAN->m_BackgroundMode )
	{
	default:
		FAIL_M( ssprintf("Invalid BackgroundMode: %i", PREFSMAN->m_BackgroundMode) );
		break;

	case PrefsManager::BGMODE_ANIMATIONS:
		GetDirListing( BG_ANIMS_DIR+"*", arrayPaths, true, true );
		break;

	case PrefsManager::BGMODE_MOVIEVIS:
		GetDirListing( VISUALIZATIONS_DIR + "*.avi", arrayPaths, false, true );
		GetDirListing( VISUALIZATIONS_DIR + "*.mpg", arrayPaths, false, true );
		GetDirListing( VISUALIZATIONS_DIR + "*.mpeg", arrayPaths, false, true );
		break;

	case PrefsManager::BGMODE_RANDOMMOVIES:
		GetDirListing( RANDOMMOVIES_DIR + "*.avi", arrayPaths, false, true );
		GetDirListing( RANDOMMOVIES_DIR + "*.mpg", arrayPaths, false, true );
		GetDirListing( RANDOMMOVIES_DIR + "*.mpeg", arrayPaths, false, true );
		break;
	}

	// strip out "cvs"
	for( int j=arrayPaths.size()-1; j>=0; j-- )
		if( !Basename(arrayPaths[j]).CompareNoCase("cvs") )
			arrayPaths.erase( arrayPaths.begin()+j, arrayPaths.begin()+j+1 );

	if( arrayPaths.empty() )
		return "";

	random_shuffle( arrayPaths.begin(), arrayPaths.end() );

	/* Find the first file in arrayPaths we havn't already loaded. */
	CString file;
	{
		set<CString> loaded;
		unsigned i;
		for( i = 0; i < m_RandomBGAnimations.size(); ++i )
			loaded.insert( m_RandomBGAnimations[i] );

		i = 0;
		while( i < arrayPaths.size() && loaded.find( arrayPaths[i] ) != loaded.end() )
			++i;
		if( i == arrayPaths.size() )
			return "";

		file = arrayPaths[i];
	}

	BGAnimation *ret = new BGAnimation;
	switch( PREFSMAN->m_BackgroundMode )
	{
	case PrefsManager::BGMODE_ANIMATIONS:	ret->LoadFromAniDir( file ); break;
	case PrefsManager::BGMODE_MOVIEVIS:		ret->LoadFromVisualization( file ); break;
	case PrefsManager::BGMODE_RANDOMMOVIES:	ret->LoadFromMovie( file ); break;
	}

	m_BGAnimations[file] = ret;
	m_RandomBGAnimations.push_back( file );
	return file;
}

void Background::LoadFromRandom( float fFirstBeat, float fLastBeat, const TimingData &timing )
{
	// change BG every 4 bars
	for( float f=fFirstBeat; f<fLastBeat; f+=BEATS_PER_MEASURE*4 )
	{
		// Don't fade.  It causes frame rate dip, especially on 
		// slower machines.
		bool bFade = false;
		//bool bFade = PREFSMAN->m_BackgroundMode==PrefsManager::BGMODE_RANDOMMOVIES || 
		//	PREFSMAN->m_BackgroundMode==PrefsManager::BGMODE_MOVIEVIS;
		
		CString sBGName = CreateRandomBGA();
		if( sBGName != "" )
			m_aBGChanges.push_back( BackgroundChange(f,sBGName,1.f,bFade) );
	}

	// change BG every BPM change that is at the beginning of a measure
	for( unsigned i=0; i<timing.m_BPMSegments.size(); i++ )
	{
		const BPMSegment& bpmseg = timing.m_BPMSegments[i];

		if( fmodf(bpmseg.m_fStartBeat, (float)BEATS_PER_MEASURE) != 0 )
			continue;	// skip

		if( bpmseg.m_fStartBeat < fFirstBeat  || bpmseg.m_fStartBeat > fLastBeat )
			continue;	// skip

		CString sBGName = CreateRandomBGA();
		if( sBGName != "" )
			m_aBGChanges.push_back( BackgroundChange(bpmseg.m_fStartBeat,sBGName) );
	}
}

void Background::LoadFromSong( const Song* pSong )
{
	Unload();

	m_pSong = pSong;

	if( PREFSMAN->m_fBGBrightness == 0 )
		return;

	/* Song backgrounds (even just background stills) can get very big; never keep them
	 * in memory. */
	RageTextureID::TexPolicy OldPolicy = TEXTUREMAN->GetDefaultTexturePolicy();
	TEXTUREMAN->SetDefaultTexturePolicy( RageTextureID::TEX_VOLATILE );

	TEXTUREMAN->DisableOddDimensionWarning();

	const float fXZoom = RECT_BACKGROUND.GetWidth() / (float)SCREEN_WIDTH;
	const float fYZoom = RECT_BACKGROUND.GetHeight() / (float)SCREEN_HEIGHT;

	CString sSongBGPath = pSong && pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathToG("Common fallback background");

	// Load the static background that will show before notes start and after notes end
	{
		BGAnimation *pTempBGA = new BGAnimation;
		pTempBGA->LoadFromStaticGraphic( sSongBGPath );
		m_BGAnimations[STATIC_BACKGROUND] = pTempBGA;
	}


	// start off showing the static song background
	m_aBGChanges.push_back( BackgroundChange(-10000,STATIC_BACKGROUND) );


	if( pSong->HasBGChanges() )
	{
		// Load all song-specified backgrounds
		for( unsigned i=0; i<pSong->m_BackgroundChanges.size(); i++ )
		{
			BackgroundChange change = pSong->m_BackgroundChanges[i];
			CString &sBGName = change.m_sBGName;
			
			bool bIsAlreadyLoaded = m_BGAnimations.find(sBGName) != m_BGAnimations.end();

			if( sBGName.CompareNoCase("-random-") && !bIsAlreadyLoaded )
			{
				BGAnimation *pTempBGA = CreateSongBGA( sBGName );
				if( pTempBGA )
					m_BGAnimations[sBGName] = pTempBGA;
				else // the background was not found.  Use a random one instead
				{
					sBGName = CreateRandomBGA();
					if( sBGName == "" )
						sBGName = STATIC_BACKGROUND;
				}
			}
			
			m_aBGChanges.push_back( change );
		}
	}
	else	// pSong doesn't have an animation plan
	{
		LoadFromRandom( pSong->m_fFirstBeat, pSong->m_fLastBeat, pSong->m_Timing );

		// end showing the static song background
		m_aBGChanges.push_back( BackgroundChange(pSong->m_fLastBeat,STATIC_BACKGROUND) );
	}

		
	// sort segments
	SortBackgroundChangesArray( m_aBGChanges );

	// Look for the filename "Random", and replace the segment with LoadFromRandom.
	unsigned i = 0;
	for( i=0; i<m_aBGChanges.size(); i++ )
	{
		BackgroundChange &change = m_aBGChanges[i];
		if( change.m_sBGName.CompareNoCase("-random-") )
			continue;

		const float fStartBeat = change.m_fStartBeat;
		const float fLastBeat = (i+1 < m_aBGChanges.size())? m_aBGChanges[i+1].m_fStartBeat: 99999;

		m_aBGChanges.erase( m_aBGChanges.begin()+i );
		--i;

		LoadFromRandom( fStartBeat, fLastBeat, pSong->m_Timing );
	}

	// At this point, we shouldn't have any BGChanges to "".  "" is an invalid name.
	for( i=0; i<m_aBGChanges.size(); i++ )
		ASSERT( !m_aBGChanges[i].m_sBGName.empty() );


	// Re-sort.
	SortBackgroundChangesArray( m_aBGChanges );

    for( map<CString,BGAnimation*>::iterator iter = m_BGAnimations.begin();
		 iter != m_BGAnimations.end();
		 iter++ )
	{
		iter->second->SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		iter->second->SetZoomX( fXZoom );
		iter->second->SetZoomY( fYZoom );
	}
		
	m_DangerAll.SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
	m_DangerAll.SetZoomX( fXZoom );
	m_DangerAll.SetZoomY( fYZoom );	

	FOREACH_PlayerNumber( p )
	{
		m_DangerPlayer[p].SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_DangerPlayer[p].SetZoomX( fXZoom );
		m_DangerPlayer[p].SetZoomY( fYZoom );	
	
		m_DeadPlayer[p].SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_DeadPlayer[p].SetZoomX( fXZoom );
		m_DeadPlayer[p].SetZoomY( fYZoom );	
	}

	TEXTUREMAN->EnableOddDimensionWarning();

	if( m_pDancingCharacters )
		m_pDancingCharacters->LoadNextSong();

	TEXTUREMAN->SetDefaultTexturePolicy( OldPolicy );
}

int Background::FindBGSegmentForBeat( float fBeat ) const
{
	if( m_aBGChanges.empty() )
		return -1;
	if( fBeat < m_aBGChanges[0].m_fStartBeat )
		return -1;
	
	// assumption: m_aBGChanges are sorted by m_fStartBeat
    int i;
	for( i=m_aBGChanges.size()-1; i>=0; i-- )
	{
		if( fBeat >= m_aBGChanges[i].m_fStartBeat )
			return i;
	}

	return i;
}

/* If the BG segment has changed, move focus to it.  Send Update() calls. */
void Background::UpdateCurBGChange( float fCurrentTime )
{
	ASSERT( fCurrentTime != GameState::MUSIC_SECONDS_INVALID );

	if( m_aBGChanges.size() == 0 )
		return;

	float fBeat, fBPS;
	bool bFreeze;
	m_pSong->m_Timing.GetBeatAndBPSFromElapsedTime( fCurrentTime, fBeat, fBPS, bFreeze );

	/* Calls to Update() should *not* be scaled by music rate; fCurrentTime is. Undo it. */
	const float fRate = GAMESTATE->m_SongOptions.m_fMusicRate;

	// Find the BGSegment we're in
	const int i = FindBGSegmentForBeat( fBeat );

	if( i != -1  &&  i != m_iCurBGChangeIndex )	// we're changing backgrounds
	{
		LOG->Trace( "old bga %d -> new bga %d, %f, %f", m_iCurBGChangeIndex, i, m_aBGChanges[i].m_fStartBeat, fBeat );

		m_iCurBGChangeIndex = i;

		const BackgroundChange& change = m_aBGChanges[i];

		BGAnimation* pOld = m_pCurrentBGA;

		if( change.m_bFadeLast )
			m_pFadingBGA = m_pCurrentBGA;
		else
			m_pFadingBGA = NULL;

		m_pCurrentBGA = m_BGAnimations[ change.m_sBGName ];

		if( pOld )
			pOld->LoseFocus();
		if( m_pCurrentBGA )
			m_pCurrentBGA->GainFocus( change.m_fRate, change.m_bRewindMovie, change.m_bLoop );

		m_fSecsLeftInFade = m_pFadingBGA!=NULL ? FADE_SECONDS : 0;

		/* How much time of this BGA have we skipped?  (This happens with SetSeconds.) */
		const float fStartSecond = m_pSong->m_Timing.GetElapsedTimeFromBeat( change.m_fStartBeat );

		/* This is affected by the music rate. */
		float fDeltaTime = fCurrentTime - fStartSecond;
		fDeltaTime /= fRate;
		if( m_pCurrentBGA )
			m_pCurrentBGA->Update( max( fDeltaTime, 0 ) );
	}
	else	// we're not changing backgrounds
	{
		/* This is affected by the music rate. */
		float fDeltaTime = fCurrentTime - m_fLastMusicSeconds;
		fDeltaTime /= fRate;
		if( m_pCurrentBGA )
			m_pCurrentBGA->Update( max( fDeltaTime, 0 ) );
	}

	float fDeltaTime = fCurrentTime - m_fLastMusicSeconds;
	fDeltaTime /= fRate;
	if( m_pFadingBGA )
		m_pFadingBGA->Update( max( fCurrentTime - m_fLastMusicSeconds, 0 ) );
	m_fLastMusicSeconds = fCurrentTime;
}

void Background::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( IsDangerAllVisible() )
	{
		m_DangerAll.Update( fDeltaTime );
	}

	FOREACH_PlayerNumber( p )
	{
		if( IsDangerPlayerVisible((PlayerNumber)p) )
			m_DangerPlayer[p].Update( fDeltaTime );
			
		if( IsDeadPlayerVisible((PlayerNumber)p) )
			m_DeadPlayer[p].Update( fDeltaTime );
	}

	/* Always update the current background, even when m_DangerAll is being displayed.
	 * Otherwise, we'll stop updating movies during danger (which may stop them from
	 * playing), and we won't start clips at the right time, which will throw backgrounds
	 * off sync. */
	UpdateCurBGChange( GAMESTATE->m_fMusicSeconds );
	
	if( m_pFadingBGA )
	{
		m_pFadingBGA->Update( fDeltaTime );
		m_fSecsLeftInFade -= fDeltaTime;
		float fPercentOpaque = m_fSecsLeftInFade / FADE_SECONDS;
		m_pFadingBGA->SetDiffuse( RageColor(1,1,1,fPercentOpaque) );
		if( fPercentOpaque <= 0 )
		{
			/* Reset its diffuse color, in case we reuse it. */
			m_pFadingBGA->SetDiffuse( RageColor(1,1,1,1) );
			m_pFadingBGA = NULL;
		}
	}

	if( m_pDancingCharacters )
		m_pDancingCharacters->Update( fDeltaTime );
}

void Background::DrawPrimitives()
{
	if( PREFSMAN->m_fBGBrightness == 0 )
		return;

	if( IsDangerAllVisible() )
	{
		// Since this only shows when DANGER is visible, it will flash red on it's own accord :)
		if( m_pDancingCharacters )
			m_pDancingCharacters->m_bDrawDangerLight = true;
		m_DangerAll.Draw();
	}
	
	if( !IsDangerAllVisible() || !DANGER_ALL_IS_OPAQUE ) 
	{	
		if( m_pDancingCharacters )
			m_pDancingCharacters->m_bDrawDangerLight = false;
		if( m_pCurrentBGA )
			m_pCurrentBGA->Draw();
		if( m_pFadingBGA )
			m_pFadingBGA->Draw();

		FOREACH_PlayerNumber( p )
		{
			if( IsDangerPlayerVisible((PlayerNumber)p) )
				m_DangerPlayer[p].Draw();
			if( IsDeadPlayerVisible((PlayerNumber)p) )
				m_DeadPlayer[p].Draw();
		}
	}

	if( m_pDancingCharacters )
		m_pDancingCharacters->Draw();

	ActorFrame::DrawPrimitives();
}

bool Background::IsDangerAllVisible()
{
	if( GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_OFF )
		return false;
	if( !PREFSMAN->m_bShowDanger )
		return false;

	/* Don't show it if everyone is already failing: it's already too late and it's
	 * annoying for it to show for the entire duration of a song. */
	if( g_CurStageStats.AllFailedEarlier() )
		return false;

	if( !GAMESTATE->AllAreInDangerOrWorse() )
		return false;

	if( BLINK_DANGER_ALL )
		return (RageTimer::GetTimeSinceStart() - (int)RageTimer::GetTimeSinceStart()) < 0.5f;
	else
		return true;
}

bool Background::IsDangerPlayerVisible( PlayerNumber pn )
{
	if( GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_OFF )
		return false;
	if( !PREFSMAN->m_bShowDanger )
		return false;
	return GAMESTATE->m_HealthState[pn] == GameState::DANGER;
}

bool Background::IsDeadPlayerVisible( PlayerNumber pn )
{
	if( GAMESTATE->m_SongOptions.m_FailType == SongOptions::FAIL_OFF )
		return false;
	return GAMESTATE->m_HealthState[pn] == GameState::DEAD;
}


BrightnessOverlay::BrightnessOverlay()
{
	float fQuadWidth = (RIGHT_EDGE-LEFT_EDGE)/2;
	fQuadWidth -= g_fBackgroundCenterWidth/2;

	m_quadBGBrightness[0].StretchTo( RectF(LEFT_EDGE,TOP_EDGE,LEFT_EDGE+fQuadWidth,BOTTOM_EDGE) );
	m_quadBGBrightnessFade.StretchTo( RectF(LEFT_EDGE+fQuadWidth,TOP_EDGE,RIGHT_EDGE-fQuadWidth,BOTTOM_EDGE) );
	m_quadBGBrightness[1].StretchTo( RectF(RIGHT_EDGE-fQuadWidth,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE) );

	this->AddChild( &m_quadBGBrightness[0] );
	this->AddChild( &m_quadBGBrightness[1] );
	this->AddChild( &m_quadBGBrightnessFade );

	SetActualBrightness();
}

void BrightnessOverlay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );
	/* If we're actually playing, then we're past fades, etc; update the background
	 * brightness to follow Cover. */
	if( GAMESTATE->m_bPastHereWeGo )
		SetActualBrightness();
}

void BrightnessOverlay::SetActualBrightness()
{
	float fLeftBrightness = 1-GAMESTATE->m_PlayerOptions[PLAYER_1].m_fCover;
	float fRightBrightness = 1-GAMESTATE->m_PlayerOptions[PLAYER_2].m_fCover;

	fLeftBrightness *= PREFSMAN->m_fBGBrightness;
	fRightBrightness *= PREFSMAN->m_fBGBrightness;

	if( !GAMESTATE->IsHumanPlayer(PLAYER_1) )
		fLeftBrightness = fRightBrightness;
	if( !GAMESTATE->IsHumanPlayer(PLAYER_2) )
		fRightBrightness = fLeftBrightness;

	RageColor LeftColor( 0,0,0,1-fLeftBrightness );
	RageColor RightColor( 0,0,0,1-fRightBrightness );

	m_quadBGBrightness[PLAYER_1].SetDiffuse( LeftColor );
	m_quadBGBrightness[PLAYER_2].SetDiffuse( RightColor );
	m_quadBGBrightnessFade.SetDiffuseLeftEdge( LeftColor );
	m_quadBGBrightnessFade.SetDiffuseRightEdge( RightColor );
}

void BrightnessOverlay::Set( float fBrightness )
{
	FOREACH_PlayerNumber(pn)
		m_quadBGBrightness[pn].SetDiffuse( RageColor(0,0,0,1-fBrightness) );
	m_quadBGBrightnessFade.SetDiffuse( RageColor(0,0,0,1-fBrightness) );
}

void BrightnessOverlay::FadeToActualBrightness()
{
	this->RunCommandOnChildren( BRIGHTNESS_FADE_COMMAND );
	SetActualBrightness();
}

/*
 * (c) 2001-2004 Chris Danford, Ben Nordstrom
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
