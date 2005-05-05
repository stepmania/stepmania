#include "global.h"
#include "MessageManager.h"
#include "Foreach.h"
#include "Actor.h"
#include "RageUtil.h"
#include "EnumHelper.h"

MessageManager*	MESSAGEMAN = NULL;	// global and accessable from anywhere in our program


static const CString MessageNames[] = {
	"CurrentSongChanged",
	"CurrentStepsP1Changed",
	"CurrentStepsP2Changed",
	"EditStepsTypeChanged",
	"EditSourceStepsChanged",
	"EditSourceStepsTypeChanged",
	"EditPreferredDifficutyP1Changed",
	"EditPreferredDifficutyP2Changed",
	"EditPreferredCourseDifficutyP1Changed",
	"EditPreferredCourseDifficutyP2Changed",
	"GoalCompleteP1",
	"GoalCompleteP2",
	"NoteCrossed",
	"NoteWillCrossIn500Ms",
	"NoteWillCrossIn1000Ms",
	"NoteWillCrossIn1500Ms",
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
	"MadeChoiceP1",
	"MadeChoiceP2",
};
XToString( Message, NUM_MESSAGES );


MessageManager::MessageManager()
{
}

MessageManager::~MessageManager()
{
}

void MessageManager::Subscribe( IMessageSubscriber* pSubscriber, const CString& sMessage )
{
	SubscribersSet& subs = m_MessageToSubscribers[sMessage];
#if _DEBUG
	SubscribersSet::iterator iter = subs.find(pSubscriber);
	ASSERT_M( iter == subs.end(), "already subscribed" );
#endif
	subs.insert( pSubscriber );
}

void MessageManager::Subscribe( IMessageSubscriber* pSubscriber, Message m )
{
	Subscribe( pSubscriber, MessageToString(m) );
}

void MessageManager::Unsubscribe( IMessageSubscriber* pSubscriber, const CString& sMessage )
{
	SubscribersSet& subs = m_MessageToSubscribers[sMessage];
	SubscribersSet::iterator iter = subs.find(pSubscriber);
	ASSERT( iter != subs.end() );
	subs.erase( iter );
}

void MessageManager::Unsubscribe( IMessageSubscriber* pSubscriber, Message m )
{
	Unsubscribe( pSubscriber, MessageToString(m) );
}

void MessageManager::Broadcast( const CString& sMessage ) const
{
	ASSERT( !sMessage.empty() );

	map<CString,SubscribersSet>::const_iterator iter = m_MessageToSubscribers.find( sMessage );
	if( iter == m_MessageToSubscribers.end() )
		return;

	FOREACHS_CONST( IMessageSubscriber*, iter->second, p )
	{
		IMessageSubscriber *pSub = *p;
		pSub->HandleMessage( sMessage );
	}
}

void MessageManager::Broadcast( Message m ) const
{
	Broadcast( MessageToString(m) );
}

// lua start
#include "LuaBinding.h"

template<class T>
class LunaMessageManager: public Luna<T>
{
public:
	LunaMessageManager() { LUA->Register( Register ); }

	static int Broadcast( T* p, lua_State *L )
	{
		p->Broadcast( SArg(1) );
		return 0;
	}

	static void Register(lua_State *L)
	{
		ADD_METHOD( Broadcast )
		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( MESSAGEMAN )
		{
			lua_pushstring(L, "MESSAGEMAN");
			MESSAGEMAN->PushSelf( LUA->L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
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
