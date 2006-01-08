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

static void GetPrefsDefaultModifiers( PlayerOptions &po, SongOptions &so )
{
	po.FromString( PREFSMAN->m_sDefaultModifiers );
	so.FromString( PREFSMAN->m_sDefaultModifiers );
}

static void SetPrefsDefaultModifiers( const PlayerOptions &po, const SongOptions &so )
{
	vector<CString> as;
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
		CString sOpt = opt.ToString();
		/* This should really be T, but we can't FromString an enum. */
		float val;
		FromString( sOpt, val );
		sel = FindClosestEntry( val, mapping, cnt );
	} else {
		/* sel -> opt */
		CString sOpt = ToString( mapping[sel] );
		opt.FromString( sOpt );
	}
}

template <class T>
static void MoveMap( int &sel, const ConfOption *pConfOption, bool ToSel, const T *mapping, unsigned cnt )
{
	IPreference *pPref = IPreference::GetPreferenceByName( pConfOption->name );
	ASSERT_M( pPref != NULL, pConfOption->name );

	MoveMap( sel, *pPref, ToSel, mapping, cnt );
}

/* "iSel" is the selection in the menu. */
static void MovePref( int &iSel, bool bToSel, const ConfOption *pConfOption )
{
	IPreference *pPref = IPreference::GetPreferenceByName( pConfOption->name );
	ASSERT_M( pPref != NULL, pConfOption->name );

	if( bToSel )
	{
		CString sOpt = pPref->ToString();
		FromString( sOpt, iSel );
	}
	else
	{
		CString sOpt = ToString( iSel );
		pPref->FromString( sOpt );
	}
}

static void GameChoices( vector<CString> &out )
{
	vector<const Game*> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	FOREACH( const Game*, aGames, g )
	{
		CString sGameName = (*g)->m_szName;
		out.push_back( sGameName );
	}
}

static void GameSel( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<CString> choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		const CString sCurGameName = GAMESTATE->m_pCurGame->m_szName;

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

static void LanguageChoices( vector<CString> &out )
{
	THEME->GetLanguages( out );
}

static void Language( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<CString> choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		sel = 0;
		for( unsigned i=1; i<choices.size(); i++ )
			if( !stricmp(choices[i], THEME->GetCurLanguage()) )
				sel = i;
	} else {
		const CString sNewLanguage = choices[sel];
		
		if( THEME->GetCurLanguage() != sNewLanguage )
			THEME->SwitchThemeAndLanguage( THEME->GetCurThemeName(), sNewLanguage, PREFSMAN->m_bPseudoLocalize );
	}
}

static void ThemeChoices( vector<CString> &out )
{
	THEME->GetThemeNames( out );
}

static void DisplayResolutionChoices( vector<CString> &out )
{
	DisplayResolutions d;
	DISPLAY->GetDisplayResolutions( d );
	
	FOREACHS_CONST( DisplayResolution, d.s, iter )
	{
		CString s = ssprintf("%dx%d", iter->iWidth, iter->iHeight);
		out.push_back( s );
	}
}

static void Theme( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<CString> choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		sel = 0;
		for( unsigned i=1; i<choices.size(); i++ )
			if( !stricmp(choices[i], THEME->GetCurThemeName()) )
				sel = i;
	} else {
		const CString sNewTheme = choices[sel];
		if( THEME->GetCurThemeName() != sNewTheme )
			THEME->SwitchThemeAndLanguage( sNewTheme, THEME->GetCurLanguage(), PREFSMAN->m_bPseudoLocalize );
	}
}

static LocalizedString OFF ("ScreenOptionsMasterPrefs","Off");
static void AnnouncerChoices( vector<CString> &out )
{
	ANNOUNCER->GetAnnouncerNames( out );
	out.insert( out.begin(), OFF );
}

static void Announcer( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<CString> choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		sel = 0;
		for( unsigned i=1; i<choices.size(); i++ )
			if( !stricmp(choices[i], ANNOUNCER->GetCurAnnouncerName()) )
				sel = i;
	} else {
		const CString sNewAnnouncer = sel? choices[sel]:CString("");
		ANNOUNCER->SwitchAnnouncer( sNewAnnouncer );
	}
}

static void DefaultNoteSkinChoices( vector<CString> &out )
{
	NOTESKIN->GetNoteSkinNames( out );
}

static void DefaultNoteSkin( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	vector<CString> choices;
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

/* Appearance options */

static void WheelSections( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const PrefsManager::MusicWheelUsesSections mapping[] = { PrefsManager::NEVER, PrefsManager::ALWAYS, PrefsManager::ABC_ONLY };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Background options */

static void BGBrightness( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void BGBrightnessNoZero( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void BGBrightnessOrStatic( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.4f,0.4f,0.7f };
	MoveMap( sel, PREFSMAN->m_fBGBrightness, ToSel, mapping, ARRAYSIZE(mapping) );

	if( ToSel && !PREFSMAN->m_bSongBackgrounds )
		sel = 0;
	if( !ToSel )
		PREFSMAN->m_bSongBackgrounds.Set( sel != 0 );
}

static void NumBackgrounds( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 5,10,15,20 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Input options */
static void MusicWheelSwitchSpeed( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 5, 10, 15, 25 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Gameplay options */
static void AllowW1( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const PrefsManager::AllowW1 mapping[] = { PrefsManager::ALLOW_W1_NEVER, PrefsManager::ALLOW_W1_COURSES_ONLY, PrefsManager::ALLOW_W1_EVERYWHERE };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void CoinModeM( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const CoinMode mapping[] = { COIN_MODE_HOME, COIN_MODE_PAY, COIN_MODE_FREE };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void CoinModeNoHome( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const CoinMode mapping[] = { COIN_MODE_PAY, COIN_MODE_FREE };
	MoveMap( sel, PREFSMAN->m_CoinMode, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void CoinsPerCredit( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void PremiumM( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const Premium mapping[] = { PREMIUM_NONE, PREMIUM_DOUBLE, PREMIUM_JOINT };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void SongsPerPlay( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void SongsPerPlayOrEventMode( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5,6 };
	MoveMap( sel, PREFSMAN->m_iSongsPerPlay, ToSel, mapping, ARRAYSIZE(mapping) );

	if( ToSel && PREFSMAN->m_bEventMode )
		sel = 5;
	if( !ToSel )
		PREFSMAN->m_bEventMode.Set( sel == 5 );
}

/* Machine options */

static void TimingWindowScale( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 1.50f,1.33f,1.16f,1.00f,0.84f,0.66f,0.50f,0.33f,0.20f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void LifeDifficulty( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 1.60f,1.40f,1.20f,1.00f,0.80f,0.60f,0.40f };
	MoveMap( sel, PREFSMAN->m_fLifeDifficultyScale, ToSel, mapping, ARRAYSIZE(mapping) );
}

static int GetLifeDifficulty()
{
	int iLifeDifficulty = 0;
	LifeDifficulty( iLifeDifficulty, true, NULL );	
	iLifeDifficulty++;	// LifeDifficulty returns an index
	return iLifeDifficulty;
}
#include "LuaFunctions.h"
LuaFunction( GetLifeDifficulty, GetLifeDifficulty() );

static void ShowSongOptions( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const PrefsManager::Maybe mapping[] = { PrefsManager::NO,PrefsManager::YES,PrefsManager::ASK };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void GetRankingName( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const PrefsManager::GetRankingName mapping[] = { PrefsManager::RANKING_OFF, PrefsManager::RANKING_ON, PrefsManager::RANKING_LIST };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

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
		case 0:	so.m_FailType = SongOptions::FAIL_IMMEDIATE;			break;
		case 1:	so.m_FailType = SongOptions::FAIL_END_OF_SONG;			break;
		case 2:	so.m_FailType = SongOptions::FAIL_OFF;					break;
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
	
	FOREACHS_CONST( DisplayResolution, d.s, iter )
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
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void MaxTextureResolution( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 256,512,1024,2048 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void TextureColorDepth( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void MovieColorDepth( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void RefreshRate( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { (int) REFRESH_DEFAULT,60,70,72,75,80,85,90,100,120,150 };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void DisplayAspectRatio( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { -1,3/4.f,1,4/3.0f,16/10.0f,16/9.f, 8/3.f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Sound options */

static void SoundVolume( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void GlobalOffsetSeconds( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	float mapping[41];
	for( int i = 0; i < 41; ++i )
		mapping[i] = SCALE( i, 0.0f, 40.0f, -0.1f, +0.1f );
	
	MoveMap( sel, pConfOption, ToSel, mapping, ARRAYSIZE(mapping) );
}

static vector<ConfOption> g_ConfOptions;
static void InitializeConfOptions()
{
	if( !g_ConfOptions.empty() )
		return;

#define ADD(x) g_ConfOptions.push_back( x )
	/* Select game */
	ADD( ConfOption( "Game",						GameSel, GameChoices ) );
	g_ConfOptions.back().m_iEffects = OPT_RESET_GAME;

	/* Appearance options */
	ADD( ConfOption( "Language",					Language, LanguageChoices ) );
	ADD( ConfOption( "Theme",						Theme, ThemeChoices ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_THEME;

	ADD( ConfOption( "Announcer",					Announcer, AnnouncerChoices ) );
	ADD( ConfOption( "DefaultNoteSkin",				DefaultNoteSkin, DefaultNoteSkinChoices ) );
	ADD( ConfOption( "ShowInstructions",			MovePref,			"Skip","Show") );
	ADD( ConfOption( "ShowCaution",					MovePref,			"Skip","Show") );
	ADD( ConfOption( "DancePointsForOni",			MovePref,			"Percent","Dance Points") );
	ADD( ConfOption( "ShowSelectGroup",				MovePref,			"All Music","Choose") );
	ADD( ConfOption( "MusicWheelUsesSections",		WheelSections,		"Never","Always","Title Only") );
	ADD( ConfOption( "CourseSortOrder",				MovePref,			"Num Songs","Average Feet","Total Feet","Ranking") );
	ADD( ConfOption( "MoveRandomToEnd",				MovePref,			"No","Yes") );
	ADD( ConfOption( "ShowNativeLanguage",			MovePref,			"Romanization","Native Language") );
	ADD( ConfOption( "ShowLyrics",					MovePref,			"Hide","Show") );

	/* Misc options */
	ADD( ConfOption( "AutogenSteps",				MovePref,			"Off","On" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_SONG;

	ADD( ConfOption( "AutogenGroupCourses",			MovePref,			"Off","On" ) );
	ADD( ConfOption( "FastLoad",					MovePref,			"Off","On" ) );

	/* Background options */
	ADD( ConfOption( "RandomBackgroundMode",		MovePref,			"Off","Animations","Random Movies" ) );
	ADD( ConfOption( "BGBrightness",				BGBrightness,		"|0%","|10%","|20%","|30%","|40%","|50%","|60%","|70%","|80%","|90%","|100%" ) );
	ADD( ConfOption( "BGBrightnessNoZero",			BGBrightnessNoZero,	"|10%","|20%","|30%","|40%","|50%","|60%","|70%","|80%","|90%","|100%" ) );
	ADD( ConfOption( "BGBrightnessOrStatic",		BGBrightnessOrStatic,"Disabled","Dim","Bright" ) );
	ADD( ConfOption( "ShowDanger",					MovePref,			"Hide","Show" ) );
	ADD( ConfOption( "ShowDancingCharacters",		MovePref,			"Default to Off","Default to Random","Select" ) );
	ADD( ConfOption( "ShowBeginnerHelper",			MovePref,			"Off","On" ) );
	ADD( ConfOption( "NumBackgrounds",				NumBackgrounds,		"|5","|10","|15","|20" ) );

	/* Input options */
	ADD( ConfOption( "AutoMapOnJoyChange",			MovePref,			"Off","On (recommended)" ) );
	ADD( ConfOption( "OnlyDedicatedMenuButtons",	MovePref,			"Use Gameplay Buttons","Only Dedicated Buttons" ) );
	ADD( ConfOption( "AutoPlay",					MovePref,			"Off","On","CPU-Controlled" ) );
	ADD( ConfOption( "DelayedBack",					MovePref,			"Instant","Hold" ) );
	ADD( ConfOption( "ArcadeOptionsNavigation",		MovePref,			"StepMania Style","Arcade Style" ) );
	ADD( ConfOption( "MusicWheelSwitchSpeed",		MusicWheelSwitchSpeed, "Slow","Normal","Fast","Really Fast" ) );

	/* Gameplay options */
	ADD( ConfOption( "SoloSingle",					MovePref,			"Off","On" ) );
	ADD( ConfOption( "HiddenSongs",					MovePref,			"Off","On" ) );
	ADD( ConfOption( "EasterEggs",					MovePref,			"Off","On" ) );
	ADD( ConfOption( "AllowW1",						AllowW1,			"Never","Courses Only","Always" ) );
	ADD( ConfOption( "AllowExtraStage",				MovePref,			"Off","On" ) );
	ADD( ConfOption( "PickExtraStage",				MovePref,			"Off","On" ) );
	ADD( ConfOption( "UseUnlockSystem",				MovePref,			"Off","On" ) );

	/* Machine options */
	ADD( ConfOption( "MenuTimer",					MovePref,			"Off","On" ) );
	ADD( ConfOption( "CoinMode",					CoinModeM,			"Home","Pay","Free Play" ) );
	ADD( ConfOption( "CoinModeNoHome",				CoinModeNoHome,		"Pay","Free Play" ) );
	ADD( ConfOption( "SongsPerPlay",				SongsPerPlay,		"|1","|2","|3","|4","|5" ) );
	ADD( ConfOption( "SongsPerPlayOrEvent",			SongsPerPlayOrEventMode, "|1","|2","|3","|4","|5","Event" ) );
	ADD( ConfOption( "EventMode",					MovePref,			"Off","On" ) );
	ADD( ConfOption( "ScoringType",					MovePref,			"New","Old" ) );
	ADD( ConfOption( "TimingWindowScale",			TimingWindowScale,	"|1","|2","|3","|4","|5","|6","|7","|8","Justice" ) );
	ADD( ConfOption( "LifeDifficulty",				LifeDifficulty,		"|1","|2","|3","|4","|5","|6","|7" ) );
	ADD( ConfOption( "ProgressiveLifebar",			MovePref,			"Off","|1","|2","|3","|4","|5","|6","|7","|8") );
	ADD( ConfOption( "ProgressiveStageLifebar",		MovePref,			"Off","|1","|2","|3","|4","|5","|6","|7","|8","Insanity") );
	ADD( ConfOption( "ProgressiveNonstopLifebar",	MovePref,			"Off","|1","|2","|3","|4","|5","|6","|7","|8","Insanity") );
	ADD( ConfOption( "DefaultFailType",				DefaultFailType,	"Immediate","End of Song","Off" ) );	
	ADD( ConfOption( "DefaultFailTypeNoOff",		DefaultFailType,	"Immediate","End of Song" ) );	
	ADD( ConfOption( "CoinsPerCredit",				CoinsPerCredit,		"|1","|2","|3","|4","|5","|6","|7","|8","|9","|10","|11","|12","|13","|14","|15","|16" ) );
	ADD( ConfOption( "Premium",						PremiumM,			"Off","Double for 1 Credit","Joint Premium" ) );
	ADD( ConfOption( "ShowSongOptions",				ShowSongOptions,	"Hide","Show","Ask" ) );
	ADD( ConfOption( "GetRankingName",				GetRankingName,		"Off", "On", "Ranking Songs" ) );

	/* Graphic options */
	ADD( ConfOption( "Windowed",					MovePref,			"Full Screen", "Windowed" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "DisplayResolution",			DisplayResolutionM, DisplayResolutionChoices ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS | OPT_APPLY_ASPECT_RATIO;
	ADD( ConfOption( "DisplayAspectRatio",			DisplayAspectRatio,	"Auto","|3:4","|1:1","|4:3","|16:10","|16:9","|8:3" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_ASPECT_RATIO;
	ADD( ConfOption( "DisplayColorDepth",			DisplayColorDepth,	"16bit","32bit" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "MaxTextureResolution",		MaxTextureResolution,"|256","|512","|1024","|2048" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "TextureColorDepth",			TextureColorDepth,	"16bit","32bit" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "MovieColorDepth",				MovieColorDepth,	"16bit","32bit" ) );
	ADD( ConfOption( "DelayedTextureDelete",		MovePref,			"Off","On" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "CelShadeModels",				MovePref,			"Off","On" ) );
	ADD( ConfOption( "SmoothLines",					MovePref,			"Off","On" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "RefreshRate",					RefreshRate,		"Default","|60","|70","|72","|75","|80","|85","|90","|100","|120","|150" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "Vsync",						MovePref,			"No", "Yes" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "ShowStats",					MovePref,			"Off","On" ) );
	ADD( ConfOption( "ShowBanners",					MovePref,			"Off","On" ) );

	/* Sound options */
	ADD( ConfOption( "SoundResampleQuality",		MovePref,			"Fast","Normal","High Quality" ) );
	ADD( ConfOption( "AttractSoundFrequency",		MovePref,			"Never","Always","2 Times","3 Times","4 Times","5 Times" ) );
	ADD( ConfOption( "SoundVolume",					SoundVolume,		"Silent","|10%","|20%","|30%","|40%","|50%","|60%","|70%","|80%","|90%","|100%" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_SOUND;
	{
		ConfOption c( "GlobalOffsetSeconds",		GlobalOffsetSeconds );
		for( int i = -100; i <= +100; i += 5 )
			c.AddOption( ssprintf("%+i ms", i) );
		ADD( c );
	}

	/* Editor options */
	ADD( ConfOption( "EditorShowBGChangesPlay",		MovePref,			"Hide","Show") );
}

/* Get a mask of effects to apply if the given option changes. */
int ConfOption::GetEffects() const
{
	return m_iEffects | OPT_SAVE_PREFERENCES;
}

ConfOption *ConfOption::Find( CString name )
{
	InitializeConfOptions();
	for( unsigned i = 0; i < g_ConfOptions.size(); ++i )
	{
		ConfOption *opt = &g_ConfOptions[i];
		CString match(opt->name);
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

void ConfOption::MakeOptionsList( vector<CString> &out ) const
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
