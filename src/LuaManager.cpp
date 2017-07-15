#include "global.h"
#include "LuaManager.h"
#include "RageMath.hpp"
#include "LuaReference.h"
#include "RageUtil.h"
#include "RageUtil.hpp"
#include "RageLog.h"
#include "RageFile.h"
#include "RageThreads.h"
#include "arch/Dialog/Dialog.h"
#include "XmlFile.h"
#include "Command.h"
#include "RageLog.h"
#include "RageTypes.h"
#include "MessageManager.h"
#include "ver.h"

#include <sstream> // conversion for lua functions.
#include <csetjmp>
#include <cassert>
#include <map>
#include <unordered_set>
#include <array>

using std::vector;

LuaManager *LUA = nullptr;
struct Impl
{
	Impl(): g_pLock("Lua") {}
	vector<lua_State *> g_FreeStateList;
	std::map<lua_State *, bool> g_ActiveStates;

	RageMutex g_pLock;
};
static Impl *pImpl = nullptr;

#if defined(_MSC_VER)
	/* "interaction between '_setjmp' and C++ object destruction is non-portable"
	 * We don't care; we'll throw a fatal exception immediately anyway. */
	#pragma warning (disable : 4611)
#endif

/** @brief Utilities for working with Lua. */
namespace LuaHelpers
{
	template<> void Push<bool>(lua_State* L, bool const& object)
	{
		lua_pushboolean(L, object);
	}
	template<> void Push<float>(lua_State* L, float const& object)
	{
		lua_pushnumber(L, object);
	}
	template<> void Push<double>(lua_State* L, double const& object)
	{
		lua_pushnumber(L, object);
	}
	template<> void Push<int>(lua_State* L, int const& object)
	{
		lua_pushinteger(L, object);
	}
	template<> void Push<unsigned int>(lua_State* L, unsigned int const& object)
	{
		lua_pushnumber(L, static_cast<double>(object));
	}
	template<> void Push<unsigned long>(lua_State* L, unsigned long const& object)
	{
		lua_pushnumber(L, static_cast<double>(object));
	}
	template<> void Push<std::string>(lua_State* L, std::string const& object)
	{
		lua_pushlstring(L, object.data(), object.size());
	}

	template<> bool FromStack<bool>(Lua* L, bool& object, int offset)
	{
		object = lua_toboolean(L, offset) != 0;
		return true;
	}
	template<> bool FromStack<float>(Lua* L, float& object, int offset)
	{
		object = static_cast<float>(lua_tonumber(L, offset));
		return true;
	}
	template<> bool FromStack<double>(Lua* L, double& object, int offset)
	{
		object = static_cast<double>(lua_tonumber(L, offset));
		return true;
	}
	template<> bool FromStack<int>(Lua* L, int& object, int offset)
	{
		object = lua_tointeger(L, offset);
		return true;
	}
	template<> bool FromStack<unsigned int>(Lua* L, unsigned int& object, int offset)
	{
		object = lua_tointeger(L, offset);
		return true;
	}
	template<> bool FromStack<unsigned long>(Lua* L, unsigned long& object, int offset)
	{
		object = lua_tointeger(L, offset);
		return true;
	}
	template<> bool FromStack<std::string>(Lua* L, std::string &object, int offset)
	{
		size_t len;
		char const *cstr = lua_tolstring(L, offset, &len);
		if (cstr != nullptr)
		{
			object.assign(cstr);
			return true;
		}
		object.clear();
		return false;
	}
	bool FromStack(Lua* L, char const *object, int offset)
	{
		std::string clean{object};
		return LuaHelpers::FromStack(L, clean, offset);
	}

	bool InReportScriptError= false;
}

void LuaManager::SetGlobal( const std::string &sName, int val )
{
	Lua *L = Get();
	LuaHelpers::Push( L, val );
	lua_setglobal( L, sName.c_str() );
	Release( L );
}

void LuaManager::SetGlobal( const std::string &sName, const std::string &val )
{
	Lua *L = Get();
	LuaHelpers::Push( L, val );
	lua_setglobal( L, sName.c_str() );
	Release( L );
}

void LuaManager::UnsetGlobal( const std::string &sName )
{
	Lua *L = Get();
	lua_pushnil( L );
	lua_setglobal( L, sName.c_str() );
	Release( L );
}

bool LuaHelpers::string_can_be_lua_identifier(lua_State* L, std::string const& str)
{
	int original_top= lua_gettop(L);
	lua_getfield(L, LUA_GLOBALSINDEX, "string");
	lua_getfield(L, -1, "match");
	int ret_start_index= lua_gettop(L);
	lua_pushstring(L, str.c_str());
	lua_pushstring(L, "^[a-zA-Z_][a-zA-Z_0-9]*$");
	lua_call(L, 2, LUA_MULTRET);
	if(lua_isnil(L, ret_start_index))
	{
		lua_settop(L, original_top);
		return false;
	}
	lua_settop(L, original_top);
	return true;
}

void LuaHelpers::push_lua_escaped_string(lua_State* L, std::string const& str)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "string");
	int str_tab_ind= lua_gettop(L);
	lua_getfield(L, -1, "format");
	lua_pushstring(L, "%q");
	lua_pushstring(L, str.c_str());
	lua_call(L, 2, 1);
	lua_remove(L, str_tab_ind);
}

static void write_lua_value_to_file(lua_State* L, int value_index,
	RageFile* file, std::string const& indent, std::unordered_set<void const*>& visited_tables, bool write_equals);
static void write_lua_table_to_file(lua_State* L, int table_index,
	RageFile* file, std::string const& indent, std::unordered_set<void const*>& visited_tables);

static void write_lua_value_to_file(lua_State* L, int value_index,
	RageFile* file, std::string const& indent, std::unordered_set<void const*>& visited_tables, bool write_equals)
{
	if(write_equals)
	{
		file->Write("= ");
	}
	switch(lua_type(L, value_index))
	{
		case LUA_TTABLE:
			write_lua_table_to_file(L, value_index, file, indent, visited_tables);
			break;
		case LUA_TSTRING:
			{
				lua_getfield(L, LUA_GLOBALSINDEX, "string");
				lua_getfield(L, -1, "format");
				lua_pushstring(L, "%q");
				lua_pushvalue(L, value_index);
				lua_call(L, 2, 1);
				file->Write(lua_tostring(L, -1));
				lua_pop(L, 2);
			}
			break;
		case LUA_TNUMBER:
			{
				double as_double= lua_tonumber(L, value_index);
				int as_int= lua_tointeger(L, value_index);
				double int_turned_double= static_cast<int>(as_int);
				std::string val_str;
				if(fabs(as_double - int_turned_double) < .001)
				{
					val_str= fmt::sprintf("%i", as_int);
				}
				else
				{
					val_str= fmt::sprintf("%.6f", as_double);
				}
				file->Write(val_str);
			}
			break;
		case LUA_TBOOLEAN:
			if(lua_toboolean(L, value_index))
			{
				file->Write("true");
			}
			else
			{
				file->Write("false");
			}
			break;
		default:
			break;
	}
	file->Write(",");
}

static void write_lua_table_to_file(lua_State* L, int table_index,
	RageFile* file, std::string const& indent, std::unordered_set<void const*>& visited_tables)
{
	visited_tables.insert(lua_topointer(L, table_index));
	// Fields shall be saved strictly ordered by key type and value.
  // String fields, double fields, int fields, bool fields.
	// Other key types shall be considered nonsense and ignored. -Kyz
	std::vector<std::string> string_fields;
	std::vector<double> double_fields;
	std::vector<int> int_fields;
	std::vector<bool> bool_fields;
	lua_pushnil(L);
	while(lua_next(L, table_index) != 0)
	{
		// Filter out anything that is not a table, string, number, or boolean
		// with a switch.  Accepted types use fallthrough, default uses continue.
		switch(lua_type(L, -1))
		{
			case LUA_TTABLE:
				{
					void const* sub_table= lua_topointer(L, -1);
					auto entry= visited_tables.find(sub_table);
					if(entry != visited_tables.end())
					{
						lua_pop(L, 1);
						continue;
					}
				}
			case LUA_TSTRING:
			case LUA_TNUMBER:
			case LUA_TBOOLEAN:
				break;
			default:
				lua_pop(L, 1);
				continue;
		}
		int key_type= lua_type(L, -2);
		switch(key_type)
		{
			case LUA_TSTRING:
				string_fields.push_back(std::string(lua_tostring(L, -2)));
				break;
			case LUA_TBOOLEAN:
				bool_fields.push_back(lua_toboolean(L, -2) != 0);
				break;
			case LUA_TNUMBER:
				{
					double as_double= lua_tonumber(L, -2);
					int as_int= lua_tointeger(L, -2);
					double int_turned_double= static_cast<int>(as_int);
					if(fabs(as_double - int_turned_double) < .001)
					{
						int_fields.push_back(as_int);
					}
					else
					{
						double_fields.push_back(as_double);
					}
				}
				break;
			default:
				break;
		}
		lua_pop(L, 1);
	}
	std::sort(string_fields.begin(), string_fields.end());
	std::sort(double_fields.begin(), double_fields.end());
	std::sort(int_fields.begin(), int_fields.end());
	std::sort(bool_fields.begin(), bool_fields.end());
	file->Write("{\n");
	std::string subindent= indent + "  ";
	for(auto&& field : string_fields)
	{
		file->Write(subindent);
		if(LuaHelpers::string_can_be_lua_identifier(L, field))
		{
			file->Write(field);
		}
		else
		{
			file->Write("[");
			LuaHelpers::push_lua_escaped_string(L, field);
			file->Write(lua_tostring(L, -1));
			file->Write("]");
			lua_pop(L, 1);
		}
		lua_getfield(L,  table_index, field.c_str());
		write_lua_value_to_file(L, lua_gettop(L), file, subindent, visited_tables, true);
		lua_pop(L, 1);
		file->Write("\n");
	}
	for(auto&& field : double_fields)
	{
		file->Write(subindent);
		file->Write(fmt::sprintf("[%.6f]", field));
		lua_pushnumber(L, field);
		lua_gettable(L, table_index);
		write_lua_value_to_file(L, lua_gettop(L), file, subindent, visited_tables, true);
		lua_pop(L, 1);
		file->Write("\n");
	}
	int next_array_style_index= 1;
	for(auto&& field : int_fields)
	{
		file->Write(subindent);
		bool needs_equals= true;
		if(field == next_array_style_index)
		{
			needs_equals= false;
			++next_array_style_index;
		}
		else
		{
			file->Write(fmt::sprintf("[%i]", field));
		}
		lua_pushnumber(L, field);
		lua_gettable(L, table_index);
		write_lua_value_to_file(L, lua_gettop(L), file, subindent, visited_tables, needs_equals);
		lua_pop(L, 1);
		file->Write("\n");
	}
	for(auto&& field : bool_fields)
	{
		file->Write(subindent);
		if(field)
		{
			file->Write("[true]");
		}
		else
		{
			file->Write("[false]");
		}
		lua_pushboolean(L, field);
		lua_gettable(L, table_index);
		write_lua_value_to_file(L, lua_gettop(L), file, subindent, visited_tables, true);
		lua_pop(L, 1);
		file->Write("\n");
	}
	file->Write(indent);
	file->Write("}");
}

void LuaHelpers::save_lua_table_to_file(lua_State* L, int table_index,
	std::string const& filename)
{
	RageFile* file= new RageFile;
	if(!file->Open(filename, RageFile::WRITE))
	{
		LuaHelpers::ReportScriptErrorFmt("Could not open %s to save lua data: %s", filename.c_str(), file->GetError().c_str());
		return;
	}
	std::unordered_set<void const*> visited_tables;
	std::string indent;
	file->Write("return ");
	write_lua_table_to_file(L, table_index, file, indent, visited_tables);
	file->Write("\n");
	file->Close();
	delete file;
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

void LuaHelpers::rec_print_table(lua_State* L, std::string const& name, std::string const& indent)
{
	switch(lua_type(L, -1))
	{
		case LUA_TNIL:
			LOG->Trace("%s%s: nil", indent.c_str(), name.c_str());
			break;
		case LUA_TNUMBER:
			LOG->Trace("%s%s number: %f", indent.c_str(), name.c_str(), lua_tonumber(L, -1));
			break;
		case LUA_TBOOLEAN:
			LOG->Trace("%s%s bool: %d", indent.c_str(), name.c_str(), lua_toboolean(L, -1));
			break;
		case LUA_TSTRING:
			LOG->Trace("%s%s string: %s", indent.c_str(), name.c_str(), lua_tostring(L, -1));
			break;
		case LUA_TTABLE:
			{
				size_t tablen= lua_objlen(L, -1);
				LOG->Trace("%s%s table: %zu", indent.c_str(), name.c_str(), tablen);
				std::string subindent= indent + "  ";
				lua_pushnil(L);
				while(lua_next(L, -2) != 0)
				{
					lua_pushvalue(L, -2);
					std::string sub_name= lua_tostring(L, -1);
					lua_pop(L, 1);
					rec_print_table(L, sub_name, subindent);
					lua_pop(L, 1);
				}
			}
			break;
		case LUA_TFUNCTION:
			LOG->Trace("%s%s function:", indent.c_str(), name.c_str());
			break;
		case LUA_TUSERDATA:
			LOG->Trace("%s%s userdata:", indent.c_str(), name.c_str());
			break;
		case LUA_TTHREAD:
			LOG->Trace("%s%s thread:", indent.c_str(), name.c_str());
			break;
		case LUA_TLIGHTUSERDATA:
			LOG->Trace("%s%s lightuserdata:", indent.c_str(), name.c_str());
			break;
		default:
			break;
	}
}

namespace
{
	// Creates a table from an XNode and leaves it on the stack.
	void CreateTableFromXNodeRecursive( Lua *L, const XNode *pNode )
	{
		// create our base table
		lua_newtable( L );

		for (auto const &pAttr: pNode->m_attrs)
		{
			lua_pushstring( L, pAttr.first.c_str() );			// push key
			pNode->PushAttrValue( L, pAttr.first );	// push value

			//add key-value pair to our table
			lua_settable( L, -3 );
		}

		for (auto const *c: *pNode)
		{
			lua_pushstring( L, c->m_sName.c_str() ); // push key

			// push value (more correctly, build this child's table and leave it there)
			CreateTableFromXNodeRecursive( L, c );

			// add key-value pair to the table
			lua_settable( L, -3 );
		}
	}
}

void LuaHelpers::CreateTableFromXNode( Lua *L, const XNode *pNode )
{
	// This creates our table and leaves it on the stack.
	CreateTableFromXNodeRecursive( L, pNode );
}

static int GetLuaStack( lua_State *L )
{
	std::string sErr;
	LuaHelpers::Pop( L, sErr );

	lua_Debug ar;

	for( int iLevel = 0; lua_getstack(L, iLevel, &ar); ++iLevel )
	{
		if( !lua_getinfo(L, "nSluf", &ar) )
		{
			break;
		}
		// The function is now on the top of the stack.
		const char *file = ar.source[0] == '@' ? ar.source + 1 : ar.short_src;
		const char *name;
		vector<std::string> vArgs;

		auto logAndPop = [&](char const *luaName) {
			auto *luaStr = lua_tostring(L, -1);
			vArgs.push_back( fmt::sprintf("%s = %s", luaName, luaStr != nullptr ? luaStr : "nil") );
			lua_pop( L, 1 ); // pop value
		};

		if( !strcmp(ar.what, "C") )
		{
			for( int i = 1; i <= ar.nups && (name = lua_getupvalue(L, -1, i)) != nullptr; ++i )
			{
				logAndPop(name);
			}
		}
		else
		{
			for( int i = 1; (name = lua_getlocal(L, &ar, i)) != nullptr; ++i )
			{
				logAndPop(name);
			}
		}

		// If the first call is this function, omit it from the trace.
		if( iLevel == 0 && lua_iscfunction(L, -1) && lua_tocfunction(L, 1) == GetLuaStack )
		{
			lua_pop( L, 1 ); // pop function
			continue;
		}
		lua_pop( L, 1 ); // pop function

		sErr += fmt::sprintf( "\n%s:", file );
		if( ar.currentline != -1 )
		{
			sErr += fmt::sprintf( "%i:", ar.currentline );
		}
		if( ar.name && ar.name[0] )
		{
			sErr += fmt::sprintf( " %s", ar.name );
		}
		else if( !strcmp(ar.what, "main") || !strcmp(ar.what, "tail") || !strcmp(ar.what, "C") )
		{
			sErr += fmt::sprintf( " %s", ar.what );
		}
		else
		{
			sErr += fmt::sprintf( " unknown" );
		}
		sErr += fmt::sprintf( "(%s)", Rage::join(",", vArgs).c_str() );
	}

	LuaHelpers::Push( L, sErr );
	return 1;
}


static int LuaPanic( lua_State *L )
{
	GetLuaStack( L );

	std::string sErr;
	LuaHelpers::Pop( L, sErr );

	RageException::Throw( "[Lua panic] %s", sErr.c_str() );
}

// Actor registration
static vector<RegisterWithLuaFn>	*g_vRegisterActorTypes = nullptr;

void LuaManager::Register( RegisterWithLuaFn pfn )
{
	if( g_vRegisterActorTypes == nullptr )
		g_vRegisterActorTypes = new vector<RegisterWithLuaFn>;

	g_vRegisterActorTypes->push_back( pfn );
}


LuaManager::LuaManager()
{
	pImpl = new Impl;
	LUA = this; // so that LUA is available when we call the Register functions

	lua_State *L = lua_open();
	ASSERT( L != nullptr );

	lua_atpanic( L, LuaPanic );
	m_pLuaMain = L;

	lua_pushcfunction( L, luaopen_base ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_math ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_string ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_table ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_debug ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_package ); lua_call( L, 0, 0 ); // this one seems safe -shake
	// these two can be dangerous. don't use them
	// (unless you know what you are doing). -aj
#if 0
	lua_pushcfunction( L, luaopen_io ); lua_call( L, 0, 0 );
	lua_pushcfunction( L, luaopen_os ); lua_call( L, 0, 0 );
#endif

	// Store the thread pool in a table on the stack, in the main thread.
#define THREAD_POOL 1
	lua_newtable( L );

	RegisterTypes();
}

LuaManager::~LuaManager()
{
	lua_close( m_pLuaMain );
	Rage::safe_delete( pImpl );
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

		// Store the new thread in THREAD_POOL, so it isn't collected.
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
	p = nullptr;
}

/*
 * Low-level access to Lua is always serialized through pImpl->g_pLock; we never run the Lua
 * core simultaneously from multiple threads.  However, when a thread has an acquired
 * lua_State, it can release Lua for use by other threads.  This allows Lua bindings
 * to process long-running actions, without blocking all other threads from using Lua
 * until it finishes.
 *
 * Lua *L = LUA->Get();			// acquires L and locks Lua
 * lua_newtable(L);				// does something with Lua
 * LUA->YieldLua();				// unlocks Lua for lengthy operation; L is still owned, but can't be used
 * std::string s = ReadFile("/filename.txt");	// time-consuming operation; other threads may use Lua in the meantime
 * LUA->UnyieldLua();			// relock Lua
 * lua_pushstring( L, s );		// finish working with it
 * LUA->Release( L );			// release L and unlock Lua
 *
 * YieldLua() must not be called when already yielded, or when a lua_State has not been
 * acquired (you have nothing to yield), and always unyield before releasing the
 * state.  Recursive handling is OK:
 *
 * L1 = LUA->Get();
 * LUA->YieldLua();				// yields
 *   L2 = LUA->Get();			// unyields
 *   LUA->Release(L2);			// re-yields
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
	Lua *L = Get();

	if( g_vRegisterActorTypes )
	{
		for (auto *actorType: *g_vRegisterActorTypes)
		{
			actorType(L);
		}
	}

	Release( L );
}

LuaThreadVariable::LuaThreadVariable( const std::string &sName, const std::string &sValue )
{
	m_Name = new LuaReference;
	m_pOldValue = new LuaReference;

	Lua *L = LUA->Get();
	LuaHelpers::Push( L, sName );
	m_Name->SetFromStack( L );
	LuaHelpers::Push( L, sValue );
	SetFromStack( L );
	LUA->Release( L );
}

LuaThreadVariable::LuaThreadVariable( const std::string &sName, const LuaReference &Value )
{
	m_Name = new LuaReference;
	m_pOldValue = new LuaReference;

	Lua *L = LUA->Get();
	LuaHelpers::Push( L, sName );
	m_Name->SetFromStack( L );

	Value.PushSelf( L );
	SetFromStack( L );
	LUA->Release( L );
}

// name and value are on the stack
LuaThreadVariable::LuaThreadVariable( lua_State *L )
{
	m_Name = new LuaReference;
	m_pOldValue = new LuaReference;

	lua_pushvalue( L, -2 );
	m_Name->SetFromStack( L );

	SetFromStack( L );

	lua_pop( L, 1 );
}

std::string LuaThreadVariable::GetCurrentThreadIDString()
{
	uint64_t iID = RageThread::GetCurrentThreadID();
	return fmt::sprintf( "%08x%08x", uint32_t(iID >> 32), uint32_t(iID) );
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

	std::string sThreadIDString = GetCurrentThreadIDString();
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
	ASSERT( lua_isnumber(L, -1) != 0 );

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

	m_Name->PushSelf( L );
	lua_gettable( L, -2 );
	m_pOldValue->SetFromStack( L );

	m_Name->PushSelf( L );
	lua_pushvalue( L, -3 );
	lua_settable( L, -3 );

	AdjustCount( L, +1 );

	lua_pop( L, 2 );
}

LuaThreadVariable::~LuaThreadVariable()
{
	Lua *L = LUA->Get();

	PushThreadTable( L, true );
	m_Name->PushSelf( L );
	m_pOldValue->PushSelf( L );
	lua_settable( L, -3 );

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
	delete m_Name;
}

namespace
{
	struct LClass
	{
		std::string m_sBaseName;
		vector<std::string> m_vMethods;
	};
}

XNode *LuaHelpers::GetLuaInformation()
{
	XNode *pLuaNode = new XNode( "Lua" );

	XNode *pGlobalsNode = pLuaNode->AppendChild( "GlobalFunctions" );
	XNode *pClassesNode = pLuaNode->AppendChild( "Classes" );
	XNode *pNamespacesNode = pLuaNode->AppendChild( "Namespaces" );
	XNode *pSingletonsNode = pLuaNode->AppendChild( "Singletons" );
	XNode *pEnumsNode = pLuaNode->AppendChild( "Enums" );
	XNode *pConstantsNode = pLuaNode->AppendChild( "Constants" );

	vector<std::string> vFunctions;
	std::map<std::string, LClass> mClasses;
	std::map<std::string, vector<std::string> > mNamespaces;
	std::map<std::string, std::string> mSingletons;
	std::map<std::string, float> mConstants;
	std::map<std::string, std::string> mStringConstants;
	std::map<std::string, vector<std::string> > mEnums;

	Lua *L = LUA->Get();
	FOREACH_LUATABLE( L, LUA_GLOBALSINDEX )
	{
		std::string sKey;
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
				luaL_getmetatable( L, name );
				ASSERT( !lua_isnil(L, -1) );
				lua_getfield( L, -1, "base" );
				name = lua_tostring( L, -1 );

				if( name )
					c.m_sBaseName = name;
				lua_pop( L, 2 ); // pop name and metatable

				// Get methods.
				FOREACH_LUATABLE( L, -1 )
				{
					std::string sMethod;
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
			std::string sType;
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
			/*
			{
				lua_Debug ar;
				lua_getfield( L, LUA_GLOBALSINDEX, sKey );
				lua_getinfo( L, ">S", &ar ); // Pops the function
				printf( "%s: %s\n", sKey.c_str(), ar.short_src );
			}
			*/
			break;
		}
	}

	// Find namespaces
	lua_pushcfunction( L, luaopen_package ); lua_call( L, 0, 0 );
	lua_getglobal( L, "package" );
	ASSERT( lua_istable(L, -1) );
	lua_getfield( L, -1, "loaded" );
	ASSERT( lua_istable(L, -1) );

	//const std::string BuiltInPackages[] = { "_G", "coroutine", "debug", "math", "package", "string", "table" };
	std::array<std::string, 7> const BuiltInPackages =
	{
		{
			"_G",
			"coroutine",
			"debug",
			"math",
			"package",
			"string",
			"table"
		}
	};
	auto endIter = BuiltInPackages.end();
	FOREACH_LUATABLE( L, -1 )
	{
		std::string sNamespace;
		LuaHelpers::Pop( L, sNamespace );
		if( find(BuiltInPackages.begin(), endIter, sNamespace) != endIter )
			continue;
		vector<std::string> &vNamespaceFunctions = mNamespaces[sNamespace];
		FOREACH_LUATABLE( L, -1 )
		{
			std::string sFunction;
			LuaHelpers::Pop( L, sFunction );
			vNamespaceFunctions.push_back( sFunction );
		}
		sort( vNamespaceFunctions.begin(), vNamespaceFunctions.end() );
	}
	lua_pop( L, 2 );

	LUA->Release( L );

	/* Globals */
	sort( vFunctions.begin(), vFunctions.end() );
	for (auto const &func: vFunctions)
	{
		XNode *pFunctionNode = pGlobalsNode->AppendChild( "Function" );
		pFunctionNode->AppendAttr( "name", func );
	}

	/* Classes */
	for (auto const &c: mClasses)
	{
		XNode *pClassNode = pClassesNode->AppendChild( "Class" );

		pClassNode->AppendAttr( "name", c.first );
		if( !c.second.m_sBaseName.empty() )
			pClassNode->AppendAttr( "base", c.second.m_sBaseName );
		for (auto const &m: c.second.m_vMethods)
		{
			XNode *pMethodNode = pClassNode->AppendChild( "Function" );
			pMethodNode->AppendAttr( "name", m );
		}
	}

	/* Singletons */
	for (auto const &s: mSingletons)
	{
		if( mClasses.find(s.first) != mClasses.end() )
			continue;
		XNode *pSingletonNode = pSingletonsNode->AppendChild( "Singleton" );
		pSingletonNode->AppendAttr( "name", s.first );
		pSingletonNode->AppendAttr( "class", s.second );
	}

	/* Namespaces */
	for (auto &iter: mNamespaces)
	{
		XNode *pNamespaceNode = pNamespacesNode->AppendChild( "Namespace" );
		const vector<std::string> &vNamespace = iter.second;
		pNamespaceNode->AppendAttr( "name", iter.first );

		for (auto const &func: vNamespace)
		{
			XNode *pFunctionNode = pNamespaceNode->AppendChild( "Function" );
			pFunctionNode->AppendAttr( "name", func );
		}
	}

	/* Enums */
	for (auto &iter: mEnums)
	{
		XNode *pEnumNode = pEnumsNode->AppendChild( "Enum" );

		const vector<std::string> &vEnum = iter.second;
		pEnumNode->AppendAttr( "name", iter.first );

		for( unsigned i = 0; i < vEnum.size(); ++i )
		{
			XNode *pEnumValueNode = pEnumNode->AppendChild( "EnumValue" );
			pEnumValueNode->AppendAttr( "name", fmt::sprintf("'%s'", vEnum[i].c_str()) );
			pEnumValueNode->AppendAttr( "value", i );
		}
	}

	/* Constants, String Constants */
	for (auto const &c: mConstants)
	{
		XNode *pConstantNode = pConstantsNode->AppendChild( "Constant" );

		pConstantNode->AppendAttr( "name", c.first );
    if( c.second == std::trunc(c.second) )
		{
			pConstantNode->AppendAttr( "value", static_cast<int>(c.second) );
		}
		else
		{
			pConstantNode->AppendAttr( "value", c.second );
		}
	}

	for (auto const &s: mStringConstants)
	{
		XNode *pConstantNode = pConstantsNode->AppendChild( "Constant" );
		pConstantNode->AppendAttr( "name", s.first );
		pConstantNode->AppendAttr( "value", fmt::sprintf("'%s'", s.second.c_str()) );
	}

	return pLuaNode;
}

bool LuaHelpers::run_script_file_in_state(lua_State* L,
	std::string const& filename, int return_values, bool blank_env)
{
	std::string script;
	if(!GetFileContents(filename, script))
	{
		if(return_values == LUA_MULTRET)
		{
			return false;
		}
		for(int i= 0; i < return_values; ++i)
		{
			lua_pushnil(L);
		}
		return false;
	}
	std::string err;
	if(!LuaHelpers::RunScript(L, script, "@" + filename, err, 0, return_values, false, blank_env))
	{
		err= fmt::sprintf("Lua runtime error: %s", err.c_str());
		LuaHelpers::ReportScriptError(err);
		return false;
	}
	return true;
}

bool LuaHelpers::RunScriptFile(const std::string &sFile, bool blank_env)
{
	std::string sScript;
	if( !GetFileContents(sFile, sScript) )
		return false;

	Lua *L = LUA->Get();

	std::string sError;
	if(!LuaHelpers::RunScript(L, sScript, "@" + sFile, sError, 0, 0, false, blank_env))
	{
		LUA->Release( L );
		sError = fmt::sprintf( "Lua runtime error: %s", sError.c_str() );
		LuaHelpers::ReportScriptError(sError);
		return false;
	}
	LUA->Release( L );

	return true;
}


bool LuaHelpers::LoadScript( Lua *L, const std::string &sScript, const std::string &sName, std::string &sError )
{
	// load string
	int ret = luaL_loadbuffer( L, sScript.data(), sScript.size(), sName.c_str() );
	if( ret )
	{
		LuaHelpers::Pop( L, sError );
		return false;
	}

	return true;
}

void LuaHelpers::ScriptErrorMessage(std::string const& Error)
{
	if (MESSAGEMAN != nullptr)
	{
		Message msg("ScriptError");
		msg.SetParam("message", Error);
		MESSAGEMAN->Broadcast(msg);
	}
}

Dialog::Result LuaHelpers::ReportScriptError(std::string const& Error, std::string ErrorType, bool UseAbort)
{
	// Protect from a recursion loop resulting from a mistake in the error reporting lua.
	if(!InReportScriptError)
	{
		InReportScriptError= true;
		ScriptErrorMessage(Error);
		InReportScriptError= false;
	}
	LOG->Warn( "%s", Error.c_str());
	if(UseAbort)
	{
		std::string with_correct= Error + "  Correct this and click Retry, or Cancel to break.";
		return Dialog::AbortRetryIgnore(with_correct, ErrorType);
	}
	//Dialog::OK(Error, ErrorType);
	return Dialog::ok;
}

void infinite_loop_preventer(lua_State* L, lua_Debug*)
{
	luaL_error(L, "Infinite loop detected, too many instructions.");
}

bool LuaHelpers::RunScriptOnStack(Lua *L, std::string &Error, int Args,
	int ReturnValues, bool ReportError, bool blank_env)
{
	if(blank_env)
	{
		lua_newtable(L);
		lua_setfenv(L, lua_gettop(L) - Args - 1);
	}
	lua_pushcfunction( L, GetLuaStack );

	// move the error function above the function and params
	int ErrFunc = lua_gettop(L) - Args - 1;
	lua_insert( L, ErrFunc );
	lua_sethook(L, infinite_loop_preventer, LUA_MASKCOUNT, 1000000000);

	// evaluate
	int ret = lua_pcall( L, Args, ReturnValues, ErrFunc );
	if( ret )
	{
		if(ReportError)
		{
			std::string lerror;
			LuaHelpers::Pop( L, lerror );
			Error+= lerror;
			ReportScriptError(Error);
		}
		else
		{
			LuaHelpers::Pop( L, Error );
		}
		lua_remove( L, ErrFunc );
		if(ReturnValues == LUA_MULTRET)
		{
			return false;
		}
		for( int i = 0; i < ReturnValues; ++i )
		{
			lua_pushnil( L );
		}
		return false;
	}

	lua_remove( L, ErrFunc );
	return true;
}

bool LuaHelpers::RunScript(Lua *L, const std::string &Script,
	const std::string &Name, std::string &Error, int Args, int ReturnValues,
	bool ReportError, bool blank_env)
{
	std::string lerror;
	if( !LoadScript(L, Script, Name, lerror) )
	{
		Error+= lerror;
		if(ReportError)
		{
			ReportScriptError(Error);
		}
		lua_pop( L, Args );
		if(ReturnValues == LUA_MULTRET)
		{
			return false;
		}
		for( int i = 0; i < ReturnValues; ++i )
		{
			lua_pushnil( L );
		}
		return false;
	}

	// move the function above the params
	lua_insert( L, lua_gettop(L) - Args );

	return LuaHelpers::RunScriptOnStack(L, Error, Args, ReturnValues,
		ReportError, blank_env);
}

bool LuaHelpers::RunExpression( Lua *L, const std::string &sExpression, const std::string &sName, bool blank_env)
{
	std::string sError= fmt::sprintf("Lua runtime error parsing \"%s\": ", sName.size()? sName.c_str():sExpression.c_str());
	if(!LuaHelpers::RunScript(L, "return " + sExpression, sName.empty()? std::string("in"):sName, sError, 0, 1, true, blank_env))
	{
		return false;
	}
	return true;
}

void LuaHelpers::ParseCommandList( Lua *L, const std::string &sCommands, const std::string &sName, bool bLegacy )
{
	std::string sLuaFunction;
	if( sCommands.size() > 0 && sCommands[0] == '\033' )
	{
		// This is a compiled Lua chunk. Just pass it on directly.
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
		ParseCommands( sCommands, cmds, bLegacy );

		// Convert cmds to a Lua function
		std::ostringstream s;

		s << "return function(self)\n";

		if( bLegacy )
			s << "\tparent = self:GetParent();\n";

		for (auto const &cmd: cmds.v)
		{
			std::string sCmdName = cmd.GetName();
			if( bLegacy )
			{
				sCmdName = Rage::make_lower(sCmdName);
			}
			s << "\tself:" << sCmdName << "(";

			bool bFirstParamIsString = bLegacy && (
					sCmdName == "horizalign" ||
					sCmdName == "vertalign" ||
					sCmdName == "effectclock" ||
					sCmdName == "blend" ||
					sCmdName == "ztestmode" ||
					sCmdName == "cullmode" ||
					sCmdName == "playcommand" ||
					sCmdName == "queuecommand" ||
					sCmdName == "queuemessage" ||
					sCmdName == "settext");

			for( unsigned i=1; i<cmd.m_vsArgs.size(); i++ )
			{
				std::string sArg = cmd.m_vsArgs[i];

				// "+200" -> "200"
				if( sArg[0] == '+' )
					sArg.erase( sArg.begin() );

				if( i==1 && bFirstParamIsString ) // string literal, legacy only
				{
					Rage::replace(sArg, "'", "\\'" );	// escape quote
					s << "'" << sArg << "'";
				}
				else if( sArg[0] == '#' )	// HTML color
				{
					Rage::Color col;	// in case FromString fails
					col.FromString( sArg );
					// col is still valid if FromString fails
					s << col.r << "," << col.g << "," << col.b << "," << col.a;
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

	std::string sError;
	if( !LuaHelpers::RunScript(L, sLuaFunction, sName, sError, 0, 1) )
		LOG->Warn( "Compiling \"%s\": %s", sLuaFunction.c_str(), sError.c_str() );

	// The function is now on the stack.
}

/* Like luaL_typerror, but without the special case for argument 1 being "self"
 * in method calls, so we give a correct error message after we remove self. */
int LuaHelpers::TypeError( Lua *L, int iArgNo, std::string const &szName )
{
	std::string sType;
	luaL_pushtype( L, iArgNo );
	LuaHelpers::Pop( L, sType );

	lua_Debug debug;
	if( !lua_getstack( L, 0, &debug ) )
	{
		return luaL_error( L, "invalid type (%s expected, got %s)",
			szName.c_str(), sType.c_str() );
	}
	else
	{
		lua_getinfo( L, "n", &debug );
		return luaL_error( L, "bad argument #%d to \"%s\" (%s expected, got %s)",
			iArgNo, debug.name? debug.name:"(unknown)", szName.c_str(), sType.c_str() );
	}
}

void LuaHelpers::DeepCopy( lua_State *L )
{
	luaL_checktype( L, -2, LUA_TTABLE );
	luaL_checktype( L, -1, LUA_TTABLE );

	// Call DeepCopy(t, u), where t is our referenced object and u is the new table.
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
		{
			lua_pushvalue( L, lua_upvalueindex(i+2) );
		}
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

#include "ProductInfo.h"
LuaFunction( ProductFamily, (std::string) PRODUCT_FAMILY );
LuaFunction( ProductVersion, (std::string) product_version );
LuaFunction( ProductID, (std::string) PRODUCT_ID );

extern char const * const version_date;
extern char const * const version_time;
LuaFunction( VersionDate, (std::string) version_date );
LuaFunction( VersionTime, (std::string) version_time );

LuaFunction( scale, Rage::scale(FArg(1), FArg(2), FArg(3), FArg(4), FArg(5)) );

LuaFunction( clamp, Rage::clamp(FArg(1), FArg(2), FArg(3)) );

#include "LuaBinding.h"
namespace
{
	static int Trace( lua_State *L )
	{
		std::string sString = SArg(1);
		LOG->Trace( "%s", sString.c_str() );
		return 0;
	}
	static int Warn( lua_State *L )
	{
		std::string sString = SArg(1);
		LOG->Warn( "%s", sString.c_str() );
		return 0;
	}
	static int Flush(lua_State*)
	{
		LOG->Flush();
		return 0;
	}
	static int CheckType( lua_State *L )
	{
		std::string sType = SArg(1);
		bool bRet = LuaBinding::CheckLuaObjectType( L, 2, sType );
		LuaHelpers::Push( L, bRet );
		return 1;
	}
	static int ReadFile( lua_State *L )
	{
		std::string sPath = SArg(1);

		/* Release Lua while we call GetFileContents, so we don't access
		 * it while we read from the disk. */
		LUA->YieldLua();

		std::string sFileContents;
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

	/* RunWithThreadVariables(func, { a = "x", b = "y" }, arg1, arg2, arg3 ... }
	 * calls func(arg1, arg2, arg3) with two LuaThreadVariable set, and returns
	 * the return values of func(). */
	static int RunWithThreadVariables( lua_State *L )
	{
		luaL_checktype( L, 1, LUA_TFUNCTION );
		luaL_checktype( L, 2, LUA_TTABLE );

		vector<LuaThreadVariable *> apVars;
		FOREACH_LUATABLE( L, 2 )
		{
			lua_pushvalue( L, -2 );
			LuaThreadVariable *pVar = new LuaThreadVariable( L );
			apVars.push_back( pVar );
		}

		lua_remove( L, 2 );

		/* XXX: We want to clean up apVars on errors, but if we lua_pcall,
		 * we won't propagate the error upwards. */
		int iArgs = lua_gettop(L) - 1;
		lua_call( L, iArgs, LUA_MULTRET );
		int iVals = lua_gettop(L);

		for (auto *v: apVars)
		{
			delete v;
		}
		return iVals;
	}

	static int GetThreadVariable( lua_State *L )
	{
		luaL_checkstring( L, 1 );
		lua_pushvalue( L, 1 );
		LuaThreadVariable::GetThreadVariable( L );
		return 1;
	}

	static int ReportScriptError(lua_State* L)
	{
		std::string error= "Script error occurred.";
		std::string error_type= "LUA_ERROR";
		if(lua_isstring(L, 1))
		{
			error= SArg(1);
		}
		if(lua_isstring(L, 2))
		{
			error_type= SArg(2);
		}
		LuaHelpers::ReportScriptError(error, error_type);
		return 0;
	}
	static int save_lua_table(lua_State* L)
	{
		std::string filename= SArg(1);
		if(lua_type(L, 2) != LUA_TTABLE)
		{
			luaL_error(L, "Second arg to save_lua_table must be a table.");
		}
		LuaHelpers::save_lua_table_to_file(L, 2, filename);
		return 0;
	}
	static int load_config_lua(lua_State* L)
	{
		std::string filename= SArg(1);
		int stack_size_before_call= lua_gettop(L);
		LuaHelpers::run_script_file_in_state(L, filename, LUA_MULTRET, true);
		int stack_size_after_call= lua_gettop(L);
		return (stack_size_after_call - stack_size_before_call);
	}

	const luaL_Reg luaTable[] =
	{
		LIST_METHOD( Trace ),
		LIST_METHOD( Warn ),
		LIST_METHOD( Flush ),
		LIST_METHOD( CheckType ),
		LIST_METHOD( ReadFile ),
		LIST_METHOD( RunWithThreadVariables ),
		LIST_METHOD( GetThreadVariable ),
		LIST_METHOD( ReportScriptError ),
		LIST_METHOD(save_lua_table),
		LIST_METHOD(load_config_lua),
		{ nullptr, nullptr }
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
