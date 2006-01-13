#include "global.h"
#include "LocalizedString.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "SubscriptionManager.h"

static SubscriptionManager<LocalizedString> m_Subscribers;

class LocalizedStringImplDefault: public ILocalizedStringImpl
{
public:
	static ILocalizedStringImpl *Create() { return new LocalizedStringImplDefault; }

	void ILocalizedStringImpl::Load( const RString& sGroup, const RString& sName )
	{
		m_sValue = sName;
	}

	const RString &GetLocalized() const { return m_sValue; }

private:
	CString m_sValue;
};

static LocalizedString::MakeLocalizer g_pMakeLocalizedStringImpl = LocalizedStringImplDefault::Create;

void LocalizedString::RegisterLocalizer( MakeLocalizer pFunc )
{
	g_pMakeLocalizedStringImpl = pFunc;
	FOREACHS( LocalizedString*, *m_Subscribers.m_pSubscribers, l )
	{
		LocalizedString *pLoc = *l;
		pLoc->CreateImpl();
	}
}

LocalizedString::LocalizedString( const RString& sGroup, const RString& sName )
{
	m_Subscribers.Subscribe( this );

	m_sGroup = sGroup;
	m_sName = sName;
	m_pImpl = NULL;

	CreateImpl();
}

LocalizedString::~LocalizedString()
{
	m_Subscribers.Unsubscribe( this );

	SAFE_DELETE( m_pImpl );
}

void LocalizedString::CreateImpl()
{
	SAFE_DELETE( m_pImpl );
	m_pImpl = g_pMakeLocalizedStringImpl();
	m_pImpl->Load(  m_sGroup, m_sName );
}

void LocalizedString::Load( const RString& sGroup, const RString& sName )
{
	m_sGroup = sGroup;
	m_sName = sName;
	CreateImpl();
}

const RString &LocalizedString::GetValue() const
{
	return m_pImpl->GetLocalized();
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
