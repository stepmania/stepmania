/* ThemeManager - Manages which graphics and sounds are loaded.  Every time a sound or graphic is loaded, it gets the path from the ThemeManager. */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "RageTypes.h"
#include "RageTimer.h"
#include <set>
#include <deque>

class IniFile;

enum ElementCategory { BGAnimations, Fonts, Graphics, Numbers, Sounds, Other, NUM_ELEMENT_CATEGORIES };

struct Theme;

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
	void GetModifierNames( set<CString>& AddTo );

	/* I renamed these for two reasons.  The overload conflicts with the ones below:
	 * GetPathToB( str, str ) was matching the ones below instead of these.  It's also
	 * easier to search for uses of obsolete functions if they have a different name. */
	CString GetPath( ElementCategory category, CString sClassName, CString sElement, bool bOptional=false );
	CString GetPathB( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(BGAnimations,sClassName,sElement,bOptional); };
	CString GetPathF( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Fonts,sClassName,sElement,bOptional); };
	CString GetPathG( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Graphics,sClassName,sElement,bOptional); };
	CString GetPathS( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Sounds,sClassName,sElement,bOptional); };
	CString GetPathO( CString sClassName, CString sElement, bool bOptional=false ) { return GetPath(Other,sClassName,sElement,bOptional); };

	// TODO: remove these and update the places that use them
	CString GetPathToB( CString sFileName, bool bOptional=false );
	CString GetPathToF( CString sFileName, bool bOptional=false );
	CString GetPathToG( CString sFileName, bool bOptional=false );
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
	void LoadThemeRecursive( deque<Theme> &theme, CString sThemeName );
	bool GetMetricRaw( CString sClassName, CString sValueName, CString &ret, int level=0 );
	CString GetPathToAndFallback( CString sThemeName, ElementCategory category, CString sClassName, CString sFile );
	CString GetPathToRaw( CString sThemeName, ElementCategory category, CString sClassName, CString sFile );
	static CString GetThemeDirFromName( const CString &sThemeName );
	CString GetElementDir( CString sThemeName );
	static CString GetMetricsIniPath( CString sThemeName );
	static void GetLanguagesForTheme( CString sThemeName, CStringArray& asLanguagesOut );
	static CString GetLanguageIniPath( CString sThemeName, CString sLanguage );

	CString m_sCurThemeName;
	CString m_sCurLanguage;
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

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
