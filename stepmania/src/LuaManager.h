#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

struct lua_State;
typedef void (*RegisterWithLuaFn)(lua_State*);

class LuaManager
{
public:
	// Every Actor should register its class at program initialization.
	static void Register( RegisterWithLuaFn pfn );

	LuaManager();
	~LuaManager();

	void PrepareExpression( CString &sInOut );	// strip "//" comments and "+"

	bool RunScriptFile( const CString &sFile );

	/* Register all subscribing types.  There's no harm in registering when already registered. */
	void RegisterTypes();

	/* Reset the environment, freeing any globals left over by previously executed scripts. */
	void ResetState();

	/* Run a script with the given name.  Return values are left on the Lua stack.
	 * Returns false on error, with sError set*/
	bool RunScript( const CString &sScript, const CString &sName, CString &sError, int iReturnValues = 0 );

	/* Convenience: run a script with one return value, displaying an error on failure.
	 * The return value is left on the Lua stack. */
	bool RunScript( const CString &sExpression, const CString &sName = "" );

	/* Run an expression in the global environment, returning the given type. */
	bool RunExpressionB( const CString &str );
	float RunExpressionF( const CString &str );
	int RunExpressionI( const CString &str );
	bool RunExpressionS( const CString &str, CString &sOut );

	/* If sStr begins with @, evaluate the rest as an expression and store the result over sStr. */
	bool RunAtExpressionS( CString &sStr );
	float RunAtExpressionF( const CString &sStr );

	void Fail( const CString &err );

	void SetGlobal( const CString &sName, int val ) { PushStack(val); SetGlobal( sName ); }
	void SetGlobal( const CString &sName, bool val ) { PushStack(val); SetGlobal( sName ); }
	void UnsetGlobal( const CString &sName ) { PushStackNil(); SetGlobal( sName ); }

	void PushStackNil();
	void PushNopFunction();
	static void PushStack( bool val, lua_State *L = NULL );
	static void PushStack( float val, lua_State *L = NULL );
	static void PushStack( int val, lua_State *L = NULL );
	static void PushStack( void *val, lua_State *L = NULL );
	static void PushStack( const CString &val, lua_State *L = NULL );
	void PopStack( CString &out );
	bool GetStack( int pos, int &out );
	void SetGlobal( const CString &sName );

	/* Create a Lua array (a table with indices starting at 1) of the given vector,
	 * and push it on the stack. */
	static void CreateTableFromArray( const vector<bool> &aIn, lua_State *L = NULL );
	/* Read the table at the top of the stack back into a vector. */
	static void ReadArrayFromTable( vector<bool> &aOut, lua_State *L = NULL );

	lua_State *L;
};

extern LuaManager *LUA;

#endif

/*
 * (c) 2004 Glenn Maynard
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
