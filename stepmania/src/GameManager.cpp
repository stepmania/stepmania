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
#include "GameInput.h"	// for GameButton constants
#include "IniFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SDL_keysym.h"		// for SDLKeys


GameManager*	GAMEMAN = NULL;	// global and accessable from anywhere in our program


const CString NOTESKIN_DIR  = "NoteSkins\\";


const int DANCE_COL_SPACING = 64;
const int DANCE_6PANEL_VERSUS_COL_SPACING = 54;
const int PUMP_COL_SPACING = 50;
const int EZ2_COL_SPACING = 46; 
const int EZ2_REAL_COL_SPACING = 40;
const int PARA_COL_SPACING = 54;
const int DS3DDX_COL_SPACING = 46;
const int BM_COL_SPACING=34;

struct {
	char *name;
	int NumTracks;
} const NotesTypes[NUM_NOTES_TYPES] = {
	{ "dance-single", 4 },
	{ "dance-double", 8 },
	{ "dance-couple", 8 },
	{ "dance-solo", 6 },
	{ "pump-single", 5 },
	{ "pump-double", 10 },
	{ "pump-couple", 10 },
	{ "ez2-single", 5 },		// Single: TL,LHH,D,RHH,TR
	{ "ez2-double", 10 },		// Double: Single x2
	{ "ez2-real", 7 },			// Real: TL,LHH,LHL,D,RHL,RHH,TR
	{ "para-single", 5 },
	{ "ds3ddx-single", 8 },
	{ "bm-single", 6 }
};

//
// Important:  Every game must define the buttons: "Start", "Back", "MenuLeft", and "MenuRight"
//
GameDef g_GameDefs[NUM_GAMES] = 
{
	{	// GAME_DANCE
		"dance",					// m_szName
		"Dance Dance Revolution",	// m_szDescription
		2,							// m_iNumControllers
		NUM_DANCE_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			"Left",
			"Right",
			"Up",
			"Down",
			"UpLeft",
			"UpRight",
			"Start",
			"Back",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
		},
		{	// m_szSecondaryFunction
			"(MenuLeft)",
			"(MenuRight)",
			"(MenuUp)",
			"(MenuDown)",
			"",
			"",
			"",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
		},
		{	// m_DedicatedMenuButton
			DANCE_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			DANCE_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			DANCE_BUTTON_MENUUP,	// MENU_BUTTON_UP
			DANCE_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			DANCE_BUTTON_START,		// MENU_BUTTON_START
			DANCE_BUTTON_BACK,		// MENU_BUTTON_BACK
			DANCE_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_SecondaryMenuButton
			DANCE_BUTTON_LEFT,		// MENU_BUTTON_LEFT
			DANCE_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
			DANCE_BUTTON_UP,		// MENU_BUTTON_UP
			DANCE_BUTTON_DOWN,		// MENU_BUTTON_DOWN
			DANCE_BUTTON_START,		// MENU_BUTTON_START
			DANCE_BUTTON_BACK,		// MENU_BUTTON_BACK
			DANCE_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_LEFT,				// DANCE_BUTTON_LEFT,
				SDLK_RIGHT,				// DANCE_BUTTON_RIGHT,
				SDLK_UP,					// DANCE_BUTTON_UP,
				SDLK_DOWN,				// DANCE_BUTTON_DOWN,
				-1, //no default key 	// DANCE_BUTTON_UPLEFT,
				-1, //no default key	// DANCE_BUTTON_UPRIGHT,
				SDLK_RETURN,				// DANCE_BUTTON_START,
				SDLK_ESCAPE,				// DANCE_BUTTON_BACK
				-1, //no default key	// DANCE_BUTTON_MENULEFT
				-1, //no default key	// DANCE_BUTTON_MENURIGHT
				SDLK_UP,					// DANCE_BUTTON_MENUUP
				SDLK_DOWN,				// DANCE_BUTTON_MENUDOWN
				SDLK_F1,				// DANCE_BUTTON_COIN
			},
			{	// PLAYER_2
				SDLK_KP4,			// DANCE_BUTTON_LEFT,
				SDLK_KP6,			// DANCE_BUTTON_RIGHT,
				SDLK_KP8,			// DANCE_BUTTON_UP,
				SDLK_KP2,			// DANCE_BUTTON_DOWN,
				SDLK_KP7,			// DANCE_BUTTON_UPLEFT,
				SDLK_KP9,			// DANCE_BUTTON_UPRIGHT,
				SDLK_KP_ENTER,		// DANCE_BUTTON_START,
				SDLK_KP0,			// DANCE_BUTTON_BACK
				-1, //no default key	// DANCE_BUTTON_MENULEFT
				-1, //no default key	// DANCE_BUTTON_MENURIGHT
				-1, //no default key	// DANCE_BUTTON_MENUUP
				-1, //no default key	// DANCE_BUTTON_MENUDOWN
				SDLK_F2,				// DANCE_BUTTON_COIN
			},
		}
	},
	{	// GAME_PUMP
		"pump",				// m_szName
		"Pump It Up",		// m_szDescription
		2,					// m_iNumControllers
		NUM_PUMP_BUTTONS,	// m_iButtonsPerController
		{	// m_szButtonNames
			"UpLeft",
			"UpRight",
			"Center",
			"DownLeft",
			"DownRight",
			"Start",
			"Back",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
		},
		{	// m_szSecondaryFunction
			"(MenuUp)",
			"(MenuDown)",
			"(Start)",
			"(MenuLeft)",
			"(MenuRight)",
			"(dedicated)",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
		},
		{	// m_DedicatedMenuButton
			PUMP_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			PUMP_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			PUMP_BUTTON_MENUUP,		// MENU_BUTTON_UP
			PUMP_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			PUMP_BUTTON_START,		// MENU_BUTTON_START
			PUMP_BUTTON_BACK,		// MENU_BUTTON_BACK
			PUMP_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_SecondaryMenuButton
			PUMP_BUTTON_DOWNLEFT,		// MENU_BUTTON_LEFT
			PUMP_BUTTON_DOWNRIGHT,	// MENU_BUTTON_RIGHT
			PUMP_BUTTON_UPLEFT,	// MENU_BUTTON_UP
			PUMP_BUTTON_UPRIGHT,	// MENU_BUTTON_DOWN
			PUMP_BUTTON_CENTER,		// MENU_BUTTON_START
			PUMP_BUTTON_BACK,		// MENU_BUTTON_BACK
			PUMP_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_q,					// PUMP_BUTTON_UPLEFT,
				SDLK_e,					// PUMP_BUTTON_UPRIGHT,
				SDLK_s,					// PUMP_BUTTON_CENTER,
				SDLK_z,					// PUMP_BUTTON_DOWNLEFT,
				SDLK_c,  				// PUMP_BUTTON_DOWNRIGHT,
				SDLK_RETURN,				// PUMP_BUTTON_START,
				SDLK_ESCAPE,				// PUMP_BUTTON_BACK,
				SDLK_LEFT,				// PUMP_BUTTON_MENULEFT
				SDLK_RIGHT,				// PUMP_BUTTON_MENURIGHT
				SDLK_UP,					// PUMP_BUTTON_MENUUP
				SDLK_DOWN,				// PUMP_BUTTON_MENUDOWN
				SDLK_F1,				// PUMP_BUTTON_COIN
			},
			{	// PLAYER_2
				SDLK_KP7,			// PUMP_BUTTON_UPLEFT,
				SDLK_KP9,			// PUMP_BUTTON_UPRIGHT,
				SDLK_KP5,			// PUMP_BUTTON_CENTER,
				SDLK_KP1,			// PUMP_BUTTON_DOWNLEFT,
				SDLK_KP3,  			// PUMP_BUTTON_DOWNRIGHT,
				SDLK_KP_ENTER,		// PUMP_BUTTON_START,
				SDLK_KP0,			// PUMP_BUTTON_BACK,
				-1,	//no default key	// PUMP_BUTTON_MENULEFT
				-1, //no default key	// PUMP_BUTTON_MENURIGHT
				-1, //no default key	// PUMP_BUTTON_MENUUP
				-1, //no default key	// PUMP_BUTTON_MENUDOWN
				SDLK_F2,				// PUMP_BUTTON_COIN
			},
		}
	},
	{
		"ez2",						// m_szName
		"Ez2Dancer",				// m_szDescription
		2,							// m_iNumControllers
		NUM_EZ2_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			"FootUpLeft",
			"FootUpRight",
			"FootDown",
			"HandUpLeft",
			"HandUpRight",
			"HandLrLeft",
			"HandLrRight",
			"Start",
			"Back",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
		},
		{	// m_szSecondaryFunction
			"(MenuUp)",
			"(MenuDown)",
			"(Start)",
			"(MenuLeft)",
			"(MenuRight)",
			"",
			"",
			"(dedicated)",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
		},
		{	// m_DedicatedMenuButton
			EZ2_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			EZ2_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			EZ2_BUTTON_MENUUP,		// MENU_BUTTON_UP
			EZ2_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			EZ2_BUTTON_START,		// MENU_BUTTON_START
			EZ2_BUTTON_BACK,		// MENU_BUTTON_BACK
			EZ2_BUTTON_COIN,	// MENU_BUTTON_COIN
		},
		{	// m_SecondaryMenuButton
			EZ2_BUTTON_HANDUPLEFT,	// MENU_BUTTON_LEFT
			EZ2_BUTTON_HANDUPRIGHT,	// MENU_BUTTON_RIGHT
			EZ2_BUTTON_FOOTUPLEFT,	// MENU_BUTTON_UP
			EZ2_BUTTON_FOOTUPRIGHT,	// MENU_BUTTON_DOWN
			EZ2_BUTTON_FOOTDOWN,	// MENU_BUTTON_START
			EZ2_BUTTON_BACK,		// MENU_BUTTON_BACK
			EZ2_BUTTON_COIN,	// MENU_BUTTON_COIN
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_z,					// EZ2_BUTTON_FOOTUPLEFT,
				SDLK_b,					// EZ2_BUTTON_FOOTUPRIGHT,
				SDLK_c,					// EZ2_BUTTON_FOOTDOWN,
				SDLK_x,					// EZ2_BUTTON_HANDUPLEFT,
				SDLK_v,					// EZ2_BUTTON_HANDUPRIGHT,
				SDLK_s,					// EZ2_BUTTON_HANDLRLEFT,
				SDLK_f,  				// EZ2_BUTTON_HANDLRRIGHT,
				SDLK_RETURN,				// EZ2_BUTTON_START,
				SDLK_ESCAPE,				// EZ2_BUTTON_BACK,
				SDLK_LEFT,				// EZ2_BUTTON_MENULEFT
				SDLK_RIGHT,				// EZ2_BUTTON_MENURIGHT
				SDLK_UP,					// EZ2_BUTTON_MENUUP
				SDLK_DOWN,				// EZ2_BUTTON_MENUDOWN
				SDLK_F1,				// EZ2_BUTTON_COIN
			},
			{	// PLAYER_2
				-1,						// EZ2_BUTTON_FOOTUPLEFT,
				-1,						// EZ2_BUTTON_FOOTUPRIGHT,
				-1,						// EZ2_BUTTON_FOOTDOWN,
				-1,						// EZ2_BUTTON_HANDUPLEFT,
				-1,						// EZ2_BUTTON_HANDUPRIGHT,
				-1,						// EZ2_BUTTON_HANDLRLEFT,
				-1,  					// EZ2_BUTTON_HANDLRRIGHT,
				-1,						// EZ2_BUTTON_START,
				-1,						// EZ2_BUTTON_BACK,
				-1,	//no default key	// EZ2_BUTTON_MENULEFT
				-1, //no default key	// EZ2_BUTTON_MENURIGHT
				-1, //no default key	// EZ2_BUTTON_MENUUP
				-1, //no default key	// EZ2_BUTTON_MENUDOWN
				SDLK_F2,				// EZ2_BUTTON_COIN
			},
		},
	},
	{	// GAME_PARA
		"para",					// m_szName
		"Para Para Paradise",	// m_szDescription
		2,							// m_iNumControllers
		NUM_DANCE_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			"Left",
			"UpLeft",
			"Up",
			"UpRight",
			"Right",
			"Start",
			"Back",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
		},
		{	// m_szSecondaryFunction
			"(MenuLeft)",
			"(MenuDown)",
			"",
			"(MenuUp)",
			"(MenuRight)",
			"",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
		},
		{	// m_DedicatedMenuButton
			PARA_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			PARA_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			PARA_BUTTON_MENUUP,	// MENU_BUTTON_UP
			PARA_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			PARA_BUTTON_START,		// MENU_BUTTON_START
			PARA_BUTTON_BACK,		// MENU_BUTTON_BACK
			PARA_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_SecondaryMenuButton
			PARA_BUTTON_LEFT,		// MENU_BUTTON_LEFT
			PARA_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
			PARA_BUTTON_UPRIGHT,			// MENU_BUTTON_UP
			PARA_BUTTON_UPLEFT,		// MENU_BUTTON_DOWN
			PARA_BUTTON_START,		// MENU_BUTTON_START
			PARA_BUTTON_BACK,		// MENU_BUTTON_BACK
			PARA_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_z,					// PARA_BUTTON_LEFT,
				SDLK_x,					// PARA_BUTTON_UPLEFT,
				SDLK_c,					// PARA_BUTTON_UP,
				SDLK_v,					// PARA_BUTTON_UPRIGHT,
				SDLK_b,					// PARA_BUTTON_RIGHT,
				SDLK_RETURN,				// PARA_BUTTON_START,
				SDLK_ESCAPE,				// PARA_BUTTON_BACK
				SDLK_LEFT, //no default key	// PARA_BUTTON_MENULEFT
				SDLK_RIGHT, //no default key	// PARA_BUTTON_MENURIGHT
				SDLK_UP,					// PARA_BUTTON_MENUUP
				SDLK_DOWN,				// PARA_BUTTON_MENUDOWN
				SDLK_F1,				// PARA_BUTTON_COIN
			},
			{	// PLAYER_2
				-1,					// PARA_BUTTON_LEFT,
				-1,					// PARA_BUTTON_UPLEFT,
				-1,					// PARA_BUTTON_UP,
				-1,					// PARA_BUTTON_UPRIGHT,
				-1,					// PARA_BUTTON_RIGHT,
				-1,				// PARA_BUTTON_START,
				-1,				// PARA_BUTTON_BACK
				-1, //no default key	// PARA_BUTTON_MENULEFT
				-1, //no default key	// PARA_BUTTON_MENURIGHT
				-1,					// PARA_BUTTON_MENUUP
				-1,
				-1,					// PARA_BUTTON_COIN
			},
		}
	},
	{	// GAME_DS3DDX
		"ds3ddx",					// m_szName
		"Dance Station 3DDX",	// m_szDescription
		2,							// m_iNumControllers
		NUM_DS3DDX_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			"HandLeft",
			"FootDownLeft",
			"FootUpLeft",
			"HandUp",
			"HandDown",
			"FootUpRight",
			"FootDownRight",
			"HandRight",
			"Start",
			"Back",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
		},
		{	// m_szSecondaryFunction
			"(MenuLeft)",
			"",
			"",
			"(MenuUp)",
			"(MenuDown)",
			"",
			"",
			"(MenuRight)",
			"",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
		},
		{	// m_DedicatedMenuButton
			DS3DDX_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			DS3DDX_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			DS3DDX_BUTTON_MENUUP,	// MENU_BUTTON_UP
			DS3DDX_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			DS3DDX_BUTTON_START,		// MENU_BUTTON_START
			DS3DDX_BUTTON_BACK,		// MENU_BUTTON_BACK
			DS3DDX_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_SecondaryMenuButton
			DS3DDX_BUTTON_HANDLEFT,		// MENU_BUTTON_LEFT
			DS3DDX_BUTTON_HANDRIGHT,		// MENU_BUTTON_RIGHT
			DS3DDX_BUTTON_HANDUP,			// MENU_BUTTON_UP
			DS3DDX_BUTTON_HANDDOWN,		// MENU_BUTTON_DOWN
			DS3DDX_BUTTON_START,		// MENU_BUTTON_START
			DS3DDX_BUTTON_BACK,		// MENU_BUTTON_BACK
			DS3DDX_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_a,					// DS3DDX_BUTTON_HANDLEFT,
				SDLK_z,					// DS3DDX_BUTTON_FOOTDOWNLEFT,
				SDLK_q,					// DS3DDX_BUTTON_FOOTUPLEFT,
				SDLK_w,					// DS3DDX_BUTTON_HANDUP,
				SDLK_x,					// DS3DDX_BUTTON_HANDDOWN,
				SDLK_e,					// DS3DDX_BUTTON_FOOTUPRIGHT,
				SDLK_c,					// DS3DDX_BUTTON_FOOTDOWNRIGHT,
				SDLK_d,					// DS3DDX_BUTTON_HANDRIGHT,
				SDLK_RETURN,				// DS3DDX_BUTTON_START,
				SDLK_ESCAPE,				// DS3DDX_BUTTON_BACK
				SDLK_LEFT, //no default key	// DS3DDX_BUTTON_MENULEFT
				SDLK_RIGHT, //no default key	// DS3DDX_BUTTON_MENURIGHT
				SDLK_UP,					// DS3DDX_BUTTON_MENUUP
				SDLK_DOWN,				// DS3DDX_BUTTON_MENUDOWN
				SDLK_F1,				// DS3DDX_BUTTON_COIN
			},
			{	// PLAYER_2
				-1,					// DS3DDX_BUTTON_HANDLEFT,
				-1,					// DS3DDX_BUTTON_FOOTDOWNLEFT,
				-1,					// DS3DDX_BUTTON_FOOTUPLEFT,
				-1,					// DS3DDX_BUTTON_HANDUP,
				-1,					// DS3DDX_BUTTON_HANDDOWN,
				-1,					// DS3DDX_BUTTON_FOOTUPRIGHT,
				-1,					// DS3DDX_BUTTON_FOOTDOWNRIGHT,
				-1,					// DS3DDX_BUTTON_HANDRIGHT,
				-1,				// DS3DDX_BUTTON_START,
				-1,				// DS3DDX_BUTTON_BACK
				-1, //no default key	// DS3DDX_BUTTON_MENULEFT
				-1, //no default key	// DS3DDX_BUTTON_MENURIGHT
				-1,					// DS3DDX_BUTTON_MENUUP
				-1,				// DS3DDX_BUTTON_MENUDOWN
				-1,				// DS3DDX_BUTTON_COIN
			},
		}
	},
	{	// GAME_BM
		"bm",				// m_szName
		"BeatMania",		// m_szDescription
		2,					// m_iNumControllers
		NUM_BM_BUTTONS,	// m_iButtonsPerController
		{	// m_szButtonNames
			"Key1",
			"Key2",
			"Key3",
			"Key4",
			"Key5",
			"Key6",
			"Key7",
			"Scratch",
			"Scratch (down)",
			"Start",
			"Select",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
		},
		{	// m_szSecondaryFunction
			"(MenuLeft)",
			"",
			"(MenuRight)",
			"",
			"",
			"",
			"",
			"(MenuUp)",
			"(MenuDown)",
			"(Start)",
			"(Back)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
		},
		{	// m_DedicatedMenuButton
			BM_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
			BM_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			BM_BUTTON_MENUUP,		// MENU_BUTTON_UP
			BM_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
			BM_BUTTON_START,		// MENU_BUTTON_START
			BM_BUTTON_SELECT,		// MENU_BUTTON_BACK
			BM_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_SecondaryMenuButton
			BM_BUTTON_KEY1,			// MENU_BUTTON_LEFT
			BM_BUTTON_KEY3,			// MENU_BUTTON_RIGHT
			BM_BUTTON_SCRATCHUP,	// MENU_BUTTON_UP
			BM_BUTTON_SCRATCHDOWN,	// MENU_BUTTON_DOWN
			BM_BUTTON_START,		// MENU_BUTTON_START
			BM_BUTTON_SELECT,		// MENU_BUTTON_BACK
			BM_BUTTON_COIN,// MENU_BUTTON_COIN
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_m,					// BM_BUTTON_KEY1,
				SDLK_k,					// BM_BUTTON_KEY2,
				SDLK_COMMA,				// BM_BUTTON_KEY3,
				SDLK_l,					// BM_BUTTON_KEY4,
				SDLK_PERIOD,			// BM_BUTTON_KEY5,
				SDLK_SEMICOLON,			// BM_BUTTON_KEY6,
				SDLK_SLASH,				// BM_BUTTON_KEY7,
				SDLK_LSHIFT,			// BM_BUTTON_SCRATCHUP,
				-1,						// BM_BUTTON_SCRATCHDOWN,
				SDLK_RETURN,			// BM_BUTTON_START,
				-1,						// BM_BUTTON_SELECT,
				SDLK_LEFT,				// BM_BUTTON_MENULEFT
				SDLK_RIGHT,				// BM_BUTTON_MENURIGHT
				SDLK_UP,				// BM_BUTTON_MENUUP
				SDLK_DOWN,				// BM_BUTTON_MENUDOWN
				SDLK_F1,				// BM_BUTTON_COIN
			},
			{	// PLAYER_2
				-1,					// BM_BUTTON_KEY1,
				-1,					// BM_BUTTON_KEY2,
				-1,					// BM_BUTTON_KEY3,
				-1,					// BM_BUTTON_KEY4,
				-1,  				// BM_BUTTON_KEY5,
				-1,  				// BM_BUTTON_KEY6,
				-1,  				// BM_BUTTON_KEY7,
				-1,					// BM_BUTTON_SCRATCHUP,
				-1,					// BM_BUTTON_SCRATCHDOWN,
				-1,					// BM_BUTTON_START,
				-1,					// BM_BUTTON_SELECT,
				-1,					// BM_BUTTON_MENULEFT
				-1,					// BM_BUTTON_MENURIGHT
				-1,					// BM_BUTTON_MENUUP
				-1,					// BM_BUTTON_MENUDOWN
				-1,					// BM_BUTTON_COIN
			},
		}
	},
};

StyleDef g_StyleDefs[NUM_STYLES] = 
{
	{	// STYLE_DANCE_SINGLE
		GAME_DANCE,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"single",								// m_szName
		NOTES_TYPE_DANCE_SINGLE,				// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
	},
	{	// STYLE_DANCE_VERSUS
		GAME_DANCE,								// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		"versus",								// m_szName
		NOTES_TYPE_DANCE_SINGLE,				// m_NotesType
		StyleDef::TWO_PLAYERS_TWO_CREDITS,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
	},
	{	// STYLE_DANCE_DOUBLE
		GAME_DANCE,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"double",								// m_szName
		NOTES_TYPE_DANCE_DOUBLE,				// m_NotesType
		StyleDef::ONE_PLAYER_TWO_CREDITS,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*3.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
	},
	{	// StyleDef
		GAME_DANCE,							// m_Game
		true,								// m_bUsedForGameplay
		false,								// m_bUsedForEdit
		"couple",							// m_szName
		NOTES_TYPE_DANCE_COUPLE,	// m_NotesType
		StyleDef::TWO_PLAYERS_TWO_CREDITS,	// m_StyleType
		{ 160, 480 },						// m_iCenterX
		4,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_5,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
	},
	{	// STYLE_DANCE_SOLO
		GAME_DANCE,							// m_Game
		true,								// m_bUsedForGameplay
		true,								// m_bUsedForEdit
		"solo",								// m_szName
		NOTES_TYPE_DANCE_SOLO,				// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,	// m_StyleType
		{ 320, 320 },						// m_iCenterX
		6,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,		-DANCE_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_UPLEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,		-DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,		+DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	DANCE_BUTTON_UPRIGHT,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,		+DANCE_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,		-DANCE_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	DANCE_BUTTON_UPLEFT,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,		-DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,		+DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	DANCE_BUTTON_UPRIGHT,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,		+DANCE_COL_SPACING*2.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5
		},
	},
	{	// STYLE_DANCE_EDIT_COUPLE
		GAME_DANCE,								// m_Game
		false,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"couple (edit)",						// m_szName
		NOTES_TYPE_DANCE_COUPLE,				// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*4.0f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*3.0f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,	-DANCE_COL_SPACING*2.0f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,	-DANCE_COL_SPACING*1.0f },
				{ TRACK_5,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,	+DANCE_COL_SPACING*1.0f },
				{ TRACK_6,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,	+DANCE_COL_SPACING*2.0f },
				{ TRACK_7,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*3.0f },
				{ TRACK_8,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*4.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,	-DANCE_COL_SPACING*4.0f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,	-DANCE_COL_SPACING*3.0f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,	-DANCE_COL_SPACING*2.0f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,	-DANCE_COL_SPACING*1.0f },
				{ TRACK_5,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,	+DANCE_COL_SPACING*1.0f },
				{ TRACK_6,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,	+DANCE_COL_SPACING*2.0f },
				{ TRACK_7,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,	+DANCE_COL_SPACING*3.0f },
				{ TRACK_8,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,	+DANCE_COL_SPACING*4.0f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
	},
/*	{	// STYLE_DANCE_SOLO_VERSUS 
		"dance-solo-versus",				// m_szName
		NOTES_TYPE_DANCE_SOLO,				// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,	// m_StyleType
		{ 160, 480 },						// m_iCenterX
		6,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DANCE_BUTTON_LEFT,		-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	DANCE_BUTTON_UPLEFT,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	DANCE_BUTTON_DOWN,		-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	DANCE_BUTTON_UP,		+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	DANCE_BUTTON_UPRIGHT,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_1,	DANCE_BUTTON_RIGHT,		+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	DANCE_BUTTON_LEFT,		-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	DANCE_BUTTON_UPLEFT,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	DANCE_BUTTON_DOWN,		-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	DANCE_BUTTON_UP,		+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	DANCE_BUTTON_UPRIGHT,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	DANCE_BUTTON_RIGHT,		+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,5,1,4,2,3		// outside in
		},
	},	*/
	{	// PUMP_STYLE_SINGLE
		GAME_PUMP,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"single",								// m_szName
		NOTES_TYPE_PUMP_SINGLE,					// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
	},
	{	// PUMP_STYLE_VERSUS
		GAME_PUMP,								// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		"versus",								// m_szName
		NOTES_TYPE_PUMP_SINGLE,					// m_NotesType
		StyleDef::TWO_PLAYERS_TWO_CREDITS,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
	},
	{	// PUMP_STYLE_DOUBLE
		GAME_PUMP,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"double",								// m_szName
		NOTES_TYPE_PUMP_DOUBLE,					// m_NotesType
		StyleDef::ONE_PLAYER_TWO_CREDITS,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*4.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*3.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,		-PUMP_COL_SPACING*2.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,		+PUMP_COL_SPACING*1.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*2.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*3.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*4.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*3.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,		-PUMP_COL_SPACING*2.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,		+PUMP_COL_SPACING*1.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*2.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*3.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*4.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3,5,7,9,6,8
		},
	},
	{	// STYLE_PUMP_COUPLE
		GAME_PUMP,								// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		"couple",								// m_szName
		NOTES_TYPE_PUMP_COUPLE,					// m_NotesType
		StyleDef::TWO_PLAYERS_TWO_CREDITS,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_6,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_7,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_8,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_9,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_10,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
	},
	{	// STYLE_PUMP_EDIT_COUPLE
		GAME_PUMP,								// m_Game
		false,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"couple (edit)",						// m_szName
		NOTES_TYPE_PUMP_COUPLE,					// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*5.0 },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*4.0 },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,		-PUMP_COL_SPACING*3.0 },
				{ TRACK_4,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,	-PUMP_COL_SPACING*2.0 },
				{ TRACK_5,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,	-PUMP_COL_SPACING*1.0 },
				{ TRACK_6,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	+PUMP_COL_SPACING*1.0 },
				{ TRACK_7,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,		+PUMP_COL_SPACING*2.0 },
				{ TRACK_8,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*3.0 },
				{ TRACK_9,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*4.0 },
				{ TRACK_10,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*5.0 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,		-PUMP_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,		+PUMP_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPRIGHT,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNRIGHT,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
	},
	{	// EZ2_STYLE_SINGLE
		GAME_EZ2,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"single",								// m_szName
		NOTES_TYPE_EZ2_SINGLE,					// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTDOWN,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTDOWN,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// EZ2_STYLE_REAL
		GAME_EZ2,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"real",									// m_szName
		NOTES_TYPE_EZ2_REAL,					// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		7,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDLRLEFT,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTDOWN,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDLRRIGHT,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_REAL_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDLRLEFT,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTDOWN,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDLRRIGHT,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_REAL_COL_SPACING*3 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// EZ2_STYLE_SINGLE_VERSUS
		GAME_EZ2,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"versus",								// m_szName
		NOTES_TYPE_EZ2_SINGLE,					// m_NotesType
		StyleDef::TWO_PLAYERS_TWO_CREDITS,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTDOWN,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTDOWN,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// EZ2_STYLE_REAL_VERSUS
		GAME_EZ2,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"versusReal",							// m_szName
		NOTES_TYPE_EZ2_REAL,					// m_NotesType
		StyleDef::TWO_PLAYERS_TWO_CREDITS,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		7,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDLRLEFT,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTDOWN,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDLRRIGHT,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_REAL_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDLRLEFT,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTDOWN,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDLRRIGHT,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_REAL_COL_SPACING*3 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// EZ2_STYLE_DOUBLE
		GAME_EZ2,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"double",								// m_szName
		NOTES_TYPE_EZ2_DOUBLE,					// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_COL_SPACING*4.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_COL_SPACING*3.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTDOWN,	-EZ2_COL_SPACING*2.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPRIGHT,	-EZ2_COL_SPACING*1.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPRIGHT,	-EZ2_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPLEFT,	+EZ2_COL_SPACING*0.5f }, 
				{ TRACK_7,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPLEFT,	+EZ2_COL_SPACING*1.5f },  
				{ TRACK_8,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTDOWN,	+EZ2_COL_SPACING*2.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_COL_SPACING*3.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPLEFT,	-EZ2_COL_SPACING*4.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPLEFT,	-EZ2_COL_SPACING*3.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTDOWN,	-EZ2_COL_SPACING*2.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	EZ2_BUTTON_HANDUPRIGHT,	-EZ2_COL_SPACING*1.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	EZ2_BUTTON_FOOTUPRIGHT,	-EZ2_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPLEFT,	+EZ2_COL_SPACING*0.5f }, 
				{ TRACK_7,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPLEFT,	+EZ2_COL_SPACING*1.5f },  
				{ TRACK_8,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTDOWN,	+EZ2_COL_SPACING*2.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	EZ2_BUTTON_HANDUPRIGHT,	+EZ2_COL_SPACING*3.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	EZ2_BUTTON_FOOTUPRIGHT,	+EZ2_COL_SPACING*4.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3,7,5,9,6,8 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
	},
	{	// PARA_SINGLE
		GAME_PARA,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"single",								// m_szName
		NOTES_TYPE_PARA_SINGLE,						// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PARA_BUTTON_LEFT,	-PARA_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_1,	PARA_BUTTON_UPLEFT,	-PARA_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_1,	PARA_BUTTON_UP,	+PARA_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_1,	PARA_BUTTON_UPRIGHT,	+PARA_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_1,	PARA_BUTTON_RIGHT,	+PARA_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	PARA_BUTTON_LEFT,	-PARA_COL_SPACING*2 },
				{ TRACK_2,	GAME_CONTROLLER_2,	PARA_BUTTON_UPLEFT,	-PARA_COL_SPACING*1 },
				{ TRACK_3,	GAME_CONTROLLER_2,	PARA_BUTTON_UP,	+PARA_COL_SPACING*0 },
				{ TRACK_4,	GAME_CONTROLLER_2,	PARA_BUTTON_UPRIGHT,	+PARA_COL_SPACING*1 },
				{ TRACK_5,	GAME_CONTROLLER_2,	PARA_BUTTON_RIGHT,	+PARA_COL_SPACING*2 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 
		},
	},
	{	// DS3DDX_SINGLE
		GAME_DS3DDX,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"single",								// m_szName
		NOTES_TYPE_DS3DDX_SINGLE,						// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_HANDLEFT,	-DS3DDX_COL_SPACING*3 },
				{ TRACK_2,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_FOOTDOWNLEFT,	-DS3DDX_COL_SPACING*2 },
				{ TRACK_3,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_FOOTUPLEFT,	-DS3DDX_COL_SPACING*1 },
				{ TRACK_4,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_HANDUP,	-DS3DDX_COL_SPACING*0 },
				{ TRACK_5,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_HANDDOWN,	+DS3DDX_COL_SPACING*0 },
				{ TRACK_6,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_FOOTUPRIGHT,	+DS3DDX_COL_SPACING*1 },
				{ TRACK_7,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_FOOTDOWNRIGHT,	+DS3DDX_COL_SPACING*2 },
				{ TRACK_8,	GAME_CONTROLLER_1,	DS3DDX_BUTTON_HANDRIGHT,	+DS3DDX_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_HANDLEFT,	-DS3DDX_COL_SPACING*3 },
				{ TRACK_2,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_FOOTDOWNLEFT,	-DS3DDX_COL_SPACING*2 },
				{ TRACK_3,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_FOOTUPLEFT,	-DS3DDX_COL_SPACING*1 },
				{ TRACK_4,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_HANDUP,	-DS3DDX_COL_SPACING*0 },
				{ TRACK_5,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_HANDDOWN,	+DS3DDX_COL_SPACING*0 },
				{ TRACK_6,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_FOOTUPRIGHT,	+DS3DDX_COL_SPACING*1 },
				{ TRACK_7,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_FOOTDOWNRIGHT,	+DS3DDX_COL_SPACING*2 },
				{ TRACK_8,	GAME_CONTROLLER_2,	DS3DDX_BUTTON_HANDRIGHT,	+DS3DDX_COL_SPACING*3 },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
	},
	{
		GAME_BM,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"BM-single",						// m_szName
		NOTES_TYPE_BM_SINGLE,					// m_NotesType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		6,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	BM_BUTTON_KEY1,		-BM_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	BM_BUTTON_KEY2,		-BM_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	BM_BUTTON_KEY3,		-BM_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	BM_BUTTON_KEY4,		+BM_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	BM_BUTTON_KEY5,		+BM_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_1,	BM_BUTTON_SCRATCHUP,+BM_COL_SPACING*3.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	BM_BUTTON_KEY1,		-BM_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	BM_BUTTON_KEY2,		-BM_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	BM_BUTTON_KEY3,		-BM_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	BM_BUTTON_KEY4,		+BM_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	BM_BUTTON_KEY5,		+BM_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	BM_BUTTON_SCRATCHUP,+BM_COL_SPACING*3.0f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5
		},
	}
};


ModeChoice g_ModeChoices[] = 
{
	{
		GAME_DANCE,
		PLAY_MODE_ARCADE,
		STYLE_DANCE_SINGLE,
		DIFFICULTY_MEDIUM,
		"single",
		1
	},
	{
		GAME_DANCE,
		PLAY_MODE_ARCADE,
		STYLE_DANCE_VERSUS,
		DIFFICULTY_MEDIUM,
		"versus",
		2
	},
	{
		GAME_DANCE,
		PLAY_MODE_ARCADE,
		STYLE_DANCE_DOUBLE,
		DIFFICULTY_MEDIUM,
		"double",
		2
	},
	{
		GAME_DANCE,
		PLAY_MODE_ARCADE,
		STYLE_DANCE_COUPLE,
		DIFFICULTY_MEDIUM,
		"couple",
		2
	},
	{
		GAME_DANCE,
		PLAY_MODE_ARCADE,
		STYLE_DANCE_SOLO,
		DIFFICULTY_MEDIUM,
		"solo",
		1
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_SINGLE,
		DIFFICULTY_EASY,
		"normal",
		1
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_SINGLE,
		DIFFICULTY_MEDIUM,
		"hard",
		1
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_SINGLE,
		DIFFICULTY_HARD,
		"crazy",
		1
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_VERSUS,
		DIFFICULTY_EASY,
		"normal",
		2
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_VERSUS,
		DIFFICULTY_MEDIUM,
		"hard",
		2
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_VERSUS,
		DIFFICULTY_HARD,
		"crazy",
		2
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_COUPLE,
		DIFFICULTY_MEDIUM,
		"battle",
		2
	},
	{
		GAME_PUMP,
		PLAY_MODE_ARCADE,
		STYLE_PUMP_DOUBLE,
		DIFFICULTY_MEDIUM,
		"double",
		1
	},
	{
		GAME_PUMP,
		PLAY_MODE_ONI,
		STYLE_PUMP_SINGLE,
		DIFFICULTY_MEDIUM,
		"nonstop",
		1
	},
	{
		GAME_PUMP,
		PLAY_MODE_ONI,
		STYLE_PUMP_VERSUS,
		DIFFICULTY_MEDIUM,
		"nonstop",
		2
	},
	{
		GAME_EZ2,
		PLAY_MODE_ARCADE,
		STYLE_EZ2_SINGLE,
		DIFFICULTY_EASY,
		"easy",
		1
	},
	{
		GAME_EZ2,
		PLAY_MODE_ARCADE,
		STYLE_EZ2_SINGLE,
		DIFFICULTY_HARD,
		"hard",
		1
	},
	{
		GAME_EZ2,
		PLAY_MODE_ARCADE,
		STYLE_EZ2_REAL,
		DIFFICULTY_MEDIUM,
		"real",
		1
	},
	{
		GAME_EZ2,
		PLAY_MODE_ARCADE,
		STYLE_EZ2_DOUBLE,
		DIFFICULTY_MEDIUM,
		"club",
		1
	},
	{
		GAME_EZ2,
		PLAY_MODE_ARCADE,
		STYLE_EZ2_SINGLE_VERSUS,
		DIFFICULTY_EASY,
		"easy",
		2
	},
	{
		GAME_EZ2,
		PLAY_MODE_ARCADE,
		STYLE_EZ2_SINGLE_VERSUS,
		DIFFICULTY_HARD,
		"hard",
		2
	},
	{
		GAME_EZ2,
		PLAY_MODE_ARCADE,
		STYLE_EZ2_REAL_VERSUS,
		DIFFICULTY_MEDIUM,
		"real",
		2
	},
	{
		GAME_PARA,
		PLAY_MODE_ARCADE,
		STYLE_PARA_SINGLE,
		DIFFICULTY_EASY,
		"para",
		1
	},
	{
		GAME_PARA,
		PLAY_MODE_ARCADE,
		STYLE_PARA_SINGLE,
		DIFFICULTY_EASY,
		"easy",
		1
	},
	{
		GAME_PARA,
		PLAY_MODE_ARCADE,
		STYLE_PARA_SINGLE,
		DIFFICULTY_MEDIUM,
		"hard",
		1
	},
	{
		GAME_PARA,
		PLAY_MODE_ARCADE,
		STYLE_PARA_SINGLE,
		DIFFICULTY_HARD,
		"expert",
		1
	},
	{
		GAME_DS3DDX,
		PLAY_MODE_ARCADE,
		STYLE_DS3DDX_SINGLE,
		DIFFICULTY_EASY,
		"pretty",
		1
	},
	{
		GAME_DS3DDX,
		PLAY_MODE_ARCADE,
		STYLE_DS3DDX_SINGLE,
		DIFFICULTY_MEDIUM,
		"power",
		1
	},
	{
		GAME_DS3DDX,
		PLAY_MODE_ARCADE,
		STYLE_DS3DDX_SINGLE,
		DIFFICULTY_HARD,
		"power2",
		1
	},
	{
		GAME_BM,
		PLAY_MODE_ARCADE,
		STYLE_BM_SINGLE,
		DIFFICULTY_EASY,
		"beginner",
		1
	},
	{
		GAME_BM,
		PLAY_MODE_ARCADE,
		STYLE_BM_SINGLE,
		DIFFICULTY_MEDIUM,
		"normal",
		1
	},
	{
		GAME_BM,
		PLAY_MODE_ARCADE,
		STYLE_BM_SINGLE,
		DIFFICULTY_HARD,
		"expert",
		1
	},
};
const int NUM_MODE_CHOICES = sizeof(g_ModeChoices) / sizeof(g_ModeChoices[0]);


GameManager::GameManager()
{
	m_pIniFile = new IniFile;
}

GameManager::~GameManager()
{
	delete m_pIniFile;
}

GameDef* GameManager::GetGameDefForGame( Game g )
{
	ASSERT( g != GAME_INVALID ); 	// the game must be set before calling this
	return &g_GameDefs[ g ];
}

const StyleDef* GameManager::GetStyleDefForStyle( Style s )
{
	ASSERT( s != STYLE_NONE );	// the style must be set before calling this
	return &g_StyleDefs[ s ];
}

void GameManager::GetGameplayStylesForGame( Game game, vector<Style>& aStylesAddTo, bool editor )
{
	for( int s=0; s<NUM_STYLES; s++ ) {
		if( g_StyleDefs[s].m_Game != game)
			continue;
		if( !editor && !g_StyleDefs[s].m_bUsedForGameplay )	
			continue;
		if( editor && !g_StyleDefs[s].m_bUsedForEdit )	
			continue;

		aStylesAddTo.push_back( (Style)s );
	}
}

void GameManager::GetModesChoicesForGame( Game game, vector<ModeChoice*>& apChoicesAddTo )
{
	for( int s=0; s<NUM_MODE_CHOICES; s++ )
		if( g_ModeChoices[s].game == game)
			apChoicesAddTo.push_back( &g_ModeChoices[s] );
}

void GameManager::GetNotesTypesForGame( Game game, vector<NotesType>& aNotesTypeAddTo )
{
	for( int nt=0; nt<NUM_NOTES_TYPES; nt++ )
	{
		bool found = false;
		for( int s=0; !found && s<NUM_STYLES; s++ )
		{
			if( g_StyleDefs[s].m_Game != game )
				continue;
			for( int pl = 0; !found && pl < NUM_PLAYERS; ++pl)
			{
				if( g_StyleDefs[s].m_NotesType != nt )	
					continue;

				found=true;
			}
		}
		if(found)
			aNotesTypeAddTo.push_back( (NotesType)nt );
	}
}

bool GameManager::DoesNoteSkinExist( CString sSkinName ) const
{
	CStringArray asSkinNames;	
	GetNoteSkinNames( asSkinNames );
	for( unsigned i=0; i<asSkinNames.size(); i++ )
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
		ASSERT( !as.empty() );
		SwitchNoteSkin( as[0] );
	}
	else
	{
		m_sCurNoteSkin = sNewNoteSkin;
		m_pIniFile->Reset();
		CString sPath = GetCurNoteSkinDir() + "metrics.ini";
		m_pIniFile->SetPath(sPath);
		if( !m_pIniFile->ReadFile() )
			RageException::Throw( "Could not read NoteSkin metrics file '%s'", sPath.GetString() );
	}
}

CString GameManager::GetCurNoteSkinDir()
{
	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();

	return NOTESKIN_DIR + ssprintf("%s\\%s\\", pGameDef->m_szName, m_sCurNoteSkin.GetString());
}

CString GameManager::GetMetric( CString sClassName, CString sValue )	// looks in GAMESTATE for the current Style
{
	CString sReturn;
	if( !m_pIniFile->GetValue( sClassName, sValue, sReturn ) )
		RageException::Throw( "Could not read metric '%s - %s' from '%smetrics.ini'", sClassName.GetString(), sValue.GetString(), GetCurNoteSkinDir().GetString() );
	return sReturn;
}

int GameManager::GetMetricI( CString sClassName, CString sValueName )
{
	return atoi( GetMetric(sClassName,sValueName) );
}

float GameManager::GetMetricF( CString sClassName, CString sValueName )
{
	return (float)atof( GetMetric(sClassName,sValueName) );
}

bool GameManager::GetMetricB( CString sClassName, CString sValueName )
{
	return atoi( GetMetric(sClassName,sValueName) ) != 0;
}

RageColor GameManager::GetMetricC( CString sClassName, CString sValueName )
{
	float r=1,b=1,g=1,a=1;	// initialize in case sscanf fails
	CString sValue = GetMetric(sClassName,sValueName);
	char szValue[40];
	strncpy( szValue, sValue, 39 );
	int result = sscanf( szValue, "%f,%f,%f,%f", &r, &g, &b, &a );
	if( result != 4 )
	{
		LOG->Warn( "The color value '%s' for theme metric '%s : %s' is invalid.", szValue, sClassName.GetString(), sValueName.GetString() );
		ASSERT(0);
	}

	return RageColor(r,g,b,a);
}

CString GameManager::GetPathTo( const int col, CString sElementName )	// looks in GAMESTATE for the current Style
{
	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();

	StyleInput SI( PLAYER_1, col );
	GameInput GI = pStyleDef->StyleInputToGameInput( SI );
	CString sButtonName = pGameDef->m_szButtonNames[GI.button];

	const CString sDir = GetCurNoteSkinDir();

	CStringArray arrayPossibleFileNames;		// fill this with the possible files

	GetDirListing( ssprintf("%s%s %s*.sprite", sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.png",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.jpg",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.bmp",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*.gif",    sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );
	GetDirListing( ssprintf("%s%s %s*",        sDir.GetString(), sButtonName.GetString(), sElementName.GetString()), arrayPossibleFileNames, false, true );

	if( arrayPossibleFileNames.empty() )
		RageException::Throw( "The NoteSkin element '%s %s' is missing from '%s'.", sButtonName.GetString(), sElementName.GetString(), sDir.GetString() );

	return DerefRedir(arrayPossibleFileNames[0]);
}

void GameManager::GetEnabledGames( vector<Game>& aGamesOut )
{
	for( int g=0; g<NUM_GAMES; g++ )
	{
		Game game = (Game)g;
		CStringArray asNoteSkins;
		GetNoteSkinNames( game, asNoteSkins );
		if( !asNoteSkins.empty() )
			aGamesOut.push_back( game );
	}
}

void GameManager::GetNoteSkinNames( Game game, CStringArray &AddTo ) const
{
	GameDef* pGameDef = GAMEMAN->GetGameDefForGame( game );

	CString sBaseSkinFolder = NOTESKIN_DIR + pGameDef->m_szName + "\\";
	GetDirListing( sBaseSkinFolder + "*", AddTo, true );

	// strip out "CVS"
	for( int i=AddTo.size()-1; i>=0; i-- )
		if( 0 == stricmp("cvs", AddTo[i]) )
			AddTo.erase( AddTo.begin()+i, AddTo.begin()+i+1 );
}

void GameManager::GetNoteSkinNames( CStringArray &AddTo ) const
{
	GetNoteSkinNames( GAMESTATE->m_CurGame, AddTo );
}

int GameManager::NotesTypeToNumTracks( NotesType nt )
{
	if(nt >= NUM_NOTES_TYPES)
	{
		// invalid NotesType
		ASSERT(0);
		return -1;
	}

	return NotesTypes[nt].NumTracks;
}

NotesType GameManager::StringToNotesType( CString sNotesType )
{
	sNotesType.MakeLower();

	// HACK!  We elminitated "ez2-single-hard", but we should still handle it.
	if( sNotesType == "ez2-single-hard" )
		sNotesType = "ez2-single";

	// HACK!  "para-single" used to be called just "para"
	if( sNotesType == "para" )
		sNotesType = "para-single";

	for( int i=0; i<NUM_NOTES_TYPES; i++ )
		if( NotesTypes[i].name == sNotesType )
			return NotesType(i);
	
	// invalid NotesType
	LOG->Warn( "Invalid NotesType string '%s' encountered.  Assuming this is 'dance-single'.", sNotesType.GetString() );
	return NOTES_TYPE_DANCE_SINGLE;
}

Game GameManager::StringToGameType( CString sGameType )
{
	for( int i=0; i<NUM_GAMES; i++ )
		if( !sGameType.CompareNoCase(g_GameDefs[i].m_szName) )
			return Game(i);

	return GAME_INVALID;
}

CString GameManager::NotesTypeToString( NotesType nt )
{
	if(nt >= NUM_NOTES_TYPES)
	{
		// invalid NotesType
		ASSERT(0);
		return "";
	}

	return NotesTypes[nt].name;
}
