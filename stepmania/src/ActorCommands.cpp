#include "global.h"
#include "ActorCommands.h"
#include "Command.h"
#include "Foreach.h"
#include "RageUtil.h"
#include <sstream>
#include "LuaBinding.h"

ActorCommands::ActorCommands( const Commands& cmds )
{
	m_cmds = cmds;
	Register();
}

void ActorCommands::Register()
{
	//
	// Convert cmds to a Lua function
	//
	ostringstream s;
	
	s << "return function(self)\n";

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
	LUA->RunScript( s2, 1 );

	/* The function is now on the stack. */
	this->SetFromStack();
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
