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
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "BackgroundUtil.h"
#include "song.h"
#include "AutoActor.h"

static ThemeMetric<float> LEFT_EDGE					("Background","LeftEdge");
static ThemeMetric<float> TOP_EDGE						("Background","TopEdge");
static ThemeMetric<float> RIGHT_EDGE					("Background","RightEdge");
static ThemeMetric<float> BOTTOM_EDGE					("Background","BottomEdge");
#define RECT_BACKGROUND RectF					(LEFT_EDGE,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE)
static ThemeMetric<bool> BLINK_DANGER_ALL				("Background","BlinkDangerAll");
static ThemeMetric<bool> DANGER_ALL_IS_OPAQUE			("Background","DangerAllIsOpaque");
static ThemeMetric<apActorCommands> BRIGHTNESS_FADE_COMMAND	("Background","BrightnessFadeCommand");
static ThemeMetric<float> CLAMP_OUTPUT_PERCENT			("Background","ClampOutputPercent");

// Width of the region separating the left and right brightness areas:
static float g_fBackgroundCenterWidth = 40;



class BrightnessOverlay: public ActorFrame
{
public:
	BrightnessOverlay();
	void Update( float fDeltaTime );

	void FadeToActualBrightness();
	void SetActualBrightness();
	void Set( float fBrightness );

private:
	Quad m_quadBGBrightness[NUM_PLAYERS];
	Quad m_quadBGBrightnessFade;
};

struct BackgroundTransition
{
	apActorCommands cmdLeaves;
	apActorCommands cmdRoot;
};


class BackgroundImpl : public ActorFrame
{
public:
	BackgroundImpl();
	~BackgroundImpl();
	void Init();

	virtual void LoadFromSong( const Song *pSong );
	virtual void Unload();
	virtual void ProcessMessages( float fDeltaTime );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void FadeToActualBrightness() { m_Brightness.FadeToActualBrightness(); }
	void SetBrightness( float fBrightness ) { m_Brightness.Set(fBrightness); } /* overrides pref and Cover */
	
	DancingCharacters* GetDancingCharacters() { return m_pDancingCharacters; };

	void GetLoadedBackgroundChanges( vector<BackgroundChange> *pBackgroundChangesOut[NUM_BackgroundLayer] );

protected:
	bool m_bInitted;
	DancingCharacters*	m_pDancingCharacters;
	const Song *m_pSong;
	map<RString,BackgroundTransition> m_mapNameToTransition;
	deque<BackgroundDef> m_RandomBGAnimations;	// random background to choose from.  These may or may not be loaded into m_BGAnimations.
	
	void LoadFromRandom( float fFirstBeat, float fEndBeat, const BackgroundChange &change );
	bool IsDangerAllVisible();
	
	class Layer
	{
	public:
		Layer();
		void Unload();

		// return true if created and added to m_BGAnimations
		bool CreateBackground( const Song *pSong, const BackgroundDef &bd );
		// return def of the background that was created and added to m_BGAnimations. calls CreateBackground
		BackgroundDef CreateRandomBGA( const Song *pSong, const RString &sEffect, deque<BackgroundDef> &RandomBGAnimations );

		int FindBGSegmentForBeat( float fBeat ) const;
		void UpdateCurBGChange( const Song *pSong, float fLastMusicSeconds, float fCurrentTime, const map<RString,BackgroundTransition> &mapNameToTransition );
		void ProcessMessages( float fDeltaTime );

		map<BackgroundDef,Actor*> m_BGAnimations;
		vector<BackgroundChange> m_aBGChanges;
		int				m_iCurBGChangeIndex;
		Actor *m_pCurrentBGA;
		Actor *m_pFadingBGA;
	};
	Layer m_Layer[NUM_BackgroundLayer];

	float m_fLastMusicSeconds;

	AutoActor		m_DangerPlayer[NUM_PLAYERS];
	AutoActor		m_DangerAll;

	AutoActor		m_DeadPlayer[NUM_PLAYERS];
	
	// cover up the edge of animations that might hang outside of the background rectangle
	Quad m_quadBorderLeft, m_quadBorderTop, m_quadBorderRight, m_quadBorderBottom;

	BrightnessOverlay m_Brightness;

	BackgroundDef m_StaticBackgroundDef;
};



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

BackgroundImpl::BackgroundImpl()
{
	m_bInitted = false;
	m_pDancingCharacters = NULL;
	m_pSong = NULL;
}

BackgroundImpl::Layer::Layer()
{
	m_iCurBGChangeIndex = -1;
	m_pCurrentBGA = NULL;
	m_pFadingBGA = NULL;
}

void BackgroundImpl::Init()
{
	if( m_bInitted )
		return;
	m_bInitted = true;
	m_StaticBackgroundDef = BackgroundDef();
	
	// load transitions
	{
		ASSERT( m_mapNameToTransition.empty() );
		vector<RString> vsPaths, vsNames;
		BackgroundUtil::GetBackgroundTransitions( "", vsPaths, vsNames );
		for( unsigned i=0; i<vsPaths.size(); i++ )
		{
			const RString &sPath = vsPaths[i];
			const RString &sName = vsNames[i];

			XNode xml;
			XmlFileUtil::LoadFromFileShowErrors(xml, sPath);
			ASSERT( xml.m_sName == "BackgroundTransition" );
			BackgroundTransition &bgt = m_mapNameToTransition[sName];

			RString sCmdLeaves;
			bool bSuccess = xml.GetAttrValue( "LeavesCommand", sCmdLeaves );
			ASSERT( bSuccess );
			bgt.cmdLeaves = apActorCommands( new ActorCommands(sCmdLeaves) );

			RString sCmdRoot;
			bSuccess = xml.GetAttrValue( "RootCommand", sCmdRoot );
			ASSERT( bSuccess );
			bgt.cmdRoot = apActorCommands( new ActorCommands(sCmdRoot) );
		}
	}

	m_DangerAll.Load( THEME->GetPathB("ScreenGameplay","danger all") );
	FOREACH_PlayerNumber( p )
		m_DangerPlayer[p].Load( THEME->GetPathB("ScreenGameplay",ssprintf("danger p%d",p+1)) );
	FOREACH_PlayerNumber( p )
		m_DeadPlayer[p].Load( THEME->GetPathB("ScreenGameplay",ssprintf("dead p%d",p+1)) );

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

BackgroundImpl::~BackgroundImpl()
{
	Unload();
	delete m_pDancingCharacters;
}

void BackgroundImpl::Unload()
{
	FOREACH_BackgroundLayer( i )
		m_Layer[i].Unload();
	m_pSong = NULL;
	m_fLastMusicSeconds	= -9999;
	m_RandomBGAnimations.clear();
}

void BackgroundImpl::Layer::Unload()
{
    FOREACHM( BackgroundDef, Actor*, m_BGAnimations, iter )
		delete iter->second;
	m_BGAnimations.clear();
	m_aBGChanges.clear();

	m_pCurrentBGA = NULL;
	m_pFadingBGA = NULL;
	m_iCurBGChangeIndex = -1;
}

Actor *MakeVisualization( const RString &sVisPath )
{
	ActorFrame *pFrame = new ActorFrame;
	pFrame->DeleteChildrenWhenDone();

	const Song* pSong = GAMESTATE->m_pCurSong;
	RString sSongBGPath = 
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

Actor *MakeMovie( const RString &sMoviePath )
{
	Sprite *pSprite = new Sprite;
	pSprite->LoadBG( sMoviePath );
	pSprite->StretchTo( FullScreenRectF );
	pSprite->EnableAnimation( false );
	return pSprite;
}

bool BackgroundImpl::Layer::CreateBackground( const Song *pSong, const BackgroundDef &bd )
{
	ASSERT( m_BGAnimations.find(bd) == m_BGAnimations.end() );

	// Resolve the background names
	vector<RString> vsToResolve;
	vsToResolve.push_back( bd.m_sFile1 );
	vsToResolve.push_back( bd.m_sFile2 );

	vector<RString> vsResolved;
	vsResolved.resize( vsToResolve.size() );
	for( unsigned i=0; i<vsToResolve.size(); i++ )
	{
		const RString &sToResolve = vsToResolve[i];
	
		LUA->SetGlobal( ssprintf("File%d",i+1), RString() );

		if( sToResolve.empty() )
		{
			if( i == 0 )
				return false;
			else
				continue;
		}

		// Look for vsFileToResolve[i] in:
		//  - song's dir
		//  - RandomMovies dir
		//  - BGAnimations dir.
		vector<RString> vsPaths, vsThrowAway;

		// Look for BGAnims in the song dir
		if( sToResolve == SONG_BACKGROUND_FILE )
			vsPaths.push_back( pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathG("Common","fallback background") );
		if( vsPaths.empty() )	BackgroundUtil::GetSongBGAnimations(	pSong, sToResolve, vsPaths, vsThrowAway );
		if( vsPaths.empty() )	BackgroundUtil::GetSongMovies(			pSong, sToResolve, vsPaths, vsThrowAway );
		if( vsPaths.empty() )	BackgroundUtil::GetSongBitmaps(			pSong, sToResolve, vsPaths, vsThrowAway );
		if( vsPaths.empty() )	BackgroundUtil::GetGlobalBGAnimations(	pSong, sToResolve, vsPaths, vsThrowAway );
		if( vsPaths.empty() )	BackgroundUtil::GetGlobalRandomMovies(	pSong, sToResolve, vsPaths, vsThrowAway );

		RString &sResolved = vsResolved[i];

		if( !vsPaths.empty() )
		{
			sResolved = vsPaths[0];
		}
		else
		{
			// If the main background file is missing, we failed.
			if( i == 0 )
				return false;
			else
				sResolved = ThemeManager::GetBlankGraphicPath();
		}
		
		ASSERT( !sResolved.empty() );

		LUA->SetGlobal( ssprintf("File%d",i+1), sResolved );
	}

	RString sEffect = bd.m_sEffect;
	if( sEffect.empty() )
	{
		FileType ft = ActorUtil::GetFileType(vsResolved[0]);
		switch( ft )
		{
		default:
			ASSERT(0);	// fall through
		case FT_Bitmap:
		case FT_Movie:
			sEffect = SBE_StretchNormal;
			break;
		case FT_Directory:
		case FT_Xml:
		case FT_Model:
			sEffect = SBE_UpperLeft;
			break;
		}
	}
	ASSERT( !sEffect.empty() );


	// Set Lua color globals
	LUA->SetGlobal( "Color1", bd.m_sColor1.empty() ? RString("1,1,1,1") : bd.m_sColor1 );
	LUA->SetGlobal( "Color2", bd.m_sColor2.empty() ? RString("1,1,1,1") : bd.m_sColor2 );


	// Resolve the effect file.
	RString sEffectFile;
	for( int i=0; i<2; i++ )
	{
		vector<RString> vsPaths, vsThrowAway;
		BackgroundUtil::GetBackgroundEffects( sEffect, vsPaths, vsThrowAway );
		if( vsPaths.empty() )
		{
			LOG->Warn( "BackgroundEffect '%s' is missing.",sEffect.c_str() );
			sEffect = SBE_Centered;
		}
		else if( vsPaths.size() > 1 )
		{
			LOG->Warn( "BackgroundEffect '%s' has more than one match.",sEffect.c_str() );
			sEffect = SBE_Centered;
		}
		else
		{
			sEffectFile = vsPaths[0];
			break;
		}
	}
	ASSERT( !sEffectFile.empty() );


	Actor *pActor = ActorUtil::MakeActor( sEffectFile );

	ASSERT( pActor );
	m_BGAnimations[bd] = pActor;

	for( unsigned i=0; i<vsResolved.size(); i++ )
		LUA->SetGlobal( ssprintf("File%d",i+1), RString() );
	LUA->UnsetGlobal( "Color1" );
	LUA->UnsetGlobal( "Color2" );

	return true;
}

BackgroundDef BackgroundImpl::Layer::CreateRandomBGA( const Song *pSong, const RString &sEffect, deque<BackgroundDef> &RandomBGAnimations )
{
	if( PREFSMAN->m_RandomBackgroundMode == PrefsManager::BGMODE_OFF )
		return BackgroundDef();

	if( RandomBGAnimations.empty() )
		return BackgroundDef();

	/* XXX: every time we fully loop, shuffle, so we don't play the same sequence
	 * over and over; and nudge the shuffle so the next one won't be a repeat */
	BackgroundDef bd = RandomBGAnimations.front();
	RandomBGAnimations.push_back( RandomBGAnimations.front() );
	RandomBGAnimations.pop_front();

	if( !sEffect.empty() )
		bd.m_sEffect = sEffect;

	map<BackgroundDef,Actor*>::const_iterator iter = m_BGAnimations.find( bd );
	
	// create the background if it's not already created
	if( iter == m_BGAnimations.end() )
	{
		bool bSuccess = CreateBackground( pSong, bd );
		ASSERT( bSuccess );	// we fed it valid files, so this shouldn't fail
	}
	return bd;
}

void BackgroundImpl::LoadFromRandom( float fFirstBeat, float fEndBeat, const BackgroundChange &change )
{
	const TimingData &timing = m_pSong->m_Timing;

	// change BG every 4 bars
	for( float f=fFirstBeat; f<fEndBeat; f+=BEATS_PER_MEASURE*4 )
	{
		// Don't fade.  It causes frame rate dip, especially on slower machines.
		BackgroundDef bd = m_Layer[0].CreateRandomBGA( m_pSong, change.m_def.m_sEffect, m_RandomBGAnimations );
		if( !bd.IsEmpty() )
		{
			BackgroundChange c = change;
			c.m_def = bd;
			c.m_fStartBeat = f;
			m_Layer[0].m_aBGChanges.push_back( c );
		}
	}

	// change BG every BPM change that is at the beginning of a measure
	int iStartIndex = BeatToNoteRow(fFirstBeat);
	int iEndIndex = BeatToNoteRow(fEndBeat);
	for( unsigned i=0; i<timing.m_BPMSegments.size(); i++ )
	{
		const BPMSegment& bpmseg = timing.m_BPMSegments[i];

		if( bpmseg.m_iStartIndex % BeatToNoteRow((float) BEATS_PER_MEASURE) != 0 )
			continue;	// skip
		
		// start so that we don't create a BGChange right on top of fEndBeat
		bool bInRange = bpmseg.m_iStartIndex >= iStartIndex && bpmseg.m_iStartIndex < iEndIndex;
		if( !bInRange )
			continue;	// skip

		BackgroundDef bd = m_Layer[0].CreateRandomBGA( m_pSong, change.m_def.m_sEffect, m_RandomBGAnimations );
		if( !bd.IsEmpty() )
		{
			BackgroundChange c = change;
			c.m_def.m_sFile1 = bd.m_sFile1;
			c.m_def.m_sFile2 = bd.m_sFile2;
			c.m_fStartBeat = NoteRowToBeat(bpmseg.m_iStartIndex);
			m_Layer[0].m_aBGChanges.push_back( c );
		}
	}
}

void BackgroundImpl::LoadFromSong( const Song* pSong )
{
	Init();
	Unload();
	m_pSong = pSong;
	m_StaticBackgroundDef.m_sFile1 = SONG_BACKGROUND_FILE;

	if( PREFSMAN->m_fBGBrightness == 0.0f )
		return;

	//
	// Choose a bunch of background that we'll use for the random file marker
	//
	{
		vector<RString> vsThrowAway, vsNames;
		switch( PREFSMAN->m_RandomBackgroundMode )
		{
		default:
			ASSERT_M( 0, ssprintf("Invalid RandomBackgroundMode: %i", int(PREFSMAN->m_RandomBackgroundMode)) );
			break;
		case PrefsManager::BGMODE_OFF:
			break;
		case PrefsManager::BGMODE_ANIMATIONS:
			BackgroundUtil::GetGlobalBGAnimations( pSong, "", vsThrowAway, vsNames );
			break;
		case PrefsManager::BGMODE_RANDOMMOVIES:
			BackgroundUtil::GetGlobalRandomMovies( pSong, "", vsThrowAway, vsNames );
			break;
		}

		random_shuffle( vsNames.begin(), vsNames.end() );
		int iSize = min( (int)PREFSMAN->m_iNumBackgrounds, (int)vsNames.size() );
		vsNames.resize( iSize );

		FOREACH_CONST( RString, vsNames, s )
		{
			BackgroundDef bd;
			bd.m_sFile1 = *s;
			m_RandomBGAnimations.push_back( bd );
		}
	}


		
	/* Song backgrounds (even just background stills) can get very big; never keep them
	 * in memory. */
	RageTextureID::TexPolicy OldPolicy = TEXTUREMAN->GetDefaultTexturePolicy();
	TEXTUREMAN->SetDefaultTexturePolicy( RageTextureID::TEX_VOLATILE );

	TEXTUREMAN->DisableOddDimensionWarning();

	const float fXZoom = RECT_BACKGROUND.GetWidth() / (float)SCREEN_WIDTH;
	const float fYZoom = RECT_BACKGROUND.GetHeight() / (float)SCREEN_HEIGHT;



	if( !PREFSMAN->m_bSongBackgrounds )
	{
		/* Backgrounds are disabled; just display the song background. */
		BackgroundChange change;
		change.m_def = m_StaticBackgroundDef;
		change.m_fStartBeat = 0;
		m_Layer[0].m_aBGChanges.push_back( change );
	}
	else if( pSong->HasBGChanges() )
	{
		FOREACH_BackgroundLayer( i )
		{
			Layer &layer = m_Layer[i];

			// Load all song-specified backgrounds
			FOREACH_CONST( BackgroundChange, pSong->GetBackgroundChanges(i), bgc )
			{
				BackgroundChange change = *bgc;
				BackgroundDef &bd = change.m_def;
				
				bool bIsAlreadyLoaded = layer.m_BGAnimations.find(bd) != layer.m_BGAnimations.end();

				if( bd.m_sFile1 != RANDOM_BACKGROUND_FILE  &&  !bIsAlreadyLoaded )
				{
					if( layer.CreateBackground( m_pSong, bd ) )
					{
						;	// do nothing.  Create was successful.
					}
					else
					{
						if( i == BACKGROUND_LAYER_1 )
						{
							// The background was not found.  Try to use a random one instead.
							bd = layer.CreateRandomBGA( pSong, "", m_RandomBGAnimations );
							if( bd.IsEmpty() )
								bd = m_StaticBackgroundDef;
						}
					}
				}
			
				if( !bd.IsEmpty() )
					layer.m_aBGChanges.push_back( change );
			}
		}
	}
	else	// pSong doesn't have an animation plan
	{
		Layer &layer = m_Layer[0];

		LoadFromRandom( pSong->m_fFirstBeat, pSong->m_fLastBeat, BackgroundChange() );

		// end showing the static song background
		BackgroundChange change;
		change.m_def = m_StaticBackgroundDef;
		change.m_fStartBeat = pSong->m_fLastBeat;
		layer.m_aBGChanges.push_back( change );
	}

		
	// sort segments
	FOREACH_BackgroundLayer( i )
	{
		Layer &layer = m_Layer[i];
		BackgroundUtil::SortBackgroundChangesArray( layer.m_aBGChanges );
	}


	Layer &mainlayer = m_Layer[0];


	/* If the first BGAnimation isn't negative, add a lead-in image showing the song
	 * background. */
	if( mainlayer.m_aBGChanges.empty() || mainlayer.m_aBGChanges.front().m_fStartBeat >= 0 )
	{
		BackgroundChange change;
		change.m_def = m_StaticBackgroundDef;
		change.m_fStartBeat = -10000;
		mainlayer.m_aBGChanges.insert( mainlayer.m_aBGChanges.begin(), change );
	}

	// If any BGChanges use the background image, load it.
	bool bStaticBackgroundUsed = false;
	FOREACH_BackgroundLayer( i )
	{
		Layer &layer = m_Layer[i];
		FOREACH_CONST( BackgroundChange, layer.m_aBGChanges, bgc )
		{
			const BackgroundDef &bd = bgc->m_def;
			if( bd == m_StaticBackgroundDef )
			{
				bStaticBackgroundUsed = true;
				break;
			}
		}
		if( bStaticBackgroundUsed )
			break;
	}

	if( bStaticBackgroundUsed )
	{
		bool bIsAlreadyLoaded = mainlayer.m_BGAnimations.find(m_StaticBackgroundDef) != mainlayer.m_BGAnimations.end();
		if( !bIsAlreadyLoaded )
		{
			bool bSuccess = mainlayer.CreateBackground( m_pSong, m_StaticBackgroundDef );
			ASSERT( bSuccess );
		}
	}


	// Look for the random file marker, and replace the segment with LoadFromRandom.
	for( unsigned i=0; i<mainlayer.m_aBGChanges.size(); i++ )
	{
		const BackgroundChange change = mainlayer.m_aBGChanges[i];
		if( change.m_def.m_sFile1 != RANDOM_BACKGROUND_FILE )
			continue;

		const float fStartBeat = change.m_fStartBeat;
		const float fEndBeat = (i+1 < mainlayer.m_aBGChanges.size())? mainlayer.m_aBGChanges[i+1].m_fStartBeat: FLT_MAX;

		mainlayer.m_aBGChanges.erase( mainlayer.m_aBGChanges.begin()+i );
		--i;

		LoadFromRandom( fStartBeat, fEndBeat, change );
	}

	// At this point, we shouldn't have any BGChanges to "".  "" is an invalid name.
	for( unsigned i=0; i<mainlayer.m_aBGChanges.size(); i++ )
		ASSERT( !mainlayer.m_aBGChanges[i].m_def.m_sFile1.empty() );


	// Re-sort.
	BackgroundUtil::SortBackgroundChangesArray( mainlayer.m_aBGChanges );

	m_DangerAll->SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
	m_DangerAll->SetZoomX( fXZoom );
	m_DangerAll->SetZoomY( fYZoom );	

	FOREACH_PlayerNumber( p )
	{
		m_DangerPlayer[p]->SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_DangerPlayer[p]->SetZoomX( fXZoom );
		m_DangerPlayer[p]->SetZoomY( fYZoom );
		m_DangerPlayer[p]->FinishTweening();
		m_DangerPlayer[p]->PlayCommand( "On" );
	
		m_DeadPlayer[p]->SetXY( (float)LEFT_EDGE, (float)TOP_EDGE );
		m_DeadPlayer[p]->SetZoomX( fXZoom );
		m_DeadPlayer[p]->SetZoomY( fYZoom );	
		m_DeadPlayer[p]->FinishTweening();
		m_DeadPlayer[p]->PlayCommand( "On" );
	}

	TEXTUREMAN->EnableOddDimensionWarning();

	if( m_pDancingCharacters )
		m_pDancingCharacters->LoadNextSong();

	TEXTUREMAN->SetDefaultTexturePolicy( OldPolicy );

	/* Song backgrounds always sync from the music by default, unlike other actors
	 * which sync from the regular clock by default.  If you don't want this, set
	 * the clock back with "effectclock,timer" in your OnCommand.  Note that at this
	 * point, we havn't run the OnCommand yet, so it'll override correctly. */
	FOREACHM( BackgroundDef, Actor*, mainlayer.m_BGAnimations, it )
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

int BackgroundImpl::Layer::FindBGSegmentForBeat( float fBeat ) const
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
void BackgroundImpl::Layer::UpdateCurBGChange( const Song *pSong, float fLastMusicSeconds, float fCurrentTime, const map<RString,BackgroundTransition> &mapNameToTransition )
{
	ASSERT( fCurrentTime != GameState::MUSIC_SECONDS_INVALID );

	if( m_aBGChanges.size() == 0 )
		return;

	float fBeat, fBPS;
	bool bFreeze;
	pSong->m_Timing.GetBeatAndBPSFromElapsedTime( fCurrentTime, fBeat, fBPS, bFreeze );

	/* Calls to Update() should *not* be scaled by music rate; fCurrentTime is. Undo it. */
	const float fRate = GAMESTATE->m_SongOptions.m_fMusicRate;

	// Find the BGSegment we're in
	const int i = FindBGSegmentForBeat( fBeat );

	float fDeltaTime = fCurrentTime - fLastMusicSeconds;
	if( i != -1  &&  i != m_iCurBGChangeIndex )	// we're changing backgrounds
	{
		LOG->Trace( "old bga %d -> new bga %d, %f, %f", m_iCurBGChangeIndex, i, m_aBGChanges[i].m_fStartBeat, fBeat );

		BackgroundChange oldChange;
		if( m_iCurBGChangeIndex != -1 )
			oldChange = m_aBGChanges[m_iCurBGChangeIndex];

		m_iCurBGChangeIndex = i;

		const BackgroundChange& change = m_aBGChanges[i];

		m_pFadingBGA = m_pCurrentBGA;

		map<BackgroundDef,Actor*>::const_iterator iter = m_BGAnimations.find( change.m_def );
		if( iter == m_BGAnimations.end() )
		{
			XNode *pNode = change.m_def.CreateNode();
			LOG->Warn( "Tried to switch to a background that was never loaded\n" + pNode->GetXML() );
			SAFE_DELETE( pNode );
			return;
		}

		m_pCurrentBGA = iter->second;

		if( m_pFadingBGA == m_pCurrentBGA )
		{
			m_pFadingBGA = NULL;
			LOG->Trace( "bg didn't actually change.  Ignoring." );
		}
		else
		{
			if( m_pFadingBGA )
			{
				m_pFadingBGA->PlayCommand( "LoseFocus" );
				
				if( !change.m_sTransition.empty() )
				{
					map<RString,BackgroundTransition>::const_iterator iter = mapNameToTransition.find( change.m_sTransition );
					ASSERT( iter != mapNameToTransition.end() );
					const BackgroundTransition &bt = iter->second;
					m_pFadingBGA->RunCommandsOnLeaves( *bt.cmdLeaves );
					m_pFadingBGA->RunCommands( *bt.cmdRoot );
				}
			}
		}

		m_pCurrentBGA->SetUpdateRate( change.m_fRate );

		// Set Lua color globals before calling Init and On
		LUA->SetGlobal( "Color1", change.m_def.m_sColor1.empty() ? RString("1,1,1,1") : change.m_def.m_sColor1 );
		LUA->SetGlobal( "Color2", change.m_def.m_sColor2.empty() ? RString("1,1,1,1") : change.m_def.m_sColor2 );
		
		m_pCurrentBGA->PlayCommand( "Init" );
		m_pCurrentBGA->PlayCommand( "On" );
		m_pCurrentBGA->PlayCommand( "GainFocus" );

		LUA->UnsetGlobal( "Color1" );
		LUA->UnsetGlobal( "Color2" );

		/* How much time of this BGA have we skipped?  (This happens with SetSeconds.) */
		const float fStartSecond = pSong->m_Timing.GetElapsedTimeFromBeat( change.m_fStartBeat );

		/* This is affected by the music rate. */
		fDeltaTime = fCurrentTime - fStartSecond;
	}

	if( m_pFadingBGA )
	{
		if( m_pFadingBGA->GetTweenTimeLeft() == 0 )
			m_pFadingBGA = NULL;
	}

	/* This is affected by the music rate. */
	float fDeltaTimeMusicRate = max( fDeltaTime / fRate, 0 );

	if( m_pCurrentBGA )
		m_pCurrentBGA->Update( fDeltaTimeMusicRate );
	if( m_pFadingBGA )
		m_pFadingBGA->Update( fDeltaTimeMusicRate );
}

void BackgroundImpl::Layer::ProcessMessages( float fDeltaTime )
{
	if( m_pCurrentBGA )
		m_pCurrentBGA->ProcessMessages( fDeltaTime );
	if( m_pFadingBGA )
		m_pFadingBGA->ProcessMessages( fDeltaTime );
}

void BackgroundImpl::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( IsDangerAllVisible() )
	{
		m_DangerAll->Update( fDeltaTime );
	}

	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsPlayerInDanger(GAMESTATE->m_pPlayerState[p]) )
			m_DangerPlayer[p]->Update( fDeltaTime );
			
		if( GAMESTATE->IsPlayerDead(GAMESTATE->m_pPlayerState[p]) )
			m_DeadPlayer[p]->Update( fDeltaTime );
	}

	if( m_pDancingCharacters )
		m_pDancingCharacters->Update( fDeltaTime );

	/* Always update the current background, even when m_DangerAll is being displayed.
	 * Otherwise, we'll stop updating movies during danger (which may stop them from
	 * playing), and we won't start clips at the right time, which will throw backgrounds
	 * off sync. */
	FOREACH_BackgroundLayer( i )
	{
		Layer &layer = m_Layer[i];
		layer.UpdateCurBGChange( m_pSong, m_fLastMusicSeconds, GAMESTATE->m_fMusicSeconds, m_mapNameToTransition );
	}
	m_fLastMusicSeconds = GAMESTATE->m_fMusicSeconds;
}

void BackgroundImpl::ProcessMessages( float fDeltaTime )
{
	ActorFrame::ProcessMessages( fDeltaTime );
	FOREACH_BackgroundLayer( i )
	{
		Layer &layer = m_Layer[i];
		layer.ProcessMessages( fDeltaTime );
	}
}

void BackgroundImpl::DrawPrimitives()
{
	if( PREFSMAN->m_fBGBrightness == 0.0f )
		return;

	if( IsDangerAllVisible() )
	{
		// Since this only shows when DANGER is visible, it will flash red on it's own accord :)
		if( m_pDancingCharacters )
			m_pDancingCharacters->m_bDrawDangerLight = true;
		m_DangerAll->Draw();
	}
	
	if( !IsDangerAllVisible() || !(bool)DANGER_ALL_IS_OPAQUE ) 
	{	
		if( m_pDancingCharacters )
			m_pDancingCharacters->m_bDrawDangerLight = false;
		
		FOREACH_BackgroundLayer( i )
		{
			Layer &layer = m_Layer[i];
			if( layer.m_pCurrentBGA )
				layer.m_pCurrentBGA->Draw();
			if( layer.m_pFadingBGA )
				layer.m_pFadingBGA->Draw();
		}

		FOREACH_PlayerNumber( p )
		{
			if( GAMESTATE->IsPlayerInDanger(GAMESTATE->m_pPlayerState[p]) )
				m_DangerPlayer[p]->Draw();
			if( GAMESTATE->IsPlayerDead(GAMESTATE->m_pPlayerState[p]) )
				m_DeadPlayer[p]->Draw();
		}
	}

	if( m_pDancingCharacters )
		m_pDancingCharacters->Draw();

	ActorFrame::DrawPrimitives();
}

void BackgroundImpl::GetLoadedBackgroundChanges( vector<BackgroundChange> *pBackgroundChangesOut[NUM_BackgroundLayer] )
{
	FOREACH_BackgroundLayer( i )
		*pBackgroundChangesOut[i] = m_Layer[i].m_aBGChanges;
}

bool BackgroundImpl::IsDangerAllVisible()
{
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->GetPlayerFailType(GAMESTATE->m_pPlayerState[p]) == SongOptions::FAIL_OFF )
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
	if( !GAMESTATE->m_bGameplayLeadIn )
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



Background::Background()	{ m_pImpl = new BackgroundImpl; this->AddChild(m_pImpl); }
Background::~Background()	{ SAFE_DELETE( m_pImpl ); }
void Background::Init()		{ m_pImpl->Init(); }
void Background::LoadFromSong( const Song *pSong )		{ m_pImpl->LoadFromSong(pSong); }
void Background::Unload()								{ m_pImpl->Unload(); }
void Background::FadeToActualBrightness()				{ m_pImpl->FadeToActualBrightness(); }
void Background::SetBrightness( float fBrightness )		{ m_pImpl->SetBrightness(fBrightness); }
DancingCharacters* Background::GetDancingCharacters()	{ return m_pImpl->GetDancingCharacters(); }
void Background::GetLoadedBackgroundChanges( vector<BackgroundChange> *pBackgroundChangesOut[NUM_BackgroundLayer] ) { m_pImpl->GetLoadedBackgroundChanges(pBackgroundChangesOut); }


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
