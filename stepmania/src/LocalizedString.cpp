#include "global.h"
#include "LocalizedString.h"
#include "Foreach.h"
#include "LuaFunctions.h"
#include "LuaManager.h"

static RString DefaultLocalizer( const RString &sSection, const RString &sName )
{
	return sName;
}
static RString (*g_pfnLocalizer)(const RString&,const RString&) = DefaultLocalizer;

void LocalizedString::RegisterLocalizer( RString (*pfnLocalizer)(const RString&, const RString&) )
{
	g_pfnLocalizer = pfnLocalizer;
}

#include "SubscriptionManager.h"
template<>
SubscriptionManager<LocalizedString> g_Subscribers;

void LocalizedString::RefreshLocalizedStrings()
{
	FOREACHS( LocalizedString*, *g_Subscribers.m_pSubscribers, p )
		(*p)->Refresh();
}

RString LocalizedString::LocalizeString( const RString &sSection, const RString &sName )
{
	return g_pfnLocalizer( sSection, sName );
}

LocalizedString::LocalizedString( const RString &sSection, const RString &sName )
{
	m_sSection = sSection;
	m_sName = sName;
	m_sValue = sName;

	g_Subscribers.Subscribe( this );

	Refresh();
}


LocalizedString::~LocalizedString()
{
	g_Subscribers.Unsubscribe( this );
}


LocalizedString::operator RString() const
{
	return GetValue();
}

const RString &LocalizedString::GetValue() const
{
	return m_sValue;
}

void LocalizedString::Refresh()
{
	m_sValue = LocalizeString( m_sSection, m_sName );
}

LuaFunction( LocalizeString, LocalizedString::LocalizeString( SArg(1), SArg(2) ) )

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
