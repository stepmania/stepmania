#include "global.h"	// testing updates

/*
-----------------------------------------------------------------------------
 Class: ActorCommands

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorCommands.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/ArchHooks/ArchHooks.h"

void IncorrectActorParametersWarning( const ParsedCommand &command, int iMaxIndexAccessed )
{
	const CString sError = ssprintf( "Actor::HandleCommand: Wrong number of parameters in command '%s'.  Expected %d but there are %u.",
		command.GetOriginalCommandString().c_str(), iMaxIndexAccessed+1, command.vTokens.size() );
	LOG->Warn( sError );
	HOOKS->MessageBoxOK( sError );
}

void ParsedCommandToken::Set( const CString &sToken )
{
	s = sToken;
	f = (float) atof( sToken );
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
