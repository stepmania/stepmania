#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GameManager

 Desc: See Header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameManager.h"


GameManager*	GAME = NULL;	// global and accessable from anywhere in our program

#define DANCE_PAD_BUTTON_LEFT		INSTRUMENT_BUTTON_1
#define DANCE_PAD_BUTTON_RIGHT		INSTRUMENT_BUTTON_2
#define DANCE_PAD_BUTTON_UP			INSTRUMENT_BUTTON_3
#define DANCE_PAD_BUTTON_DOWN		INSTRUMENT_BUTTON_4
#define DANCE_PAD_BUTTON_UPLEFT		INSTRUMENT_BUTTON_5
#define DANCE_PAD_BUTTON_UPRIGHT	INSTRUMENT_BUTTON_6
#define DANCE_PAD_BUTTON_NEXT		INSTRUMENT_BUTTON_7
#define DANCE_PAD_BUTTON_BACK		INSTRUMENT_BUTTON_8
#define NUM_DANCE_PAD_BUTTONS		8


//
// ----------------------------------
//   This is where all Games and Styles are definied.  
//   If you want to add support for new games or styles, do it here.
// ----------------------------------
//
GameDef g_GameDefs[] = 
{
	{
		"dance",					// m_szName
		"Dance Dance Revolution",	// m_szDescription
		"",							// m_szGraphicPath

		2,							// m_iNumInstruments
		NUM_DANCE_PAD_BUTTONS,		// m_iNumButtons
		{ "Left", "Right", "Up", "Down", "UpLeft", "UpRight", "Next", "Back" },	// m_sButtonNames[NUM_INSTRUMENT_BUTTONS]
		{	// m_iMenuButtons
			DANCE_PAD_BUTTON_LEFT,
			DANCE_PAD_BUTTON_RIGHT,
			DANCE_PAD_BUTTON_UP,
			DANCE_PAD_BUTTON_DOWN,
			DANCE_PAD_BUTTON_NEXT,
			DANCE_PAD_BUTTON_BACK,
		},
		5,							// m_iNumStyleDefs
		{ // StyleDef list
			{	// StyleDef
				"single",	// m_szName
				"single",	// m_szReads
				1,			// m_iNumPlayers
				4,			// m_iNumTracks
				{	// m_StyleInputToGameInput[NUM_PLAYERS][NUM_NOTE_COLS];
					{	// player 1
						{ 0, INSTRUMENT_1, DANCE_PAD_BUTTON_LEFT },		// column 1
						{ 1, INSTRUMENT_1, DANCE_PAD_BUTTON_DOWN },		// column 2
						{ 2, INSTRUMENT_1, DANCE_PAD_BUTTON_UP },		// column 3
						{ 3, INSTRUMENT_1, DANCE_PAD_BUTTON_RIGHT },	// column 4
					}
				},
			},
			{	// StyleDef
				"versus",	// m_szName
				"single",	// m_szReads
				1,			// m_iNumPlayers
				4,			// m_iNumTracks
				{	// m_StyleInputToGameInput[NUM_PLAYERS][NUM_NOTE_COLS];
					{	// player 1
						{ 0, INSTRUMENT_1, DANCE_PAD_BUTTON_LEFT },		// column 1
						{ 1, INSTRUMENT_1, DANCE_PAD_BUTTON_DOWN },		// column 2
						{ 2, INSTRUMENT_1, DANCE_PAD_BUTTON_UP },		// column 3
						{ 3, INSTRUMENT_1, DANCE_PAD_BUTTON_RIGHT },	// column 4
					},
					{	// player 2
						{ 0, INSTRUMENT_2, DANCE_PAD_BUTTON_LEFT },		// column 1
						{ 1, INSTRUMENT_2, DANCE_PAD_BUTTON_DOWN },		// column 2
						{ 2, INSTRUMENT_2, DANCE_PAD_BUTTON_UP },		// column 3
						{ 3, INSTRUMENT_2, DANCE_PAD_BUTTON_RIGHT },	// column 4
					}

				},
			},
			{	// StyleDef
				"double",	// m_szName
				"double",	// m_szReads
				1,			// m_iNumPlayers
				8,			// m_iNumTracks
				{	// m_StyleInputToGameInput[NUM_PLAYERS][NUM_NOTE_COLS];
					{	// player 1
						{ 0, INSTRUMENT_1, DANCE_PAD_BUTTON_LEFT },		// column 1
						{ 1, INSTRUMENT_1, DANCE_PAD_BUTTON_DOWN },		// column 2
						{ 2, INSTRUMENT_1, DANCE_PAD_BUTTON_UP },		// column 3
						{ 3, INSTRUMENT_1, DANCE_PAD_BUTTON_RIGHT },	// column 4
						{ 4, INSTRUMENT_2, DANCE_PAD_BUTTON_LEFT },		// column 5
						{ 5, INSTRUMENT_2, DANCE_PAD_BUTTON_DOWN },		// column 6
						{ 6, INSTRUMENT_2, DANCE_PAD_BUTTON_UP },		// column 7
						{ 7, INSTRUMENT_2, DANCE_PAD_BUTTON_RIGHT },	// column 8
					},
				},
			},
			{	// StyleDef
				"couple",	// m_szName
				"couple",	// m_szReads
				1,			// m_iNumPlayers
				8,			// m_iNumTracks
				{	// m_StyleInputToGameInput[NUM_PLAYERS][NUM_NOTE_COLS];
					{	// player 1
						{ 0, INSTRUMENT_1, DANCE_PAD_BUTTON_LEFT },		// column 1
						{ 1, INSTRUMENT_1, DANCE_PAD_BUTTON_DOWN },		// column 2
						{ 2, INSTRUMENT_1, DANCE_PAD_BUTTON_UP },		// column 3
						{ 3, INSTRUMENT_1, DANCE_PAD_BUTTON_RIGHT },	// column 4
					},
					{	// player 2
						{ 4, INSTRUMENT_2, DANCE_PAD_BUTTON_LEFT },		// column 1
						{ 5, INSTRUMENT_2, DANCE_PAD_BUTTON_DOWN },		// column 2
						{ 6, INSTRUMENT_2, DANCE_PAD_BUTTON_UP },		// column 3
						{ 7, INSTRUMENT_2, DANCE_PAD_BUTTON_RIGHT },	// column 4
					}

				},
			},
			{	// StyleDef
				"solo",		// m_szName
				"solo",		// m_szReads
				1,			// m_iNumPlayers
				6,			// m_iNumTracks
				{	// m_StyleInputToGameInput[NUM_PLAYERS][NUM_NOTE_COLS];
					{	// player 1
						{ 0, INSTRUMENT_1, DANCE_PAD_BUTTON_LEFT },		// column 1
						{ 1, INSTRUMENT_1, DANCE_PAD_BUTTON_UPLEFT },	// column 2
						{ 2, INSTRUMENT_1, DANCE_PAD_BUTTON_DOWN },		// column 3
						{ 3, INSTRUMENT_1, DANCE_PAD_BUTTON_UP },		// column 4
						{ 4, INSTRUMENT_1, DANCE_PAD_BUTTON_UPRIGHT },	// column 5
						{ 5, INSTRUMENT_1, DANCE_PAD_BUTTON_RIGHT },	// column 6
					}
				},
			},

		}
	}
};


GameManager::GameManager()
{
	SwitchGame( "dance" );
	SwitchStyle( "single" );
}


GameDef* GameManager::GetGameDef( CString sGame )
{
	for( int i=0; i<NUM_GAME_DEFS; i++ )
	{
		if( g_GameDefs[i].m_szName == sGame )
			return &g_GameDefs[i];
	}

	return NULL;
}

StyleDef* GameManager::GetStyleDef( CString sGame, CString sStyle )
{
	GameDef* pGameDef = GetGameDef( sGame );

	for( int i=0; i<pGameDef->m_iNumStyleDefs; i++ )
	{
		if( pGameDef->m_StyleDefs[i].m_szName == sStyle )
			return &pGameDef->m_StyleDefs[i];
	}

	return NULL;
}


bool GameManager::IsPlayerEnabled( PlayerNumber PlayerNo )
{
	StyleDef* pStyleDef = GetCurrentStyleDef();
	ASSERT( pStyleDef != NULL );

	switch( pStyleDef->m_iNumPlayers )
	{
	case 1:
		switch( PlayerNo )
		{
		case PLAYER_1:	return true;
		case PLAYER_2:	return false;
		default:	ASSERT( false );
		}
		break;
	case 2:
		switch( PlayerNo )
		{
		case PLAYER_1:	return true;
		case PLAYER_2:	return true;
		default:	ASSERT( false );
		}
		break;
	default:
		ASSERT( false );	// invalid m_iNumPlayers
	}

	return false;
}

