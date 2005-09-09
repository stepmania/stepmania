/* MessageManager - Control lights. */

#ifndef MessageManager_H
#define MessageManager_H

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
	Message_CurrentStyleChanged,
	Message_PlayModeChanged,
	Message_CurrentSongChanged,
	Message_CurrentStepsP1Changed,
	Message_CurrentStepsP2Changed,
	Message_CurrentCourseChanged,
	Message_CurrentTrailP1Changed,
	Message_CurrentTrailP2Changed,
	Message_EditStepsTypeChanged,
	Message_EditSourceStepsChanged,
	Message_EditSourceStepsTypeChanged,
	Message_PreferredDifficultyP1Changed,
	Message_PreferredDifficultyP2Changed,
	Message_PreferredCourseDifficultyP1Changed,
	Message_PreferredCourseDifficultyP2Changed,
	Message_EditCourseEntryIndexChanged,
	Message_EditLocalProfileIDChanged,
	Message_GoalCompleteP1,
	Message_GoalCompleteP2,
	Message_NoteCrossed,
	Message_NoteWillCrossIn500Ms,
	Message_NoteWillCrossIn1000Ms,
	Message_NoteWillCrossIn1500Ms,
	Message_CardRemovedP1,
	Message_CardRemovedP2,
	Message_BeatCrossed,
	Message_MenuUpP1,
	Message_MenuUpP2,
	Message_MenuDownP1,
	Message_MenuDownP2,
	Message_MenuLeftP1,
	Message_MenuLeftP2,
	Message_MenuRightP1,
	Message_MenuRightP2,
	Message_MadeChoiceP1,
	Message_MadeChoiceP2,
	Message_CoinInserted,
	Message_SideJoinedP1,
	Message_SideJoinedP2,
	Message_PlayersFinalized,
	Message_AssistTickChanged,
	Message_AutosyncChanged,
	Message_PreferredSongGroupChanged,
	Message_PreferredCourseGroupChanged,
	Message_SortOrderChanged,
	Message_LessonYourTurn,
	NUM_Message,	// leave this at the end
	Message_Invalud
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
	/* This is only intended to be used for setting temporary values; always
	 * restore the original value when finished, so listeners don't get confused
	 * due to missing a message. */
	void SetWithoutBroadcast( T* t ) { val = t; }
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
