#include "stdafx.h"
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

#include <fstream>
using namespace std;

ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


const CString BASE_THEME_NAME = "default";
const CString THEMES_DIR  = "Themes\\";

ThemeManager::ThemeManager()
{
	m_pIniMetrics = new IniFile;

	/* Update the metric cache on the first call to GetMetric. */
	m_fNextReloadTicks = 0;

	m_sCurThemeName = BASE_THEME_NAME;	// Use the base theme for now.  It's up to PrefsManager to change this.

	CStringArray arrayThemeNames;
	GetAllThemeNames( arrayThemeNames );
}

ThemeManager::~ThemeManager()
{
	delete m_pIniMetrics;
}

void ThemeManager::GetAllThemeNames( CStringArray& AddTo )
{
	GetDirListing( THEMES_DIR+"\\*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( CStringArray::iterator i=AddTo.begin(); i != AddTo.end(); ++i )
	{
		if( !stricmp(*i,"cvs") ) {
			AddTo.erase(i, i+1);
			break;
		}
	}
}

void ThemeManager::GetThemeNamesForCurGame( CStringArray& AddTo )
{
	GetAllThemeNames( AddTo );

	/*
	// strip out announcers that don't have the current game name in them
	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
	sGameName.MakeLower();
	for( unsigned i=AddTo.size()-1; i>=0; i-- )
	{
		CString sLowercaseVer = AddTo[i];
		sLowercaseVer.MakeLower();
		if( sLowercaseVer.Find(sGameName)==-1 )
			AddTo.RemoveAt(i);
	}
	*/
}

bool ThemeManager::DoesThemeExist( CString sThemeName )
{
	CStringArray asThemeNames;	
	GetAllThemeNames( asThemeNames );
	for( unsigned i=0; i<asThemeNames.size(); i++ )
	{
		if( 0==stricmp(sThemeName, asThemeNames[i]) )
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
}

CString ThemeManager::GetThemeDirFromName( CString sThemeName )
{
	return THEMES_DIR + sThemeName + "\\";
}

CString ThemeManager::GetPathTo( CString sAssetCategory, CString sFileName ) 
{
#ifdef _DEBUG
try_element_again:
#endif

	sAssetCategory.MakeLower();
	sFileName.MakeLower();

	const CString sCurrentThemeDir = GetThemeDirFromName( m_sCurThemeName );
	const CString sDefaultThemeDir = GetThemeDirFromName( BASE_THEME_NAME );	

	CStringArray asPossibleElementFilePaths;

	// look for a redirect
	GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + "*.redir", asPossibleElementFilePaths, false, true );
	if( !asPossibleElementFilePaths.empty() )
	{
		ifstream file(asPossibleElementFilePaths[0]);
		CString sLine;
		getline(file, sLine);
	}


	///////////////////////////////////////
	// Search both the current theme and the default theme dirs for this element
	///////////////////////////////////////
	static const char *graphic_masks[] = {
		"*.sprite", "*.png", "*.jpg", "*.bmp", "*.gif", "*.redir",
		"*.avi", "*.mpg", "*.mpeg", NULL
	};
	static const char *sound_masks[] = { ".set", ".mp3", ".ogg", ".wav", ".redir", NULL };
	static const char *font_masks[] = { " 16x16.png", ".redir", NULL };
	static const char *numbers_masks[] = { " 5x3.png", ".redir", NULL };
	static const char *bganimations_masks[] = { ".", ".redir", NULL };
	const char **asset_masks = NULL;
	if( sAssetCategory == "graphics" ) asset_masks = graphic_masks;
	else if( sAssetCategory == "sounds" ) asset_masks = sound_masks;
	else if( sAssetCategory == "fonts" ) asset_masks = font_masks;
	else if( sAssetCategory == "numbers" ) asset_masks = numbers_masks;
	else if( sAssetCategory == "bganimations" ) asset_masks = bganimations_masks;
	else ASSERT(0); // Unknown theme asset category
	int i;
	for(i = 0; asset_masks[i]; ++i)
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + asset_masks[i],
						asPossibleElementFilePaths, false, true );
	for(i = 0; asset_masks[i]; ++i)
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + asset_masks[i],
						asPossibleElementFilePaths, false, true );

	if( asPossibleElementFilePaths.empty() )
	{
#ifdef _DEBUG
		CString sMessage = ssprintf("The theme element %s/%s is missing.",sAssetCategory.GetString(),sFileName.GetString());
		switch( MessageBox(NULL, sMessage, "ThemeManager", MB_ABORTRETRYIGNORE ) )
		{
		case IDRETRY:
			goto try_element_again;
			break;
		case IDABORT:
#endif
			RageException::Throw( "Theme element '%s/%s' could not be found in '%s' or '%s'.", 
				sAssetCategory.GetString(),
				sFileName.GetString(), 
				GetThemeDirFromName(m_sCurThemeName).GetString(), 
				GetThemeDirFromName(BASE_THEME_NAME).GetString() );
#ifdef _DEBUG
		case IDIGNORE:
			LOG->Warn( 
				"Theme element '%s/%s' could not be found in '%s' or '%s'.", 
				sAssetCategory.GetString(),
				sFileName.GetString(), 
				GetThemeDirFromName(m_sCurThemeName), 
				GetThemeDirFromName(BASE_THEME_NAME) );
			return GetPathTo( sAssetCategory, "_missing" );
			break;
		}
#endif
	}
	asPossibleElementFilePaths[0].MakeLower();
	if( asPossibleElementFilePaths[0].GetLength() > 5  &&  asPossibleElementFilePaths[0].Right(5) == "redir" )	// this is a redirect file
	{
		CString sRedirFilePath = asPossibleElementFilePaths[0];
		
		CString sDir, sFName, sExt;
		splitrelpath( sRedirFilePath, sDir, sFName, sExt );

		CString sNewFileName;
		{
			ifstream file(sRedirFilePath);
			getline(file, sNewFileName );
//			CString sNewFilePath = sDir+"\\"+sNewFileName; // This is what it used to be, FONT redirs were getting extra slashes
		// at the start of their file names, so I took out this extra slash - Andy.
		}

		CString sNewFilePath = sDir+sNewFileName;
		if( sNewFileName == ""  ||  !DoesFileExist(sNewFilePath) )
		{
#ifdef _DEBUG
			if( IDRETRY == MessageBox(NULL,"ThemeManager",ssprintf("The redirect '%s' points to the file '%s', which does not exist.  Verify that this redirect is correct.", sRedirFilePath.GetString(), sNewFilePath.GetString()), MB_RETRYCANCEL ) )
				goto try_element_again;
#endif
			RageException::Throw( "The redirect '%s' points to the file '%s', which does not exist.  Verify that this redirect is correct.", sRedirFilePath.GetString(), sNewFilePath.GetString() ); 
		}
		else
			return sNewFilePath;
	}

	return asPossibleElementFilePaths[0];
}


CString ThemeManager::GetMetricsPathFromName( CString sThemeName )
{
	return GetThemeDirFromName( sThemeName ) + "metrics.ini";
}

CString ThemeManager::GetMetric( CString sClassName, CString sValueName )
{
#ifdef _DEBUG
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
			SwitchTheme(m_sCurThemeName);	// force a reload of the metrics cache
		}
	}

	CString sValue;
	if( m_pIniMetrics->GetValue(sClassName,sValueName,sValue) )
	{
		sValue.Replace("::","\n");	// "::" means newline since you can't use line breaks in an ini file.
		return sValue;
	}

#ifdef _DEBUG
	if( IDRETRY == MessageBox(NULL,ssprintf("The theme metric %s-%s is missing.  Correct this and click Retry, or Cancel to break.",sClassName.GetString(),sValueName.GetString()),"ThemeManager",MB_RETRYCANCEL ) )
		goto try_metric_again;
#endif

	RageException::Throw( "Theme metric '%s : %s' could not be found in '%s' or '%s'.", 
		sClassName.GetString(),
		sValueName.GetString(),
		sCurMetricPath.GetString(), 
		sDefaultMetricPath.GetString()
		);
}

int ThemeManager::GetMetricI( CString sClassName, CString sValueName )
{
	return atoi( GetMetric(sClassName,sValueName) );
}

float ThemeManager::GetMetricF( CString sClassName, CString sValueName )
{
	return (float)atof( GetMetric(sClassName,sValueName) );
}

bool ThemeManager::GetMetricB( CString sClassName, CString sValueName )
{
	return atoi( GetMetric(sClassName,sValueName) ) != 0;
}

RageColor ThemeManager::GetMetricC( CString sClassName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(sClassName,sValueName);
	char szValue[40];
	strncpy( szValue, sValue, 39 );
	int result = sscanf( szValue, "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for NoteSkin metric '%s : %s' is invalid.", szValue, sClassName.GetString(), sValueName.GetString() );
		ASSERT(0);
	}

	return RageColor(r,g,b,a);
}
