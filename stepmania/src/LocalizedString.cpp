#include "global.h"
#include "LocalizedString.h"
#include "Foreach.h"

static RString (*g_pfnLocalizer)(const RString&,const RString&) = NULL;

void LocalizedString::RegisterLocalizer( RString (*pfnLocalizer)(const RString&, const RString&) )
{
	g_pfnLocalizer = pfnLocalizer;
}

#include "SubscriptionManager.h"
template<>
set<LocalizedString*>* SubscriptionManager<LocalizedString>::s_pSubscribers = NULL;

void LocalizedString::RefreshLocalizedStrings()
{
	FOREACHS( LocalizedString*, *SubscriptionManager<LocalizedString>::s_pSubscribers, p )
		(*p)->Refresh();
}

RString LocalizedString::LocalizeString( const RString &sSection, const RString &sName )
{
	ASSERT( g_pfnLocalizer );
	return g_pfnLocalizer( sSection, sName );
}

LocalizedString::LocalizedString( const RString &sSection, const RString &sName )
{
	m_bLoaded = false;
	m_sSection = sSection;
	m_sName = sName;

	SubscriptionManager<LocalizedString>::Subscribe( this );
}


LocalizedString::~LocalizedString()
{
	SubscriptionManager<LocalizedString>::Unsubscribe( this );
}


LocalizedString::operator RString() const
{
	return GetValue();
}

const RString &LocalizedString::GetValue() const
{
	ASSERT(m_bLoaded);
	return m_sValue;
}

void LocalizedString::Refresh()
{
	m_sValue = LocalizeString( m_sSection, m_sName );
	m_bLoaded = true;
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
