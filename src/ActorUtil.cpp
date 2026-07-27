#include "global.h"
#include "ActorUtil.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "EnumHelper.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "IniFile.h"
#include "LuaManager.h"
#include "Song.h"
#include "Course.h"
#include "GameState.h"

#include "arch/Dialog/Dialog.h"

#include <vector>


// Actor registration
static std::map<RString,CreateActorFn>	*g_pmapRegistrees = nullptr;

static bool IsRegistered( const RString& sClassName )
{
	return g_pmapRegistrees->find( sClassName ) != g_pmapRegistrees->end();
}

void ActorUtil::Register( const RString& sClassName, CreateActorFn pfn )
{
	if( g_pmapRegistrees == nullptr )
		g_pmapRegistrees = new std::map<RString,CreateActorFn>;

	std::map<RString,CreateActorFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter == g_pmapRegistrees->end(), ssprintf("Actor class '%s' already registered.", sClassName.c_str()) );

	(*g_pmapRegistrees)[sClassName] = pfn;
}

/* Resolves actor paths a la LoadActor("..."), with autowildcarding and .redir
 * files.  Returns a path *within* the Rage filesystem, unlike the FILEMAN
 * function of the same name. */
bool ActorUtil::ResolvePath( RString &sPath, const RString &sName, bool optional )
{
	CollapsePath( sPath );

	// If we know this is an exact match, don't bother with the GetDirListing,
	// so "foo" doesn't partial match "foobar" if "foo" exists.
	RageFileManager::FileType ft = FILEMAN->GetFileType( sPath );
	if( ft != RageFileManager::TYPE_FILE && ft != RageFileManager::TYPE_DIR )
	{
		std::vector<RString> asPaths;
		GetDirListing( sPath + "*", asPaths, false, true );	// return path too

		if( asPaths.empty() )
		{
			if(optional)
			{
				return false;
			}
			RString sError = ssprintf( "%s: references a file \"%s\" which doesn't exist", sName.c_str(), sPath.c_str() );
			switch(LuaHelpers::ReportScriptError(sError, "BROKEN_FILE_REFERENCE", true))
			{
			case Dialog::abort:
				RageException::Throw( "%s", sError.c_str() );
				break;
			case Dialog::retry:
				FILEMAN->FlushDirCache();
				return ResolvePath( sPath, sName );
			case Dialog::ignore:
				return false;
			default:
				FAIL_M("Invalid response to Abort/Retry/Ignore dialog");
			}
		}

		THEME->FilterFileLanguages( asPaths );

		if( asPaths.size() > 1 )
		{
			RString sError = ssprintf( "%s: references a file \"%s\" which has multiple matches", sName.c_str(), sPath.c_str() );
			sError += "\n" + join( "\n", asPaths );
			switch(LuaHelpers::ReportScriptError(sError, "BROKEN_FILE_REFERENCE", true))
			{
			case Dialog::abort:
				RageException::Throw( "%s", sError.c_str() );
				break;
			case Dialog::retry:
				FILEMAN->FlushDirCache();
				return ResolvePath( sPath, sName );
			case Dialog::ignore:
				asPaths.erase( asPaths.begin()+1, asPaths.end() );
				break;
			default:
				FAIL_M("Invalid response to Abort/Retry/Ignore dialog");
			}
		}

		sPath = asPaths[0];
	}

	if( ft == RageFileManager::TYPE_DIR )
	{
		RString sLuaPath = sPath + "/default.lua";
		if( DoesFileExist(sLuaPath) )
		{
			sPath = sLuaPath;
			return true;
		}
	}

	sPath = DerefRedir( sPath );
	return true;
}

namespace
{
	RString GetLegacyActorClass(XNode *pActor)
	{
		DEBUG_ASSERT(PREFSMAN->m_bQuirksMode);
		ASSERT(pActor);

		// The non-legacy LoadFromNode has already checked the Class and
		// Type attributes.

		if (pActor->GetAttr("Text") != nullptr)
			return "BitmapText";

		RString sFile;
		if (pActor->GetAttrValue("File", sFile) && sFile != "")
		{
			// Backward compatibility hacks for "special" filenames
			if (sFile.EqualsNoCase("songbackground"))
			{
				XNodeStringValue *pVal = new XNodeStringValue;
				Song *pSong = GAMESTATE->m_pCurSong;
				if (pSong && pSong->HasBackground())
					pVal->SetValue(pSong->GetBackgroundPath());
				else
					pVal->SetValue(THEME->GetPathG("Common", "fallback background"));
				pActor->AppendAttrFrom("Texture", pVal, false);
				return "Sprite";
			}
			else if (sFile.EqualsNoCase("songbanner"))
			{
				XNodeStringValue *pVal = new XNodeStringValue;
				Song *pSong = GAMESTATE->m_pCurSong;
				if (pSong && pSong->HasBanner())
					pVal->SetValue(pSong->GetBannerPath());
				else
					pVal->SetValue(THEME->GetPathG("Common", "fallback banner"));
				pActor->AppendAttrFrom("Texture", pVal, false);
				return "Sprite";
			}
			else if (sFile.EqualsNoCase("coursebanner"))
			{
				XNodeStringValue *pVal = new XNodeStringValue;
				Course *pCourse = GAMESTATE->m_pCurCourse;
				if (pCourse && pCourse->HasBanner())
					pVal->SetValue(pCourse->GetBannerPath());
				else
					pVal->SetValue(THEME->GetPathG("Common", "fallback banner"));
				pActor->AppendAttrFrom("Texture", pVal, false);
				return "Sprite";
			}
		}

		// Fallback: use XML tag name for actor class
		return pActor->m_sName;
	}
}

Actor *ActorUtil::LoadFromNode( const XNode* _pNode, Actor *pParentActor )
{
	ASSERT( _pNode != nullptr );

	XNode node = *_pNode;

	// Remove this in favor of using conditionals in Lua. -Chris
	// There are a number of themes out there that depend on this (including
	// sm-ssc default). Probably for the best to leave this in. -aj
	{
		bool bCond;
		if( node.GetAttrValue("Condition", bCond) && !bCond )
			return nullptr;
	}

	RString sClass;
	bool bHasClass = node.GetAttrValue( "Class", sClass );
	if( !bHasClass )
		bHasClass = node.GetAttrValue( "Type", sClass );

	bool bLegacy = (node.GetAttr( "_LegacyXml" ) != nullptr);
	if( !bHasClass && bLegacy )
		sClass = GetLegacyActorClass( &node );

	std::map<RString,CreateActorFn>::iterator iter = g_pmapRegistrees->find( sClass );
	if( iter == g_pmapRegistrees->end() )
	{
		RString sFile;
		if (bLegacy && node.GetAttrValue("File", sFile) && sFile != "")
		{
			RString sPath;
			// Handle absolute paths correctly
			if (sFile.Left(1) == "/")
				sPath = sFile;
			else
				sPath = Dirname(GetSourcePath(&node)) + sFile;
			if (ResolvePath(sPath, GetWhere(&node)))
			{
				Actor *pNewActor = MakeActor(sPath, pParentActor);
				if (pNewActor == nullptr)
					return nullptr;
				if (pParentActor)
					pNewActor->SetParent(pParentActor);
				pNewActor->LoadFromNode(&node);
				return pNewActor;
			}
		}

		// sClass is invalid
		RString sError = ssprintf( "%s: invalid Class \"%s\"",
			ActorUtil::GetWhere(&node).c_str(), sClass.c_str() );
		LuaHelpers::ReportScriptError(sError);
		return new Actor;	// Return a dummy object so that we don't crash in AutoActor later.
	}

	const CreateActorFn &pfn = iter->second;
	Actor *pRet = pfn();

	if( pParentActor )
		pRet->SetParent( pParentActor );

	pRet->LoadFromNode( &node );
	return pRet;
}

namespace
{
	XNode *LoadXNodeFromLuaShowErrors( const RString &sFile )
	{
		RString sScript;
		if( !GetFileContents(sFile, sScript) )
			return nullptr;

		Lua *L = LUA->Get();

		RString sError;
		if( !LuaHelpers::LoadScript(L, sScript, "@" + sFile, sError) )
		{
			LUA->Release( L );
			sError = ssprintf( "Lua runtime error: %s", sError.c_str() );
			LuaHelpers::ReportScriptError(sError);
			return nullptr;
		}

		XNode *pRet = nullptr;
		if( ActorUtil::LoadTableFromStackShowErrors(L) )
			pRet = XmlFileUtil::XNodeFromTable( L );

		LUA->Release( L );
		return pRet;
	}
}

/* Run the function at the top of the stack, which returns an actor description
 * table.  If the table was returned, return true and leave it on the stack.
 * If not, display an error, return false, and leave nothing on the stack. */
bool ActorUtil::LoadTableFromStackShowErrors( Lua *L )
{
	LuaReference func;
	lua_pushvalue( L, -1 );
	func.SetFromStack( L );

	RString Error= "Lua runtime error: ";
	if( !LuaHelpers::RunScriptOnStack(L, Error, 0, 1, true) )
	{
		lua_pop( L, 1 );
		return false;
	}

	if( lua_type(L, -1) != LUA_TTABLE )
	{
		lua_pop( L, 1 );

		func.PushSelf( L );
		lua_Debug debug;
		lua_getinfo( L, ">nS", &debug );

		Error = ssprintf( "%s: must return a table", debug.short_src );

		LuaHelpers::ReportScriptError(Error, "LUA_ERROR");
		return false;
	}
	return true;
}

// NOTE: This function can return nullptr if the actor should not be displayed.
// Callers should be aware of this and handle it appropriately.
Actor* ActorUtil::MakeActor( const RString &sPath_, Actor *pParentActor )
{
	RString sPath( sPath_ );

	FileType ft = GetFileType( sPath );
	switch( ft )
	{
	case FT_Lua:
		{
			std::unique_ptr<XNode> pNode( LoadXNodeFromLuaShowErrors(sPath) );
			if( pNode.get() == nullptr )
			{
				// XNode will warn about the error
				return new Actor;
			}

			Actor *pRet = ActorUtil::LoadFromNode( pNode.get(), pParentActor );
			return pRet;
		}
	case FT_Xml:
		{
			// Legacy actors; only supported in quirks mode
			if ( !PREFSMAN->m_bQuirksMode )
				return new Actor;

			XNode xml;
			if ( !XmlFileUtil::LoadFromFileShowErrors(xml, sPath) )
				return new Actor;
			XmlFileUtil::CompileXNodeTree( &xml, sPath );
			XmlFileUtil::AnnotateXNodeTree( &xml, sPath );
			return LoadFromNode( &xml, pParentActor );
		}
	case FT_Directory:
		{
			if( sPath.Right(1) != "/" )
				sPath += '/';

			RString sXml = sPath + "default.xml";
			if (DoesFileExist(sXml))
				return MakeActor(sXml, pParentActor);

			XNode xml;
			xml.AppendAttr( "Class", "BGAnimation" );
			xml.AppendAttr( "AniDir", sPath );

			return ActorUtil::LoadFromNode( &xml, pParentActor );
		}
	case FT_Bitmap:
	case FT_Movie:
		{
			XNode xml;
			xml.AppendAttr( "Class", "Sprite" );
			xml.AppendAttr( "Texture", sPath );

			return ActorUtil::LoadFromNode( &xml, pParentActor );
		}
	case FT_Sprite:
		{
			// Legacy actor; only supported in quirks mode
			if( !PREFSMAN->m_bQuirksMode )
				return new Actor;

			IniFile ini;
			ini.ReadFile( sPath );
			XmlFileUtil::AnnotateXNodeTree( &ini, sPath );

			return ActorUtil::LoadFromNode( ini.GetChild("Sprite"), pParentActor );
		}
	case FT_Model:
		{
			XNode xml;
			xml.AppendAttr( "Class", "Model" );
			xml.AppendAttr( "Meshes", sPath );
			xml.AppendAttr( "Materials", sPath );
			xml.AppendAttr( "Bones", sPath );

			return ActorUtil::LoadFromNode( &xml, pParentActor );
		}
	default:
		{
			LOG->Warn( "File \"%s\" has unknown type, \"%s\".", sPath.c_str(), FileTypeToString(ft).c_str() );

			XNode xml;
			xml.AppendAttr( "Class", "Actor" );
			return ActorUtil::LoadFromNode( &xml, pParentActor );
		}
	}
}

RString ActorUtil::GetSourcePath( const XNode *pNode )
{
	RString sRet;
	pNode->GetAttrValue( "_Source", sRet );
	if( sRet.substr(0, 1) == "@" )
		sRet.erase( 0, 1 );

	return sRet;
}

RString ActorUtil::GetWhere( const XNode *pNode )
{
	RString sPath = GetSourcePath( pNode );

	int iLine;
	if( pNode->GetAttrValue("_Line", iLine) )
		sPath += ssprintf( ":%i", iLine );
	return sPath;
}

bool ActorUtil::GetAttrPath( const XNode *pNode, const RString &sName, RString &sOut, bool optional )
{
	if( !pNode->GetAttrValue(sName, sOut) )
		return false;

	bool bIsRelativePath = sOut.Left(1) != "/";
	if( bIsRelativePath )
	{
		RString sDir;
		if( !pNode->GetAttrValue("_Dir", sDir) )
		{
			if(!optional)
			{
				LOG->Warn( "Relative path \"%s\", but path is unknown", sOut.c_str() );
			}
			return false;
		}
		sOut = sDir+sOut;
	}

	return ActorUtil::ResolvePath( sOut, ActorUtil::GetWhere(pNode), optional );
}

apActorCommands ActorUtil::ParseActorCommands( const RString &sCommands, const RString &sName )
{
	Lua *L = LUA->Get();
	LuaHelpers::ParseCommandList( L, sCommands, sName, false );
	LuaReference *pRet = new LuaReference;
	pRet->SetFromStack( L );
	LUA->Release( L );

	return apActorCommands( pRet );
}

void ActorUtil::SetXY( Actor& actor, const RString &sMetricsGroup )
{
	ASSERT( !actor.GetName().empty() );

	/*
	 * Hack: This is run after InitCommand, and InitCommand might set X/Y.  If
	 * these are both 0, leave the actor where it is.  If InitCommand doesn't,
	 * then 0,0 is the default, anyway.
	 */
	float fX = THEME->GetMetricF(sMetricsGroup,actor.GetName()+"X");
	float fY = THEME->GetMetricF(sMetricsGroup,actor.GetName()+"Y");
	if( fX != 0 || fY != 0 )
		actor.SetXY( fX, fY );
}

void ActorUtil::LoadCommand( Actor& actor, const RString &sMetricsGroup, const RString &sCommandName )
{
	ActorUtil::LoadCommandFromName( actor, sMetricsGroup, sCommandName, actor.GetName() );
}

void ActorUtil::LoadCommandFromName( Actor& actor, const RString &sMetricsGroup, const RString &sCommandName, const RString &sName )
{
	actor.AddCommand( sCommandName, THEME->GetMetricA(sMetricsGroup,sName+sCommandName+"Command") );
}

void ActorUtil::LoadAllCommands( Actor& actor, const RString &sMetricsGroup )
{
	LoadAllCommandsFromName( actor, sMetricsGroup, actor.GetName() );
}

void ActorUtil::LoadAllCommandsFromName( Actor& actor, const RString &sMetricsGroup, const RString &sName )
{
	std::set<RString> vsValueNames;
	THEME->GetMetricsThatBeginWith( sMetricsGroup, sName, vsValueNames );

	for (RString const & sv : vsValueNames)
	{
		static const RString sEnding = "Command";
		if( EndsWith(sv,sEnding) )
		{
			RString sCommandName( sv.begin()+sName.size(), sv.end()-sEnding.size() );
			LoadCommandFromName( actor, sMetricsGroup, sCommandName, sName );
		}
	}
}

static bool CompareActorsByZPosition(const Actor *p1, const Actor *p2)
{
	return p1->GetZ() < p2->GetZ();
}

void ActorUtil::SortByZPosition( std::vector<Actor*> &vActors )
{
	// Preserve ordering of Actors with equal Z positions.
	stable_sort( vActors.begin(), vActors.end(), CompareActorsByZPosition );
}

static const char *FileTypeNames[] = {
	"Bitmap",
	"Sprite",
	"Sound",
	"Movie",
	"Directory",
	"Xml",
	"Model",
	"Lua",
	"Ini",
};
XToString( FileType );
LuaXType( FileType );

// convenience so the for-loop lines can be shorter.
typedef std::map<RString, FileType> etft_cont_t;
typedef std::map<FileType, std::vector<RString> > fttel_cont_t;
etft_cont_t ExtensionToFileType;
fttel_cont_t FileTypeToExtensionList;

void ActorUtil::InitFileTypeLists()
{
	// This function creates things to serve two purposes:
	// 1.  A map from extensions to filetypes, so extensions can be converted.
	// 2.  A reverse map for things that need a list of extensions to look for.
	// The first section creates the map from extensions to filetypes, then the
	// second section uses that map to build the reverse map.
	ExtensionToFileType["lua"]= FT_Lua;

	ExtensionToFileType["xml"]= FT_Xml;

	ExtensionToFileType["ini"]= FT_Ini;

	// Update RageSurfaceUtils when adding new image formats.
	ExtensionToFileType["bmp"]= FT_Bitmap;
	ExtensionToFileType["gif"]= FT_Bitmap;
	ExtensionToFileType["jpeg"]= FT_Bitmap;
	ExtensionToFileType["jpg"]= FT_Bitmap;
	ExtensionToFileType["png"]= FT_Bitmap;

	// Update RageSoundReader_FileReader when adding new sound formats.
	ExtensionToFileType["mp3"]= FT_Sound;
	ExtensionToFileType["oga"]= FT_Sound;
	ExtensionToFileType["ogg"]= FT_Sound;
	ExtensionToFileType["wav"]= FT_Sound;

	// ffmpeg takes care of loading videos, not sure whether this list should
	// have everything ffmpeg supports.
	ExtensionToFileType["avi"]= FT_Movie;
	ExtensionToFileType["f4v"]= FT_Movie;
	ExtensionToFileType["flv"]= FT_Movie;
	ExtensionToFileType["mkv"]= FT_Movie;
	ExtensionToFileType["mp4"]= FT_Movie;
	ExtensionToFileType["mpeg"]= FT_Movie;
	ExtensionToFileType["mpg"]= FT_Movie;
	ExtensionToFileType["mov"]= FT_Movie;
	ExtensionToFileType["ogv"]= FT_Movie;
	ExtensionToFileType["webm"]= FT_Movie;
	ExtensionToFileType["wmv"]= FT_Movie;

	ExtensionToFileType["sprite"]= FT_Sprite;

	ExtensionToFileType["txt"]= FT_Model;

	// When adding new extensions, do not add them below this line.  This line
	// marks the point where the function switches to building the reverse map.
	for(etft_cont_t::iterator curr_ext= ExtensionToFileType.begin();
		curr_ext != ExtensionToFileType.end(); ++curr_ext)
	{
		FileTypeToExtensionList[curr_ext->second].push_back(curr_ext->first);
	}
}

std::vector<RString> const& ActorUtil::GetTypeExtensionList(FileType ft)
{
	return FileTypeToExtensionList[ft];
}

void ActorUtil::AddTypeExtensionsToList(FileType ft, std::vector<RString>& add_to)
{
	fttel_cont_t::iterator ext_list= FileTypeToExtensionList.find(ft);
	if(ext_list != FileTypeToExtensionList.end())
	{
		add_to.reserve(add_to.size() + ext_list->second.size());
		for(std::vector<RString>::iterator curr= ext_list->second.begin();
				curr != ext_list->second.end(); ++curr)
		{
			add_to.push_back(*curr);
		}
	}
}

FileType ActorUtil::GetFileType( const RString &sPath )
{
	RString sExt = GetExtension( sPath );
	sExt.MakeLower();

	etft_cont_t::iterator conversion_entry= ExtensionToFileType.find(sExt);
	if(conversion_entry != ExtensionToFileType.end())
	{
		return conversion_entry->second;
	}
	else if(sPath.size() > 0 && sPath[sPath.size()-1] == '/')
	{
		return FT_Directory;
	}
	/* Do this last, to avoid the IsADirectory in most cases. */
	else if(IsADirectory(sPath))
	{
		return FT_Directory;
	}
	return FileType_Invalid;
}


// lua start
#include "LuaBinding.h"

namespace
{
	int GetFileType( lua_State *L )		{ Enum::Push( L, ActorUtil::GetFileType(SArg(1)) ); return 1; }
	int ResolvePath( lua_State *L )
	{
		RString sPath( SArg(1) );
		int iLevel = IArg(2);
		bool optional= lua_toboolean(L, 3);
		luaL_where( L, iLevel );
		RString sWhere = lua_tostring( L, -1 );
		if( sWhere.size() > 2 && sWhere.substr(sWhere.size()-2, 2) == ": " )
			sWhere = sWhere.substr( 0, sWhere.size()-2 ); // remove trailing ": "

		LUA->YieldLua();
		bool bRet = ActorUtil::ResolvePath(sPath, sWhere, optional);
		LUA->UnyieldLua();

		if( !bRet )
			return 0;
		LuaHelpers::Push( L, sPath );
		return 1;
	}
	int IsRegisteredClass( lua_State *L )
	{
		lua_pushboolean( L, IsRegistered(SArg(1)) );
		return 1;
	}
	static void name_error(Actor* p, lua_State* L)
	{
		if(p->GetName() == "")
		{
			luaL_error(L, "LoadAllCommands requires the actor to have a name.");
		}
	}
	static int LoadAllCommands( lua_State *L )
	{
		Actor *p = Luna<Actor>::check( L, 1 );
		name_error(p, L);
		ActorUtil::LoadAllCommands( p, SArg(2) );
		return 0;
	}
	static int LoadAllCommandsFromName( lua_State *L )
	{
		Actor *p = Luna<Actor>::check( L, 1 );
		name_error(p, L);
		ActorUtil::LoadAllCommandsFromName( *p, SArg(2), SArg(3) );
		return 0;
	}
	static int LoadAllCommandsAndSetXY( lua_State *L )
	{
		Actor *p = Luna<Actor>::check( L, 1 );
		name_error(p, L);
		ActorUtil::LoadAllCommandsAndSetXY( p, SArg(2) );
		return 0;
	}

	const luaL_Reg ActorUtilTable[] =
	{
		LIST_METHOD( GetFileType ),
		LIST_METHOD( ResolvePath ),
		LIST_METHOD( IsRegisteredClass ),
		LIST_METHOD( LoadAllCommands ),
		LIST_METHOD( LoadAllCommandsFromName ),
		LIST_METHOD( LoadAllCommandsAndSetXY ),
		{ nullptr, nullptr }
	};
}

LUA_REGISTER_NAMESPACE( ActorUtil )

/*
 * (c) 2003-2004 Chris Danford
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
