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


const int NUM_GAME_DEFS = 1;


class GameManager
{
public:
	GameManager();

	void SwitchGame( CString sGame )
	{
		m_sCurrentGame = sGame;
		m_pCurrentGameDef = GetGameDef( sGame );
		ASSERT( m_pCurrentGameDef != NULL );
	};
	void SwitchStyle( CString sStyle )
	{
		m_sCurrentStyle = sStyle;
		m_pCurrentStyleDef = GetStyleDef( m_sCurrentGame, sStyle );
		ASSERT( m_pCurrentStyleDef != NULL );
	};

	CString		m_sCurrentGame;		// currently only "dance"
	CString		m_sCurrentStyle;	// currently only "single", "versus", "double", "couple", "solo"
	inline GameDef*		GetCurrentGameDef()	{ return m_pCurrentGameDef; };
	inline StyleDef*	GetCurrentStyleDef() { return m_pCurrentStyleDef; };

	bool IsPlayerEnabled( PlayerNumber PlayerNo );

	// graphic stuff
	CString GetPathToGraphic( const PlayerNumber p, const NoteColumn col, const GameButtonGraphic gbg )
	{
		StyleInput si( p, col );
		GameInput gi = GetCurrentStyleDef()->StyleInputToGameInput( si );
		InstrumentButton b = gi.button;
		return GetCurrentGameDef()->GetPathToGraphic( b, gbg );
	}


protected:
	GameDef*	m_pCurrentGameDef;
	StyleDef*	m_pCurrentStyleDef;

	GameDef*	GetGameDef( CString sGame );
	StyleDef*	GetStyleDef( CString sGame, CString sStyle );
};


extern GameManager*	GAME;	// global and accessable from anywhere in our program
