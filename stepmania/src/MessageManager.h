/* MessageManager - Control lights. */

#ifndef MessageManager_H
#define MessageManager_H

#include "LuaManager.h"
struct lua_State;
class LuaTable;
class LuaReference;

enum MessageID
{
	Message_CurrentGameChanged,
	Message_CurrentStyleChanged,
	Message_PlayModeChanged,
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
	Message_MenuSelectionChanged,
	Message_CoinInserted,
	Message_SideJoinedP1,
	Message_SideJoinedP2,
	Message_PlayersFinalized,
	Message_AssistTickChanged,
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
	Message_ShowJudgmentMuliPlayerP1,
	Message_ShowJudgmentMuliPlayerP2,
	Message_ShowJudgmentMuliPlayerP3,
	Message_ShowJudgmentMuliPlayerP4,
	Message_ShowJudgmentMuliPlayerP5,
	Message_ShowJudgmentMuliPlayerP6,
	Message_ShowJudgmentMuliPlayerP7,
	Message_ShowJudgmentMuliPlayerP8,
	Message_ShowJudgmentMuliPlayerP9,
	Message_ShowJudgmentMuliPlayerP10,
	Message_ShowJudgmentMuliPlayerP11,
	Message_ShowJudgmentMuliPlayerP12,
	Message_ShowJudgmentMuliPlayerP13,
	Message_ShowJudgmentMuliPlayerP14,
	Message_ShowJudgmentMuliPlayerP15,
	Message_ShowJudgmentMuliPlayerP16,
	Message_ShowJudgmentMuliPlayerP17,
	Message_ShowJudgmentMuliPlayerP18,
	Message_ShowJudgmentMuliPlayerP19,
	Message_ShowJudgmentMuliPlayerP20,
	Message_ShowJudgmentMuliPlayerP21,
	Message_ShowJudgmentMuliPlayerP22,
	Message_ShowJudgmentMuliPlayerP23,
	Message_ShowJudgmentMuliPlayerP24,
	Message_ShowJudgmentMuliPlayerP25,
	Message_ShowJudgmentMuliPlayerP26,
	Message_ShowJudgmentMuliPlayerP27,
	Message_ShowJudgmentMuliPlayerP28,
	Message_ShowJudgmentMuliPlayerP29,
	Message_ShowJudgmentMuliPlayerP30,
	Message_ShowJudgmentMuliPlayerP31,
	Message_ShowJudgmentMuliPlayerP32,
	Message_ShowHoldJudgmentMuliPlayerP1,
	Message_ShowHoldJudgmentMuliPlayerP2,
	Message_ShowHoldJudgmentMuliPlayerP3,
	Message_ShowHoldJudgmentMuliPlayerP4,
	Message_ShowHoldJudgmentMuliPlayerP5,
	Message_ShowHoldJudgmentMuliPlayerP6,
	Message_ShowHoldJudgmentMuliPlayerP7,
	Message_ShowHoldJudgmentMuliPlayerP8,
	Message_ShowHoldJudgmentMuliPlayerP9,
	Message_ShowHoldJudgmentMuliPlayerP10,
	Message_ShowHoldJudgmentMuliPlayerP11,
	Message_ShowHoldJudgmentMuliPlayerP12,
	Message_ShowHoldJudgmentMuliPlayerP13,
	Message_ShowHoldJudgmentMuliPlayerP14,
	Message_ShowHoldJudgmentMuliPlayerP15,
	Message_ShowHoldJudgmentMuliPlayerP16,
	Message_ShowHoldJudgmentMuliPlayerP17,
	Message_ShowHoldJudgmentMuliPlayerP18,
	Message_ShowHoldJudgmentMuliPlayerP19,
	Message_ShowHoldJudgmentMuliPlayerP20,
	Message_ShowHoldJudgmentMuliPlayerP21,
	Message_ShowHoldJudgmentMuliPlayerP22,
	Message_ShowHoldJudgmentMuliPlayerP23,
	Message_ShowHoldJudgmentMuliPlayerP24,
	Message_ShowHoldJudgmentMuliPlayerP25,
	Message_ShowHoldJudgmentMuliPlayerP26,
	Message_ShowHoldJudgmentMuliPlayerP27,
	Message_ShowHoldJudgmentMuliPlayerP28,
	Message_ShowHoldJudgmentMuliPlayerP29,
	Message_ShowHoldJudgmentMuliPlayerP30,
	Message_ShowHoldJudgmentMuliPlayerP31,
	Message_ShowHoldJudgmentMuliPlayerP32,
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
	Message_ScoreChangedP1,
	Message_ScoreChangedP2,
	NUM_MessageID,	// leave this at the end
	MessageID_Invalid
};
const RString& MessageIDToString( MessageID m );

struct Message
{
	explicit Message( const RString &s );
	~Message();
	RString GetName() const { return m_sName; }

	void PushParamTable( lua_State *L );
	const LuaReference &GetParamTable() const;

	void SetParamFromStack( lua_State *L, const RString &sName );

	template<typename T>
	void SetParam( const RString &sName, const T &val )
	{
		Lua *L = LUA->Get();
		LuaHelpers::Push( L, val );
		SetParamFromStack( L, sName );
		LUA->Release( L );
	}

	bool operator==( const RString &s ) const { return m_sName == s; }
	bool operator==( MessageID id ) const { return MessageIDToString(id) == m_sName; }

private:
	RString m_sName;
	LuaTable *m_pParams;
};

class IMessageSubscriber
{
public:
	virtual ~IMessageSubscriber() { }
	virtual void HandleMessage( const Message &msg ) = 0;
	virtual void ProcessMessages( float fDeltaTime );
	void ClearMessages( const RString sMessage = "" );

private:
	friend class MessageManager;
};

class MessageSubscriber : public IMessageSubscriber
{
public:
	MessageSubscriber() {}
	MessageSubscriber( const MessageSubscriber &cpy );

	//
	// Messages
	//
	void SubscribeToMessage( MessageID message ); // will automatically unsubscribe
	void SubscribeToMessage( const RString &sMessageName ); // will automatically unsubscribe

	void UnsubscribeAll();

private:
	vector<RString> m_vsSubscribedTo;
};


class MessageManager
{
public:
	MessageManager();
	~MessageManager();

	void Subscribe( IMessageSubscriber* pSubscriber, const RString& sMessage );
	void Subscribe( IMessageSubscriber* pSubscriber, MessageID m );
	void Unsubscribe( IMessageSubscriber* pSubscriber, const RString& sMessage );
	void Unsubscribe( IMessageSubscriber* pSubscriber, MessageID m );
	void Broadcast( const Message &msg ) const;
	void Broadcast( const RString& sMessage ) const;
	void Broadcast( MessageID m ) const;

	// Lua
	void PushSelf( lua_State *L );
};

extern MessageManager*	MESSAGEMAN;	// global and accessable from anywhere in our program

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

namespace LuaHelpers { template<class T> void Push( lua_State *L, const BroadcastOnChange<T> &Object ) { LuaHelpers::Push<T>( L, Object.Get() ); } }

template<class T, int N>
class BroadcastOnChange1D
{
private:
	typedef BroadcastOnChange<T> MyType;
	vector<MyType> val;
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
	explicit BroadcastOnChangePtr( MessageID m ) { mSendWhenChanged = m; val = NULL; }
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
	vector<MyType> val;
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
