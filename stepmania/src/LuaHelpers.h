#ifndef LUA_HELPERS_H
#define LUA_HELPERS_H

struct lua_State;
namespace Lua
{
	bool RunExpression( const CString &str );

	void Fail( lua_State *L, const CString &err );

	/* Add all registered functions into L. */
	void RegisterFunctions( lua_State *L );

	void PushStack( lua_State *L, bool out );
	void PushStack( lua_State *L, int out );
	void PushStack( lua_State *L, void *out );
	void PushStack( lua_State *L, const CString &out );
	void PopStack( lua_State *L, CString &out );
	bool GetStack( lua_State *L, int pos, int &out );
};

#endif
/*
 * Copyright (c) 2004 by the person(s) listed below.  All rights reserved.
 *	Glenn Maynard
 */
