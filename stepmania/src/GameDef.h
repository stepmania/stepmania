#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameDef

 Desc: Holds information about a particular style of a game (e.g. "single", "double").

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "StyleDef.h"
#include "NoteMetadata.h"
#include "NoteData.h"
#include "GameInput.h"
#include "GameTypes.h"
#include "MenuInput.h"



const int MAX_STYLES_PER_GAME = 10;



struct GameDef
{
public:

	LPCTSTR m_szName;
	LPCTSTR m_szDescription;
	LPCTSTR m_szGraphicPath;

	// instrument stuff
	int m_iNumInstruments;
	int m_iNumButtons;
	LPCTSTR m_szButtonNames[NUM_INSTRUMENT_BUTTONS];	// e.g. "left", "right", "middle C", "snare"
	int m_iMenuButtons[NUM_MENU_BUTTONS];	// map from MenuButton to InstrumentButton

	int m_iNumStyleDefs;
	StyleDef m_StyleDefs[MAX_STYLES_PER_GAME];


	// graphic stuff
	CString GetPathToGraphic( const int iInstrumentButton, const GameButtonGraphic gbg );
	CString ElementToGraphicSuffix( const GameButtonGraphic gbg );


	inline MenuInput GameInputToMenuInput( const GameInput GameI )
	{
		PlayerNumber p = (PlayerNumber)GameI.number;
		for( int b=0; b<NUM_MENU_BUTTONS; b++ )
		{
			if( m_iMenuButtons[b] == GameI.button )
				return MenuInput( p, (MenuButton)b );
		}
		return MenuInput( PLAYER_NONE, MENU_BUTTON_NONE );
	};
	inline GameInput MenuInputToGameInput( const MenuInput MenuI )
	{
		return GameInput( (InstrumentNumber)MenuI.player, (InstrumentButton)m_iMenuButtons[MenuI.button] );
	};

};
