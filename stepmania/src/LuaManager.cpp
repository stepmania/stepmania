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
#include "RageTypes.h"

#include <sstream>
#include <csetjmp>
#include <cassert>
#include <map>

LuaManager *LUA = NULL;
struct Impl
{
	Impl(): g_pLock("Lua") {}
	vector<lua_State *> g_FreeStateList;
	map<lua_State *, bool> g_ActiveStates;

	RageMutex g_pLock;
};
static Impl *pImpl = NULL;

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


static int GetLuaStack( lua_State *L )
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

	LuaHelpers::Push( L, sErr );
	return 1;
}


static int LuaPanic( lua_State *L )
{
	GetLuaStack( L );

	RString sErr;
	LuaHelpers::Pop( L, sErr );

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
	pImpl = new Impl;
	LUA = this;	// so that LUA is available when we call the Register functions

	lua_State *L = lua_open();
	ASSERT( L );

	lua_atpanic( L, LuaPanic );
	m_pLuaMain = L;

	lua_pushcfunction( L, luaopen_base ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_math ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_string ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_table ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_debug ); lua_call( L, 0, 0 );

	/* Store the thread pool in a table on the stack, in the main thread. */
#define THREAD_POOL 1
	lua_newtable( L );

	RegisterTypes();
}

LuaManager::~LuaManager()
{
	lua_close( m_pLuaMain );
	SAFE_DELETE( pImpl );
}

Lua *LuaManager::Get()
{
	bool bLocked = false;
	if( !pImpl->g_pLock.IsLockedByThisThread() )
	{
		pImpl->g_pLock.Lock();
		bLocked = true;
	}

	ASSERT( lua_gettop(m_pLuaMain) == 1 );

	lua_State *pRet;
	if( pImpl->g_FreeStateList.empty() )
	{
		pRet = lua_newthread( m_pLuaMain );

		/* Store the new thread in THREAD_POOL, so it isn't collected. */
		int iLast = lua_objlen( m_pLuaMain, THREAD_POOL );
		lua_rawseti( m_pLuaMain, THREAD_POOL, iLast+1 );
	}
	else
	{
		pRet = pImpl->g_FreeStateList.back();
		pImpl->g_FreeStateList.pop_back();
	}

	pImpl->g_ActiveStates[pRet] = bLocked;
	return pRet;
}

void LuaManager::Release( Lua *&p )
{
	pImpl->g_FreeStateList.push_back( p );

	ASSERT( lua_gettop(p) == 0 );
	ASSERT( pImpl->g_ActiveStates.find(p) != pImpl->g_ActiveStates.end() );
	bool bDoUnlock = pImpl->g_ActiveStates[p];
	pImpl->g_ActiveStates.erase( p );

	if( bDoUnlock )
		pImpl->g_pLock.Unlock();
	p = NULL;
}

/*
 * Low-level access to Lua is always serialized through pImpl->g_pLock; we never run the Lua
 * core simultaneously from multiple threads.  However, when a thread has an acquired
 * lua_State, it can release Lua for use by other threads.  This allows Lua bindings
 * to process long-running actions, without blocking all other threads from using Lua
 * until it finishes.
 *
 * Lua *L = LUA->Get();				// acquires L and locks Lua
 * lua_newtable(L);				// does something with Lua
 * LUA->YieldLua();				// unlocks Lua for lengthy operation; L is still owned, but can't be used
 * RString s = ReadFile("/filename.txt");	// time-consuming operation; other threads may use Lua in the meantime
 * LUA->UnyieldLua();				// relock Lua
 * lua_pushstring( L, s );			// finish working with it
 * LUA->Release( L );				// release L and unlock Lua
 *
 * YieldLua() must not be called when already yielded, or when a lua_State has not been
 * acquired (you have nothing to yield), and always unyield before releasing the
 * state.  Recursive handling is OK:
 *
 * L1 = LUA->Get();
 * LUA->YieldLua();				// yields
 *   L2 = LUA->Get();				// unyields
 *   LUA->Release(L2);				// re-yields
 * LUA->UnyieldLua();
 * LUA->Release(L1);
 */
void LuaManager::YieldLua()
{
	ASSERT( pImpl->g_pLock.IsLockedByThisThread() );

	pImpl->g_pLock.Unlock();
}

void LuaManager::UnyieldLua()
{
	pImpl->g_pLock.Lock();
}

void LuaManager::RegisterTypes()
{
	Lua *L = LUA->Get();

	if( g_vRegisterActorTypes )
	{
		for( unsigned i=0; i<g_vRegisterActorTypes->size(); i++ )
		{
			RegisterWithLuaFn fn = (*g_vRegisterActorTypes)[i];
			fn( L );
		}
	}

	LUA->Release( L );
}

LuaThreadVariable::LuaThreadVariable( const RString &sName, const RString &sValue )
{
	m_sName = sName;
	m_pOldValue = new LuaReference;

	Lua *L = LUA->Get();
	LuaHelpers::Push( L, sValue );
	SetFromStack( L );
	LUA->Release( L );
}

LuaThreadVariable::LuaThreadVariable( const RString &sName, const LuaReference &Value )
{
	m_sName = sName;
	m_pOldValue = new LuaReference;

	Lua *L = LUA->Get();
	Value.PushSelf( L );
	SetFromStack( L );
	LUA->Release( L );
}

RString LuaThreadVariable::GetCurrentThreadIDString()
{
	uint64_t iID = RageThread::GetCurrentThreadID();
	return ssprintf( "%08x%08x", uint32_t(iID >> 32), uint32_t(iID) );
}

bool LuaThreadVariable::PushThreadTable( lua_State *L, bool bCreate )
{
	lua_getfield( L, LUA_REGISTRYINDEX, "LuaThreadVariableTable" );
	if( lua_isnil(L, -1) )
	{
		lua_pop( L, 1 );
		if( !bCreate )
			return false;
		lua_newtable( L );

		lua_pushvalue( L, -1 );
		lua_setfield( L, LUA_REGISTRYINDEX, "LuaThreadVariableTable" );
	}

	RString sThreadIDString = GetCurrentThreadIDString();
	LuaHelpers::Push( L, sThreadIDString );
	lua_gettable( L, -2 );
	if( lua_isnil(L, -1) )
	{
		lua_pop( L, 1 );
		if( !bCreate )
		{
			lua_pop( L, 1 );
			return false;
		}
		lua_newtable( L );

		lua_pushinteger( L, 0 );
		lua_rawseti( L, -2, 0 );

		LuaHelpers::Push( L, sThreadIDString );
		lua_pushvalue( L, -2 );
		lua_settable( L, -4 );
	}

	lua_remove( L, -2 );
	return true;
}

void LuaThreadVariable::GetThreadVariable( lua_State *L )
{
	if( !PushThreadTable(L, false) )
	{
		lua_pop( L, 1 );
		lua_pushnil( L );
		return;
	}

	lua_pushvalue( L, -2 );
	lua_gettable( L, -2 );
	lua_remove( L, -2 );
	lua_remove( L, -2 );
}

int LuaThreadVariable::AdjustCount( lua_State *L, int iAdd )
{
	ASSERT( lua_istable(L, -1) );

	lua_rawgeti( L, -1, 0 );
	ASSERT( lua_isnumber(L, -1) );

	int iCount = lua_tointeger( L, -1 );
	lua_pop( L, 1 );

	iCount += iAdd;
	lua_pushinteger( L, iCount );
	lua_rawseti( L, -2, 0 );

	return iCount;
}

void LuaThreadVariable::SetFromStack( lua_State *L )
{
	ASSERT( !m_pOldValue->IsSet() ); // don't call twice

	PushThreadTable( L, true );

	lua_getfield( L, -1, m_sName );
	m_pOldValue->SetFromStack( L );

	lua_pushvalue( L, -2 );
	lua_setfield( L, -2, m_sName );

	AdjustCount( L, +1 );

	lua_pop( L, 2 );
}

LuaThreadVariable::~LuaThreadVariable()
{
	Lua *L = LUA->Get();

	PushThreadTable( L, true );
	m_pOldValue->PushSelf( L );
	lua_setfield( L, -2, m_sName );

	if( AdjustCount( L, -1 ) == 0 )
	{
		// if empty, delete the table
		lua_getfield( L, LUA_REGISTRYINDEX, "LuaThreadVariableTable" );
		ASSERT( lua_istable(L, -1) );

		LuaHelpers::Push( L, GetCurrentThreadIDString() );
		lua_pushnil( L );
		lua_settable( L, -3 );
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );

	LUA->Release( L );

	delete m_pOldValue;
}

namespace
{
	struct LClass
	{
		RString m_sBaseName;
		vector<RString> m_vMethods;
	};
}

XNode *LuaHelpers::GetLuaInformation()
{
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
	
	Lua *L = LUA->Get();
	FOREACH_LUATABLE( L, LUA_GLOBALSINDEX )
	{
		RString sKey;
		LuaHelpers::Pop( L, sKey );
		
		switch( lua_type(L, -1) )
		{
		case LUA_TTABLE:
		{
			if( luaL_getmetafield(L, -1, "class") )
			{
				const char *name = lua_tostring( L, -1 );
				
				if( !name )
					break;
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
				FOREACH_LUATABLE( L, -1 )
				{
					RString sMethod;
					if( LuaHelpers::FromStack(L, sMethod, -1) )
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
				LuaHelpers::ReadArrayFromTable( mEnums[sKey], L );
			else
				mSingletons[sKey] = sType;
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
	}
	LUA->Release( L );

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

	return pLuaNode;
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


bool LuaHelpers::RunScript( Lua *L, const RString &sScript, const RString &sName, RString &sError, int iArgs, int iReturnValues )
{
	lua_pushcfunction( L, GetLuaStack );
	int iErrFunc = lua_gettop( L );

	// load string
	{
		int ret = luaL_loadbuffer( L, sScript.data(), sScript.size(), sName );
		if( ret )
		{
			LuaHelpers::Pop( L, sError );
			lua_pop( L, iArgs );
			lua_remove( L, iErrFunc );
			for( int i = 0; i < iReturnValues; ++i )
				lua_pushnil( L );
			return false;
		}
	}

	// move the function above the params
	lua_insert( L, lua_gettop(L) - iArgs );

	// evaluate
	{
		int ret = lua_pcall( L, iArgs, iReturnValues, iErrFunc );
		if( ret )
		{
			LuaHelpers::Pop( L, sError );
			lua_remove( L, iErrFunc );
			for( int i = 0; i < iReturnValues; ++i )
				lua_pushnil( L );
			return false;
		}
	}

	lua_remove( L, iErrFunc );
	return true;
}

bool LuaHelpers::RunExpression( Lua *L, const RString &sExpression, const RString &sName )
{
	RString sError;
	if( !LuaHelpers::RunScript(L, "return " + sExpression, sName.empty()? RString("in"):sName, sError, 0, 1) )
	{
		sError = ssprintf( "Lua runtime error parsing \"%s\": %s", sName.size()? sName.c_str():sExpression.c_str(), sError.c_str() );
		Dialog::OK( sError, "LUA_ERROR" );
		return false;
	}
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
		
		s << "return function(self)\n";

		FOREACH_CONST( Command, cmds.v, c )
		{
			const Command& cmd = (*c);
			RString sName = cmd.GetName();
			TrimLeft( sName );
			TrimRight( sName );
			s << "\tself:" << sName << "(";

			for( unsigned i=1; i<cmd.m_vsArgs.size(); i++ )
			{
				RString sArg = cmd.m_vsArgs[i];

				// "+200" -> "200"
				if( sArg[0] == '+' )
					sArg.erase( sArg.begin() );

				if( sArg[0] == '#' )	// HTML color
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
	if( !LuaHelpers::RunScript(L, sLuaFunction, sName, sError, 0, 1) )
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

void LuaHelpers::DeepCopy( lua_State *L )
{
	luaL_checktype( L, -2, LUA_TTABLE );
	luaL_checktype( L, -1, LUA_TTABLE );

	/* Call DeepCopy(t, u), where t is our referenced object and u is the new table. */
	lua_getglobal( L, "DeepCopy" );

	ASSERT_M( !lua_isnil(L, -1), "DeepCopy() missing" );
	ASSERT_M( lua_isfunction(L, -1), "DeepCopy() not a function" );
	lua_insert( L, lua_gettop(L)-2 );

	lua_call( L, 2, 0 );
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

#include "LuaBinding.h"
namespace
{
	static int ReadFile( lua_State *L )
	{
		RString sPath = SArg(1);

		/* Release Lua while we call GetFileContents, so we don't it while we read from he disk. */
		LUA->YieldLua();

		RString sFileContents;
		bool bRet = GetFileContents( sPath, sFileContents );

		LUA->UnyieldLua();
		if( !bRet )
		{
			lua_pushnil( L );
			lua_pushstring( L, "error" ); // XXX
			return 2;
		}
		else
		{
			LuaHelpers::Push( L, sFileContents );
			return 1;
		}
	}

	static int GetThreadVariable( lua_State *L )
	{
		luaL_checkstring( L, 1 );
		lua_pushvalue( L, 1 );
		LuaThreadVariable::GetThreadVariable( L );
		return 1;
	}
	
	const luaL_Reg luaTable[] =
	{
		LIST_METHOD( ReadFile ),
		LIST_METHOD( GetThreadVariable ),
		{ NULL, NULL }
	};
}

LUA_REGISTER_NAMESPACE( lua )

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
