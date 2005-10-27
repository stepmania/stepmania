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
#include "InputMapper.h"
#include "StepMania.h"
#include "Game.h"
#include "Foreach.h"
#include "GameConstantsAndTypes.h"

static void GetPrefsDefaultModifiers( PlayerOptions &po, SongOptions &so )
{
	po.FromString( PREFSMAN->GetCurrentGamePrefs().m_sDefaultModifiers );
	so.FromString( PREFSMAN->GetCurrentGamePrefs().m_sDefaultModifiers );
}

static void SetPrefsDefaultModifiers( const PlayerOptions &po, const SongOptions &so )
{
	CStringArray as;
	if( po.GetString() != "" )
		as.push_back( po.GetString() );
	if( so.GetString() != "" )
		as.push_back( so.GetString() );

	PREFSMAN->GetCurrentGamePrefs().m_sDefaultModifiers = join( ", ",as );
}

template<class T>
int FindClosestEntry( T value, const T *mapping, unsigned cnt )
{
	int iBestIndex = 0;
	T best_dist = T();
	bool have_best = false;

	for( unsigned i = 0; i < cnt; ++i )
	{
		const T val = mapping[i];
		T dist = value < val? (T)(val-value):(T)(value-val);
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
static void MoveMap( int &sel, Preference<T> &opt, bool ToSel, const T *mapping, unsigned cnt )
{
	if( ToSel )
	{
		sel = FindClosestEntry( opt.Get(), mapping, cnt );
	} else {
		/* sel -> opt */
		opt.Set( mapping[sel] );
	}
}

static void MovePref( int &iSel, bool bToSel, const ConfOption *pConfOption )
{
	IPreference *pPref = PREFSMAN->GetPreferenceByName( pConfOption->name );
	ASSERT_M( pPref != NULL, pConfOption->name );

	if( bToSel )
	{
		const CString sVal = pPref->ToString();
		iSel = atoi( sVal );
	}
	else
	{
		const CString sVal = ToString(iSel);
		pPref->FromString( sVal );
	}
}

/* "sel" is the selection in the menu. */
template<class T>
static void MoveData( int &sel, Preference<T> &opt, bool ToSel )
{
	if( ToSel )	sel = opt;
	else		opt.Set( (T)sel );
}

static void MoveData( int &sel, Preference<bool> &opt, bool ToSel )
{
	if( ToSel )	sel = opt;
	else		opt.Set( !!sel );
}

#define MOVE( name, opt ) \
	static void name( int &sel, bool ToSel, const ConfOption *pConfOption ) \
	{ \
		MoveData( sel, opt, ToSel ); \
	}


static void GameChoices( CStringArray &out )
{
	vector<const Game*> aGames;
	GAMEMAN->GetEnabledGames( aGames );
	FOREACH( const Game*, aGames, g )
	{
		CString sGameName = (*g)->m_szName;
		sGameName.MakeUpper();
		out.push_back( sGameName );
	}
}

static void GameSel( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	CStringArray choices;
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
		ChangeCurrentGame( aGames[sel] );
	}
}

static void LanguageChoices( CStringArray &out )
{
	THEME->GetLanguages( out );
}

static void Language( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	CStringArray choices;
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
			THEME->SwitchThemeAndLanguage( THEME->GetCurThemeName(), sNewLanguage );
	}
}

static void ThemeChoices( CStringArray &out )
{
	THEME->GetThemeNames( out );
}

static void Theme( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	CStringArray choices;
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
			THEME->SwitchThemeAndLanguage( sNewTheme, THEME->GetCurLanguage() );
	}
}

static void AnnouncerChoices( CStringArray &out )
{
	ANNOUNCER->GetAnnouncerNames( out );
	out.insert( out.begin(), "OFF" );
}

static void Announcer( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	CStringArray choices;
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

static void DefaultNoteSkinChoices( CStringArray &out )
{
	NOTESKIN->GetNoteSkinNames( out );
	for( unsigned i = 0; i < out.size(); ++i )
		out[i].MakeUpper();
}

static void DefaultNoteSkin( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	CStringArray choices;
	pConfOption->MakeOptionsList( choices );

	if( ToSel )
	{
		PlayerOptions po;
		po.FromString( PREFSMAN->GetCurrentGamePrefs().m_sDefaultModifiers );
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
MOVE( Instructions,			PREFSMAN->m_bShowInstructions );
MOVE( OniScoreDisplay,		PREFSMAN->m_bDancePointsForOni );
MOVE( SongGroup,			PREFSMAN->m_bShowSelectGroup );

static void WheelSections( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	// XXX: NEVER, ALWAYS, etc. seem kinda ambiguous...
	const PrefsManager::MusicWheelUsesSections mapping[] = { PrefsManager::NEVER, PrefsManager::ALWAYS, PrefsManager::ABC_ONLY };
	MoveMap( sel, PREFSMAN->m_MusicWheelUsesSections, ToSel, mapping, ARRAYSIZE(mapping) );
}

MOVE( CourseSort,			PREFSMAN->m_CourseSortOrder );
MOVE( RandomAtEnd,			PREFSMAN->m_bMoveRandomToEnd );
MOVE( Translations,			PREFSMAN->m_bShowNativeLanguage );
MOVE( Lyrics,				PREFSMAN->m_bShowLyrics );

/* Misc. options */
MOVE( AutogenSteps,			PREFSMAN->m_bAutogenSteps );
MOVE( AutogenGroupCourses,	PREFSMAN->m_bAutogenGroupCourses );

/* Background options */
MOVE( DancingCharacters,	PREFSMAN->m_ShowDancingCharacters );
MOVE( BeginnerHelper,		PREFSMAN->m_bShowBeginnerHelper );

static void BGBrightness( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, PREFSMAN->m_fBGBrightness, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void NumBackgrounds( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 5,10,15,20 };
	MoveMap( sel, PREFSMAN->m_iNumBackgrounds, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Input options */
MOVE( AutoMapOnJoyChange,	PREFSMAN->m_bAutoMapOnJoyChange );
MOVE( AutoPlay,				PREFSMAN->m_AutoPlay );
MOVE( BackDelayed,			PREFSMAN->m_bDelayedBack );
MOVE( OptionsNavigation,	PREFSMAN->m_bArcadeOptionsNavigation );

static void WheelSpeed( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 5, 10, 15, 25 };
	MoveMap( sel, PREFSMAN->m_iMusicWheelSwitchSpeed, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Gameplay options */
MOVE( SoloSingles,			PREFSMAN->m_bSoloSingle );
MOVE( EasterEggs,			PREFSMAN->m_bEasterEggs );
MOVE( AllowExtraStage,		PREFSMAN->m_bAllowExtraStage );
MOVE( PickExtraStage,		PREFSMAN->m_bPickExtraStage );
MOVE( UnlockSystem,			PREFSMAN->m_bUseUnlockSystem );

static void AllowW1( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const PrefsManager::AllowW1 mapping[] = { PrefsManager::ALLOW_W1_NEVER, PrefsManager::ALLOW_W1_COURSES_ONLY, PrefsManager::ALLOW_W1_EVERYWHERE };
	MoveMap( sel, PREFSMAN->m_AllowW1, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void CoinModeM( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const CoinMode mapping[] = { COIN_MODE_HOME, COIN_MODE_PAY, COIN_MODE_FREE };
	MoveMap( sel, PREFSMAN->m_CoinMode, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void CoinModeNoHome( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const CoinMode mapping[] = { COIN_MODE_PAY, COIN_MODE_FREE };
	MoveMap( sel, PREFSMAN->m_CoinMode, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void CoinsPerCredit( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	MoveMap( sel, PREFSMAN->m_iCoinsPerCredit, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void PremiumM( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const Premium mapping[] = { PREMIUM_NONE, PREMIUM_DOUBLE, PREMIUM_JOINT };
	MoveMap( sel, PREFSMAN->m_Premium, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void SongsPerPlay( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 1,2,3,4,5 };
	MoveMap( sel, PREFSMAN->m_iSongsPerPlay, ToSel, mapping, ARRAYSIZE(mapping) );
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
MOVE( ScoringType,			PREFSMAN->m_ScoringType );

static void JudgeDifficulty( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 1.50f,1.33f,1.16f,1.00f,0.84f,0.66f,0.50f,0.33f,0.20f };
	MoveMap( sel, PREFSMAN->m_fTimingWindowScale, ToSel, mapping, ARRAYSIZE(mapping) );
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
	MoveMap( sel, PREFSMAN->m_ShowSongOptions, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void ShowNameEntry( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const PrefsManager::GetRankingName mapping[] = { PrefsManager::RANKING_OFF, PrefsManager::RANKING_ON, PrefsManager::RANKING_LIST };
	MoveMap( sel, PREFSMAN->m_GetRankingName, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void DefaultFailType( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	if( ToSel )
	{
		SongOptions so;
		so.FromString( PREFSMAN->GetCurrentGamePrefs().m_sDefaultModifiers );
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

MOVE( ProgressiveLifebar,			PREFSMAN->m_iProgressiveLifebar );
MOVE( ProgressiveStageLifebar,		PREFSMAN->m_iProgressiveStageLifebar );
MOVE( ProgressiveNonstopLifebar,	PREFSMAN->m_iProgressiveNonstopLifebar );

/* Graphic options */
MOVE( CelShadeModels,		PREFSMAN->m_bCelShadeModels );
MOVE( SmoothLines,			PREFSMAN->m_bSmoothLines );

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
};

static void DisplayResolution( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const res_t mapping[] =
	{
		res_t(320, 240),
		res_t(400, 300),
		res_t(512, 384),
		res_t(640, 480),
		res_t(800, 600),
		res_t(1024, 768),
		res_t(1280, 960),
		res_t(1280, 1024)
	};
	res_t sel_res( PREFSMAN->m_iDisplayWidth, PREFSMAN->m_iDisplayHeight );
	MoveMap( sel, sel_res, ToSel, mapping, ARRAYSIZE(mapping) );
	if( !ToSel )
	{
		PREFSMAN->m_iDisplayWidth.Set( sel_res.w );
		PREFSMAN->m_iDisplayHeight.Set( sel_res.h );
	}
}

static void DisplayColor( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iDisplayColorDepth, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void TextureResolution( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 256,512,1024,2048 };
	MoveMap( sel, PREFSMAN->m_iMaxTextureResolution, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void TextureColor( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iTextureColorDepth, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void MovieColor( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iMovieColorDepth, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void RefreshRate( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const int mapping[] = { (int) REFRESH_DEFAULT,60,70,72,75,80,85,90,100,120,150 };
	MoveMap( sel, PREFSMAN->m_iRefreshRate, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void AspectRatio( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 3/4.f,1,4/3.0f,16/10.0f,16/9.f, 8/3.f };
	MoveMap( sel, PREFSMAN->m_fDisplayAspectRatio, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Sound options */
MOVE( ResamplingQuality,	PREFSMAN->m_SoundResampleQuality );
MOVE( AttractSoundFrequency,PREFSMAN->m_AttractSoundFrequency );

static void SoundVolume( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, PREFSMAN->m_fSoundVolume, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void GlobalOffsetSeconds( int &sel, bool ToSel, const ConfOption *pConfOption )
{
	float mapping[41];
	for( int i = 0; i < 41; ++i )
		mapping[i] = SCALE( i, 0.0f, 40.0f, -0.1f, +0.1f );
	
	MoveMap( sel, PREFSMAN->m_fGlobalOffsetSeconds, ToSel, mapping, ARRAYSIZE(mapping) );
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
	g_ConfOptions.back().m_iEffects = OPT_APPLY_THEME;

	ADD( ConfOption( "Theme",						Theme, ThemeChoices ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_THEME;

	ADD( ConfOption( "Announcer",					Announcer, AnnouncerChoices ) );
	ADD( ConfOption( "DefaultNoteSkin",				DefaultNoteSkin, DefaultNoteSkinChoices ) );
	ADD( ConfOption( "Instructions",				Instructions,		"SKIP","SHOW") );
	ADD( ConfOption( "ShowCaution",					MovePref,			"SKIP","SHOW") );
	ADD( ConfOption( "OniScoreDisplay",				OniScoreDisplay,	"PERCENT","DANCE POINTS") );
	ADD( ConfOption( "SongGroup",					SongGroup,			"ALL MUSIC","CHOOSE") );
	ADD( ConfOption( "WheelSections",				WheelSections,		"NEVER","ALWAYS", "ABC ONLY") );
	ADD( ConfOption( "CourseSort",					CourseSort,			"# SONGS","AVG FEET","TOTAL FEET","RANKING") );
	ADD( ConfOption( "RandomAtEnd",					RandomAtEnd,		"NO","YES") );
	ADD( ConfOption( "Translations",				Translations,		"ROMANIZATION","NATIVE LANGUAGE") );
	ADD( ConfOption( "Lyrics",						Lyrics,				"HIDE","SHOW") );

	/* Misc options */
	ADD( ConfOption( "AutogenSteps",				AutogenSteps, "OFF","ON" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_SONG;

	ADD( ConfOption( "AutogenGroupCourses",			AutogenGroupCourses, "OFF","ON" ) );
	ADD( ConfOption( "FastLoad",					MovePref,			"OFF","ON" ) );

	/* Background options */
	ADD( ConfOption( "BackgroundMode",				MovePref,			"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES" ) );
	ADD( ConfOption( "Brightness",					BGBrightness,		"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%" ) );
	ADD( ConfOption( "ShowDanger",					MovePref,			"HIDE","SHOW" ) );
	ADD( ConfOption( "DancingCharacters",			DancingCharacters,	"DEFAULT TO OFF","DEFAULT TO RANDOM","SELECT" ) );
	ADD( ConfOption( "BeginnerHelper",				BeginnerHelper,		"OFF","ON" ) );
	ADD( ConfOption( "RandomBackgrounds",			NumBackgrounds,		"5","10","15","20" ) );

	/* Input options */
	ADD( ConfOption( "AutoMapOnJoyChange",			AutoMapOnJoyChange,	"OFF","ON (recommended)" ) );
	ADD( ConfOption( "OnlyDedicatedMenuButtons",	MovePref,			"USE GAMEPLAY BUTTONS","ONLY DEDICATED BUTTONS" ) );
	ADD( ConfOption( "AutoPlay",					AutoPlay,			"OFF","ON","CPU-Controlled" ) );
	ADD( ConfOption( "BackDelayed",					BackDelayed,		"INSTANT","HOLD" ) );
	ADD( ConfOption( "OptionsNavigation",			OptionsNavigation,	"SM STYLE","ARCADE STYLE" ) );
	ADD( ConfOption( "WheelSpeed",					WheelSpeed,			"SLOW","NORMAL","FAST","REALLY FAST" ) );

	/* Gameplay options */
	ADD( ConfOption( "SoloSingles",					SoloSingles,		"OFF","ON" ) );
	ADD( ConfOption( "HiddenSongs",					MovePref,			"OFF","ON" ) );
	ADD( ConfOption( "EasterEggs",					EasterEggs,			"OFF","ON" ) );
	ADD( ConfOption( "AllowW1",				AllowW1,	"NEVER","COURSES ONLY","ALWAYS" ) );
	ADD( ConfOption( "AllowExtraStage",				AllowExtraStage,	"OFF","ON" ) );
	ADD( ConfOption( "PickExtraStage",				PickExtraStage,		"OFF","ON" ) );
	ADD( ConfOption( "UnlockSystem",				UnlockSystem,		"OFF","ON" ) );

	/* Machine options */
	ADD( ConfOption( "MenuTimer",					MovePref,			"OFF","ON" ) );
	ADD( ConfOption( "CoinMode",					CoinModeM,			"HOME","PAY","FREE PLAY" ) );
	ADD( ConfOption( "CoinModeNoHome",				CoinModeNoHome,		"PAY","FREE PLAY" ) );
	ADD( ConfOption( "SongsPerPlay",				SongsPerPlay,		"1","2","3","4","5" ) );
	ADD( ConfOption( "SongsPerPlayOrEvent",			SongsPerPlayOrEventMode, "1","2","3","4","5","EVENT" ) );
	ADD( ConfOption( "EventMode",					MovePref,			"OFF","ON" ) );
	ADD( ConfOption( "ScoringType",					ScoringType,		"NEW","OLD" ) );
	ADD( ConfOption( "JudgeDifficulty",				JudgeDifficulty,	"1","2","3","4","5","6","7","8","JUSTICE" ) );
	ADD( ConfOption( "LifeDifficulty",				LifeDifficulty,		"1","2","3","4","5","6","7" ) );
	ADD( ConfOption( "ProgressiveLifebar",			ProgressiveLifebar,	"OFF","1","2","3","4","5","6","7","8") );
	ADD( ConfOption( "ProgressiveStageLifebar",		ProgressiveStageLifebar,	"OFF","1","2","3","4","5","6","7","8","INSANITY") );
	ADD( ConfOption( "ProgressiveNonstopLifebar",	ProgressiveNonstopLifebar,"OFF","1","2","3","4","5","6","7","8","INSANITY") );
	ADD( ConfOption( "DefaultFailType",				DefaultFailType,	"IMMEDIATE","END OF SONG","OFF" ) );	
	ADD( ConfOption( "DefaultFailTypeNoOff",		DefaultFailType,	"IMMEDIATE","END OF SONG" ) );	
	ADD( ConfOption( "CoinsPerCredit",				CoinsPerCredit,		"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16" ) );
	ADD( ConfOption( "Premium",						PremiumM,			"OFF","DOUBLE FOR 1 CREDIT","JOINT PREMIUM" ) );
	ADD( ConfOption( "ShowSongOptions",				ShowSongOptions,	"HIDE","SHOW","ASK" ) );
	ADD( ConfOption( "ShowNameEntry",				ShowNameEntry,		"OFF", "ON", "RANKING SONGS" ) );

	/* Graphic options */
	ADD( ConfOption( "Windowed",					MovePref,			"FULLSCREEN", "WINDOWED" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "DisplayResolution",			DisplayResolution,	"320","400","512","640","800","1024","1280x960","1280x1024" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "DisplayColor",				DisplayColor,		"16BIT","32BIT" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "TextureResolution",			TextureResolution,	"256","512","1024","2048" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "TextureColor",				TextureColor,		"16BIT","32BIT" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "MovieColor",					MovieColor,			"16BIT","32BIT" ) );
	ADD( ConfOption( "DelayedTextureDelete",		MovePref,			"OFF","ON" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "CelShadeModels",				CelShadeModels,		"OFF","ON" ) );
	ADD( ConfOption( "SmoothLines",					SmoothLines,		"OFF","ON" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "RefreshRate",					RefreshRate,		"DEFAULT","60","70","72","75","80","85","90","100","120","150" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "AspectRatio",					AspectRatio,		"3:4","1:1","4:3","16:10","16:9","8:3" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_ASPECT_RATIO;
	ADD( ConfOption( "Vsync",						MovePref,			"NO", "YES" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_GRAPHICS;
	ADD( ConfOption( "ShowStats",					MovePref,			"OFF","ON" ) );
	ADD( ConfOption( "ShowBanners",					MovePref,			"OFF","ON" ) );

	/* Sound options */
	ADD( ConfOption( "ResamplingQuality",			ResamplingQuality,	"FAST","NORMAL","HIGH QUALITY" ) );
	ADD( ConfOption( "AttractSoundFrequency",		AttractSoundFrequency,	"NEVER","ALWAYS","2 TIMES","3 TIMES","4 TIMES","5 TIMES" ) );
	ADD( ConfOption( "SoundVolume",					SoundVolume,		"SILENT","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%" ) );
	g_ConfOptions.back().m_iEffects = OPT_APPLY_SOUND;
	{
		ConfOption c( "GlobalOffsetSeconds",		GlobalOffsetSeconds );
		for( int i = -100; i <= +100; i += 5 )
			c.AddOption( ssprintf("%+i ms", i) );
		ADD( c );
	}

	/* Editor options */
	ADD( ConfOption( "EditorShowBGChangesPlay",		MovePref,			"HIDE","SHOW") );
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

void ConfOption::MakeOptionsList( CStringArray &out ) const
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
