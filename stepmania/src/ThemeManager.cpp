#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ThemeManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ThemeManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"
#include "GameDef.h"
#include "IniFile.h"
#include "RageTimer.h"
#include "Font.h"
#include "FontCharAliases.h"
#include "RageDisplay.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "arch/arch.h"
#include "RageFile.h"
#include "ScreenManager.h"


ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


const CString BASE_THEME_NAME = "default";
const CString FALLBACK_THEME_NAME = "fallback";
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

/* We spend a lot of time doing redundant theme path lookups.  Cache results. */
static map<CString, CString> g_ThemePathCache[NUM_ELEMENT_CATEGORIES];

ThemeManager::ThemeManager()
{
	m_pIniMetrics = new IniFile;

	m_sCurThemeName = BASE_THEME_NAME;	// Use the base theme for now.  It's up to PrefsManager to change this.
	m_uHashForCurThemeMetrics = 0;
	m_uHashForBaseThemeMetrics = 0;
	m_uHashForCurThemeCurLanguage = 0;
	m_uHashForBaseThemeCurLanguage = 0;
	m_uHashForCurThemeBaseLanguage = 0;
	m_uHashForBaseThemeBaseLanguage = 0;
	
	CStringArray arrayThemeNames;
	GetThemeNames( arrayThemeNames );
}

ThemeManager::~ThemeManager()
{
	delete m_pIniMetrics;
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

bool ThemeManager::DoesThemeExist( CString sThemeName )
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

	GetLanguagesForTheme( m_sCurThemeName, AddTo );
	GetLanguagesForTheme( BASE_THEME_NAME, asTemp );
	AddTo.insert( AddTo.begin(), asTemp.begin(), asTemp.end() );

	// remove dupes
	sort( AddTo.begin(), AddTo.end() );
	CStringArray::iterator it = unique( AddTo.begin(), AddTo.end() );
	AddTo.erase(it, AddTo.end());
}

bool ThemeManager::DoesLanguageExist( CString sLanguage )
{
	CStringArray asLanguages;
	GetLanguages( asLanguages );

	for( unsigned i=0; i<asLanguages.size(); i++ )
		if( sLanguage.CompareNoCase(asLanguages[i])==0 )
			return true;
	return false;
}

void ThemeManager::SwitchThemeAndLanguage( CString sThemeName, CString sLanguage )
{
	if( !DoesThemeExist(sThemeName) )
		m_sCurThemeName = BASE_THEME_NAME;
	else
		m_sCurThemeName = sThemeName;

	if( !DoesLanguageExist(sLanguage) )
		m_sCurLanguage = BASE_LANGUAGE;
	else
		m_sCurLanguage = sLanguage;

	// clear theme path cache
	for( int i = 0; i < NUM_ELEMENT_CATEGORIES; ++i )
		g_ThemePathCache[i].clear();

	// update hashes for metrics files
	m_uHashForCurThemeMetrics = GetHashForFile( GetMetricsIniPath(m_sCurThemeName) );
	m_uHashForBaseThemeMetrics = GetHashForFile( GetMetricsIniPath(BASE_THEME_NAME) );
	m_uHashForBaseThemeBaseLanguage = GetHashForFile( GetLanguageIniPath(BASE_THEME_NAME,BASE_LANGUAGE) );
	m_uHashForCurThemeBaseLanguage = GetHashForFile( GetLanguageIniPath(m_sCurThemeName,BASE_LANGUAGE) );
	m_uHashForBaseThemeCurLanguage = GetHashForFile( GetLanguageIniPath(BASE_THEME_NAME,m_sCurLanguage) );
	m_uHashForCurThemeCurLanguage = GetHashForFile( GetLanguageIniPath(m_sCurThemeName,m_sCurLanguage) );

	// read new metrics.  First read base metrics, then read cur theme's metrics, overriding base theme
	m_pIniMetrics->Reset();
	m_pIniMetrics->SetPath( GetMetricsIniPath(FALLBACK_THEME_NAME) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetMetricsIniPath(BASE_THEME_NAME) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetMetricsIniPath(m_sCurThemeName) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetLanguageIniPath(FALLBACK_THEME_NAME,BASE_LANGUAGE) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetLanguageIniPath(BASE_THEME_NAME,BASE_LANGUAGE) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetLanguageIniPath(m_sCurThemeName,BASE_LANGUAGE) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetLanguageIniPath(FALLBACK_THEME_NAME,m_sCurLanguage) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetLanguageIniPath(BASE_THEME_NAME,m_sCurLanguage) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetLanguageIniPath(m_sCurThemeName,m_sCurLanguage) );
	m_pIniMetrics->ReadFile();

	LOG->MapLog("theme", "Theme: %s", sThemeName.c_str());
	LOG->MapLog("language", "Language: %s", sLanguage.c_str());
}

CString ThemeManager::GetThemeDirFromName( const CString &sThemeName )
{
	return THEMES_DIR + sThemeName + "/";
}

CString ThemeManager::GetPathTo( CString sThemeName, ElementCategory category, CString sFileName ) 
{
try_element_again:
	sFileName.MakeLower();

	const CString sThemeDir = GetThemeDirFromName( sThemeName );
	const CString sCategory = ELEMENT_CATEGORY_STRING[category];

	CStringArray asElementPaths;

	// If sFileName already has an extension, we're looking for a specific file
	bool bLookingForSpecificFile = sFileName.find_last_of('.') != sFileName.npos;
	bool bDirsOnly = category==BGAnimations;

	if( bLookingForSpecificFile )
	{
		GetDirListing( sThemeDir + sCategory+"/"+sFileName, asElementPaths, bDirsOnly, true );
	}
	else	// look for all files starting with sFileName that have types we can use
	{
		/* First, look for redirs. */
		GetDirListing( sThemeDir + sCategory + "/" + sFileName + ".redir",
						asElementPaths, false, true );

		const CString wildcard = (category == BGAnimations? "":"*");
		CStringArray asPaths;
		GetDirListing( sThemeDir + sCategory + "/" + sFileName + wildcard,
						asPaths, bDirsOnly, true );

		for( unsigned p = 0; p < asPaths.size(); ++p )
		{
			static const char *masks[NUM_ELEMENT_CATEGORIES][12] = {
				{ "", NULL },
				{ "ini", NULL },
				{ "actor", "sprite", "png", "jpg", "bmp", "gif","avi", "mpg", "mpeg", "txt", "", NULL},
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
			Font::WeedFontNames(asElementPaths, sFileName);
	}
	

	if( asElementPaths.size() == 0 )
	{
		return "";	// This isn't fatal.
	}

	if( asElementPaths.size() > 1 )
	{
		FlushDirCache();
		g_ThemePathCache[category].clear();

		CString message = ssprintf( 
			"ThemeManager:  There is more than one theme element element that matches "
			"'%s/%s/%s'.  Please remove all but one of these matches.",
			sThemeName.c_str(), sCategory.c_str(), sFileName.c_str() );

		switch( HOOKS->MessageBoxAbortRetryIgnore(message) )
		{
		case ArchHooks::abort:
			RageException::Throw( message ); 
			break;
		case ArchHooks::retry:
			goto try_element_again;
		case ArchHooks::ignore:
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
		CString sNewPath = GetPathTo(category, sNewFileName, true);

		if( !sNewPath.empty() )
			return sNewPath;
		else
		{
			CString message = ssprintf(
					"ThemeManager:  The redirect '%s' points to the file '%s', which does not exist. "
					"Verify that this redirect is correct.",
					sPath.c_str(), sNewFileName.c_str());

			if( ArchHooks::retry == HOOKS->MessageBoxAbortRetryIgnore(message) )
				goto try_element_again;

			RageException::Throw( "%s", message.c_str() ); 
		}
	}
}

CString ThemeManager::GetPathTo( ElementCategory category, CString sFileName, bool bOptional ) 
{
	map<CString, CString> &Cache = g_ThemePathCache[category];
	{
		map<CString, CString>::const_iterator i;
		
		i = Cache.find( sFileName );
		if( i != Cache.end() )
			return i->second;
	}
	
	// TODO: Use HOOKS->MessageBox()
try_element_again:

	CString ret = GetPathTo( m_sCurThemeName, category, sFileName);
	if( !ret.empty() )	// we found something
	{
		Cache[sFileName] = ret;
		return ret;
	}

	ret = GetPathTo( BASE_THEME_NAME, category, sFileName);
	if( !ret.empty() )	// we found something
	{
		Cache[sFileName] = ret;
		return ret;
	}
	else if( bOptional )
	{
		Cache[sFileName] = "";
		return "";
	}

	CString sCategory = ELEMENT_CATEGORY_STRING[category];

	CString sMessage = "The theme element '" + sCategory + "/" + sFileName +"' is missing.";
	switch( HOOKS->MessageBoxAbortRetryIgnore(sMessage, "MissingThemeElement") )
	{
	case ArchHooks::retry:
		FlushDirCache();
		g_ThemePathCache[category].clear();
		goto try_element_again;
	case ArchHooks::ignore:
		LOG->Warn( 
			"Theme element '%s/%s' could not be found in '%s' or '%s'.", 
			sCategory.c_str(),
			sFileName.c_str(), 
			GetThemeDirFromName(m_sCurThemeName).c_str(), 
			GetThemeDirFromName(BASE_THEME_NAME).c_str() );

		/* Err? */
		if(sFileName == "_missing")
			RageException::Throw("'_missing' isn't present in '%s%s'", GetThemeDirFromName(BASE_THEME_NAME).c_str(), sCategory.c_str() );

		Cache[sFileName] = GetPathTo( category, "_missing" );
		return Cache[sFileName];
	case ArchHooks::abort:
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


CString ThemeManager::GetMetricsIniPath( CString sThemeName )
{
	return GetThemeDirFromName( sThemeName ) + METRICS_FILE;
}

bool ThemeManager::HasMetric( CString sClassName, CString sValueName )
{
	CString sThrowAway;
	return m_pIniMetrics->GetValue(sClassName,sValueName,sThrowAway);
}

void ThemeManager::ReloadMetricsIfNecessary()
{
	//
	// reload metrics if file has changed
	//
	CString sCurMetricPath = GetMetricsIniPath(m_sCurThemeName);
	CString sDefaultMetricPath = GetMetricsIniPath(BASE_THEME_NAME);

	if( m_uHashForCurThemeMetrics != GetHashForFile(sCurMetricPath)  ||
		m_uHashForBaseThemeMetrics != GetHashForFile(sDefaultMetricPath) )
	{
		ReloadMetrics();
	}
}

void ThemeManager::ReloadMetrics()
{
	SwitchThemeAndLanguage(m_sCurThemeName, m_sCurLanguage);	// force a reload of the metrics cache
	if( SCREENMAN )
		SCREENMAN->SystemMessage( "Reloaded metrics" );

	//
	// clear theme path cache
	//
	for( int i = 0; i < NUM_ELEMENT_CATEGORIES; ++i )
		g_ThemePathCache[i].clear();
}


CString ThemeManager::GetMetricRaw( CString sClassName, CString sValueName )
{
try_metric_again:

	CString sCurMetricPath = GetMetricsIniPath(m_sCurThemeName);
	CString sDefaultMetricPath = GetMetricsIniPath(BASE_THEME_NAME);

	CString sValue;
	if( m_pIniMetrics->GetValue(sClassName,sValueName,sValue) )
		return sValue;

	CString sMessage = ssprintf( "The theme metric '%s-%s' is missing.  Correct this and click Retry, or Cancel to break.",sClassName.c_str(),sValueName.c_str() );
	switch( HOOKS->MessageBoxAbortRetryIgnore(sMessage) )
	{
	case ArchHooks::abort:
		break;	// fall through
	case ArchHooks::retry:
		FlushDirCache();
		ReloadMetrics();
		goto try_metric_again;
	case ArchHooks::ignore:
		return "";
	default:
		ASSERT(0);
	}

	RageException::Throw( "Theme metric '%s : %s' could not be found in '%s' or '%s'.", 
		sClassName.c_str(),
		sValueName.c_str(),
		sCurMetricPath.c_str(), 
		sDefaultMetricPath.c_str()
		);
}

/* Get a string metric. */
CString ThemeManager::GetMetric( CString sClassName, CString sValueName )
{
	CString sValue = GetMetricRaw(sClassName,sValueName);

	// "::" means newline since you can't use line breaks in an ini file.
	sValue.Replace("::","\n");

	/* XXX: add a parameter to turn this off if there are some metrics where
	 * we don't want markers */
	FontCharAliases::ReplaceMarkers(sValue);

	return sValue;
}

int ThemeManager::GetMetricI( CString sClassName, CString sValueName )
{
	return atoi( GetMetricRaw(sClassName,sValueName) );
}

float ThemeManager::GetMetricF( CString sClassName, CString sValueName )
{
	return (float)atof( GetMetricRaw(sClassName,sValueName) );
}

bool ThemeManager::GetMetricB( CString sClassName, CString sValueName )
{
	return atoi( GetMetricRaw(sClassName,sValueName) ) != 0;
}

RageColor ThemeManager::GetMetricC( CString sClassName, CString sValueName )
{
	RageColor ret(1,1,1,1);
	if( !ret.FromString( GetMetricRaw(sClassName,sValueName) ) )
		LOG->Warn( "The color value '%s' for NoteSkin metric '%s : %s' is invalid.", GetMetricRaw(sClassName,sValueName).c_str(), sClassName.c_str(), sValueName.c_str() );
	return ret;
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

void ThemeManager::GetLanguagesForTheme( CString sThemeName, CStringArray& asLanguagesOut )
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

CString ThemeManager::GetLanguageIniPath( CString sThemeName, CString sLanguage )
{
	return GetThemeDirFromName(sThemeName) + LANGUAGES_SUBDIR + sLanguage + ".ini";
}
