/* Commands - Actor command parsing and reading helpers. */

#ifndef Commands_H
#define Commands_H

#include "RageTypes.h"

class Command
{
public:
	void Load( const CString &sCommand );

	CString GetOriginalCommandString() const;	// used when reporting an error in number of args

	CString		GetName() const;	// the command name is the first argument in all-lowercase 

	void Clear() { m_vsArgs.clear(); }

	struct Arg
	{
		CString s;
		operator CString () const;
		operator float () const;
		operator int () const;
		operator bool () const;
	};
	Arg GetArg( unsigned index ) const;

	vector<CString> m_vsArgs;
};

class Commands
{
public:
	vector<Command> v;
};

// Take a command list string and return pointers to each of the tokens in the 
// string.  sCommand list is a list of commands separated by ';'.
// TODO: This is expensive to do during the game.  Eventually, 
// move all calls to ParseCommands to happen during load, then execute
// from the parsed Command structures.
void ParseCommands( const CString &sCmds, Commands &vCmdsOut );
Commands ParseCommands( const CString &sCmds );


#define BeginHandleArgs int iMaxIndexAccessed = 0;
#define GET_ARG(type,i) iMaxIndexAccessed = max( i, iMaxIndexAccessed ); command.GetArg##type( i );
#define sArg(i) GetArg<CString>(command,i,iMaxIndexAccessed)
#define fArg(i) GetArg<float>(command,i,iMaxIndexAccessed)
#define iArg(i) GetArg<int>(command,i,iMaxIndexAccessed)
#define bArg(i) GetArg<bool>(command,i,iMaxIndexAccessed)
#define cArg(i) GetColorArg(command,i,iMaxIndexAccessed)
#define EndHandleArgs if( iMaxIndexAccessed != (int)command.m_vsArgs.size()-1 ) { IncorrectNumberArgsWarning( command, iMaxIndexAccessed ); }
void IncorrectNumberArgsWarning( const Command& command, int iMaxIndexAccessed );

template<class T>
inline T GetArg( const Command& command, int iIndex, int& iMaxIndexAccessedOut )
{
	iMaxIndexAccessedOut = max( iIndex, iMaxIndexAccessedOut );
	return (T)command.GetArg(iIndex);
}

inline RageColor GetColorArg( const Command& command, int iIndex, int& iMaxIndexAccessed )
{
	RageColor c;
	if( c.FromString( GetArg<CString>(command,iIndex,iMaxIndexAccessed) ) )
		return c;
	else 
		return RageColor( fArg(iIndex+0),fArg(iIndex+1),fArg(iIndex+2),fArg(iIndex+3) );
}

#endif

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
