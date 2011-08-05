/* LuaDriverHandle: an object representing the internal handles and methods
 * needed for a LuaDriver to interact with an I/O device. */

#ifndef LUA_DRIVER_HANDLE_H
#define LUA_DRIVER_HANDLE_H

struct lua_State;

/* Registers a LuaDriverHandle with the core class. */
#include "RageUtil.h"	// for CreateClass

#define REGISTER_LUA_DRIVER_HANDLE( API, fn ) \
	struct Register##API { \
		Register##API() { LuaDriverHandle::RegisterAPI( #API, CreateClass<LuaDriverHandle_##API, LuaDriverHandle>() ); } \
	}; \
	static Register##API register##API;

typedef LuaDriverHandle* (*MakeHandleFn)();

class LuaDriverHandle
{
public:
	static void RegisterAPI( const RString &sName, MakeHandleFn fn );
	static void PushAPIHandle( lua_State *L, const RString &sName );

	LuaDriverHandle() { }
	virtual ~LuaDriverHandle() { }

	/* can't implement Open() abstractly; the call differs between APIs */
	virtual void Close() { }

	virtual bool IsOpen() const { return false; }

	virtual int GetRevisionMajor() const { return 0; }
	virtual int GetRevisionMinor() const { return 0; }

	virtual int GetError() const { return 0; }
	virtual const char* GetErrorStr( int err ) const { return NULL; }

	void PushSelf( lua_State *L );
};

#endif // LUA_DRIVER_HANDLE_H

/*
 * (c) 2011 Mark Cannon
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
