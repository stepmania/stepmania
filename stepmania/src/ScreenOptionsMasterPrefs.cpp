#include "global.h"

#include "ScreenOptionsMasterPrefs.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "NoteSkinManager.h"
#include "PlayerOptions.h"
#include "SongOptions.h"

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
MOVE( Instructions,		PREFSMAN->m_bInstructions );
MOVE( Caution,			PREFSMAN->m_bShowDontDie );
MOVE( OniScoreDisplay,	PREFSMAN->m_bDancePointsForOni );
MOVE( SongGroup,		PREFSMAN->m_bShowSelectGroup );
MOVE( WheelSections,	PREFSMAN->m_MusicWheelUsesSections );
MOVE( TenFootInRed,		PREFSMAN->m_bTenFooterInRed );
MOVE( CourseSort,		PREFSMAN->m_iCourseSortOrder );
MOVE( RandomAtEnd,		PREFSMAN->m_bMoveRandomToEnd );
MOVE( Translations,		PREFSMAN->m_bShowNative );
MOVE( Lyrics,			PREFSMAN->m_bShowLyrics );

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

/* Sound options */
MOVE( PreloadSounds,	PREFSMAN->m_bSoundPreloadAll );
MOVE( ResamplingQuality,PREFSMAN->m_iSoundResampleQuality );


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

	/* Sound options */
	ConfOption( "Preload\nSounds",		PreloadSounds, "NO","YES" ),
	ConfOption( "Resampling\nQuality",	ResamplingQuality, "FAST","NORMAL","HIGH QUALITY" ),
	ConfOption( "", NULL )
};

const ConfOption *FindConfOption( CString name )
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

