#ifndef NOTE_SKIN_MANAGER_H
#define NOTE_SKIN_MANAGER_H

#include "Actor.h"
#include "RageTypes.h"
#include "PlayerNumber.h"
#include "GameInput.h"
#include "IniFile.h"

struct Game;
struct NoteSkinData;

/** @brief Loads note skins. */
class NoteSkinManager
{
public:
	NoteSkinManager();
	~NoteSkinManager();

	void RefreshNoteSkinData( const Game* game );
	void GetNoteSkinNames( const Game* game, std::vector<std::string> &AddTo );
	void GetNoteSkinNames( std::vector<std::string> &AddTo );	// looks up current const Game* in GAMESTATE
	bool NoteSkinNameInList(std::string const name, std::vector<std::string> name_list);
	bool DoesNoteSkinExist( const std::string &sNoteSkin );	// looks up current const Game* in GAMESTATE
	bool DoNoteSkinsExistForGame( const Game *pGame );
	std::string GetDefaultNoteSkinName();	// looks up current const Game* in GAMESTATE

	void ValidateNoteSkinName(std::string& name);

	void SetCurrentNoteSkin( const std::string &sNoteSkin ) { m_sCurrentNoteSkin = sNoteSkin; }
	const std::string &GetCurrentNoteSkin() { return m_sCurrentNoteSkin; }
	void SetPlayerNumber( PlayerNumber pn ) { m_PlayerNumber = pn; }
	void SetGameController( GameController gc ) { m_GameController = gc; }
	std::string GetPath( const std::string &sButtonName, const std::string &sElement );
	bool PushActorTemplate( Lua *L, const std::string &sButton, const std::string &sElement, bool bSpriteOnly );
	Actor *LoadActor( const std::string &sButton, const std::string &sElement, Actor *pParent = nullptr, bool bSpriteOnly = false );

	std::string		GetMetric( const std::string &sButtonName, const std::string &sValue );
	int		GetMetricI( const std::string &sButtonName, const std::string &sValueName );
	float		GetMetricF( const std::string &sButtonName, const std::string &sValueName );
	bool		GetMetricB( const std::string &sButtonName, const std::string &sValueName );
	apActorCommands	GetMetricA( const std::string &sButtonName, const std::string &sValueName );

	// Lua
	void PushSelf( lua_State *L );

protected:
	std::string GetPathFromDirAndFile( const std::string &sDir, const std::string &sFileName );
	void GetAllNoteSkinNamesForGame( const Game *pGame, std::vector<std::string> &AddTo );

	bool LoadNoteSkinData( const std::string &sNoteSkinName, NoteSkinData& data_out );
	bool LoadNoteSkinDataRecursive( const std::string &sNoteSkinName, NoteSkinData& data_out );
	std::string m_sCurrentNoteSkin;
	const Game* m_pCurGame;

	// xxx: is this the best way to implement this? -freem
	PlayerNumber m_PlayerNumber;
	GameController m_GameController;
};

extern NoteSkinManager*	NOTESKIN;	// global and accessible from anywhere in our program

class LockNoteSkin
{
public:
	LockNoteSkin( const std::string &sNoteSkin ) { ASSERT( NOTESKIN->GetCurrentNoteSkin().empty() ); NOTESKIN->SetCurrentNoteSkin( sNoteSkin ); }
	~LockNoteSkin() { NOTESKIN->SetCurrentNoteSkin( std::string() ); }
};


#endif

/**
 * @file
 * @author Chris Danford (c) 2003-2004
 * @section LICENSE
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
