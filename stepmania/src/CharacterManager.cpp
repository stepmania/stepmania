#include "global.h"
#include "CharacterManager.h"
#include "Character.h"
#include "GameState.h"
#include "Foreach.h"
#include "LuaManager.h"

#define CHARACTERS_DIR "/Characters/"

CharacterManager*	CHARMAN = NULL;	// global object accessable from anywhere in the program

CharacterManager::CharacterManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "CHARMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}

	for( unsigned i=0; i<m_pCharacters.size(); i++ )
		SAFE_DELETE( m_pCharacters[i] );
	m_pCharacters.clear();

	vector<RString> as;
	GetDirListing( CHARACTERS_DIR "*", as, true, true );
	StripCvs( as );

	bool FoundDefault = false;
	for( unsigned i=0; i<as.size(); i++ )
	{
		RString sCharName, sDummy;
		splitpath(as[i], sDummy, sCharName, sDummy);
		sCharName.MakeLower();

		if( sCharName.CompareNoCase("default")==0 )
			FoundDefault = true;

		Character* pChar = new Character;
		if( pChar->Load( as[i] ) )
			m_pCharacters.push_back( pChar );
		else
			delete pChar;
	}
	
	if( !FoundDefault )
		RageException::Throw( "'Characters/default' is missing." );

	// If FoundDefault, then we're not empty. -Chris
//	if( m_pCharacters.empty() )
//		RageException::Throw( "Couldn't find any character definitions" );
}

CharacterManager::~CharacterManager()
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
		SAFE_DELETE( m_pCharacters[i] );

	// Unregister with Lua.
	LUA->UnsetGlobal( "CHARMAN" );
}

void CharacterManager::GetCharacters( vector<Character*> &apCharactersOut )
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
		if( !m_pCharacters[i]->IsDefaultCharacter() )
			apCharactersOut.push_back( m_pCharacters[i] );
}

Character* CharacterManager::GetRandomCharacter()
{
	vector<Character*> apCharacters;
	GetCharacters( apCharacters );
	if( apCharacters.size() )
		return apCharacters[RandomInt(apCharacters.size())];
	else
		return GetDefaultCharacter();
}

Character* CharacterManager::GetDefaultCharacter()
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
	{
		if( m_pCharacters[i]->IsDefaultCharacter() )
			return m_pCharacters[i];
	}

	/* We always have the default character. */
	ASSERT(0);
	return NULL;
}

void CharacterManager::DemandGraphics()
{
	FOREACH( Character*, m_pCharacters, c )
		(*c)->DemandGraphics();
}

void CharacterManager::UndemandGraphics()
{
	FOREACH( Character*, m_pCharacters, c )
		(*c)->UndemandGraphics();
}

Character* CharacterManager::GetCharacterFromID( RString sCharacterID )
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
	{
		if( m_pCharacters[i]->m_sCharacterID == sCharacterID )
			return m_pCharacters[i];
	}

	return NULL;
}


// lua start
#include "LuaBinding.h"

class LunaCharacterManager: public Luna<CharacterManager>
{
public:
	static int GetCharacter( T* p, lua_State *L )
	{
		Character *pCharacter = p->GetCharacterFromID(SArg(1));
		if( pCharacter != NULL )
			pCharacter->PushSelf( L );
		else
			lua_pushnil( L );

		return 1;
	}

	LunaCharacterManager()
	{
		LUA->Register( Register );

		ADD_METHOD( GetCharacter );
	}
};

LUA_REGISTER_CLASS( CharacterManager )
// lua end


/*
 * (c) 2001-2004 Chris Danford
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
