/* GameManager - Manages GameDefs and Styles. */

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

class IniFile;
class Style;
class GameDef;

#include "GameConstantsAndTypes.h"

class GameManager
{
public:
	GameManager();
	~GameManager();

	void	GetStylesForGame( const GameDef* pGame, vector<const Style*>& aStylesAddTo, bool editor=false );
	void	GetAllStyles( vector<const Style*>& aStylesAddTo, bool editor=false );
	void	GetStepsTypesForGame( const GameDef* pGame, vector<StepsType>& aStepsTypeAddTo );
	const Style*	GetEditorStyleForStepsType( StepsType st );
	const Style*	GetDemonstrationStyleForGame( const GameDef* pGame );
	const Style*	GetHowToPlayStyleForGame( const GameDef* pGame );

	void GetEnabledGames( vector<const GameDef*>& aGamesOut );
	const GameDef* GetDefaultGame();
	bool IsGameEnabled( const GameDef* pGame );
	int GetIndexFromGame( const GameDef* pGame );
	const GameDef* GetGameFromIndex( int index );

	static int StepsTypeToNumTracks( StepsType st );
	static StepsType StringToStepsType( CString sStepsType );
	static CString StepsTypeToString( StepsType st );
	static CString StepsTypeToThemedString( StepsType st );
	static const GameDef* StringToGameType( CString sGameType );
	const Style* GameAndStringToStyle( const GameDef* pGame, CString sStyle );
	static CString StyleToThemedString( const Style* s );
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
