#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: ThemeManager

 Desc: Manages which graphics and sounds are chosed to load.  Every time 
	a sound or graphic is loaded, it gets the path from the ThemeManager.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"
#include "RageTypes.h"

class IniFile;

class ThemeManager
{
public:
	ThemeManager();
	~ThemeManager();

	void GetThemeNamesForCurGame( CStringArray& AddTo );
	bool DoesThemeExist( CString sThemeName );
	void SwitchTheme( CString sThemeName );
	CString GetCurThemeName() { return m_sCurThemeName; };

	CString GetPathTo( CString sAssetCategory, CString sFileName );	// looks up the current theme in PREFSMAN
	CString GetPathToOptional( CString sAssetCategory, CString sFileName );	// looks up the current theme in PREFSMAN

	CString		GetMetric( CString sClassName, CString sValueName );
	int			GetMetricI( CString sClassName, CString sValueName );
	float		GetMetricF( CString sClassName, CString sValueName );
	bool		GetMetricB( CString sClassName, CString sValueName );
	RageColor	GetMetricC( CString sClassName, CString sValueName );

protected:
	void GetAllThemeNames( CStringArray& AddTo );

	static CString GetThemeDirFromName( const CString &sThemeName );
	CString GetElementDir( CString sThemeName );
	static CString GetMetricsPathFromName( CString sThemeName );

	CString m_sCurThemeName;

	IniFile* m_pIniMetrics;	// make this a pointer so we don't have to include IniFile in this header!
	float m_fNextReloadTicks;
	unsigned m_uHashForCurThemeMetrics;
	unsigned m_uHashForBaseThemeMetrics;
};



extern ThemeManager*	THEME;	// global and accessable from anywhere in our program

#endif
