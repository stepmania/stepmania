#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GameManager

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"


GameManager*	GAMEMAN = NULL;	// global and accessable from anywhere in our program


InstrumentButton	DANCE_BUTTON_LEFT		= (InstrumentButton)0;
InstrumentButton	DANCE_BUTTON_RIGHT		= (InstrumentButton)1;
InstrumentButton	DANCE_BUTTON_UP			= (InstrumentButton)2;
InstrumentButton	DANCE_BUTTON_DOWN		= (InstrumentButton)3;
InstrumentButton	DANCE_BUTTON_UPLEFT		= (InstrumentButton)4;
InstrumentButton	DANCE_BUTTON_UPRIGHT	= (InstrumentButton)5;
InstrumentButton	DANCE_BUTTON_SELECT		= (InstrumentButton)6;
InstrumentButton	DANCE_BUTTON_START		= (InstrumentButton)7;
int					NUM_DANCE_BUTTONS		= 8;

InstrumentButton	PUMP_BUTTON_UPLEFT		= (InstrumentButton)0;
InstrumentButton	PUMP_BUTTON_UPRIGHT		= (InstrumentButton)1;
InstrumentButton	PUMP_BUTTON_CENTER		= (InstrumentButton)2;
InstrumentButton	PUMP_BUTTON_DOWNLEFT	= (InstrumentButton)3;
InstrumentButton	PUMP_BUTTON_DOWNRIGHT	= (InstrumentButton)4;
InstrumentButton	PUMP_BUTTON_SELECT		= (InstrumentButton)5;
int					NUM_PUMP_BUTTONS		= 6;

InstrumentButton	EZ2_BUTTON_UPLEFT		= (InstrumentButton)0;
InstrumentButton	EZ2_BUTTON_UPLEFTHAND	= (InstrumentButton)1;
InstrumentButton	EZ2_BUTTON_LRLEFTHAND	= (InstrumentButton)2;
InstrumentButton	EZ2_BUTTON_DOWN			= (InstrumentButton)3;
InstrumentButton	EZ2_BUTTON_LRRIGHTHAND	= (InstrumentButton)4;
InstrumentButton	EZ2_BUTTON_UPRIGHTHAND	= (InstrumentButton)5;
InstrumentButton	EZ2_BUTTON_UPRIGHT		= (InstrumentButton)6;
InstrumentButton	EZ2_BUTTON_SELECT		= (InstrumentButton)7;
int					NUM_EZ2_BUTTONS			= 8;

const int DANCE_COL_SPACING = 64;
const int DANCE_6PANEL_VERSUS_COL_SPACING = 54;
const int PUMP_COL_SPACING = 60;
const int EZ2_COL_SPACING = 20; // CHANGE THIS LATER (Actually don't know colspaces yet)

GameDef g_GameDefs[NUM_GAMES] = 
{
	{	// GAME_DANCE
		"dance",					// m_szName
		"Dance Dance Revolution",	// m_szDescription
		2,							// m_iNumInstruments
		NUM_DANCE_BUTTONS,			// m_iButtonsPerInstrument
		{	// m_szButtonNames
			"Left",
			"Right",
			"Up",
			"Down",
			"UpLeft",
			"UpRight",
			"Back",
			"Start"
		},
		{	// m_iMenuButtons[NUM_MENU_BUTTONS];	// map from MenuButton to m_szButtonNames
			DANCE_BUTTON_LEFT,		// MENU_BUTTON_LEFT
			DANCE_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
			DANCE_BUTTON_UP,		// MENU_BUTTON_UP
			DANCE_BUTTON_DOWN,		// MENU_BUTTON_DOWN
			DANCE_BUTTON_START,		// MENU_BUTTON_START
			DANCE_BUTTON_SELECT,	// MENU_BUTTON_BACK
		},
	},
	{	// GAME_PUMP
		"pump",				// m_szName
		"Pump It Up",		// m_szDescription
		2,					// m_iNumInstruments
		NUM_DANCE_BUTTONS,	// m_iButtonsPerInstrument //? DANCE buttons (from Andy)
		{	// m_szButtonNames
			"UpLeft",
			"UpRight",
			"Center",
			"DownLeft",
			"DownRight",
			"Back"
		},
		{	// m_iMenuButtons[NUM_MENU_BUTTONS];	// map from MenuButton to m_szButtonNames
			PUMP_BUTTON_UPLEFT,		// MENU_BUTTON_LEFT
			PUMP_BUTTON_UPRIGHT,	// MENU_BUTTON_RIGHT
			PUMP_BUTTON_DOWNRIGHT,	// MENU_BUTTON_UP
			PUMP_BUTTON_DOWNLEFT,	// MENU_BUTTON_DOWN
			PUMP_BUTTON_CENTER,		// MENU_BUTTON_START
			PUMP_BUTTON_SELECT,		// MENU_BUTTON_BACK
		},
	},
	{	// GAME_EZ2 ********* TEMPORARY DDR BORROWAGE FOR MENUS (The Input System needs WORK!) ********
		"ez2",						// m_szName
		"Ez2dancer",				// m_szDescription
		2,							// m_iNumInstruments
		NUM_DANCE_BUTTONS,			// m_iButtonsPerInstrument // If it works, i'll go with it, but i'm a bit unsure here...
		{	// m_szButtonNames
			"UpLeft",
			"UpLeftHand",
			"LrLeftHand",
			"Down",
			"LrRightHand",
			"UpRightHand",
			"UpRight",
			"Start"
		},
		{	// m_iMenuButtons[NUM_MENU_BUTTONS];	// map from MenuButton to m_szButtonNames
			DANCE_BUTTON_LEFT,		// MENU_BUTTON_LEFT
			DANCE_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
			DANCE_BUTTON_UP,		// MENU_BUTTON_UP
			DANCE_BUTTON_DOWN,		// MENU_BUTTON_DOWN
			DANCE_BUTTON_START,		// MENU_BUTTON_START
			DANCE_BUTTON_SELECT,	// MENU_BUTTON_BACK
		},
	},

};

StyleDef g_StyleDefs[NUM_STYLES] = 
{
	{	// STYLE_DANCE_SINGLE
		"dance-single",						// m_szName
		NOTES_TYPE_DANCE_SINGLE,			// m_NotesType
		StyleDef::ONE_PLAYER_USES_ONE_SIDE,	// m_StyleType
		{ 160, 480 },						// m_iCenterX
		4,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	INSTRUMENT_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	INSTRUMENT_1,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_1,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	INSTRUMENT_2,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	INSTRUMENT_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
	},
	{	// STYLE_DANCE_VERSUS
		"dance-versus",							// m_szName
		NOTES_TYPE_DANCE_SINGLE,				// m_NotesType
		StyleDef::TWO_PLAYERS_USE_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	INSTRUMENT_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	INSTRUMENT_1,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_1,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	INSTRUMENT_2,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	INSTRUMENT_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
	},
	{	// STYLE_DANCE_DOUBLE
		"dance-double",							// m_szName
		NOTES_TYPE_DANCE_DOUBLE,				// m_NotesType
		StyleDef::ONE_PLAYER_USES_TWO_SIDES,	// m_StyleType
		{ 320, 320 },							// m_iCenterX
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	INSTRUMENT_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	INSTRUMENT_1,	DANCE_BUTTON_UP,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	INSTRUMENT_1,	DANCE_BUTTON_RIGHT,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_2,	DANCE_BUTTON_LEFT,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	INSTRUMENT_2,	DANCE_BUTTON_DOWN,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	INSTRUMENT_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	INSTRUMENT_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	INSTRUMENT_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	INSTRUMENT_1,	DANCE_BUTTON_UP,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	INSTRUMENT_1,	DANCE_BUTTON_RIGHT,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_2,	DANCE_BUTTON_LEFT,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	INSTRUMENT_2,	DANCE_BUTTON_DOWN,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	INSTRUMENT_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	INSTRUMENT_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*3.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
	},
	{	// StyleDef
		"dance-couple",							// m_szName
		NOTES_TYPE_DANCE_SINGLE,				// m_NotesType
		StyleDef::TWO_PLAYERS_USE_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	INSTRUMENT_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	INSTRUMENT_1,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_1,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	INSTRUMENT_2,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	INSTRUMENT_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
	},
	{	// STYLE_DANCE_SOLO
		"dance-solo",						// m_szName
		NOTES_TYPE_DANCE_SOLO,				// m_NotesType
		StyleDef::ONE_PLAYER_USES_ONE_SIDE,	// m_StyleType
		{ 240, 400 },						// m_iCenterX
		6,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	DANCE_BUTTON_LEFT,		-DANCE_COL_SPACING*2.5f },
				{ TRACK_2,	INSTRUMENT_1,	DANCE_BUTTON_UPLEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_3,	INSTRUMENT_1,	DANCE_BUTTON_DOWN,		-DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_1,	DANCE_BUTTON_UP,		+DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_1,	DANCE_BUTTON_UPRIGHT,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	INSTRUMENT_1,	DANCE_BUTTON_RIGHT,		+DANCE_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	DANCE_BUTTON_LEFT,		-DANCE_COL_SPACING*2.5f },
				{ TRACK_2,	INSTRUMENT_2,	DANCE_BUTTON_UPLEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_3,	INSTRUMENT_2,	DANCE_BUTTON_DOWN,		-DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_2,	DANCE_BUTTON_UP,		+DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_2,	DANCE_BUTTON_UPRIGHT,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	INSTRUMENT_2,	DANCE_BUTTON_RIGHT,		+DANCE_COL_SPACING*2.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5
		},
	},
/*	{	// STYLE_DANCE_SOLO_VERSUS 
		"dance-solo-versus",				// m_szName
		NOTES_TYPE_DANCE_SOLO,				// m_NotesType
		StyleDef::ONE_PLAYER_USES_ONE_SIDE,	// m_StyleType
		{ 160, 480 },						// m_iCenterX
		6,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	DANCE_BUTTON_LEFT,		-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
				{ TRACK_2,	INSTRUMENT_1,	DANCE_BUTTON_UPLEFT,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_3,	INSTRUMENT_1,	DANCE_BUTTON_DOWN,		-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_1,	DANCE_BUTTON_UP,		+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_1,	DANCE_BUTTON_UPRIGHT,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_6,	INSTRUMENT_1,	DANCE_BUTTON_RIGHT,		+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	DANCE_BUTTON_LEFT,		-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
				{ TRACK_2,	INSTRUMENT_2,	DANCE_BUTTON_UPLEFT,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_3,	INSTRUMENT_2,	DANCE_BUTTON_DOWN,		-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_4,	INSTRUMENT_2,	DANCE_BUTTON_UP,		+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_2,	DANCE_BUTTON_UPRIGHT,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_6,	INSTRUMENT_2,	DANCE_BUTTON_RIGHT,		+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,5,1,4,2,3		// outside in
		},
	},
*/	{	// PUMP_STYLE_SINGLE
		"pump-single",							// m_szName
		NOTES_TYPE_PUMP_SINGLE,					// m_NotesType
		StyleDef::ONE_PLAYER_USES_ONE_SIDE,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	INSTRUMENT_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	INSTRUMENT_1,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_1,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_1,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	INSTRUMENT_2,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	INSTRUMENT_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
	},
	{	// PUMP_STYLE_VERSUS
		"pump-versus",							// m_szName
		NOTES_TYPE_PUMP_SINGLE,					// m_NotesType
		StyleDef::TWO_PLAYERS_USE_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	INSTRUMENT_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	INSTRUMENT_1,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_1,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_1,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	INSTRUMENT_2,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	INSTRUMENT_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
	},
	{	// PUMP_STYLE_DOUBLE
		"pump-double",							// m_szName
		NOTES_TYPE_PUMP_SINGLE,					// m_NotesType
		StyleDef::ONE_PLAYER_USES_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*4.5f },
				{ TRACK_2,	INSTRUMENT_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*3.5f },
				{ TRACK_3,	INSTRUMENT_1,	PUMP_BUTTON_CENTER,		-PUMP_COL_SPACING*2.5f },
				{ TRACK_4,	INSTRUMENT_1,	PUMP_BUTTON_UPRIGHT,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_5,	INSTRUMENT_1,	PUMP_BUTTON_DOWNRIGHT,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_6,	INSTRUMENT_2,	PUMP_BUTTON_DOWNLEFT,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_7,	INSTRUMENT_2,	PUMP_BUTTON_UPLEFT,		+PUMP_COL_SPACING*1.5f },
				{ TRACK_8,	INSTRUMENT_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*2.5f },
				{ TRACK_9,	INSTRUMENT_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*3.5f },
				{ TRACK_10,	INSTRUMENT_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*4.5f },
				{ TRACK_2,	INSTRUMENT_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*3.5f },
				{ TRACK_3,	INSTRUMENT_1,	PUMP_BUTTON_CENTER,		-PUMP_COL_SPACING*2.5f },
				{ TRACK_4,	INSTRUMENT_1,	PUMP_BUTTON_UPRIGHT,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_5,	INSTRUMENT_1,	PUMP_BUTTON_DOWNRIGHT,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_6,	INSTRUMENT_2,	PUMP_BUTTON_DOWNLEFT,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_7,	INSTRUMENT_2,	PUMP_BUTTON_UPLEFT,		+PUMP_COL_SPACING*1.5f },
				{ TRACK_8,	INSTRUMENT_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*2.5f },
				{ TRACK_9,	INSTRUMENT_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*3.5f },
				{ TRACK_10,	INSTRUMENT_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*4.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3,5,7,9,6,8
		},
	},


	{	// EZ2_STYLE_SINGLE
		"ez2-single",							// m_szName
		NOTES_TYPE_EZ2_SINGLE,					// m_NotesType
		StyleDef::ONE_PLAYER_USES_ONE_SIDE,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*1.0f },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*0.5f },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*1.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*2 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		    // 0,1,2,3,4
		},
	},
	{	// EZ2_STYLE_SINGLE_HARD
		"ez2-single-hard",							// m_szName
		NOTES_TYPE_EZ2_SINGLE_HARD,					// m_NotesType
		StyleDef::ONE_PLAYER_USES_ONE_SIDE,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*1.0f },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*0.5f },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*0.5f },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*1.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*2 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		    // 0,1,2,3,4
		},
	},
	{	// EZ2_STYLE_DOUBLE
		"ez2-double",							// m_szName
		NOTES_TYPE_EZ2_DOUBLE,					// m_NotesType
		StyleDef::ONE_PLAYER_USES_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*4.5f },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*3.5f },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		-EZ2_COL_SPACING*2.5f },
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	-EZ2_COL_SPACING*1.5f },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		-EZ2_COL_SPACING*0.5f },
				{ TRACK_6,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		+EZ2_COL_SPACING*0.5f }, 
				{ TRACK_7,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	+EZ2_COL_SPACING*1.5f },  
				{ TRACK_8,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*2.5f },
				{ TRACK_9,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*3.5f },
				{ TRACK_10,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*4.5f },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*3.5f },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		-EZ2_COL_SPACING*2.5f },
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	-EZ2_COL_SPACING*1.5f },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		-EZ2_COL_SPACING*0.5f },
				{ TRACK_6,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		+EZ2_COL_SPACING*0.5f }, 
				{ TRACK_7,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	+EZ2_COL_SPACING*1.5f },  
				{ TRACK_8,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*2.5f },
				{ TRACK_9,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*3.5f },
				{ TRACK_10,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*4.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// EZ2_STYLE_REAL
		"ez2-real",							// m_szName
		NOTES_TYPE_EZ2_REAL,					// m_NotesType
		StyleDef::ONE_PLAYER_USES_ONE_SIDE,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		7,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*3 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_LRLEFTHAND,	-EZ2_COL_SPACING*2 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 }, 
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 }, 
				{ TRACK_6,	INSTRUMENT_1,	EZ2_BUTTON_LRRIGHTHAND,	+EZ2_COL_SPACING*2 },
				{ TRACK_7,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*3 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_2,	EZ2_BUTTON_LRLEFTHAND,	-EZ2_COL_SPACING*2 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 }, 
				{ TRACK_4,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_5,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 }, 
				{ TRACK_6,	INSTRUMENT_2,	EZ2_BUTTON_LRRIGHTHAND,	+EZ2_COL_SPACING*2 },
				{ TRACK_7,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*3 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// EZ2_STYLE_SINGLE_VERSUS
		"ez2-single-versus",							// m_szName
		NOTES_TYPE_EZ2_SINGLE,					// m_NotesType
		StyleDef::TWO_PLAYERS_USE_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*2 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*2 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// EZ2_STYLE_SINGLE_VERSUS
		"ez2-single-hard-versus",							// m_szName
		NOTES_TYPE_EZ2_SINGLE_HARD,					// m_NotesType
		StyleDef::TWO_PLAYERS_USE_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*2 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*2 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_4,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
	},

	{	// EZ2_STYLE_REAL_VERSUS
		"ez2-real-versus",							// m_szName
		NOTES_TYPE_EZ2_REAL,					// m_NotesType
		StyleDef::TWO_PLAYERS_USE_TWO_SIDES,	// m_StyleType
		{ 160, 480 },							// m_iCenterX
		7,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*3 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_1,	EZ2_BUTTON_LRLEFTHAND,	-EZ2_COL_SPACING*2 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_1,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 }, 
				{ TRACK_4,	INSTRUMENT_1,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_5,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 }, 
				{ TRACK_6,	INSTRUMENT_1,	EZ2_BUTTON_LRRIGHTHAND,	+EZ2_COL_SPACING*2 },
				{ TRACK_7,	INSTRUMENT_1,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFT,		-EZ2_COL_SPACING*3 },   // CHANGE THESE WHEN WE CAN SEE THE
				{ TRACK_2,	INSTRUMENT_2,	EZ2_BUTTON_LRLEFTHAND,	-EZ2_COL_SPACING*2 },  // NOTES IN-GAME !!!!!!!!!!
				{ TRACK_3,	INSTRUMENT_2,	EZ2_BUTTON_UPLEFTHAND,	-EZ2_COL_SPACING*1 }, 
				{ TRACK_4,	INSTRUMENT_2,	EZ2_BUTTON_DOWN,		+EZ2_COL_SPACING*0 },
				{ TRACK_5,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHTHAND,	+EZ2_COL_SPACING*1 }, 
				{ TRACK_6,	INSTRUMENT_2,	EZ2_BUTTON_LRRIGHTHAND,	+EZ2_COL_SPACING*2 },
				{ TRACK_7,	INSTRUMENT_2,	EZ2_BUTTON_UPRIGHT,		+EZ2_COL_SPACING*3 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
	},
};


GameManager::GameManager()
{
}


GameDef* GameManager::GetGameDefForGame( Game g )
{
	ASSERT( g != GAME_INVALID );
	return &g_GameDefs[ g ];
}

StyleDef* GameManager::GetStyleDefForStyle( Style s )
{
	ASSERT( s != STYLE_NONE );
	return &g_StyleDefs[ s ];
}

Style GameManager::GetStyleThatPlaysNotesType( NotesType nt )
{
	for( int i=0; i<NUM_STYLES; i++ )
		if( g_StyleDefs[i].m_NotesType == nt )
			return (Style)i;

	return STYLE_NONE;
}

void GameManager::GetGameNames( CStringArray &AddTo )
{
	for( int i=0; i<NUM_GAMES; i++ )
		AddTo.Add( g_GameDefs[i].m_szName );
}


void GameManager::GetNoteSkinNames( CStringArray &AddTo )
{
	GAMESTATE->GetCurrentGameDef()->GetSkinNames( AddTo );
}

bool GameManager::DoesNoteSkinExist( CString sSkinName )
{
	CStringArray asSkinNames;	
	GetNoteSkinNames( asSkinNames );
	for( int i=0; i<asSkinNames.GetSize(); i++ )
		if( 0==stricmp(sSkinName, asSkinNames[i]) )
			return true;
	return false;
}

void GameManager::SwitchNoteSkin( CString sNewNoteSkin )
{
	if( sNewNoteSkin == ""  ||  !DoesNoteSkinExist(sNewNoteSkin) )
	{
		CStringArray as;
		GetNoteSkinNames( as );
		m_sCurNoteSkin = as[0];
	}
	else
		m_sCurNoteSkin = sNewNoteSkin;
}

CString GameManager::GetPathTo( Game g, CString sSkinName, CString sButtonName, const SkinElement gbg )
{
	return GetGameDefForGame(g)->GetPathToGraphic( sSkinName, sButtonName, gbg );
}

void GameManager::GetTweenColors( Game g, CString sSkinName, CString sButtonName, CArray<D3DXCOLOR,D3DXCOLOR> &aTweenColorsAddTo )
{
	GetGameDefForGame(g)->GetTweenColors( sSkinName, sButtonName, aTweenColorsAddTo );
}

CString GameManager::GetPathTo( const int col, const SkinElement gbg )	// looks in GAMESTATE for the current Style
{
	StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();

	StyleInput SI( PLAYER_1, col );
	GameInput GI = pStyleDef->StyleInputToGameInput( SI );
	CString sButtonName = pGameDef->m_szButtonNames[GI.button];
	return pGameDef->GetPathToGraphic( m_sCurNoteSkin, sButtonName, gbg );
}

void GameManager::GetTweenColors( const int col, CArray<D3DXCOLOR,D3DXCOLOR> &aTweenColorsAddTo )	// looks in GAMESTATE for the current Style
{
	ASSERT( m_sCurNoteSkin != "" );	// if this == NULL, SwitchGame() was never called

	StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();

	StyleInput SI( PLAYER_1, col );
	GameInput GI = pStyleDef->StyleInputToGameInput( SI );
	CString sButtonName = pGameDef->m_szButtonNames[GI.button];
	pGameDef->GetTweenColors( m_sCurNoteSkin, sButtonName, aTweenColorsAddTo );
}
