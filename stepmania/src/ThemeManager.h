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

	/* I renamed these for two reasons.  The overload conflicts with the ones below:
	 * GetPathToB( str, str ) was matching the ones below instead of these.  It's also
	 * easier to search for uses of obsolete functions if they have a different name. */
	CString GetPath( ElementCategory category, CString sClassName, CString sElement, bool bOptional=false );
	CString GetPathB( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(BGAnimations,sClassName,sElement,bOptional); };
	CString GetPathF( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Fonts,sClassName,sElement,bOptional); };
	CString GetPathG( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Graphics,sClassName,sElement,bOptional); };
	CString GetPathN( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Numbers,sClassName,sElement,bOptional); };
	CString GetPathS( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Sounds,sClassName,sElement,bOptional); };
	CString GetPathO( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Other,sClassName,sElement,bOptional); };

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
	bool GetMetricRaw( CString sClassName, CString sValueName, CString &ret );
	CString GetPathToAndFallback( CString sThemeName, ElementCategory category, CString sClassName, CString sFile );
	CString GetPathToRaw( CString sThemeName, ElementCategory category, CString sClassName, CString sFile );
	static CString GetThemeDirFromName( const CString &sThemeName );
	CString GetElementDir( CString sThemeName );
	static CString GetMetricsIniPath( CString sThemeName );
	static void GetLanguagesForTheme( CString sThemeName, CStringArray& asLanguagesOut );
	static CString GetLanguageIniPath( CString sThemeName, CString sLanguage );

	CString m_sCurThemeName;
	CString m_sCurLanguage;

	IniFile* m_pIniCurMetrics;	// make this a pointer so we don't have to include IniFile here
	IniFile* m_pIniBaseMetrics;	// make this a pointer so we don't have to include IniFile here
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
