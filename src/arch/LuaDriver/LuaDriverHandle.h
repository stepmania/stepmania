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
