/* ActorCommands - Actor command parsing and reading helpers. */

#ifndef ActorCommands_H
#define ActorCommands_H

#include "RageTypes.h"

struct ParsedCommandToken
{
	void Set( const CString &sToken );	// fill in all the different types

	CString s;
	float f;
	int i;
	bool b;
	RageColor c;	// HTML-type color which is packed into one parameter
	bool bColorIsValid;	// true if c is a valid HTML-type color

	operator const CString () const		{ return s; };
	operator const float () const		{ return f; };
	operator const int () const			{ return i; };
	operator const bool () const		{ return b; };
	operator const RageColor () const	{ return c; };
};

struct ParsedCommand
{
	void Set( const CString &sCommand );

	vector<ParsedCommandToken> vTokens;

	CString GetOriginalCommandString() const;	// for when reporting an error in number of params
};

// Take a command list string and return pointers to each of the tokens in the 
// string.  sCommand list is a list of commands separated by ';'.
void ParseCommands( const CString &sCommands, vector<ParsedCommand> &vCommandsOut );


#define HandleParams int iMaxIndexAccessed = 0;
#define sParam(i) (GetParam<CString>(command,i,iMaxIndexAccessed))
#define fParam(i) (GetParam<float>(command,i,iMaxIndexAccessed))
#define iParam(i) (GetParam<int>(command,i,iMaxIndexAccessed))
#define bParam(i) (GetParam<bool>(command,i,iMaxIndexAccessed))
#define cParam(i) (ColorParam(command,i,iMaxIndexAccessed))
#define CheckHandledParams if( iMaxIndexAccessed != (int)command.vTokens.size()-1 ) { IncorrectActorParametersWarning( command, iMaxIndexAccessed ); }
void IncorrectActorParametersWarning( const ParsedCommand& command, int iMaxIndexAccessed );

template<class T>
inline T GetParam( const ParsedCommand& command, int iIndex, int& iMaxIndexAccessedOut )
{
	iMaxIndexAccessedOut = max( iIndex, iMaxIndexAccessedOut );
	if( iIndex < int(command.vTokens.size()) )
	{
		return (T)command.vTokens[iIndex];
	}
	else
	{
		ParsedCommandToken pct;
		pct.Set( "" );
		return (T)pct;
	}
}

inline RageColor ColorParam( const ParsedCommand& command, int iIndex, int& iMaxIndexAccessed )
{
	if( command.vTokens[iIndex].bColorIsValid )
		return GetParam<RageColor>(command,iIndex,iMaxIndexAccessed);
	else 
		return RageColor( fParam(iIndex+0),fParam(iIndex+1),fParam(iIndex+2),fParam(iIndex+3) );
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
