#include "global.h"
#include "ScreenMessage.h"
#include "RageLog.h"

#include <map>

const ScreenMessage SM_Invalid = "";
AutoScreenMessage(SM_None);
AutoScreenMessage(SM_MenuTimer);
AutoScreenMessage(SM_DoneFadingIn);
AutoScreenMessage(SM_BeginFadingOut);
AutoScreenMessage(SM_GoToNextScreen);
AutoScreenMessage(SM_GoToPrevScreen);
AutoScreenMessage(SM_GainFocus);
AutoScreenMessage(SM_LoseFocus);
AutoScreenMessage(SM_Pause);
AutoScreenMessage(SM_Success);
AutoScreenMessage(SM_Failure);

static map<RString, ScreenMessage> *m_pScreenMessages;

ScreenMessage ScreenMessageHelpers::ToScreenMessage( const RString &sName )
{
	if( m_pScreenMessages == nullptr )
		m_pScreenMessages = new map<RString, ScreenMessage>;

	if( m_pScreenMessages->find( sName ) == m_pScreenMessages->end() )
		(*m_pScreenMessages)[sName] = (ScreenMessage)sName;

	return (*m_pScreenMessages)[sName];
}

RString	ScreenMessageHelpers::ScreenMessageToString( ScreenMessage SM )
{
	for (auto const & it : *m_pScreenMessages)
		if( SM == it.second )
			return it.first;

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
