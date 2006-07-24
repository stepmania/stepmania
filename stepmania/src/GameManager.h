/* GameManager - Manages Games and Styles. */

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

class Style;
class Game;
struct lua_State;

#include "GameConstantsAndTypes.h"
#include "GameInput.h"
#include "MenuInput.h"

class GameManager
{
public:
	GameManager();
	~GameManager();

	void	GetStylesForGame( const Game* pGame, vector<const Style*>& aStylesAddTo, bool editor=false ) const;
	void	GetStepsTypesForGame( const Game* pGame, vector<StepsType>& aStepsTypeAddTo ) const;
	const Style*	GetEditorStyleForStepsType( StepsType st ) const;
	void GetDemonstrationStylesForGame( const Game *pGame, vector<const Style*> &vpStylesOut ) const;
	const Style*	GetHowToPlayStyleForGame( const Game* pGame ) const;

	void GetEnabledGames( vector<const Game*>& aGamesOut ) const;
	const Game* GetDefaultGame() const;
	bool IsGameEnabled( const Game* pGame ) const;
	int GetIndexFromGame( const Game* pGame ) const;
	const Game* GetGameFromIndex( int index ) const;

	static int StepsTypeToNumTracks( StepsType st );
	static bool CanAutoGenStepsType( StepsType st );
	static StepsType StringToStepsType( RString sStepsType );
	static RString StepsTypeToString( StepsType st );
	static RString StepsTypeToLocalizedString( StepsType st );
	static const Game* StringToGameType( RString sGameType );
	const Style* GameAndStringToStyle( const Game* pGame, RString sStyle );
	static RString StyleToLocalizedString( const Style* s );
	MenuButton GetMenuButtonSecondaryFunction( const Game *pGame, GameButton gb ) const;

	// Lua
	void PushSelf( lua_State *L );
};

extern GameManager*	GAMEMAN;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
