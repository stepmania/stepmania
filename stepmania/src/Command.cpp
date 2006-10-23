#include "global.h"
#include "Command.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"
#include "Foreach.h"


RString Command::GetName() const 
{
	if( m_vsArgs.empty() )
		return RString();
	RString s = m_vsArgs[0];
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

void Command::Load( const RString &sCommand )
{
	m_vsArgs.clear();
	split( sCommand, ",", m_vsArgs, false );	// don't ignore empty
}

RString Command::GetOriginalCommandString() const
{
	return join( ",", m_vsArgs );
}

static void SplitWithQuotes( const RString sSource, const char Delimitor, vector<RString> &asOut, const bool bIgnoreEmpty )
{
	/* Short-circuit if the source is empty; we want to return an empty vector if
	 * the string is empty, even if bIgnoreEmpty is true. */
	if( sSource.empty() )
		return;

	size_t startpos = 0;
	do {
		size_t pos = startpos;
		while( pos < sSource.size() )
		{
			if( sSource[pos] == Delimitor )
				break;

			if( sSource[pos] == '"' || sSource[pos] == '\'' )
			{
				/* We've found a quote.  Search for the close. */
				pos = sSource.find( sSource[pos], pos+1 );
				if( pos == string::npos )
					pos = sSource.size();
				else
					++pos;
			}
			else
				++pos;
		}

		if( pos-startpos > 0 || !bIgnoreEmpty )
		{
			/* Optimization: if we're copying the whole string, avoid substr; this
			 * allows this copy to be refcounted, which is much faster. */
			if( startpos == 0 && pos-startpos == sSource.size() )
				asOut.push_back( sSource );
			else
			{
				const RString AddCString = sSource.substr( startpos, pos-startpos );
				asOut.push_back( AddCString );
			}
		}

		startpos = pos+1;
	} while( startpos <= sSource.size() );
}

RString Commands::GetOriginalCommandString() const
{
	RString s;
	FOREACH_CONST( Command, v, c )
		s += c->GetOriginalCommandString();
	return s;
}

void ParseCommands( const RString &sCommands, Commands &vCommandsOut )
{
	vector<RString> vsCommands;
	SplitWithQuotes( sCommands, ';', vsCommands, true );	// do ignore empty
	vCommandsOut.v.resize( vsCommands.size() );

	for( unsigned i=0; i<vsCommands.size(); i++ )
	{
		Command &cmd = vCommandsOut.v[i];
		cmd.Load( vsCommands[i] );
	}
}

Commands ParseCommands( const RString &sCommands )
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
