#ifndef LUA_HELPERS_H
#define LUA_HELPERS_H

struct lua_State;
class LuaManager
{
public:
	LuaManager();
	~LuaManager();

	void PrepareExpression( CString &sInOut );	// strip "//" comments and "+"

	/* Run an expression in the global environment, returning the given type. */
	bool RunExpressionB( const CString &str );
	float RunExpressionF( const CString &str );
	bool RunExpressionS( const CString &str, CString &sOut );

	void Fail( const CString &err );

	void SetGlobal( const CString &sName, int val ) { PushStack(val); SetGlobal( sName ); }

	void PushStack( bool val );
	void PushStack( int val );
	void PushStack( void *val );
	void PushStack( const CString &val );
	void PopStack( CString &out );
	bool GetStack( int pos, int &out );
	void SetGlobal( const CString &sName );

private:
	/* Register all functions in g_LuaFunctionList. */
	void RegisterFunctions();

	/* Run an expression.  The result is left on the Lua stack. */
	bool RunExpression( const CString &str );
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
