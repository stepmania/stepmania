#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameDef

 Desc: Holds information about a particular style of a game (e.g. "single", "double").

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "NoteMetadata.h"
#include "NoteData.h"
#include "GameInput.h"
#include "GameTypes.h"
#include "MenuInput.h"



const int MAX_STYLES_PER_GAME = 10;
const int MAX_SKIN_PATHS = 16;


class StyleDef;

class GameDef
{
public:
	GameDef( CString sGameDir );
	~GameDef();

	CString m_sGameDir;
	CString m_sName;
	CString m_sDescription;

	// instrument stuff
	int m_iNumInstruments;
	int m_iButtonsPerInstrument;
	CString m_sButtonNames[MAX_INSTRUMENT_BUTTONS];	// e.g. "left", "right", "middle C", "snare"
	int ButtonNameToIndex( const CString &sButtonName )
	{
		for( int i=0; i<m_iButtonsPerInstrument; i++ ) 
			if( m_sButtonNames[i] == sButtonName )
				return i;
		return -1;
	}
	int m_iMenuButtons[NUM_MENU_BUTTONS];	// map from MenuButton to m_sButtonNames

	int m_iNumStyleDefs;
	StyleDef* m_pStyleDefs[MAX_STYLES_PER_GAME];
	StyleDef* GetStyleDef( CString sStyle );


	// graphic stuff
	int		m_iNumSkinFolders;
	CString m_sSkinFolders[MAX_SKIN_PATHS];		// path to skin folders relative to m_sGameDir
	bool HasASkinNamed( CString sSkin )
	{
		for( int i=0; i<m_iNumSkinFolders; i++ )
			if( m_sSkinFolders[i] == sSkin )
				return true;

		return false;
	}
	CString GetPathToGraphic( const CString sSkinName, const int iInstrumentButton, const GameButtonGraphic gbg );
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
