/* MessageManager - Control lights. */

#ifndef MessageManager_H
#define MessageManager_H

#include <set>
#include <map>

class IMessageSubscriber
{
public:
	virtual void HandleMessage( const CString& sMessage ) = 0;
};

struct lua_State;

enum Message
{
	MESSAGE_CURRENT_SONG_CHANGED,
	MESSAGE_CURRENT_STEPS_P1_CHANGED,
	MESSAGE_CURRENT_STEPS_P2_CHANGED,
	MESSAGE_EDIT_STEPS_TYPE_CHANGED,
	MESSAGE_EDIT_SOURCE_STEPS_CHANGED,
	MESSAGE_EDIT_SOURCE_STEPS_TYPE_CHANGED,
	NUM_MESSAGES
};
const CString& MessageToString( Message m );

template<class T>
class BroadcastOnChange
{
private:
	Message mSendWhenChanged;
	T val;
public:
	BroadcastOnChange( Message m ) { mSendWhenChanged = m; }
	const T Get() const { return val; }
	void Set( T t ) { val = t; MESSAGEMAN->Broadcast( MessageToString(mSendWhenChanged) ); }
	operator T () const { return val; }
};

template<class T>
class BroadcastOnChangePtr
{
private:
	Message mSendWhenChanged;
	T *val;
public:
	BroadcastOnChangePtr( Message m ) { mSendWhenChanged = m; }
	const T* Get() const { return val; }
	void Set( T* t ) { val = t; MESSAGEMAN->Broadcast( MessageToString(mSendWhenChanged) ); }
	operator T* () const { return val; }
	T* operator->() const { return val; }
};

template<class T, int N>
class BroadcastOnChangePtr1D
{
private:
	Message mSendWhenChanged;
	T *val[N];
public:
	BroadcastOnChangePtr1D( Message m ) { mSendWhenChanged = m; }
	const T* Get( unsigned i ) const { return val[i]; }
	void Set( unsigned i, T* t ) { val[i] = t; MESSAGEMAN->Broadcast( MessageToString((Message)(mSendWhenChanged+i)) ); }
	T* operator[]( unsigned i ) const { return val[i]; }
};

class MessageManager
{
public:
	MessageManager();
	~MessageManager();

	void Subscribe( IMessageSubscriber* pSubscriber, const CString& sMessage );
	void Subscribe( IMessageSubscriber* pSubscriber, Message m );
	void Unsubscribe( IMessageSubscriber* pSubscriber, const CString& sMessage );
	void Unsubscribe( IMessageSubscriber* pSubscriber, Message m );
	void Broadcast( const CString& sMessage ) const;

	// Lua
	void PushSelf( lua_State *L );

private:
	typedef set<IMessageSubscriber*> SubscribersSet;
	map<CString,SubscribersSet> m_MessageToSubscribers;
};


extern MessageManager*	MESSAGEMAN;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2003-2004 Chris Danford
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
