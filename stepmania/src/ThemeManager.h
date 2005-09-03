/* ThemeManager - Manages theme paths and metrics. */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "RageTypes.h"
#include <set>
#include <deque>
#include "Command.h"
#include "ActorCommands.h"

class IThemeMetric;
class IniFile;
struct lua_State;

enum ElementCategory
{
	EC_BGANIMATIONS,
	EC_FONTS,
	EC_GRAPHICS,
	EC_SOUNDS,
	EC_OTHER,
	NUM_ElementCategory
};
#define FOREACH_ElementCategory( ec ) FOREACH_ENUM( ElementCategory, NUM_ElementCategory, ec )
const CString& ElementCategoryToString( ElementCategory ec );
ElementCategory StringToElementCategory( const CString& s );

struct Theme;

class ThemeManager
{
public:
	ThemeManager();
	~ThemeManager();

	void GetThemeNames( CStringArray& AddTo );
	bool DoesThemeExist( const CString &sThemeName );
	void GetLanguages( CStringArray& AddTo );
	bool DoesLanguageExist( const CString &sLanguage );
	void SwitchThemeAndLanguage( const CString &sThemeName, const CString &sLanguage );
	void UpdateLuaGlobals();
	CString GetCurThemeName() const { return m_sCurThemeName; };
	bool IsThemeLoaded() const { return !m_sCurThemeName.empty(); };
	CString GetCurLanguage() const { return m_sCurLanguage; };
	CString GetCurThemeDir() const { return GetThemeDirFromName(m_sCurThemeName); };
	void NextTheme();
	void ReloadMetrics();
	void GetModifierNames( vector<CString>& AddTo );

	static void EvaluateString( CString &sText );

	/* I renamed these for two reasons.  The overload conflicts with the ones below:
	 * GetPathToB( str, str ) was matching the ones below instead of these.  It's also
	 * easier to search for uses of obsolete functions if they have a different name. */
	CString GetPath( ElementCategory category, const CString &sClassName, const CString &sElement, bool bOptional=false );
	CString GetPathB( const CString &sClassName, const CString &sElement, bool bOptional=false ) { return GetPath(EC_BGANIMATIONS,sClassName,sElement,bOptional); };
	CString GetPathF( const CString &sClassName, const CString &sElement, bool bOptional=false ) { return GetPath(EC_FONTS,sClassName,sElement,bOptional); };
	CString GetPathG( const CString &sClassName, const CString &sElement, bool bOptional=false ) { return GetPath(EC_GRAPHICS,sClassName,sElement,bOptional); };
	CString GetPathS( const CString &sClassName, const CString &sElement, bool bOptional=false ) { return GetPath(EC_SOUNDS,sClassName,sElement,bOptional); };
	CString GetPathO( const CString &sClassName, const CString &sElement, bool bOptional=false ) { return GetPath(EC_OTHER,sClassName,sElement,bOptional); };
	void ClearThemePathCache();

	// TODO: Make these return values const refs.
	bool		HasMetric( const CString &sClassName, const CString &sValueName );
	CString		GetMetricRaw( const CString &sClassName, const CString &sValueName );
	CString		GetMetric( const CString &sClassName, const CString &sValueName );
	int			GetMetricI( const CString &sClassName, const CString &sValueName );
	float		GetMetricF( const CString &sClassName, const CString &sValueName );
	bool		GetMetricB( const CString &sClassName, const CString &sValueName );
	RageColor	GetMetricC( const CString &sClassName, const CString &sValueName );
	Commands	GetMetricM( const CString &sClassName, const CString &sValueName );
	apActorCommands	GetMetricA( const CString &sClassName, const CString &sValueName );

	void	GetMetric( const CString &sClassName, const CString &sValueName, CString &valueOut )		{ valueOut = GetMetric( sClassName, sValueName ); }
	void	GetMetric( const CString &sClassName, const CString &sValueName, int &valueOut )			{ valueOut = GetMetricI( sClassName, sValueName ); }
	void	GetMetric( const CString &sClassName, const CString &sValueName, float &valueOut )			{ valueOut = GetMetricF( sClassName, sValueName ); }
	void	GetMetric( const CString &sClassName, const CString &sValueName, bool &valueOut )			{ valueOut = GetMetricB( sClassName, sValueName ); }
	void	GetMetric( const CString &sClassName, const CString &sValueName, RageColor &valueOut )		{ valueOut = GetMetricC( sClassName, sValueName ); }
	void	GetMetric( const CString &sClassName, const CString &sValueName, Command &valueOut );
	void	GetMetric( const CString &sClassName, const CString &sValueName, LuaExpression &valueOut );
	void	GetMetric( const CString &sClassName, const CString &sValueName, apActorCommands &valueOut );

	void GetMetricsThatBeginWith( const CString &sClassName, const CString &sValueName, set<CString> &vsValueNamesOut );
	void LoadPreferencesFromSection( const CString &sClassName );

	static CString GetBlankGraphicPath();

	//
	// For self-registering metrics
	//
	static void Subscribe( IThemeMetric *p );
	static void Unsubscribe( IThemeMetric *p );

	// Lua
	void PushSelf( lua_State *L );

protected:
	void RunLuaScripts( const CString &sMask );
	void LoadThemeRecursive( deque<Theme> &theme, const CString &sThemeName );
	bool GetMetricRaw( const CString &sClassName, const CString &sValueName, CString &ret, int level=0 );
	CString GetPathToAndFallback( const CString &sThemeName, ElementCategory category, const CString &sClassName, const CString &sFile );
	CString GetPathToRaw( const CString &sThemeName, ElementCategory category, const CString &sClassName, const CString &sFile );
	static CString GetThemeDirFromName( const CString &sThemeName );
	CString GetElementDir( const CString &sThemeName );
	static CString GetMetricsIniPath( const CString &sThemeName );
	static void GetLanguagesForTheme( const CString &sThemeName, CStringArray& asLanguagesOut );
	static CString GetLanguageIniPath( const CString &sThemeName, const CString &sLanguage );

	CString m_sCurThemeName;
	CString m_sCurLanguage;
};

extern ThemeManager*	THEME;	// global and accessable from anywhere in our program

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
