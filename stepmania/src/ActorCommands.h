#ifndef ActorCommands_H
#define ActorCommands_H
/*
-----------------------------------------------------------------------------
 Class: ActorCommands

 Desc: Actor command parsing and reading helpers

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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
