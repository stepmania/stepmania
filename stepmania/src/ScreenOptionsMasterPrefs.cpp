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
		const CString sNewAnnouncer = sel? choices[sel]:"";
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
		CString sModifiers = PREFSMAN->m_sDefaultModifiers;
		PlayerOptions po;
		po.FromString( sModifiers );
		SongOptions so;
		so.FromString( sModifiers );

		po.m_sNoteSkin = choices[sel];

		CStringArray as;
		if( po.GetString() != "" )
			as.push_back( po.GetString() );
		if( so.GetString() != "" )
			as.push_back( so.GetString() );

		PREFSMAN->m_sDefaultModifiers = join(", ",as);
	}
}

/* Appearance options */
MOVE( Instructions,			PREFSMAN->m_bInstructions );
MOVE( Caution,				PREFSMAN->m_bShowDontDie );
MOVE( OniScoreDisplay,		PREFSMAN->m_bDancePointsForOni );
MOVE( SongGroup,			PREFSMAN->m_bShowSelectGroup );
MOVE( WheelSections,		PREFSMAN->m_MusicWheelUsesSections );
MOVE( TenFootInRed,			PREFSMAN->m_bTenFooterInRed );
MOVE( CourseSort,			PREFSMAN->m_iCourseSortOrder );
MOVE( RandomAtEnd,			PREFSMAN->m_bMoveRandomToEnd );
MOVE( Translations,			PREFSMAN->m_bShowNative );
MOVE( Lyrics,				PREFSMAN->m_bShowLyrics );

/* Autogen options */
MOVE( AutogenMissingTypes,	PREFSMAN->m_bAutogenMissingTypes );
MOVE( AutogenGroupCourses,	PREFSMAN->m_bAutogenGroupCourses );

/* Background options */
MOVE( BackgroundMode,		PREFSMAN->m_BackgroundMode );
MOVE( ShowDanger,			PREFSMAN->m_bShowDanger );
MOVE( DancingCharacters,	PREFSMAN->m_ShowDancingCharacters );
MOVE( BeginnerHelper,		PREFSMAN->m_bShowBeginnerHelper );

static void BGBrightness( int &sel, bool ToSel, const CStringArray &choices )
{
	if( ToSel )
		sel = clamp( (int)( PREFSMAN->m_fBGBrightness*10+0.5f ), 0, 10 );
	else
		PREFSMAN->m_fBGBrightness = sel / 10.0f;
}

static void NumBackgrounds( int &sel, bool ToSel, const CStringArray &choices )
{
	if( ToSel )
		sel = clamp((PREFSMAN->m_iNumBackgrounds/5)-1, 0, 3);
	else
		PREFSMAN->m_iNumBackgrounds = (sel+1) * 5;
}

/* Input options */
MOVE( AutoMapJoysticks,		PREFSMAN->m_bAutoMapJoysticks );
MOVE( MenuButtons,			PREFSMAN->m_bOnlyDedicatedMenuButtons );
MOVE( AutoPlay,				PREFSMAN->m_bAutoPlay );
MOVE( BackDelayed,			PREFSMAN->m_bDelayedEscape );
MOVE( OptionsNavigation,	PREFSMAN->m_bArcadeOptionsNavigation );

static void WheelSpeed( int &sel, bool ToSel, const CStringArray &choices )
{
	if( ToSel )
	{
		sel =
			(PREFSMAN->m_iMusicWheelSwitchSpeed <= 5)? 0:
			(PREFSMAN->m_iMusicWheelSwitchSpeed <= 10)? 1:
			(PREFSMAN->m_iMusicWheelSwitchSpeed <= 15)? 2:3;
	}
	else
	{
		int map[] = { 5, 10, 15, 25 };
		ASSERT( sel >= 0 && sel < 4 );
		PREFSMAN->m_iMusicWheelSwitchSpeed = map[sel];
	}
}

/* Gameplay options */
MOVE( SoloSingles,			PREFSMAN->m_bSoloSingle );
MOVE( HiddenSongs,			PREFSMAN->m_bHiddenSongs );
MOVE( EasterEggs,			PREFSMAN->m_bEasterEggs );
MOVE( MarvelousTiming,		PREFSMAN->m_iMarvelousTiming );
MOVE( PickExtraStage,		PREFSMAN->m_bPickExtraStage );
MOVE( UnlockSystem,			PREFSMAN->m_bUseUnlockSystem );

/* Graphic options */
MOVE( DisplayMode,			PREFSMAN->m_bWindowed );
MOVE( WaitForVsync,			PREFSMAN->m_bVsync );
MOVE( ShowStats,			PREFSMAN->m_bShowStats );
MOVE( KeepTexturesInMemory,	PREFSMAN->m_bDelayedTextureDelete );

template<class T>
static void MoveMap( int &sel, T &opt, bool ToSel, const T *map, unsigned cnt )
{
	if( ToSel )
	{
		/* opt -> sel.  Find the closest entry in map. */
		T best_dist = -1;
		
		for( unsigned i = 0; i < cnt; ++i )
		{
			const T val = map[i];
			T dist = opt-val;
			if( dist < 0 )
				dist = -dist;
			if( best_dist != -1 && dist > best_dist )
				continue;
			
			best_dist = dist;

			sel = i;
		}
	} else {
		/* sel -> opt */
		opt = map[sel];
	}
}

static void DisplayResolution( int &sel, bool ToSel, const CStringArray &choices )
{
	const int map[] = { 320,400,512,640,800,1024,1280 };
	MoveMap( sel, PREFSMAN->m_iDisplayWidth, ToSel, map, ARRAYSIZE(map) );
	if( !ToSel )
		PREFSMAN->m_iDisplayHeight = PREFSMAN->m_iDisplayWidth * 3 / 4;
}

static void DisplayColor( int &sel, bool ToSel, const CStringArray &choices )
{
	const int map[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iDisplayColorDepth, ToSel, map, ARRAYSIZE(map) );
}

static void TextureResolution( int &sel, bool ToSel, const CStringArray &choices )
{
	const int map[] = { 256,512,1024,2048 };
	MoveMap( sel, PREFSMAN->m_iMaxTextureResolution, ToSel, map, ARRAYSIZE(map) );
}

static void TextureColor( int &sel, bool ToSel, const CStringArray &choices )
{
	const int map[] = { 16,32 };
	MoveMap( sel, PREFSMAN->m_iTextureColorDepth, ToSel, map, ARRAYSIZE(map) );
}

static void RefreshRate( int &sel, bool ToSel, const CStringArray &choices )
{
	const int map[] = { (int) REFRESH_DEFAULT,60,70,72,75,80,85,90,100,120,150 };
	MoveMap( sel, PREFSMAN->m_iRefreshRate, ToSel, map, ARRAYSIZE(map) );
}

static void MovieDecode( int &sel, bool ToSel, const CStringArray &choices )
{
	const int map[] = { 1,2,3,4 };
	MoveMap( sel, PREFSMAN->m_iMovieDecodeMS, ToSel, map, ARRAYSIZE(map) );
}

/* Sound options */
MOVE( PreloadSounds,		PREFSMAN->m_bSoundPreloadAll );
MOVE( ResamplingQuality,	PREFSMAN->m_iSoundResampleQuality );


static const ConfOption g_ConfOptions[] =
{
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
	ConfOption( "10+ foot\nIn Red",		TenFootInRed,		"NO", "YES"),
	ConfOption( "Course\nSort",			CourseSort,			"# SONGS","AVG FEET","TOTAL FEET","RANKING"),
	ConfOption( "Random\nAt End",		RandomAtEnd,		"NO","YES"),
	ConfOption( "Translations",			Translations,		"ROMANIZATION","NATIVE LANGUAGE"),
	ConfOption( "Lyrics",				Lyrics,				"HIDE","SHOW"),

	/* Autogen options */
	ConfOption( "Autogen\nMissing Types", AutogenMissingTypes, "OFF","ON" ),
	ConfOption( "Autogen\nGroup Courses", AutogenGroupCourses, "OFF","ON" ),

	/* Background options */
	ConfOption( "Background\nMode",		BackgroundMode,		"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES" ),
	ConfOption( "Brightness",			BGBrightness,		"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%" ),
	ConfOption( "Danger",				ShowDanger,			"HIDE","SHOW" ),
	ConfOption( "Dancing\nCharacters",	DancingCharacters,	"DEFAULT TO OFF","DEFAULT TO RANDOM","SELECT" ),
	ConfOption( "Beginner\nHelper",		BeginnerHelper,		"OFF","ON" ),
	ConfOption( "Random\nBackgrounds",	NumBackgrounds,		"5","10","15","20" ),

	/* Input options */
	ConfOption( "Auto Map\nJoysticks",	AutoMapJoysticks,	"OFF","ON (recommended)" ),
	ConfOption( "MenuButtons",			MenuButtons,		"USE GAMEPLAY BUTTONS","ONLY DEDICATED BUTTONS" ),
	ConfOption( "AutoPlay",				AutoPlay,			"OFF","ON" ),
	ConfOption( "Back\nDelayed",		BackDelayed,		"INSTANT","HOLD" ),
	ConfOption( "Options\nNavigation",	OptionsNavigation,	"SM STYLE","ARCADE STYLE" ),
	ConfOption( "Wheel\nSpeed",			WheelSpeed,			"SLOW","NORMAL","FAST","REALLY FAST" ),

	/* Gameplay options */
	ConfOption( "Solo\nSingles",		SoloSingles,		"OFF","ON" ),
	ConfOption( "Hidden\nSongs",		HiddenSongs,		"OFF","ON" ),
	ConfOption( "Easter\nEggs",			EasterEggs,			"OFF","ON" ),
	ConfOption( "Marvelous\nTiming",	MarvelousTiming,	"NEVER","COURSES ONLY","ALWAYS" ),
	ConfOption( "Pick Extra\nStage",	PickExtraStage,		"OFF","ON" ),
	ConfOption( "Unlock\nSystem",		UnlockSystem,		"OFF","ON" ),

	/* Graphic options */
	ConfOption( "Display\nMode",		DisplayMode,		"FULLSCREEN", "WINDOWED" ),
	ConfOption( "Display\nResolution",	DisplayResolution,	"320","400","512","640","800","1024","1280" ),
	ConfOption( "Display\nColor",		DisplayColor,		"16BIT","32BIT" ),
	ConfOption( "Texture\nResolution",	TextureResolution,	"256","512","1024","2048" ),
	ConfOption( "Texture\nColor",		TextureColor,		"16BIT","32BIT" ),
	ConfOption( "Keep Textures\nIn Memory", KeepTexturesInMemory,	"NO","YES" ),
	ConfOption( "Refresh\nRate",		RefreshRate,		"DEFAULT","60","70","72","75","80","85","90","100","120","150" ),
	ConfOption( "Movie\nDecode",		MovieDecode,		"1ms","2ms","3ms","4ms" ),
	ConfOption( "Wait For\nVsync",		WaitForVsync,		"NO", "YES" ),
	ConfOption( "Show\nStats",			ShowStats,			"OFF","ON" ),

	/* Sound options */
	ConfOption( "Preload\nSounds",		PreloadSounds,		"NO","YES" ),
	ConfOption( "Resampling\nQuality",	ResamplingQuality,	"FAST","NORMAL","HIGH QUALITY" ),
	ConfOption( "", NULL )
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
		{ KeepTexturesInMemory,	OPT_APPLY_GRAPHICS },
		{ RefreshRate,			OPT_APPLY_GRAPHICS },
		{ WaitForVsync,			OPT_APPLY_GRAPHICS }
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

