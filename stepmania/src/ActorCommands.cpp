#include "global.h"
#include "ActorCommands.h"
#include "Command.h"
#include "Foreach.h"
#include "RageUtil.h"
#include <sstream>
#include "LuaManager.h"
#include "LuaBinding.h"

#define MAX_SIMULTANEOUS_LUA_FUNCTIONS 10000

static CString GetNextFunctionName()
{
	static int id = 0;
	id = (id+1) % MAX_SIMULTANEOUS_LUA_FUNCTIONS;
	return ssprintf("F%d",id);
}

ActorCommands::ActorCommands( const Commands& cmds )
{
	Register( cmds );
}

ActorCommands::~ActorCommands()
{
	if( m_sLuaFunctionName.size() )
		Unregister();
}

CString ActorCommands::GetFunctionName() const
{
	return m_sLuaFunctionName;
}

void ActorCommands::Register( const Commands& cmds )
{
	// TODO: calculate a better function name, or figure out how
	// to keep a pointer directly to the Lua function so no global
	// table lookup is necessary.
	m_sLuaFunctionName = GetNextFunctionName();

	//
	// Convert cmds to a Lua function
	//
	ostringstream s;
	
	s << m_sLuaFunctionName << " = function(self)\n";

	FOREACH_CONST( Command, cmds.v, c )
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

	lua_pushstring( LUA->L, m_sLuaFunctionName );
	lua_pushnil( LUA->L );
	lua_settable( LUA->L, LUA_GLOBALSINDEX );

	m_sLuaFunctionName = "";
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
