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


ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


const CString BASE_THEME_NAME = "default";
const CString THEMES_DIR  = "Themes/";
const CString ELEMENT_CATEGORY_STRING[NUM_ELEMENT_CATEGORIES] =
{
	"BGAnimations",
	"Fonts",
	"Graphics",
	"Numbers",
	"Sounds"
};

ThemeManager::ThemeManager()
{
	m_pIniMetrics = new IniFile;

	/* Update the metric cache on the first call to GetMetric. */
	m_fNextReloadTicks = 0;

	m_sCurThemeName = BASE_THEME_NAME;	// Use the base theme for now.  It's up to PrefsManager to change this.
	m_uHashForCurThemeMetrics = m_uHashForBaseThemeMetrics = 0;
	
	CStringArray arrayThemeNames;
	GetThemeNames( arrayThemeNames );
}

ThemeManager::~ThemeManager()
{
	delete m_pIniMetrics;
}

void ThemeManager::GetThemeNames( CStringArray& AddTo )
{
	GetDirListing( THEMES_DIR+"/*", AddTo, true );
	
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

void ThemeManager::SwitchTheme( CString sThemeName )
{
	if( !DoesThemeExist(sThemeName) )
		m_sCurThemeName = BASE_THEME_NAME;
	else
		m_sCurThemeName = sThemeName;

	// update hashes for metrics files
	m_uHashForCurThemeMetrics = GetHashForFile( GetMetricsPathFromName(m_sCurThemeName) );
	m_uHashForBaseThemeMetrics = GetHashForFile( GetMetricsPathFromName(BASE_THEME_NAME) );

	// read new metrics.  First read base metrics, then read cur theme's metrics, overriding base theme
	m_pIniMetrics->Reset();
	m_pIniMetrics->SetPath( GetMetricsPathFromName(BASE_THEME_NAME) );
	m_pIniMetrics->ReadFile();
	m_pIniMetrics->SetPath( GetMetricsPathFromName(m_sCurThemeName) );
	m_pIniMetrics->ReadFile();

	LOG->Info("Set theme %s", sThemeName.c_str());
}

CString ThemeManager::GetThemeDirFromName( const CString &sThemeName )
{
	return THEMES_DIR + sThemeName + "/";
}

CString ThemeManager::GetPathTo( CString sThemeName, ElementCategory category, CString sFileName ) 
{
#if defined(WIN32) // XXX arch?
try_element_again:
#endif
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
		GetDirListing( sThemeDir + sCategory+"/"+sFileName + ".redir",
						asElementPaths, false, true );

		static const char *masks[NUM_ELEMENT_CATEGORIES][12] = {
			{ "", NULL },
			{ "*.ini", NULL },
			{"*.sprite", "*.png", "*.jpg", "*.bmp", "*.gif","*.avi", "*.mpg", "*.mpeg", NULL},
			{ "*.png", NULL },
			{ ".set", ".mp3", ".ogg", ".wav", NULL },
		};		
		const char **asset_masks = masks[category];

		for( int i = 0; asset_masks[i]; ++i )
			GetDirListing( sThemeDir + sCategory+"/" + sFileName + asset_masks[i],
							asElementPaths, bDirsOnly, true );
		if( category == Fonts )
			Font::WeedFontNames(asElementPaths, sFileName);
	}
	

	if( asElementPaths.size() > 1 )
	{
		FDB.FlushDirCache();

		CString message = ssprintf( 
			"There is more than one theme element element that matches "
			"'%s/%s/%s'.  Please remove all but one of these matches.",
			sThemeName.GetString(), sCategory.GetString(), sFileName.GetString() );
							
#if defined(WIN32) // XXX arch?
		if( DISPLAY->IsWindowed() )
			if( MessageBox(NULL, message, "ThemeManager", MB_RETRYCANCEL ) == IDRETRY)
				goto try_element_again;
#endif
		RageException::Throw( message ); 
	}
	else if( asElementPaths.size() == 0 )
	{
		return "";	// This isn't fatal.
	}
	else	// asElementPaths.size() == 1
	{
		ASSERT( asElementPaths.size() == 1 );

		CString sPath = asElementPaths[0];
		bool bIsARedirect = sPath.length()>6  &&  sPath.Right(6).CompareNoCase(".redir")==0;

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
						"The redirect '%s' points to the file '%s', which does not exist. "
						"Verify that this redirect is correct.",
						sPath.GetString(), sNewFileName.GetString());

#if defined(WIN32) // XXX arch?
				if( DISPLAY->IsWindowed() )
					if( MessageBox(NULL, message, "ThemeManager", MB_RETRYCANCEL ) == IDRETRY)
						goto try_element_again;
#endif
				RageException::Throw( "%s", message.GetString() ); 
			}
		}
	}
}

CString ThemeManager::GetPathTo( ElementCategory category, CString sFileName, bool bOptional ) 
{
#if defined(DEBUG) && defined(WIN32)
try_element_again:
#endif

	CString ret = GetPathTo( m_sCurThemeName, category, sFileName);
	if( !ret.empty() )	// we found something
		return ret;
	ret = GetPathTo( BASE_THEME_NAME, category, sFileName);
	if( !ret.empty() )	// we found something
		return ret;
	else if( bOptional )
		return "";

	CString sCategory = ELEMENT_CATEGORY_STRING[category];

#if defined(DEBUG) && defined(WIN32)
	CString sMessage = ssprintf("The theme element '%s/%s' is missing.",sCategory.GetString(),sFileName.GetString());
	switch( MessageBox(NULL, sMessage, "ThemeManager", MB_RETRYCANCEL ) )
	{
	case IDRETRY:
		FDB.FlushDirCache();
		goto try_element_again;
	case IDCANCEL:
		RageException::Throw( "Theme element '%s/%s' could not be found in '%s' or '%s'.", 
			sCategory.GetString(),
			sFileName.GetString(), 
			GetThemeDirFromName(m_sCurThemeName).GetString(), 
			GetThemeDirFromName(BASE_THEME_NAME).GetString() );
		break;
	}
#endif

	LOG->Warn( 
		"Theme element '%s/%s' could not be found in '%s' or '%s'.", 
		sCategory.GetString(),
		sFileName.GetString(), 
		GetThemeDirFromName(m_sCurThemeName).GetString(), 
		GetThemeDirFromName(BASE_THEME_NAME).GetString() );

	/* Err? */
	if(sFileName == "_missing")
		RageException::Throw("_missing element missing from %s/%s", GetThemeDirFromName(BASE_THEME_NAME).GetString(), sCategory.GetString() );
	return GetPathTo( category, "_missing" );
}


CString ThemeManager::GetMetricsPathFromName( CString sThemeName )
{
	return GetThemeDirFromName( sThemeName ) + "metrics.ini";
}

bool ThemeManager::HasMetric( CString sClassName, CString sValueName )
{
	CString sThrowAway;
	return m_pIniMetrics->GetValue(sClassName,sValueName,sThrowAway);
}

CString ThemeManager::GetMetricRaw( CString sClassName, CString sValueName )
{
#if defined(DEBUG) && defined(WIN32)
try_metric_again:
#endif
	CString sCurMetricPath = GetMetricsPathFromName(m_sCurThemeName);
	CString sDefaultMetricPath = GetMetricsPathFromName(BASE_THEME_NAME);

	// Is our metric cache out of date?
	// XXX: GTSS wraps every ~40 days.  Need a better way to handler timers like this.
	if (RageTimer::GetTimeSinceStart() >= m_fNextReloadTicks)
	{
		m_fNextReloadTicks = RageTimer::GetTimeSinceStart()+1.0f;
		if( m_uHashForCurThemeMetrics != GetHashForFile(sCurMetricPath)  ||
			m_uHashForBaseThemeMetrics != GetHashForFile(sDefaultMetricPath) )
		{
			if(m_uHashForBaseThemeMetrics)
			{
				LOG->Warn( "Metrics file is out of date.  Reloading..." );
//				MessageBeep( MB_OK );
			}
			SwitchTheme(m_sCurThemeName);	// force a reload of the metrics cache
		}
	}

	CString sValue;
	if( m_pIniMetrics->GetValue(sClassName,sValueName,sValue) )
		return sValue;

#if defined(DEBUG) && defined(WIN32)
	if( IDRETRY == MessageBox(NULL,ssprintf("The theme metric '%s-%s' is missing.  Correct this and click Retry, or Cancel to break.",sClassName.GetString(),sValueName.GetString()),"ThemeManager",MB_RETRYCANCEL ) )
		goto try_metric_again;
#endif

	RageException::Throw( "Theme metric '%s : %s' could not be found in '%s' or '%s'.", 
		sClassName.GetString(),
		sValueName.GetString(),
		sCurMetricPath.GetString(), 
		sDefaultMetricPath.GetString()
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
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetricRaw(sClassName,sValueName);
	int result = sscanf( GetMetricRaw(sClassName,sValueName), "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for NoteSkin metric '%s : %s' is invalid.", GetMetricRaw(sClassName,sValueName).GetString(), sClassName.GetString(), sValueName.GetString() );
		ASSERT(0);
	}

	return RageColor(r,g,b,a);
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
	SwitchTheme( as[iNewIndex] );
}
