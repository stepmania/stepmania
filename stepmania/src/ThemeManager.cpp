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


ThemeManager*	THEME = NULL;	// global object accessable from anywhere in the program


const CString BASE_THEME_NAME = "default";
const CString THEMES_DIR  = "Themes\\";

ThemeManager::ThemeManager()
{
	m_pIniMetrics = new IniFile;

	/* Update the metric cache on the first call to GetMetric. */
	NextReloadCheck = -1;

	m_sCurThemeName = BASE_THEME_NAME;	// Use te base theme for now.  It's up to PrefsManager to change this.

	CStringArray arrayThemeNames;
	GetAllThemeNames( arrayThemeNames );

	// Disabled for now... it takes ages here to run - bbf
//	for( int i=0; i<arrayThemeNames.GetSize(); i++ )
//		AssertThemeIsComplete( arrayThemeNames[i] );
}

ThemeManager::~ThemeManager()
{
	delete m_pIniMetrics;
}

void ThemeManager::GetAllThemeNames( CStringArray& AddTo )
{
	GetDirListing( THEMES_DIR+"\\*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
		if( 0 == stricmp(AddTo[i],"cvs") )
			AddTo.RemoveAt(i);
}

void ThemeManager::GetThemeNamesForCurGame( CStringArray& AddTo )
{
	GetAllThemeNames( AddTo );

	/*
	// strip out announcers that don't have the current game name in them
	CString sGameName = GAMESTATE->GetCurrentGameDef()->m_szName;
	sGameName.MakeLower();
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
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
	for( int i=0; i<asThemeNames.GetSize(); i++ )
	{
		if( 0==stricmp(sThemeName, asThemeNames[i]) )
			return true;
	}
	return false;
}

void ThemeManager::SwitchTheme( CString sThemeName )
{
	if( 0==stricmp(BASE_THEME_NAME, sThemeName) )
		m_sCurThemeName = "";	// can't select the base theme
	else if( !DoesThemeExist(sThemeName) )
		m_sCurThemeName = BASE_THEME_NAME;
	else
		m_sCurThemeName = sThemeName;

	// update hashes for metrics files
	m_iHashForCurThemeMetrics = GetHashForFile( GetMetricsPathFromName(m_sCurThemeName) );
	m_iHashForBaseThemeMetrics = GetHashForFile( GetMetricsPathFromName(BASE_THEME_NAME) );

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
	if( asPossibleElementFilePaths.GetSize() > 0 )
	{
		CStdioFile file;
		file.Open( asPossibleElementFilePaths[0], CFile::modeRead );
		CString sLine;
		file.ReadString( sLine );
	}


	///////////////////////////////////////
	// Search both the current theme and the default theme dirs for this element
	///////////////////////////////////////
	static const char *graphic_masks[] = {
		"*.sprite", "*.png", "*.jpg", "*.bmp", "*.gif", "*.redir",
		"*.avi", "*.mpg", "*.mpeg", NULL
	};
	static const char *sound_masks[] = { ".set", ".mp3", ".ogg", ".wav", ".redir", NULL };
	static const char *font_masks[] = { ".font", ".redir", NULL };
	const char **asset_masks = NULL;
	if( sAssetCategory == "graphics" ) asset_masks = graphic_masks;
	else if( sAssetCategory == "sounds" ) asset_masks = sound_masks;
	else if( sAssetCategory == "fonts" ) asset_masks = font_masks;
	else ASSERT(0); // Unknown theme asset category
	int i;
	for(i = 0; asset_masks[i]; ++i)
		GetDirListing( sCurrentThemeDir + sAssetCategory+"\\"+sFileName + asset_masks[i],
						asPossibleElementFilePaths, false, true );
	for(i = 0; asset_masks[i]; ++i)
		GetDirListing( sDefaultThemeDir + sAssetCategory+"\\"+sFileName + asset_masks[i],
						asPossibleElementFilePaths, false, true );

	if( asPossibleElementFilePaths.GetSize() == 0 )
	{
#ifdef _DEBUG
		if( IDRETRY == AfxMessageBox( ssprintf("The theme element %s/%s is missing.  Correct this and click Retry, or Cancel to break.",sAssetCategory,sFileName), MB_RETRYCANCEL ) )
			goto try_element_again;
#endif
		throw RageException( "Theme element '%s/%s' could not be found in '%s' or '%s'.", 
			sAssetCategory,
			sFileName, 
			GetThemeDirFromName(m_sCurThemeName), 
			GetThemeDirFromName(BASE_THEME_NAME) );
	}
	else
	{
		asPossibleElementFilePaths[0].MakeLower();
		if( asPossibleElementFilePaths[0].GetLength() > 5  &&  asPossibleElementFilePaths[0].Right(5) == "redir" )	// this is a redirect file
		{
			CString sRedirFilePath = asPossibleElementFilePaths[0];
			
			CString sDir, sFName, sExt;
			splitrelpath( sRedirFilePath, sDir, sFName, sExt );

			CStdioFile file;
			file.Open( sRedirFilePath, CFile::modeRead );
			CString sNewFileName;
			file.ReadString( sNewFileName );
			CString sNewFilePath = sDir+"\\"+sNewFileName;
			if( sNewFileName == ""  ||  !DoesFileExist(sNewFilePath) )
			{
				throw RageException( "The redirect '%s' points to the file '%s', which does not exist.  Verify that this redirect is correct.", sRedirFilePath, sNewFilePath ); 
			}
			else
				return sNewFilePath;
		}
		else
		{
			return asPossibleElementFilePaths[0];
		}
	}

	return "";
}


CString ThemeManager::GetMetricsPathFromName( CString sThemeName )
{
	return GetThemeDirFromName( sThemeName ) + "metrics.ini";
}

CString ThemeManager::GetMetric( CString sScreenName, CString sValueName )
{
#ifdef _DEBUG
try_metric_again:
#endif
	CString sCurMetricPath = GetMetricsPathFromName(m_sCurThemeName);
	CString sDefaultMetricPath = GetMetricsPathFromName(BASE_THEME_NAME);

	// Is our metric cache out of date?
	if (NextReloadCheck == -1 || TIMER->GetTimeSinceStart() > NextReloadCheck)
	{
		NextReloadCheck = TIMER->GetTimeSinceStart()+1.0f;
		if( m_iHashForCurThemeMetrics != GetHashForFile(sCurMetricPath)  ||
			m_iHashForBaseThemeMetrics != GetHashForFile(sDefaultMetricPath) )
		{
			SwitchTheme(m_sCurThemeName);	// force a reload of the metrics cache
		}
	}

	CString sValue;
	if( m_pIniMetrics->GetValue(sScreenName,sValueName,sValue) )
		return sValue;

#ifdef _DEBUG
	if( IDRETRY == AfxMessageBox( ssprintf("The theme metric %s-%s is missing.  Correct this and click Retry, or Cancel to break.",sScreenName,sValueName), MB_RETRYCANCEL ) )
		goto try_metric_again;
#endif

	throw RageException( "Theme metric '%s : %s' could not be found in '%s' or '%s'.", 
		sScreenName,
		sValueName,
		sCurMetricPath, 
		sDefaultMetricPath
		);
}

int ThemeManager::GetMetricI( CString sScreenName, CString sValueName )
{
	return atoi( GetMetric(sScreenName,sValueName) );
}

float ThemeManager::GetMetricF( CString sScreenName, CString sValueName )
{
	return (float)atof( GetMetric(sScreenName,sValueName) );
}

bool ThemeManager::GetMetricB( CString sScreenName, CString sValueName )
{
	return atoi( GetMetric(sScreenName,sValueName) ) != 0;
}

D3DXCOLOR ThemeManager::GetMetricC( CString sScreenName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(sScreenName,sValueName);
	char szValue[40];
	strcpy( szValue, sValue );
	int result = sscanf( szValue, "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for theme metric '%s : %s' is invalid.", szValue, sScreenName, sValueName );
		ASSERT(0);
	}

	return D3DXCOLOR(r,g,b,a);
}
