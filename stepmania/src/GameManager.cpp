#include "global.h"
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
#include "NoteSkinManager.h"
#include "SDL_keysym.h"		// for SDLKeys


GameManager*	GAMEMAN = NULL;	// global and accessable from anywhere in our program


const int DANCE_COL_SPACING = 64;
const int PUMP_COL_SPACING = 50;
const int EZ2_COL_SPACING = 46; 
const int EZ2_REAL_COL_SPACING = 40;
const int PARA_COL_SPACING = 54;
const int DS3DDX_COL_SPACING = 46;
const int BM_COL_SPACING=34;
const int MANIAX_COL_SPACING = 36;
const int TECHNO_COL_SPACING = 64;
const int PNM5_COL_SPACING = 32; 
const int PNM9_COL_SPACING = 32; 

struct {
	char *name;
	int NumTracks;
} const NotesTypes[NUM_STEPS_TYPES] = {
	{ "dance-single",	4 },
	{ "dance-double",	8 },
	{ "dance-couple",	8 },
	{ "dance-solo",		6 },
	{ "pump-single",	5 },
	{ "pump-halfdouble",6 },
	{ "pump-double",	10 },
	{ "pump-couple",	10 },
	{ "ez2-single",		5 },	// Single: TL,LHH,D,RHH,TR
	{ "ez2-double",		10 },	// Double: Single x2
	{ "ez2-real",		7 },	// Real: TL,LHH,LHL,D,RHL,RHH,TR
	{ "para-single",	5 },
	{ "ds3ddx-single",	8 },
	{ "bm-single",		6 },
// MD 10/26/03 - for future support
// MD 10/28/03 - The future is here!
	{ "bm-double",      12 },
	{ "bm-single7",     8 },
	{ "bm-double7",     16 },
	{ "maniax-single",	4 },
	{ "maniax-double",	8 },
	{ "techno-single8", 8 },
	{ "pnm-five", 5 },
	{ "pnm-nine", 9 },
};

//
// Important:  Every game must define the buttons: "Start", "Back", "MenuLeft", "Operator" and "MenuRight"
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
			"Operator",
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
			"",
		},
		{	// m_DedicatedMenuButton
			DANCE_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			DANCE_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			DANCE_BUTTON_MENUUP,	// MENU_BUTTON_UP
			DANCE_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			DANCE_BUTTON_START,		// MENU_BUTTON_START
			DANCE_BUTTON_BACK,		// MENU_BUTTON_BACK
			DANCE_BUTTON_COIN,		// MENU_BUTTON_COIN
			DANCE_BUTTON_OPERATOR	// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			DANCE_BUTTON_LEFT,		// MENU_BUTTON_LEFT
			DANCE_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
			DANCE_BUTTON_UP,		// MENU_BUTTON_UP
			DANCE_BUTTON_DOWN,		// MENU_BUTTON_DOWN
			DANCE_BUTTON_START,		// MENU_BUTTON_START
			DANCE_BUTTON_BACK,		// MENU_BUTTON_BACK
			DANCE_BUTTON_COIN,		// MENU_BUTTON_COIN
			DANCE_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_LEFT,				// DANCE_BUTTON_LEFT,
				SDLK_RIGHT,				// DANCE_BUTTON_RIGHT,
				SDLK_UP,				// DANCE_BUTTON_UP,
				SDLK_DOWN,				// DANCE_BUTTON_DOWN,
				-1, //no default key 	// DANCE_BUTTON_UPLEFT,
				-1, //no default key	// DANCE_BUTTON_UPRIGHT,
				SDLK_RETURN,			// DANCE_BUTTON_START,
				SDLK_ESCAPE,			// DANCE_BUTTON_BACK
				SDLK_DELETE,			// DANCE_BUTTON_MENULEFT
				SDLK_PAGEDOWN,			// DANCE_BUTTON_MENURIGHT
				SDLK_HOME,				// DANCE_BUTTON_MENUUP
				SDLK_END,				// DANCE_BUTTON_MENUDOWN
				SDLK_F1,				// DANCE_BUTTON_COIN
				SDLK_SCROLLOCK			// DANCE_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				SDLK_KP4,				// DANCE_BUTTON_LEFT,
				SDLK_KP6,				// DANCE_BUTTON_RIGHT,
				SDLK_KP8,				// DANCE_BUTTON_UP,
				SDLK_KP2,				// DANCE_BUTTON_DOWN,
				SDLK_KP7,				// DANCE_BUTTON_UPLEFT,
				SDLK_KP9,				// DANCE_BUTTON_UPRIGHT,
				SDLK_KP_ENTER,			// DANCE_BUTTON_START,
				SDLK_KP0,				// DANCE_BUTTON_BACK
				SDLK_KP_DIVIDE,			// DANCE_BUTTON_MENULEFT
				SDLK_KP_MULTIPLY,		// DANCE_BUTTON_MENURIGHT
				SDLK_KP_MINUS,			// DANCE_BUTTON_MENUUP
				SDLK_KP_PLUS,			// DANCE_BUTTON_MENUDOWN
				-1,						// DANCE_BUTTON_COIN
				-1						// DANCE_BUTTON_OPERATOR
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
			"Operator",
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
			"",
		},
		{	// m_DedicatedMenuButton
			PUMP_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			PUMP_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			PUMP_BUTTON_MENUUP,		// MENU_BUTTON_UP
			PUMP_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			PUMP_BUTTON_START,		// MENU_BUTTON_START
			PUMP_BUTTON_BACK,		// MENU_BUTTON_BACK
			PUMP_BUTTON_COIN,		// MENU_BUTTON_COIN
			PUMP_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			PUMP_BUTTON_DOWNLEFT,	// MENU_BUTTON_LEFT
			PUMP_BUTTON_DOWNRIGHT,	// MENU_BUTTON_RIGHT
			PUMP_BUTTON_UPLEFT,		// MENU_BUTTON_UP
			PUMP_BUTTON_UPRIGHT,	// MENU_BUTTON_DOWN
			PUMP_BUTTON_CENTER,		// MENU_BUTTON_START
			PUMP_BUTTON_BACK,		// MENU_BUTTON_BACK
			PUMP_BUTTON_COIN,		// MENU_BUTTON_COIN
			PUMP_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_q,					// PUMP_BUTTON_UPLEFT,
				SDLK_e,					// PUMP_BUTTON_UPRIGHT,
				SDLK_s,					// PUMP_BUTTON_CENTER,
				SDLK_z,					// PUMP_BUTTON_DOWNLEFT,
				SDLK_c,  				// PUMP_BUTTON_DOWNRIGHT,
				SDLK_RETURN,			// PUMP_BUTTON_START,
				SDLK_ESCAPE,			// PUMP_BUTTON_BACK,
				SDLK_LEFT,				// PUMP_BUTTON_MENULEFT
				SDLK_RIGHT,				// PUMP_BUTTON_MENURIGHT
				SDLK_UP,				// PUMP_BUTTON_MENUUP
				SDLK_DOWN,				// PUMP_BUTTON_MENUDOWN
				SDLK_F1,				// PUMP_BUTTON_COIN
				SDLK_SCROLLOCK			// PUMP_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				SDLK_KP7,				// PUMP_BUTTON_UPLEFT,
				SDLK_KP9,				// PUMP_BUTTON_UPRIGHT,
				SDLK_KP5,				// PUMP_BUTTON_CENTER,
				SDLK_KP1,				// PUMP_BUTTON_DOWNLEFT,
				SDLK_KP3,  				// PUMP_BUTTON_DOWNRIGHT,
				SDLK_KP_ENTER,			// PUMP_BUTTON_START,
				SDLK_KP0,				// PUMP_BUTTON_BACK,
				-1,	//no default key	// PUMP_BUTTON_MENULEFT
				-1, //no default key	// PUMP_BUTTON_MENURIGHT
				-1, //no default key	// PUMP_BUTTON_MENUUP
				-1, //no default key	// PUMP_BUTTON_MENUDOWN
				-1,						// PUMP_BUTTON_COIN
				-1						// PUMP_BUTTON_OPERATOR
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
			"Operator",
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
			"",
		},
		{	// m_DedicatedMenuButton
			EZ2_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			EZ2_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			EZ2_BUTTON_MENUUP,		// MENU_BUTTON_UP
			EZ2_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			EZ2_BUTTON_START,		// MENU_BUTTON_START
			EZ2_BUTTON_BACK,		// MENU_BUTTON_BACK
			EZ2_BUTTON_COIN,		// MENU_BUTTON_COIN
			EZ2_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			EZ2_BUTTON_HANDUPLEFT,	// MENU_BUTTON_LEFT
			EZ2_BUTTON_HANDUPRIGHT,	// MENU_BUTTON_RIGHT
			EZ2_BUTTON_FOOTUPLEFT,	// MENU_BUTTON_UP
			EZ2_BUTTON_FOOTUPRIGHT,	// MENU_BUTTON_DOWN
			EZ2_BUTTON_FOOTDOWN,	// MENU_BUTTON_START
			EZ2_BUTTON_BACK,		// MENU_BUTTON_BACK
			EZ2_BUTTON_COIN,		// MENU_BUTTON_COIN
			EZ2_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
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
				SDLK_RETURN,			// EZ2_BUTTON_START,
				SDLK_ESCAPE,			// EZ2_BUTTON_BACK,
				SDLK_LEFT,				// EZ2_BUTTON_MENULEFT
				SDLK_RIGHT,				// EZ2_BUTTON_MENURIGHT
				SDLK_UP,				// EZ2_BUTTON_MENUUP
				SDLK_DOWN,				// EZ2_BUTTON_MENUDOWN
				SDLK_F1,				// EZ2_BUTTON_COIN
				SDLK_SCROLLOCK			// EZ2_BUTTON_OPERATOR
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
				-1,						// EZ2_BUTTON_COIN
				-1						// EZ2_BUTTON_OPERATOR
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
			"Operator",
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
			"",
		},
		{	// m_DedicatedMenuButton
			PARA_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			PARA_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			PARA_BUTTON_MENUUP,		// MENU_BUTTON_UP
			PARA_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			PARA_BUTTON_START,		// MENU_BUTTON_START
			PARA_BUTTON_BACK,		// MENU_BUTTON_BACK
			PARA_BUTTON_COIN,		// MENU_BUTTON_COIN
			PARA_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			PARA_BUTTON_LEFT,		// MENU_BUTTON_LEFT
			PARA_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
			PARA_BUTTON_UPRIGHT,	// MENU_BUTTON_UP
			PARA_BUTTON_UPLEFT,		// MENU_BUTTON_DOWN
			PARA_BUTTON_START,		// MENU_BUTTON_START
			PARA_BUTTON_BACK,		// MENU_BUTTON_BACK
			PARA_BUTTON_COIN,		// MENU_BUTTON_COIN
			PARA_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_z,							// PARA_BUTTON_LEFT,
				SDLK_x,							// PARA_BUTTON_UPLEFT,
				SDLK_c,							// PARA_BUTTON_UP,
				SDLK_v,							// PARA_BUTTON_UPRIGHT,
				SDLK_b,							// PARA_BUTTON_RIGHT,
				SDLK_RETURN,					// PARA_BUTTON_START,
				SDLK_ESCAPE,					// PARA_BUTTON_BACK
				SDLK_LEFT, //no default key		// PARA_BUTTON_MENULEFT
				SDLK_RIGHT, //no default key	// PARA_BUTTON_MENURIGHT
				SDLK_UP,						// PARA_BUTTON_MENUUP
				SDLK_DOWN,						// PARA_BUTTON_MENUDOWN
				SDLK_F1,						// PARA_BUTTON_COIN
				SDLK_SCROLLOCK					// PARA_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				-1,						// PARA_BUTTON_LEFT,
				-1,						// PARA_BUTTON_UPLEFT,
				-1,						// PARA_BUTTON_UP,
				-1,						// PARA_BUTTON_UPRIGHT,
				-1,						// PARA_BUTTON_RIGHT,
				-1,						// PARA_BUTTON_START,
				-1,						// PARA_BUTTON_BACK
				-1, //no default key	// PARA_BUTTON_MENULEFT
				-1, //no default key	// PARA_BUTTON_MENURIGHT
				-1,						// PARA_BUTTON_MENUUP
				-1,	
				-1,						// PARA_BUTTON_COIN
				-1						// PARA_BUTTON_OPERATOR
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
			"Operator",
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
			"",
		},
		{	// m_DedicatedMenuButton
			DS3DDX_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
			DS3DDX_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			DS3DDX_BUTTON_MENUUP,		// MENU_BUTTON_UP
			DS3DDX_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
			DS3DDX_BUTTON_START,		// MENU_BUTTON_START
			DS3DDX_BUTTON_BACK,			// MENU_BUTTON_BACK
			DS3DDX_BUTTON_COIN,			// MENU_BUTTON_COIN
			DS3DDX_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			DS3DDX_BUTTON_HANDLEFT,		// MENU_BUTTON_LEFT
			DS3DDX_BUTTON_HANDRIGHT,	// MENU_BUTTON_RIGHT
			DS3DDX_BUTTON_HANDUP,		// MENU_BUTTON_UP
			DS3DDX_BUTTON_HANDDOWN,		// MENU_BUTTON_DOWN
			DS3DDX_BUTTON_START,		// MENU_BUTTON_START
			DS3DDX_BUTTON_BACK,			// MENU_BUTTON_BACK
			DS3DDX_BUTTON_COIN,			// MENU_BUTTON_COIN
			DS3DDX_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_a,							// DS3DDX_BUTTON_HANDLEFT,
				SDLK_z,							// DS3DDX_BUTTON_FOOTDOWNLEFT,
				SDLK_q,							// DS3DDX_BUTTON_FOOTUPLEFT,
				SDLK_w,							// DS3DDX_BUTTON_HANDUP,
				SDLK_x,							// DS3DDX_BUTTON_HANDDOWN,
				SDLK_e,							// DS3DDX_BUTTON_FOOTUPRIGHT,
				SDLK_c,							// DS3DDX_BUTTON_FOOTDOWNRIGHT,
				SDLK_d,							// DS3DDX_BUTTON_HANDRIGHT,
				SDLK_RETURN,					// DS3DDX_BUTTON_START,
				SDLK_ESCAPE,					// DS3DDX_BUTTON_BACK
				SDLK_LEFT, //no default key		// DS3DDX_BUTTON_MENULEFT
				SDLK_RIGHT, //no default key	// DS3DDX_BUTTON_MENURIGHT
				SDLK_UP,						// DS3DDX_BUTTON_MENUUP
				SDLK_DOWN,						// DS3DDX_BUTTON_MENUDOWN
				SDLK_F1,						// DS3DDX_BUTTON_COIN
				SDLK_SCROLLOCK					// DS3DDX_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				-1,						// DS3DDX_BUTTON_HANDLEFT,
				-1,						// DS3DDX_BUTTON_FOOTDOWNLEFT,
				-1,						// DS3DDX_BUTTON_FOOTUPLEFT,
				-1,						// DS3DDX_BUTTON_HANDUP,
				-1,						// DS3DDX_BUTTON_HANDDOWN,
				-1,						// DS3DDX_BUTTON_FOOTUPRIGHT,
				-1,						// DS3DDX_BUTTON_FOOTDOWNRIGHT,
				-1,						// DS3DDX_BUTTON_HANDRIGHT,
				-1,						// DS3DDX_BUTTON_START,
				-1,						// DS3DDX_BUTTON_BACK
				-1, //no default key	// DS3DDX_BUTTON_MENULEFT
				-1, //no default key	// DS3DDX_BUTTON_MENURIGHT
				-1,						// DS3DDX_BUTTON_MENUUP
				-1,						// DS3DDX_BUTTON_MENUDOWN
				-1,						// DS3DDX_BUTTON_COIN
				-1						// DS3DDX_BUTTON_OPERATOR
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
			"Operator",
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
			"",
		},
		{	// m_DedicatedMenuButton
			BM_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
			BM_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			BM_BUTTON_MENUUP,		// MENU_BUTTON_UP
			BM_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
			BM_BUTTON_START,		// MENU_BUTTON_START
			BM_BUTTON_SELECT,		// MENU_BUTTON_BACK
			BM_BUTTON_COIN,			// MENU_BUTTON_COIN
			BM_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			BM_BUTTON_KEY1,			// MENU_BUTTON_LEFT
			BM_BUTTON_KEY3,			// MENU_BUTTON_RIGHT
			BM_BUTTON_SCRATCHUP,	// MENU_BUTTON_UP
			BM_BUTTON_SCRATCHDOWN,	// MENU_BUTTON_DOWN
			BM_BUTTON_START,		// MENU_BUTTON_START
			BM_BUTTON_SELECT,		// MENU_BUTTON_BACK
			BM_BUTTON_COIN,			// MENU_BUTTON_COIN
			BM_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
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
				SDLK_SCROLLOCK			// BM_BUTTON_OPERATOR
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
				-1					// BM_BUTTON_OPERATOR
			},
		}
	},
	{	// GAME_MANIAX
		"maniax",					// m_szName
		"Dance Maniax",				// m_szDescription
		2,							// m_iNumControllers
		NUM_MANIAX_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
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
			"Operator",
		},
		{	// m_szSecondaryFunction
			"(MenuLeft)",
			"(MenuRight)",
			"(MenuDown)",
			"(MenuUp)",
			"",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
			"",
		},
		{	// m_DedicatedMenuButton
			MANIAX_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
			MANIAX_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			MANIAX_BUTTON_MENUUP,		// MENU_BUTTON_UP
			MANIAX_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
			MANIAX_BUTTON_START,		// MENU_BUTTON_START
			MANIAX_BUTTON_BACK,			// MENU_BUTTON_BACK
			MANIAX_BUTTON_COIN,			// MENU_BUTTON_COIN
			MANIAX_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			MANIAX_BUTTON_HANDUPLEFT,	// MENU_BUTTON_LEFT
			MANIAX_BUTTON_HANDUPRIGHT,	// MENU_BUTTON_RIGHT
			MANIAX_BUTTON_HANDLRRIGHT,	// MENU_BUTTON_UP
			MANIAX_BUTTON_HANDLRLEFT,	// MENU_BUTTON_DOWN
			MANIAX_BUTTON_START,		// MENU_BUTTON_START
			MANIAX_BUTTON_BACK,			// MENU_BUTTON_BACK
			MANIAX_BUTTON_COIN,			// MENU_BUTTON_COIN
			MANIAX_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_a,				// MANIAX_BUTTON_HANDUPLEFT,
				SDLK_s,				// MANIAX_BUTTON_HANDUPRIGHT,
				SDLK_z,				// MANIAX_BUTTON_HANDLRLEFT,
				SDLK_x,				// MANIAX_BUTTON_HANDLRRIGHT,
				SDLK_RETURN,		// MANIAX_BUTTON_START,
				SDLK_ESCAPE,		// MANIAX_BUTTON_BACK
				SDLK_LEFT,			// MANIAX_BUTTON_MENULEFT
				SDLK_RIGHT,			// MANIAX_BUTTON_MENURIGHT
				SDLK_UP,			// MANIAX_BUTTON_MENUUP
				SDLK_DOWN,			// MANIAX_BUTTON_MENUDOWN
				SDLK_F1,			// MANIAX_BUTTON_COIN
				SDLK_SCROLLOCK		// MANIAX_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				SDLK_KP4,			// MANIAX_BUTTON_HANDUPLEFT,
				SDLK_KP5,			// MANIAX_BUTTON_HANDUPRIGHT,
				SDLK_KP1,			// MANIAX_BUTTON_HANDLRLEFT,
				SDLK_KP2,			// MANIAX_BUTTON_HANDLRRIGHT,
				SDLK_KP_ENTER,		// MANIAX_BUTTON_START,
				SDLK_KP0,			// MANIAX_BUTTON_BACK
				SDLK_KP_DIVIDE,		// MANIAX_BUTTON_MENULEFT
				SDLK_KP_MULTIPLY,	// MANIAX_BUTTON_MENURIGHT
				SDLK_KP_MINUS,		// MANIAX_BUTTON_MENUUP
				SDLK_KP_PLUS,		// MANIAX_BUTTON_MENUDOWN
				-1,					// MANIAX_BUTTON_COIN
				-1					// MANIAX_BUTTON_OPERATOR
			},
		}
	},
	{	// GAME_MANIAX
		"techno",					// m_szName
		"TechnoMotion",				// m_szDescription
		2,							// m_iNumControllers
		NUM_TECHNO_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			"Left",
			"Right",
			"Up",
			"Down",
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
			"Operator",
		},
		{	// m_szSecondaryFunction
			"(MenuLeft)",
			"(MenuRight)",
			"(MenuDown)",
			"(MenuUp)",
			"",
			"",
			"",
			"",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
			"",
		},
		{	// m_DedicatedMenuButton
			TECHNO_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
			TECHNO_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			TECHNO_BUTTON_MENUUP,		// MENU_BUTTON_UP
			TECHNO_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
			TECHNO_BUTTON_START,		// MENU_BUTTON_START
			TECHNO_BUTTON_BACK,			// MENU_BUTTON_BACK
			TECHNO_BUTTON_COIN,			// MENU_BUTTON_COIN
			TECHNO_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			TECHNO_BUTTON_LEFT,			// MENU_BUTTON_LEFT
			TECHNO_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
			TECHNO_BUTTON_UP,			// MENU_BUTTON_UP
			TECHNO_BUTTON_DOWN,			// MENU_BUTTON_DOWN
			TECHNO_BUTTON_START,		// MENU_BUTTON_START
			TECHNO_BUTTON_BACK,			// MENU_BUTTON_BACK
			TECHNO_BUTTON_COIN,			// MENU_BUTTON_COIN
			TECHNO_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_a,				// TECHNO_BUTTON_LEFT,
				SDLK_d,				// TECHNO_BUTTON_RIGHT,
				SDLK_w,				// TECHNO_BUTTON_UP,
				SDLK_x,				// TECHNO_BUTTON_DOWN,
				SDLK_q,				// TECHNO_BUTTON_UPLEFT,
				SDLK_e,				// TECHNO_BUTTON_UPRIGHT,
				SDLK_s,				// TECHNO_BUTTON_CENTER,
				SDLK_z,				// TECHNO_BUTTON_DOWNLEFT,
				SDLK_c,				// TECHNO_BUTTON_DOWNRIGHT,
				SDLK_RETURN,		// TECHNO_BUTTON_START,
				SDLK_ESCAPE,		// TECHNO_BUTTON_BACK
				SDLK_LEFT,			// TECHNO_BUTTON_MENULEFT
				SDLK_RIGHT,			// TECHNO_BUTTON_MENURIGHT
				SDLK_UP,			// TECHNO_BUTTON_MENUUP
				SDLK_DOWN,			// TECHNO_BUTTON_MENUDOWN
				SDLK_F1,			// TECHNO_BUTTON_COIN
				SDLK_SCROLLOCK		// TECHNO_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				SDLK_KP4,			// TECHNO_BUTTON_LEFT,
				SDLK_KP6,			// TECHNO_BUTTON_RIGHT,
				SDLK_KP8,			// TECHNO_BUTTON_UP,
				SDLK_KP2,			// TECHNO_BUTTON_DOWN,
				SDLK_KP7,			// TECHNO_BUTTON_UPLEFT,
				SDLK_KP9,			// TECHNO_BUTTON_UPRIGHT,
				SDLK_KP5,			// TECHNO_BUTTON_CENTER,
				SDLK_KP1,			// TECHNO_BUTTON_DOWNLEFT,
				SDLK_KP3,			// TECHNO_BUTTON_DOWNRIGHT,
				SDLK_KP_ENTER,		// TECHNO_BUTTON_START,
				SDLK_KP0,			// TECHNO_BUTTON_BACK
				SDLK_KP_DIVIDE,		// TECHNO_BUTTON_MENULEFT
				SDLK_KP_MULTIPLY,	// TECHNO_BUTTON_MENURIGHT
				SDLK_KP_MINUS,		// TECHNO_BUTTON_MENUUP
				SDLK_KP_PLUS,		// TECHNO_BUTTON_MENUDOWN
				-1,					// TECHNO_BUTTON_COIN
				-1					// TECHNO_BUTTON_OPERATOR
			},
		}
	},
	{	// GAME_PNM
		"pnm",				// m_szName
		"Pop'n Music",		// m_szDescription
		1,					// m_iNumControllers
		NUM_PNM_BUTTONS,	// m_iButtonsPerController
		{	// m_szButtonNames
			"Left White",
			"Left Yellow",
			"Left Green",
			"Left Blue",
			"Red",
			"Right Blue",
			"Right Green",
			"Right Yellow",
			"Right White",
			"Back",
			"Start",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
			"Operator",
		},
		{	// m_szSecondaryFunction
			"",
			"(MenuUp)",
			"",
			"(MenuLeft)",
			"",
			"(MenuRight)",
			"",
			"(MenuDown)",
			"",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"(dedicated)",
			"",
			"",
		},
		{	// m_DedicatedMenuButton
			PNM_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			PNM_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			PNM_BUTTON_MENUUP,	// MENU_BUTTON_UP
			PNM_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			PNM_BUTTON_START,		// MENU_BUTTON_START
			PNM_BUTTON_BACK,		// MENU_BUTTON_BACK
			PNM_BUTTON_COIN,		// MENU_BUTTON_COIN
			PNM_BUTTON_OPERATOR	// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			PNM_BUTTON_LEFT_BLUE,		
			PNM_BUTTON_RIGHT_BLUE,		
			PNM_BUTTON_LEFT_YELLOW,		
			PNM_BUTTON_RIGHT_YELLOW,	
			PNM_BUTTON_RED,	
			PNM_BUTTON_BACK,	
			PNM_BUTTON_COIN,			// MENU_BUTTON_COIN
			PNM_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				SDLK_z,					// PNM_BUTTON_LEFT_WHITE,
				SDLK_s,					// PNM_BUTTON_LEFT_YELLOW,
				SDLK_x,				// PNM_BUTTON_LEFT_GREEN,
				SDLK_d,					// PNM_BUTTON_LEFT_BLUE,
				SDLK_c,			// PNM_BUTTON_RED,
				SDLK_f,			// PNM_BUTTON_RIGHT_BLUE,
				SDLK_v,				// PNM_BUTTON_RIGHT_GREEN,
				SDLK_g,			// PNM_BUTTON_RIGHT_YELLOW,
				SDLK_b,						// PNM_BUTTON_RIGHT_WHITE,
				SDLK_RETURN,				// PNM_BUTTON_MENUSTART
				SDLK_ESCAPE,				// PNM_BUTTON_MENUBACK		
				SDLK_LEFT,			// PNM_BUTTON_MENULEFT,
				SDLK_RIGHT,						// PNM_BUTTON_MENURIGHT,
				SDLK_UP,				// PNM_BUTTON_MENUUP
				SDLK_DOWN,				// PNM_BUTTON_MENUDOWN
				SDLK_F1,				// PNM_BUTTON_COIN
				SDLK_SCROLLOCK			// PNM_BUTTON_OPERATOR
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
				-1					// BM_BUTTON_OPERATOR
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
		STEPS_TYPE_DANCE_SINGLE,				// m_StepsType
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
		STEPS_TYPE_DANCE_SINGLE,				// m_StepsType
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
		STEPS_TYPE_DANCE_DOUBLE,				// m_StepsType
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
	{	// STYLE_DANCE_COUPLE
		GAME_DANCE,							// m_Game
		true,								// m_bUsedForGameplay
		false,								// m_bUsedForEdit
		"couple",							// m_szName
		STEPS_TYPE_DANCE_COUPLE,	// m_StepsType
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
		STEPS_TYPE_DANCE_SOLO,				// m_StepsType
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
		STEPS_TYPE_DANCE_COUPLE,				// m_StepsType
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
		STEPS_TYPE_DANCE_SOLO,				// m_StepsType
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
		STEPS_TYPE_PUMP_SINGLE,					// m_StepsType
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
		STEPS_TYPE_PUMP_SINGLE,					// m_StepsType
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
	{	// PUMP_STYLE_HALFDOUBLE
		GAME_PUMP,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"halfdouble",								// m_szName
		STEPS_TYPE_PUMP_HALFDOUBLE,					// m_StepsType
		StyleDef::ONE_PLAYER_TWO_CREDITS,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		6,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,	-PUMP_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,		-PUMP_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,		-PUMP_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,	+PUMP_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,	+PUMP_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_1,	PUMP_BUTTON_CENTER,	-PUMP_COL_SPACING*2.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	PUMP_BUTTON_UPRIGHT,		-PUMP_COL_SPACING*1.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	PUMP_BUTTON_DOWNRIGHT,		-PUMP_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	PUMP_BUTTON_DOWNLEFT,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	PUMP_BUTTON_UPLEFT,	+PUMP_COL_SPACING*1.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	PUMP_BUTTON_CENTER,	+PUMP_COL_SPACING*2.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,5,2,4,3,1
		},
	},
	{	// PUMP_STYLE_DOUBLE
		GAME_PUMP,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"double",								// m_szName
		STEPS_TYPE_PUMP_DOUBLE,					// m_StepsType
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
		STEPS_TYPE_PUMP_COUPLE,					// m_StepsType
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
		STEPS_TYPE_PUMP_COUPLE,					// m_StepsType
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
		STEPS_TYPE_EZ2_SINGLE,					// m_StepsType
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
		STEPS_TYPE_EZ2_REAL,					// m_StepsType
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
		STEPS_TYPE_EZ2_SINGLE,					// m_StepsType
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
		STEPS_TYPE_EZ2_REAL,					// m_StepsType
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
		STEPS_TYPE_EZ2_DOUBLE,					// m_StepsType
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
		STEPS_TYPE_PARA_SINGLE,						// m_StepsType
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
		STEPS_TYPE_DS3DDX_SINGLE,						// m_StepsType
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
	{	// STYLE_BM_SINGLE
		GAME_BM,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"BM-single",						// m_szName
		STEPS_TYPE_BM_SINGLE,					// m_StepsType
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
	},
// MD 10/28/03 - New additions.
	{	// STYLE_BM_DOUBLE
		GAME_BM,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"BM-double",						// m_szName
		STEPS_TYPE_BM_DOUBLE,					// m_StepsType
		StyleDef::ONE_PLAYER_TWO_CREDITS,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		12,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	BM_BUTTON_KEY1,		-BM_COL_SPACING*6.0f },
				{ TRACK_2,	GAME_CONTROLLER_1,	BM_BUTTON_KEY2,		-BM_COL_SPACING*5.0f },
				{ TRACK_3,	GAME_CONTROLLER_1,	BM_BUTTON_KEY3,		-BM_COL_SPACING*4.0f },
				{ TRACK_4,	GAME_CONTROLLER_1,	BM_BUTTON_KEY4,		-BM_COL_SPACING*3.0f },
				{ TRACK_5,	GAME_CONTROLLER_1,	BM_BUTTON_KEY5,		-BM_COL_SPACING*2.0f },
				{ TRACK_6,	GAME_CONTROLLER_1,	BM_BUTTON_SCRATCHUP,-BM_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	BM_BUTTON_KEY1,		+BM_COL_SPACING*0.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	BM_BUTTON_KEY2,		+BM_COL_SPACING*1.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	BM_BUTTON_KEY3,		+BM_COL_SPACING*2.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	BM_BUTTON_KEY4,		+BM_COL_SPACING*3.5f },
				{ TRACK_11,	GAME_CONTROLLER_2,	BM_BUTTON_KEY5,		+BM_COL_SPACING*4.5f },
				{ TRACK_12,	GAME_CONTROLLER_2,	BM_BUTTON_SCRATCHUP,+BM_COL_SPACING*6.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_1,	BM_BUTTON_KEY1,		-BM_COL_SPACING*6.0f },
				{ TRACK_2,	GAME_CONTROLLER_1,	BM_BUTTON_KEY2,		-BM_COL_SPACING*5.0f },
				{ TRACK_3,	GAME_CONTROLLER_1,	BM_BUTTON_KEY3,		-BM_COL_SPACING*4.0f },
				{ TRACK_4,	GAME_CONTROLLER_1,	BM_BUTTON_KEY4,		-BM_COL_SPACING*3.0f },
				{ TRACK_5,	GAME_CONTROLLER_1,	BM_BUTTON_KEY5,		-BM_COL_SPACING*2.0f },
				{ TRACK_6,	GAME_CONTROLLER_1,	BM_BUTTON_SCRATCHUP,-BM_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	BM_BUTTON_KEY1,		+BM_COL_SPACING*0.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	BM_BUTTON_KEY2,		+BM_COL_SPACING*1.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	BM_BUTTON_KEY3,		+BM_COL_SPACING*2.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	BM_BUTTON_KEY4,		+BM_COL_SPACING*3.5f },
				{ TRACK_11,	GAME_CONTROLLER_2,	BM_BUTTON_KEY5,		+BM_COL_SPACING*4.5f },
				{ TRACK_12,	GAME_CONTROLLER_2,	BM_BUTTON_SCRATCHUP,+BM_COL_SPACING*6.0f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8,9,10,11
		},
	},
	{	// STYLE_BM_SINGLE7
		GAME_BM,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"BM-single7",						// m_szName
		STEPS_TYPE_BM_SINGLE,					// m_StepsType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_8,	GAME_CONTROLLER_1,	BM_BUTTON_SCRATCHUP,-BM_COL_SPACING*3.5f },
				{ TRACK_1,	GAME_CONTROLLER_1,	BM_BUTTON_KEY1,		-BM_COL_SPACING*2.0f },
				{ TRACK_2,	GAME_CONTROLLER_1,	BM_BUTTON_KEY2,		-BM_COL_SPACING*1.0f },
				{ TRACK_3,	GAME_CONTROLLER_1,	BM_BUTTON_KEY3,		-BM_COL_SPACING*0.0f },
				{ TRACK_4,	GAME_CONTROLLER_1,	BM_BUTTON_KEY4,		+BM_COL_SPACING*1.0f },
				{ TRACK_5,	GAME_CONTROLLER_1,	BM_BUTTON_KEY5,		+BM_COL_SPACING*2.0f },
				{ TRACK_6,	GAME_CONTROLLER_1,	BM_BUTTON_KEY6,		+BM_COL_SPACING*3.0f },
				{ TRACK_7,	GAME_CONTROLLER_1,	BM_BUTTON_KEY7,		+BM_COL_SPACING*4.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	BM_BUTTON_KEY1,		-BM_COL_SPACING*3.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	BM_BUTTON_KEY2,		-BM_COL_SPACING*2.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	BM_BUTTON_KEY3,		-BM_COL_SPACING*1.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	BM_BUTTON_KEY4,		-BM_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	BM_BUTTON_KEY5,		+BM_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	BM_BUTTON_KEY6,		+BM_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	BM_BUTTON_KEY7,		+BM_COL_SPACING*2.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	BM_BUTTON_SCRATCHUP,+BM_COL_SPACING*4.0f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
	},
	{	// STYLE_BM_DOUBLE7
		GAME_BM,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"BM-double7",						// m_szName
		STEPS_TYPE_BM_DOUBLE7,					// m_StepsType
		StyleDef::ONE_PLAYER_TWO_CREDITS,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		16,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_8,	GAME_CONTROLLER_1,	BM_BUTTON_SCRATCHUP,-BM_COL_SPACING*8.0f },
				{ TRACK_1,	GAME_CONTROLLER_1,	BM_BUTTON_KEY1,		-BM_COL_SPACING*6.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	BM_BUTTON_KEY2,		-BM_COL_SPACING*5.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	BM_BUTTON_KEY3,		-BM_COL_SPACING*4.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	BM_BUTTON_KEY4,		-BM_COL_SPACING*3.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	BM_BUTTON_KEY5,		-BM_COL_SPACING*2.5f },
				{ TRACK_6,	GAME_CONTROLLER_1,	BM_BUTTON_KEY6,		-BM_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_1,	BM_BUTTON_KEY7,		-BM_COL_SPACING*0.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	BM_BUTTON_KEY1,		+BM_COL_SPACING*0.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	BM_BUTTON_KEY2,		+BM_COL_SPACING*1.5f },
				{ TRACK_11,	GAME_CONTROLLER_2,	BM_BUTTON_KEY3,		+BM_COL_SPACING*2.5f },
				{ TRACK_12,	GAME_CONTROLLER_2,	BM_BUTTON_KEY4,		+BM_COL_SPACING*3.5f },
				{ TRACK_13,	GAME_CONTROLLER_2,	BM_BUTTON_KEY5,		+BM_COL_SPACING*4.5f },
				{ TRACK_14,	GAME_CONTROLLER_2,	BM_BUTTON_KEY6,		+BM_COL_SPACING*5.5f },
				{ TRACK_15,	GAME_CONTROLLER_2,	BM_BUTTON_KEY7,		+BM_COL_SPACING*6.5f },
				{ TRACK_16,	GAME_CONTROLLER_2,	BM_BUTTON_SCRATCHUP,+BM_COL_SPACING*8.0f },
			},
			{	// PLAYER_2
				{ TRACK_8,	GAME_CONTROLLER_1,	BM_BUTTON_SCRATCHUP,-BM_COL_SPACING*8.0f },
				{ TRACK_1,	GAME_CONTROLLER_1,	BM_BUTTON_KEY1,		-BM_COL_SPACING*6.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	BM_BUTTON_KEY2,		-BM_COL_SPACING*5.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	BM_BUTTON_KEY3,		-BM_COL_SPACING*4.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	BM_BUTTON_KEY4,		-BM_COL_SPACING*3.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	BM_BUTTON_KEY5,		-BM_COL_SPACING*2.5f },
				{ TRACK_6,	GAME_CONTROLLER_1,	BM_BUTTON_KEY6,		-BM_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_1,	BM_BUTTON_KEY7,		-BM_COL_SPACING*0.5f },
				{ TRACK_9,	GAME_CONTROLLER_2,	BM_BUTTON_KEY1,		+BM_COL_SPACING*0.5f },
				{ TRACK_10,	GAME_CONTROLLER_2,	BM_BUTTON_KEY2,		+BM_COL_SPACING*1.5f },
				{ TRACK_11,	GAME_CONTROLLER_2,	BM_BUTTON_KEY3,		+BM_COL_SPACING*2.5f },
				{ TRACK_12,	GAME_CONTROLLER_2,	BM_BUTTON_KEY4,		+BM_COL_SPACING*3.5f },
				{ TRACK_13,	GAME_CONTROLLER_2,	BM_BUTTON_KEY5,		+BM_COL_SPACING*4.5f },
				{ TRACK_14,	GAME_CONTROLLER_2,	BM_BUTTON_KEY6,		+BM_COL_SPACING*5.5f },
				{ TRACK_15,	GAME_CONTROLLER_2,	BM_BUTTON_KEY7,		+BM_COL_SPACING*6.5f },
				{ TRACK_16,	GAME_CONTROLLER_2,	BM_BUTTON_SCRATCHUP,+BM_COL_SPACING*8.0f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
		},
	},
// end MD 10/28/03
	{	// STYLE_MANIAX_SINGLE
		GAME_MANIAX,								// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"single",								// m_szName
		STEPS_TYPE_MANIAX_SINGLE,				// m_StepsType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRLEFT,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPLEFT,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPRIGHT,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRRIGHT,	+MANIAX_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRLEFT,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPLEFT,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPRIGHT,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRRIGHT,	+MANIAX_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
	},
	{	// STYLE_MANIAX_VERSUS
		GAME_MANIAX,							// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		"versus",								// m_szName
		STEPS_TYPE_MANIAX_SINGLE,				// m_StepsType
		StyleDef::TWO_PLAYERS_TWO_CREDITS,		// m_StyleType
		{ 160, 480 },							// m_iCenterX
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRLEFT,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPLEFT,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPRIGHT,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRRIGHT,	+MANIAX_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRLEFT,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPLEFT,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPRIGHT,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRRIGHT,	+MANIAX_COL_SPACING*1.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
	},
	{	// STYLE_MANIAX_DOUBLE
		GAME_MANIAX,							// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"double",								// m_szName
		STEPS_TYPE_MANIAX_DOUBLE,				// m_StepsType
		StyleDef::ONE_PLAYER_TWO_CREDITS,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRLEFT,	-MANIAX_COL_SPACING*3.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPLEFT,	-MANIAX_COL_SPACING*2.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPRIGHT,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRRIGHT,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRLEFT,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPLEFT,	+MANIAX_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPRIGHT,	+MANIAX_COL_SPACING*2.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRRIGHT,	+MANIAX_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRLEFT,	-MANIAX_COL_SPACING*3.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPLEFT,	-MANIAX_COL_SPACING*2.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDUPRIGHT,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	MANIAX_BUTTON_HANDLRRIGHT,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRLEFT,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPLEFT,	+MANIAX_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDUPRIGHT,	+MANIAX_COL_SPACING*2.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	MANIAX_BUTTON_HANDLRRIGHT,	+MANIAX_COL_SPACING*3.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
	},
	{	// STYLE_TECHNO_SINGLE8
		GAME_TECHNO,							// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"single8",								// m_szName
		STEPS_TYPE_TECHNO_SINGLE8,				// m_StepsType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	TECHNO_BUTTON_DOWNLEFT,	-TECHNO_COL_SPACING*3.5f },
				{ TRACK_2,	GAME_CONTROLLER_1,	TECHNO_BUTTON_LEFT,		-TECHNO_COL_SPACING*2.5f },
				{ TRACK_3,	GAME_CONTROLLER_1,	TECHNO_BUTTON_UPLEFT,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_4,	GAME_CONTROLLER_1,	TECHNO_BUTTON_DOWN,		-TECHNO_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_1,	TECHNO_BUTTON_UP,		+TECHNO_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_1,	TECHNO_BUTTON_UPRIGHT,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_1,	TECHNO_BUTTON_RIGHT,	+TECHNO_COL_SPACING*2.5f },
				{ TRACK_8,	GAME_CONTROLLER_1,	TECHNO_BUTTON_DOWNRIGHT,+TECHNO_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	TECHNO_BUTTON_DOWNLEFT,	-TECHNO_COL_SPACING*3.5f },
				{ TRACK_2,	GAME_CONTROLLER_2,	TECHNO_BUTTON_LEFT,		-TECHNO_COL_SPACING*2.5f },
				{ TRACK_3,	GAME_CONTROLLER_2,	TECHNO_BUTTON_UPLEFT,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_4,	GAME_CONTROLLER_2,	TECHNO_BUTTON_DOWN,		-TECHNO_COL_SPACING*0.5f },
				{ TRACK_5,	GAME_CONTROLLER_2,	TECHNO_BUTTON_UP,		+TECHNO_COL_SPACING*0.5f },
				{ TRACK_6,	GAME_CONTROLLER_2,	TECHNO_BUTTON_UPRIGHT,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_7,	GAME_CONTROLLER_2,	TECHNO_BUTTON_RIGHT,	+TECHNO_COL_SPACING*2.5f },
				{ TRACK_8,	GAME_CONTROLLER_2,	TECHNO_BUTTON_DOWNRIGHT,+TECHNO_COL_SPACING*3.5f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
	},
	{	// STYLE_PNM_FIVE
		GAME_PNM,							// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"pnm-five",								// m_szName
		STEPS_TYPE_PNM_FIVE,				// m_StepsType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PNM_BUTTON_LEFT_GREEN,	-PNM5_COL_SPACING*2.0f },
				{ TRACK_2,	GAME_CONTROLLER_1,	PNM_BUTTON_LEFT_BLUE,		-PNM5_COL_SPACING*1.0f },
				{ TRACK_3,	GAME_CONTROLLER_1,	PNM_BUTTON_RED,	-PNM5_COL_SPACING*0.0f },
				{ TRACK_4,	GAME_CONTROLLER_1,	PNM_BUTTON_RIGHT_BLUE,		+PNM5_COL_SPACING*1.0f },
				{ TRACK_5,	GAME_CONTROLLER_1,	PNM_BUTTON_RIGHT_GREEN,		+PNM5_COL_SPACING*2.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	PNM_BUTTON_LEFT_GREEN,	-PNM5_COL_SPACING*2.0f },
				{ TRACK_2,	GAME_CONTROLLER_2,	PNM_BUTTON_LEFT_BLUE,		-PNM5_COL_SPACING*1.0f },
				{ TRACK_3,	GAME_CONTROLLER_2,	PNM_BUTTON_RED,	-PNM5_COL_SPACING*0.0f },
				{ TRACK_4,	GAME_CONTROLLER_2,	PNM_BUTTON_RIGHT_BLUE,		+PNM5_COL_SPACING*1.0f },
				{ TRACK_5,	GAME_CONTROLLER_2,	PNM_BUTTON_RIGHT_GREEN,		+PNM5_COL_SPACING*2.0f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4
		},
	},
	{	// STYLE_PNM_NINE
		GAME_PNM,							// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		"pnm-nine",								// m_szName
		STEPS_TYPE_PNM_NINE,				// m_StepsType
		StyleDef::ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		{ 320, 320 },							// m_iCenterX
		9,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	GAME_CONTROLLER_1,	PNM_BUTTON_LEFT_WHITE,	-PNM9_COL_SPACING*4.0f },
				{ TRACK_2,	GAME_CONTROLLER_1,	PNM_BUTTON_LEFT_YELLOW,		-PNM9_COL_SPACING*3.0f },
				{ TRACK_3,	GAME_CONTROLLER_1,	PNM_BUTTON_LEFT_GREEN,	-PNM9_COL_SPACING*2.0f },
				{ TRACK_4,	GAME_CONTROLLER_1,	PNM_BUTTON_LEFT_BLUE,		-PNM9_COL_SPACING*1.0f },
				{ TRACK_5,	GAME_CONTROLLER_1,	PNM_BUTTON_RED,	-PNM9_COL_SPACING*0.0f },
				{ TRACK_6,	GAME_CONTROLLER_1,	PNM_BUTTON_RIGHT_BLUE,		+PNM9_COL_SPACING*1.0f },
				{ TRACK_7,	GAME_CONTROLLER_1,	PNM_BUTTON_RIGHT_GREEN,		+PNM9_COL_SPACING*2.0f },
				{ TRACK_8,	GAME_CONTROLLER_1,	PNM_BUTTON_RIGHT_YELLOW,	+PNM9_COL_SPACING*3.0f },
				{ TRACK_9,	GAME_CONTROLLER_1,	PNM_BUTTON_RIGHT_WHITE,		+PNM9_COL_SPACING*4.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	GAME_CONTROLLER_2,	PNM_BUTTON_LEFT_WHITE,	-PNM9_COL_SPACING*4.0f },
				{ TRACK_2,	GAME_CONTROLLER_2,	PNM_BUTTON_LEFT_YELLOW,		-PNM9_COL_SPACING*3.0f },
				{ TRACK_3,	GAME_CONTROLLER_2,	PNM_BUTTON_LEFT_GREEN,	-PNM9_COL_SPACING*2.0f },
				{ TRACK_4,	GAME_CONTROLLER_2,	PNM_BUTTON_LEFT_BLUE,		-PNM9_COL_SPACING*1.0f },
				{ TRACK_5,	GAME_CONTROLLER_2,	PNM_BUTTON_RED,	-PNM9_COL_SPACING*0.0f },
				{ TRACK_6,	GAME_CONTROLLER_2,	PNM_BUTTON_RIGHT_BLUE,		+PNM9_COL_SPACING*1.0f },
				{ TRACK_7,	GAME_CONTROLLER_2,	PNM_BUTTON_RIGHT_GREEN,		+PNM9_COL_SPACING*2.0f },
				{ TRACK_8,	GAME_CONTROLLER_2,	PNM_BUTTON_RIGHT_YELLOW,	+PNM9_COL_SPACING*3.0f },
				{ TRACK_9,	GAME_CONTROLLER_2,	PNM_BUTTON_RIGHT_WHITE,		+PNM9_COL_SPACING*4.0f },
			},
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8
		},
	},
};



GameManager::GameManager()
{
}

GameManager::~GameManager()
{
}

GameDef* GameManager::GetGameDefForGame( Game g )
{
	ASSERT( g != GAME_INVALID ); 	// the game must be set before calling this
	return &g_GameDefs[ g ];
}

const StyleDef* GameManager::GetStyleDefForStyle( Style s )
{
	ASSERT( s != STYLE_INVALID );	// the style must be set before calling this
	return &g_StyleDefs[ s ];
}

void GameManager::GetStylesForGame( Game game, vector<Style>& aStylesAddTo, bool editor )
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

Style GameManager::GetEditorStyleForNotesType( StepsType nt )
{
	for( int s=0; s<NUM_STYLES; s++ )
		if( g_StyleDefs[s].m_StepsType == nt && g_StyleDefs[s].m_bUsedForEdit )
			return (Style)s;

	ASSERT(0);	// this style doesn't have a StyleDef that can be used with the editor!
	return STYLE_INVALID;
}


void GameManager::GetNotesTypesForGame( Game game, vector<StepsType>& aNotesTypeAddTo )
{
	for( int nt=0; nt<NUM_STEPS_TYPES; nt++ )
	{
		bool found = false;
		for( int s=0; !found && s<NUM_STYLES; s++ )
		{
			if( g_StyleDefs[s].m_Game != game )
				continue;
			for( int pl = 0; !found && pl < NUM_PLAYERS; ++pl)
			{
				if( g_StyleDefs[s].m_StepsType != nt )	
					continue;

				found=true;
			}
		}
		if(found)
			aNotesTypeAddTo.push_back( (StepsType)nt );
	}
}

void GameManager::GetEnabledGames( vector<Game>& aGamesOut )
{
	for( int g=0; g<NUM_GAMES; g++ )
	{
		Game game = (Game)g;
		CStringArray asNoteSkins;
		NOTESKIN->GetNoteSkinNames( game, asNoteSkins );
		if( !asNoteSkins.empty() )
			aGamesOut.push_back( game );
	}
}

bool GameManager::IsGameEnabled( Game game )
{
	vector<Game> aGames;
	GetEnabledGames( aGames );
	return find( aGames.begin(), aGames.end(), game ) != aGames.end();
}

int GameManager::NotesTypeToNumTracks( StepsType nt )
{
	if(nt >= NUM_STEPS_TYPES)
	{
		// invalid StepsType
		ASSERT(0);
		return -1;
	}

	return NotesTypes[nt].NumTracks;
}

StepsType GameManager::StringToNotesType( CString sNotesType )
{
	sNotesType.MakeLower();

	// HACK!  We elminitated "ez2-single-hard", but we should still handle it.
	if( sNotesType == "ez2-single-hard" )
		sNotesType = "ez2-single";

	// HACK!  "para-single" used to be called just "para"
	if( sNotesType == "para" )
		sNotesType = "para-single";

	for( int i=0; i<NUM_STEPS_TYPES; i++ )
		if( NotesTypes[i].name == sNotesType )
			return StepsType(i);
	
	// invalid StepsType
	LOG->Warn( "Invalid StepsType string '%s' encountered.  Assuming this is 'dance-single'.", sNotesType.c_str() );
	return STEPS_TYPE_DANCE_SINGLE;
}

CString GameManager::NotesTypeToString( StepsType nt )
{
	if(nt >= NUM_STEPS_TYPES)
	{
		// invalid StepsType
		ASSERT(0);
		return "";
	}

	return NotesTypes[nt].name;
}

Game GameManager::StringToGameType( CString sGameType )
{
	for( int i=0; i<NUM_GAMES; i++ )
		if( !sGameType.CompareNoCase(g_GameDefs[i].m_szName) )
			return Game(i);

	return GAME_INVALID;
}


Style GameManager::GameAndStringToStyle( Game game, CString sStyle )
{
	for( unsigned s=0; s<NUM_STYLES; s++ )
	{
		Style style = (Style)s;
		const StyleDef* pStyleDef = GetStyleDefForStyle( style );
		if( pStyleDef->m_Game != game )
			continue;
		if( sStyle.CompareNoCase(pStyleDef->m_szName) == 0 )
			return style;
	}

	return STYLE_INVALID;
}
