#pragma once
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


const int MAX_GAME_DEFS = 10;


class GameManager
{
public:
	GameManager();

	Style			m_CurStyle;
	NotesType		m_CurNotesType;		// only used in Edit
	CString			m_sCurrentSkin[NUM_PLAYERS];
	PlayerNumber	m_sMasterPlayerNumber;

	Game			GetCurrentGame();	// inferred from m_CurStyle
	GameDef*		GetCurrentGameDef();
	StyleDef*		GetCurrentStyleDef();

	void GetGameNames( CStringArray &AddTo );
	void GetSkinNames( CStringArray &AddTo );
	bool IsPlayerEnabled( PlayerNumber PlayerNo );
	CString GetPathToGraphic( const PlayerNumber p, const int col, const GameButtonGraphic gbg );
	void GetTweenColors( const PlayerNumber p, const int col, CArray<D3DXCOLOR,D3DXCOLOR> &aTweenColorsAddTo );
};

extern GameManager*	GAMEMAN;	// global and accessable from anywhere in our program
