#include "global.h"
#include "MessageManager.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "RageThreads.h"
#include "EnumHelper.h"
#include "LuaManager.h"

#include <set>
#include <map>

MessageManager*	MESSAGEMAN = NULL;	// global and accessable from anywhere in our program


static const char *MessageIDNames[] = {
	"CurrentGameChanged",
	"CurrentStyleChanged",
	"PlayModeChanged",
	"CurrentSongChanged",
	"CurrentStepsP1Changed",
	"CurrentStepsP2Changed",
	"CurrentCourseChanged",
	"CurrentTrailP1Changed",
	"CurrentTrailP2Changed",
	"GameplayLeadInChanged",
	"EditStepsTypeChanged",
	"EditCourseDifficultyChanged",
	"EditSourceStepsChanged",
	"EditSourceStepsTypeChanged",
	"PreferredDifficutyP1Changed",
	"PreferredDifficutyP2Changed",
	"PreferredCourseDifficutyP1Changed",
	"PreferredCourseDifficutyP2Changed",
	"EditCourseEntryIndexChanged",
	"EditLocalProfileIDChanged",
	"GoalCompleteP1",
	"GoalCompleteP2",
	"NoteCrossed",
	"NoteWillCrossIn400Ms",
	"NoteWillCrossIn800Ms",
	"NoteWillCrossIn1200Ms",
	"CardRemovedP1",
	"CardRemovedP2",
	"BeatCrossed",
	"MenuUpP1",
	"MenuUpP2",
	"MenuDownP1",
	"MenuDownP2",
	"MenuLeftP1",
	"MenuLeftP2",
	"MenuRightP1",
	"MenuRightP2",
	"MenuSelectionChanged",
	"CoinInserted",
	"SideJoinedP1",
	"SideJoinedP2",
	"PlayersFinalized",
	"AssistTickChanged",
	"AutosyncChanged",
	"PreferredSongGroupChanged",
	"PreferredCourseGroupChanged",
	"SortOrderChanged",
	"LessonTry1",
	"LessonTry2",
	"LessonTry3",
	"LessonCleared",
	"LessonFailed",
	"StorageDevicesChanged",
	"AutoJoyMappingApplied",
	"ScreenChanged",
	"ShowJudgmentMuliPlayerP1",
	"ShowJudgmentMuliPlayerP2",
	"ShowJudgmentMuliPlayerP3",
	"ShowJudgmentMuliPlayerP4",
	"ShowJudgmentMuliPlayerP5",
	"ShowJudgmentMuliPlayerP6",
	"ShowJudgmentMuliPlayerP7",
	"ShowJudgmentMuliPlayerP8",
	"ShowJudgmentMuliPlayerP9",
	"ShowJudgmentMuliPlayerP10",
	"ShowJudgmentMuliPlayerP11",
	"ShowJudgmentMuliPlayerP12",
	"ShowJudgmentMuliPlayerP13",
	"ShowJudgmentMuliPlayerP14",
	"ShowJudgmentMuliPlayerP15",
	"ShowJudgmentMuliPlayerP16",
	"ShowJudgmentMuliPlayerP17",
	"ShowJudgmentMuliPlayerP18",
	"ShowJudgmentMuliPlayerP19",
	"ShowJudgmentMuliPlayerP20",
	"ShowJudgmentMuliPlayerP21",
	"ShowJudgmentMuliPlayerP22",
	"ShowJudgmentMuliPlayerP23",
	"ShowJudgmentMuliPlayerP24",
	"ShowJudgmentMuliPlayerP25",
	"ShowJudgmentMuliPlayerP26",
	"ShowJudgmentMuliPlayerP27",
	"ShowJudgmentMuliPlayerP28",
	"ShowJudgmentMuliPlayerP29",
	"ShowJudgmentMuliPlayerP30",
	"ShowJudgmentMuliPlayerP31",
	"ShowJudgmentMuliPlayerP32",
	"ShowHoldJudgmentMuliPlayerP1",
	"ShowHoldJudgmentMuliPlayerP2",
	"ShowHoldJudgmentMuliPlayerP3",
	"ShowHoldJudgmentMuliPlayerP4",
	"ShowHoldJudgmentMuliPlayerP5",
	"ShowHoldJudgmentMuliPlayerP6",
	"ShowHoldJudgmentMuliPlayerP7",
	"ShowHoldJudgmentMuliPlayerP8",
	"ShowHoldJudgmentMuliPlayerP9",
	"ShowHoldJudgmentMuliPlayerP10",
	"ShowHoldJudgmentMuliPlayerP11",
	"ShowHoldJudgmentMuliPlayerP12",
	"ShowHoldJudgmentMuliPlayerP13",
	"ShowHoldJudgmentMuliPlayerP14",
	"ShowHoldJudgmentMuliPlayerP15",
	"ShowHoldJudgmentMuliPlayerP16",
	"ShowHoldJudgmentMuliPlayerP17",
	"ShowHoldJudgmentMuliPlayerP18",
	"ShowHoldJudgmentMuliPlayerP19",
	"ShowHoldJudgmentMuliPlayerP20",
	"ShowHoldJudgmentMuliPlayerP21",
	"ShowHoldJudgmentMuliPlayerP22",
	"ShowHoldJudgmentMuliPlayerP23",
	"ShowHoldJudgmentMuliPlayerP24",
	"ShowHoldJudgmentMuliPlayerP25",
	"ShowHoldJudgmentMuliPlayerP26",
	"ShowHoldJudgmentMuliPlayerP27",
	"ShowHoldJudgmentMuliPlayerP28",
	"ShowHoldJudgmentMuliPlayerP29",
	"ShowHoldJudgmentMuliPlayerP30",
	"ShowHoldJudgmentMuliPlayerP31",
	"ShowHoldJudgmentMuliPlayerP32",
	"SongModified",
	"ScoreMultiplierChangedP1",
	"ScoreMultiplierChangedP2",
	"StarPowerChangedP1",
	"StarPowerChangedP2",
	"CurrentComboChangedP1",
	"CurrentComboChangedP2",
	"StarMeterChangedP1",
	"StarMeterChangedP2",
	"LifeMeterChangedP1",
	"LifeMeterChangedP2",
	"ScoreChangedP1",
	"ScoreChangedP2",
};
XToString( MessageID );

static RageMutex g_Mutex( "MessageManager" );

typedef set<IMessageSubscriber*> SubscribersSet;
static map<RString,SubscribersSet> g_MessageToSubscribers;

Message::Message( const RString &s )
{
	m_sName = s;
	m_pParams = new LuaTable;
}

Message::~Message()
{
	delete m_pParams;
}

void Message::PushParamTable( lua_State *L )
{
	m_pParams->PushSelf( L );
}

const LuaReference &Message::GetParamTable() const
{
	return *m_pParams;
}

void Message::SetParamFromStack( lua_State *L, const RString &sName )
{
	m_pParams->Set( L, sName );
}

MessageManager::MessageManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "MESSAGEMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

MessageManager::~MessageManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "MESSAGEMAN" );
}

void MessageManager::Subscribe( IMessageSubscriber* pSubscriber, const RString& sMessage )
{
	LockMut(g_Mutex);

	SubscribersSet& subs = g_MessageToSubscribers[sMessage];
#if _DEBUG
	SubscribersSet::iterator iter = subs.find(pSubscriber);
	ASSERT_M( iter == subs.end(), "already subscribed" );
#endif
	subs.insert( pSubscriber );
}

void MessageManager::Subscribe( IMessageSubscriber* pSubscriber, MessageID m )
{
	Subscribe( pSubscriber, MessageIDToString(m) );
}

void MessageManager::Unsubscribe( IMessageSubscriber* pSubscriber, const RString& sMessage )
{
	LockMut(g_Mutex);

	SubscribersSet& subs = g_MessageToSubscribers[sMessage];
	SubscribersSet::iterator iter = subs.find(pSubscriber);
	ASSERT( iter != subs.end() );
	subs.erase( iter );
}

void MessageManager::Unsubscribe( IMessageSubscriber* pSubscriber, MessageID m )
{
	Unsubscribe( pSubscriber, MessageIDToString(m) );
}

void MessageManager::Broadcast( const Message &msg ) const
{
	LockMut(g_Mutex);

	map<RString,SubscribersSet>::const_iterator iter = g_MessageToSubscribers.find( msg.GetName() );
	if( iter == g_MessageToSubscribers.end() )
		return;

	FOREACHS_CONST( IMessageSubscriber*, iter->second, p )
	{
		IMessageSubscriber *pSub = *p;
		pSub->HandleMessage( msg );
	}
}

void MessageManager::Broadcast( const RString& sMessage ) const
{
	ASSERT( !sMessage.empty() );
	Broadcast( Message(sMessage) );
}

void MessageManager::Broadcast( MessageID m ) const
{
	Broadcast( MessageIDToString(m) );
}

void IMessageSubscriber::ClearMessages( const RString sMessage )
{
}

MessageSubscriber::MessageSubscriber( const MessageSubscriber &cpy ):
	IMessageSubscriber(cpy)
{
	FOREACH_CONST( RString, cpy.m_vsSubscribedTo, msg )
		this->SubscribeToMessage( *msg );
}

void MessageSubscriber::SubscribeToMessage( const RString &sMessageName )
{
	MESSAGEMAN->Subscribe( this, sMessageName );
	m_vsSubscribedTo.push_back( sMessageName );
}

void MessageSubscriber::SubscribeToMessage( MessageID message )
{
	MESSAGEMAN->Subscribe( this, message );
	m_vsSubscribedTo.push_back( MessageIDToString(message) );
}

void MessageSubscriber::UnsubscribeAll()
{
	FOREACH_CONST( RString, m_vsSubscribedTo, s )
		MESSAGEMAN->Unsubscribe( this, *s );
	m_vsSubscribedTo.clear();
}


// lua start
#include "LuaBinding.h"

class LunaMessageManager: public Luna<MessageManager>
{
public:
	static int Broadcast( T* p, lua_State *L )
	{
		if( !lua_istable(L, 2) && !lua_isnoneornil(L, 2) )
			luaL_typerror( L, 2, "table or nil" );

		LuaReference ParamTable;
		lua_pushvalue( L, 2 );
		ParamTable.SetFromStack( L );

		p->Broadcast( SArg(1) );
		return 0;
	}

	LunaMessageManager()
	{
		ADD_METHOD( Broadcast );
	}
};

LUA_REGISTER_CLASS( MessageManager )
// lua end

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
