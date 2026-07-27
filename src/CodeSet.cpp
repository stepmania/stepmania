#include "global.h"
#include "CodeSet.h"
#include "ThemeManager.h"
#include "InputEventPlus.h"
#include "MessageManager.h"

#include <vector>


#define CODE_NAMES		THEME->GetMetric (sType,"CodeNames")
#define CODE( s )		THEME->GetMetric (sType,ssprintf("Code%s",s.c_str()))
void InputQueueCodeSet::Load( const RString &sType )
{
	//
	// Load codes
	//
	split( CODE_NAMES, ",", m_asCodeNames, true );

	for( unsigned c=0; c<m_asCodeNames.size(); c++ )
	{
		std::vector<RString> asBits;
		split( m_asCodeNames[c], "=", asBits, true );
		RString sCodeName = asBits[0];
		if( asBits.size() > 1 )
			m_asCodeNames[c] = asBits[1];

		InputQueueCode code;
		if( !code.Load(CODE(sCodeName)) )
			continue;

		m_aCodes.push_back( code );
	}
}

RString InputQueueCodeSet::Input( const InputEventPlus &input ) const
{
	for( unsigned i = 0; i < m_aCodes.size(); ++i )
	{
		if( !m_aCodes[i].EnteredCode(input.GameI.controller) )
			continue;

		return m_asCodeNames[i];
	}
	return "";
}

bool InputQueueCodeSet::InputMessage( const InputEventPlus &input, Message &msg ) const
{
	RString sCodeName = Input( input );
	if( sCodeName.empty() )
		return false;

	msg.SetName("Code");
	msg.SetParam( "PlayerNumber", input.pn );
	msg.SetParam( "Name", sCodeName );
	return true;
}

/*
 * (c) 2007 Glenn Maynard
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
