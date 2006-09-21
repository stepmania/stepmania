#include "global.h"
#include "ActorCommands.h"
#include "Command.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "RageLog.h"
#include <sstream>
#include "LuaManager.h"

ActorCommands::ActorCommands( const RString &sCommands )
{
	RString sLuaFunction;
	if( sCommands.size() > 0 && sCommands[0] == '\033' )
	{
		/* This is a compiled Lua chunk.  Just pass it on directly. */
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
		ParseCommands( sCommands, cmds );
		
		//
		// Convert cmds to a Lua function
		//
		ostringstream s;
		
		s << "return function(self,parent)\n";

		FOREACH_CONST( Command, cmds.v, c )
		{
			const Command& cmd = (*c);
			RString sName = cmd.GetName();
			TrimLeft( sName );
			TrimRight( sName );
			s << "\tself:" << sName << "(";

			bool bFirstParamIsString =
				sName == "horizalign" ||
				sName == "vertalign" ||
				sName == "effectclock" ||
				sName == "blend" ||
				sName == "ztestmode" ||
				sName == "cullmode" ||
				sName == "playcommand" ||
				sName == "queuecommand" ||
				sName == "queuemessage";

			for( unsigned i=1; i<cmd.m_vsArgs.size(); i++ )
			{
				RString sArg = cmd.m_vsArgs[i];

				// "+200" -> "200"
				if( sArg[0] == '+' )
					sArg.erase( sArg.begin() );

				if( i==1 && bFirstParamIsString ) // string literal
				{
					sArg.Replace( "'", "\\'" );	// escape quote
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

		sLuaFunction = s.str();
	}

	Lua *L = LUA->Get();

	RString sError;
	if( !LuaHelpers::RunScript( L, sLuaFunction, "in", sError, 1 ) )
	{
		FAIL_M( ssprintf("Compiling \"%s\": %s", sLuaFunction.c_str(), sError.c_str()) );
	}

	/* The function is now on the stack. */
	this->SetFromStack( L );
	LUA->Release( L );

	ASSERT_M( !this->IsNil(), sLuaFunction.c_str() );

	SetName( sLuaFunction );
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
