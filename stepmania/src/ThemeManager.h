/* ThemeManager - Manages theme paths and metrics. */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "RageTypes.h"
#include <set>
#include <deque>
#include "LuaReference.h"

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
#define FOREACH_ElementCategory( ec ) FOREACH_ENUM2( ElementCategory, ec )
const RString& ElementCategoryToString( ElementCategory ec );
ElementCategory StringToElementCategory( const RString& s );

struct Theme;

class ThemeManager
{
public:
	ThemeManager();
	~ThemeManager();

	void GetThemeNames( vector<RString>& AddTo );
	void GetSelectableThemeNames( vector<RString>& AddTo );
	int GetNumSelectableThemes();
	bool DoesThemeExist( const RString &sThemeName );
	bool IsThemeSelectable( const RString &sThemeName );
	RString GetThemeDisplayName( const RString &sThemeName );
	void GetLanguages( vector<RString>& AddTo );
	bool DoesLanguageExist( const RString &sLanguage );
	void SwitchThemeAndLanguage( const RString &sThemeName, const RString &sLanguage, bool bPseudoLocalize );
	void UpdateLuaGlobals();
	RString GetCurThemeName() const { return m_sCurThemeName; };
	bool IsThemeLoaded() const { return !m_sCurThemeName.empty(); };
	RString GetCurLanguage() const { return m_sCurLanguage; };
	RString GetCurThemeDir() const { return GetThemeDirFromName(m_sCurThemeName); };
	RString GetNextTheme();
	void ReloadMetrics();
	void ReloadSubscribers();
	void ClearSubscribers();
	void GetOptionNames( vector<RString>& AddTo );

	static void EvaluateString( RString &sText );

	/* I renamed these for two reasons.  The overload conflicts with the ones below:
	 * GetPathToB( str, str ) was matching the ones below instead of these.  It's also
	 * easier to search for uses of obsolete functions if they have a different name. */
	RString GetPath( ElementCategory category, const RString &sClassName, const RString &sElement, bool bOptional=false );
	RString GetPathB( const RString &sClassName, const RString &sElement, bool bOptional=false ) { return GetPath(EC_BGANIMATIONS,sClassName,sElement,bOptional); };
	RString GetPathF( const RString &sClassName, const RString &sElement, bool bOptional=false ) { return GetPath(EC_FONTS,sClassName,sElement,bOptional); };
	RString GetPathG( const RString &sClassName, const RString &sElement, bool bOptional=false ) { return GetPath(EC_GRAPHICS,sClassName,sElement,bOptional); };
	RString GetPathS( const RString &sClassName, const RString &sElement, bool bOptional=false ) { return GetPath(EC_SOUNDS,sClassName,sElement,bOptional); };
	RString GetPathO( const RString &sClassName, const RString &sElement, bool bOptional=false ) { return GetPath(EC_OTHER,sClassName,sElement,bOptional); };
	void ClearThemePathCache();

	bool		HasMetric( const RString &sClassName, const RString &sValueName );
	void		PushMetric( Lua *L, const RString &sClassName, const RString &sValueName );
	RString		GetMetric( const RString &sClassName, const RString &sValueName );
	int		GetMetricI( const RString &sClassName, const RString &sValueName );
	float		GetMetricF( const RString &sClassName, const RString &sValueName );
	bool		GetMetricB( const RString &sClassName, const RString &sValueName );
	RageColor	GetMetricC( const RString &sClassName, const RString &sValueName );
	LuaReference	GetMetricR( const RString &sClassName, const RString &sValueName );
#if !defined(SMPACKAGE)
	apActorCommands	GetMetricA( const RString &sClassName, const RString &sValueName );
#endif

	void	GetMetric( const RString &sClassName, const RString &sValueName, LuaReference &valueOut );

	// Languages
	bool	HasString( const RString &sClassName, const RString &sValueName );
	RString	GetString( const RString &sClassName, const RString &sValueName );
	void	GetString( const RString &sClassName, const RString &sValueName, RString &valueOut )		{ valueOut = GetString( sClassName, sValueName ); }
	void FilterFileLanguages( vector<RString> &asElementPaths );

	void GetMetricsThatBeginWith( const RString &sClassName, const RString &sValueName, set<RString> &vsValueNamesOut );

	RString GetClassFallback( const RString &sClassName );

	static RString GetBlankGraphicPath();

	//
	// For self-registering metrics
	//
	static void Subscribe( IThemeMetric *p );
	static void Unsubscribe( IThemeMetric *p );

	// Lua
	void PushSelf( lua_State *L );

protected:
	void RunLuaScripts( const RString &sMask );
	void LoadThemeMetrics( deque<Theme> &theme, const RString &sThemeName, const RString &sLanguage_ );
	RString GetMetricRaw( const IniFile &ini, const RString &sClassName, const RString &sValueName );
	bool GetMetricRawRecursive( const IniFile &ini, const RString &sClassName, const RString &sValueName, RString &sRet );
	RString GetPathToAndFallback( ElementCategory category, const RString &sClassName, const RString &sFile );
	RString GetPathToRaw( const RString &sThemeName, ElementCategory category, const RString &sClassName, const RString &sFile );
	static RString GetThemeDirFromName( const RString &sThemeName );
	RString GetElementDir( const RString &sThemeName );
	static RString GetMetricsIniPath( const RString &sThemeName );
	static void GetLanguagesForTheme( const RString &sThemeName, vector<RString>& asLanguagesOut );
	static RString GetLanguageIniPath( const RString &sThemeName, const RString &sLanguage );
	RString GetDefaultLanguage();

	RString m_sCurThemeName;
	RString m_sCurLanguage;
	bool m_bPseudoLocalize;
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
