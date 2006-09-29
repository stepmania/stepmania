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

#include <csetjmp>
#include <cassert>

LuaManager *LUA = NULL;
static LuaFunctionList *g_LuaFunctions = NULL;

#if defined(_MSC_VER) || defined (_XBOX)
	/* "interaction between '_setjmp' and C++ object destruction is non-portable"
	 * We don't care; we'll throw a fatal exception immediately anyway. */
	#pragma warning (disable : 4611)
#endif

struct ChunkReaderString
{
	ChunkReaderString( const RString &sBuf ): m_sBuf(sBuf) { m_bDone = false; }
	static const char *Reader( lua_State *L, void *ptr, size_t *size );

	const RString m_sBuf;
	bool m_bDone;
};

const char *ChunkReaderString::Reader( lua_State *L, void *pPtr, size_t *pSize )
{
	ChunkReaderString *pData = (ChunkReaderString *) pPtr;
	if( pData->m_bDone )
		return NULL;

	pData->m_bDone = true;

	*pSize = pData->m_sBuf.size();
	const char *pRet = pData->m_sBuf.data();
	
	return pRet;
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


void LuaHelpers::Push( lua_State *L, const bool &Object ) { lua_pushboolean( L, Object ); }
void LuaHelpers::Push( lua_State *L, const float &Object ) { lua_pushnumber( L, Object ); }
void LuaHelpers::Push( lua_State *L, const int &Object ) { lua_pushinteger( L, Object ); }
void LuaHelpers::Push( lua_State *L, const RString &Object ) { lua_pushlstring( L, Object.data(), Object.size() ); }

bool LuaHelpers::FromStack( Lua *L, bool &Object, int iOffset ) { Object = !!lua_toboolean( L, iOffset ); return true; }
bool LuaHelpers::FromStack( Lua *L, float &Object, int iOffset ) { Object = (float)lua_tonumber( L, iOffset ); return true; }
bool LuaHelpers::FromStack( Lua *L, int &Object, int iOffset ) { Object = lua_tointeger( L, iOffset ); return true; }
bool LuaHelpers::FromStack( Lua *L, RString &Object, int iOffset )
{
	size_t iLen;
	const char *pStr = lua_tolstring( L, iOffset, &iLen );
	if( pStr != NULL )
		Object.assign( pStr, iLen );

	return pStr != NULL;
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
	XNode *pLuaNode = new XNode;
	pLuaNode->m_sName = "Lua";

	XNode *pGlobalsNode = pLuaNode->AppendChild( "GlobalFunctions" );
	XNode *pClassesNode = pLuaNode->AppendChild( "Classes" );
	XNode *pSingletonsNode = pLuaNode->AppendChild( "Singletons" );
	XNode *pEnumsNode = pLuaNode->AppendChild( "Enums" );
	XNode *pConstantsNode = pLuaNode->AppendChild( "Constants" );
	
	vector<RString> vFunctions;
	for( const LuaFunctionList *p = g_LuaFunctions; p; p = p->next )
		vFunctions.push_back( p->name );
	
	sort( vFunctions.begin(), vFunctions.end() );
	
	FOREACH_CONST( RString, vFunctions, func )
	{
		XNode *pFunctionNode = pGlobalsNode->AppendChild( "Function" );
		pFunctionNode->AppendAttr( "name", *func );
	}
	
	// Tricky. We have to get the classes from lua.
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
			// Check for the metatable.
			if( !lua_getmetatable(L, -1) )
			{
				/* If the key ends in "Index", check for the non "Index" version.
				 * If both exist, then we have found an enum. */
				if( !EndsWith(sKey, "Index") || sKey.size() <= 5 )
					break;
				sKey = sKey.substr( 0, sKey.size() - 5 );
				LuaHelpers::Push( L, sKey );
				lua_rawget( L, LUA_GLOBALSINDEX );
				if( lua_istable(L, -1) )
				{
					vector<RString> &vEnum = mEnums[sKey];
					LuaHelpers::ReadArrayFromTable( vEnum, L );
				}
				lua_pop( L, 2 ); // pop table and key
				break;
			}

			lua_pushliteral( L, "type" );
			lua_rawget( L, -2 );
			
			const char *name = lua_tostring( L, -1 );
			
			if( !name )
			{
				lua_pop( L, 2 ); // pop nil and metatable
				break;
			}
			LClass &c = mClasses[name];
			lua_pop( L, 1 ); // pop name
			
			// Get base class.
			lua_pushliteral( L, "__index" );
			lua_rawget( L, -2 );
			if( lua_istable(L, -1) && lua_getmetatable(L, -1) )
			{
				lua_pushliteral( L, "type" );
				lua_gettable( L, -2 );
				name = lua_tostring( L, -1 );
				
				if( name )
					c.m_sBaseName = name;
				lua_pop( L, 2 ); // pop name and metatable
			}
			lua_pop( L, 2 ); // pop (table or other) and metatable
			
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
		case LUA_TUSERDATA:
		{
			/* Tricky. The singletons have a metatable but it doesn't have a type key-value pair.
			 * Instead, we need to get the methods using the __index key and then get the
			 * metatable of the methods table. */
			if( !lua_getmetatable(L, -1) )
				break;
			lua_pushliteral( L, "__index" );
			lua_rawget( L, -2 );
			if( !lua_istable(L, -1) || !lua_getmetatable(L, -1) )
			{
				lua_pop( L, 2 ); // pop method table and metatable
				break;
			}
			lua_pushliteral( L, "type" );
			lua_rawget( L, -2 );
			
			/* The stack now looks like:
			 * -1: type
			 * -2: method metatable
			 * -3: method table
			 * -4: metatable
			 * -5: user data
			 * -6: key (object's name) */
			const char *type = lua_tostring( L, -1 );
			
			if( type )
				mSingletons[sKey] = type;
			lua_pop( L, 4 ); // pop type, method metatable, method table, and metatable
			break;
		}
		case LUA_TNUMBER:
		{
			float fNum;
			LuaHelpers::FromStack( L, fNum, -1 );
			mConstants[sKey] = fNum;
			break;
		}
		case LUA_TSTRING:
			RString sValue;
			LuaHelpers::FromStack( L, sValue, -1 );
			mStringConstants[sKey] = sValue;
			break;
		}
		lua_pop( L, 1 ); // pop value
	}
			
	FOREACHM_CONST( RString, LClass, mClasses, c )
	{
		XNode *pClassNode = pClassesNode->AppendChild( "Class" );
		
		pClassNode->AppendAttr( "name", c->first );
		if( !c->second.m_sBaseName.empty() )
			pClassNode->AppendAttr( "base", c->second.m_sBaseName );
		FOREACH_CONST( RString, c->second.m_vMethods, m )
		{
			XNode *pMethodNode = new XNode;
			
			pMethodNode->m_sName = "Function";
			pMethodNode->AppendAttr( "name", *m );
			pClassNode->AppendChild( pMethodNode );
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
			pEnumValueNode->AppendAttr( "name", vEnum[i] );
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
		pConstantNode->AppendAttr( "value", s->second );
	}
		
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
	RageFile f;
	if( !f.Open( sFile ) )
	{
		RString sError = ssprintf( "Couldn't open Lua script \"%s\": %s", sFile.c_str(), f.GetError().c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}

	RString sScript;
	if( f.Read( sScript ) == -1 )
	{
		RString sError = ssprintf( "Error reading Lua script \"%s\": %s", sFile.c_str(), f.GetError().c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}
	f.Close();

	Lua *L = LUA->Get();

	RString sError;
	if( !LuaHelpers::RunScript( L, sScript, sFile, sError, 0 ) )
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
		ChunkReaderString data( sScript );
		int ret = lua_load( L, ChunkReaderString::Reader, &data, sName );

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

int LuaHelpers::RunExpressionI( const RString &str )
{
	return (int) LuaHelpers::RunExpressionF(str);
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

/* Like luaL_typerror, but without the special case for argument 1 being "self"
 * in method calls, so we give a correct error message after we remove self. */
static RString GetLuaBindingType( Lua *L, int iArgNo )
{
	if( lua_isnil(L, iArgNo) )
		return "nil";

	int iTop = lua_gettop( L );
	if( !lua_getmetatable(L, iArgNo) )
	{
		lua_settop( L, iTop );
		return ssprintf( "non-bound %s", lua_typename(L, lua_type(L, iArgNo)) );
	}

	int iMetatable = lua_gettop( L );
	lua_pushstring( L, "type" );
	lua_rawget( L, iMetatable );
	RString sActualType;
	LuaHelpers::FromStack( L, sActualType, -1 );

	lua_settop( L, iTop );
	return sActualType;
}

/* Like luaL_typerror, but without the special case for argument 1 being "self"
 * in method calls, so we give a correct error message after we remove self. */
int LuaHelpers::TypeError( Lua *L, int iArgNo, const char *szName )
{
	RString sType = GetLuaBindingType( L, iArgNo );

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
