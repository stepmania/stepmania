#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameManager

 Desc: Manages GameDefs (which define different games, like "dance" and "pump")
	and StyleDefs (which define different games, like "single" and "couple")

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
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
	~GameManager();

	void ReadGamesAndStylesFromDir( CString sDir );
	void SwitchGame( CString sGame );
	void SwitchStyle( CString sStyle );
	void SwitchSkin( PlayerNumber p, CString sSkin );

	CString				m_sCurrentGame;		// currently only "dance"
	CString				m_sCurrentStyle;	// currently only "single", "versus", "double", "couple", "solo"
	inline GameDef*		GetCurrentGameDef()	{ return m_pCurrentGameDef; };
	inline StyleDef*	GetCurrentStyleDef() { return m_pCurrentStyleDef; };
	CString				m_sCurrentSkin[NUM_PLAYERS];		// 

	void GetGameNames( CStringArray &arrayGameNames );
	void GetStyleNames( CString sGameName, CStringArray &arrayStyleNames );
	void GetSkinNames( CString sGameName, CStringArray &arraySkinNames );
	void GetSkinNames( CStringArray &arraySkinNames ) { GetSkinNames( m_sCurrentGame, arraySkinNames ); };

	bool IsPlayerEnabled( PlayerNumber PlayerNo );

	// graphic stuff
	CString GetPathToGraphic( const PlayerNumber p, const ColumnNumber col, const GameButtonGraphic gbg )
	{
		StyleInput si( p, col );
		GameInput gi = GetCurrentStyleDef()->StyleInputToGameInput( si );
		InstrumentButton b = gi.button;
		return GetCurrentGameDef()->GetPathToGraphic( m_sCurrentSkin[p], b, gbg );
	}


protected:
	int			m_iNumGameDefs;
	GameDef*	m_pGameDefs[MAX_GAME_DEFS];

	GameDef*	m_pCurrentGameDef;
	StyleDef*	m_pCurrentStyleDef;

	GameDef*	GetGameDef( CString sGame );
	StyleDef*	GetStyleDef( CString sGame, CString sStyle );
};


extern GameManager*	GAME;	// global and accessable from anywhere in our program
