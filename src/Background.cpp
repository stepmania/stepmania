#include "global.h"
#include "Background.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "RageDisplay.h"
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
#include "ActorUtil.h"
#include <float.h>
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "BackgroundUtil.h"
#include "Song.h"
#include "AutoActor.h"

static ThemeMetric<float> LEFT_EDGE				("Background","LeftEdge");
static ThemeMetric<float> TOP_EDGE				("Background","TopEdge");
static ThemeMetric<float> RIGHT_EDGE				("Background","RightEdge");
static ThemeMetric<float> BOTTOM_EDGE				("Background","BottomEdge");
static ThemeMetric<float> CLAMP_OUTPUT_PERCENT			("Background","ClampOutputPercent");
static ThemeMetric<bool> SHOW_DANCING_CHARACTERS		("Background","ShowDancingCharacters");
static ThemeMetric<bool> USE_STATIC_BG		("Background","UseStaticBackground");
static ThemeMetric<float> RAND_BG_START_BEAT("Background", "RandomBGStartBeat");
static ThemeMetric<float> RAND_BG_CHANGE_MEASURES("Background", "RandomBGChangeMeasures");
static ThemeMetric<bool> RAND_BG_CHANGES_WHEN_BPM_CHANGES("Background", "RandomBGChangesWhenBPMChangesAtMeasureStart");
static ThemeMetric<bool> RAND_BG_ENDS_AT_LAST_BEAT("Background", "RandomBGEndsAtLastBeat");


static Preference<bool>	g_bShowDanger( "ShowDanger", true );
static Preference<float> g_fBGBrightness( "BGBrightness", 0.7f );
static Preference<RandomBackgroundMode> g_RandomBackgroundMode( "RandomBackgroundMode",	BGMODE_RANDOMMOVIES );
static Preference<int> g_iNumBackgrounds( "NumBackgrounds", 10 );
static Preference<bool> g_bSongBackgrounds( "SongBackgrounds", true );

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
		bool CreateBackground( const Song *pSong, const BackgroundDef &bd, Actor *pParent );
		// return def of the background that was created and added to m_BGAnimations. calls CreateBackground
		BackgroundDef CreateRandomBGA( const Song *pSong, const RString &sEffect, deque<BackgroundDef> &RandomBGAnimations, Actor *pParent );

		int FindBGSegmentForBeat( float fBeat ) const;
		void UpdateCurBGChange( const Song *pSong, float fLastMusicSeconds, float fCurrentTime, const map<RString,BackgroundTransition> &mapNameToTransition );

		map<BackgroundDef,Actor*> m_BGAnimations;
		vector<BackgroundChange> m_aBGChanges;
		int	  m_iCurBGChangeIndex;
		Actor *m_pCurrentBGA;
		Actor *m_pFadingBGA;
	};
	Layer m_Layer[NUM_BackgroundLayer];

	float m_fLastMusicSeconds;
	bool m_bDangerAllWasVisible;

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
	m_pDancingCharacters = nullptr;
	m_pSong = nullptr;
}

BackgroundImpl::Layer::Layer()
{
	m_iCurBGChangeIndex = -1;
	m_pCurrentBGA = nullptr;
	m_pFadingBGA = nullptr;
}

void BackgroundImpl::Init()
{
	if( m_bInitted )
		return;
	m_bInitted = true;
	m_bDangerAllWasVisible = false;
	m_StaticBackgroundDef = BackgroundDef();

	if( !USE_STATIC_BG )
	{
		m_StaticBackgroundDef.m_sColor1 = "#00000000";
		m_StaticBackgroundDef.m_sColor2 = "#00000000";
	}

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
			ASSERT( xml.GetName() == "BackgroundTransition" );
			BackgroundTransition &bgt = m_mapNameToTransition[sName];

			RString sCmdLeaves;
			bool bSuccess = xml.GetAttrValue( "LeavesCommand", sCmdLeaves );
			ASSERT( bSuccess );
			bgt.cmdLeaves = ActorUtil::ParseActorCommands( sCmdLeaves );

			RString sCmdRoot;
			bSuccess = xml.GetAttrValue( "RootCommand", sCmdRoot );
			ASSERT( bSuccess );
			bgt.cmdRoot = ActorUtil::ParseActorCommands( sCmdRoot );
		}
	}

	bool bOneOrMoreChars = false;
	bool bShowingBeginnerHelper = false;
	FOREACH_HumanPlayer( p )
	{
		bOneOrMoreChars = true;
		// Disable dancing characters if Beginner Helper will be showing.
		if( PREFSMAN->m_bShowBeginnerHelper && BeginnerHelper::CanUse(p) &&
			GAMESTATE->m_pCurSteps[p] && GAMESTATE->m_pCurSteps[p]->GetDifficulty() == Difficulty_Beginner )
			bShowingBeginnerHelper = true;
	}

	if( bOneOrMoreChars && !bShowingBeginnerHelper && SHOW_DANCING_CHARACTERS )
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
	m_pSong = nullptr;
	m_fLastMusicSeconds	= -9999;
	m_RandomBGAnimations.clear();
}

void BackgroundImpl::Layer::Unload()
{
	for (std::pair<BackgroundDef const &, Actor *> iter : m_BGAnimations)
		delete iter.second;
	m_BGAnimations.clear();
	m_aBGChanges.clear();

	m_pCurrentBGA = nullptr;
	m_pFadingBGA = nullptr;
	m_iCurBGChangeIndex = -1;
}

bool BackgroundImpl::Layer::CreateBackground( const Song *pSong, const BackgroundDef &bd, Actor *pParent )
{
	ASSERT( m_BGAnimations.find(bd) == m_BGAnimations.end() );

	// Resolve the background names
	vector<RString> vsToResolve;
	vsToResolve.push_back( bd.m_sFile1 );
	vsToResolve.push_back( bd.m_sFile2 );

	vector<RString> vsResolved;
	vsResolved.resize( vsToResolve.size() );
	vector<LuaThreadVariable *> vsResolvedRef;
	vsResolvedRef.resize( vsToResolve.size() );

	for( unsigned i=0; i<vsToResolve.size(); i++ )
	{
		const RString &sToResolve = vsToResolve[i];

		if( sToResolve.empty() )
		{
			if( i == 0 )
				return false;
			else
				continue;
		}

		/* Look for vsFileToResolve[i] in:
			* song's dir
			* RandomMovies dir
			* BGAnimations dir.
		*/
		vector<RString> vsPaths, vsThrowAway;

		// Look for BGAnims in the song dir
		if( sToResolve == SONG_BACKGROUND_FILE )
			vsPaths.push_back( pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathG("Common","fallback background") );
		if( vsPaths.empty() )	BackgroundUtil::GetSongBGAnimations(	pSong, sToResolve, vsPaths, vsThrowAway );
		if( vsPaths.empty() )	BackgroundUtil::GetSongMovies(		pSong, sToResolve, vsPaths, vsThrowAway );
		if( vsPaths.empty() )	BackgroundUtil::GetSongBitmaps(		pSong, sToResolve, vsPaths, vsThrowAway );
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
				sResolved = "../"+ThemeManager::GetBlankGraphicPath();
		}

		ASSERT( !sResolved.empty() );

		vsResolvedRef[i] = new LuaThreadVariable( ssprintf("File%d",i+1), sResolved );
	}

	RString sEffect = bd.m_sEffect;
	if( sEffect.empty() )
	{
		FileType ft = ActorUtil::GetFileType(vsResolved[0]);
		switch( ft )
		{
		default:
			LuaHelpers::ReportScriptErrorFmt( "CreateBackground() Unknown file type '%s'", vsResolved[0].c_str() );
			// fall through
		case FT_Bitmap:
		case FT_Sprite:
		case FT_Movie:
			sEffect = SBE_StretchNormal;
			break;
		case FT_Lua:
		case FT_Model:
			sEffect = SBE_UpperLeft;
			break;
		case FT_Xml:
			sEffect = SBE_Centered;
			break;
		case FT_Directory:
			if( DoesFileExist(vsResolved[0] + "/default.lua") )
				sEffect = SBE_UpperLeft;
			else
				sEffect = SBE_Centered;
			break;
		}
	}
	ASSERT( !sEffect.empty() );

	// Set Lua color globals
	LuaThreadVariable sColor1( "Color1", bd.m_sColor1.empty() ? RString("#FFFFFFFF") : bd.m_sColor1 );
	LuaThreadVariable sColor2( "Color2", bd.m_sColor2.empty() ? RString("#FFFFFFFF") : bd.m_sColor2 );

	// Resolve the effect file.
	RString sEffectFile;
	for( int i=0; i<2; i++ )
	{
		vector<RString> vsPaths, vsThrowAway;
		BackgroundUtil::GetBackgroundEffects( sEffect, vsPaths, vsThrowAway );
		if( vsPaths.empty() )
		{
			LuaHelpers::ReportScriptErrorFmt( "BackgroundEffect '%s' is missing.",sEffect.c_str() );
			sEffect = SBE_Centered;
		}
		else if( vsPaths.size() > 1 )
		{
			LuaHelpers::ReportScriptErrorFmt( "BackgroundEffect '%s' has more than one match.",sEffect.c_str() );
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

	if( pActor == nullptr )
		pActor = new Actor;
	m_BGAnimations[bd] = pActor;

	for( unsigned i=0; i<vsResolvedRef.size(); i++ )
		delete vsResolvedRef[i];

	return true;
}

BackgroundDef BackgroundImpl::Layer::CreateRandomBGA( const Song *pSong, const RString &sEffect, deque<BackgroundDef> &RandomBGAnimations, Actor *pParent )
{
	if( g_RandomBackgroundMode == BGMODE_OFF )
		return BackgroundDef();

	// Set to not show any BGChanges, whether scripted or random
	if( GAMESTATE->m_SongOptions.GetCurrent().m_bStaticBackground )
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
		bool bSuccess = CreateBackground( pSong, bd, pParent );
		ASSERT( bSuccess );	// we fed it valid files, so this shouldn't fail
	}
	return bd;
}

void BackgroundImpl::LoadFromRandom( float fFirstBeat, float fEndBeat, const BackgroundChange &change )
{
	int iStartRow = BeatToNoteRow(fFirstBeat);
	int iEndRow = BeatToNoteRow(fEndBeat);

	const TimingData &timing = m_pSong->m_SongTiming;

	// change BG every time signature change or 4 measures
	const vector<TimingSegment *> &tSigs = timing.GetTimingSegments(SEGMENT_TIME_SIG);

	for (unsigned i = 0; i < tSigs.size(); i++)
	{
		TimeSignatureSegment *ts = static_cast<TimeSignatureSegment *>(tSigs[i]);
		int iSegmentEndRow = (i + 1 == tSigs.size()) ? iEndRow : tSigs[i+1]->GetRow();


		int time_signature_start= max(ts->GetRow(),iStartRow);
		for(int j= time_signature_start;
			j<min(iEndRow,iSegmentEndRow);
			j+= int(RAND_BG_CHANGE_MEASURES * ts->GetNoteRowsPerMeasure()))
		{
			// Don't fade. It causes frame rate dip, especially on slower machines.
			BackgroundDef bd = m_Layer[0].CreateRandomBGA(m_pSong,
														  change.m_def.m_sEffect,
														  m_RandomBGAnimations, this);
			if( !bd.IsEmpty() )
			{
				BackgroundChange c = change;
				c.m_def = bd;
				if(j == time_signature_start && i == 0)
				{
					c.m_fStartBeat = RAND_BG_START_BEAT;
				}
				else
				{
					c.m_fStartBeat = NoteRowToBeat(j);
				}
				m_Layer[0].m_aBGChanges.push_back( c );
			}
		}
	}

	if(RAND_BG_CHANGES_WHEN_BPM_CHANGES)
	{
		// change BG every BPM change that is at the beginning of a measure
		const vector<TimingSegment *> &bpms = timing.GetTimingSegments(SEGMENT_BPM);
		for( unsigned i=0; i<bpms.size(); i++ )
		{
			bool bAtBeginningOfMeasure = false;
			for (unsigned j=0; j<tSigs.size(); j++)
			{
				TimeSignatureSegment *ts = static_cast<TimeSignatureSegment *>(tSigs[j]);
				if ((bpms[i]->GetRow() - ts->GetRow()) % ts->GetNoteRowsPerMeasure() == 0)
				{
					bAtBeginningOfMeasure = true;
					break;
				}
			}

			if( !bAtBeginningOfMeasure )
			continue; // skip

			// start so that we don't create a BGChange right on top of fEndBeat
			bool bInRange = bpms[i]->GetRow() >= iStartRow && bpms[i]->GetRow() < iEndRow;
			if( !bInRange )
			continue; // skip

			BackgroundDef bd = m_Layer[0].CreateRandomBGA( m_pSong, change.m_def.m_sEffect, m_RandomBGAnimations, this );
			if( !bd.IsEmpty() )
			{
				BackgroundChange c = change;
				c.m_def.m_sFile1 = bd.m_sFile1;
				c.m_def.m_sFile2 = bd.m_sFile2;
				c.m_fStartBeat = bpms[i]->GetBeat();
				m_Layer[0].m_aBGChanges.push_back( c );
			}
		}
	}
}

void BackgroundImpl::LoadFromSong( const Song* pSong )
{
	Init();
	Unload();
	m_pSong = pSong;
	m_StaticBackgroundDef.m_sFile1 = SONG_BACKGROUND_FILE;

	if( g_fBGBrightness == 0.0f )
		return;

	// Choose a bunch of backgrounds that we'll use for the random file marker
	{
		vector<RString> vsThrowAway, vsNames;
		switch( g_RandomBackgroundMode )
		{
		default:
			ASSERT_M( 0, ssprintf("Invalid RandomBackgroundMode: %i", int(g_RandomBackgroundMode)) );
			break;
		case BGMODE_OFF:
			break;
		case BGMODE_ANIMATIONS:
			BackgroundUtil::GetGlobalBGAnimations( pSong, "", vsThrowAway, vsNames );
			break;
		case BGMODE_RANDOMMOVIES:
			BackgroundUtil::GetGlobalRandomMovies( pSong, "", vsThrowAway, vsNames, true, true );
			break;
		}

		// Pick the same random items every time the song is played.
		RandomGen rnd( GetHashForString(pSong->GetSongDir()) );
		random_shuffle( vsNames.begin(), vsNames.end(), rnd );
		int iSize = min( (int)g_iNumBackgrounds, (int)vsNames.size() );
		vsNames.resize( iSize );

		for (RString const &s : vsNames)
		{
			BackgroundDef bd;
			bd.m_sFile1 = s;
			m_RandomBGAnimations.push_back( bd );
		}
	}

	/* Song backgrounds (even just background stills) can get very big; never keep them
	 * in memory. */
	RageTextureID::TexPolicy OldPolicy = TEXTUREMAN->GetDefaultTexturePolicy();
	TEXTUREMAN->SetDefaultTexturePolicy( RageTextureID::TEX_VOLATILE );

	TEXTUREMAN->DisableOddDimensionWarning();

	// Set to not show any BGChanges, whether scripted or random if m_bStaticBackground is on
	if( !g_bSongBackgrounds || GAMESTATE->m_SongOptions.GetCurrent().m_bStaticBackground )
	{
		// Backgrounds are disabled; just display the song background.
		BackgroundChange change;
		change.m_def = m_StaticBackgroundDef;
		change.m_fStartBeat = 0;
		m_Layer[0].m_aBGChanges.push_back( change );
	}
	// If m_bRandomBGOnly is on, then we want to ignore the scripted BG in favour of randomly loaded BGs
	else if( pSong->HasBGChanges() && !GAMESTATE->m_SongOptions.GetCurrent().m_bRandomBGOnly )
	{
		FOREACH_BackgroundLayer( i )
		{
			Layer &layer = m_Layer[i];

			// Load all song-specified backgrounds
			for (BackgroundChange const &bgc : pSong->GetBackgroundChanges(i))
			{
				BackgroundChange change = bgc;
				BackgroundDef &bd = change.m_def;

				bool bIsAlreadyLoaded = layer.m_BGAnimations.find(bd) != layer.m_BGAnimations.end();

				if( bd.m_sFile1 != RANDOM_BACKGROUND_FILE  &&  !bIsAlreadyLoaded )
				{
					if( layer.CreateBackground( m_pSong, bd, this ) )
					{
						;	// do nothing.  Create was successful.
					}
					else
					{
						if( i == BACKGROUND_LAYER_1 )
						{
							// The background was not found. Try to use a random one instead.
							// Don't use the BackgroundDef's effect, because it may be an
							// effect that requires 2 files, and random BGA will only supply one file
							bd = layer.CreateRandomBGA( pSong, "", m_RandomBGAnimations, this );
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
		float firstBeat = pSong->GetFirstBeat();
		float lastBeat = pSong->GetLastBeat();

		LoadFromRandom( firstBeat, lastBeat, BackgroundChange() );

		if(RAND_BG_ENDS_AT_LAST_BEAT)
		{
			// end showing the static song background
			BackgroundChange change;
			change.m_def = m_StaticBackgroundDef;
			change.m_fStartBeat = lastBeat;
			layer.m_aBGChanges.push_back( change );
		}
	}

	// sort segments
	FOREACH_BackgroundLayer( i )
	{
		Layer &layer = m_Layer[i];
		BackgroundUtil::SortBackgroundChangesArray( layer.m_aBGChanges );
	}

	Layer &mainlayer = m_Layer[0];

	BackgroundChange change;
	change.m_def = m_StaticBackgroundDef;
	change.m_fStartBeat = -10000;
	mainlayer.m_aBGChanges.insert( mainlayer.m_aBGChanges.begin(), change );

	// If any BGChanges use the background image, load it.
	bool bStaticBackgroundUsed = false;
	FOREACH_BackgroundLayer( i )
	{
		Layer &layer = m_Layer[i];
		for (BackgroundChange const &bgc : layer.m_aBGChanges)
		{
			const BackgroundDef &bd = bgc.m_def;
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
			bool bSuccess = mainlayer.CreateBackground( m_pSong, m_StaticBackgroundDef, this );
			ASSERT( bSuccess );
		}
	}

	// Look for the random file marker, and replace the segment with LoadFromRandom.
	for( unsigned i=0; i<mainlayer.m_aBGChanges.size(); i++ )
	{
		const BackgroundChange change = mainlayer.m_aBGChanges[i];
		if( change.m_def.m_sFile1 != RANDOM_BACKGROUND_FILE )
			continue;

		float fStartBeat = change.m_fStartBeat;
		float fEndBeat = pSong->GetLastBeat();
		if( i+1 < mainlayer.m_aBGChanges.size() )
			fEndBeat = mainlayer.m_aBGChanges[i+1].m_fStartBeat;

		mainlayer.m_aBGChanges.erase( mainlayer.m_aBGChanges.begin()+i );
		--i;

		LoadFromRandom( fStartBeat, fEndBeat, change );
	}

	// At this point, we shouldn't have any BGChanges to "".  "" is an invalid name.
	for( unsigned i=0; i<mainlayer.m_aBGChanges.size(); i++ )
		ASSERT( !mainlayer.m_aBGChanges[i].m_def.m_sFile1.empty() );

	// Re-sort.
	BackgroundUtil::SortBackgroundChangesArray( mainlayer.m_aBGChanges );

	TEXTUREMAN->EnableOddDimensionWarning();

	if( m_pDancingCharacters )
		m_pDancingCharacters->LoadNextSong();

	TEXTUREMAN->SetDefaultTexturePolicy( OldPolicy );
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

/* If the BG segment has changed, move focus to it. Send Update() calls. */
void BackgroundImpl::Layer::UpdateCurBGChange( const Song *pSong, float fLastMusicSeconds, float fCurrentTime, const map<RString,BackgroundTransition> &mapNameToTransition )
{
	ASSERT( fCurrentTime != GameState::MUSIC_SECONDS_INVALID );

	if( m_aBGChanges.size() == 0 )
		return;

	TimingData::GetBeatArgs beat_info;
	beat_info.elapsed_time= fCurrentTime;
	pSong->m_SongTiming.GetBeatAndBPSFromElapsedTime(beat_info);

	// Calls to Update() should *not* be scaled by music rate unless RateModsAffectFGChanges is enabled; fCurrentTime is. Undo it.
	const float fRate = PREFSMAN->m_bRateModsAffectTweens ? 1.0f : GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;

	// Find the BGSegment we're in
	const int i = FindBGSegmentForBeat(beat_info.beat);

	float fDeltaTime = fCurrentTime - fLastMusicSeconds;
	if( i != -1  &&  i != m_iCurBGChangeIndex )	// we're changing backgrounds
	{
		//LOG->Trace( "old bga %d -> new bga %d (%s), %f, %f", m_iCurBGChangeIndex, i, m_aBGChanges[i].GetTextDescription().c_str(), m_aBGChanges[i].m_fStartBeat, fBeat );

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
			RString xml = XmlFileUtil::GetXML( pNode );
			Trim( xml );
			LuaHelpers::ReportScriptErrorFmt( "Tried to switch to a background that was never loaded:\n%s", xml.c_str() );
			SAFE_DELETE( pNode );
			return;
		}

		m_pCurrentBGA = iter->second;

		if( m_pFadingBGA == m_pCurrentBGA )
		{
			m_pFadingBGA = nullptr;
			//LOG->Trace( "bg didn't actually change.  Ignoring." );
		}
		else
		{
			if( m_pFadingBGA )
			{
				m_pFadingBGA->PlayCommand( "LoseFocus" );

				if( !change.m_sTransition.empty() )
				{
					map<RString,BackgroundTransition>::const_iterator lIter = mapNameToTransition.find( change.m_sTransition );
					if(lIter == mapNameToTransition.end())
					{
						LuaHelpers::ReportScriptErrorFmt("'%s' is not the name of a BackgroundTransition file.", change.m_sTransition.c_str());
					}
					else
					{
						const BackgroundTransition &bt = lIter->second;
						m_pFadingBGA->RunCommandsOnLeaves( *bt.cmdLeaves );
						m_pFadingBGA->RunCommands( *bt.cmdRoot );
					}
				}
			}
		}

		m_pCurrentBGA->SetUpdateRate( change.m_fRate );

		m_pCurrentBGA->InitState();
		m_pCurrentBGA->PlayCommand( "On" );
		m_pCurrentBGA->PlayCommand( "GainFocus" );

		/* How much time of this BGA have we skipped?  (This happens with SetSeconds.) */
		const float fStartSecond = pSong->m_SongTiming.GetElapsedTimeFromBeat( change.m_fStartBeat );

		/* This is affected by the music rate. */
		fDeltaTime = fCurrentTime - fStartSecond;
	}

	if( m_pFadingBGA )
	{
		if( m_pFadingBGA->GetTweenTimeLeft() == 0 )
			m_pFadingBGA = nullptr;
	}

	/* This is unaffected by the music rate. */
	float fDeltaTimeNoMusicRate = max( fDeltaTime / fRate, 0 );

	if( m_pCurrentBGA )
		m_pCurrentBGA->Update( fDeltaTimeNoMusicRate );
	if( m_pFadingBGA )
		m_pFadingBGA->Update( fDeltaTimeNoMusicRate );
}

void BackgroundImpl::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	{
		bool bVisible = IsDangerAllVisible();
		if( m_bDangerAllWasVisible != bVisible )
			MESSAGEMAN->Broadcast( bVisible? "ShowDangerAll":"HideDangerAll" );
		m_bDangerAllWasVisible = bVisible;
	}

	if( m_pDancingCharacters )
		m_pDancingCharacters->Update( fDeltaTime );

	FOREACH_BackgroundLayer( i )
	{
		Layer &layer = m_Layer[i];
		layer.UpdateCurBGChange( m_pSong, m_fLastMusicSeconds, GAMESTATE->m_Position.m_fMusicSeconds, m_mapNameToTransition );
	}
	m_fLastMusicSeconds = GAMESTATE->m_Position.m_fMusicSeconds;
}

void BackgroundImpl::DrawPrimitives()
{
	if( g_fBGBrightness == 0.0f )
		return;

	if( IsDangerAllVisible() )
	{
		// Since this only shows when DANGER is visible, it will flash red on it's own accord :)
		if( m_pDancingCharacters )
			m_pDancingCharacters->m_bDrawDangerLight = true;
	}

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
	}

	if( m_pDancingCharacters )
	{
		m_pDancingCharacters->Draw();
		DISPLAY->ClearZBuffer();
	}

	ActorFrame::DrawPrimitives();
}

void BackgroundImpl::GetLoadedBackgroundChanges( vector<BackgroundChange> *pBackgroundChangesOut[NUM_BackgroundLayer] )
{
	FOREACH_BackgroundLayer( i )
		*pBackgroundChangesOut[i] = m_Layer[i].m_aBGChanges;
}

bool BackgroundImpl::IsDangerAllVisible()
{
	// The players are never in danger in FAIL_OFF.
	FOREACH_PlayerNumber( p )
		if( GAMESTATE->GetPlayerFailType(GAMESTATE->m_pPlayerState[p]) == FailType_Off )
			return false;
	if( !g_bShowDanger )
		return false;

	/* Don't show it if everyone is already failing: it's already too late and it's
	 * annoying for it to show for the entire duration of a song. */
	if( STATSMAN->m_CurStageStats.AllFailed() )
		return false;

	return GAMESTATE->AllAreInDangerOrWorse();
}

BrightnessOverlay::BrightnessOverlay()
{
	float fQuadWidth = (RIGHT_EDGE-LEFT_EDGE)/2;
	fQuadWidth -= g_fBackgroundCenterWidth/2;

	m_quadBGBrightness[0].StretchTo( RectF(LEFT_EDGE,TOP_EDGE,LEFT_EDGE+fQuadWidth,BOTTOM_EDGE) );
	m_quadBGBrightnessFade.StretchTo( RectF(LEFT_EDGE+fQuadWidth,TOP_EDGE,RIGHT_EDGE-fQuadWidth,BOTTOM_EDGE) );
	m_quadBGBrightness[1].StretchTo( RectF(RIGHT_EDGE-fQuadWidth,TOP_EDGE,RIGHT_EDGE,BOTTOM_EDGE) );

	m_quadBGBrightness[0].SetName( "BrightnessOverlay" );
	ActorUtil::LoadAllCommands( m_quadBGBrightness[0], "Background" );
	this->AddChild( &m_quadBGBrightness[0] );

	m_quadBGBrightness[1].SetName( "BrightnessOverlay" );
	ActorUtil::LoadAllCommands( m_quadBGBrightness[1], "Background" );
	this->AddChild( &m_quadBGBrightness[1] );

	m_quadBGBrightnessFade.SetName( "BrightnessOverlay" );
	ActorUtil::LoadAllCommands( m_quadBGBrightnessFade, "Background" );
	this->AddChild( &m_quadBGBrightnessFade );

	SetActualBrightness();
}

void BrightnessOverlay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );
	/* If we're actually playing, then we're past fades, etc; update the
	 * background brightness to follow Cover. */
	if( !GAMESTATE->m_bGameplayLeadIn )
		SetActualBrightness();
}

void BrightnessOverlay::SetActualBrightness()
{
	float fLeftBrightness = 1-GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.GetCurrent().m_fCover;
	float fRightBrightness = 1-GAMESTATE->m_pPlayerState[PLAYER_2]->m_PlayerOptions.GetCurrent().m_fCover;

	float fBaseBGBrightness = g_fBGBrightness;

	// Revision:  Themes that implement a training mode should handle the
	// brightness for it.  The engine should not force the brightness for
	// anything. -Kyz
	/*
	// HACK: Always show training in full brightness
	if( GAMESTATE->m_pCurSong && GAMESTATE->m_pCurSong->IsTutorial() )
		fBaseBGBrightness = 1.0f;
	*/

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
	this->PlayCommand( "Fade" );
	SetActualBrightness();
}

Background::Background()				{ m_disable_draw= false; m_pImpl = new BackgroundImpl; this->AddChild(m_pImpl); }
Background::~Background()				{ SAFE_DELETE( m_pImpl ); }
void Background::Init()					{ m_pImpl->Init(); }
void Background::LoadFromSong( const Song *pSong )	{ m_pImpl->LoadFromSong(pSong); }
void Background::Unload()				{ m_pImpl->Unload(); }
void Background::FadeToActualBrightness()		{ m_pImpl->FadeToActualBrightness(); }
void Background::SetBrightness( float fBrightness )	{ m_pImpl->SetBrightness(fBrightness); }
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
