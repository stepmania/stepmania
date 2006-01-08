#include "global.h"
#include "LocalizedString.h"
#include "Foreach.h"
#include "LuaFunctions.h"
#include "LuaManager.h"
#include "ThemeMetric.h"
#include "RageUtil.h"

class LocalizedStringImpl : public ThemeMetric<RString>
{
public:
	LocalizedStringImpl( const RString& sGroup, const RString& sName ) :
		ThemeMetric<RString>( sGroup, sName )
	{
		// Ugly.  Our virtual method Read() isn't yet set up when Read gets
		// called through the constructor.  Read again explicitly.
		Read();
	}

	virtual void Read()
	{
		if( m_sName != ""  &&  THEME  &&   THEME->IsThemeLoaded() )
		{
			THEME->GetString( m_sGroup, m_sName, m_currentValue );
			m_bIsLoaded = true;
		}
	}

};

LocalizedString::LocalizedString( const RString& sGroup, const RString& sName )
{
	m_pImpl = new LocalizedStringImpl(sGroup,sName);
}

LocalizedString::~LocalizedString()
{
	SAFE_DELETE( m_pImpl );
}

void LocalizedString::Load( const RString& sGroup, const RString& sName )
{
	m_pImpl->Load( sGroup, sName );
}

const RString &LocalizedString::GetValue() const
{
	return m_pImpl->GetValue();
}

bool LocalizedString::IsLoaded() const
{
	return m_pImpl->IsLoaded();
}

/*
 * Copyright (c) 2001-2005 Chris Danford
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
