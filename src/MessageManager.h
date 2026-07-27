#ifndef MessageManager_H
#define MessageManager_H

#include "LuaManager.h"

#include <vector>


struct lua_State;
class LuaTable;
class LuaReference;

/** @brief The various messages available to watch for. */
enum MessageID
{
	Message_CurrentGameChanged,
	Message_CurrentStyleChanged,
	Message_PlayModeChanged,
	Message_CoinsChanged,
	Message_CurrentSongChanged,
	Message_CurrentStepsP1Changed,
	Message_CurrentStepsP2Changed,
	Message_CurrentCourseChanged,
	Message_CurrentTrailP1Changed,
	Message_CurrentTrailP2Changed,
	Message_GameplayLeadInChanged,
	Message_EditStepsTypeChanged,
	Message_EditCourseDifficultyChanged,
	Message_EditSourceStepsChanged,
	Message_EditSourceStepsTypeChanged,
	Message_PreferredStepsTypeChanged,
	Message_PreferredDifficultyP1Changed,
	Message_PreferredDifficultyP2Changed,
	Message_PreferredCourseDifficultyP1Changed,
	Message_PreferredCourseDifficultyP2Changed,
	Message_EditCourseEntryIndexChanged,
	Message_EditLocalProfileIDChanged,
	Message_GoalCompleteP1,
	Message_GoalCompleteP2,
	Message_NoteCrossed,
	Message_NoteWillCrossIn400Ms,
	Message_NoteWillCrossIn800Ms,
	Message_NoteWillCrossIn1200Ms,
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
	Message_MenuStartP1,
	Message_MenuStartP2,
	Message_MenuSelectionChanged,
	Message_PlayerJoined,
	Message_PlayerUnjoined,
	Message_PlayerProfileSet,
	Message_AutosyncChanged,
	Message_PreferredSongGroupChanged,
	Message_PreferredCourseGroupChanged,
	Message_SortOrderChanged,
	Message_LessonTry1,
	Message_LessonTry2,
	Message_LessonTry3,
	Message_LessonCleared,
	Message_LessonFailed,
	Message_StorageDevicesChanged,
	Message_AutoJoyMappingApplied,
	Message_ScreenChanged,
	Message_SongModified,
	Message_ScoreMultiplierChangedP1,
	Message_ScoreMultiplierChangedP2,
	Message_StarPowerChangedP1,
	Message_StarPowerChangedP2,
	Message_CurrentComboChangedP1,
	Message_CurrentComboChangedP2,
	Message_StarMeterChangedP1,
	Message_StarMeterChangedP2,
	Message_LifeMeterChangedP1,
	Message_LifeMeterChangedP2,
	Message_UpdateScreenHeader,
	Message_LeftClick,
	Message_RightClick,
	Message_MiddleClick,
	Message_MouseWheelUp,
	Message_MouseWheelDown,
	NUM_MessageID,	// leave this at the end
	MessageID_Invalid
};
const RString& MessageIDToString( MessageID m );

struct Message
{
	explicit Message( const RString &s );
	explicit Message(const MessageID id);
	Message( const RString &s, const LuaReference &params );
	~Message();

	void SetName( const RString &sName ) { m_sName = sName; }
	RString GetName() const { return m_sName; }

	bool IsBroadcast() const { return m_bBroadcast; }
	void SetBroadcast( bool b ) { m_bBroadcast = b; }

	void PushParamTable( lua_State *L );
	const LuaReference &GetParamTable() const;
	void SetParamTable( const LuaReference &params );

	void GetParamFromStack( lua_State *L, const RString &sName ) const;
	void SetParamFromStack( lua_State *L, const RString &sName );

	template<typename T>
	bool GetParam( const RString &sName, T &val ) const
	{
		Lua *L = LUA->Get();
		GetParamFromStack( L, sName );
		bool bRet = LuaHelpers::Pop( L, val );
		LUA->Release( L );
		return bRet;
	}

	template<typename T>
	void SetParam( const RString &sName, const T &val )
	{
		Lua *L = LUA->Get();
		LuaHelpers::Push( L, val );
		SetParamFromStack( L, sName );
		LUA->Release( L );
	}

	template<typename T>
	void SetParam( const RString &sName, const std::vector<T> &val )
	{
		Lua *L = LUA->Get();
		LuaHelpers::CreateTableFromArray( val, L );
		SetParamFromStack( L , sName );
		LUA->Release( L );
	}

	bool operator==( const RString &s ) const { return m_sName == s; }
	bool operator==( MessageID id ) const { return MessageIDToString(id) == m_sName; }

private:
	RString m_sName;
	LuaTable *m_pParams;
	bool m_bBroadcast;

	Message &operator=( const Message &rhs ); // don't use
/* Work around a gcc bug where HandleMessage( Message("Init") ) fails because the copy ctor is private.
 * The copy ctor is not even used so I have no idea why it being private is an issue. Also, if the
 * Message object were constructed implicitly (remove explicit above), it works: HandleMessage( "Init" ).
 * Leaving this undefined but public changes a compile time error into a link time error. Hmm.*/
public:
	Message( const Message &rhs ); // don't use
};

class IMessageSubscriber
{
public:
	virtual ~IMessageSubscriber() { }
	virtual void HandleMessage( const Message &msg ) = 0;
	void ClearMessages( const RString sMessage = "" );

private:
	friend class MessageManager;
};

class MessageSubscriber : public IMessageSubscriber
{
public:
	MessageSubscriber(): m_vsSubscribedTo() {}
	MessageSubscriber( const MessageSubscriber &cpy );
	MessageSubscriber &operator=(const MessageSubscriber &cpy);

	//
	// Messages
	//
	void SubscribeToMessage( MessageID message ); // will automatically unsubscribe
	void SubscribeToMessage( const RString &sMessageName ); // will automatically unsubscribe

	void UnsubscribeAll();

private:
	std::vector<RString> m_vsSubscribedTo;
};

/** @brief Deliver messages to any part of the program as needed. */
class MessageManager
{
public:
	MessageManager();
	~MessageManager();

	void Subscribe( IMessageSubscriber* pSubscriber, const RString& sMessage );
	void Subscribe( IMessageSubscriber* pSubscriber, MessageID m );
	void Unsubscribe( IMessageSubscriber* pSubscriber, const RString& sMessage );
	void Unsubscribe( IMessageSubscriber* pSubscriber, MessageID m );
	void Broadcast( Message &msg ) const;
	void Broadcast( const RString& sMessage ) const;
	void Broadcast( MessageID m ) const;
	bool IsSubscribedToMessage( IMessageSubscriber* pSubscriber, const RString &sMessage ) const;
	inline bool IsSubscribedToMessage( IMessageSubscriber* pSubscriber, MessageID message ) const { return IsSubscribedToMessage( pSubscriber, MessageIDToString(message) ); }

	void SetLogging(bool set) { m_Logging= set; }
	bool m_Logging;

	// Lua
	void PushSelf( lua_State *L );
};

extern MessageManager*	MESSAGEMAN;	// global and accessible from anywhere in our program

template<class T>
class BroadcastOnChange
{
private:
	MessageID mSendWhenChanged;
	T val;

public:
	explicit BroadcastOnChange( MessageID m ) { mSendWhenChanged = m; }
	const T Get() const { return val; }
	void Set( T t ) { val = t; MESSAGEMAN->Broadcast( MessageIDToString(mSendWhenChanged) ); }
	operator T () const { return val; }
	bool operator == ( const T &other ) const { return val == other; }
	bool operator != ( const T &other ) const { return val != other; }
};

/** @brief Utilities for working with Lua. */
namespace LuaHelpers
{
	template<class T> void Push( lua_State *L, const BroadcastOnChange<T> &Object )
	{
		LuaHelpers::Push<T>( L, Object.Get() );
	}
}

template<class T, int N>
class BroadcastOnChange1D
{
private:
	typedef BroadcastOnChange<T> MyType;
	std::vector<MyType> val;
public:
	explicit BroadcastOnChange1D( MessageID m )
	{
		for( unsigned i=0; i<N; i++ )
			val.push_back( BroadcastOnChange<T>((MessageID)(m+i)) );
	}
	const BroadcastOnChange<T>& operator[]( unsigned i ) const { return val[i]; }
	BroadcastOnChange<T>& operator[]( unsigned i ) { return val[i]; }
};

template<class T>
class BroadcastOnChangePtr
{
private:
	MessageID mSendWhenChanged;
	T *val;
public:
	explicit BroadcastOnChangePtr( MessageID m ) { mSendWhenChanged = m; val = nullptr; }
	T* Get() const { return val; }
	void Set( T* t ) { val = t; if(MESSAGEMAN) MESSAGEMAN->Broadcast( MessageIDToString(mSendWhenChanged) ); }
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
	std::vector<MyType> val;
public:
	explicit BroadcastOnChangePtr1D( MessageID m )
	{
		for( unsigned i=0; i<N; i++ )
			val.push_back( BroadcastOnChangePtr<T>((MessageID)(m+i)) );
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
