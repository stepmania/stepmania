#include "global.h"
#include "Background.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageException.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageTextureManager.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "NoteTypes.h"
#include "Steps.h"
#include "DancingCharacters.h"
#include "BeginnerHelper.h"
#include "StatsManager.h"
#include "ScreenDimensions.h"
#include "ThemeMetric.h"
#include "PlayerState.h"
#include "Command.h"
#include "ActorUtil.h"
#include <set>
#include <float.h>

const float FADE_SECONDS = 1.0f;

ThemeMetric<float> LEFT_EDGE						("Background","LeftEdge");
ThemeMetric<float> TOP_EDGE							("Background","TopEdge");
ThemeMetric<float> RIGHT_EDGE						("Background","RightEdge");
ThemeMetric<float> BOTTOM_EDGE						("Background","BottomEdge");
#define RECT_BACKGROUND RectF						(LEFT_EDGE,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE)
ThemeMetric<bool> BLINK_DANGER_ALL					("Background","BlinkDangerAll");
ThemeMetric<bool> DANGER_ALL_IS_OPAQUE				("Background","DangerAllIsOpaque");
ThemeMetric<apActorCommands> BRIGHTNESS_FADE_COMMAND	("Background","BrightnessFadeCommand");
ThemeMetric<float> CLAMP_OUTPUT_PERCENT				("Background","ClampOutputPercent");

static float g_fBackgroundCenterWidth = 40;
const CString STATIC_BACKGROUND = "static background";

static RageColor GetBrightnessColor( float fBrightnessPercent )
{
	RageColor cBrightness = RageColor( 0,0,0,1-fBrightnessPercent );
	RageColor cClamp = RageColor( 0.5f,0.5f,0.5f,CLAMP_OUTPUT_PERCENT );

	// blend the two colors above as if cBrightness is drawn, then cClamp drawn on top
	cBrightness.a *= (1-cClamp.a);	// premultiply alpha

	RageColor ret;
	ret.a = cBrightness.a + cClamp.a;
	ret.r = (cBrightness.r * cBrightness.a + cClamp.r * cClamp.a) / ret.a;
	ret.g = (cBrightness.g * cBrightness.a + cClamp.g * cClamp.a) / ret.a;
	ret.b = (cBrightness.b * cBrightness.a + cClamp.b * cClamp.a) / ret.a;
	return ret;
}

Background::Background()
{
	m_iCurBGChangeIndex = -1;
	m_pCurrentBGA = NULL;
	m_pFadingBGA = NULL;
	m_fSecsLeftInFade = 0;
	m_pDancingCharacters = NULL;
	m_bInitted = false;
}

void Background::Init()
{
	if( m_bInitted )
		return;
	m_bInitted = true;

	m_DangerAll.LoadFromAniDir( THEME->GetPathB("ScreenGameplay","danger all") );
	FOREACH_PlayerNumber( p )
		m_DangerPlayer[p].LoadFromAniDir( THEME->GetPathB("ScreenGameplay",ssprintf("danger p%d",p+1)) );
	FOREACH_PlayerNumber( p )
		m_DeadPlayer[p].LoadFromAniDir( THEME->GetPathB("ScreenGameplay",ssprintf("dead p%d",p+1)) );

	bool bOneOrMoreChars = false;
	bool bShowingBeginnerHelper = false;
	FOREACH_HumanPlayer( p )
	{
		bOneOrMoreChars = true;
		// Disable dancing characters if BH will be showing.
		if( PREFSMAN->m_bShowBeginnerHelper && BeginnerHelper::CanUse() && 
			GAMESTATE->m_pCurSteps[p] && GAMESTATE->m_pCurSteps[p]->GetDifficulty() == DIFFICULTY_BEGINNER )
			bShowingBeginnerHelper = true;
	}

	if( bOneOrMoreChars && !bShowingBeginnerHelper )
		m_pDancingCharacters = new DancingCharacters;

	RageColor c = GetBrightnessColor(0);

	m_quadBorderLeft.StretchTo( RectF(SCREEN_LEFT,SCREEN_TOP,LEFT_EDGE,SCREEN_BOTTOM) );
	m_quadBorderLeft.SetDiffuse( c );
	m_quadBorderTop.StretchTo( RectF(LEFT_EDGE,SCREEN_TOP,RIGHT_EDGE,TOP_EDGE) );
	m_quadBorderTop.SetDiffuse( c );
	m_quadBorderRight.StretchTo( RectF(RIGHT_EDGE,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_quadBorderRight.SetDiffuse( c );
	m_quadBorderBottom.StretchTo( RectF(LEFT_EDGE,BOTTOM_EDGE,RIGHT_EDGE,SCREEN_BOTTOM) );
	m_quadBorderBottom.SetDiffuse( c );

	this->AddChild( &m_quadBorderLeft );
	this->AddChild( &m_quadBorderTop );
	this->AddChild( &m_quadBorderRight );
	this->AddChild( &m_quadBorderBottom );

	this->AddChild( &m_Brightness );
}

Background::~Background()
{
	Unload();
	delete m_pDancingCharacters;
}

void Background::Unload()
{
    for( map<CString,Actor*>::iterator iter = m_BGAnimations.begin();
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

Actor *MakeVisualization( const CString &sVisPath )
{
	ActorFrame *pFrame = new ActorFrame;
	pFrame->DeleteChildrenWhenDone();

	const Song* pSong = GAMESTATE->m_pCurSong;
	CString sSongBGPath = 
		(pSong && pSong->HasBackground()) ? pSong->GetBackgroundPath() : THEME->GetPathG("Common","fallback background");

	Sprite* pSprite = new Sprite;
	pSprite->LoadBG( sSongBGPath );
	pSprite->StretchTo( FullScreenRectF );
	pFrame->AddChild( pSprite );

	pSprite = new Sprite;
	pSprite->LoadBG( sVisPath );
	pSprite->StretchTo( FullScreenRectF );
	pSprite->SetBlendMode( BLEND_ADD );
	pFrame->AddChild( pSprite );

	return pFrame;
}

Actor *MakeMovie( const CString &sMoviePath )
{
	Sprite *pSprite = new Sprite;
	pSprite->LoadBG( sMoviePath );
	pSprite->StretchTo( FullScreenRectF );
	pSprite->EnableAnimation( false );
	return pSprite;
}

Actor *Background::CreateSongBGA( CString sBGName ) const
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
		/* If default.xml exists, use the regular generic actor load.  However,
		 * if it's an old BGAnimation.ini, load it ourself so we can set bGeneric
		 * to false. */
		if( DoesFileExist(asFiles[0] + "/default.xml") )
			return ActorUtil::MakeActor( asFiles[0] );

		pTempBGA = new BGAnimation;
		pTempBGA->LoadFromAniDir( asFiles[0], false );
		return pTempBGA;
	}
	// Look for BG movies or static graphics in the song dir
	GetDirListing( m_pSong->GetSongDir()+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
	{
		const CString sExt = GetExtension( asFiles[0]) ;
		if( sExt.CompareNoCase("avi")==0 ||
			sExt.CompareNoCase("mpg")==0 ||
			sExt.CompareNoCase("mpeg")==0 )
		{
			return MakeMovie( asFiles[0] );
		}
		else
		{
			Sprite *pSprite = new Sprite;
			pSprite->LoadBG( asFiles[0] );
			pSprite->StretchTo( FullScreenRectF );
			return pSprite;
		}
	}
	// Look for movies in the RandomMovies dir
	GetDirListing( RANDOMMOVIES_DIR+sBGName, asFiles, false, true );
	if( !asFiles.empty() )
		return MakeMovie( asFiles[0] );

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
		return MakeVisualization( asFiles[0] );

	// There is no background by this name.  
	return NULL;
}

CString Background::CreateRandomBGA( CString sPreferredSubDir )
{
	if( sPreferredSubDir.Right(1) != "/" )
		sPreferredSubDir += '/';

	if( PREFSMAN->m_BackgroundMode == PrefsManager::BGMODE_OFF )
		return "";

	/* If we already have enough random BGAs loaded, use them round-robin. */
	if( (int)m_RandomBGAnimations.size() >= PREFSMAN->m_iNumBackgrounds )
	{
		/* XXX: every time we fully loop, shuffle, so we don't play the same sequence
		 * over and over; and nudge the shuffle so the next one won't be a repeat */
		const CString first = m_RandomBGAnimations.front();
		m_RandomBGAnimations.push_back( m_RandomBGAnimations.front() );
		m_RandomBGAnimations.pop_front();
		return first;
	}

	CStringArray arrayPaths;
	for( int i=0; i<2; i++ )
	{
		switch( PREFSMAN->m_BackgroundMode )
		{
		default:
			FAIL_M( ssprintf("Invalid BackgroundMode: %i", (PrefsManager::BackgroundMode)PREFSMAN->m_BackgroundMode) );
			break;

		case PrefsManager::BGMODE_ANIMATIONS:
			GetDirListing( BG_ANIMS_DIR + sPreferredSubDir + "*", arrayPaths, true, true );
			break;

		case PrefsManager::BGMODE_MOVIEVIS:
			GetDirListing( VISUALIZATIONS_DIR + sPreferredSubDir + "*.avi", arrayPaths, false, true );
			GetDirListing( VISUALIZATIONS_DIR + sPreferredSubDir + "*.mpg", arrayPaths, false, true );
			GetDirListing( VISUALIZATIONS_DIR + sPreferredSubDir + "*.mpeg", arrayPaths, false, true );
			break;

		case PrefsManager::BGMODE_RANDOMMOVIES:
			GetDirListing( RANDOMMOVIES_DIR + sPreferredSubDir + "*.avi", arrayPaths, false, true );
			GetDirListing( RANDOMMOVIES_DIR + sPreferredSubDir + "*.mpg", arrayPaths, false, true );
			GetDirListing( RANDOMMOVIES_DIR + sPreferredSubDir + "*.mpeg", arrayPaths, false, true );
			break;
		}

		// strip out "cvs"
		for( int j=arrayPaths.size()-1; j>=0; j-- )
			if( Basename(arrayPaths[j]).CompareNoCase("cvs")==0 )
				arrayPaths.erase( arrayPaths.begin()+j, arrayPaths.begin()+j+1 );

		if( !arrayPaths.empty() )	// found one
			break;

		// now search without a subdir
		sPreferredSubDir = "";
	}

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

	Actor *ret;
	switch( PREFSMAN->m_BackgroundMode )
	{
	case PrefsManager::BGMODE_ANIMATIONS:
	{
		BGAnimation *p = new BGAnimation;
		p->LoadFromAniDir( file ); 
		ret = p;
		break;
	}
	case PrefsManager::BGMODE_MOVIEVIS:		ret = MakeVisualization( file ); break;
	case PrefsManager::BGMODE_RANDOMMOVIES: ret = MakeMovie( file ); break;
	default: FAIL_M( ssprintf("%i", (int) PREFSMAN->m_BackgroundMode) );
	}

	m_BGAnimations[file] = ret;
	m_RandomBGAnimations.push_back( file );
	return file;
}

void Background::LoadFromRandom( float fFirstBeat, float fLastBeat, const TimingData &timing, CString sPreferredSubDir )
{
	// change BG every 4 bars
	for( float f=fFirstBeat; f<fLastBeat; f+=BEATS_PER_MEASURE*4 )
	{
		// Don't fade.  It causes frame rate dip, especially on 
		// slower machines.
		bool bFade = false;
		//bool bFade = PREFSMAN->m_BackgroundMode==PrefsManager::BGMODE_RANDOMMOVIES || 
		//	PREFSMAN->m_BackgroundMode==PrefsManager::BGMODE_MOVIEVIS;
		
		CString sBGName = CreateRandomBGA( sPreferredSubDir );
		if( sBGName != "" )
			m_aBGChanges.push_back( BackgroundChange(f,sBGName,1.f,bFade) );
	}

	// change BG every BPM change that is at the beginning of a measure
	int iStartIndex = BeatToNoteRow(fFirstBeat);
	int iEndIndex = BeatToNoteRow(fLastBeat);
	for( unsigned i=0; i<timing.m_BPMSegments.size(); i++ )
	{
		const BPMSegment& bpmseg = timing.m_BPMSegments[i];

		if( bpmseg.m_iStartIndex % BeatToNoteRow((float) BEATS_PER_MEASURE) != 0 )
			continue;	// skip

		if( bpmseg.m_iStartIndex < iStartIndex  || bpmseg.m_iStartIndex > iEndIndex )
			continue;	// skip

		CString sBGName = CreateRandomBGA( sPreferredSubDir );
		if( sBGName != "" )
			m_aBGChanges.push_back( BackgroundChange(NoteRowToBeat(bpmseg.m_iStartIndex),sBGName) );
	}
}

void Background::LoadFromSong( const Song* pSong )
{
	Init();

	Unload();

	m_pSong = pSong;

	if( PREFSMAN->m_fBGBrightness == 0.0f )
		return;

	/* Song backgrounds (even just background stills) can get very big; never keep them
	 * in memory. */
	RageTextureID::TexPolicy OldPolicy = TEXTUREMAN->GetDefaultTexturePolicy();
	TEXTUREMAN->SetDefaultTexturePolicy( RageTextureID::TEX_VOLATILE );

	TEXTUREMAN->DisableOddDimensionWarning();

	const float fXZoom = RECT_BACKGROUND.GetWidth() / (float)SCREEN_WIDTH;
	const float fYZoom = RECT_BACKGROUND.GetHeight() / (float)SCREEN_HEIGHT;



	if( pSong->HasBGChanges() )
	{
		// Load all song-specified backgrounds
		for( unsigned i=0; i<pSong->m_BackgroundChanges.size(); i++ )
		{
			BackgroundChange change = pSong->m_BackgroundChanges[i];
			CString &sBGName = change.m_sBGName;
			
			bool bIsAlreadyLoaded = m_BGAnimations.find(sBGName) != m_BGAnimations.end();

			if( sBGName.CompareNoCase("-random-")!=0 && !bIsAlreadyLoaded )
			{
				Actor *pTempBGA = CreateSongBGA( sBGName );
				if( pTempBGA )
				{
					m_BGAnimations[sBGName] = pTempBGA;
				}
				else // the background was not found.  Use a random one instead
				{
					sBGName = CreateRandomBGA( pSong->m_sGroupName );
					if( sBGName == "" )
						sBGName = STATIC_BACKGROUND;
				}
			}
			
			m_aBGChanges.push_back( change );
		}
	}
	else	// pSong doesn't have an animation plan
	{
		LoadFromRandom( pSong->m_fFirstBeat, pSong->m_fLastBeat, pSong->m_Timing, pSong->m_sGroupName );

		// end showing the static song background
		m_aBGChanges.push_back( BackgroundChange(pSong->m_fLastBeat,STATIC_BACKGROUND) );
	}

		
	// sort segments
	SortBackgroundChangesArray( m_aBGChanges );

	/* If the first BGAnimation isn't negative, add a lead-in image showing the song
	 * background. */
	if( m_aBGChanges.empty() || m_aBGChanges.front().m_fStartBeat >= 0 )
		m_aBGChanges.insert( m_aBGChanges.begin(), BackgroundChange(-10000,STATIC_BACKGROUND) );

	// If any BGChanges use the background image, load it.
	bool bStaticBackgroundUsed = false;
	for( unsigned i=0; i<m_aBGChanges.size(); i++ )
		if( m_aBGChanges[i].m_sBGName == STATIC_BACKGROUND )
			bStaticBackgroundUsed = true;
	if( bStaticBackgroundUsed )
	{
		CString sSongBGPath = 
			pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathG("Common","fallback background");
		Sprite* pSprite = new Sprite;
		pSprite->LoadBG( sSongBGPath );
		pSprite->StretchTo( FullScreenRectF );
		m_BGAnimations[STATIC_BACKGROUND] = pSprite;
	}


	// Look for the filename "Random", and replace the segment with LoadFromRandom.
	for( unsigned i=0; i<m_aBGChanges.size(); i++ )
	{
		BackgroundChange &change = m_aBGChanges[i];
		if( change.m_sBGName.CompareNoCase("-random-") )
			continue;

		const float fStartBeat = change.m_fStartBeat;
		const float fLastBeat = (i+1 < m_aBGChanges.size())? m_aBGChanges[i+1].m_fStartBeat: FLT_MAX;

		m_aBGChanges.erase( m_aBGChanges.begin()+i );
		--i;

		LoadFromRandom( fStartBeat, fLastBeat, pSong->m_Timing, pSong->m_sGroupName );
	}

	// At this point, we shouldn't have any BGChanges to "".  "" is an invalid name.
	for( unsigned i=0; i<m_aBGChanges.size(); i++ )
		ASSERT( !m_aBGChanges[i].m_sBGName.empty() );


	// Re-sort.
	SortBackgroundChangesArray( m_aBGChanges );

	m_DangerAll.SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
	m_DangerAll.SetZoomX( fXZoom );
	m_DangerAll.SetZoomY( fYZoom );	

	FOREACH_PlayerNumber( p )
	{
		m_DangerPlayer[p].SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_DangerPlayer[p].SetZoomX( fXZoom );
		m_DangerPlayer[p].SetZoomY( fYZoom );
		m_DangerPlayer[p].FinishTweening();
		m_DangerPlayer[p].PlayCommand( "On" );
	
		m_DeadPlayer[p].SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_DeadPlayer[p].SetZoomX( fXZoom );
		m_DeadPlayer[p].SetZoomY( fYZoom );	
		m_DeadPlayer[p].FinishTweening();
		m_DeadPlayer[p].PlayCommand( "On" );
	}

	TEXTUREMAN->EnableOddDimensionWarning();

	if( m_pDancingCharacters )
		m_pDancingCharacters->LoadNextSong();

	TEXTUREMAN->SetDefaultTexturePolicy( OldPolicy );

	/* Song backgrounds always sync from the music by default, unlike other actors
	 * which sync from the regular clock by default.  If you don't want this, set
	 * the clock back with "effectclock,timer" in your OnCommand.  Note that at this
	 * point, we havn't run the OnCommand yet, so it'll override correctly. */
	map<CString,Actor*>::iterator it;
	for( it = m_BGAnimations.begin(); it != m_BGAnimations.end(); ++it )
	{
		Actor *pBGA = it->second;

		/* Be sure that we run this command recursively on all children; the tree
		 * may look something like "BGAnimation, BGAnimationLayer, Sprite" or it
		 * may be deeper, like "BGAnimation, BGAnimationLayer, BGAnimation,
		 * BGAnimationLayer, Sprite". */
		ActorCommands acmds( "effectclock,music" );
		pBGA->RunCommands( acmds );
	}
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

		Actor *pOld = m_pCurrentBGA;

		if( change.m_bFadeLast )
			m_pFadingBGA = m_pCurrentBGA;
		else
			m_pFadingBGA = NULL;

		m_pCurrentBGA = m_BGAnimations[ change.m_sBGName ];

		if( pOld )
		{
			pOld->LoseFocus();
			pOld->PlayCommand( "LoseFocus" );
		}
		if( m_pCurrentBGA )
		{
			m_pCurrentBGA->GainFocus( change.m_fRate, change.m_bRewindMovie, change.m_bLoop );
			m_pCurrentBGA->PlayCommand( "On" );
			m_pCurrentBGA->PlayCommand( "GainFocus" );
		}

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
		if( GAMESTATE->IsPlayerInDanger(p) )
			m_DangerPlayer[p].Update( fDeltaTime );
			
		if( GAMESTATE->IsPlayerDead(p) )
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
	if( PREFSMAN->m_fBGBrightness == 0.0f )
		return;

	if( IsDangerAllVisible() )
	{
		// Since this only shows when DANGER is visible, it will flash red on it's own accord :)
		if( m_pDancingCharacters )
			m_pDancingCharacters->m_bDrawDangerLight = true;
		m_DangerAll.Draw();
	}
	
	if( !IsDangerAllVisible() || !(bool)DANGER_ALL_IS_OPAQUE ) 
	{	
		if( m_pDancingCharacters )
			m_pDancingCharacters->m_bDrawDangerLight = false;
		if( m_pCurrentBGA )
			m_pCurrentBGA->Draw();
		if( m_pFadingBGA )
			m_pFadingBGA->Draw();

		FOREACH_PlayerNumber( p )
		{
			if( GAMESTATE->IsPlayerInDanger(p) )
				m_DangerPlayer[p].Draw();
			if( GAMESTATE->IsPlayerDead(p) )
				m_DeadPlayer[p].Draw();
		}
	}

	if( m_pDancingCharacters )
		m_pDancingCharacters->Draw();

	ActorFrame::DrawPrimitives();
}

bool Background::IsDangerAllVisible()
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->GetPlayerFailType(p) == SongOptions::FAIL_OFF )
			return false;
	if( !PREFSMAN->m_bShowDanger )
		return false;

	/* Don't show it if everyone is already failing: it's already too late and it's
	 * annoying for it to show for the entire duration of a song. */
	if( STATSMAN->m_CurStageStats.AllFailedEarlier() )
		return false;

	if( !GAMESTATE->AllAreInDangerOrWorse() )
		return false;

	if( BLINK_DANGER_ALL )
		return (RageTimer::GetTimeSinceStartFast() - (int)RageTimer::GetTimeSinceStartFast()) < 0.5f;
	else
		return true;
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
	float fLeftBrightness = 1-GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.m_fCover;
	float fRightBrightness = 1-GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions.m_fCover;

	float fBaseBGBrightness = PREFSMAN->m_fBGBrightness;

	// HACK: Always show training in full brightness
	if( GAMESTATE->m_pCurSong && GAMESTATE->m_pCurSong->IsTutorial() )
		fBaseBGBrightness = 1.0f;
	
	fLeftBrightness *= fBaseBGBrightness;
	fRightBrightness *= fBaseBGBrightness;

	if( !GAMESTATE->IsHumanPlayer(PLAYER_1) )
		fLeftBrightness = fRightBrightness;
	if( !GAMESTATE->IsHumanPlayer(PLAYER_2) )
		fRightBrightness = fLeftBrightness;

	RageColor LeftColor = GetBrightnessColor(fLeftBrightness);
	RageColor RightColor = GetBrightnessColor(fRightBrightness);

	m_quadBGBrightness[PLAYER_1].SetDiffuse( LeftColor );
	m_quadBGBrightness[PLAYER_2].SetDiffuse( RightColor );
	m_quadBGBrightnessFade.SetDiffuseLeftEdge( LeftColor );
	m_quadBGBrightnessFade.SetDiffuseRightEdge( RightColor );
}

void BrightnessOverlay::Set( float fBrightness )
{
	RageColor c = GetBrightnessColor(fBrightness);

	FOREACH_PlayerNumber(pn)
		m_quadBGBrightness[pn].SetDiffuse( c );
	m_quadBGBrightnessFade.SetDiffuse( c );
}

void BrightnessOverlay::FadeToActualBrightness()
{
	this->RunCommandsOnChildren( BRIGHTNESS_FADE_COMMAND );
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
