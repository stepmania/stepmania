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
#include "RageTimer.h"

class IniFile;

enum ElementCategory { BGAnimations, Fonts, Graphics, Numbers, Sounds, Other, NUM_ELEMENT_CATEGORIES };

class ThemeManager
{
public:
	ThemeManager();
	~ThemeManager();

	void GetThemeNames( CStringArray& AddTo );
	bool DoesThemeExist( CString sThemeName );
	void GetLanguages( CStringArray& AddTo );
	bool DoesLanguageExist( CString sLanguage );
	void SwitchThemeAndLanguage( CString sThemeName, CString sLanguage );
	CString GetCurThemeName() { return m_sCurThemeName; };
	CString GetCurLanguage() { return m_sCurLanguage; };
	CString GetCurThemeDir() { return GetThemeDirFromName(m_sCurThemeName); };
	void NextTheme();
	void ReloadMetrics();
	void ReloadMetricsIfNecessary();

	CString GetPathTo( ElementCategory category, CString sClassName, CString sElement, bool bOptional=false );
	CString GetPathToB( CString sClassName, CString sElement, bool bOptional=false ) { return GetPathTo(BGAnimations,sClassName,sElement,bOptional); };
	CString GetPathToF( CString sClassName, CString sElement, bool bOptional=false ) { return GetPathTo(Fonts,sClassName,sElement,bOptional); };
	CString GetPathToG( CString sClassName, CString sElement, bool bOptional=false ) { return GetPathTo(Graphics,sClassName,sElement,bOptional); };
	CString GetPathToN( CString sClassName, CString sElement, bool bOptional=false ) { return GetPathTo(Numbers,sClassName,sElement,bOptional); };
	CString GetPathToS( CString sClassName, CString sElement, bool bOptional=false ) { return GetPathTo(Sounds,sClassName,sElement,bOptional); };
	CString GetPathToO( CString sClassName, CString sElement, bool bOptional=false ) { return GetPathTo(Other,sClassName,sElement,bOptional); };

	// TODO: remove these and update the places that use them
	CString GetPathToB( CString sFileName, bool bOptional=false );
	CString GetPathToF( CString sFileName, bool bOptional=false );
	CString GetPathToG( CString sFileName, bool bOptional=false );
	CString GetPathToN( CString sFileName, bool bOptional=false );
	CString GetPathToS( CString sFileName, bool bOptional=false );
	CString GetPathToO( CString sFileName, bool bOptional=false );


	bool		HasMetric( CString sClassName, CString sValueName );
	CString		GetMetricRaw( CString sClassName, CString sValueName );
	CString		GetMetric( CString sClassName, CString sValueName );
	int			GetMetricI( CString sClassName, CString sValueName );
	float		GetMetricF( CString sClassName, CString sValueName );
	bool		GetMetricB( CString sClassName, CString sValueName );
	RageColor	GetMetricC( CString sClassName, CString sValueName );

protected:
	CString GetPathToAndFallback( CString sThemeName, ElementCategory category, CString sClassName, CString sFile );
	CString GetPathToRaw( CString sThemeName, ElementCategory category, CString sClassName, CString sFile );
	static CString GetThemeDirFromName( const CString &sThemeName );
	CString GetElementDir( CString sThemeName );
	static CString GetMetricsIniPath( CString sThemeName );
	static void GetLanguagesForTheme( CString sThemeName, CStringArray& asLanguagesOut );
	static CString GetLanguageIniPath( CString sThemeName, CString sLanguage );

	CString m_sCurThemeName;
	CString m_sCurLanguage;

	IniFile* m_pIniMetrics;	// make this a pointer so we don't have to include IniFile here
	unsigned m_uHashForCurThemeMetrics;
	unsigned m_uHashForBaseThemeMetrics;
	unsigned m_uHashForCurThemeCurLanguage;
	unsigned m_uHashForBaseThemeCurLanguage;
	unsigned m_uHashForCurThemeBaseLanguage;
	unsigned m_uHashForBaseThemeBaseLanguage;
};

extern ThemeManager*	THEME;	// global and accessable from anywhere in our program

class CachedThemeMetric
{
protected:
	CString m_sClassName;
	CString m_sValueName;
	bool	m_bInited;

	CString		m_sValue;

	virtual void Update() { }

public:
	CachedThemeMetric( CString sClassName, CString sValueName ):
		m_sClassName( sClassName ),
		m_sValueName( sValueName ),
		m_bInited( false )
	{
	}
	virtual ~CachedThemeMetric() { }

	void Refresh( CString sClassName = "" )
	{
		m_sValue = THEME->GetMetric(sClassName==""? m_sClassName:sClassName,m_sValueName);
		Update();
		m_bInited = true;
	}

    operator const CString () const		{ ASSERT(m_bInited);	return m_sValue; };
};

class CachedThemeMetricF : public CachedThemeMetric
{
	float		m_fValue;
public:
	void Update() { m_fValue = (float)atof( m_sValue ); }
	CachedThemeMetricF( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
    operator const float () const		{ ASSERT(m_bInited);	return m_fValue; };
};

class CachedThemeMetricI : public CachedThemeMetric
{
	int			m_iValue;
public:
	void Update() { m_iValue = atoi( m_sValue ); }
	CachedThemeMetricI( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
	operator const int () const			{ ASSERT(m_bInited);	return m_iValue; };
};

class CachedThemeMetricB : public CachedThemeMetric
{
	bool		m_bValue;
public:
	void Update() { m_bValue = atoi( m_sValue ) != 0; }
	CachedThemeMetricB( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
    operator const bool () const		{ ASSERT(m_bInited);	return m_bValue; };
};

		
		
class CachedThemeMetricC : public CachedThemeMetric
{
	RageColor	m_cValue;
public:
	void Update()
	{
		m_cValue = RageColor(1,1,1,1);
		sscanf( m_sValue, "%f,%f,%f,%f", &m_cValue.r, &m_cValue.g, &m_cValue.b, &m_cValue.a );
	}
	CachedThemeMetricC( CString sClassName, CString sValueName ) : CachedThemeMetric( sClassName, sValueName ) {}
    operator const RageColor () const	{ ASSERT(m_bInited);	return m_cValue; };
};



#endif
