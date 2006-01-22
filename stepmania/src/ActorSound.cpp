#include "global.h"
#include "ActorSound.h"
#include "ActorUtil.h"
#include "LuaManager.h"
#include "XmlFile.h"
#include "RageSoundManager.h"
#include "RageUtil.h"

REGISTER_ACTOR_CLASS_WITH_NAME( ActorSound, Sound )

void ActorSound::Load( const RString &sPath )
{
	m_Sound.Load( sPath, true );
}

void ActorSound::Play()
{
	SOUNDMAN->PlayCopyOfSound( m_Sound );
}

void ActorSound::LoadFromNode( const RString& sDir, const XNode* pNode )
{
	Actor::LoadFromNode( sDir, pNode );

	RString sFile;
	if( pNode->GetAttrValue("File", sFile) || pNode->GetAttrValue("Path", sFile) ) /* Path deprecated */
	{
		LuaHelpers::RunAtExpressionS( sFile );
		FixSlashesInPlace( sFile );

		RString sNewPath = sFile.Left(1) == "/"? sFile : sDir+sFile;
		ActorUtil::ResolvePath( sNewPath, sDir );

		Load( sNewPath );
	}
}

// lua start
#include "LuaBinding.h"

class LunaActorSound: public Luna<ActorSound>
{
public:
	LunaActorSound() { LUA->Register( Register ); }

	static int load( T* p, lua_State *L )			{ p->Load(SArg(1)); return 0; }
	static int play( T* p, lua_State *L )			{ p->Play(); return 0; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( load );
		ADD_METHOD( play );

		Luna<T>::Register( L );
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
