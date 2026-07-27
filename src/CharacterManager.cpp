#include "global.h"
#include "CharacterManager.h"
#include "Character.h"
#include "GameState.h"

#include "LuaManager.h"

#include <vector>


#define CHARACTERS_DIR "/Characters/"

CharacterManager*	CHARMAN = nullptr;	// global object accessible from anywhere in the program

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

	std::vector<RString> as;
	GetDirListing( CHARACTERS_DIR "*", as, true, true );
	StripCvsAndSvn( as );
	StripMacResourceForks( as );

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

void CharacterManager::GetCharacters( std::vector<Character*> &apCharactersOut )
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
		if( !m_pCharacters[i]->IsDefaultCharacter() )
			apCharactersOut.push_back( m_pCharacters[i] );
}

Character* CharacterManager::GetRandomCharacter()
{
	std::vector<Character*> apCharacters;
	GetCharacters( apCharacters );
	if( apCharacters.size() )
		return apCharacters[RandomInt(apCharacters.size())];
	else
		return GetDefaultCharacter();
}

Character* CharacterManager::GetDefaultCharacter()
{
	for (Character *c : m_pCharacters)
	{
		if( c->IsDefaultCharacter() )
			return c;
	}

	/* We always have the default character. */
	FAIL_M("There must be a default character available!");
}

void CharacterManager::DemandGraphics()
{
	for (Character *c : m_pCharacters)
		c->DemandGraphics();
}

void CharacterManager::UndemandGraphics()
{
	for (Character *c : m_pCharacters)
		c->UndemandGraphics();
}

Character* CharacterManager::GetCharacterFromID( RString sCharacterID )
{
	for( unsigned i=0; i<m_pCharacters.size(); i++ )
	{
		if( m_pCharacters[i]->m_sCharacterID == sCharacterID )
			return m_pCharacters[i];
	}

	return nullptr;
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the CharacterManager. */
class LunaCharacterManager: public Luna<CharacterManager>
{
public:
	static int GetCharacter( T* p, lua_State *L )
	{
		Character *pCharacter = p->GetCharacterFromID(SArg(1));
		if( pCharacter != nullptr )
			pCharacter->PushSelf( L );
		else
			lua_pushnil( L );

		return 1;
	}
	static int GetRandomCharacter( T* p, lua_State *L )
	{
		Character *pCharacter = p->GetRandomCharacter();
		if( pCharacter != nullptr )
			pCharacter->PushSelf( L );
		else
			lua_pushnil( L );

		return 1;
	}
	static int GetAllCharacters( T* p, lua_State *L )
	{
		std::vector<Character*> vChars;
		p->GetCharacters(vChars);

		LuaHelpers::CreateTableFromArray(vChars, L);
		return 1;
	}
	static int GetCharacterCount(T* p, lua_State *L)
	{
		std::vector<Character*> chars;
		p->GetCharacters(chars);
		lua_pushnumber(L, chars.size());
		return 1;
	}

	LunaCharacterManager()
	{
		ADD_METHOD( GetCharacter );
		// sm-ssc adds:
		ADD_METHOD( GetRandomCharacter );
		ADD_METHOD( GetAllCharacters );
		ADD_METHOD( GetCharacterCount );
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
