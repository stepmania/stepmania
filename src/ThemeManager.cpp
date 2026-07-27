#include "global.h"
#include "ThemeManager.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include "IniFile.h"
#include "RageTimer.h"
#include "FontCharAliases.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "arch/Dialog/Dialog.h"
#include "RageFile.h"
#if !defined(SMPACKAGE)
#include "ScreenManager.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "ActorUtil.h"
#endif
#include "GameLoop.h" // For ChangeTheme
#include "ThemeMetric.h"
#include "SubscriptionManager.h"
#include "LuaManager.h"
#include "ScreenDimensions.h"
#include "LocalizedString.h"
#include "SpecialFiles.h"
#include "EnumHelper.h"
#include "PrefsManager.h"
#include "XmlFileUtil.h"

#include <cstddef>
#include <deque>
#include <vector>


ThemeManager*	THEME = nullptr;	// global object accessible from anywhere in the program

static const RString THEME_INFO_INI = "ThemeInfo.ini";

static const char *ElementCategoryNames[] = {
	"BGAnimations",
	"Fonts",
	"Graphics",
	"Sounds",
	"Other"
};
XToString( ElementCategory );
StringToX( ElementCategory );


struct Theme
{
	RString sThemeName;
};
// When looking for a metric or an element, search these from head to tail.
static std::deque<Theme> g_vThemes;
class LoadedThemeData
{
public:
	IniFile iniMetrics;
	IniFile iniStrings;
	void ClearAll()
	{
		iniMetrics.Clear();
		iniStrings.Clear();
	}
};
LoadedThemeData *g_pLoadedThemeData = nullptr;


// For self-registering metrics
#include "SubscriptionManager.h"
static SubscriptionManager<IThemeMetric> g_Subscribers;

class LocalizedStringImplThemeMetric : public ILocalizedStringImpl, public ThemeMetric<RString>
{
public:
	static ILocalizedStringImpl *Create() { return new LocalizedStringImplThemeMetric; }

	void Load( const RString& sGroup, const RString& sName )
	{
		ThemeMetric<RString>::Load( sGroup, sName );
	}

	virtual void Read()
	{
		if( m_sName != ""  &&  THEME  &&   THEME->IsThemeLoaded() )
		{
			THEME->GetString( m_sGroup, m_sName, m_currentValue );
			m_Value.SetFromNil();
		}
	}

	const RString &GetLocalized() const
	{
		if( IsLoaded() )
		{
			return GetValue();
		}
		RString const & curLanguage = (THEME && THEME->IsThemeLoaded() ? THEME->GetCurLanguage() : RString("current"));
		LOG->Warn("Missing translation for %s in the %s language.", m_sName.c_str(), curLanguage.c_str());
		return m_sName;
	}
};

void ThemeManager::Subscribe( IThemeMetric *p )
{
	g_Subscribers.Subscribe( p );

	// It's ThemeManager's responsibility to make sure all of its subscribers
	// are updated with current data.  If a metric is created after
	// a theme is loaded, ThemeManager should update it right away (not just
	// when the theme changes).
	if( THEME && THEME->GetCurThemeName().size() )
		p->Read();
}

void ThemeManager::Unsubscribe( IThemeMetric *p )
{
	g_Subscribers.Unsubscribe( p );
}


// We spend a lot of time doing redundant theme path lookups. Cache results.
static std::map<RString, ThemeManager::PathInfo> g_ThemePathCache[NUM_ElementCategory];
void ThemeManager::ClearThemePathCache()
{
	for( int i = 0; i < NUM_ElementCategory; ++i )
		g_ThemePathCache[i].clear();
}

static void FileNameToMetricsGroupAndElement( const RString &sFileName, RString &sMetricsGroupOut, RString &sElementOut )
{
	// split into class name and file name
	RString::size_type iIndexOfFirstSpace = sFileName.find(" ");
	if( iIndexOfFirstSpace == std::string::npos ) // no space
	{
		sMetricsGroupOut = "";
		sElementOut = sFileName;
	}
	else
	{
		sMetricsGroupOut = sFileName.Left( iIndexOfFirstSpace );
		sElementOut = sFileName.Right( sFileName.size() - iIndexOfFirstSpace - 1 );
	}
}


static RString MetricsGroupAndElementToFileName( const RString &sMetricsGroup, const RString &sElement )
{
	if( sMetricsGroup.empty() )
		return sElement;
	else
		return sMetricsGroup + " " + sElement;
}

ThemeManager::ThemeManager()
{
	THEME = this;	// so that we can Register THEME on construction

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring(L, "THEME");
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}

	// We don't have any theme loaded until SwitchThemeAndLanguage is called.
	m_sCurThemeName = "";
	m_bPseudoLocalize = false;

	std::vector<RString> arrayThemeNames;
	GetThemeNames( arrayThemeNames );
}

ThemeManager::~ThemeManager()
{
	g_vThemes.clear();
	SAFE_DELETE( g_pLoadedThemeData );

	// Unregister with Lua.
	LUA->UnsetGlobal( "THEME" );
}

void ThemeManager::GetThemeNames( std::vector<RString>& AddTo )
{
	GetDirListing( SpecialFiles::THEMES_DIR + "*", AddTo, true );
	StripCvsAndSvn( AddTo );
	StripMacResourceForks( AddTo );
}

void ThemeManager::GetSelectableThemeNames( std::vector<RString>& AddTo )
{
	GetThemeNames( AddTo );
	for( int i=AddTo.size()-1; i>=0; i-- )
	{
		if(!IsThemeNameValid(AddTo[i]))
		{
			AddTo.erase( AddTo.begin()+i );
		}
	}
}

int ThemeManager::GetNumSelectableThemes()
{
	std::vector<RString> vs;
	GetSelectableThemeNames( vs );
	return vs.size();
}

bool ThemeManager::DoesThemeExist( const RString &sThemeName )
{
	std::vector<RString> asThemeNames;
	GetThemeNames( asThemeNames );
	for( unsigned i=0; i<asThemeNames.size(); i++ )
	{
		if( !sThemeName.CompareNoCase(asThemeNames[i]) )
			return true;
	}
	return false;
}

bool ThemeManager::IsThemeSelectable(RString const& name)
{
	return IsThemeNameValid(name) && DoesThemeExist(name);
}

bool ThemeManager::IsThemeNameValid(RString const& name)
{
	return name.Left(1) != "_";
}

RString ThemeManager::GetThemeDisplayName( const RString &sThemeName )
{
	RString sDir = GetThemeDirFromName(sThemeName);
	IniFile ini;
	ini.ReadFile( sDir + THEME_INFO_INI );

	RString s;
	if( ini.GetValue("ThemeInfo","DisplayName",s) )
		return s;

	return sThemeName;
}

RString ThemeManager::GetThemeAuthor( const RString &sThemeName )
{
	RString sDir = GetThemeDirFromName(sThemeName);
	IniFile ini;
	ini.ReadFile( sDir + THEME_INFO_INI );

	RString s;
	if( ini.GetValue("ThemeInfo","Author",s) )
		return s;

	return "[unknown author]";
}

static bool EqualsNoCase( const RString &s1, const RString &s2 )
{
	return s1.EqualsNoCase(s2);
}
void ThemeManager::GetLanguages( std::vector<RString>& AddTo )
{
	AddTo.clear();

	for( unsigned i = 0; i < g_vThemes.size(); ++i )
		GetLanguagesForTheme( g_vThemes[i].sThemeName, AddTo );

	// remove dupes
	sort( AddTo.begin(), AddTo.end() );
	std::vector<RString>::iterator it = unique( AddTo.begin(), AddTo.end(), EqualsNoCase );
	AddTo.erase(it, AddTo.end());
}

bool ThemeManager::DoesLanguageExist( const RString &sLanguage )
{
	std::vector<RString> asLanguages;
	GetLanguages( asLanguages );

	for( unsigned i=0; i<asLanguages.size(); i++ )
		if( sLanguage.CompareNoCase(asLanguages[i])==0 )
			return true;
	return false;
}

void ThemeManager::LoadThemeMetrics( const RString &sThemeName_, const RString &sLanguage_ )
{
	if( g_pLoadedThemeData == nullptr )
		g_pLoadedThemeData = new LoadedThemeData;

	// Don't delete and recreate LoadedThemeData.  There are references iniMetrics and iniStrings
	// on the stack, so Clear them instead.
	g_pLoadedThemeData->ClearAll();
	g_vThemes.clear();

	RString sThemeName(sThemeName_);
	RString sLanguage(sLanguage_);

	m_sCurThemeName = sThemeName;
	m_sCurLanguage = sLanguage;

	bool bLoadedBase = false;
	for(;;)
	{
		ASSERT_M( g_vThemes.size() < 20, "Circular theme fallback references detected." );

		g_vThemes.push_back( Theme() );
		Theme &t = g_vThemes.back();
		t.sThemeName = sThemeName;

		IniFile iniMetrics;
		IniFile iniStrings;
		iniMetrics.ReadFile( GetMetricsIniPath(sThemeName) );
		// Load optional language inis (probably mounted by a package) first so that they can be overridden by the current theme.
		{
			std::vector<RString> vs;
			GetOptionalLanguageIniPaths(vs,sThemeName,sLanguage);
			for (RString const &s : vs)
				iniStrings.ReadFile( s );
		}
		iniStrings.ReadFile( GetLanguageIniPath(sThemeName,SpecialFiles::BASE_LANGUAGE) );
		if( sLanguage.CompareNoCase(SpecialFiles::BASE_LANGUAGE) )
		{
			iniStrings.ReadFile( GetLanguageIniPath(sThemeName,sLanguage) );
		}
		bool bIsBaseTheme = !sThemeName.CompareNoCase(SpecialFiles::BASE_THEME_NAME);
		iniMetrics.GetValue( "Global", "IsBaseTheme", bIsBaseTheme );
		if( bIsBaseTheme )
		{
			bLoadedBase = true;
		}
		/* Read the fallback theme. If no fallback theme is specified, and we haven't
		 * already loaded it, fall back on SpecialFiles::BASE_THEME_NAME.
		 * That way, default theme fallbacks can be disabled with
		 * "FallbackTheme=". */
		RString sFallback;
		if( !iniMetrics.GetValue("Global","FallbackTheme",sFallback) )
		{
			if( sThemeName.CompareNoCase( SpecialFiles::BASE_THEME_NAME ) && !bLoadedBase )
			{
				sFallback = SpecialFiles::BASE_THEME_NAME;
			}
		}
		/* We actually want to load themes bottom-to-top, loading fallback themes
		 * first, so derived themes overwrite metrics in fallback themes. But, we
		 * need to load the derived theme first, to find out the name of the fallback
		 * theme.  Avoid having to load IniFile twice, merging the fallback theme
		 * into the derived theme that we've already loaded. */
		XmlFileUtil::MergeIniUnder( &iniMetrics, &g_pLoadedThemeData->iniMetrics );
		XmlFileUtil::MergeIniUnder( &iniStrings, &g_pLoadedThemeData->iniStrings );

		if( sFallback.empty() )
		{
			break;
		}
		sThemeName = sFallback;
	}

	// Overlay metrics from the command line.
	RString sMetric;
	for( int i = 0; GetCommandlineArgument( "metric", &sMetric, i ); ++i )
	{
		/* sMetric must be "foo::bar=baz". "foo" and "bar" never contain "=", so
		 * in "foo::bar=1+1=2", "baz" is always "1+1=2". Neither foo nor bar may
		 * be empty, but baz may be. */
		Regex re( "^([^=]+)::([^=]+)=(.*)$" );
		std::vector<RString> sBits;
		if( !re.Compare( sMetric, sBits ) )
			RageException::Throw( "Invalid argument \"--metric=%s\".", sMetric.c_str() );

		g_pLoadedThemeData->iniMetrics.SetValue( sBits[0], sBits[1], sBits[2] );
	}

	LOG->MapLog( "theme", "Theme: %s", m_sCurThemeName.c_str() );
	LOG->MapLog( "language", "Language: %s", m_sCurLanguage.c_str() );
}

RString ThemeManager::GetDefaultLanguage()
{
	RString sLangCode = HOOKS->GetPreferredLanguage();
	return sLangCode;
}

void ThemeManager::SwitchThemeAndLanguage( const RString &sThemeName_, const RString &sLanguage_, bool bPseudoLocalize, bool bForceThemeReload )
{
	RString sThemeName = sThemeName_;
	RString sLanguage = sLanguage_;
	// todo: if the theme isn't selectable, find the next theme that is,
	// and change to that instead of asserting/crashing since
	// SpecialFiles::BASE_THEME_NAME is _fallback now. -aj
	if(!IsThemeSelectable(sThemeName))
	{
		RString to_try= PREFSMAN->m_sTheme.GetDefault();
		LOG->Warn("Selected theme '%s' not found.  "
			"Trying Theme preference default value '%s'.",
			sThemeName.c_str(), to_try.c_str());
		sThemeName = to_try;
		// sm-ssc's SpecialFiles::BASE_THEME_NAME is _fallback, which you can't
		// select. This requires a preference, which allows it to be adapted for
		// other purposes (e.g. PARASTAR).
		if(!IsThemeSelectable(sThemeName))
		{
			to_try= PREFSMAN->m_sDefaultTheme;
			LOG->Warn("Theme preference defaults to '%s', which cannot be used."
				"  Trying DefaultTheme preference '%s'.",
				sThemeName.c_str(), to_try.c_str());
			sThemeName = to_try;
			if(!IsThemeSelectable(sThemeName))
			{
				std::vector<RString> theme_names;
				GetSelectableThemeNames(theme_names);
				ASSERT_M(!theme_names.empty(), "No themes found, unable to start stepmania.");
				to_try= theme_names[0];
				LOG->Warn("DefaultTheme preference is '%s', which cannot be found."
					"  Using '%s'.",
					sThemeName.c_str(), to_try.c_str());
				sThemeName= to_try;
				PREFSMAN->m_sDefaultTheme.Set(to_try);
			}
		}
		PREFSMAN->m_sTheme.Set(sThemeName);
	}

	/* We haven't actually loaded the theme yet, so we can't check whether
	 * sLanguage exists. Just check for empty. */
	if( sLanguage.empty() )
		sLanguage = GetDefaultLanguage();
	LOG->Trace("ThemeManager::SwitchThemeAndLanguage: \"%s\", \"%s\"",
		sThemeName.c_str(), sLanguage.c_str() );

	bool bNothingChanging = sThemeName == m_sCurThemeName && sLanguage == m_sCurLanguage && m_bPseudoLocalize == bPseudoLocalize;
	if( bNothingChanging && !bForceThemeReload )
		return;

	m_bPseudoLocalize = bPseudoLocalize;

	// Load theme metrics. If only the language is changing, this is all
	// we need to reload.
	bool bThemeChanging = (sThemeName != m_sCurThemeName);
	LoadThemeMetrics( sThemeName, sLanguage );

	// Clear the theme path cache. This caches language-specific graphic paths,
	// so do this even if only the language is changing.
	ClearThemePathCache();
	if(bThemeChanging || bForceThemeReload)
	{
#if !defined(SMPACKAGE)
		// reload common sounds
		if( SCREENMAN != nullptr )
			SCREENMAN->ThemeChanged();

#endif

		/* Lua globals can use metrics which are cached, and vice versa.  Update Lua
		 * globals first; it's Lua's job to explicitly update cached metrics that it
		 * uses. */
		UpdateLuaGlobals();

		// Reload MachineProfile with new theme's CustomLoadFunction
		if( PROFILEMAN != nullptr )
		{
			Profile* pProfile = PROFILEMAN->GetMachineProfile();
			pProfile->LoadCustomFunction("/Save/MachineProfile/", PlayerNumber_Invalid);
		}
	}

	// Use theme metrics for localization.
	LocalizedString::RegisterLocalizer( LocalizedStringImplThemeMetric::Create );

	ReloadSubscribers();
}

void ThemeManager::ReloadSubscribers()
{
	// reload subscribers
	if( g_Subscribers.m_pSubscribers )
	{
		for (IThemeMetric *metric : *g_Subscribers.m_pSubscribers)
			metric->Read();
	}
}

void ThemeManager::ClearSubscribers()
{
	if( g_Subscribers.m_pSubscribers )
	{
		for (IThemeMetric *metric : *g_Subscribers.m_pSubscribers)
			metric->Clear();
	}
}

void ThemeManager::RunLuaScripts( const RString &sMask, bool bUseThemeDir )
{
	/* Run all script files with the given mask in Lua for all themes.  Start
	 * from the deepest fallback theme and work outwards. */

	/* TODO: verify whether this final check is necessary. */
	const RString sCurThemeName = m_sCurThemeName;

	std::deque<Theme>::const_iterator iter = g_vThemes.end();
	do
	{
		--iter;

		/* HACK: pretend to be the theme these are under by setting m_sCurThemeName
		 * to the name of the theme whose scripts are currently running; if those
		 * scripts call GetThemeName(), it'll return the theme the script is in. */

		m_sCurThemeName = iter->sThemeName;
		const RString &sScriptDir = bUseThemeDir ? GetThemeDirFromName( m_sCurThemeName ) : RString("/");

		std::vector<RString> asElementPaths;
		// get files from directories
		std::vector<RString> asElementChildPaths;
		std::vector<RString> arrayScriptDirs;
		GetDirListing( sScriptDir + "Scripts/*", arrayScriptDirs, true );
		SortRStringArray( arrayScriptDirs );
		StripCvsAndSvn( arrayScriptDirs );
		StripMacResourceForks( arrayScriptDirs );
		for( RString const &sScriptDirName : arrayScriptDirs )	// foreach dir in /Scripts/
		{
			// Find all Lua files in this directory, add them to asElementPaths
			GetDirListing( sScriptDir + "Scripts/" + sScriptDirName + "/" + sMask, asElementChildPaths, false, true );
			for( unsigned i = 0; i < asElementChildPaths.size(); ++i )
			{
				// push these Lua files into the main element paths
				const RString &sPath = asElementChildPaths[i];
				asElementPaths.push_back(sPath);
			}
		}

		// get regular Lua files
		GetDirListing( sScriptDir + "Scripts/" + sMask, asElementPaths, false, true );

		// load Lua files
		for( unsigned i = 0; i < asElementPaths.size(); ++i )
		{
			const RString &sPath = asElementPaths[i];
			LOG->Trace( "Loading \"%s\" ...", sPath.c_str() );
			LuaHelpers::RunScriptFile( sPath );
		}
	}
	while( iter != g_vThemes.begin() );

	/* TODO: verify whether this final check is necessary. */
	if( sCurThemeName != m_sCurThemeName )
	{
		LOG->Warn( "ThemeManager: theme name was not restored after RunLuaScripts" );
		m_sCurThemeName = sCurThemeName;
	}
}

void ThemeManager::UpdateLuaGlobals()
{
#if !defined(SMPACKAGE)
	// explicitly refresh cached metrics that we use.
	ScreenDimensions::ReloadScreenDimensions();

	// run global scripts
	RunLuaScripts( "*.lua" );
	// run theme scripts
	RunLuaScripts( "*.lua", true );
#endif
}

RString ThemeManager::GetThemeDirFromName( const RString &sThemeName )
{
	return SpecialFiles::THEMES_DIR + sThemeName + "/";
}

struct CompareLanguageTag
{
	RString m_sLanguageString;
	CompareLanguageTag( const RString &sLang )
	{
		m_sLanguageString = RString("(lang ") + sLang + ")";
		LOG->Trace( "try \"%s\"", sLang.c_str() );
		m_sLanguageString.MakeLower();
	}

	bool operator()( const RString &sFile ) const
	{
		RString sLower( sFile );
		sLower.MakeLower();
		std::size_t iPos = sLower.find( m_sLanguageString );
		return iPos != RString::npos;
	}
};

/* If there's more than one result, check for language tags.  For example,
 *
 * ScreenCompany graphic (lang English).png
 * ScreenCompany graphic (lang French).png
 *
 * We still want to warn for ambiguous results.  Use std::unique to filter
 * files with the current language tag to the top, so choosing "ignore" from
 * the multiple-match dialog will cause it to default to the first entry, so
 * it'll still use a preferred language match if there were any. */
void ThemeManager::FilterFileLanguages( std::vector<RString> &asPaths )
{
	if( asPaths.size() <= 1 )
		return;
	std::vector<RString>::iterator it =
		partition( asPaths.begin(), asPaths.end(), CompareLanguageTag(m_sCurLanguage) );

	int iDist = distance( asPaths.begin(), it );
	if( iDist == 0 )
	{
		// We didn't find any for the current language.  Try BASE_LANGUAGE.
		it = partition( asPaths.begin(), asPaths.end(), CompareLanguageTag(SpecialFiles::BASE_LANGUAGE) );
		iDist = distance( asPaths.begin(), it );
	}

	if( iDist == 1 )
		asPaths.erase( it, asPaths.end() );
}

bool ThemeManager::GetPathInfoToRaw( PathInfo &out, const RString &sThemeName_, ElementCategory category, const RString &sMetricsGroup_, const RString &sElement_ )
{
	/* Ugly: the parameters to this function may be a reference into g_vThemes,
	 * or something else that might suddenly go away when we call ReloadMetrics,
	 * so make a copy. */
	const RString sThemeName = sThemeName_;
	const RString sMetricsGroup = sMetricsGroup_;
	const RString sElement = sElement_;

	const RString sThemeDir = GetThemeDirFromName( sThemeName );
	const RString &sCategory = ElementCategoryToString(category);

	std::vector<RString> asElementPaths;

	// If sFileName already has an extension, we're looking for a specific file
	bool bLookingForSpecificFile = sElement.find_last_of('.') != sElement.npos;

	if( bLookingForSpecificFile )
	{
		GetDirListing( sThemeDir + sCategory+"/"+MetricsGroupAndElementToFileName(sMetricsGroup,sElement), asElementPaths, false, true );
	}
	else	// look for all files starting with sFileName that have types we can use
	{
		std::vector<RString> asPaths;
		GetDirListing( sThemeDir + sCategory + "/" + MetricsGroupAndElementToFileName(sMetricsGroup,sElement) + "*",
						asPaths, false, true );

		for( unsigned p = 0; p < asPaths.size(); ++p )
		{
			// BGAnimations, Fonts, Graphics, Sounds, Other
			const RString ext = GetExtension(asPaths[p]);
			bool matches= category == EC_OTHER || ext == "redir";
			if(!matches)
			{
				FileType ft= ActorUtil::GetFileType(asPaths[p]);
				switch(ft)
				{
					case FT_Bitmap:
					case FT_Sprite:
					case FT_Movie:
					case FT_Xml:
					case FT_Model:
					case FT_Lua:
						matches= category == EC_BGANIMATIONS || category == EC_GRAPHICS
							|| category == EC_SOUNDS;
						break;
					case FT_Ini:
						matches= category == EC_FONTS;
						break;
					case FT_Directory:
						{
							RString sXMLPath = asPaths[p] + "/default.xml";
							if(DoesFileExist(sXMLPath))
							{
								asElementPaths.push_back(sXMLPath);
								break;
							}
							RString sLuaPath = asPaths[p] + "/default.lua";
							if(DoesFileExist(sLuaPath))
							{
								asElementPaths.push_back(sLuaPath);
								break;
							}
						}
						matches= category == EC_BGANIMATIONS || category == EC_GRAPHICS;
						break;
					case FT_Sound:
						matches= category == EC_SOUNDS;
						break;
					default:
						matches= false;
						break;
				}
			}
			if(matches)
			{
				asElementPaths.push_back(asPaths[p]);
			}
		}
	}

	if( asElementPaths.size() == 0 )
		return false;	// This isn't fatal.

	FilterFileLanguages( asElementPaths );

	if( asElementPaths.size() > 1 )
	{
		g_ThemePathCache[category].clear();

		RString message = ssprintf(
			"ThemeManager:  There is more than one theme element that matches "
			"'%s/%s/%s'.  Please remove all but one of these matches: ",
			sThemeName.c_str(), sCategory.c_str(), MetricsGroupAndElementToFileName(sMetricsGroup,sElement).c_str() );
		message+= asElementPaths[1];
		for(std::size_t i= 1; i < asElementPaths.size(); ++i)
		{
			message+= ", " + asElementPaths[i];
		}

		switch( LuaHelpers::ReportScriptError(message, "", true) )
		{
			case Dialog::abort:
				RageException::Throw( "%s", message.c_str() );
				break;
			case Dialog::retry:
				ReloadMetrics();
				return GetPathInfoToRaw( out, sThemeName_, category, sMetricsGroup_, sElement_ );
			case Dialog::ignore:
			default:
				break;
		}
	}


	RString sPath = asElementPaths[0];
	bool bIsARedirect = GetExtension(sPath).CompareNoCase("redir")==0;

	if( !bIsARedirect )
	{
		out.sResolvedPath = sPath;
		out.sMatchingMetricsGroup = sMetricsGroup;
		out.sMatchingElement = sElement;
		return true;
	}

	RString sNewFileName;
	GetFileContents( sPath, sNewFileName, true );

	RString sNewClassName, sNewFile;
	FileNameToMetricsGroupAndElement(sNewFileName, sNewClassName, sNewFile);

	/* Important: We need to do a full search.  For example, BG redirs in
	 * the default theme point to "_shared background", and themes override
	 * just "_shared background"; the redirs in the default theme should end
	 * up resolving to the overridden background. */
	/* Use GetPathToOptional because we don't want report that there's an
	 * element missing. Instead we want to report that the redirect is invalid. */
	if( GetPathInfo(out,category,sNewClassName,sNewFile,true) )
		return true;

	RString sMessage = ssprintf(
			"ThemeManager:  The redirect '%s' points to the file '%s', which does not exist. "
			"Verify that this redirect is correct.",
			sPath.c_str(), sNewFileName.c_str());

	switch(LuaHelpers::ReportScriptError(sMessage, "", true))
	{
		case Dialog::retry:
			ReloadMetrics();
			return GetPathInfoToRaw( out, sThemeName_, category, sMetricsGroup_, sElement_ );
		case Dialog::ignore:
			GetPathInfo( out, category, "", "_missing" );
			return true;
		default:
			RageException::Throw( "%s", sMessage.c_str() );
	}
}

bool ThemeManager::GetPathInfoToAndFallback( PathInfo &out, ElementCategory category, const RString &sMetricsGroup_, const RString &sElement )
{
	RString sMetricsGroup( sMetricsGroup_ );

	int n = 100;
	while( n-- )
	{
		for (Theme const &theme : g_vThemes)
		{
			// search with requested name
			if( GetPathInfoToRaw( out, theme.sThemeName, category, sMetricsGroup, sElement ) )
				return true;
		}

		if( sMetricsGroup.empty() )
			return false;

		// search fallback name (if any)
		sMetricsGroup = GetMetricsGroupFallback( sMetricsGroup );
		if( sMetricsGroup.empty() )
			return false;
	}

	LuaHelpers::ReportScriptErrorFmt("Infinite recursion looking up theme element \"%s\"",
		MetricsGroupAndElementToFileName(sMetricsGroup, sElement).c_str());
	return false;
}

bool ThemeManager::GetPathInfo( PathInfo &out, ElementCategory category, const RString &sMetricsGroup_, const RString &sElement_, bool bOptional )
{
	/* Ugly: the parameters to this function may be a reference into g_vThemes,
	 * or something else that might suddenly go away when we call ReloadMetrics. */
	const RString sMetricsGroup = sMetricsGroup_;
	const RString sElement = sElement_;

	RString sFileName = MetricsGroupAndElementToFileName( sMetricsGroup, sElement );

	std::map<RString, PathInfo> &Cache = g_ThemePathCache[category];
	{
		std::map<RString, PathInfo>::const_iterator i;

		i = Cache.find( sFileName );
		if( i != Cache.end() )
		{
			out = i->second;
			return true;
		}
	}

try_element_again:

	// search the current theme
	if( GetPathInfoToAndFallback( out, category, sMetricsGroup, sElement ) )	// we found something
	{
		Cache[sFileName] = out;
		return true;
	}

	if( bOptional )
	{
		Cache[sFileName] = PathInfo();	// clear cache entry
		return false;
	}

	const RString &sCategory = ElementCategoryToString(category);

	// We can't fall back on _missing in Other: the file types are unknown.
	RString sMessage = "The theme element \"" + sCategory + "/" + sFileName +"\" is missing.";
	Dialog::Result res;
	if( category != EC_OTHER )
		res = Dialog::AbortRetryIgnore( sMessage, "MissingThemeElement" );
	else
		res = Dialog::AbortRetry( sMessage, "MissingThemeElement" );
	switch( res )
	{
	case Dialog::retry:
		ReloadMetrics();
		goto try_element_again;
	case Dialog::ignore:
		{
			RString element = sCategory + '/' + sFileName;
			RString error = "could not be found in \"" +
				GetThemeDirFromName(m_sCurThemeName) + "\" or \"" +
				GetThemeDirFromName(SpecialFiles::BASE_THEME_NAME) + "\".";
			LOG->UserLog("Theme element", element.c_str(), "%s", error.c_str());
			LOG->Warn( "%s %s", element.c_str(), error.c_str());
			LuaHelpers::ScriptErrorMessage("'" + element + "' " + error);
		}

		// Err?
		if( sFileName == "_missing" )
			RageException::Throw( "\"_missing\" isn't present in \"%s%s\".", GetThemeDirFromName(SpecialFiles::BASE_THEME_NAME).c_str(), sCategory.c_str() );

		GetPathInfo( out, category, "", "_missing" );
		Cache[sFileName] = out;
		return true;
	case Dialog::abort:
		LOG->UserLog( "Theme element", sCategory + '/' + sFileName,
					"could not be found in \"%s\" or \"%s\".",
					GetThemeDirFromName(m_sCurThemeName).c_str(),
					GetThemeDirFromName(SpecialFiles::BASE_THEME_NAME).c_str() );
		RageException::Throw( "Theme element \"%s/%s\" could not be found in \"%s\" or \"%s\".",
			sCategory.c_str(),
			sFileName.c_str(),
			GetThemeDirFromName(m_sCurThemeName).c_str(),
			GetThemeDirFromName(SpecialFiles::BASE_THEME_NAME).c_str() );
	DEFAULT_FAIL( res );
	}
}

RString ThemeManager::GetPath( ElementCategory category, const RString &sMetricsGroup, const RString &sElement, bool bOptional )
{
	PathInfo pi;
	GetPathInfo( pi, category, sMetricsGroup, sElement, bOptional );
	if(!bOptional && pi.sResolvedPath.empty())
	{
		LuaHelpers::ReportScriptErrorFmt("Theme element not found and not "
			"optional: Category: %s  Metrics group: %s  Element name: %s.",
			ElementCategoryToString(category).c_str(), sMetricsGroup.c_str(),
			sElement.c_str());
	}
	return pi.sResolvedPath;
}

RString ThemeManager::GetMetricsIniPath( const RString &sThemeName )
{
	return GetThemeDirFromName( sThemeName ) + SpecialFiles::METRICS_FILE;
}

bool ThemeManager::HasMetric( const RString &sMetricsGroup, const RString &sValueName )
{
	RString sThrowAway;
	if(sMetricsGroup == "" || sValueName == "")
	{
		return false;
	}
	return GetMetricRawRecursive( g_pLoadedThemeData->iniMetrics, sMetricsGroup, sValueName, sThrowAway );
}

bool ThemeManager::HasString( const RString &sMetricsGroup, const RString &sValueName )
{
	RString sThrowAway;
	if(sMetricsGroup == "" || sValueName == "")
	{
		return false;
	}
	return GetMetricRawRecursive( g_pLoadedThemeData->iniStrings, sMetricsGroup, sValueName, sThrowAway );
}

// the strings that were here before were moved to StepMania.cpp;
// sorry for the inconvienence/sloppy coding. -aj
void ThemeManager::ReloadMetrics()
{
	FILEMAN->FlushDirCache( GetCurThemeDir() );

	// Reloading Lua scripts can cause crashes; don't do this. -aj
	//UpdateLuaGlobals();

	// force a reload of the metrics cache
	LoadThemeMetrics( m_sCurThemeName, m_sCurLanguage );
	ReloadSubscribers();

	ClearThemePathCache();
}


RString ThemeManager::GetMetricsGroupFallback( const RString &sMetricsGroup )
{
	ASSERT( g_pLoadedThemeData != nullptr );

	// always look in iniMetrics for "Fallback"
	RString sFallback;
	if( !GetMetricRawRecursive(g_pLoadedThemeData->iniMetrics,sMetricsGroup,"Fallback",sFallback) )
		return RString();

	Lua *L = LUA->Get();
	LuaHelpers::RunExpression( L, sFallback );
	RString sRet;
	LuaHelpers::Pop( L, sRet );
	LUA->Release( L );

	return sRet;
}

bool ThemeManager::GetMetricRawRecursive( const IniFile &ini, const RString &sMetricsGroup_, const RString &sValueName, RString &sOut )
{
	ASSERT( sValueName != "" );
	RString sMetricsGroup( sMetricsGroup_ );

	int n = 100;
	while( n-- )
	{
		if( ini.GetValue(sMetricsGroup,sValueName,sOut) )
			return true;

		if( !sValueName.compare("Fallback") )
			return false;

		sMetricsGroup = GetMetricsGroupFallback( sMetricsGroup );
		if( sMetricsGroup.empty() )
			return false;
	}

	LuaHelpers::ReportScriptErrorFmt("Infinite recursion looking up theme metric \"%s::%s\".", sMetricsGroup.c_str(), sValueName.c_str());
	return false;
}

RString ThemeManager::GetMetricRaw( const IniFile &ini, const RString &sMetricsGroup_, const RString &sValueName_ )
{
	/* Ugly: the parameters to this function may be a reference into g_vThemes, or something
	 * else that might suddenly go away when we call ReloadMetrics. */
	const RString sMetricsGroup = sMetricsGroup_;
	const RString sValueName = sValueName_;

	for(;;)
	{
		RString ret;
		if( ThemeManager::GetMetricRawRecursive(ini, sMetricsGroup, sValueName, ret) )
		{
			return ret;
		}
		RString sCurMetricPath = GetMetricsIniPath( m_sCurThemeName );
		RString sDefaultMetricPath = GetMetricsIniPath( SpecialFiles::BASE_THEME_NAME );

		RString sType;
		if( &ini == &g_pLoadedThemeData->iniStrings )
			sType = "String";
		else if( &ini == &g_pLoadedThemeData->iniMetrics )
			sType = "Metric";
		else
			FAIL_M("");

		RString sMessage = ssprintf( "%s \"%s::%s\" is missing.",
			sType.c_str(),
			sMetricsGroup.c_str(),
			sValueName.c_str() );

		switch( LuaHelpers::ReportScriptError(sMessage, "", true) )
		{
			case Dialog::abort:
				{
					RageException::Throw( "%s \"%s::%s\" could not be found in \"%s\"' or \"%s\".",
						sType.c_str(),
						sMetricsGroup.c_str(),
						sValueName.c_str(),
						sCurMetricPath.c_str(),
						sDefaultMetricPath.c_str() );
				}
			case Dialog::retry:
				ReloadMetrics();
				continue;
			case Dialog::ignore:
				LOG->UserLog(
					sType,
					sMetricsGroup + "::" + sValueName,
					"could not be found in \"%s\" or \"%s\".",
					sCurMetricPath.c_str(),
					sDefaultMetricPath.c_str() );
				return RString();
			default:
				FAIL_M("Unexpected answer to Abort/Retry/Ignore dialog");
		}
	}
}

template<typename T>
void GetAndConvertMetric( const RString &sMetricsGroup, const RString &sValueName, T &out )
{
	Lua *L = LUA->Get();

	THEME->PushMetric( L, sMetricsGroup, sValueName );
	LuaHelpers::FromStack( L, out, -1 );
	lua_pop( L, 1 );

	LUA->Release(L);
}

/* Get a string metric. */
RString ThemeManager::GetMetric( const RString &sMetricsGroup, const RString &sValueName )
{
	RString sRet;
	GetAndConvertMetric( sMetricsGroup, sValueName, sRet );
	return sRet;
}

int ThemeManager::GetMetricI( const RString &sMetricsGroup, const RString &sValueName )
{
	int iRet = 0;
	GetAndConvertMetric( sMetricsGroup, sValueName, iRet );
	return iRet;
}

float ThemeManager::GetMetricF( const RString &sMetricsGroup, const RString &sValueName )
{
	float fRet = 0;
	GetAndConvertMetric( sMetricsGroup, sValueName, fRet );
	return fRet;
}

bool ThemeManager::GetMetricB( const RString &sMetricsGroup, const RString &sValueName )
{
	bool bRet = 0;
	GetAndConvertMetric( sMetricsGroup, sValueName, bRet );
	return bRet;
}

RageColor ThemeManager::GetMetricC( const RString &sMetricsGroup, const RString &sValueName )
{
	RageColor ret;
	GetAndConvertMetric( sMetricsGroup, sValueName, ret );
	return ret;
}

LuaReference ThemeManager::GetMetricR( const RString &sMetricsGroup, const RString &sValueName )
{
	LuaReference ref;
	GetMetric( sMetricsGroup, sValueName, ref );
	return ref;
}

void ThemeManager::PushMetric( Lua *L, const RString &sMetricsGroup, const RString &sValueName )
{
	if(sMetricsGroup == "" || sValueName == "")
	{
		LuaHelpers::ReportScriptError("PushMetric:  Attempted to fetch metric with empty group name or empty value name.");
		lua_pushnil(L);
		return;
	}
	RString sValue = GetMetricRaw( g_pLoadedThemeData->iniMetrics, sMetricsGroup, sValueName );

	RString sName = ssprintf( "%s::%s", sMetricsGroup.c_str(), sValueName.c_str() );
	if( EndsWith(sValueName, "Command") )
	{
		LuaHelpers::ParseCommandList( L, sValue, sName, false );
	}
	else
	{
		// Remove unary +, eg. "+50"; Lua doesn't support that.
		if( sValue.size() >= 1 && sValue[0] == '+' )
			sValue.erase( 0, 1 );

		LuaHelpers::RunExpression( L, sValue, sName );
	}
}

void ThemeManager::GetMetric( const RString &sMetricsGroup, const RString &sValueName, LuaReference &valueOut )
{
	Lua *L = LUA->Get();
	PushMetric( L, sMetricsGroup, sValueName );
	valueOut.SetFromStack( L );
	LUA->Release( L );
}

#if !defined(SMPACKAGE)
apActorCommands ThemeManager::GetMetricA( const RString &sMetricsGroup, const RString &sValueName )
{
	LuaReference *pRef = new LuaReference;
	GetMetric( sMetricsGroup, sValueName, *pRef );
	return apActorCommands( pRef );
}
#endif

void ThemeManager::EvaluateString( RString &sText )
{
	FontCharAliases::ReplaceMarkers( sText );
}

RString ThemeManager::GetNextTheme()
{
	std::vector<RString> as;
	GetThemeNames( as );
	unsigned i;
	for( i=0; i<as.size(); i++ )
		if( as[i].CompareNoCase(m_sCurThemeName)==0 )
			break;
	int iNewIndex = (i+1)%as.size();
	return as[iNewIndex];
}

RString ThemeManager::GetNextSelectableTheme()
{
	std::vector<RString> as;
	GetSelectableThemeNames( as );
	unsigned i;
	for( i=0; i<as.size(); i++ )
		if( as[i].CompareNoCase(m_sCurThemeName)==0 )
			break;
	int iNewIndex = (i+1)%as.size();
	return as[iNewIndex];
}

void ThemeManager::GetLanguagesForTheme( const RString &sThemeName, std::vector<RString>& asLanguagesOut )
{
	RString sLanguageDir = GetThemeDirFromName(sThemeName) + SpecialFiles::LANGUAGES_SUBDIR;
	std::vector<RString> as;
	GetDirListing( sLanguageDir + "*.ini", as );

	for (RString const &s : as)
	{
		// ignore metrics.ini
		if( s.CompareNoCase(SpecialFiles::METRICS_FILE)==0 )
			continue;

		// Ignore filenames with a space.  These are optional language inis that probably came from a mounted package.
		if( s.find(" ") != RString::npos )
			continue;

		// strip ".ini"
		RString s2 = s.Left( s.size()-4 );

		asLanguagesOut.push_back( s2 );
	}
}

RString ThemeManager::GetLanguageIniPath( const RString &sThemeName, const RString &sLanguage )
{
	return GetThemeDirFromName(sThemeName) + SpecialFiles::LANGUAGES_SUBDIR + sLanguage + ".ini";
}

void ThemeManager::GetOptionalLanguageIniPaths( std::vector<RString> &vsPathsOut, const RString &sThemeName, const RString &sLanguage )
{
	// optional ini names look like: "en PackageName.ini"
	GetDirListing( GetThemeDirFromName(sThemeName) + SpecialFiles::LANGUAGES_SUBDIR + sLanguage + " *.ini", vsPathsOut, false, true );
}

void ThemeManager::GetOptionNames( std::vector<RString>& AddTo )
{
	const XNode *cur = g_pLoadedThemeData->iniStrings.GetChild( "OptionNames" );
	if( cur )
	{
		FOREACH_CONST_Attr( cur, p )
			AddTo.push_back( p->first );
	}
}

static RString PseudoLocalize( RString s )
{
	s.Replace( "a", "\xc3\xa0\xc3\xa1" ); // àá
	s.Replace( "A", "\xc3\x80\xc3\x80" ); // ÀÀ
	s.Replace( "e", "\xc3\xa9\xc3\xa9" ); // éé
	s.Replace( "E", "\xc3\x89\xc3\x89" ); // ÉÉ
	s.Replace( "i", "\xc3\xad\xc3\xad" ); // íí
	s.Replace( "I", "\xc3\x8d\xc3\x8d" ); // ÍÍ
	s.Replace( "o", "\xc3\xb3\xc3\xb3" ); // óó
	s.Replace( "O", "\xc3\x93\xc3\x93" ); // ÓÓ
	s.Replace( "u", "\xc3\xbc\xc3\xbc" ); // üü
	s.Replace( "U", "\xc3\x9c\xc3\x9c" ); // ÜÜ
	s.Replace( "n", "\xc3\xb1" ); // ñ
	s.Replace( "N", "\xc3\x91" ); // Ñ
	s.Replace( "c", "\xc3\xa7" ); // ç
	s.Replace( "C", "\xc3\x87" ); // Ç
	// transformations that help expose punctuation assumptions
	//s.Replace( ":", " :" );	// this messes up "::" help text tip separator markers
	s.Replace( "?", " ?" );
	s.Replace( "!", " !" );

	return s;
}

RString ThemeManager::GetString( const RString &sMetricsGroup, const RString &sValueName_ )
{
	RString sValueName = sValueName_;
	if(sMetricsGroup == "" || sValueName == "")
	{
		LuaHelpers::ReportScriptError("PushMetric:  Attempted to fetch metric with empty group name or empty value name.");
		return "";
	}

	// TODO: Handle escaping = with \=
	DEBUG_ASSERT( sValueName.find('=') == sValueName.npos );

	// TODO: Move this escaping into IniFile?
	sValueName.Replace( "\r\n", "\\n" );
	sValueName.Replace( "\n", "\\n" );

	ASSERT( g_pLoadedThemeData != nullptr );
	RString s = GetMetricRaw( g_pLoadedThemeData->iniStrings, sMetricsGroup, sValueName );
	FontCharAliases::ReplaceMarkers( s );

	// Don't EvalulateString.  Strings are raw and shouldn't allow Lua.
	//EvaluateString( s );

	s.Replace( "\\n", "\n" );

	if( m_bPseudoLocalize )
	{
		// pseudolocalize ignoring replace markers.  e.g.: "%{steps} steps: %{author}"
		RString sTranslated;

		for( ; true; )
		{
			RString::size_type pos = s.find( "%{" );
			if( pos == s.npos )
			{
				sTranslated += PseudoLocalize( s );
				s = RString();
				break;
			}
			else
			{
				sTranslated += PseudoLocalize( s.substr(0,pos) );
				s.erase( s.begin(), s.begin()+pos );
			}

			pos = s.find( "}" );
			sTranslated += s.substr(0,pos+1);
			s.erase( s.begin(), s.begin()+pos+1 );
		}

		s = sTranslated;
	}

	return s;
}

void ThemeManager::GetMetricsThatBeginWith( const RString &sMetricsGroup_, const RString &sValueName, std::set<RString> &vsValueNamesOut )
{
	RString sMetricsGroup( sMetricsGroup_ );
	while( !sMetricsGroup.empty() )
	{
		const XNode *cur = g_pLoadedThemeData->iniMetrics.GetChild( sMetricsGroup );
		if( cur != nullptr )
		{
			// Iterate over all metrics that match.
			for( XAttrs::const_iterator j = cur->m_attrs.lower_bound( sValueName ); j != cur->m_attrs.end(); ++j )
			{
				const RString &sv = j->first;
				if( sv.Left(sValueName.size()) == sValueName )
					vsValueNamesOut.insert( sv );
				else	// we passed the last metric that matched sValueName
					break;
			}
		}

		// put the fallback (if any) in sMetricsGroup
		sMetricsGroup = GetMetricsGroupFallback( sMetricsGroup );
	}
}


RString ThemeManager::GetBlankGraphicPath()
{
	return SpecialFiles::THEMES_DIR + SpecialFiles::BASE_THEME_NAME + "/" + ElementCategoryToString(EC_GRAPHICS) + "/_blank.png";
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ThemeManager. */
class LunaThemeManager: public Luna<ThemeManager>
{
public:
	static int ReloadMetrics( T* p, lua_State *L )		{ p->ReloadMetrics(); return 0; }

	static int HasMetric( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasMetric(SArg(1),SArg(2))); return 1; }
	static int GetMetric( T* p, lua_State *L )
	{
		RString group= SArg(1);
		RString name= SArg(2);
		if(group == "" || name == "")
		{
			luaL_error(L, "Cannot fetch metric with empty group name or empty value name.");
		}
		p->PushMetric(L, group, name);
		return 1;
	}
	static int HasString( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasString(SArg(1),SArg(2))); return 1; }
	static int GetString( T* p, lua_State *L )
	{
		RString group= SArg(1);
		RString name= SArg(2);
		if(group == "" || name == "")
		{
			luaL_error(L, "Cannot fetch string with empty group name or empty value name.");
		}
		lua_pushstring(L, p->GetString(group, name));
		return 1;
	}
	static int GetPathInfoB( T* p, lua_State *L )
	{
		ThemeManager::PathInfo pi;
		p->GetPathInfo( pi, EC_BGANIMATIONS, SArg(1), SArg(2) );
		lua_pushstring(L, pi.sResolvedPath);
		lua_pushstring(L, pi.sMatchingMetricsGroup);
		lua_pushstring(L, pi.sMatchingElement);
		return 3;
	}
	// GENERAL_GET_PATH uses lua_toboolean instead of BArg because that makes
	// it optional. -Kyz
#define GENERAL_GET_PATH(get_path_name) \
	static int get_path_name(T* p, lua_State* L) \
	{ \
		lua_pushstring(L, p->get_path_name( \
				SArg(1), SArg(2), lua_toboolean(L, 3))); \
		return 1; \
	}
	GENERAL_GET_PATH(GetPathF);
	GENERAL_GET_PATH(GetPathG);
	GENERAL_GET_PATH(GetPathB);
	GENERAL_GET_PATH(GetPathS);
	GENERAL_GET_PATH(GetPathO);
#undef GENERAL_GET_PATH

	static int RunLuaScripts( T* p, lua_State *L )			{ p->RunLuaScripts(SArg(1)); return 1; }

	static int GetSelectableThemeNames( T* p, lua_State *L )
	{
		// pushes a table of theme folders from GetSelectableThemeNames()
		//lua_pushnumber(L, p->GetNumSelectableThemes() );
		std::vector<RString> sThemes;
		p->GetSelectableThemeNames(sThemes);
		LuaHelpers::CreateTableFromArray<RString>( sThemes, L );
		return 1;
	}

	static int GetNumSelectableThemes( T* p, lua_State *L )		{ lua_pushnumber(L, p->GetNumSelectableThemes() ); return 1; }

	DEFINE_METHOD( GetCurrentThemeDirectory, GetCurThemeDir() );
	DEFINE_METHOD( GetCurLanguage, GetCurLanguage() );
	static int GetThemeDisplayName( T* p, lua_State *L )			{  lua_pushstring(L, p->GetThemeDisplayName(p->GetCurThemeName())); return 1; }
	static int GetThemeAuthor( T* p, lua_State *L )			{  lua_pushstring(L, p->GetThemeAuthor(p->GetCurThemeName())); return 1; }
	DEFINE_METHOD( DoesThemeExist, DoesThemeExist(SArg(1)) );
	DEFINE_METHOD( IsThemeSelectable, IsThemeSelectable(SArg(1)) );
	DEFINE_METHOD( DoesLanguageExist, DoesLanguageExist(SArg(1)) );
	DEFINE_METHOD( GetCurThemeName, GetCurThemeName() );

	static void PushMetricNamesInGroup(IniFile const& ini, lua_State* L)
	{
		RString group_name= SArg(1);
		const XNode* metric_node= ini.GetChild(group_name);
		if(metric_node != nullptr)
		{
			// Placed in a table indexed by number, so the order is always the same.
			lua_createtable(L, metric_node->m_attrs.size(), 0);
			int next_index= 1;
			for(XAttrs::const_iterator n= metric_node->m_attrs.begin(); n != metric_node->m_attrs.end(); ++n)
			{
				LuaHelpers::Push(L, n->first);
				lua_rawseti(L, -2, next_index);
				++next_index;
			}
		}
		else
		{
			lua_pushnil(L);
		}
	}

	static int GetMetricNamesInGroup(T* p, lua_State* L)
	{
		PushMetricNamesInGroup(g_pLoadedThemeData->iniMetrics, L);
		return 1;
	}

	static int GetStringNamesInGroup(T* p, lua_State* L)
	{
		PushMetricNamesInGroup(g_pLoadedThemeData->iniStrings, L);
		return 1;
	}

	static int SetTheme(T* p, lua_State* L)
	{
		RString theme_name= SArg(1);
		if(!p->IsThemeSelectable(theme_name))
		{
			luaL_error(L, "SetTheme: Invalid Theme: '%s'", theme_name.c_str());
		}
		GameLoop::ChangeTheme(theme_name);
		return 0;
	}

	static int get_theme_fallback_list(T* p, lua_State* L)
	{
		lua_createtable(L, g_vThemes.size(), 0);
		int ret= lua_gettop(L);
		for(std::size_t tid= 0; tid < g_vThemes.size(); ++tid)
		{
			lua_pushstring(L, g_vThemes[tid].sThemeName.c_str());
			lua_rawseti(L, ret, tid+1);
		}
		return 1;
	}

	LunaThemeManager()
	{
		ADD_METHOD( ReloadMetrics );
		ADD_METHOD( GetMetric );
		ADD_METHOD( GetString );
		ADD_METHOD( GetPathInfoB );
		ADD_METHOD( GetPathF );
		ADD_METHOD( GetPathG );
		ADD_METHOD( GetPathB );
		ADD_METHOD( GetPathS );
		ADD_METHOD( GetPathO );
		ADD_METHOD( RunLuaScripts );
		ADD_METHOD( GetSelectableThemeNames );
		ADD_METHOD( GetNumSelectableThemes );
		ADD_METHOD( GetCurrentThemeDirectory );
		ADD_METHOD( GetCurLanguage );
		ADD_METHOD( GetThemeDisplayName );
		ADD_METHOD( GetThemeAuthor );
		ADD_METHOD( DoesThemeExist );
		ADD_METHOD( IsThemeSelectable );
		ADD_METHOD( DoesLanguageExist );
		ADD_METHOD( GetCurThemeName );
		ADD_METHOD( HasMetric );
		ADD_METHOD( HasString );
		ADD_METHOD( GetMetricNamesInGroup );
		ADD_METHOD( GetStringNamesInGroup );
		ADD_METHOD( SetTheme );
		ADD_METHOD(get_theme_fallback_list);
	}
};

LUA_REGISTER_CLASS( ThemeManager )
// lua end


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
