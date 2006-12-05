#include "global.h"
#include "ScreenOptionsMasterPrefs.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "NoteSkinManager.h"
#include "PlayerOptions.h"
#include "SongOptions.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "GameManager.h"
#include "GameState.h"
#include "StepMania.h"
#include "Game.h"
#include "Foreach.h"
#include "GameConstantsAndTypes.h"
#include "DisplayResolutions.h"
#include "LocalizedString.h"
#include "SpecialFiles.h"
#include "RageLog.h"

using namespace StringConversion;

static void GetPrefsDefaultModifiers( PlayerOptions &po, SongOptions &so )
{
	po.FromString( PREFSMAN->m_sDefaultModifiers );
	so.FromString( PREFSMAN->m_sDefaultModifiers );
}

static void SetPrefsDefaultModifiers( const PlayerOptions &po, const SongOptions &so )
{
	vector<RString> as;
	if( po.GetString() != "" )
		as.push_back( po.GetString() );
	if( so.GetString() != "" )
		as.push_back( so.GetString() );

	PREFSMAN->m_sDefaultModifiers.Set( join(", ",as) );
}

/* Ugly: the input values may be a different type than the mapping.  For example,
 * the mapping may be an enum, and value an int.  This is because we don't
 * have FromString/ToString for every enum type.  Assume that the distance between
 * T and U can be represented as a float. */
template<class T,class U>
int FindClosestEntry( T value, const U *mapping, unsigned cnt )
{
	int iBestIndex = 0;
	float best_dist = 0;
	bool have_best = false;

	for( unsigned i = 0; i < cnt; ++i )
	{
		const U val = mapping[i];
		float dist = value < val? (float)(val-value):(float)(value-val);
		if( have_best && best_dist < dist )
			continue;

		have_best = true;
		best_dist = dist;

		iBestIndex = i;
	}

	if( have_best )
		return iBestIndex;
	else
		return 0;
}

template <class T>
static void MoveMap( int &sel, T &opt, bool ToSel, const T *mapping, unsigned cnt )
{
	if( ToSel )
	{
		sel = FindClosestEntry( opt, mapping, cnt );
	} else {
		/* sel -> opt */
		opt = mapping[sel];
	}
}

template <class T>
static void MoveMap( int &sel, IPreference &opt, bool ToSel, const T *mapping, unsigned cnt )
{
	if( ToSel )
	{
		RString sOpt = opt.ToString();
		/* This should really be T, but we can't FromString an enum. */
		float val;
		FromString( sOpt, val );
		sel = FindClosestEntry( val, mapping, cnt );
	} else {
		/* sel -> opt */
		RString sOpt = ToString( mapping[sel] );
		opt.FromString( sOpt );
	}
}

template <class T>
static void MoveMap( int &sel, const ConfOption *pConfOption, bool ToSel, const T *mapping, unsigned cnt )
{
	IPreference *pPref = IPreference::GetPreferenceByName( pConfOption->m_sPrefName );
	ASSERT_M( pPref != NULL, pConfOption->m_sPrefName );

	MoveMap( sel, *pPref, ToSel, mapping, cnt );
}

template <class T>
static void MovePref( int &iSel, bool bToSel, const ConfOption *pConfOption )
{
	IPreference *pPref = IPreference::GetPreferenceByName( pConfOption->m_sPrefName );
	ASSERT_M( pPref != NULL, pConfOption->m_sPrefName );

	if( bToSel )
	{
		// TODO: why not get the int directly from pPref?  Why are we writing it to a string and then back?
		T t;
		FromString( pPref->ToString(), t );
		iSel = static_cast<int>( t );
	}
	else
	{
		pPref->FromString( ToString( static_cast<T>( iSel ) ) );
	}
}

template <>
static void MovePref<bool>( int &iSel, bool bToSel, const ConfOption *pConfOption )
{
	IPreference *pPref = IPreference::GetPreferenceByName( pConfOption->m_sPrefName );
	ASSERT_M( pPref != NULL, pConfOption->m_sPrefName );

	if( bToSel )
	{
		// TODO: why not get the int directly from pPref?  Why are we writing it to a string and then back?
		bool b;
		FromString( pPref->ToString(), b );
		iSel = b ? 1 : 0;
	}
	else
	{
		// If we don't make a specific instantiation of MovePref<bool>, there is a compile warning here because of
		// static_cast<bool>( iSel ) where iSel is an int.  What is the best way to remove that compile warning?
		pPref->FromString( ToString<bool>( iSel ? true : false ) );
	}
}

static void MoveNop( int &iSel, bool bToSel, const ConfOption *pConfOption )
{
	if( bToSel )
		iSel = 0;
}

static void GameChoices( vector<RString> &out )
{
	vector<const Game*> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	FOREACH( const Game*, aGames, g )
	{
		RString sGameName = (*g)->m_szName;
		out.push_back( sGameName );
	}
}

static void GameSel( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<RString> choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		const RString sCurGameName = GAMESTATE->m_pCurGame->m_szName;

		sel = 0;
		for(unsigned i = 0; i < choices.size(); ++i)
			if( !stricmp(choices[i], sCurGameName) )
				sel = i;
	} else {
		vector<const Game*> aGames;
		GAMEMAN->GetEnabledGames( aGames );
		StepMania::ChangeCurrentGame( aGames[sel] );
	}
}

static void LanguageChoices( vector<RString> &out )
{
	vector<RString> vs;
	THEME->GetLanguages( vs );
	SortRStringArray( vs, true );

	FOREACH_CONST( RString, vs, s )
	{
		const LanguageInfo *pLI = GetLanguageInfo( *s );
		if( pLI )
			out.push_back( pLI->szNativeName );
		else
			out.push_back( *s );
	}
}

static void Language( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<RString> vs;
	THEME->GetLanguages( vs );
	SortRStringArray( vs, true );

	if( ToSel )
	{
		sel = -1;
		for( unsigned i=0; sel == -1 && i < vs.size(); ++i )
			if( !stricmp(vs[i], THEME->GetCurLanguage()) )
				sel = i;

		/* If the current language doesn't exist, we'll show BASE_LANGUAGE, so select that. */
		for( unsigned i=0; sel == -1 && i < vs.size(); ++i )
			if( !stricmp(vs[i], SpecialFiles::BASE_LANGUAGE) )
				sel = i;

		if( sel == -1 )
		{
			LOG->Warn( "Couldn't find language \"%s\" or fallback \"%s\"; using \"%s\"",
				THEME->GetCurLanguage().c_str(), SpecialFiles::BASE_LANGUAGE.c_str(), vs[0].c_str() );
			sel = 0;
		}
	} else {
		const RString &sNewLanguage = vs[sel];
		
		PREFSMAN->m_sLanguage.Set( sNewLanguage );
		if( THEME->GetCurLanguage() != sNewLanguage )
			THEME->SwitchThemeAndLanguage( THEME->GetCurThemeName(), PREFSMAN->m_sLanguage, PREFSMAN->m_bPseudoLocalize );
	}
}

static void ThemeChoices( vector<RString> &out )
{
	THEME->GetSelectableThemeNames( out );
	FOREACH( RString, out, s )
		*s = THEME->GetThemeDisplayName( *s );
}

static void DisplayResolutionChoices( vector<RString> &out )
{
	DisplayResolutions d;
	DISPLAY->GetDisplayResolutions( d );
	
	FOREACHS_CONST( DisplayResolution, d, iter )
	{
		RString s = ssprintf("%dx%d", iter->iWidth, iter->iHeight);
		out.push_back( s );
	}
}

static void Theme( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<RString> choices;
	pConfOption->MakeOptionsList( choices );

	vector<RString> vsThemeNames;
	THEME->GetSelectableThemeNames( vsThemeNames );

	if( ToSel )
	{
		sel = 0;
		for( unsigned i=1; i<vsThemeNames.size(); i++ )
			if( !stricmp(vsThemeNames[i], PREFSMAN->m_sTheme.Get()) )
				sel = i;
	}
	else
	{
		const RString sNewTheme = vsThemeNames[sel];
		PREFSMAN->m_sTheme.Set( sNewTheme ); // OPT_APPLY_THEME will load the theme
	}
}

static LocalizedString OFF ("ScreenOptionsMasterPrefs","Off");
static void AnnouncerChoices( vector<RString> &out )
{
	ANNOUNCER->GetAnnouncerNames( out );
	out.insert( out.begin(), OFF );
}

static void Announcer( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<RString> choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		sel = 0;
		for( unsigned i=1; i<choices.size(); i++ )
			if( !stricmp(choices[i], ANNOUNCER->GetCurAnnouncerName()) )
				sel = i;
	} else {
		const RString sNewAnnouncer = sel? choices[sel]:RString("");
		ANNOUNCER->SwitchAnnouncer( sNewAnnouncer );
	}
}

static void DefaultNoteSkinChoices( vector<RString> &out )
{
	NOTESKIN->GetNoteSkinNames( out );
}

static void DefaultNoteSkin( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<RString> choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		PlayerOptions po;
		po.FromString( PREFSMAN->m_sDefaultModifiers );
		sel = 0;
		for( unsigned i=0; i < choices.size(); i++ )
			if( !stricmp(choices[i], po.m_sNoteSkin) )
				sel = i;
	} else {
		PlayerOptions po;
		SongOptions so;
		GetPrefsDefaultModifiers( po, so );
		po.m_sNoteSkin = choices[sel];
		SetPrefsDefaultModifiers( po, so );
	}
}

/* Background options */

static void BGBrightness( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	// TODO: I hate the way the list of numbers is duplicated here and where the option is created.
	// Try to find a way to only use the same list once.  Do that for all of these float and int lists.
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void BGBrightnessNoZero( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void BGBrightnessOrStatic( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.5f,0.25f,0.5f,0.75f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );

	if( ToSel && !PREFSMAN->m_bSongBackgrounds )
		sel = 0;
	if( !ToSel )
		PREFSMAN->m_bSongBackgrounds.Set( sel != 0 );
}

static void NumBackgrounds( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 5,10,15,20 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

/* Input options */
static void MusicWheelSwitchSpeed( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 5, 10, 15, 25 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

/* Gameplay options */
static void CoinModeNoHome( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	// The mapping without home is easy: subtract one to compensate for the missing COIN_MODE_HOME
	if( ToSel )
	{
		MovePref<CoinMode>( sel, ToSel, pConfOption );
		if( sel > static_cast<int>(COIN_MODE_HOME) )
			--sel;
	}
	else
	{
		if( sel >= static_cast<int>(COIN_MODE_HOME) )
			++sel;
		MovePref<CoinMode>( sel, ToSel, pConfOption );
	}
}

static void CoinsPerCredit( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void SongsPerPlay( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void SongsPerPlayOrEventMode( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5,6 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );

	if( ToSel && PREFSMAN->m_bEventMode )
		sel = 5;
	if( !ToSel )
		PREFSMAN->m_bEventMode.Set( sel == 5 );
}

/* Machine options */

static void TimingWindowScale( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 1.50f,1.33f,1.16f,1.00f,0.84f,0.66f,0.50f,0.33f,0.20f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void LifeDifficulty( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 1.60f,1.40f,1.20f,1.00f,0.80f,0.60f,0.40f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static int GetLifeDifficulty()
{
	int iLifeDifficulty = 0;
	LifeDifficulty( iLifeDifficulty, true, NULL );	
	iLifeDifficulty++;	// LifeDifficulty returns an index
	return iLifeDifficulty;
}
#include "LuaManager.h"
LuaFunction( GetLifeDifficulty, GetLifeDifficulty() );

static void DefaultFailType( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	if( ToSel )
	{
		SongOptions so;
		so.FromString( PREFSMAN->m_sDefaultModifiers );
		sel = so.m_FailType;
	}
	else
	{
		PlayerOptions po;
		SongOptions so;
		GetPrefsDefaultModifiers( po, so );

		switch( sel )
		{
		case 0:	so.m_FailType = SongOptions::FAIL_IMMEDIATE;	break;
		case 1:	so.m_FailType = SongOptions::FAIL_IMMEDIATE_CONTINUE;	break;
		case 2:	so.m_FailType = SongOptions::FAIL_AT_END;	break;
		case 3:	so.m_FailType = SongOptions::FAIL_OFF;		break;
		default:
			ASSERT(0);
		}

		SetPrefsDefaultModifiers( po, so );
	}
}

/* Graphic options */

struct res_t
{
	int w, h;
	res_t(): w(0), h(0) { }
	res_t( int w_, int h_ ): w(w_), h(h_) { }
	res_t operator-( const res_t &rhs ) const
	{
		return res_t( w-rhs.w, h-rhs.h );
	}

	bool operator<( const res_t &rhs ) const
	{
		if( w != rhs.w )
			return w < rhs.w;
		return h < rhs.h;
	}

	/* Ugly: allow convert to a float for FindClosestEntry. */
	operator float() const { return w * 5000.0f + h; }
};

static void DisplayResolutionM( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<res_t> v;
	
	DisplayResolutions d;
	DISPLAY->GetDisplayResolutions( d );
	
	FOREACHS_CONST( DisplayResolution, d, iter )
	{
		v.push_back( res_t(iter->iWidth, iter->iHeight) );
	}

	res_t sel_res( PREFSMAN->m_iDisplayWidth, PREFSMAN->m_iDisplayHeight );
	MoveMap( sel, sel_res, ToSel, &v[0], v.size() );
	if( !ToSel )
	{
		PREFSMAN->m_iDisplayWidth.Set( sel_res.w );
		PREFSMAN->m_iDisplayHeight.Set( sel_res.h );
	}
}

static void DisplayColorDepth( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void MaxTextureResolution( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 256,512,1024,2048 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void TextureColorDepth( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void MovieColorDepth( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void RefreshRate( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { (int) REFRESH_DEFAULT,60,70,72,75,80,85,90,100,120,150 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void DisplayAspectRatio( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 3/4.f,1,4/3.0f,16/10.0f,16/9.f, 8/3.f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

/* Simpler DisplayAspectRatio setting, which only offers "on" and "off".  "On" can be 16:9
 * or 16:10. */
static void WideScreen16_10( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 4/3.0f, 16/10.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void WideScreen16_9( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 4/3.0f, 16/9.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

/* Sound options */

static void SoundVolume( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static void GlobalOffsetSeconds( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	float mapping[41];
	for( int i = 0; i < 41; ++i )
		mapping[i] = SCALE( i, 0.0f, 40.0f, -0.1f, +0.1f );
	
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYLEN(mapping) );
}

static vector<ConfOption> g_ConfOptions;
static void InitializeConfOptions()
{
	if( !g_ConfOptions.empty() )
		return;

	// There are a couple ways of getting the current preference column or turning a new choice in the interface 
	// into a new preference.  The easiest is when the interface choices are an exact mapping to the values the 
	// preference can be.  In that case, the easiest thing to do is use MovePref<bool or enum>.  The next
	// easiest case is when there is a hardcoded mapping that is not 1-1, such as CoinModeNoHome.  In that case,
	// you need to remap the result of MovePref<enum> to the correct mapping.  Harder yet is when there is a
	// float or a dynamic set of options, such as Language or Theme.  Those require individual attention.
#define ADD(x) g_ConfOptions.push_back( x )
	/* Select game */
	ADD( ConfOption( "Game",			GameSel,		GameChoices ) );
	g_ConfOptions.back().m_iEffects = OPT_RESET_GAME;

	/* Appearance options */
	ADD( ConfOption( "Language",			Language,		LanguageChoices ) );
	ADD( ConfOption( "Theme",			Theme,			ThemeChoices ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_THEME;

	ADD( ConfOption( "Announcer",			Announcer,		AnnouncerChoices ) );
	ADD( ConfOption( "DefaultNoteSkin",		DefaultNoteSkin,	DefaultNoteSkinChoices ) );
	ADD( ConfOption( "ShowInstructions",		MovePref<bool>,		"Skip","Show") );
	ADD( ConfOption( "ShowCaution",			MovePref<bool>,		"Skip","Show") );
	ADD( ConfOption( "DancePointsForOni",		MovePref<bool>,		"Percent","Dance Points") );
	ADD( ConfOption( "ShowSelectGroup",		MovePref<bool>,		"All Music","Choose") );
	ADD( ConfOption( "MusicWheelUsesSections",	MovePref<MusicWheelUsesSections>, "Never","Always","Title Only") );
	ADD( ConfOption( "CourseSortOrder",		MovePref<CourseSortOrders>, "Num Songs","Average Feet","Total Feet","Ranking") );
	ADD( ConfOption( "MoveRandomToEnd",		MovePref<bool>,		"No","Yes") );
	ADD( ConfOption( "ShowNativeLanguage",		MovePref<bool>,		"Romanization","Native Language") );
	ADD( ConfOption( "ShowLyrics",			MovePref<bool>,		"Hide","Show") );

	/* Misc options */
	ADD( ConfOption( "AutogenSteps",		MovePref<bool>,		"Off","On" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_SONG;

	ADD( ConfOption( "AutogenGroupCourses",		MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "FastLoad",			MovePref<bool>,		"Off","On" ) );

	/* Background options */
	ADD( ConfOption( "RandomBackgroundMode",	MovePref<RandomBackgroundMode>, "Off","Animations","Random Movies" ) );
	ADD( ConfOption( "BGBrightness",		BGBrightness,		"|0%","|10%","|20%","|30%","|40%","|50%","|60%","|70%","|80%","|90%","|100%" ) );
	ADD( ConfOption( "BGBrightnessNoZero",		BGBrightnessNoZero,	"|10%","|20%","|30%","|40%","|50%","|60%","|70%","|80%","|90%","|100%" ) );
	g_ConfOptions.back().m_sPrefName = "BGBrightness";
	ADD( ConfOption( "BGBrightnessOrStatic",	BGBrightnessOrStatic,	"Disabled","25% Bright","50% Bright","75% Bright" ) );
	g_ConfOptions.back().m_sPrefName = "BGBrightness";

	ADD( ConfOption( "ShowDanger",			MovePref<bool>,		"Hide","Show" ) );
	ADD( ConfOption( "ShowDancingCharacters",	MovePref<ShowDancingCharacters>, "Default to Off","Default to Random","Select" ) );
	ADD( ConfOption( "ShowBeginnerHelper",		MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "NumBackgrounds",		NumBackgrounds,		"|5","|10","|15","|20" ) );

	/* Input options */
	ADD( ConfOption( "AutoMapOnJoyChange",		MovePref<bool>,		"Off","On (recommended)" ) );
	ADD( ConfOption( "OnlyDedicatedMenuButtons",	MovePref<bool>,		"Use Gameplay Buttons","Only Dedicated Buttons" ) );
	ADD( ConfOption( "AutoPlay",			MovePref<PlayerController>, "Off","On","CPU-Controlled" ) );
	ADD( ConfOption( "DelayedBack",			MovePref<bool>,		"Instant","Hold" ) );
	ADD( ConfOption( "ArcadeOptionsNavigation",	MovePref<bool>,		"StepMania Style","Arcade Style" ) );
	ADD( ConfOption( "MusicWheelSwitchSpeed",	MusicWheelSwitchSpeed,	"Slow","Normal","Fast","Really Fast" ) );

	/* Gameplay options */
	ADD( ConfOption( "Center1Player",		MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "HiddenSongs",			MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "EasterEggs",			MovePref<bool>,		"Off","On" ) );
	// W1 is Fantastic Timing
	ADD( ConfOption( "AllowW1",			MovePref<AllowW1>,	"Never","Courses Only","Always" ) );
	ADD( ConfOption( "AllowExtraStage",		MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "PickExtraStage",		MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "UseUnlockSystem",		MovePref<bool>,		"Off","On" ) );

	/* Machine options */
	ADD( ConfOption( "MenuTimer",			MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "CoinMode",			MovePref<CoinMode>,	"Home","Pay","Free Play" ) );
	ADD( ConfOption( "CoinModeNoHome",		CoinModeNoHome,		"Pay","Free Play" ) );
	g_ConfOptions.back().m_sPrefName = "CoinMode";

	ADD( ConfOption( "SongsPerPlay",		SongsPerPlay,		"|1","|2","|3","|4","|5" ) );
	ADD( ConfOption( "SongsPerPlayOrEvent",		SongsPerPlayOrEventMode,"|1","|2","|3","|4","|5","Event" ) );
	g_ConfOptions.back().m_sPrefName = "SongsPerPlay";

	ADD( ConfOption( "EventMode",			MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "ScoringType",			MovePref<ScoringType>,	"New","Old" ) );
	ADD( ConfOption( "TimingWindowScale",		TimingWindowScale,	"|1","|2","|3","|4","|5","|6","|7","|8","Justice" ) );
	ADD( ConfOption( "LifeDifficulty",		LifeDifficulty,		"|1","|2","|3","|4","|5","|6","|7" ) );
	g_ConfOptions.back().m_sPrefName = "LifeDifficultyScale";
	ADD( ConfOption( "ProgressiveLifebar",		MovePref<int>,		"Off","|1","|2","|3","|4","|5","|6","|7","|8") );
	ADD( ConfOption( "ProgressiveStageLifebar",	MovePref<int>,		"Off","|1","|2","|3","|4","|5","|6","|7","|8","Insanity") );
	ADD( ConfOption( "ProgressiveNonstopLifebar",	MovePref<int>,		"Off","|1","|2","|3","|4","|5","|6","|7","|8","Insanity") );
	ADD( ConfOption( "DefaultFailType",		DefaultFailType,	"Immediate","ImmediateContinue","End of Song","Off" ) );	
	ADD( ConfOption( "CoinsPerCredit",		CoinsPerCredit,		"|1","|2","|3","|4","|5","|6","|7","|8","|9","|10","|11","|12","|13","|14","|15","|16" ) );
	ADD( ConfOption( "Premium",			MovePref<Premium>,	"Off","Double for 1 Credit","Joint Premium" ) );
	ADD( ConfOption( "ShowSongOptions",		MovePref<Maybe>,	"Ask", "Hide","Show" ) );
	ADD( ConfOption( "GetRankingName",		MovePref<GetRankingName>, "Off", "On", "Ranking Songs" ) );

	/* Graphic options */
	ADD( ConfOption( "Windowed",			MovePref<bool>,		"Full Screen", "Windowed" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "DisplayResolution",		DisplayResolutionM, DisplayResolutionChoices ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS | OPT_APPLY_ASPECT_RATIO;
	ADD( ConfOption( "DisplayAspectRatio",		DisplayAspectRatio,	"|3:4","|1:1","|4:3","|16:10","|16:9","|8:3" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_ASPECT_RATIO;
	ADD( ConfOption( "WideScreen16_10",		WideScreen16_10,	"Off", "On" ) );
	g_ConfOptions.back().m_sPrefName = "DisplayAspectRatio";
	g_ConfOptions.back().m_iEffects = OPT_APPLY_ASPECT_RATIO;
	ADD( ConfOption( "WideScreen16_9",		WideScreen16_9,		"Off", "On" ) );
	g_ConfOptions.back().m_sPrefName = "DisplayAspectRatio";
	g_ConfOptions.back().m_iEffects = OPT_APPLY_ASPECT_RATIO;
	ADD( ConfOption( "DisplayColorDepth",		DisplayColorDepth,	"16bit","32bit" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "MaxTextureResolution",	MaxTextureResolution,	"|256","|512","|1024","|2048" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "TextureColorDepth",		TextureColorDepth,	"16bit","32bit" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "MovieColorDepth",		MovieColorDepth,	"16bit","32bit" ) );
	ADD( ConfOption( "DelayedTextureDelete",	MovePref<bool>,		"Off","On" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "CelShadeModels",		MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "SmoothLines",			MovePref<bool>,		"Off","On" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "RefreshRate",			RefreshRate,		"Default","|60","|70","|72","|75","|80","|85","|90","|100","|120","|150" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "Vsync",			MovePref<bool>,		"No", "Yes" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "ShowStats",			MovePref<bool>,		"Off","On" ) );
	ADD( ConfOption( "ShowBanners",			MovePref<bool>,		"Off","On" ) );

	/* Sound options */
	ADD( ConfOption( "AttractSoundFrequency",	MovePref<AttractSoundFrequency>, "Never","Always","2 Times","3 Times","4 Times","5 Times" ) );
	ADD( ConfOption( "SoundVolume",			SoundVolume,		"Silent","|10%","|20%","|30%","|40%","|50%","|60%","|70%","|80%","|90%","|100%" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_SOUND;
	{
		ConfOption c( "GlobalOffsetSeconds",		GlobalOffsetSeconds );
		for( int i = -100; i <= +100; i += 5 )
			c.AddOption( ssprintf("%+i ms", i) );
		ADD( c );
	}

	/* Editor options */
	ADD( ConfOption( "EditorShowBGChangesPlay",	MovePref<bool>,		"Hide","Show") );

	ADD( ConfOption( "Invalid",			MoveNop,		"|Invalid option") );
}

/* Get a mask of effects to apply if the given option changes. */
int ConfOption::GetEffects() const
{
	return m_iEffects | OPT_SAVE_PREFERENCES;
}

ConfOption *ConfOption::Find( RString name )
{
	InitializeConfOptions();
	for( unsigned i = 0; i < g_ConfOptions.size(); ++i )
	{
		ConfOption *opt = &g_ConfOptions[i];
		RString match(opt->name);
		if( match.CompareNoCase(name) )
			continue;
		return opt;
	}

	return NULL;
}

void ConfOption::UpdateAvailableOptions()
{
	if( MakeOptionsListCB != NULL )
	{
		names.clear();
		MakeOptionsListCB( names );
	}
}

void ConfOption::MakeOptionsList( vector<RString> &out ) const
{
	out = names;
}

/*
 * (c) 2003-2004 Glenn Maynard
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
