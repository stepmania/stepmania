#include "global.h"	// testing updates
#include "ActorCommands.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"

void IncorrectActorParametersWarning( const ParsedCommand &command, int iMaxIndexAccessed )
{
	const CString sError = ssprintf( "Actor::HandleCommand: Wrong number of parameters in command '%s'.  Expected %d but there are %u.",
		command.GetOriginalCommandString().c_str(), iMaxIndexAccessed+1, unsigned(command.vTokens.size()) );
	LOG->Warn( sError );
	Dialog::OK( sError );
}

void ParsedCommandToken::Set( const CString &sToken )
{
	s = sToken;
	f = strtof( sToken, NULL );
	i = atoi( sToken );
	b = i != 0;
	bColorIsValid = c.FromString( sToken );
}

void ParsedCommand::Set( const CString &sCommand )
{
	CStringArray vsTokens;
	split( sCommand, ",", vsTokens, false );	// don't ignore empty

	vTokens.resize( vsTokens.size() );

	for( unsigned j=0; j<vsTokens.size(); j++ )
	{
		const CString &sToken = vsTokens[j];
		vTokens[j].Set( sToken );
	}
}

CString ParsedCommand::GetOriginalCommandString() const
{
	CStringArray asTokens;
	for( unsigned i=0; i<vTokens.size(); i++ )
		asTokens.push_back( vTokens[i].s );
	return join( ",", asTokens );
}

void ParseCommands( const CString &sCommands, vector<ParsedCommand> &vCommandsOut )
{
	vCommandsOut.clear();
	CStringArray vsCommands;
	split( sCommands, ";", vsCommands, true );	// do ignore empty
	
	vCommandsOut.resize( vsCommands.size() );
	
	for( unsigned i=0; i<vsCommands.size(); i++ )
	{
		const CString &sCommand = vsCommands[i];
		ParsedCommand &pc = vCommandsOut[i];
		pc.Set( sCommand );
	}
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
