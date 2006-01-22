#include "global.h"
#include "ScreenMessage.h"
#include "RageLog.h"

ASMHClass AutoScreenMessageHandler;

ScreenMessage ASMHClass::ToMessageNumber( const RString &Name )
{
	//If uninitilized, initilize the map.  We KNOW it will be NULL
	if ( m_pScreenMessages == NULL )
	{
		m_pScreenMessages = new map < RString, ScreenMessage >;
		m_iCurScreenMessage = SM_User;
	}

	//If the ScreenMessage doesn't exist yet, create it
	if ( m_pScreenMessages->find( Name ) == m_pScreenMessages->end() )
	{
		m_iCurScreenMessage = ScreenMessage((int)m_iCurScreenMessage + 1);
		(*m_pScreenMessages)[ Name ] = m_iCurScreenMessage;
	}

	return (*m_pScreenMessages)[Name];
}

void ASMHClass::LogMessageNumbers( )
{
	map < RString, ScreenMessage >::iterator iter;

	//Log all ScreenMessages currently in the map

	LOG->Trace( "Current ScreenMessage: %d  -- SM_User: %d", m_iCurScreenMessage, SM_User );
    for ( iter = m_pScreenMessages->begin(); iter != m_pScreenMessages->end(); iter++ )
		LOG->Trace( "ScreenMessage: %3d: %s", iter->second, (*iter).first.c_str() );
}

RString	ASMHClass::NumberToString( ScreenMessage SM )
{
	map < RString, ScreenMessage >::iterator iter;

	//Log all ScreenMessages currently in the map

	LOG->Trace( "Current ScreenMessage: %d  -- SM_User: %d", m_iCurScreenMessage, SM_User );
    for ( iter = m_pScreenMessages->begin(); iter != m_pScreenMessages->end(); iter++ )
		if ( SM == iter->second )
			return (*iter).first;

	return RString();
}

/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard, Charles Lohr
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
