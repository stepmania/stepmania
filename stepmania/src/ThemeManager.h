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

#include "RageTypes.h"

class IniFile;

enum ElementCategory { BGAnimations, Fonts, Graphics, Numbers, Sounds, NUM_ELEMENT_CATEGORIES };

class ThemeManager
{
public:
	ThemeManager();
	~ThemeManager();

	void GetThemeNames( CStringArray& AddTo );
	bool DoesThemeExist( CString sThemeName );
	void SwitchTheme( CString sThemeName );
	CString GetCurThemeName() { return m_sCurThemeName; };
	CString GetCurThemeDir() { return GetThemeDirFromName(m_sCurThemeName); };
	void NextTheme();

	CString GetPathTo( ElementCategory category, CString sFileName, bool bOptional=false );
	CString GetPathToB( CString sFileName ) { return GetPathTo(BGAnimations,sFileName); };
	CString GetPathToF( CString sFileName ) { return GetPathTo(Fonts,sFileName); };
	CString GetPathToG( CString sFileName ) { return GetPathTo(Graphics,sFileName); };
	CString GetPathToN( CString sFileName ) { return GetPathTo(Numbers,sFileName); };
	CString GetPathToS( CString sFileName ) { return GetPathTo(Sounds,sFileName); };

	bool		HasMetric( CString sClassName, CString sValueName );
	CString		GetMetricRaw( CString sClassName, CString sValueName );
	CString		GetMetric( CString sClassName, CString sValueName );
	int			GetMetricI( CString sClassName, CString sValueName );
	float		GetMetricF( CString sClassName, CString sValueName );
	bool		GetMetricB( CString sClassName, CString sValueName );
	RageColor	GetMetricC( CString sClassName, CString sValueName );

protected:
	CString GetPathTo( CString sThemeName, ElementCategory category, CString sFileName );
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

class CachedThemeMetric
{
protected:
	CString m_sName;
	CString m_sValueName;
	bool	m_bInited;

	CString		m_sValue;
	int			m_iValue;
	float		m_fValue;
	bool		m_bValue;
	RageColor	m_cValue;

public:
	CachedThemeMetric( CString sClassName, CString sValueName )
	{
		m_sName = sClassName;
		m_sValueName = sValueName;
		m_bInited = false;
	}

	void Refresh()
	{
		m_sValue = THEME->GetMetric(m_sName,m_sValueName);
		m_iValue = atoi( m_sValue );
		m_fValue = (float)atof( m_sValue );
		m_bValue = atoi( m_sValue ) != 0;
		m_cValue = RageColor(1,1,1,1);
		sscanf( m_sValue, "%f,%f,%f,%f", &m_cValue.r, &m_cValue.g, &m_cValue.b, &m_cValue.a );
		m_bInited = true;
	}

    operator const CString () const		{ ASSERT(m_bInited);	return m_sValue; };
};

class CachedThemeMetricF : public CachedThemeMetric
{
public:
	CachedThemeMetricF( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
    operator const float () const		{ ASSERT(m_bInited);	return m_fValue; };
};

class CachedThemeMetricI : public CachedThemeMetric
{
public:
	CachedThemeMetricI( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
	operator const int () const			{ ASSERT(m_bInited);	return m_iValue; };
};

class CachedThemeMetricB : public CachedThemeMetric
{
public:
	CachedThemeMetricB( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
    operator const bool () const		{ ASSERT(m_bInited);	return m_bValue; };
};

class CachedThemeMetricC : public CachedThemeMetric
{
public:
	CachedThemeMetricC( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
    operator const RageColor () const	{ ASSERT(m_bInited);	return m_cValue; };
};



#endif
