/* GameManager - Manages GameDefs and StyleDefs. */

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "GameDef.h"
#include "StyleDef.h"
#include "Game.h"
class IniFile;


class GameManager
{
public:
	GameManager();
	~GameManager();

	GameDef*	GetGameDefForGame( Game g );
	
	void	GetStylesForGame( Game game, vector<const StyleDef*>& aStylesAddTo, bool editor=false );
	void	GetAllStyles( vector<const StyleDef*>& aStylesAddTo, bool editor=false );
	void	GetNotesTypesForGame( Game game, vector<StepsType>& aNotesTypeAddTo );
	const StyleDef*	GetEditorStyleForNotesType( StepsType st );
	const StyleDef*	GetDemonstrationStyleForGame( Game game );
	const StyleDef*	GetHowToPlayStyleForGame( Game game );

	void GetEnabledGames( vector<Game>& aGamesOut );
	bool IsGameEnabled( Game game );

	static int NotesTypeToNumTracks( StepsType st );
	static StepsType StringToNotesType( CString sNotesType );
	static CString NotesTypeToString( StepsType st );
	static CString NotesTypeToThemedString( StepsType st );
	static Game StringToGameType( CString sGameType );
	const StyleDef* GameAndStringToStyle( Game game, CString sStyle );
	static CString StyleToThemedString( const StyleDef* s );
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
