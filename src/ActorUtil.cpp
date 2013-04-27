#include "global.h"
#include "ActorUtil.h"
#include "ThemeManager.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "EnumHelper.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"
#include "LuaManager.h"
#include "Foreach.h"

#include "arch/Dialog/Dialog.h"


// Actor registration
static map<RString,CreateActorFn>	*g_pmapRegistrees = NULL;

static bool IsRegistered( const RString& sClassName )
{
	return g_pmapRegistrees->find( sClassName ) != g_pmapRegistrees->end();
}

void ActorUtil::Register( const RString& sClassName, CreateActorFn pfn )
{
	if( g_pmapRegistrees == NULL )
		g_pmapRegistrees = new map<RString,CreateActorFn>;

	map<RString,CreateActorFn>::iterator iter = g_pmapRegistrees->find( sClassName );
	ASSERT_M( iter == g_pmapRegistrees->end(), ssprintf("Actor class '%s' already registered.", sClassName.c_str()) );

	(*g_pmapRegistrees)[sClassName] = pfn;
}

bool ActorUtil::ResolvePath( RString &sPath, const RString &sName )
{
	CollapsePath( sPath );

	// If we know this is an exact match, don't bother with the GetDirListing,
	// so "foo" doesn't partial match "foobar" if "foo" exists.
	RageFileManager::FileType ft = FILEMAN->GetFileType( sPath );
	if( ft != RageFileManager::TYPE_FILE && ft != RageFileManager::TYPE_DIR )
	{
		vector<RString> asPaths;
		GetDirListing( sPath + "*", asPaths, false, true );	// return path too

		if( asPaths.empty() )
		{
			RString sError = ssprintf( "%s: references a file \"%s\" which doesn't exist", sName.c_str(), sPath.c_str() );
			switch( Dialog::AbortRetryIgnore( sError, "BROKEN_FILE_REFERENCE" ) )
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
			switch( Dialog::AbortRetryIgnore( sError, "BROKEN_FILE_REFERENCE" ) )
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

Actor* ActorUtil::LoadFromNode( const XNode* pNode, Actor *pParentActor )
{
	ASSERT( pNode != NULL );

	// Remove this in favor of using conditionals in Lua. -Chris
	// There are a number of themes out there that depend on this (including
	// sm-ssc default). Probably for the best to leave this in. -aj
	{
		bool bCond;
		if( pNode->GetAttrValue("Condition", bCond) && !bCond )
			return NULL;
	}

	RString sClass;
	bool bHasClass = pNode->GetAttrValue( "Class", sClass );
	if( !bHasClass )
		bHasClass = pNode->GetAttrValue( "Type", sClass );

	map<RString,CreateActorFn>::iterator iter = g_pmapRegistrees->find( sClass );
	if( iter == g_pmapRegistrees->end() )
	{
		// sClass is invalid
		RString sError = ssprintf( "%s: invalid Class \"%s\"",
			ActorUtil::GetWhere(pNode).c_str(), sClass.c_str() );
		Dialog::OK( sError );
		return new Actor;	// Return a dummy object so that we don't crash in AutoActor later.
	}

	const CreateActorFn &pfn = iter->second;
	Actor *pRet = pfn();

	if( pParentActor )
		pRet->SetParent( pParentActor );

	pRet->LoadFromNode( pNode );
	return pRet;
}

namespace
{
	XNode *LoadXNodeFromLuaShowErrors( const RString &sFile )
	{
		RString sScript;
		if( !GetFileContents(sFile, sScript) )
			return NULL;

		Lua *L = LUA->Get();

		RString sError;
		if( !LuaHelpers::LoadScript(L, sScript, "@" + sFile, sError) )
		{
			LUA->Release( L );
			sError = ssprintf( "Lua runtime error: %s", sError.c_str() );
			Dialog::OK( sError, "LUA_ERROR" );
			return NULL;
		}

		XNode *pRet = NULL;
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

	RString sError;
	if( !LuaHelpers::RunScriptOnStack(L, sError, 0, 1) )
	{
		lua_pop( L, 1 );
		sError = ssprintf( "Lua runtime error: %s", sError.c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}

	if( lua_type(L, -1) != LUA_TTABLE )
	{
		lua_pop( L, 1 );

		func.PushSelf( L );
		lua_Debug debug;
		lua_getinfo( L, ">nS", &debug );

		sError = ssprintf( "%s: must return a table", debug.short_src );

		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}
	return true;
}

Actor* ActorUtil::MakeActor( const RString &sPath_, Actor *pParentActor )
{
	RString sPath( sPath_ );

	FileType ft = GetFileType( sPath );
	switch( ft )
	{
	case FT_Lua:
		{
			auto_ptr<XNode> pNode( LoadXNodeFromLuaShowErrors(sPath) );
			if( pNode.get() == NULL )
			{
				// XNode will warn about the error
				return new Actor;
			}

			Actor *pRet = ActorUtil::LoadFromNode( pNode.get(), pParentActor );
			return pRet;
		}
	case FT_Directory:
		{
			if( sPath.Right(1) != "/" )
				sPath += '/';
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

bool ActorUtil::GetAttrPath( const XNode *pNode, const RString &sName, RString &sOut )
{
	if( !pNode->GetAttrValue(sName, sOut) )
		return false;

	bool bIsRelativePath = sOut.Left(1) != "/";
	if( bIsRelativePath )
	{
		RString sDir;
		if( !pNode->GetAttrValue("_Dir", sDir) )
		{
			LOG->Warn( "Relative path \"%s\", but path is unknown", sOut.c_str() );
			return false;
		}
		sOut = sDir+sOut;
	}

	return ActorUtil::ResolvePath( sOut, ActorUtil::GetWhere(pNode) );
}

apActorCommands ActorUtil::ParseActorCommands( const RString &sCommands, const RString &sName )
{
	Lua *L = LUA->Get();
	LuaHelpers::ParseCommandList( L, sCommands, sName );
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
	set<RString> vsValueNames;
	THEME->GetMetricsThatBeginWith( sMetricsGroup, sName, vsValueNames );

	FOREACHS_CONST( RString, vsValueNames, v )
	{
		const RString &sv = *v;
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

void ActorUtil::SortByZPosition( vector<Actor*> &vActors )
{
	// Preserve ordering of Actors with equal Z positions.
	stable_sort( vActors.begin(), vActors.end(), CompareActorsByZPosition );
}

static const char *FileTypeNames[] = {
	"Bitmap", 
	"Sound", 
	"Movie", 
	"Directory", 
	"Lua", 
	"Model", 
};
XToString( FileType );
LuaXType( FileType );

FileType ActorUtil::GetFileType( const RString &sPath )
{
	RString sExt = GetExtension( sPath );
	sExt.MakeLower();
	
	if( sExt=="lua" )		return FT_Lua;
	else if( 
		sExt=="png" ||
		sExt=="jpg" || 
		sExt=="jpeg" || 
		sExt=="gif" || 
		sExt=="bmp" )		return FT_Bitmap;
	else if( 
		sExt=="ogg" ||
		sExt=="oga" ||
		sExt=="wav" || 
		sExt=="mp3" )		return FT_Sound;
	else if( 
		sExt=="ogv" || 
		sExt=="avi" || 
		sExt=="mpeg" || 
		sExt=="mpg" )		return FT_Movie;
	else if( 
		sExt=="txt" )		return FT_Model;
	else if( sPath.size() > 0 && sPath[sPath.size()-1] == '/' )
					return FT_Directory;
	/* Do this last, to avoid the IsADirectory in most cases. */
	else if( IsADirectory(sPath)  )	return FT_Directory;
	else				return FileType_Invalid;
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
		luaL_where( L, iLevel );
		RString sWhere = lua_tostring( L, -1 );
		if( sWhere.size() > 2 && sWhere.substr(sWhere.size()-2, 2) == ": " )
			sWhere = sWhere.substr( 0, sWhere.size()-2 ); // remove trailing ": "

		LUA->YieldLua();
		bool bRet = ActorUtil::ResolvePath( sPath, sWhere );
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
	static int LoadAllCommands( lua_State *L )
	{
		Actor *p = Luna<Actor>::check( L, 1 );
		ActorUtil::LoadAllCommands( p, SArg(2) );
		return 0;
	}
	static int LoadAllCommandsFromName( lua_State *L )
	{
		Actor *p = Luna<Actor>::check( L, 1 );
		ActorUtil::LoadAllCommandsFromName( *p, SArg(2), SArg(3) );
		return 0;
	}
	static int LoadAllCommandsAndSetXY( lua_State *L )
	{
		Actor *p = Luna<Actor>::check( L, 1 );
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
		{ NULL, NULL }
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
