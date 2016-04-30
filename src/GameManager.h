/** @brief GameManager - Manages Games and Styles. */

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

class Style;
struct Game;
struct lua_State;

#include "GameConstantsAndTypes.h"
#include "GameInput.h"

/** @brief The collective information about a Steps' Type. */
struct StepsTypeInfo
{
	std::string stepTypeName;
	/** @brief The number of tracks, or columns, of this type. */
	int iNumTracks;
	/** @brief A flag to determine if we allow this type to be autogen'ed to other types. */
	bool bAllowAutogen;
	/** @brief The most basic StyleType that this StpesTypeInfo is used with. */
	StepsTypeCategory m_StepsTypeCategory;
	std::string GetLocalizedString() const;
};

class GameManager
{
public:
	GameManager();
	~GameManager();

	void GetStylesForGame( const Game* pGame, std::vector<const Style*>& aStylesAddTo, bool editor=false );
	const Game *GetGameForStyle( const Style *pStyle );
	void GetStepsTypesForGame( const Game* pGame, std::vector<StepsType>& aStepsTypeAddTo );
	const Style *GetEditorStyleForStepsType( StepsType st );
	void GetDemonstrationStylesForGame( const Game *pGame, std::vector<const Style*> &vpStylesOut );
	const Style *GetHowToPlayStyleForGame( const Game* pGame );
	void GetCompatibleStyles( const Game *pGame, int iNumPlayers, std::vector<const Style*> &vpStylesOut );
	const Style *GetFirstCompatibleStyle( const Game *pGame, int iNumPlayers, StepsType st );

	void GetEnabledGames( std::vector<const Game*>& aGamesOut );
	const Game* GetDefaultGame();
	bool IsGameEnabled( const Game* pGame );
	int GetIndexFromGame( const Game* pGame );
	const Game* GetGameFromIndex( int index );

	const StepsTypeInfo &GetStepsTypeInfo( StepsType st );
	StepsType StringToStepsType( std::string sStepsType );
	const Game* StringToGame( std::string sGame );
	const Style* GameAndStringToStyle( const Game* pGame, std::string sStyle );
	std::string StyleToLocalizedString( const Style* s );

	// stepstype_is_multiplayer doesn't really belong in GAMEMAN, but we don't
	// have a stepstype manager.  Also, I want to start calling routine mode
	// multiplayer, something to make it more explicitly about having multiple
	// players on a single notefield. -Kyz
	bool stepstype_is_multiplayer(StepsType st);
	// Some mods need to know the number of pads being used.
	int get_num_pads_for_stepstype(StepsType st);


	// Lua
	void PushSelf( lua_State *L );
};

extern GameManager*	GAMEMAN;	// global and accessible from anywhere in our program

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
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
