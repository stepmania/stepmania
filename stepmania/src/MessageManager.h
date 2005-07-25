/* MessageManager - Control lights. */

#ifndef MessageManager_H
#define MessageManager_H

#include <deque>

class IMessageSubscriber
{
public:
	virtual ~IMessageSubscriber() { }
	virtual void HandleMessage( const CString& sMessage ) = 0;
	virtual void ProcessMessages( float fDeltaTime );
	void ClearMessages( const CString sMessage = "" );

private:
	struct QueuedMessage
	{
		CString sMessage;
		float fDelayRemaining;
	};
	void HandleMessageInternal( const CString& sMessage );
	vector<QueuedMessage> m_aMessages;

	friend class MessageManager;
};

struct lua_State;

enum Message
{
	MESSAGE_CURRENT_STYLE_CHANGED,
	MESSAGE_PLAY_MODE_CHANGED,
	MESSAGE_CURRENT_SONG_CHANGED,
	MESSAGE_CURRENT_STEPS_P1_CHANGED,
	MESSAGE_CURRENT_STEPS_P2_CHANGED,
	MESSAGE_CURRENT_COURSE_CHANGED,
	MESSAGE_CURRENT_TRAIL_P1_CHANGED,
	MESSAGE_CURRENT_TRAIL_P2_CHANGED,
	MESSAGE_EDIT_STEPS_TYPE_CHANGED,
	MESSAGE_EDIT_SOURCE_STEPS_CHANGED,
	MESSAGE_EDIT_SOURCE_STEPS_TYPE_CHANGED,
	MESSAGE_EDIT_PREFERRED_DIFFICULTY_P1_CHANGED,
	MESSAGE_EDIT_PREFERRED_DIFFICULTY_P2_CHANGED,
	MESSAGE_EDIT_PREFERRED_COURSE_DIFFICULTY_P1_CHANGED,
	MESSAGE_EDIT_PREFERRED_COURSE_DIFFICULTY_P2_CHANGED,
	MESSAGE_EDIT_COURSE_ENTRY_INDEX_CHANGED,
	MESSAGE_GOAL_COMPLETE_P1,
	MESSAGE_GOAL_COMPLETE_P2,
	MESSAGE_NOTE_CROSSED,
	MESSAGE_NOTE_WILL_CROSS_IN_500MS,
	MESSAGE_NOTE_WILL_CROSS_IN_1000MS,
	MESSAGE_NOTE_WILL_CROSS_IN_1500MS,
	MESSAGE_CARD_REMOVED_P1,
	MESSAGE_CARD_REMOVED_P2,
	MESSAGE_BEAT_CROSSED,
	MESSAGE_MENU_UP_P1,
	MESSAGE_MENU_UP_P2,
	MESSAGE_MENU_DOWN_P1,
	MESSAGE_MENU_DOWN_P2,
	MESSAGE_MENU_LEFT_P1,
	MESSAGE_MENU_LEFT_P2,
	MESSAGE_MENU_RIGHT_P1,
	MESSAGE_MENU_RIGHT_P2,
	MESSAGE_MADE_CHOICE_P1,
	MESSAGE_MADE_CHOICE_P2,
	MESSAGE_COIN_INSERTED,
	MESSAGE_SIDE_JOINED_P1,
	MESSAGE_SIDE_JOINED_P2,
	MESSAGE_PLAYERS_FINALIZED,
	MESSAGE_ASSIST_TICK_CHANGED,
	MESSAGE_AUTOSYNC_CHANGED,
	MESSAGE_PREFERRED_SONG_GROUP_CHANGED,
	MESSAGE_PREFERRED_COURSE_GROUP_CHANGED,
	NUM_MESSAGES,	// leave this at the end
	MESSAGE_INVALID
};
const CString& MessageToString( Message m );

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
	void Broadcast( Message m ) const;

	// Lua
	void PushSelf( lua_State *L );
};

extern MessageManager*	MESSAGEMAN;	// global and accessable from anywhere in our program

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
	bool operator == ( const T &other ) const { return val == other; }
	bool operator != ( const T &other ) const { return val != other; }
};

template<class T, int N>
class BroadcastOnChange1D
{
private:
	typedef BroadcastOnChange<T> MyType;
	vector<MyType> val;
public:
	BroadcastOnChange1D( Message m )
	{
		for( unsigned i=0; i<N; i++ )
			val.push_back( BroadcastOnChange<T>((Message)(m+i)) );
	}
	const BroadcastOnChange<T>& operator[]( unsigned i ) const { return val[i]; }
	BroadcastOnChange<T>& operator[]( unsigned i ) { return val[i]; }
};

template<class T>
class BroadcastOnChangePtr
{
private:
	Message mSendWhenChanged;
	T *val;
public:
	BroadcastOnChangePtr( Message m ) { mSendWhenChanged = m; val = NULL; }
	T* Get() const { return val; }
	void Set( T* t ) { val = t; if(MESSAGEMAN) MESSAGEMAN->Broadcast( MessageToString(mSendWhenChanged) ); }
	operator T* () const { return val; }
	T* operator->() const { return val; }
};

template<class T, int N>
class BroadcastOnChangePtr1D
{
private:
	typedef BroadcastOnChangePtr<T> MyType;
	vector<MyType> val;
public:
	BroadcastOnChangePtr1D( Message m )
	{
		for( unsigned i=0; i<N; i++ )
			val.push_back( BroadcastOnChangePtr<T>((Message)(m+i)) );
	}
	const BroadcastOnChangePtr<T>& operator[]( unsigned i ) const { return val[i]; }
	BroadcastOnChangePtr<T>& operator[]( unsigned i ) { return val[i]; }
};

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
