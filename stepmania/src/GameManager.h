#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: GameManager

 Desc: Manages GameDefs (which define different games, like "dance" and "pump")
	and StyleDefs (which define different games, like "single" and "couple")

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "GameDef.h"
#include "StyleDef.h"
#include "Style.h"
#include "Game.h"
class IniFile;


class GameManager
{
public:
	GameManager();
	~GameManager();

	GameDef*	GetGameDefForGame( Game g );
	const StyleDef*	GetStyleDefForStyle( Style s );

	void	GetStylesForGame( Game game, vector<Style>& aStylesAddTo, bool editor=false );
	void	GetNotesTypesForGame( Game game, vector<NotesType>& aNotesTypeAddTo );
	Style	GetEditorStyleForNotesType( NotesType nt );

	void GetEnabledGames( vector<Game>& aGamesOut );

	static int NotesTypeToNumTracks( NotesType nt );
	static NotesType StringToNotesType( CString sNotesType );
	static CString NotesTypeToString( NotesType nt );
	static Game StringToGameType( CString sGameType );
	Style GameAndStringToStyle( Game game, CString sStyle );

protected:
};

extern GameManager*	GAMEMAN;	// global and accessable from anywhere in our program

#endif
