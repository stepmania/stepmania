#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameDef

 Desc: Holds information about a particular style of a game (e.g. "single", "double").

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Notes.h"
#include "NoteData.h"
#include "GameInput.h"
#include "GameConstantsAndTypes.h"
#include "MenuInput.h"


const int MAX_STYLES_PER_GAME = 10;


enum GameButtonGraphic { 
	GRAPHIC_NOTE_COLOR_PART,
	GRAPHIC_NOTE_GRAY_PART,
	GRAPHIC_RECEPTOR,
	GRAPHIC_TAP_EXPLOSION_BRIGHT,
	GRAPHIC_TAP_EXPLOSION_DIM,
	GRAPHIC_HOLD_EXPLOSION,
	NUM_GAME_BUTTON_GRAPHICS	// leave this at the end
};


class StyleDef;

class GameDef
{
public:
	char	m_szName[60];
	char	m_szDescription[60];
	int		m_iNumInstruments;
	int		m_iButtonsPerInstrument;
	char	m_szButtonNames[60][MAX_INSTRUMENT_BUTTONS];	// e.g. "left", "right", "middle C", "snare"

//	int ButtonNameToIndex( const CString &sButtonName )
//	{
//		for( int i=0; i<m_iButtonsPerInstrument; i++ ) 
//			if( m_sButtonNames[i] == sButtonName )
//				return i;
//		return -1;
//	}

	int m_iMenuButtons[NUM_MENU_BUTTONS];	// map from MenuButton to m_sButtonNames


	// skin stuff
	void GetSkinNames( CStringArray &asSkinNames );
	bool HasASkinNamed( CString sSkin );
	void AssertSkinsAreComplete();

	CString GetPathToGraphic( const CString sSkinName, const int iInstrumentButton, const GameButtonGraphic gbg );
	void	GetTweenColors( const CString sSkinName, const int iInstrumentButton, CArray<D3DXCOLOR,D3DXCOLOR> &arrayTweenColors );
	CString ElementToGraphicSuffix( const GameButtonGraphic gbg );


	inline MenuInput GameInputToMenuInput( const GameInput GameI )
	{
		PlayerNumber p = (PlayerNumber)GameI.number;
		for( int b=0; b<NUM_MENU_BUTTONS; b++ )
		{
			if( m_iMenuButtons[b] == GameI.button )
				return MenuInput( p, (MenuButton)b );
		}
		return MenuInput( PLAYER_INVALID, MENU_BUTTON_INVALID );
	};
	inline GameInput MenuInputToGameInput( const MenuInput MenuI )
	{
		return GameInput( (InstrumentNumber)MenuI.player, (InstrumentButton)m_iMenuButtons[MenuI.button] );
	};

};
