#include "global.h"
#include "ActorCommands.h"
#include "Command.h"
#include "Foreach.h"
#include "RageUtil.h"
#include <sstream>
#include "LuaManager.h"
#include "LuaBinding.h"

#include "SubscriptionManager.h"
template<>
set<ActorCommands*>* SubscriptionManager<ActorCommands>::s_pSubscribers = NULL;


static CString GetNextFunctionName()
{
	static int id = 0;
	++id;
	return ssprintf( "F%08x",id );
}

ActorCommands::ActorCommands( const Commands& cmds )
{
	SubscriptionManager<ActorCommands>::Subscribe( this );
	
	m_cmds = cmds;

	Register();
}

ActorCommands::~ActorCommands()
{
	SubscriptionManager<ActorCommands>::Unsubscribe( this );

	if( m_sLuaFunctionName.size() )
		Unregister();
}

ActorCommands::ActorCommands( const ActorCommands& cpy )
{
	SubscriptionManager<ActorCommands>::Subscribe( this );

	m_sLuaFunctionName = GetNextFunctionName();

	/* We need to make a new function, since we'll be unregistered separately.  Set
	 * the function by reference, so we don't make a new function unless we're actually
	 * changed. */
	ostringstream s;
	s << m_sLuaFunctionName << " = " << cpy.m_sLuaFunctionName;

	CString s2 = s.str();
	LUA->RunScript( s2 );
}

ActorCommands &ActorCommands::operator=( const ActorCommands& cpy )
{
	if( this == &cpy )
		return *this;

	*this = cpy;
	return *this;
}

void ActorCommands::PushSelf( lua_State *L ) const
{
	lua_pushstring( L, m_sLuaFunctionName ); // function name
	lua_gettable( L, LUA_GLOBALSINDEX ); // function to be called

	ASSERT_M( !lua_isnil(L, -1), m_sLuaFunctionName.c_str() )
}

void ActorCommands::Register()
{
	if( m_cmds.v.size() == 0 )
	{
		m_sLuaFunctionName = "nop";
		return;
	}

	// TODO: calculate a better function name, or figure out how
	// to keep a pointer directly to the Lua function so no global
	// table lookup is necessary.
	m_sLuaFunctionName = GetNextFunctionName();

	//
	// Convert cmds to a Lua function
	//
	ostringstream s;
	
	s << m_sLuaFunctionName << " = function(self)\n";

	FOREACH_CONST( Command, m_cmds.v, c )
	{
		const Command& cmd = (*c);
		CString sName = cmd.GetName();
		s << "\tself:" << sName << "(";

		bool bFirstParamIsString =
			sName == "horizalign" ||
			sName == "vertalign" ||
			sName == "effectclock" ||
			sName == "blend" ||
			sName == "ztestmode" ||
			sName == "cullmode" ||
			sName == "playcommand" ||
			sName == "queuecommand";

		for( unsigned i=1; i<cmd.m_vsArgs.size(); i++ )
		{
			CString sArg = cmd.m_vsArgs[i];

			// "+200" -> "200"
			if( sArg[0] == '+' )
				sArg.erase( sArg.begin() );

			if( i==1 && bFirstParamIsString ) // string literal
			{
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


	CString s2 = s.str();
	LUA->RunScript( s2 );
}

void ActorCommands::Unregister()
{
	if( LUA == NULL )
		return;	// nothing to do

	ASSERT( m_sLuaFunctionName.size() );

	if( m_sLuaFunctionName != "nop" )
	{
		lua_pushstring( LUA->L, m_sLuaFunctionName );
		lua_pushnil( LUA->L );
		lua_settable( LUA->L, LUA_GLOBALSINDEX );
	}

	m_sLuaFunctionName = "";
}

void ActorCommands::ReRegisterAll()
{
	if( SubscriptionManager<ActorCommands>::s_pSubscribers == NULL )
		return;
	FOREACHS( ActorCommands*, *SubscriptionManager<ActorCommands>::s_pSubscribers, p )
		(*p)->Register();
}


/*
 * (c) 2004 Chris Danford
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
