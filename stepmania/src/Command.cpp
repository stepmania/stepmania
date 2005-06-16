#include "global.h"	// testing updates
#include "Command.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"
#include "LuaManager.h"


void IncorrectNumberArgsWarning( const Command &command, int iMaxIndexAccessed )
{
	const CString sError = ssprintf( "Actor::HandleCommand: Wrong number of arguments in command '%s'.  Expected %d but there are %u.",
		command.GetOriginalCommandString().c_str(), iMaxIndexAccessed+1, unsigned(command.m_vsArgs.size()) );
	LOG->Warn( sError );
	Dialog::OK( sError );
}

CString Command::GetName() const 
{
	if( m_vsArgs.empty() )
		return "";
	CString s = m_vsArgs[0];
	s.MakeLower();
	return s;
}

Command::Arg Command::GetArg( unsigned index ) const
{
	Arg a;
	if( index < m_vsArgs.size() )
		a.s = m_vsArgs[index];
	return a;
}

Command::Arg::operator CString ()
{
	CString sValue = s;
	LUA->RunAtExpressionS( sValue );
	return sValue;
}

Command::Arg::operator float ()
{
	LUA->PrepareExpression( s );	// strip invalid chars
	return LuaHelpers::RunExpressionF( s );
}

Command::Arg::operator int ()
{
	LUA->PrepareExpression( s );	// strip invalid chars
	return (int) LuaHelpers::RunExpressionF( s );
}

Command::Arg::operator bool ()
{
	LUA->PrepareExpression( s );	// strip invalid chars
	return LuaHelpers::RunExpressionF( s ) != 0.0f;
}

void Command::Load( const CString &sCommand )
{
	m_vsArgs.clear();
	split( sCommand, ",", m_vsArgs, false );	// don't ignore empty
}

CString Command::GetOriginalCommandString() const
{
	return join( ",", m_vsArgs );
}

void ParseCommands( const CString &sCommands, Commands &vCommandsOut )
{
	CStringArray vsCommands;
	split( sCommands, ";", vsCommands, true );	// do ignore empty
	
	vCommandsOut.v.resize( vsCommands.size() );

	for( unsigned i=0; i<vsCommands.size(); i++ )
	{
		Command &cmd = vCommandsOut.v[i];
		cmd.Load( vsCommands[i] );
	}
}

Commands ParseCommands( const CString &sCommands )
{
	Commands vCommands;
	ParseCommands( sCommands, vCommands );
	return vCommands;
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
