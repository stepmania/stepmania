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

static void GetDefaultModifiers( PlayerOptions &po, SongOptions &so )
{
	po.FromString( PREFSMAN->m_sDefaultModifiers );
	so.FromString( PREFSMAN->m_sDefaultModifiers );
}

static void SetDefaultModifiers( const PlayerOptions &po, const SongOptions &so )
{
	CStringArray as;
	if( po.GetString() != "" )
		as.push_back( po.GetString() );
	if( so.GetString() != "" )
		as.push_back( so.GetString() );

	PREFSMAN->m_sDefaultModifiers = join(", ",as);
}

template<class T>
static void MoveMap( int &sel, T &opt, bool ToSel, const T *mapping, unsigned cnt )
{
	if( ToSel )
	{
		/* opt -> sel.  Find the closest entry in mapping. */
		T best_dist = T();
		bool have_best = false;

		for( unsigned i = 0; i < cnt; ++i )
		{
			const T val = mapping[i];
			T dist = opt < val? (T)(val-opt):(T)(opt-val);
			if( have_best && best_dist < dist )
				continue;

			have_best = true;
			best_dist = dist;

			sel = i;
		}
	} else {
		/* sel -> opt */
		opt = mapping[sel];
	}
}


/* "sel" is the selection in the menu. */
template<class T>
static void MoveData( int &sel, T &opt, bool ToSel )
{
	if( ToSel )	(int&) sel = opt;
	else		opt = (T) sel;
}

template<>
static void MoveData( int &sel, bool &opt, bool ToSel )
{
	if( ToSel )	sel = opt;
	else		opt = !!sel;
}

#define MOVE( name, opt ) \
	static void name( int &sel, bool ToSel, const CStringArray &choices ) \
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

static void GameSel( int &sel, bool ToSel, const CStringArray &choices )
{
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

static void Language( int &sel, bool ToSel, const CStringArray &choices )
{
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

static void Theme( int &sel, bool ToSel, const CStringArray &choices )
{
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

static void Announcer( int &sel, bool ToSel, const CStringArray &choices )
{
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

static void DefaultNoteSkin( int &sel, bool ToSel, const CStringArray &choices )
{
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
		GetDefaultModifiers( po, so );
		po.m_sNoteSkin = choices[sel];
		SetDefaultModifiers( po, so );
	}
}

/* Appearance options */
MOVE( Instructions,			PREFSMAN->m_bInstructions );
MOVE( Caution,				PREFSMAN->m_bShowDontDie );
MOVE( OniScoreDisplay,		PREFSMAN->m_bDancePointsForOni );
MOVE( SongGroup,			PREFSMAN->m_bShowSelectGroup );
MOVE( WheelSections,		PREFSMAN->m_MusicWheelUsesSections );
MOVE( CourseSort,			PREFSMAN->m_iCourseSortOrder );
MOVE( RandomAtEnd,			PREFSMAN->m_bMoveRandomToEnd );
MOVE( Translations,			PREFSMAN->m_bShowNative );
MOVE( Lyrics,				PREFSMAN->m_bShowLyrics );

/* Misc. options */
MOVE( AutogenSteps,			PREFSMAN->m_bAutogenSteps );
MOVE( AutogenGroupCourses,	PREFSMAN->m_bAutogenGroupCourses );
MOVE( FastLoad,				PREFSMAN->m_bFastLoad );

/* Background options */
MOVE( BackgroundMode,		PREFSMAN->m_BackgroundMode );
MOVE( ShowDanger,			PREFSMAN->m_bShowDanger );
MOVE( DancingCharacters,	PREFSMAN->m_ShowDancingCharacters );
MOVE( BeginnerHelper,		PREFSMAN->m_bShowBeginnerHelper );

static void BGBrightness( int &sel, bool ToSel, const CStringArray &choices )
{
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, PREFSMAN->m_fBGBrightness, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void NumBackgrounds( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 5,10,15,20 };
	MoveMap( sel, PREFSMAN->m_iNumBackgrounds, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Input options */
MOVE( AutoMapOnJoyChange,	PREFSMAN->m_bAutoMapOnJoyChange );
MOVE( MenuButtons,			PREFSMAN->m_bOnlyDedicatedMenuButtons );
MOVE( AutoPlay,				PREFSMAN->m_bAutoPlay );
MOVE( BackDelayed,			PREFSMAN->m_bDelayedEscape );
MOVE( OptionsNavigation,	PREFSMAN->m_bArcadeOptionsNavigation );

static void WheelSpeed( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 5, 10, 15, 25 };
	MoveMap( sel, PREFSMAN->m_iMusicWheelSwitchSpeed, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Gameplay options */
MOVE( SoloSingles,			PREFSMAN->m_bSoloSingle );
MOVE( HiddenSongs,			PREFSMAN->m_bHiddenSongs );
MOVE( EasterEggs,			PREFSMAN->m_bEasterEggs );
MOVE( MarvelousTiming,		PREFSMAN->m_iMarvelousTiming );
MOVE( AllowExtraStage,		PREFSMAN->m_bAllowExtraStage );
MOVE( PickExtraStage,		PREFSMAN->m_bPickExtraStage );
MOVE( UnlockSystem,			PREFSMAN->m_bUseUnlockSystem );

/* Coin options */
MOVE( CoinMode,			PREFSMAN->m_iCoinMode );

static void CoinModeNoHome( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 1,2 };
	MoveMap( sel, PREFSMAN->m_iCoinMode, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void CoinsPerCredit( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 1,2,3,4,5,6,7,8 };
	MoveMap( sel, PREFSMAN->m_iCoinsPerCredit, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void Premium( int &sel, bool ToSel, const CStringArray &choices )
{
	const PrefsManager::Premium mapping[] = { PrefsManager::NO_PREMIUM,PrefsManager::DOUBLES_PREMIUM,PrefsManager::JOINT_PREMIUM };
	MoveMap( sel, PREFSMAN->m_Premium, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void SongsPerPlay( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 1,2,3,4,5,6,7 };
	MoveMap( sel, PREFSMAN->m_iNumArcadeStages, ToSel, mapping, ARRAYSIZE(mapping) );
}
MOVE( EventMode,			PREFSMAN->m_bEventMode );

/* Machine options */
MOVE( MenuTimer,			PREFSMAN->m_bMenuTimer );
MOVE( ScoringType,			PREFSMAN->m_iScoringType );

static void JudgeDifficulty( int &sel, bool ToSel, const CStringArray &choices )
{
	const float mapping[] = { 1.50f,1.33f,1.16f,1.00f,0.84f,0.66f,0.50f,0.33f,0.20f };
	MoveMap( sel, PREFSMAN->m_fJudgeWindowScale, ToSel, mapping, ARRAYSIZE(mapping) );
}

void LifeDifficulty( int &sel, bool ToSel, const CStringArray &choices )
{
	const float mapping[] = { 1.60f,1.40f,1.20f,1.00f,0.80f,0.60f,0.40f };
	MoveMap( sel, PREFSMAN->m_fLifeDifficultyScale, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void ShowSongOptions( int &sel, bool ToSel, const CStringArray &choices )
{
	const PrefsManager::Maybe mapping[] = { PrefsManager::NO,PrefsManager::YES,PrefsManager::ASK };
	MoveMap( sel, PREFSMAN->m_ShowSongOptions, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void ShowNameEntry( int &sel, bool ToSel, const CStringArray &choices )
{
	const PrefsManager::GetRankingName mapping[] = { PrefsManager::RANKING_OFF, PrefsManager::RANKING_ON, PrefsManager::RANKING_LIST };
	MoveMap( sel, PREFSMAN->m_iGetRankingName, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void DefaultFailType( int &sel, bool ToSel, const CStringArray &choices )
{
	if( ToSel )
	{
		SongOptions so;
		so.FromString( PREFSMAN->m_sDefaultModifiers );
		sel = so.m_FailType;
	} else {
		PlayerOptions po;
		SongOptions so;
		GetDefaultModifiers( po, so );

		switch( sel )
		{
		case 0:	so.m_FailType = SongOptions::FAIL_IMMEDIATE;			break;
		case 1:	so.m_FailType = SongOptions::FAIL_COMBO_OF_30_MISSES;	break;
		case 2:	so.m_FailType = SongOptions::FAIL_END_OF_SONG;			break;
		case 3:	so.m_FailType = SongOptions::FAIL_OFF;					break;
		default:
			ASSERT(0);
		}

		SetDefaultModifiers( po, so );
	}
}

MOVE( ProgressiveLifebar,	PREFSMAN->m_iProgressiveLifebar );
MOVE( ProgressiveStageLifebar,		PREFSMAN->m_iProgressiveStageLifebar );
MOVE( ProgressiveNonstopLifebar,	PREFSMAN->m_iProgressiveNonstopLifebar );

/* Graphic options */
MOVE( DisplayMode,			PREFSMAN->m_bWindowed );
MOVE( WaitForVsync,			PREFSMAN->m_bVsync );
MOVE( ShowStats,			PREFSMAN->m_bShowStats );
MOVE( ShowBanners,			PREFSMAN->m_bShowBanners );
MOVE( KeepTexturesInMemory,	PREFSMAN->m_bDelayedTextureDelete );
MOVE( CelShadeModels,			PREFSMAN->m_bCelShadeModels );
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

static void DisplayResolution( int &sel, bool ToSel, const CStringArray &choices )
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
		PREFSMAN->m_iDisplayWidth = sel_res.w;
		PREFSMAN->m_iDisplayHeight = sel_res.h;
	}
}

static void DisplayColor( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iDisplayColorDepth, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void TextureResolution( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 256,512,1024,2048 };
	MoveMap( sel, PREFSMAN->m_iMaxTextureResolution, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void TextureColor( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iTextureColorDepth, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void MovieColor( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iMovieColorDepth, ToSel, mapping, ARRAYSIZE(mapping) );
}

static void RefreshRate( int &sel, bool ToSel, const CStringArray &choices )
{
	const int mapping[] = { (int) REFRESH_DEFAULT,60,70,72,75,80,85,90,100,120,150 };
	MoveMap( sel, PREFSMAN->m_iRefreshRate, ToSel, mapping, ARRAYSIZE(mapping) );
}

/* Sound options */
MOVE( ResamplingQuality,	PREFSMAN->m_iSoundResampleQuality );
MOVE( AttractSoundFrequency,PREFSMAN->m_iAttractSoundFrequency );

static void SoundVolume( int &sel, bool ToSel, const CStringArray &choices )
{
	const float mapping[] = { 0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f };
	MoveMap( sel, PREFSMAN->m_fSoundVolume, ToSel, mapping, ARRAYSIZE(mapping) );
}


static const ConfOption g_ConfOptions[] =
{
	/* Select game */
	ConfOption( "Game",					GameSel, GameChoices ),

	/* Appearance options */
	ConfOption( "Language",				Language, LanguageChoices ),
	ConfOption( "Theme",				Theme, ThemeChoices ),
	ConfOption( "Announcer",			Announcer, AnnouncerChoices ),
	ConfOption( "Default\nNoteSkin",	DefaultNoteSkin, DefaultNoteSkinChoices ),
	ConfOption( "Instructions",			Instructions,		"SKIP","SHOW"),
	ConfOption( "Caution",				Caution,			"SKIP","SHOW"),
	ConfOption( "Oni Score\nDisplay",	OniScoreDisplay,	"PERCENT","DANCE POINTS"),
	ConfOption( "Song\nGroup",			SongGroup,			"ALL MUSIC","CHOOSE"),
	ConfOption( "Wheel\nSections",		WheelSections,		"NEVER","ALWAYS", "ABC ONLY"),
	ConfOption( "Course\nSort",			CourseSort,			"# SONGS","AVG FEET","TOTAL FEET","RANKING"),
	ConfOption( "Random\nAt End",		RandomAtEnd,		"NO","YES"),
	ConfOption( "Translations",			Translations,		"ROMANIZATION","NATIVE LANGUAGE"),
	ConfOption( "Lyrics",				Lyrics,				"HIDE","SHOW"),

	/* Misc options */
	ConfOption( "Autogen\nSteps",		AutogenSteps, "OFF","ON" ),
	ConfOption( "Autogen\nGroup Courses", AutogenGroupCourses, "OFF","ON" ),
	ConfOption( "Fast\nLoad",			  FastLoad,			   "OFF","ON" ),

	/* Background options */
	ConfOption( "Background\nMode",		BackgroundMode,		"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES" ),
	ConfOption( "Brightness",			BGBrightness,		"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%" ),
	ConfOption( "Danger",				ShowDanger,			"HIDE","SHOW" ),
	ConfOption( "Dancing\nCharacters",	DancingCharacters,	"DEFAULT TO OFF","DEFAULT TO RANDOM","SELECT" ),
	ConfOption( "Beginner\nHelper",		BeginnerHelper,		"OFF","ON" ),
	ConfOption( "Random\nBackgrounds",	NumBackgrounds,		"5","10","15","20" ),

	/* Input options */
	ConfOption( "Auto Map\nOn Joy Change",	AutoMapOnJoyChange,	"OFF","ON (recommended)" ),
	ConfOption( "MenuButtons",				MenuButtons,		"USE GAMEPLAY BUTTONS","ONLY DEDICATED BUTTONS" ),
	ConfOption( "AutoPlay",					AutoPlay,			"OFF","ON" ),
	ConfOption( "Back\nDelayed",			BackDelayed,		"INSTANT","HOLD" ),
	ConfOption( "Options\nNavigation",		OptionsNavigation,	"SM STYLE","ARCADE STYLE" ),
	ConfOption( "Wheel\nSpeed",				WheelSpeed,			"SLOW","NORMAL","FAST","REALLY FAST" ),

	/* Gameplay options */
	ConfOption( "Solo\nSingles",		SoloSingles,		"OFF","ON" ),
	ConfOption( "Hidden\nSongs",		HiddenSongs,		"OFF","ON" ),
	ConfOption( "Easter\nEggs",			EasterEggs,			"OFF","ON" ),
	ConfOption( "Marvelous\nTiming",	MarvelousTiming,	"NEVER","COURSES ONLY","ALWAYS" ),
	ConfOption( "Allow Extra\nStage",	AllowExtraStage,	"OFF","ON" ),
	ConfOption( "Pick Extra\nStage",	PickExtraStage,		"OFF","ON" ),
	ConfOption( "Unlock\nSystem",		UnlockSystem,		"OFF","ON" ),

	/* Machine options */
	ConfOption( "Menu\nTimer",			MenuTimer,			"OFF","ON" ),
	ConfOption( "CoinMode",				CoinMode,			"HOME","PAY","FREE PLAY" ),
	ConfOption( "CoinModeNoHome",		CoinModeNoHome,		"PAY","FREE PLAY" ),
	ConfOption( "Songs Per\nPlay",		SongsPerPlay,		"1","2","3","4","5","6","7" ),
	ConfOption( "Event\nMode",			EventMode,			"OFF","ON" ),
	ConfOption( "Scoring\nType",		ScoringType,		"MAX2","5TH" ),
	ConfOption( "Judge\nDifficulty",	JudgeDifficulty,	"1","2","3","4","5","6","7","8","JUSTICE" ),
	ConfOption( "Life\nDifficulty",		LifeDifficulty,		"1","2","3","4","5","6","7" ),
	ConfOption( "Progressive\nLifebar",	ProgressiveLifebar,	"OFF","1","2","3","4","5","6","7","8"),
	ConfOption( "Progressive\nStage Lifebar",ProgressiveStageLifebar,	"OFF","1","2","3","4","5","6","7","8","INSANITY"),
	ConfOption( "Progressive\nNonstop Lifebar",ProgressiveNonstopLifebar,"OFF","1","2","3","4","5","6","7","8","INSANITY"),
	ConfOption( "Default\nFail Type",	DefaultFailType,	"IMMEDIATE","COMBO OF 30 MISSES","END OF SONG","OFF" ),	
	ConfOption( "DefaultFailTypeNoOff",	DefaultFailType,	"IMMEDIATE","COMBO OF 30 MISSES","END OF SONG" ),	
	ConfOption( "Coins Per\nCredit",	CoinsPerCredit,		"1","2","3","4","5","6","7","8" ),
	ConfOption( "Premium",				Premium,			"OFF","DOUBLE FOR 1 CREDIT","JOINT PREMIUM" ),
	ConfOption( "Show Song\nOptions",	ShowSongOptions,	"HIDE","SHOW","ASK" ),
	ConfOption( "Show Name\nEntry",     ShowNameEntry,		"OFF", "ON", "RANKING SONGS" ),

	/* Graphic options */
	ConfOption( "Display\nMode",		DisplayMode,		"FULLSCREEN", "WINDOWED" ),
	ConfOption( "Display\nResolution",	DisplayResolution,	"320","400","512","640","800","1024","1280x960","1280x1024" ),
	ConfOption( "Display\nColor",		DisplayColor,		"16BIT","32BIT" ),
	ConfOption( "Texture\nResolution",	TextureResolution,	"256","512","1024","2048" ),
	ConfOption( "Texture\nColor",		TextureColor,		"16BIT","32BIT" ),
	ConfOption( "Movie\nColor",			MovieColor,			"16BIT","32BIT" ),
	ConfOption( "Keep Textures\nIn Memory", KeepTexturesInMemory,	"OFF","ON" ),
	ConfOption( "CelShade\nModels",		CelShadeModels,		"OFF","ON" ),
	ConfOption( "SmoothLines",			SmoothLines,		"OFF","ON" ),
	ConfOption( "Refresh\nRate",		RefreshRate,		"DEFAULT","60","70","72","75","80","85","90","100","120","150" ),
	ConfOption( "Wait For\nVsync",		WaitForVsync,		"NO", "YES" ),
	ConfOption( "Show\nStats",			ShowStats,			"OFF","ON" ),
	ConfOption( "Show\nBanners",		ShowBanners,		"OFF","ON" ),

	/* Sound options */
	ConfOption( "Resampling\nQuality",	ResamplingQuality,	"FAST","NORMAL","HIGH QUALITY" ),
	ConfOption( "Attract\nSound Frequency",	AttractSoundFrequency,	"NEVER","ALWAYS","2 TIMES","3 TIMES","4 TIMES","5 TIMES" ),
	ConfOption( "Sound\nVolume",		SoundVolume,		"SILENT","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%" ),
	ConfOption( "", NULL )	// end marker
};

/* Get a mask of effects to apply if the given option changes. */
int ConfOption::GetEffects() const
{
	struct opts {
		ConfOption::MoveData_t ptr;
		int effects;
	} opts[] = {
		{ Language,				OPT_APPLY_THEME },
		{ Theme,				OPT_APPLY_THEME },
		{ DisplayMode,			OPT_APPLY_GRAPHICS },
		{ DisplayResolution,	OPT_APPLY_GRAPHICS },
		{ DisplayColor,			OPT_APPLY_GRAPHICS },
		{ TextureResolution,	OPT_APPLY_GRAPHICS },
		{ TextureColor,			OPT_APPLY_GRAPHICS },
		{ KeepTexturesInMemory,	OPT_APPLY_GRAPHICS },
		{ SmoothLines,			OPT_APPLY_GRAPHICS },
		{ RefreshRate,			OPT_APPLY_GRAPHICS },
		{ WaitForVsync,			OPT_APPLY_GRAPHICS },
		{ GameSel,				OPT_RESET_GAME },
		{ SoundVolume,			OPT_APPLY_SOUND },
		{ AutogenSteps,			OPT_APPLY_SONG },
	};

	int ret = OPT_SAVE_PREFERENCES;
	for( unsigned i = 0; i < ARRAYSIZE(opts); ++i )
		if( opts[i].ptr == MoveData )
			return ret |= opts[i].effects;

	return ret;
}

const ConfOption *ConfOption::Find( CString name )
{
	for( unsigned i = 0; g_ConfOptions[i].name != ""; ++i )
	{
		const ConfOption *opt = &g_ConfOptions[i];

		CString match(opt->name);
		match.Replace("\n", "");
		match.Replace("-", "");
		match.Replace(" ", "");

		if( match.CompareNoCase(name) )
			continue;

		return opt;
	}

	return NULL;
}

void ConfOption::MakeOptionsList( CStringArray &out ) const
{
	if( MakeOptionsListCB == NULL )
	{
		out = names;
		return;
	}

	MakeOptionsListCB( out );
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
