#ifndef NOTE_SKIN_MANAGER_H
#define NOTE_SKIN_MANAGER_H

#include "Actor.h"
#include "RageTypes.h"
#include "PlayerNumber.h"
#include "GameInput.h"
#include "IniFile.h"

#include <vector>


struct Game;
struct NoteSkinData;

/** @brief Loads note skins. */
class NoteSkinManager
{
public:
	NoteSkinManager();
	~NoteSkinManager();

	void RefreshNoteSkinData( const Game* game );
	void GetNoteSkinNames( const Game* game, std::vector<RString> &AddTo );
	void GetNoteSkinNames( std::vector<RString> &AddTo );	// looks up current const Game* in GAMESTATE
	bool NoteSkinNameInList(const RString name, std::vector<RString> name_list);
	bool DoesNoteSkinExist( const RString &sNoteSkin );	// looks up current const Game* in GAMESTATE
	bool DoNoteSkinsExistForGame( const Game *pGame );
	RString GetDefaultNoteSkinName();	// looks up current const Game* in GAMESTATE

	void ValidateNoteSkinName(RString& name);

	void SetCurrentNoteSkin( const RString &sNoteSkin ) { m_sCurrentNoteSkin = sNoteSkin; }
	const RString &GetCurrentNoteSkin() { return m_sCurrentNoteSkin; }
	void SetPlayerNumber( PlayerNumber pn ) { m_PlayerNumber = pn; }
	void SetGameController( GameController gc ) { m_GameController = gc; }
	RString GetPath( const RString &sButtonName, const RString &sElement );
	bool PushActorTemplate( Lua *L, const RString &sButton, const RString &sElement, bool bSpriteOnly );
	Actor *LoadActor( const RString &sButton, const RString &sElement, Actor *pParent = nullptr, bool bSpriteOnly = false );

	RString		GetMetric( const RString &sButtonName, const RString &sValue );
	int		GetMetricI( const RString &sButtonName, const RString &sValueName );
	float		GetMetricF( const RString &sButtonName, const RString &sValueName );
	bool		GetMetricB( const RString &sButtonName, const RString &sValueName );
	apActorCommands	GetMetricA( const RString &sButtonName, const RString &sValueName );

	// Lua
	void PushSelf( lua_State *L );

protected:
	RString GetPathFromDirAndFile( const RString &sDir, const RString &sFileName );
	void GetAllNoteSkinNamesForGame( const Game *pGame, std::vector<RString> &AddTo );

	bool LoadNoteSkinData( const RString &sNoteSkinName, NoteSkinData& data_out );
	bool LoadNoteSkinDataRecursive( const RString &sNoteSkinName, NoteSkinData& data_out );
	RString m_sCurrentNoteSkin;
	const Game* m_pCurGame;

	// xxx: is this the best way to implement this? -freem
	PlayerNumber m_PlayerNumber;
	GameController m_GameController;
};

extern NoteSkinManager*	NOTESKIN;	// global and accessible from anywhere in our program

class LockNoteSkin
{
public:
	LockNoteSkin( const RString &sNoteSkin ) { ASSERT( NOTESKIN->GetCurrentNoteSkin().empty() ); NOTESKIN->SetCurrentNoteSkin( sNoteSkin ); }
	~LockNoteSkin() { NOTESKIN->SetCurrentNoteSkin( RString() ); }
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
