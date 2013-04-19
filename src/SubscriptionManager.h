/* SubscriptionManager - Object that accepts subscriptions. */

#ifndef SubscriptionManager_H
#define SubscriptionManager_H

#include <set>

// When using this class be sure to use the global static initializer trick by wrapping the static object in a function like the
// following example:
//
// SubscriptionManager<T> & GetSubscribers()
// {
//    static SubscriptionManager<T> subscribers;
//    return subscribers;
// }
//
// This ensures that the object is always available when it's needed.
template<class T>
class SubscriptionManager
{
public:
	set<T*> m_pSubscribers;

	void Subscribe( T* p )
	{
#ifdef DEBUG
		typename set<T*>::iterator iter = m_pSubscribers.find( p );
		ASSERT_M( iter == m_pSubscribers.end(), "already subscribed" );
#endif
		m_pSubscribers.insert( p );
	}

	void Unsubscribe( T* p )
	{
		typename set<T*>::iterator iter = m_pSubscribers.find( p );
		ASSERT( iter != m_pSubscribers.end() );
		m_pSubscribers.erase( iter );
	}
};

#endif

/*
 * (c) 2004-2005 Chris Danford
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
