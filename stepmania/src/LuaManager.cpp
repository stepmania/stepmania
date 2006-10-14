#include "global.h"
#include "LuaManager.h"
#include "LuaReference.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageThreads.h"
#include "Foreach.h"
#include "arch/Dialog/Dialog.h"
#include "XmlFile.h"
#include "Command.h"

#include <sstream>
#include <csetjmp>
#include <cassert>

LuaManager *LUA = NULL;
static LuaFunctionList *g_LuaFunctions = NULL;

#if defined(_MSC_VER) || defined (_XBOX)
	/* "interaction between '_setjmp' and C++ object destruction is non-portable"
	 * We don't care; we'll throw a fatal exception immediately anyway. */
	#pragma warning (disable : 4611)
#endif

namespace LuaHelpers
{
	template<> void Push<bool>( lua_State *L, const bool &Object );
	template<> void Push<float>( lua_State *L, const float &Object );
	template<> void Push<int>( lua_State *L, const int &Object );
	template<> void Push<RString>( lua_State *L, const RString &Object );

	template<> bool FromStack<bool>( Lua *L, bool &Object, int iOffset );
	template<> bool FromStack<float>( Lua *L, float &Object, int iOffset );
	template<> bool FromStack<int>( Lua *L, int &Object, int iOffset );
	template<> bool FromStack<RString>( Lua *L, RString &Object, int iOffset );
}

void LuaManager::SetGlobal( const RString &sName, int val )
{
	Lua *L = LUA->Get();
	LuaHelpers::Push( L, val );
	lua_setglobal( L, sName );
	LUA->Release( L );
}

void LuaManager::SetGlobal( const RString &sName, const RString &val )
{
	Lua *L = LUA->Get();
	LuaHelpers::Push( L, val );
	lua_setglobal( L, sName );
	LUA->Release( L );
}

void LuaManager::UnsetGlobal( const RString &sName )
{
	Lua *L = LUA->Get();
	lua_pushnil( L );
	lua_setglobal( L, sName );
	LUA->Release( L );
}

namespace LuaHelpers
{
	template<> void Push<bool>( lua_State *L, const bool &Object ) { lua_pushboolean( L, Object ); }
	template<> void Push<float>( lua_State *L, const float &Object ) { lua_pushnumber( L, Object ); }
	template<> void Push<int>( lua_State *L, const int &Object ) { lua_pushinteger( L, Object ); }
	template<> void Push<RString>( lua_State *L, const RString &Object ) { lua_pushlstring( L, Object.data(), Object.size() ); }

	template<> bool FromStack<bool>( Lua *L, bool &Object, int iOffset ) { Object = !!lua_toboolean( L, iOffset ); return true; }
	template<> bool FromStack<float>( Lua *L, float &Object, int iOffset ) { Object = (float)lua_tonumber( L, iOffset ); return true; }
	template<> bool FromStack<int>( Lua *L, int &Object, int iOffset ) { Object = lua_tointeger( L, iOffset ); return true; }
	template<> bool FromStack<RString>( Lua *L, RString &Object, int iOffset )
	{
		size_t iLen;
		const char *pStr = lua_tolstring( L, iOffset, &iLen );
		if( pStr != NULL )
			Object.assign( pStr, iLen );
		else
			Object.clear();

		return pStr != NULL;
	}
}

void LuaHelpers::CreateTableFromArrayB( Lua *L, const vector<bool> &aIn )
{
	lua_newtable( L );
	for( unsigned i = 0; i < aIn.size(); ++i )
	{
		lua_pushboolean( L, aIn[i] );
		lua_rawseti( L, -2, i+1 );
	}
}

void LuaHelpers::ReadArrayFromTableB( Lua *L, vector<bool> &aOut )
{
	luaL_checktype( L, -1, LUA_TTABLE );

	for( unsigned i = 0; i < aOut.size(); ++i )
	{
		lua_rawgeti( L, -1, i+1 );
		bool bOn = !!lua_toboolean( L, -1 );
		aOut[i] = bOn;
		lua_pop( L, 1 );
	}
}


static int LuaPanic( lua_State *L )
{
	RString sErr;
	LuaHelpers::Pop( L, sErr );
	
	lua_Debug ar;
	int level = 0;
	
	while( lua_getstack(L, level++, &ar) )
	{
		if( !lua_getinfo(L, "nSluf", &ar) )
			break;
		// The function is now on the top of the stack.
		const char *file = ar.source[0] == '@' ? ar.source + 1 : ar.short_src;
		const char *name;
		vector<RString> vArgs;
		
		for( int i = 1; i <= ar.nups && (name = lua_getupvalue(L, -1, i)) != NULL; ++i )
		{
			// XXX: do we need to do local variables for lua functions instead?
			vArgs.push_back( ssprintf("%s = %s", name, lua_tostring(L, -1)) );
			lua_pop( L, 1 ); // pop value
		}
		lua_pop( L, 1 ); // pop function
		
		name = ar.name ? ar.name : "[UNKNOWN]";
		sErr += ssprintf( "\n%s %s %s( %s ) %s:%d", ar.namewhat, ar.what, name,
				  join(",", vArgs).c_str(), file, ar.currentline );
	}		

	RageException::Throw( "%s", sErr.c_str() );
}

// Actor registration
static vector<RegisterWithLuaFn>	*g_vRegisterActorTypes = NULL;

void LuaManager::Register( RegisterWithLuaFn pfn )
{
	if( g_vRegisterActorTypes == NULL )
		g_vRegisterActorTypes = new vector<RegisterWithLuaFn>;

	g_vRegisterActorTypes->push_back( pfn );
}


LuaManager::LuaManager()
{
	LUA = this;	// so that LUA is available when we call the Register functions

	m_pLock = new RageMutex( "Lua" );

	L = lua_open();
	ASSERT( L );

	lua_atpanic( L, LuaPanic );

	luaopen_base( L );
	luaopen_math( L );
	luaopen_string( L );
	luaopen_table( L );
	luaopen_debug( L );
	lua_settop(L, 0); // luaopen_* pushes stuff onto the stack that we don't need

	RegisterTypes();
}

LuaManager::~LuaManager()
{
	lua_close( L );
	delete m_pLock;
}

/* Keep track of Lua stack, and enforce that when we release Lua, the stack is
 * back where it was when we locked it. */
static int g_iStackCounts[32];
static int g_iNumStackCounts = 0;
Lua *LuaManager::Get()
{
	m_pLock->Lock();
	if( size_t(g_iNumStackCounts) < ARRAYLEN(g_iStackCounts) )
	{
		g_iStackCounts[g_iNumStackCounts] = lua_gettop( L );
	}
	++g_iNumStackCounts;
	return L;
}

void LuaManager::Release( Lua *&p )
{
	ASSERT( p == L );
	ASSERT( g_iNumStackCounts != 0 );
	--g_iNumStackCounts;
	if( size_t(g_iNumStackCounts) < ARRAYLEN(g_iStackCounts) )
	{
		ASSERT( g_iStackCounts[g_iNumStackCounts] == lua_gettop(L) );
	}
	m_pLock->Unlock();
	p = NULL;
}


void LuaManager::RegisterTypes()
{
	for( const LuaFunctionList *p = g_LuaFunctions; p; p=p->next )
		lua_register( L, p->name, p->func );
	
	if( g_vRegisterActorTypes )
	{
		for( unsigned i=0; i<g_vRegisterActorTypes->size(); i++ )
		{
			RegisterWithLuaFn fn = (*g_vRegisterActorTypes)[i];
			fn( L );
		}
	}
}


namespace
{
	struct LClass
	{
		RString m_sBaseName;
		vector<RString> m_vMethods;
	};
}	

XNode *LuaManager::GetLuaInformation() const
{
	Lua *L = LUA->Get();

	XNode *pLuaNode = new XNode( "Lua" );

	XNode *pGlobalsNode = pLuaNode->AppendChild( "GlobalFunctions" );
	XNode *pClassesNode = pLuaNode->AppendChild( "Classes" );
	XNode *pSingletonsNode = pLuaNode->AppendChild( "Singletons" );
	XNode *pEnumsNode = pLuaNode->AppendChild( "Enums" );
	XNode *pConstantsNode = pLuaNode->AppendChild( "Constants" );
	
	// Tricky. We have to get the classes from lua.
	vector<RString> vFunctions;
	map<RString, LClass> mClasses;
	map<RString, RString> mSingletons;
	map<RString, float> mConstants;
	map<RString, RString> mStringConstants;
	map<RString, vector<RString> > mEnums;
	
	lua_pushnil( L ); // initial key
	while( lua_next(L, LUA_GLOBALSINDEX) )
	{
		// key is at -2, value is at -1
		
		/* Tricky. FromStack() calls lua_tolstring() which changes the underlying cell
		 * if it is not a string. This confuses lua_next(). Copy the value first.
		 * http://www.lua.org/manual/5.1/manual.html#lua_tolstring */
		
		RString sKey;
		
		lua_pushvalue( L, -2 );
		LuaHelpers::Pop( L, sKey );
		
		switch( lua_type(L, -1) )
		{
		case LUA_TTABLE:
		{
			if( luaL_getmetafield(L, -1, "class") )
			{
				const char *name = lua_tostring( L, -1 );
				
				if( !name )
				{
					lua_pop( L, 1 ); // pop nil
					break;
				}
				LClass &c = mClasses[name];
				lua_pop( L, 1 ); // pop name
				
				// Get base class.
				if( luaL_getmetafield( L, -1, "base" ) )
				{
					name = lua_tostring( L, -1 );
					
					if( name )
						c.m_sBaseName = name;
					lua_pop( L, 1 ); // pop name
				}
				
				// Get methods.
				lua_pushnil( L ); // initial key
				while( lua_next(L, -2) )
				{
					// Again, be careful about reading the key as a string.
					lua_pop( L, 1 ); // pop value
					lua_pushvalue( L, -1 );
					RString sMethod;
					if( LuaHelpers::Pop(L, sMethod) )
						c.m_vMethods.push_back( sMethod );
				}
				sort( c.m_vMethods.begin(), c.m_vMethods.end() );
				break;
			}
		}
			// fall through
		case LUA_TUSERDATA: // table or userdata: class instance
		{
			if( !luaL_callmeta(L, -1, "__type") )
				break;
			RString sType;
			if( !LuaHelpers::Pop(L, sType) )
				break;
			if( sType == "Enum" )
			{
				vector<RString> &vEnum = mEnums[sKey];
				LuaHelpers::ReadArrayFromTable( vEnum, L );
			}
			else
			{
				mSingletons[sKey] = sType;
			}
			
			break;
		}
		case LUA_TNUMBER:
			LuaHelpers::FromStack( L, mConstants[sKey], -1 );
			break;
		case LUA_TSTRING:
			LuaHelpers::FromStack( L, mStringConstants[sKey], -1 );
			break;
		case LUA_TFUNCTION:
			vFunctions.push_back( sKey );
			break;
		}
		lua_pop( L, 1 ); // pop value
	}
			
	sort( vFunctions.begin(), vFunctions.end() );
	FOREACH_CONST( RString, vFunctions, func )
	{
		XNode *pFunctionNode = pGlobalsNode->AppendChild( "Function" );
		pFunctionNode->AppendAttr( "name", *func );
	}
	
	FOREACHM_CONST( RString, LClass, mClasses, c )
	{
		XNode *pClassNode = pClassesNode->AppendChild( "Class" );
		
		pClassNode->AppendAttr( "name", c->first );
		if( !c->second.m_sBaseName.empty() )
			pClassNode->AppendAttr( "base", c->second.m_sBaseName );
		FOREACH_CONST( RString, c->second.m_vMethods, m )
		{
			XNode *pMethodNode = pClassNode->AppendChild( "Function" );
			pMethodNode->AppendAttr( "name", *m );
		}
	}
	
	FOREACHM_CONST( RString, RString, mSingletons, s )
	{
		if( mClasses.find(s->first) != mClasses.end() )
			continue;
		XNode *pSingletonNode = pSingletonsNode->AppendChild( "Singleton" );
		pSingletonNode->AppendAttr( "name", s->first );
		pSingletonNode->AppendAttr( "class", s->second );
	}

	for( map<RString, vector<RString> >::const_iterator iter = mEnums.begin(); iter != mEnums.end(); ++iter )
	{
		XNode *pEnumNode = pEnumsNode->AppendChild( "Enum" );

		const vector<RString> &vEnum = iter->second;
		pEnumNode->AppendAttr( "name", iter->first );
		
		for( unsigned i = 0; i < vEnum.size(); ++i )
		{
			XNode *pEnumValueNode = pEnumNode->AppendChild( "EnumValue" );
			pEnumValueNode->AppendAttr( "name", ssprintf("'%s'", vEnum[i].c_str()) );
			pEnumValueNode->AppendAttr( "value", i );
		}
	}

	FOREACHM_CONST( RString, float, mConstants, c )
	{
		XNode *pConstantNode = pConstantsNode->AppendChild( "Constant" );
		
		pConstantNode->AppendAttr( "name", c->first );
		if( c->second == truncf(c->second) )
			pConstantNode->AppendAttr( "value", int(c->second) );
		else
			pConstantNode->AppendAttr( "value", c->second );
	}
	FOREACHM_CONST( RString, RString, mStringConstants, s )
	{
		XNode *pConstantNode = pConstantsNode->AppendChild( "Constant" );
		pConstantNode->AppendAttr( "name", s->first );
		pConstantNode->AppendAttr( "value", ssprintf("'%s'", s->second.c_str()) );
	}
		
	LUA->Release( L );

	return pLuaNode;
}
	

void LuaHelpers::PrepareExpression( RString &sInOut )
{
	// HACK: Many metrics have "// foo" and "# foo" comments that Lua fails to parse.
	// Replace them with Lua-style comments.
	// XXX: "Foo=Func('#AABBCC')" and "Text=Adjust('// subtitle') aren't comments.
	sInOut.Replace( "//", "--" );
	sInOut.Replace( "#", "--" );
	
	// Remove unary +, eg. "+50"; Lua doesn't support that.
	if( sInOut.size() >= 1 && sInOut[0] == '+' )
		sInOut.erase( 0, 1 );
}

bool LuaHelpers::RunScriptFile( const RString &sFile )
{
	RString sScript;
	if( !GetFileContents(sFile, sScript) )
		return false;

	Lua *L = LUA->Get();

	RString sError;
	if( !LuaHelpers::RunScript( L, sScript, "@" + sFile, sError, 0 ) )
	{
		LUA->Release( L );
		sError = ssprintf( "Lua runtime error: %s", sError.c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}
	LUA->Release( L );

	return true;
}


bool LuaHelpers::RunScript( Lua *L, const RString &sScript, const RString &sName, RString &sError, int iReturnValues )
{
	// load string
	{
		int ret = luaL_loadbuffer( L, sScript.data(), sScript.size(), sName );
		if( ret )
		{
			LuaHelpers::Pop( L, sError );
			for( int i = 0; i < iReturnValues; ++i )
				lua_pushnil( L );
			return false;
		}
	}

	// evaluate
	{
		int ret = lua_pcall( L, 0, iReturnValues, 0 );
		if( ret )
		{
			LuaHelpers::Pop( L, sError );
			for( int i = 0; i < iReturnValues; ++i )
				lua_pushnil( L );
			return false;
		}
	}

	return true;
}

bool LuaHelpers::RunExpression( Lua *L, const RString &sExpression, const RString &sName )
{
	RString sError;
	if( !LuaHelpers::RunScript(L, "return " + sExpression, sName.empty()? RString("in"):sName, sError, 1) )
	{
		sError = ssprintf( "Lua runtime error parsing \"%s\": %s", sName.size()? sName.c_str():sExpression.c_str(), sError.c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}
	return true;
}

bool LuaHelpers::RunExpressionB( const RString &str )
{
	Lua *L = LUA->Get();

	RunExpression( L, str );

	bool result;
	LuaHelpers::Pop( L, result );

	LUA->Release( L );
	return result;
}

float LuaHelpers::RunExpressionF( const RString &str )
{
	Lua *L = LUA->Get();

	RunExpression( L, str );

	float result;
	LuaHelpers::Pop( L, result );

	LUA->Release( L );
	return result;
}

void LuaHelpers::RunExpressionS( const RString &str, RString &sOut )
{
	Lua *L = LUA->Get();

	RunExpression( L, str );

	LuaHelpers::Pop( L, sOut );

	LUA->Release( L );
}

bool LuaHelpers::RunAtExpressionS( RString &sStr )
{
	if( sStr.size() == 0 || sStr[0] != '@' )
		return false;

	/* Erase "@". */
	sStr.erase( 0, 1 );

	RString sOut;
	LuaHelpers::RunExpressionS( sStr, sOut );
	sStr = sOut;
	return true;
}

void LuaHelpers::ParseCommandList( Lua *L, const RString &sCommands, const RString &sName )
{
	RString sLuaFunction;
	if( sCommands.size() > 0 && sCommands[0] == '\033' )
	{
		/* This is a compiled Lua chunk.  Just pass it on directly. */
		sLuaFunction = sCommands;
	}
	else if( sCommands.size() > 0 && sCommands[0] == '%' )
	{
		sLuaFunction = "return ";
		sLuaFunction.append( sCommands.begin()+1, sCommands.end() );
	}
	else
	{
		Commands cmds;
		ParseCommands( sCommands, cmds );
		
		//
		// Convert cmds to a Lua function
		//
		ostringstream s;
		
		s << "return function(self,parent)\n";

		FOREACH_CONST( Command, cmds.v, c )
		{
			const Command& cmd = (*c);
			RString sName = cmd.GetName();
			TrimLeft( sName );
			TrimRight( sName );
			s << "\tself:" << sName << "(";

			bool bFirstParamIsString =
				sName == "effectclock" ||
				sName == "playcommand" ||
				sName == "queuecommand" ||
				sName == "queuemessage";

			for( unsigned i=1; i<cmd.m_vsArgs.size(); i++ )
			{
				RString sArg = cmd.m_vsArgs[i];

				// "+200" -> "200"
				if( sArg[0] == '+' )
					sArg.erase( sArg.begin() );

				if( i==1 && bFirstParamIsString ) // string literal
				{
					sArg.Replace( "'", "\\'" );	// escape quote
					s << "'" << sArg << "'";
				}
				else if( sArg[0] == '#' )	// HTML color
				{
					RageColor c;	// in case FromString fails
					c.FromString( sArg );
					// c is still valid if FromString fails
					s << c.r << "," << c.g << "," << c.b << "," << c.a;
				}
				else
				{
					s << sArg;
				}

				if( i != cmd.m_vsArgs.size()-1 )
					s << ",";
			}
			s << ")\n";
		}

		s << "end\n";

		sLuaFunction = s.str();
	}

	RString sError;
	if( !LuaHelpers::RunScript( L, sLuaFunction, sName, sError, 1 ) )
		LOG->Warn( "Compiling \"%s\": %s", sLuaFunction.c_str(), sError.c_str() );

	/* The function is now on the stack. */
}

/* Like luaL_typerror, but without the special case for argument 1 being "self"
 * in method calls, so we give a correct error message after we remove self. */
int LuaHelpers::TypeError( Lua *L, int iArgNo, const char *szName )
{
	RString sType;
	luaL_pushtype( L, iArgNo );
	LuaHelpers::Pop( L, sType );

	lua_Debug debug;
	if( !lua_getstack( L, 0, &debug ) )
	{
		return luaL_error( L, "invalid type (%s expected, got %s)",
			szName, sType.c_str() );
	}
	else
	{
		lua_getinfo( L, "n", &debug );
		return luaL_error( L, "bad argument #%d to \"%s\" (%s expected, got %s)",
			iArgNo, debug.name? debug.name:"(unknown)", szName, sType.c_str() );
	}
}

namespace
{
	int lua_pushvalues( lua_State *L )
	{
		int iArgs = lua_tointeger( L, lua_upvalueindex(1) );
		for( int i = 0; i < iArgs; ++i )
			lua_pushvalue( L, lua_upvalueindex(i+2) );
		return iArgs;
	}
}

void LuaHelpers::PushValueFunc( lua_State *L, int iArgs )
{
	int iTop = lua_gettop( L ) - iArgs + 1;
	lua_pushinteger( L, iArgs );
	lua_insert( L, iTop );
	lua_pushcclosure( L, lua_pushvalues, iArgs+1 );
}

LuaFunctionList::LuaFunctionList( RString name_, lua_CFunction func_ )
{
	name = name_;
	func = func_;
	next = g_LuaFunctions;
	g_LuaFunctions = this;
}


static bool Trace( const RString &sString )
{
	LOG->Trace( "%s", sString.c_str() );
	return true;
}
LuaFunction( Trace, Trace(SArg(1)) );

static bool Warn( const RString &sString )
{
	LOG->Warn( "%s", sString.c_str() );
	return true;
}
LuaFunction( Warn, Warn(SArg(1)) );

#include "ProductInfo.h"
LuaFunction( ProductVersion, (RString) PRODUCT_VER );
LuaFunction( ProductID, (RString) PRODUCT_ID );

static float scale( float x, float l1, float h1, float l2, float h2 )
{
	return SCALE( x, l1, h1, l2, h2 );
}
LuaFunction( scale, scale(FArg(1), FArg(2), FArg(3), FArg(4), FArg(5)) );

LuaFunction( clamp, clamp(FArg(1), FArg(2), FArg(3)) );

/*
 * (c) 2004-2006 Glenn Maynard, Steve Checkoway
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
