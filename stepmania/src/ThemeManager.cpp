#include "global.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include "Game.h"
#include "IniFile.h"
#include "RageTimer.h"
#include "Font.h"
#include "FontCharAliases.h"
#include "RageDisplay.h"
#include "arch/Dialog/Dialog.h"
#include "RageFile.h"
#include "ScreenManager.h"
#include "StepMania.h"
#include "Foreach.h"
#include "ThemeMetric.h"
#include "LuaHelpers.h"
#include "ScreenDimensions.h"


ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


const CString BASE_THEME_NAME = "default";
const CString LANGUAGES_SUBDIR = "Languages/";
const CString BASE_LANGUAGE = "english";
const CString THEMES_DIR  = "Themes/";
const CString METRICS_FILE = "metrics.ini";
const CString ELEMENT_CATEGORY_STRING[NUM_ELEMENT_CATEGORIES] =
{
	"BGAnimations",
	"Fonts",
	"Graphics",
	"Numbers",
	"Sounds",
	"Other"
};


struct Theme
{
	CString sThemeName;
	IniFile *iniMetrics;	// pointer because the copy constructor isn't a deep copy
};
// When looking for a metric or an element, search these from head to tail.
deque<Theme> g_vThemes;




//
// For self-registering metrics
//
static vector<IThemeMetric*> *g_pvpSubscribers = NULL;

void ThemeManager::Subscribe( IThemeMetric *p )
{
	// TRICKY: If we make this a global vector instead of a global pointer,
	// then we'd have to be careful that the static constructors of all
	// Preferences are called before the vector constructor.  It's
	// too tricky to enfore that, so we'll allocate the vector ourself
	// so that the compiler can't possibly call the vector constructor
	// after we've already added to the vector.
	if( g_pvpSubscribers == NULL )
		g_pvpSubscribers = new vector<IThemeMetric*>;
	g_pvpSubscribers->push_back( p );


	// It's ThemeManager's responsibility to make sure all of it's subscribers
	// are updated with current data.  If a metric is created after 
	// a theme is loaded, ThemeManager should update it right away (not just
	// when the theme changes).
	if( THEME && THEME->GetCurThemeName().size() )
		p->Read();
}

void ThemeManager::Unsubscribe( IThemeMetric *p )
{
	vector<IThemeMetric*>::iterator iter = find( g_pvpSubscribers->begin(), g_pvpSubscribers->end(), p );
	ASSERT( iter != g_pvpSubscribers->end() );	// tried to unregister when not registered
	g_pvpSubscribers->erase( iter );
}



/* We spend a lot of time doing redundant theme path lookups.  Cache results. */
static map<CString, CString> g_ThemePathCache[NUM_ELEMENT_CATEGORIES];

void FileNameToClassAndElement( const CString &sFileName, CString &sClassNameOut, CString &sElementOut )
{
	// split into class name and file name
	int iIndexOfFirstSpace = sFileName.Find(" ");
	if( iIndexOfFirstSpace == -1 )	// no space
	{
		sClassNameOut = "";
		sElementOut = sFileName;
	}
	else
	{
		sClassNameOut = sFileName.Left( iIndexOfFirstSpace );
		sElementOut = sFileName.Right( sFileName.length() - iIndexOfFirstSpace - 1 );
	}
}


CString ClassAndElementToFileName( const CString &sClassName, const CString &sElement )
{
	if( sClassName.empty() )
		return sElement;
	else
		return sClassName + " " + sElement;
}

ThemeManager::ThemeManager()
{
	/* We don't have any theme loaded until SwitchThemeAndLanguage is called. */
	m_sCurThemeName = "";
	
	CStringArray arrayThemeNames;
	GetThemeNames( arrayThemeNames );
}

ThemeManager::~ThemeManager()
{
}

void ThemeManager::GetThemeNames( CStringArray& AddTo )
{
	GetDirListing( THEMES_DIR + "*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( CStringArray::iterator i=AddTo.begin(); i != AddTo.end(); ++i )
	{
		if( *i == "CVS" ) {
			AddTo.erase(i, i+1);
			break;
		}
	}
}

bool ThemeManager::DoesThemeExist( const CString &sThemeName )
{
	CStringArray asThemeNames;	
	GetThemeNames( asThemeNames );
	for( unsigned i=0; i<asThemeNames.size(); i++ )
	{
		if( !sThemeName.CompareNoCase(asThemeNames[i]) )
			return true;
	}
	return false;
}

void ThemeManager::GetLanguages( CStringArray& AddTo )
{
	AddTo.clear();
	CStringArray asTemp;

	/* XXX: this should use FallbackTheme */
	GetLanguagesForTheme( m_sCurThemeName, AddTo );
	GetLanguagesForTheme( BASE_THEME_NAME, asTemp );
	AddTo.insert( AddTo.begin(), asTemp.begin(), asTemp.end() );

	// remove dupes
	sort( AddTo.begin(), AddTo.end() );
	CStringArray::iterator it = unique( AddTo.begin(), AddTo.end() );
	AddTo.erase(it, AddTo.end());
}

bool ThemeManager::DoesLanguageExist( const CString &sLanguage )
{
	CStringArray asLanguages;
	GetLanguages( asLanguages );

	for( unsigned i=0; i<asLanguages.size(); i++ )
		if( sLanguage.CompareNoCase(asLanguages[i])==0 )
			return true;
	return false;
}

void ThemeManager::LoadThemeRecursive( deque<Theme> &theme, const CString &sThemeName )
{
	static int depth = 0;
	static bool loaded_base = false;
	depth++;
	ASSERT_M( depth < 20, "Circular NoteSkin fallback references detected." );

	if( !sThemeName.CompareNoCase(BASE_THEME_NAME) )
		loaded_base = true;

	Theme t;
	t.iniMetrics = new IniFile;
	t.sThemeName = sThemeName;
	t.iniMetrics->ReadFile( GetMetricsIniPath(sThemeName) );
	t.iniMetrics->ReadFile( GetLanguageIniPath(sThemeName,BASE_LANGUAGE) );
	if( m_sCurLanguage.CompareNoCase(BASE_LANGUAGE) )
		t.iniMetrics->ReadFile( GetLanguageIniPath(sThemeName,m_sCurLanguage) );

	/* Read the fallback theme.  If no fallback theme is specified, and we havn't
	 * already loaded it, fall back on BASE_THEME_NAME.  That way, default theme
	 * fallbacks can be disabled with "FallbackTheme=". */
	CString sFallback;
	if( !t.iniMetrics->GetValue("Global","FallbackTheme",sFallback) )
	{
		if( sThemeName.CompareNoCase( BASE_THEME_NAME ) && !loaded_base )
			sFallback = BASE_THEME_NAME;
	}
	if( !sFallback.empty() )
		LoadThemeRecursive( theme, sFallback );

	g_vThemes.push_front( t );

	if( !sThemeName.CompareNoCase(sThemeName) )
		loaded_base = false;

	depth--;
}

void ThemeManager::SwitchThemeAndLanguage( const CString &sThemeName, const CString &sLanguage )
{
	CString sTheme = sThemeName;
	CString sLang = sLanguage;
	if( !DoesThemeExist(sTheme) )
		sTheme = BASE_THEME_NAME;
	if( !DoesLanguageExist(sLang) )
		sLang = BASE_LANGUAGE;
	LOG->Trace("ThemeManager::SwitchThemeAndLanguage: \"%s\", \"%s\"",
		sTheme.c_str(), sLang.c_str() );

	if( sTheme == m_sCurThemeName && sLang == m_sCurLanguage )
		return;

	m_sCurThemeName = sTheme;
	m_sCurLanguage = sLang;

	// clear theme path cache
	for( int i = 0; i < NUM_ELEMENT_CATEGORIES; ++i )
		g_ThemePathCache[i].clear();

	g_vThemes.clear();

	// load current theme
	LoadThemeRecursive( g_vThemes, m_sCurThemeName );

	CString sMetric;
	for( int i = 0; GetCommandlineArgument( "metric", &sMetric, i ); ++i )
	{
		/* sMetric must be "foo::bar=baz".  "foo" and "bar" never contain "=", so in
		 * "foo::bar=1+1=2", "baz" is always "1+1=2".  Neither foo nor bar may be
		 * empty, but baz may be. */
		Regex re( "^([^=]+)::([^=]+)=(.*)$" );
		vector<CString> sBits;
		if( !re.Compare( sMetric, sBits ) )
			RageException::Throw( "Invalid argument \"--metric=%s\"", sMetric.c_str() );

		g_vThemes.front().iniMetrics->SetValue( sBits[0], sBits[1], sBits[2] );
	}
	
	LOG->MapLog("theme", "Theme: %s", sTheme.c_str());
	LOG->MapLog("language", "Language: %s", sLang.c_str());

	// reload common sounds
	if ( SCREENMAN != NULL )
		SCREENMAN->ThemeChanged();

	/* Lua globals can use metrics which are cached, and vice versa.  Update Lua
	 * globals first; it's Lua's job to explicitly update cached metrics that it
	 * uses. */
	UpdateLuaGlobals();

	// reload subscribers
	FOREACH( IThemeMetric*, *g_pvpSubscribers, p ) (*p)->Read();
}

void ThemeManager::UpdateLuaGlobals()
{
	/* Important: explicitly refresh cached metrics that we use. */
	THEME_SCREEN_WIDTH.Read();
	THEME_SCREEN_HEIGHT.Read();

	lua_State *L = Lua::GetGlobalState();

	Lua::PushStack( L, (int) SCREEN_WIDTH );
	Lua::SetGlobal( L, "SCREEN_WIDTH" );
	Lua::PushStack( L, (int) SCREEN_HEIGHT );
	Lua::SetGlobal( L, "SCREEN_HEIGHT" );
	Lua::PushStack( L, (int) SCREEN_LEFT );
	Lua::SetGlobal( L, "SCREEN_LEFT" );
	Lua::PushStack( L, (int) SCREEN_RIGHT );
	Lua::SetGlobal( L, "SCREEN_RIGHT" );
	Lua::PushStack( L, (int) SCREEN_TOP );
	Lua::SetGlobal( L, "SCREEN_TOP" );
	Lua::PushStack( L, (int) SCREEN_BOTTOM );
	Lua::SetGlobal( L, "SCREEN_BOTTOM" );
	Lua::PushStack( L, (int) SCREEN_CENTER_X );
	Lua::SetGlobal( L, "SCREEN_CENTER_X" );
	Lua::PushStack( L, (int) SCREEN_CENTER_Y );
	Lua::SetGlobal( L, "SCREEN_CENTER_Y" );
}

CString ThemeManager::GetThemeDirFromName( const CString &sThemeName )
{
	return THEMES_DIR + sThemeName + "/";
}

CString ThemeManager::GetPathToAndFallback( const CString &sThemeName, ElementCategory category, const CString &sClassName, const CString &sElement ) 
{
	CString sClass = sClassName;

	int n = 100;
	while( n-- )
	{
		// search with requested name
		CString sRet = GetPathToRaw( sThemeName, category, sClass, sElement );
		if( !sRet.empty() )
			return sRet;

		// search fallback name (if any)
		CString sFallback;
		GetMetricRaw( sClass, "Fallback", sFallback );
		if( sFallback.empty() )
			return "";
		sClass = sFallback;
	}

	RageException::Throw("Infinite recursion looking up theme element from theme \"%s\", class \"%s\"",
		sThemeName.c_str(), sClass.c_str() );
}

CString ThemeManager::GetPathToRaw( const CString &sThemeName, ElementCategory category, const CString &sClassName, const CString &sElement ) 
{
try_element_again:

	const CString sThemeDir = GetThemeDirFromName( sThemeName );
	const CString &sCategory = ELEMENT_CATEGORY_STRING[category];

	CStringArray asElementPaths;

	// If sFileName already has an extension, we're looking for a specific file
	bool bLookingForSpecificFile = sElement.find_last_of('.') != sElement.npos;
	bool bDirsOnly = category==BGAnimations;

	if( bLookingForSpecificFile )
	{
		GetDirListing( sThemeDir + sCategory+"/"+ClassAndElementToFileName(sClassName,sElement), asElementPaths, bDirsOnly, true );
	}
	else	// look for all files starting with sFileName that have types we can use
	{
		const CString wildcard = (category == BGAnimations? "":"*");

		/* First, look for redirs. */
		GetDirListing( sThemeDir + sCategory + "/" + ClassAndElementToFileName(sClassName,sElement) + wildcard + ".redir",
						asElementPaths, false, true );

		CStringArray asPaths;
		GetDirListing( sThemeDir + sCategory + "/" + ClassAndElementToFileName(sClassName,sElement) + wildcard,
						asPaths, bDirsOnly, true );

		for( unsigned p = 0; p < asPaths.size(); ++p )
		{
			static const char *masks[NUM_ELEMENT_CATEGORIES][13] = {
				{ "", NULL },
				{ "ini", NULL },
				{ "xml", "actor", "sprite", "png", "jpg", "bmp", "gif","avi", "mpg", "mpeg", "txt", "", NULL},
				{ "png", NULL },
				{ "mp3", "ogg", "wav", NULL },
				{ "sm", NULL },
			};		
			const char **asset_masks = masks[category];

			const CString ext = GetExtension( asPaths[p] );

			if( ext == "redir" )
				continue; // got it already

			for( int i = 0; asset_masks[i]; ++i )
			{
				/* No extension means directories. */
				if( asset_masks[i][0] == 0 && !IsADirectory(asPaths[p]) )
					continue;

				if( ext == asset_masks[i] )
				{
					asElementPaths.push_back( asPaths[p] );
					break;
				}
			}
		}
		
		if( category == Fonts )
			Font::WeedFontNames(asElementPaths, ClassAndElementToFileName(sClassName,sElement));
	}
	

	if( asElementPaths.size() == 0 )
	{
		// HACK: have Fonts fall back to Numbers.  Eventually Numbers will be removed.
		if( category == Fonts )
			return GetPathToRaw( sThemeName, Numbers, sClassName, sElement ) ;
		return "";	// This isn't fatal.
	}

	if( asElementPaths.size() > 1 )
	{
		FlushDirCache();
		g_ThemePathCache[category].clear();

		CString message = ssprintf( 
			"ThemeManager:  There is more than one theme element element that matches "
			"'%s/%s/%s %s'.  Please remove all but one of these matches.",
			sThemeName.c_str(), sCategory.c_str(), sClassName.c_str(), sElement.c_str() );

		switch( Dialog::AbortRetryIgnore(message) )
		{
		case Dialog::abort:
			RageException::Throw( message ); 
			break;
		case Dialog::retry:
			FlushDirCache();
			ReloadMetrics();
			goto try_element_again;
		case Dialog::ignore:
			break;
		}
	}


	CString sPath = asElementPaths[0];
	bool bIsARedirect = GetExtension(sPath).CompareNoCase("redir")==0;

	if( !bIsARedirect )
	{
		return sPath;
	}
	else	// bIsARedirect
	{
		CString sNewFileName = GetRedirContents(sPath);

		CString sNewClassName, sNewFile;
		FileNameToClassAndElement(sNewFileName, sNewClassName, sNewFile);
		
		/* backwards-compatibility hack */
		if( category == Fonts )
			sNewFileName.Replace(" 16x16.png", "");

		/* Search again.  For example, themes/default/Fonts/foo might redir
		* to "Hello"; but "Hello" might be overridden in themes/hot pink/Fonts/Hello. */
		/* Important: We need to do a full search.  For example, BG redirs in
		* the default theme point to "_shared background", and themes override
		* just "_shared background"; the redirs in the default theme should end
		* up resolving to the overridden background. */
		/* Use GetPathToOptional because we don't want report that there's an element
		 * missing.  Instead we want to report that the redirect is invalid. */
		CString sNewPath = GetPath(category, sNewClassName, sNewFile, true);

		if( !sNewPath.empty() )
			return sNewPath;
		else
		{
			CString message = ssprintf(
					"ThemeManager:  The redirect '%s' points to the file '%s', which does not exist. "
					"Verify that this redirect is correct.",
					sPath.c_str(), sNewFileName.c_str());

			if( Dialog::AbortRetryIgnore(message) == Dialog::retry )
			{
				FlushDirCache();
				ReloadMetrics();
				goto try_element_again;
			}

			RageException::Throw( "%s", message.c_str() ); 
		}
	}
}

CString ThemeManager::GetPath( ElementCategory category, const CString &sClassName, const CString &sElement, bool bOptional ) 
{
	CString sFileName = ClassAndElementToFileName( sClassName, sElement );

	map<CString, CString> &Cache = g_ThemePathCache[category];
	{
		map<CString, CString>::const_iterator i;
		
		i = Cache.find( sFileName );
		if( i != Cache.end() )
			return i->second;
	}
	
try_element_again:
	
	for( deque<Theme>::const_iterator iter = g_vThemes.begin();
		iter != g_vThemes.end();
		iter++ )
	{
		// search the current theme
		CString ret = GetPathToAndFallback( iter->sThemeName, category, sClassName, sElement );
		if( !ret.empty() )	// we found something
		{
			Cache[sFileName] = ret;
			return ret;
		}
	}

	if( bOptional )
	{
		Cache[sFileName] = "";
		return "";
	}

	const CString &sCategory = ELEMENT_CATEGORY_STRING[category];

	/* We can't fall back on _missing in Other: the file types are unknown. */
	CString sMessage = "The theme element \"" + sCategory + "/" + sFileName +"\" is missing.";
	Dialog::Result res;
	if( category != Other )
		res = Dialog::AbortRetryIgnore(sMessage, "MissingThemeElement");
	else
		res = Dialog::RetryCancel(sMessage, "MissingThemeElement");
	switch( res )
	{
	case Dialog::retry:
		FlushDirCache();
		ReloadMetrics();
		goto try_element_again;
	case Dialog::ignore:
		LOG->Warn( 
			"Theme element '%s/%s' could not be found in '%s' or '%s'.", 
			sCategory.c_str(),
			sFileName.c_str(), 
			GetThemeDirFromName(m_sCurThemeName).c_str(), 
			GetThemeDirFromName(BASE_THEME_NAME).c_str() );

		/* Err? */
		if(sFileName == "_missing")
			RageException::Throw("'_missing' isn't present in '%s%s'", GetThemeDirFromName(BASE_THEME_NAME).c_str(), sCategory.c_str() );

		Cache[sFileName] = GetPath( category, "", "_missing" );
		return Cache[sFileName];
	/* XXX: "abort" and "cancel" are synonyms; merge */
	case Dialog::abort:
	case Dialog::cancel:
		RageException::Throw( "Theme element '%s/%s' could not be found in '%s' or '%s'.", 
			sCategory.c_str(),
			sFileName.c_str(), 
			GetThemeDirFromName(m_sCurThemeName).c_str(), 
			GetThemeDirFromName(BASE_THEME_NAME).c_str() );
	default:
		ASSERT(0);
		return "";
	}
}


CString ThemeManager::GetMetricsIniPath( const CString &sThemeName )
{
	return GetThemeDirFromName( sThemeName ) + METRICS_FILE;
}

bool ThemeManager::HasMetric( const CString &sClassName, const CString &sValueName )
{
	CString sThrowAway;
	return GetMetricRaw( sClassName, sValueName, sThrowAway );
}

void ThemeManager::ReloadMetrics()
{
	// force a reload of the metrics cache
	const CString sThemeName = m_sCurThemeName;
	const CString sCurLanguage = m_sCurLanguage;
	m_sCurThemeName = "";
	m_sCurLanguage = "";

	SwitchThemeAndLanguage( sThemeName, sCurLanguage );
	if( SCREENMAN )
		SCREENMAN->SystemMessage( "Reloaded metrics" );

	//
	// clear theme path cache
	//
	for( int i = 0; i < NUM_ELEMENT_CATEGORIES; ++i )
		g_ThemePathCache[i].clear();
}


bool ThemeManager::GetMetricRaw( const CString &sClassName, const CString &sValueName, CString &ret, int level )
{
	if( level > 100 )
		RageException::Throw("Infinite recursion looking up theme metric \"%s::%s\"", sClassName.c_str(), sValueName.c_str() );

	CString sFallback;

	for( deque<Theme>::const_iterator iter = g_vThemes.begin();
		iter != g_vThemes.end();
		iter++ )
	{
		if( iter->iniMetrics->GetValue(sClassName,sValueName,ret) )
			return true;
		if( iter->iniMetrics->GetValue(sClassName,"Fallback",sFallback) )
		{
			if( GetMetricRaw(sFallback,sValueName,ret,level+1) )
				return true;
		}
	}

	return false;
}

CString ThemeManager::GetMetricRaw( const CString &sClassName, const CString &sValueName )
{
try_metric_again:

	CString ret;
	if( ThemeManager::GetMetricRaw( sClassName, sValueName, ret ) )
		return ret;


	CString sMessage = ssprintf( "The theme metric '%s-%s' is missing.  Correct this and click Retry, or Cancel to break.",sClassName.c_str(),sValueName.c_str() );
	switch( Dialog::AbortRetryIgnore(sMessage) )
	{
	case Dialog::abort:
		break;	// fall through
	case Dialog::retry:
		FlushDirCache();
		ReloadMetrics();
		goto try_metric_again;
	case Dialog::ignore:
		return "";
	default:
		ASSERT(0);
	}

	CString sCurMetricPath = GetMetricsIniPath(m_sCurThemeName);
	CString sDefaultMetricPath = GetMetricsIniPath(BASE_THEME_NAME);

	RageException::Throw( "Theme metric '%s : %s' could not be found in '%s' or '%s'.", 
		sClassName.c_str(),
		sValueName.c_str(),
		sCurMetricPath.c_str(), 
		sDefaultMetricPath.c_str()
		);
}

/* Get a string metric. */
CString ThemeManager::GetMetric( const CString &sClassName, const CString &sValueName )
{
	CString sValue = GetMetricRaw(sClassName,sValueName);

	// "::" means newline since you can't use line breaks in an ini file.
	sValue.Replace("::","\n");

	/* XXX: add a parameter to turn this off if there are some metrics where
	 * we don't want markers */
	FontCharAliases::ReplaceMarkers(sValue);

	return sValue;
}

int ThemeManager::GetMetricI( const CString &sClassName, const CString &sValueName )
{
	return atoi( GetMetricRaw(sClassName,sValueName) );
}

float ThemeManager::GetMetricF( const CString &sClassName, const CString &sValueName )
{
	CString sValue = GetMetricRaw( sClassName, sValueName );

#if defined(DEBUG)
	if( sValueName.Right(1) == "X" || sValueName.Right(1) == "Y" )	// an absolute X or Y position
	{
		if( isdigit(sValue[0]) && sValue.Find('-') == -1 && sValue.Find('+') == -1 )
		{
			LOG->Warn( "Absolute position metric '%s'-'%s' should contain a SCREEN_* constant", sClassName.c_str(), sValueName.c_str() );
		}
	}
#endif

	Lua::PrepareExpression( sValue );

	return Lua::RunExpressionF( sValue );
}

// #include "LuaHelpers.h"
bool ThemeManager::GetMetricB( const CString &sClassName, const CString &sValueName )
{
	CString sValue = GetMetricRaw( sClassName,sValueName );

	/* Watch out: "0" and "1" are not false and true in Lua (all string values are
	 * true).  Make sure that we catch all values that are supposed to be simple
	 * booleans. */
	TrimLeft( sValue );
	TrimRight( sValue );
	if( sValue.Left(1) == "0" )
		return false; /* optimization */
	if( sValue.Left(1) == "1" )
		return true; /* optimization */

	Lua::PrepareExpression( sValue );
	
	return Lua::RunExpressionB( sValue );
}

RageColor ThemeManager::GetMetricC( const CString &sClassName, const CString &sValueName )
{
	RageColor ret(1,1,1,1);
	if( !ret.FromString( GetMetricRaw(sClassName,sValueName) ) )
		LOG->Warn( "The color value '%s' for metric '%s : %s' is invalid.", GetMetricRaw(sClassName,sValueName).c_str(), sClassName.c_str(), sValueName.c_str() );
	return ret;
}

Commands ThemeManager::GetMetricA( const CString &sClassName, const CString &sValueName )
{
	return ParseCommands( GetMetricRaw(sClassName,sValueName) );
}

void ThemeManager::NextTheme()
{
	CStringArray as;
	GetThemeNames( as );
	unsigned i;
	for( i=0; i<as.size(); i++ )
		if( as[i].CompareNoCase(m_sCurThemeName)==0 )
			break;
	int iNewIndex = (i+1)%as.size();
	SwitchThemeAndLanguage( as[iNewIndex], m_sCurLanguage );
}

void ThemeManager::GetLanguagesForTheme( const CString &sThemeName, CStringArray& asLanguagesOut )
{
	CString sLanguageDir = GetThemeDirFromName(sThemeName) + LANGUAGES_SUBDIR;
	CStringArray as;
	GetDirListing( sLanguageDir + "*.ini", as );
	
	// stip out metrics.ini
	for( int i=as.size()-1; i>=0; i-- )
	{
		if( as[i].CompareNoCase(METRICS_FILE)==0 )
			as.erase( as.begin()+i );
		// strip ".ini"
		as[i] = as[i].Left( as[i].size()-4 );
	}

	asLanguagesOut = as;
}

CString ThemeManager::GetLanguageIniPath( const CString &sThemeName, const CString &sLanguage )
{
	return GetThemeDirFromName(sThemeName) + LANGUAGES_SUBDIR + sLanguage + ".ini";
}

// TODO: remove these and update the places that use them
CString ThemeManager::GetPathToB( const CString &sFileName, bool bOptional ) { CString sClassName, sElement; FileNameToClassAndElement(sFileName,sClassName,sElement); return GetPathB(sClassName,sElement,bOptional); }
CString ThemeManager::GetPathToF( const CString &sFileName, bool bOptional ) { CString sClassName, sElement; FileNameToClassAndElement(sFileName,sClassName,sElement); return GetPathF(sClassName,sElement,bOptional); }
CString ThemeManager::GetPathToG( const CString &sFileName, bool bOptional ) { CString sClassName, sElement; FileNameToClassAndElement(sFileName,sClassName,sElement); return GetPathG(sClassName,sElement,bOptional); }
CString ThemeManager::GetPathToS( const CString &sFileName, bool bOptional ) { CString sClassName, sElement; FileNameToClassAndElement(sFileName,sClassName,sElement); return GetPathS(sClassName,sElement,bOptional); }
CString ThemeManager::GetPathToO( const CString &sFileName, bool bOptional ) { CString sClassName, sElement; FileNameToClassAndElement(sFileName,sClassName,sElement); return GetPathO(sClassName,sElement,bOptional); }

void ThemeManager::GetModifierNames( set<CString>& AddTo )
{
	for( deque<Theme>::const_iterator iter = g_vThemes.begin();
		iter != g_vThemes.end();
		++iter )
	{
		const XNode *cur = iter->iniMetrics->GetChild( "OptionNames" );
		if( cur )
		{
			FOREACH_CONST_Attr( cur, p )
				AddTo.insert( p->m_sName );
		}
	}
}

void ThemeManager::GetMetric( const CString &sClassName, const CString &sValueName, Commands &valueOut )
{
	valueOut = GetMetricA( sClassName, sValueName ); 
}

/*
 * (c) 2001-2004 Chris Danford
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
