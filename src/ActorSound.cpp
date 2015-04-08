#include "global.h"
#include "ActorSound.h"
#include "ActorUtil.h"
#include "LuaManager.h"
#include "XmlFile.h"
#include "RageUtil.h"

REGISTER_ACTOR_CLASS_WITH_NAME( ActorSound, Sound );

void ActorSound::Load( const RString &sPath )
{
	m_Sound.Load( sPath, true );
}

void ActorSound::Play()
{
	m_Sound.Play(m_is_action);
}

void ActorSound::Pause( bool bPause )
{
	m_Sound.Pause(bPause);
}

void ActorSound::Stop()
{
	m_Sound.Stop();
}

void ActorSound::LoadFromNode( const XNode* pNode )
{
	RageSoundLoadParams params;
	pNode->GetAttrValue("SupportPan", params.m_bSupportPan);
	pNode->GetAttrValue("SupportRateChanging", params.m_bSupportRateChanging);
	pNode->GetAttrValue("IsAction", m_is_action);

	bool bPrecache = true;
	pNode->GetAttrValue( "Precache", bPrecache );

	Actor::LoadFromNode( pNode );

	RString sFile;
	if( ActorUtil::GetAttrPath(pNode, "File", sFile) )
		m_Sound.Load( sFile, bPrecache, &params );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ActorSound. */ 
class LunaActorSound: public Luna<ActorSound>
{
public:
	static int load( T* p, lua_State *L )			{ p->Load(SArg(1)); COMMON_RETURN_SELF; }
	static int play( T* p, lua_State *L )			{ p->Play(); COMMON_RETURN_SELF; }
	static int pause( T* p, lua_State *L )			{ p->Pause(BArg(1)); COMMON_RETURN_SELF; }
	static int stop( T* p, lua_State *L )			{ p->Stop(); COMMON_RETURN_SELF; }
	static int get( T* p, lua_State *L )			{ p->PushSound( L ); return 1; }
	static int set_is_action(T* p, lua_State* L)
	{
		p->m_is_action= BArg(1);
		COMMON_RETURN_SELF;
	}
	DEFINE_METHOD(get_is_action, m_is_action);

	LunaActorSound()
	{
		ADD_METHOD( load );
		ADD_METHOD( play );
		ADD_METHOD( pause );
		ADD_METHOD( stop );
		ADD_METHOD( get );
		ADD_GET_SET_METHODS(is_action);
	}
};

LUA_REGISTER_DERIVED_CLASS( ActorSound, Actor )
// lua end


/*
 * (c) 2005 Glenn Maynard
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
