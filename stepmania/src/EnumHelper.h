#ifndef ENUM_HELPER_H
#define ENUM_HELPER_H

#include "LuaReference.h"
#include "RageUtil.h"

extern "C"
{
#include "lua-5.1/src/lua.h"
}

#define FOREACH_ENUM_N( e, max, var )	for( e var=(e)0; var<max; enum_add<e>( var, +1 ) )
#define FOREACH_ENUM( e, var )	for( e var=(e)0; var<NUM_##e; enum_add<e>( var, +1 ) )

int CheckEnum( lua_State *L, LuaReference &table, int iPos, int iInvalid, const char *szType, bool bAllowInvalid );

template<typename T>
struct EnumTraits
{
	static LuaReference StringToEnum;
	static LuaReference EnumToString;
	static T Invalid;
	static const char *szName;
};
template<typename T> LuaReference EnumTraits<T>::StringToEnum;
template<typename T> LuaReference EnumTraits<T>::EnumToString;

namespace Enum
{
	template<typename T>
	static T Check( lua_State *L, int iPos, bool bAllowInvalid = false )
	{
		return (T) CheckEnum( L, EnumTraits<T>::StringToEnum, iPos, EnumTraits<T>::Invalid, EnumTraits<T>::szName, bAllowInvalid );
	}
	template<typename T>
	static void Push( lua_State *L, T iVal )
	{
		/* Enum_Invalid values are nil in Lua. */
		if( iVal == EnumTraits<T>::Invalid )
		{
			lua_pushnil( L );
			return;
		}

		/* Look up the string value. */
		EnumTraits<T>::EnumToString.PushSelf( L );
		lua_rawgeti( L, -1, iVal + 1 );
		lua_remove( L, -2 );
	}

	void SetMetatable( lua_State *L, LuaReference &EnumTable, LuaReference &EnumIndexTable, const char *szName );
};

const RString &EnumToString( int iVal, int iMax, const char **szNameArray, auto_ptr<RString> *pNameCache ); // XToString helper

#define XToString2(X) \
	COMPILE_ASSERT( NUM_##X == ARRAYLEN(X##Names) ); \
	const RString& X##ToString( X x ) \
	{	\
		static auto_ptr<RString> as_##X##Name[NUM_##X+2]; \
		return EnumToString( x, NUM_##X, X##Names, as_##X##Name ); \
	} \
	namespace StringConversion { template<> RString ToString<X>( const X &value ) { return X##ToString(value); } }
#define XToString(X, CNT) XToString2(X)

#define XToLocalizedString(X)      \
	const RString &X##ToLocalizedString( X x ) \
	{       \
		static auto_ptr<LocalizedString> g_##X##Name[NUM_##X]; \
		if( g_##X##Name[0].get() == NULL ) { \
			for( unsigned i = 0; i < NUM_##X; ++i ) \
			{ \
				auto_ptr<LocalizedString> ap( new LocalizedString(#X, X##ToString((X)i)) ); \
				g_##X##Name[i] = ap; \
			} \
		} \
		return g_##X##Name[x]->GetValue();  \
	}

#define StringToX(X)	\
	X StringTo##X( const RString& s ) \
	{	\
		for( unsigned i = 0; i < ARRAYLEN(X##Names); ++i )	\
			if( !s.CompareNoCase(X##Names[i]) )	\
				return (X)i;	\
		return X##_Invalid;	\
	} \
	namespace StringConversion { template<> bool FromString<X>( const RString &sValue, X &out ) { out = StringTo##X(sValue); return out != X##_Invalid; } }

// currently unused
#define LuaDeclareType(X)

#define LuaXType(X)	\
template struct EnumTraits<X>; \
static void Lua##X(lua_State* L) \
{ \
	/* Create the EnumToString table: { "UnlockEntry_ArcadePoints", "UnlockEntry_DancePoints" } */ \
	lua_newtable( L ); \
	FOREACH_ENUM( X, i ) \
	{ \
		RString s = X##ToString( i ); \
		lua_pushstring( L, (#X "_")+s ); \
		lua_rawseti( L, -2, i+1 ); /* 1-based */ \
	} \
	EnumTraits<X>::EnumToString.SetFromStack( L ); \
	EnumTraits<X>::EnumToString.PushSelf( L ); \
	lua_setglobal( L, #X ); \
	/* Create the StringToEnum table: { "UnlockEntry_ArcadePoints" = 0, "UnlockEntry_DancePoints" = 1 } */ \
	lua_newtable( L ); \
	FOREACH_ENUM( X, i ) \
	{ \
		RString s = X##ToString( i ); \
		lua_pushstring( L, (#X "_")+s ); \
		lua_pushnumber( L, i ); /* 0-based */ \
		lua_rawset( L, -3 ); \
	} \
	EnumTraits<X>::StringToEnum.SetFromStack( L ); \
	EnumTraits<X>::StringToEnum.PushSelf( L ); \
	Enum::SetMetatable( L, EnumTraits<X>::EnumToString, EnumTraits<X>::StringToEnum, #X ); \
} \
REGISTER_WITH_LUA_FUNCTION( Lua##X ); \
template<> X EnumTraits<X>::Invalid = X##_Invalid; \
template<> const char *EnumTraits<X>::szName = #X; \
namespace LuaHelpers { template<> bool FromStack<X>( lua_State *L, X &Object, int iOffset ) { Object = Enum::Check<X>( L, iOffset ); return true; } } \
namespace LuaHelpers { template<> void Push<X>( lua_State *L, const X &Object ) { Enum::Push<X>( L, Object ); } }

#endif

/*
 * (c) 2004-2006 Chris Danford, Glenn Maynard
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
