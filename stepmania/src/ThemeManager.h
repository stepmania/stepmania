#pragma once
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
#include "D3DX8Math.h"	// for D3DXCOLOR
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

	CString		GetMetric( CString sScreenName, CString sValueName );
	int			GetMetricI( CString sScreenName, CString sValueName );
	float		GetMetricF( CString sScreenName, CString sValueName );
	bool		GetMetricB( CString sScreenName, CString sValueName );
	D3DXCOLOR	GetMetricC( CString sScreenName, CString sValueName );

protected:
	void GetAllThemeNames( CStringArray& AddTo );

	static CString GetThemeDirFromName( CString sThemeName );
	CString GetElementDir( CString sThemeName );
	static CString GetMetricsPathFromName( CString sThemeName );

	CString m_sCurThemeName;

	IniFile* m_pIniMetrics;	// make this a pointer so we don't have to include IniFile in this header!
	DWORD m_uNextReloadTicks;
	int m_iHashForCurThemeMetrics;
	int m_iHashForBaseThemeMetrics;
};



extern ThemeManager*	THEME;	// global and accessable from anywhere in our program
