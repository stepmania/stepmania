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
	m_uNextReloadTicks = 0;

	m_sCurThemeName = BASE_THEME_NAME;	// Use te base theme for now.  It's up to PrefsManager to change this.

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
	static const char *font_masks[] = { " 16x16.png", ".redir", NULL };
	static const char *numbers_masks[] = { " 5x3.png", ".redir", NULL };
	const char **asset_masks = NULL;
	if( sAssetCategory == "graphics" ) asset_masks = graphic_masks;
	else if( sAssetCategory == "sounds" ) asset_masks = sound_masks;
	else if( sAssetCategory == "fonts" ) asset_masks = font_masks;
	else if( sAssetCategory == "numbers" ) asset_masks = numbers_masks;
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
//			CString sNewFilePath = sDir+"\\"+sNewFileName; // This is what it used to be, FONT redirs were getting extra slashes
		// at the start of their file names, so I took out this extra slash - Andy.
		file.Close();
		CString sNewFilePath = sDir+sNewFileName;
		if( sNewFileName == ""  ||  !DoesFileExist(sNewFilePath) )
		{
#ifdef _DEBUG
			if( IDRETRY == AfxMessageBox( ssprintf("The redirect '%s' points to the file '%s', which does not exist.  Verify that this redirect is correct.", sRedirFilePath, sNewFilePath), MB_RETRYCANCEL ) )
				goto try_element_again;
#endif
			throw RageException( "The redirect '%s' points to the file '%s', which does not exist.  Verify that this redirect is correct.", sRedirFilePath, sNewFilePath ); 
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
	if (m_uNextReloadTicks == 0 || ::GetTickCount() > m_uNextReloadTicks)
	{
		m_uNextReloadTicks = GetTickCount()+1000;
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
	if( IDRETRY == AfxMessageBox( ssprintf("The theme metric %s-%s is missing.  Correct this and click Retry, or Cancel to break.",sClassName,sValueName), MB_RETRYCANCEL ) )
		goto try_metric_again;
#endif

	throw RageException( "Theme metric '%s : %s' could not be found in '%s' or '%s'.", 
		sClassName,
		sValueName,
		sCurMetricPath, 
		sDefaultMetricPath
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

D3DXCOLOR ThemeManager::GetMetricC( CString sClassName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(sClassName,sValueName);
	char szValue[40];
	strncpy( szValue, sValue, 39 );
	int result = sscanf( szValue, "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for NoteSkin metric '%s : %s' is invalid.", szValue, sClassName, sValueName );
		ASSERT(0);
	}

	return D3DXCOLOR(r,g,b,a);
}
