#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "RageTypes.h"
#include <set>
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
	NUM_ElementCategory,
	ElementCategory_Invalid
};
/** @brief A special foreach loop going through each ElementCategory. */
#define FOREACH_ElementCategory( ec ) FOREACH_ENUM( ElementCategory, ec )
std::string const ElementCategoryToString( ElementCategory ec );
ElementCategory StringToElementCategory( const std::string& s );

struct Theme;
/** @brief Manages theme paths and metrics. */
class ThemeManager
{
public:
	ThemeManager();
	~ThemeManager();

	void GetThemeNames( std::vector<std::string>& AddTo );
	void GetSelectableThemeNames( std::vector<std::string>& AddTo );
	int GetNumSelectableThemes();
	bool DoesThemeExist( const std::string &sThemeName );
	bool IsThemeSelectable( const std::string &name );
	bool IsThemeNameValid( const std::string &name );
	std::string GetThemeDisplayName( const std::string &sThemeName );
	std::string GetThemeAuthor( const std::string &sThemeName );
	void GetLanguages( std::vector<std::string>& AddTo );
	bool DoesLanguageExist( const std::string &sLanguage );
	void SwitchThemeAndLanguage( const std::string &sThemeName, const std::string &sLanguage, bool bPseudoLocalize, bool bForceThemeReload = false );
	void UpdateLuaGlobals();
	std::string GetCurThemeName() const { return m_sCurThemeName; };
	bool IsThemeLoaded() const { return !m_sCurThemeName.empty(); };
	std::string GetCurLanguage() const { return m_sCurLanguage; };
	std::string GetCurThemeDir() const { return GetThemeDirFromName(m_sCurThemeName); };
	std::string GetNextTheme();
	std::string GetNextSelectableTheme();
	void ReloadMetrics();
	void ReloadSubscribers();
	void ClearSubscribers();
	void GetOptionNames( std::vector<std::string>& AddTo );

	static void EvaluateString( std::string &sText );

	struct PathInfo
	{
		std::string sResolvedPath;
		std::string sMatchingMetricsGroup;
		std::string sMatchingElement;
	};

	bool GetPathInfo( PathInfo &out, ElementCategory category, const std::string &sMetricsGroup, const std::string &sElement, bool bOptional=false );
	std::string GetPath( ElementCategory category, const std::string &sMetricsGroup, const std::string &sElement, bool bOptional=false );
	std::string GetPathB( const std::string &sMetricsGroup, const std::string &sElement, bool bOptional=false ) { return GetPath(EC_BGANIMATIONS,sMetricsGroup,sElement,bOptional); };
	std::string GetPathF( const std::string &sMetricsGroup, const std::string &sElement, bool bOptional=false ) { return GetPath(EC_FONTS,sMetricsGroup,sElement,bOptional); };
	std::string GetPathG( const std::string &sMetricsGroup, const std::string &sElement, bool bOptional=false ) { return GetPath(EC_GRAPHICS,sMetricsGroup,sElement,bOptional); };
	std::string GetPathS( const std::string &sMetricsGroup, const std::string &sElement, bool bOptional=false ) { return GetPath(EC_SOUNDS,sMetricsGroup,sElement,bOptional); };
	std::string GetPathO( const std::string &sMetricsGroup, const std::string &sElement, bool bOptional=false ) { return GetPath(EC_OTHER,sMetricsGroup,sElement,bOptional); };
	void ClearThemePathCache();

	bool		HasMetric( const std::string &sMetricsGroup, const std::string &sValueName );
	void		PushMetric( Lua *L, const std::string &sMetricsGroup, const std::string &sValueName );
	std::string		GetMetric( const std::string &sMetricsGroup, const std::string &sValueName );
	int			GetMetricI( const std::string &sMetricsGroup, const std::string &sValueName );
	float		GetMetricF( const std::string &sMetricsGroup, const std::string &sValueName );
	bool		GetMetricB( const std::string &sMetricsGroup, const std::string &sValueName );
	Rage::Color	GetMetricC( const std::string &sMetricsGroup, const std::string &sValueName );
	LuaReference	GetMetricR( const std::string &sMetricsGroup, const std::string &sValueName );
#if !defined(SMPACKAGE)
	apActorCommands	GetMetricA( const std::string &sMetricsGroup, const std::string &sValueName );
#endif

	void	GetMetric( const std::string &sMetricsGroup, const std::string &sValueName, LuaReference &valueOut );

	// Languages
	bool	HasString( std::string const &sMetricsGroup, std::string const &sValueName );
	std::string	GetString( std::string const &sMetricsGroup, std::string const &sValueName );
	void	GetString( std::string const &sMetricsGroup, std::string const &sValueName, std::string &valueOut )
	{
		valueOut = GetString( sMetricsGroup, sValueName );
	}
	void FilterFileLanguages( std::vector<std::string> &asElementPaths );

	void GetMetricsThatBeginWith( const std::string &sMetricsGroup, const std::string &sValueName, std::set<std::string> &vsValueNamesOut );

	std::string GetMetricsGroupFallback( const std::string &sMetricsGroup );

	static std::string GetBlankGraphicPath();

	//needs to be public for its binding to work
	void RunLuaScripts( const std::string &sMask, bool bUseThemeDir = false );

	// For self-registering metrics
	static void Subscribe( IThemeMetric *p );
	static void Unsubscribe( IThemeMetric *p );

	// Lua
	void PushSelf( lua_State *L );

protected:
	void LoadThemeMetrics( const std::string &sThemeName, const std::string &sLanguage_ );
	std::string GetMetricRaw( const IniFile &ini, const std::string &sMetricsGroup, const std::string &sValueName );
	bool GetMetricRawRecursive( const IniFile &ini, const std::string &sMetricsGroup, const std::string &sValueName, std::string &sRet );

	bool GetPathInfoToAndFallback( PathInfo &out, ElementCategory category, const std::string &sMetricsGroup, const std::string &sFile );
	bool GetPathInfoToRaw( PathInfo &out, const std::string &sThemeName, ElementCategory category, const std::string &sMetricsGroup, const std::string &sFile );
	static std::string GetThemeDirFromName( const std::string &sThemeName );
	std::string GetElementDir( const std::string &sThemeName );
	static std::string GetMetricsIniPath( const std::string &sThemeName );
	static void GetLanguagesForTheme( std::string const &sThemeName, std::vector<std::string>& asLanguagesOut );
	static std::string GetLanguageIniPath( const std::string &sThemeName, const std::string &sLanguage );
	void GetOptionalLanguageIniPaths( std::vector<std::string> &vsPathsOut, std::string const &sThemeName, std::string const &sLanguage );
	std::string GetDefaultLanguage();

	std::string m_sCurThemeName;
	std::string m_sCurLanguage;
	bool m_bPseudoLocalize;
};

extern ThemeManager*	THEME;	// global and accessible from anywhere in our program

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
