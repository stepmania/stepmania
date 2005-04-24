#include "global.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "GameInput.h"	// for GameButton constants
#include "IniFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "NoteSkinManager.h"
#include "RageInputDevice.h"
#include "ThemeManager.h"
#include "LightsManager.h"	// for NUM_CABINET_LIGHTS
#include "Game.h"
#include "Style.h"

GameManager*	GAMEMAN = NULL;	// global and accessable from anywhere in our program

enum 
{
	TRACK_1 = 0,
	TRACK_2,
	TRACK_3,
	TRACK_4,
	TRACK_5,
	TRACK_6,
	TRACK_7,
	TRACK_8,
	TRACK_9,
	TRACK_10,
	TRACK_11,
	TRACK_12,
	TRACK_13,
	TRACK_14,
	TRACK_15,
	TRACK_16,
	// BMS reader needs 16 tracks for bm-double7
};

enum
{
	GAME_DANCE,		// Dance Dance Revolution
	GAME_PUMP,		// Pump It Up
	GAME_EZ2,		// Ez2dancer
	GAME_PARA,		// ParaPAraParadise
	GAME_DS3DDX,	// Dance Station 3DDX.
	GAME_BM,		// Beatmania IIDX
	GAME_MANIAX,	// DanceManiax
	GAME_TECHNO,	// TechnoMotion
	GAME_PNM,		// pop n music
	GAME_LIGHTS,	// cabinet lights (not really a game)
	NUM_GAMES,		// leave this at the end
	GAME_INVALID,
};


const int DANCE_COL_SPACING = 64;
const int PUMP_COL_SPACING = 50;
const int EZ2_COL_SPACING = 46; 
const int EZ2_REAL_COL_SPACING = 40;
const int PARA_COL_SPACING = 54;
const int DS3DDX_COL_SPACING = 46;
const int BM_COL_SPACING = 34;
const int MANIAX_COL_SPACING = 36;
const int TECHNO_COL_SPACING = 56;
const int TECHNO_VERSUS_COL_SPACING = 33;
const int PNM5_COL_SPACING = 32; 
const int PNM9_COL_SPACING = 32; 

struct {
	char *name;
	int NumTracks;
} const StepsTypes[NUM_STEPS_TYPES] = {
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
	{ "para-versus",	10 },
	{ "ds3ddx-single",	8 },
	{ "bm-single5",		6 },
	{ "bm-double5",      12 },
	{ "bm-single7",   8 },
	{ "bm-double7",   16 },
	{ "maniax-single",	4 },
	{ "maniax-double",	8 },
	{ "techno-single4", 4 },
	{ "techno-single5", 5 },
	{ "techno-single8", 8 },
	{ "techno-double4", 8 },
	{ "techno-double5", 10 },
	{ "pnm-five",		5 },
	{ "pnm-nine",		9 },
	{ "lights-cabinet",	NUM_CABINET_LIGHTS },
};

//
// Important:  Every game must define the buttons: "Start", "Back", "MenuLeft", "Operator" and "MenuRight"
//
Game g_Games[NUM_GAMES] = 
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
				KEY_LEFT,				// DANCE_BUTTON_LEFT,
				KEY_RIGHT,				// DANCE_BUTTON_RIGHT,
				KEY_UP,				// DANCE_BUTTON_UP,
				KEY_DOWN,				// DANCE_BUTTON_DOWN,
				NO_DEFAULT_KEY,		 	// DANCE_BUTTON_UPLEFT,
				NO_DEFAULT_KEY,			// DANCE_BUTTON_UPRIGHT,
				KEY_ENTER,			// DANCE_BUTTON_START,
				KEY_ESC,			// DANCE_BUTTON_BACK
				KEY_DEL,			// DANCE_BUTTON_MENULEFT
				KEY_PGDN,			// DANCE_BUTTON_MENURIGHT
				KEY_HOME,				// DANCE_BUTTON_MENUUP
				KEY_END,				// DANCE_BUTTON_MENUDOWN
				KEY_F1,				// DANCE_BUTTON_COIN
				KEY_SCRLLOCK			// DANCE_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				KEY_KP_C4,				// DANCE_BUTTON_LEFT,
				KEY_KP_C6,				// DANCE_BUTTON_RIGHT,
				KEY_KP_C8,				// DANCE_BUTTON_UP,
				KEY_KP_C2,				// DANCE_BUTTON_DOWN,
				KEY_KP_C7,				// DANCE_BUTTON_UPLEFT,
				KEY_KP_C9,				// DANCE_BUTTON_UPRIGHT,
				KEY_KP_ENTER,			// DANCE_BUTTON_START,
				KEY_NUMLOCK,			// DANCE_BUTTON_BACK
				KEY_KP_SLASH,			// DANCE_BUTTON_MENULEFT
				KEY_KP_ASTERISK,		// DANCE_BUTTON_MENURIGHT
				KEY_KP_HYPHEN,			// DANCE_BUTTON_MENUUP
				KEY_KP_PLUS,			// DANCE_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,			// DANCE_BUTTON_COIN
				NO_DEFAULT_KEY			// DANCE_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
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
				KEY_Cq,					// PUMP_BUTTON_UPLEFT,
				KEY_Ce,					// PUMP_BUTTON_UPRIGHT,
				KEY_Cs,					// PUMP_BUTTON_CENTER,
				KEY_Cz,					// PUMP_BUTTON_DOWNLEFT,
				KEY_Cc,  				// PUMP_BUTTON_DOWNRIGHT,
				KEY_ENTER,			// PUMP_BUTTON_START,
				KEY_ESC,			// PUMP_BUTTON_BACK,
				KEY_LEFT,				// PUMP_BUTTON_MENULEFT
				KEY_RIGHT,				// PUMP_BUTTON_MENURIGHT
				KEY_UP,				// PUMP_BUTTON_MENUUP
				KEY_DOWN,				// PUMP_BUTTON_MENUDOWN
				KEY_F1,				// PUMP_BUTTON_COIN
				KEY_SCRLLOCK			// PUMP_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				KEY_KP_C7,				// PUMP_BUTTON_UPLEFT,
				KEY_KP_C9,				// PUMP_BUTTON_UPRIGHT,
				KEY_KP_C5,				// PUMP_BUTTON_CENTER,
				KEY_KP_C1,				// PUMP_BUTTON_DOWNLEFT,
				KEY_KP_C3,  				// PUMP_BUTTON_DOWNRIGHT,
				KEY_KP_ENTER,			// PUMP_BUTTON_START,
				KEY_NUMLOCK,			// PUMP_BUTTON_BACK,
				NO_DEFAULT_KEY	// PUMP_BUTTON_MENULEFT
				NO_DEFAULT_KEY	// PUMP_BUTTON_MENURIGHT
				NO_DEFAULT_KEY	// PUMP_BUTTON_MENUUP
				NO_DEFAULT_KEY	// PUMP_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,						// PUMP_BUTTON_COIN
				NO_DEFAULT_KEY						// PUMP_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
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
				KEY_Cz,					// EZ2_BUTTON_FOOTUPLEFT,
				KEY_Cb,					// EZ2_BUTTON_FOOTUPRIGHT,
				KEY_Cc,					// EZ2_BUTTON_FOOTDOWN,
				KEY_Cx,					// EZ2_BUTTON_HANDUPLEFT,
				KEY_Cv,					// EZ2_BUTTON_HANDUPRIGHT,
				KEY_Cs,					// EZ2_BUTTON_HANDLRLEFT,
				KEY_Cf,  				// EZ2_BUTTON_HANDLRRIGHT,
				KEY_ENTER,			// EZ2_BUTTON_START,
				KEY_ESC,			// EZ2_BUTTON_BACK,
				KEY_LEFT,				// EZ2_BUTTON_MENULEFT
				KEY_RIGHT,				// EZ2_BUTTON_MENURIGHT
				KEY_UP,				// EZ2_BUTTON_MENUUP
				KEY_DOWN,				// EZ2_BUTTON_MENUDOWN
				KEY_F1,				// EZ2_BUTTON_COIN
				KEY_SCRLLOCK			// EZ2_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				NO_DEFAULT_KEY,						// EZ2_BUTTON_FOOTUPLEFT,
				NO_DEFAULT_KEY,						// EZ2_BUTTON_FOOTUPRIGHT,
				NO_DEFAULT_KEY,						// EZ2_BUTTON_FOOTDOWN,
				NO_DEFAULT_KEY,						// EZ2_BUTTON_HANDUPLEFT,
				NO_DEFAULT_KEY,						// EZ2_BUTTON_HANDUPRIGHT,
				NO_DEFAULT_KEY,						// EZ2_BUTTON_HANDLRLEFT,
				NO_DEFAULT_KEY,  					// EZ2_BUTTON_HANDLRRIGHT,
				NO_DEFAULT_KEY,						// EZ2_BUTTON_START,
				NO_DEFAULT_KEY,						// EZ2_BUTTON_BACK,
				NO_DEFAULT_KEY	// EZ2_BUTTON_MENULEFT
				NO_DEFAULT_KEY	// EZ2_BUTTON_MENURIGHT
				NO_DEFAULT_KEY	// EZ2_BUTTON_MENUUP
				NO_DEFAULT_KEY	// EZ2_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,						// EZ2_BUTTON_COIN
				NO_DEFAULT_KEY						// EZ2_BUTTON_OPERATOR
			},
		},
		TNS_PERFECT,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_PERFECT,	// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_MISS,		// m_mapBooTo
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
				KEY_Cz,							// PARA_BUTTON_LEFT,
				KEY_Cx,							// PARA_BUTTON_UPLEFT,
				KEY_Cc,							// PARA_BUTTON_UP,
				KEY_Cv,							// PARA_BUTTON_UPRIGHT,
				KEY_Cb,							// PARA_BUTTON_RIGHT,
				KEY_ENTER,					// PARA_BUTTON_START,
				KEY_ESC,					// PARA_BUTTON_BACK
				KEY_LEFT, //no default key		// PARA_BUTTON_MENULEFT
				KEY_RIGHT, //no default key	// PARA_BUTTON_MENURIGHT
				KEY_UP,						// PARA_BUTTON_MENUUP
				KEY_DOWN,						// PARA_BUTTON_MENUDOWN
				KEY_F1,						// PARA_BUTTON_COIN
				KEY_SCRLLOCK					// PARA_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				NO_DEFAULT_KEY,						// PARA_BUTTON_LEFT,
				NO_DEFAULT_KEY,						// PARA_BUTTON_UPLEFT,
				NO_DEFAULT_KEY,						// PARA_BUTTON_UP,
				NO_DEFAULT_KEY,						// PARA_BUTTON_UPRIGHT,
				NO_DEFAULT_KEY,						// PARA_BUTTON_RIGHT,
				NO_DEFAULT_KEY,						// PARA_BUTTON_START,
				NO_DEFAULT_KEY,						// PARA_BUTTON_BACK
				NO_DEFAULT_KEY,						// PARA_BUTTON_MENULEFT
				NO_DEFAULT_KEY,						// PARA_BUTTON_MENURIGHT
				NO_DEFAULT_KEY,						// PARA_BUTTON_MENUUP
				NO_DEFAULT_KEY,	
				NO_DEFAULT_KEY,						// PARA_BUTTON_COIN
				NO_DEFAULT_KEY						// PARA_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
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
				KEY_Ca,							// DS3DDX_BUTTON_HANDLEFT,
				KEY_Cz,							// DS3DDX_BUTTON_FOOTDOWNLEFT,
				KEY_Cq,							// DS3DDX_BUTTON_FOOTUPLEFT,
				KEY_Cw,							// DS3DDX_BUTTON_HANDUP,
				KEY_Cx,							// DS3DDX_BUTTON_HANDDOWN,
				KEY_Ce,							// DS3DDX_BUTTON_FOOTUPRIGHT,
				KEY_Cc,							// DS3DDX_BUTTON_FOOTDOWNRIGHT,
				KEY_Cd,							// DS3DDX_BUTTON_HANDRIGHT,
				KEY_ENTER,						// DS3DDX_BUTTON_START,
				KEY_ESC,						// DS3DDX_BUTTON_BACK
				KEY_LEFT, //no default key		// DS3DDX_BUTTON_MENULEFT
				KEY_RIGHT, //no default key		// DS3DDX_BUTTON_MENURIGHT
				KEY_UP,							// DS3DDX_BUTTON_MENUUP
				KEY_DOWN,						// DS3DDX_BUTTON_MENUDOWN
				KEY_F1,							// DS3DDX_BUTTON_COIN
				KEY_SCRLLOCK					// DS3DDX_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_HANDLEFT,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_FOOTDOWNLEFT,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_FOOTUPLEFT,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_HANDUP,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_HANDDOWN,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_FOOTUPRIGHT,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_FOOTDOWNRIGHT,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_HANDRIGHT,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_START,
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_BACK
				NO_DEFAULT_KEY	// DS3DDX_BUTTON_MENULEFT
				NO_DEFAULT_KEY	// DS3DDX_BUTTON_MENURIGHT
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_MENUUP
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,						// DS3DDX_BUTTON_COIN
				NO_DEFAULT_KEY						// DS3DDX_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
	},
	{	// GAME_BM
		"bm",				// m_szName
		"BeatMania IIDX",	// m_szDescription
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
			"Scratch up",
			"Scratch down",
			"Start",
			"Back",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
			"Operator",
		},
		{	// m_DedicatedMenuButton
			BM_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			BM_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			BM_BUTTON_MENUUP,		// MENU_BUTTON_UP
			BM_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			BM_BUTTON_START,		// MENU_BUTTON_START
			BM_BUTTON_BACK,		// MENU_BUTTON_BACK
			BM_BUTTON_COIN,		// MENU_BUTTON_COIN
			BM_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			BM_BUTTON_KEY1,		// MENU_BUTTON_LEFT
			BM_BUTTON_KEY3,		// MENU_BUTTON_RIGHT
			BM_BUTTON_SCRATCHUP,	// MENU_BUTTON_UP
			BM_BUTTON_SCRATCHDOWN,// MENU_BUTTON_DOWN
			BM_BUTTON_START,		// MENU_BUTTON_START
			BM_BUTTON_BACK,		// MENU_BUTTON_BACK
			BM_BUTTON_COIN,		// MENU_BUTTON_COIN
			BM_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				KEY_Cm,				// BM_BUTTON_KEY1,
				KEY_Ck,				// BM_BUTTON_KEY2,
				KEY_COMMA,			// BM_BUTTON_KEY3,
				KEY_Cl,				// BM_BUTTON_KEY4,
				KEY_PERIOD,			// BM_BUTTON_KEY5,
				KEY_SEMICOLON,		// BM_BUTTON_KEY6,
				KEY_SLASH,			// BM_BUTTON_KEY7,
				KEY_LSHIFT,			// BM_BUTTON_SCRATCHUP,
				NO_DEFAULT_KEY,		// BM_BUTTON_SCRATCHDOWN,
				KEY_ENTER,			// BM_BUTTON_START,
				KEY_ESC,			// BM_BUTTON_BACK,
				KEY_LEFT,			// BM_BUTTON_MENULEFT
				KEY_RIGHT,			// BM_BUTTON_MENURIGHT
				KEY_UP,				// BM_BUTTON_MENUUP
				KEY_DOWN,			// BM_BUTTON_MENUDOWN
				KEY_F1,				// BM_BUTTON_COIN
				KEY_SCRLLOCK		// BM_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY1,
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY2,
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY3,
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY4,
				NO_DEFAULT_KEY,  				// BM_BUTTON_KEY5,
				NO_DEFAULT_KEY,  				// BM_BUTTON_KEY6,
				NO_DEFAULT_KEY,  				// BM_BUTTON_KEY7,
				NO_DEFAULT_KEY,					// BM_BUTTON_SCRATCHUP,
				NO_DEFAULT_KEY,					// BM_BUTTON_SCRATCHDOWN,
				NO_DEFAULT_KEY,					// BM_BUTTON_START,
				NO_DEFAULT_KEY,					// BM_BUTTON_BACK,
				NO_DEFAULT_KEY,					// BM_BUTTON_MENULEFT
				NO_DEFAULT_KEY,					// BM_BUTTON_MENURIGHT
				NO_DEFAULT_KEY,					// BM_BUTTON_MENUUP
				NO_DEFAULT_KEY,					// BM_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,					// BM_BUTTON_COIN
				NO_DEFAULT_KEY					// BM_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
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
				KEY_Ca,				// MANIAX_BUTTON_HANDUPLEFT,
				KEY_Cs,				// MANIAX_BUTTON_HANDUPRIGHT,
				KEY_Cz,				// MANIAX_BUTTON_HANDLRLEFT,
				KEY_Cx,				// MANIAX_BUTTON_HANDLRRIGHT,
				KEY_ENTER,		// MANIAX_BUTTON_START,
				KEY_ESC,		// MANIAX_BUTTON_BACK
				KEY_LEFT,			// MANIAX_BUTTON_MENULEFT
				KEY_RIGHT,			// MANIAX_BUTTON_MENURIGHT
				KEY_UP,			// MANIAX_BUTTON_MENUUP
				KEY_DOWN,			// MANIAX_BUTTON_MENUDOWN
				KEY_F1,			// MANIAX_BUTTON_COIN
				KEY_SCRLLOCK		// MANIAX_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				KEY_KP_C4,			// MANIAX_BUTTON_HANDUPLEFT,
				KEY_KP_C5,			// MANIAX_BUTTON_HANDUPRIGHT,
				KEY_KP_C1,			// MANIAX_BUTTON_HANDLRLEFT,
				KEY_KP_C2,			// MANIAX_BUTTON_HANDLRRIGHT,
				KEY_KP_ENTER,		// MANIAX_BUTTON_START,
				KEY_NUMLOCK,		// MANIAX_BUTTON_BACK
				KEY_KP_SLASH,		// MANIAX_BUTTON_MENULEFT
				KEY_KP_ASTERISK,	// MANIAX_BUTTON_MENURIGHT
				KEY_KP_HYPHEN,		// MANIAX_BUTTON_MENUUP
				KEY_KP_PLUS,		// MANIAX_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,					// MANIAX_BUTTON_COIN
				NO_DEFAULT_KEY					// MANIAX_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
	},
	{	// GAME_TECHNO
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
				KEY_Ca,				// TECHNO_BUTTON_LEFT,
				KEY_Cd,				// TECHNO_BUTTON_RIGHT,
				KEY_Cw,				// TECHNO_BUTTON_UP,
				KEY_Cx,				// TECHNO_BUTTON_DOWN,
				KEY_Cq,				// TECHNO_BUTTON_UPLEFT,
				KEY_Ce,				// TECHNO_BUTTON_UPRIGHT,
				KEY_Cs,				// TECHNO_BUTTON_CENTER,
				KEY_Cz,				// TECHNO_BUTTON_DOWNLEFT,
				KEY_Cc,				// TECHNO_BUTTON_DOWNRIGHT,
				KEY_ENTER,		// TECHNO_BUTTON_START,
				KEY_ESC,		// TECHNO_BUTTON_BACK
				KEY_LEFT,			// TECHNO_BUTTON_MENULEFT
				KEY_RIGHT,			// TECHNO_BUTTON_MENURIGHT
				KEY_UP,			// TECHNO_BUTTON_MENUUP
				KEY_DOWN,			// TECHNO_BUTTON_MENUDOWN
				KEY_F1,			// TECHNO_BUTTON_COIN
				KEY_SCRLLOCK		// TECHNO_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				KEY_KP_C4,			// TECHNO_BUTTON_LEFT,
				KEY_KP_C6,			// TECHNO_BUTTON_RIGHT,
				KEY_KP_C8,			// TECHNO_BUTTON_UP,
				KEY_KP_C2,			// TECHNO_BUTTON_DOWN,
				KEY_KP_C7,			// TECHNO_BUTTON_UPLEFT,
				KEY_KP_C9,			// TECHNO_BUTTON_UPRIGHT,
				KEY_KP_C5,			// TECHNO_BUTTON_CENTER,
				KEY_KP_C1,			// TECHNO_BUTTON_DOWNLEFT,
				KEY_KP_C3,			// TECHNO_BUTTON_DOWNRIGHT,
				KEY_KP_ENTER,		// TECHNO_BUTTON_START,
				KEY_NUMLOCK,		// TECHNO_BUTTON_BACK
				KEY_KP_SLASH,		// TECHNO_BUTTON_MENULEFT
				KEY_KP_ASTERISK,	// TECHNO_BUTTON_MENURIGHT
				KEY_KP_HYPHEN,		// TECHNO_BUTTON_MENUUP
				KEY_KP_PLUS,		// TECHNO_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,					// TECHNO_BUTTON_COIN
				NO_DEFAULT_KEY					// TECHNO_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
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
				KEY_Cz,					// PNM_BUTTON_LEFT_WHITE,
				KEY_Cs,					// PNM_BUTTON_LEFT_YELLOW,
				KEY_Cx,				// PNM_BUTTON_LEFT_GREEN,
				KEY_Cd,					// PNM_BUTTON_LEFT_BLUE,
				KEY_Cc,			// PNM_BUTTON_RED,
				KEY_Cf,			// PNM_BUTTON_RIGHT_BLUE,
				KEY_Cv,				// PNM_BUTTON_RIGHT_GREEN,
				KEY_Cg,			// PNM_BUTTON_RIGHT_YELLOW,
				KEY_Cb,						// PNM_BUTTON_RIGHT_WHITE,
				KEY_ENTER,				// PNM_BUTTON_MENUSTART
				KEY_ESC,				// PNM_BUTTON_MENUBACK		
				KEY_LEFT,			// PNM_BUTTON_MENULEFT,
				KEY_RIGHT,						// PNM_BUTTON_MENURIGHT,
				KEY_UP,				// PNM_BUTTON_MENUUP
				KEY_DOWN,				// PNM_BUTTON_MENUDOWN
				KEY_F1,				// PNM_BUTTON_COIN
				KEY_SCRLLOCK			// PNM_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY1,
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY2,
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY3,
				NO_DEFAULT_KEY,					// BM_BUTTON_KEY4,
				NO_DEFAULT_KEY,  				// BM_BUTTON_KEY5,
				NO_DEFAULT_KEY,  				// BM_BUTTON_KEY6,
				NO_DEFAULT_KEY,  				// BM_BUTTON_KEY7,
				NO_DEFAULT_KEY,					// BM_BUTTON_SCRATCHUP,
				NO_DEFAULT_KEY,					// BM_BUTTON_SCRATCHDOWN,
				NO_DEFAULT_KEY,					// BM_BUTTON_START,
				NO_DEFAULT_KEY,					// BM_BUTTON_BACK,
				NO_DEFAULT_KEY,					// BM_BUTTON_MENULEFT
				NO_DEFAULT_KEY,					// BM_BUTTON_MENURIGHT
				NO_DEFAULT_KEY,					// BM_BUTTON_MENUUP
				NO_DEFAULT_KEY,					// BM_BUTTON_MENUDOWN
				NO_DEFAULT_KEY,					// BM_BUTTON_COIN
				NO_DEFAULT_KEY					// BM_BUTTON_OPERATOR
			},
		},
		TNS_PERFECT,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GREAT,		// m_mapGoodTo
		TNS_MISS,		// m_mapBooTo
	},
	{	// GAME_LIGHTS
		"lights",					// m_szName
		"Lights",					// m_szDescription
		1,							// m_iNumControllers
		NUM_LIGHTS_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			"MarqueeUpLeft",
			"MarqueeUpRight",
			"MarqueeLrLeft",
			"MarqueeLrRight",
			"ButtonsLeft",
			"ButtonsRight",
			"BassLeft",
			"BassRight",
			"Start",
			"Back",
			"MenuLeft",
			"MenuRight",
			"MenuUp",
			"MenuDown",
			"Insert Coin",
			"Operator",
		},
		{	// m_DedicatedMenuButton
			LIGHTS_BUTTON_MENULEFT,	// MENU_BUTTON_LEFT
			LIGHTS_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
			LIGHTS_BUTTON_MENUUP,	// MENU_BUTTON_UP
			LIGHTS_BUTTON_MENUDOWN,	// MENU_BUTTON_DOWN
			LIGHTS_BUTTON_START,		// MENU_BUTTON_START
			LIGHTS_BUTTON_BACK,		// MENU_BUTTON_BACK
			LIGHTS_BUTTON_COIN,		// MENU_BUTTON_COIN
			LIGHTS_BUTTON_OPERATOR	// MENU_BUTTON_OPERATOR
		},
		{	// m_SecondaryMenuButton
			LIGHTS_BUTTON_MARQUEE_UP_LEFT,		// MENU_BUTTON_LEFT
			LIGHTS_BUTTON_MARQUEE_UP_RIGHT,		// MENU_BUTTON_RIGHT
			LIGHTS_BUTTON_MARQUEE_LR_LEFT,		// MENU_BUTTON_UP
			LIGHTS_BUTTON_MARQUEE_LR_RIGHT,		// MENU_BUTTON_DOWN
			LIGHTS_BUTTON_START,		// MENU_BUTTON_START
			LIGHTS_BUTTON_BACK,		// MENU_BUTTON_BACK
			LIGHTS_BUTTON_COIN,		// MENU_BUTTON_COIN
			LIGHTS_BUTTON_OPERATOR,	// MENU_BUTTON_OPERATOR
		},
		{	// m_iDefaultKeyboardKey
			{	// PLAYER_1
				KEY_Cq,				// LIGHTS_BUTTON_MARQUEE_UP_LEFT,
				KEY_Cw,				// LIGHTS_BUTTON_MARQUEE_UP_RIGHT,
				KEY_Ce,				// LIGHTS_BUTTON_MARQUEE_LR_LEFT,
				KEY_Cr,				// LIGHTS_BUTTON_MARQUEE_LR_RIGHT,
				KEY_Ct,			 	// LIGHTS_BUTTON_BUTTONS_LEFT,
				KEY_Cy,					// LIGHTS_BUTTON_BUTTONS_RIGHT,
				KEY_Cu,					// LIGHTS_BUTTON_BASS_LEFT,
				KEY_Ci,					// LIGHTS_BUTTON_BASS_RIGHT,
				KEY_ENTER,			// LIGHTS_BUTTON_START,
				KEY_ESC,			// LIGHTS_BUTTON_BACK
				KEY_DEL,			// LIGHTS_BUTTON_MENULEFT
				KEY_PGDN,			// LIGHTS_BUTTON_MENURIGHT
				KEY_HOME,				// LIGHTS_BUTTON_MENUUP
				KEY_END,				// LIGHTS_BUTTON_MENUDOWN
				KEY_F1,				// LIGHTS_BUTTON_COIN
				KEY_SCRLLOCK			// LIGHTS_BUTTON_OPERATOR
			},
			{	// PLAYER_2
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MARQUEE_UP_LEFT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MARQUEE_UP_RIGHT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MARQUEE_LR_LEFT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MARQUEE_LR_RIGHT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_BUTTONS_LEFT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_BUTTONS_RIGHT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_BASS_LEFT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_BASS_RIGHT,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_START,
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_BACK
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MENULEFT
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MENURIGHT
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MENUUP
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_MENUDOWN
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_COIN
				NO_DEFAULT_KEY		// LIGHTS_BUTTON_OPERATOR
			},
		},
		TNS_MARVELOUS,	// m_mapMarvelousTo
		TNS_PERFECT,	// m_mapPerfectTo
		TNS_GREAT,		// m_mapGreatTo
		TNS_GOOD,		// m_mapGoodTo
		TNS_BOO,		// m_mapBooTo
	},
};

Style g_Styles[] = 
{
	{	// STYLE_DANCE_SINGLE
		&g_Games[GAME_DANCE],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single",								// m_szName
		STEPS_TYPE_DANCE_SINGLE,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
		false, // m_bNeedsZoomOutWith2Players
		true, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_DANCE_VERSUS
		&g_Games[GAME_DANCE],				// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versus",								// m_szName
		STEPS_TYPE_DANCE_SINGLE,				// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
		false, // m_bNeedsZoomOutWith2Players
		true, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_DANCE_DOUBLE
		&g_Games[GAME_DANCE],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double",								// m_szName
		STEPS_TYPE_DANCE_DOUBLE,				// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 4, 7, 6, 5, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_DANCE_COUPLE
		&g_Games[GAME_DANCE],			// m_Game
		true,								// m_bUsedForGameplay
		false,								// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"couple",							// m_szName
		STEPS_TYPE_DANCE_COUPLE,	// m_StepsType
		TWO_PLAYERS_TWO_SIDES,	// m_StyleType
		4,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_5,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_7,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_8,	+DANCE_COL_SPACING*1.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
		false, // m_bNeedsZoomOutWith2Players
		true, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_DANCE_SOLO
		&g_Games[GAME_DANCE],			// m_Game
		true,								// m_bUsedForGameplay
		true,								// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"solo",								// m_szName
		STEPS_TYPE_DANCE_SOLO,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,	// m_StyleType
		6,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_3,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	+DANCE_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_3,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_6,	+DANCE_COL_SPACING*2.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING },
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_DANCE_EDIT_COUPLE
		&g_Games[GAME_DANCE],				// m_Game
		false,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"couple-edit",						// m_szName
		STEPS_TYPE_DANCE_COUPLE,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*4.0f },
				{ TRACK_2,	-DANCE_COL_SPACING*3.0f },
				{ TRACK_3,	-DANCE_COL_SPACING*2.0f },
				{ TRACK_4,	-DANCE_COL_SPACING*1.0f },
				{ TRACK_5,	+DANCE_COL_SPACING*1.0f },
				{ TRACK_6,	+DANCE_COL_SPACING*2.0f },
				{ TRACK_7,	+DANCE_COL_SPACING*3.0f },
				{ TRACK_8,	+DANCE_COL_SPACING*4.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*4.0f },
				{ TRACK_2,	-DANCE_COL_SPACING*3.0f },
				{ TRACK_3,	-DANCE_COL_SPACING*2.0f },
				{ TRACK_4,	-DANCE_COL_SPACING*1.0f },
				{ TRACK_5,	+DANCE_COL_SPACING*1.0f },
				{ TRACK_6,	+DANCE_COL_SPACING*2.0f },
				{ TRACK_7,	+DANCE_COL_SPACING*3.0f },
				{ TRACK_8,	+DANCE_COL_SPACING*4.0f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, Style::END_MAPPING },
			{ 4, 5, 6, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
/*	{	// STYLE_DANCE_SOLO_VERSUS 
		"dance-solo-versus",				// m_szName
 		STEPS_TYPE_DANCE_SOLO,				// m_StepsType
		ONE_PLAYER_ONE_CREDIT,	// m_StyleType
		6,									// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
				{ TRACK_2,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_3,	-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_6,	+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
				{ TRACK_2,	-DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_3,	-DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_4,	+DANCE_6PANEL_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_6PANEL_VERSUS_COL_SPACING*1.5f },
				{ TRACK_6,	+DANCE_6PANEL_VERSUS_COL_SPACING*2.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING },
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,5,1,4,2,3		// outside in
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},	*/
	{	// STYLE_PUMP_SINGLE
		&g_Games[GAME_PUMP],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single",								// m_szName
		STEPS_TYPE_PUMP_SINGLE,					// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	-PUMP_COL_SPACING*1 },
				{ TRACK_3,	+PUMP_COL_SPACING*0 },
				{ TRACK_4,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	-PUMP_COL_SPACING*1 },
				{ TRACK_3,	+PUMP_COL_SPACING*0 },
				{ TRACK_4,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PUMP_VERSUS
		&g_Games[GAME_PUMP],					// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versus",								// m_szName
		STEPS_TYPE_PUMP_SINGLE,					// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	-PUMP_COL_SPACING*1 },
				{ TRACK_3,	+PUMP_COL_SPACING*0 },
				{ TRACK_4,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	-PUMP_COL_SPACING*1 },
				{ TRACK_3,	+PUMP_COL_SPACING*0 },
				{ TRACK_4,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PUMP_HALFDOUBLE
		&g_Games[GAME_PUMP],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"halfdouble",								// m_szName
		STEPS_TYPE_PUMP_HALFDOUBLE,					// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		6,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2.5f },
				{ TRACK_2,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_3,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_4,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_5,	+PUMP_COL_SPACING*1.5f },
				{ TRACK_6,	+PUMP_COL_SPACING*2.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2.5f },
				{ TRACK_2,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_3,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_4,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_5,	+PUMP_COL_SPACING*1.5f },
				{ TRACK_6,	+PUMP_COL_SPACING*2.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ Style::NO_MAPPING, 1, 0, Style::NO_MAPPING, 2, Style::END_MAPPING },
			{ 3, Style::NO_MAPPING, 5, 4, Style::NO_MAPPING, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,5,2,4,3,1
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PUMP_DOUBLE
		&g_Games[GAME_PUMP],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double",								// m_szName
		STEPS_TYPE_PUMP_DOUBLE,					// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*4.5f },
				{ TRACK_2,	-PUMP_COL_SPACING*3.5f },
				{ TRACK_3,	-PUMP_COL_SPACING*2.5f },
				{ TRACK_4,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_5,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_6,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_7,	+PUMP_COL_SPACING*1.5f },
				{ TRACK_8,	+PUMP_COL_SPACING*2.5f },
				{ TRACK_9,	+PUMP_COL_SPACING*3.5f },
				{ TRACK_10,	+PUMP_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*4.5f },
				{ TRACK_2,	-PUMP_COL_SPACING*3.5f },
				{ TRACK_3,	-PUMP_COL_SPACING*2.5f },
				{ TRACK_4,	-PUMP_COL_SPACING*1.5f },
				{ TRACK_5,	-PUMP_COL_SPACING*0.5f },
				{ TRACK_6,	+PUMP_COL_SPACING*0.5f },
				{ TRACK_7,	+PUMP_COL_SPACING*1.5f },
				{ TRACK_8,	+PUMP_COL_SPACING*2.5f },
				{ TRACK_9,	+PUMP_COL_SPACING*3.5f },
				{ TRACK_10,	+PUMP_COL_SPACING*4.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 6, 8, 7, 5, 9, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3,5,7,9,6,8
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PUMP_COUPLE
		&g_Games[GAME_PUMP],					// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"couple",								// m_szName
		STEPS_TYPE_PUMP_COUPLE,					// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	-PUMP_COL_SPACING*1 },
				{ TRACK_3,	+PUMP_COL_SPACING*0 },
				{ TRACK_4,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	+PUMP_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_6,	-PUMP_COL_SPACING*2 },
				{ TRACK_7,	-PUMP_COL_SPACING*1 },
				{ TRACK_8,	+PUMP_COL_SPACING*0 },
				{ TRACK_9,	+PUMP_COL_SPACING*1 },
				{ TRACK_10,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PUMP_EDIT_COUPLE
		&g_Games[GAME_PUMP],					// m_Game
		false,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"couple-edit",						// m_szName
		STEPS_TYPE_PUMP_COUPLE,					// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*5.0 },
				{ TRACK_2,	-PUMP_COL_SPACING*4.0 },
				{ TRACK_3,	-PUMP_COL_SPACING*3.0 },
				{ TRACK_4,	-PUMP_COL_SPACING*2.0 },
				{ TRACK_5,	-PUMP_COL_SPACING*1.0 },
				{ TRACK_6,	+PUMP_COL_SPACING*1.0 },
				{ TRACK_7,	+PUMP_COL_SPACING*2.0 },
				{ TRACK_8,	+PUMP_COL_SPACING*3.0 },
				{ TRACK_9,	+PUMP_COL_SPACING*4.0 },
				{ TRACK_10,	+PUMP_COL_SPACING*5.0 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2 },
				{ TRACK_2,	-PUMP_COL_SPACING*1 },
				{ TRACK_3,	+PUMP_COL_SPACING*0 },
				{ TRACK_4,	+PUMP_COL_SPACING*1 },
				{ TRACK_5,	+PUMP_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 3, 2, 0, 4, 6, 8, 7, 5, 9, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, 6, 8, 7, 5, 9, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_EZ2_SINGLE
		&g_Games[GAME_EZ2],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single",								// m_szName
		STEPS_TYPE_EZ2_SINGLE,					// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	+EZ2_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_EZ2_REAL
		&g_Games[GAME_EZ2],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"real",									// m_szName
		STEPS_TYPE_EZ2_REAL,					// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		7,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_EZ2_SINGLE_VERSUS
		&g_Games[GAME_EZ2],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versus",								// m_szName
		STEPS_TYPE_EZ2_SINGLE,					// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	+EZ2_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_COL_SPACING*2 },
				{ TRACK_2,	-EZ2_COL_SPACING*1 },
				{ TRACK_3,	+EZ2_COL_SPACING*0 },
				{ TRACK_4,	+EZ2_COL_SPACING*1 },
				{ TRACK_5,	+EZ2_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_EZ2_REAL_VERSUS
		&g_Games[GAME_EZ2],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versusReal",							// m_szName
		STEPS_TYPE_EZ2_REAL,					// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		7,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3 },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2 },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0 },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1 }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2 },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_EZ2_DOUBLE
		&g_Games[GAME_EZ2],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double",								// m_szName
		STEPS_TYPE_EZ2_DOUBLE,					// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_COL_SPACING*4.5f },
				{ TRACK_2,	-EZ2_COL_SPACING*3.5f },
				{ TRACK_3,	-EZ2_COL_SPACING*2.5f },
				{ TRACK_4,	-EZ2_COL_SPACING*1.5f },
				{ TRACK_5,	-EZ2_COL_SPACING*0.5f },
				{ TRACK_6,	+EZ2_COL_SPACING*0.5f }, 
				{ TRACK_7,	+EZ2_COL_SPACING*1.5f },  
				{ TRACK_8,	+EZ2_COL_SPACING*2.5f },
				{ TRACK_9,	+EZ2_COL_SPACING*3.5f },
				{ TRACK_10,	+EZ2_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_COL_SPACING*4.5f },
				{ TRACK_2,	-EZ2_COL_SPACING*3.5f },
				{ TRACK_3,	-EZ2_COL_SPACING*2.5f },
				{ TRACK_4,	-EZ2_COL_SPACING*1.5f },
				{ TRACK_5,	-EZ2_COL_SPACING*0.5f },
				{ TRACK_6,	+EZ2_COL_SPACING*0.5f }, 
				{ TRACK_7,	+EZ2_COL_SPACING*1.5f },  
				{ TRACK_8,	+EZ2_COL_SPACING*2.5f },
				{ TRACK_9,	+EZ2_COL_SPACING*3.5f },
				{ TRACK_10,	+EZ2_COL_SPACING*4.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
			{ 5, 9, 7, 6, 8, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3,7,5,9,6,8 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PARA_SINGLE
		&g_Games[GAME_PARA],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single",								// m_szName
		STEPS_TYPE_PARA_SINGLE,						// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PARA_COL_SPACING*2 },
				{ TRACK_2,	-PARA_COL_SPACING*1 },
				{ TRACK_3,	+PARA_COL_SPACING*0 },
				{ TRACK_4,	+PARA_COL_SPACING*1 },
				{ TRACK_5,	+PARA_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PARA_COL_SPACING*2 },
				{ TRACK_2,	-PARA_COL_SPACING*1 },
				{ TRACK_3,	+PARA_COL_SPACING*0 },
				{ TRACK_4,	+PARA_COL_SPACING*1 },
				{ TRACK_5,	+PARA_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PARA_VERSUS
		&g_Games[GAME_PARA],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"versus",								// m_szName
		STEPS_TYPE_PARA_VERSUS,						// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PARA_COL_SPACING*2 },
				{ TRACK_2,	-PARA_COL_SPACING*1 },
				{ TRACK_3,	+PARA_COL_SPACING*0 },
				{ TRACK_4,	+PARA_COL_SPACING*1 },
				{ TRACK_5,	+PARA_COL_SPACING*2 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PARA_COL_SPACING*2 },
				{ TRACK_2,	-PARA_COL_SPACING*1 },
				{ TRACK_3,	+PARA_COL_SPACING*0 },
				{ TRACK_4,	+PARA_COL_SPACING*1 },
				{ TRACK_5,	+PARA_COL_SPACING*2 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_DS3DDX_SINGLE
		&g_Games[GAME_DS3DDX],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single",								// m_szName
		STEPS_TYPE_DS3DDX_SINGLE,						// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DS3DDX_COL_SPACING*3 },
				{ TRACK_2,	-DS3DDX_COL_SPACING*2 },
				{ TRACK_3,	-DS3DDX_COL_SPACING*1 },
				{ TRACK_4,	-DS3DDX_COL_SPACING*0 },
				{ TRACK_5,	+DS3DDX_COL_SPACING*0 },
				{ TRACK_6,	+DS3DDX_COL_SPACING*1 },
				{ TRACK_7,	+DS3DDX_COL_SPACING*2 },
				{ TRACK_8,	+DS3DDX_COL_SPACING*3 },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DS3DDX_COL_SPACING*3 },
				{ TRACK_2,	-DS3DDX_COL_SPACING*2 },
				{ TRACK_3,	-DS3DDX_COL_SPACING*1 },
				{ TRACK_4,	-DS3DDX_COL_SPACING*0 },
				{ TRACK_5,	+DS3DDX_COL_SPACING*0 },
				{ TRACK_6,	+DS3DDX_COL_SPACING*1 },
				{ TRACK_7,	+DS3DDX_COL_SPACING*2 },
				{ TRACK_8,	+DS3DDX_COL_SPACING*3 },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_BM_SINGLE5
		&g_Games[GAME_BM],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single5",								// m_szName
		STEPS_TYPE_BM_SINGLE5,					// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		6,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-BM_COL_SPACING*2.5f },
				{ TRACK_2,	-BM_COL_SPACING*1.5f },
				{ TRACK_3,	-BM_COL_SPACING*0.5f },
				{ TRACK_4,	+BM_COL_SPACING*0.5f },
				{ TRACK_5,	+BM_COL_SPACING*1.5f },
				{ TRACK_6,	+BM_COL_SPACING*3.0f, "scratch" },
			},
			{	// PLAYER_2
				{ TRACK_1,	-BM_COL_SPACING*2.5f },
				{ TRACK_2,	-BM_COL_SPACING*1.5f },
				{ TRACK_3,	-BM_COL_SPACING*0.5f },
				{ TRACK_4,	+BM_COL_SPACING*0.5f },
				{ TRACK_5,	+BM_COL_SPACING*1.5f },
				{ TRACK_6,	+BM_COL_SPACING*3.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_BM_DOUBLE
		&g_Games[GAME_BM],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double5",								// m_szName
		STEPS_TYPE_BM_DOUBLE5,					// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		12,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-BM_COL_SPACING*6.0f },
				{ TRACK_2,	-BM_COL_SPACING*5.0f },
				{ TRACK_3,	-BM_COL_SPACING*4.0f },
				{ TRACK_4,	-BM_COL_SPACING*3.0f },
				{ TRACK_5,	-BM_COL_SPACING*2.0f },
				{ TRACK_6,	-BM_COL_SPACING*1.5f, "scratch" },
				{ TRACK_7,	+BM_COL_SPACING*0.5f },
				{ TRACK_8,	+BM_COL_SPACING*1.5f },
				{ TRACK_9,	+BM_COL_SPACING*2.5f },
				{ TRACK_10,	+BM_COL_SPACING*3.5f },
				{ TRACK_11,	+BM_COL_SPACING*4.5f },
				{ TRACK_12,	+BM_COL_SPACING*6.0f, "scratch" },
			},
			{	// PLAYER_2
				{ TRACK_1,	-BM_COL_SPACING*6.0f },
				{ TRACK_2,	-BM_COL_SPACING*5.0f },
				{ TRACK_3,	-BM_COL_SPACING*4.0f },
				{ TRACK_4,	-BM_COL_SPACING*3.0f },
				{ TRACK_5,	-BM_COL_SPACING*2.0f },
				{ TRACK_6,	-BM_COL_SPACING*1.5f, "scratch" },
				{ TRACK_7,	+BM_COL_SPACING*0.5f },
				{ TRACK_8,	+BM_COL_SPACING*1.5f },
				{ TRACK_9,	+BM_COL_SPACING*2.5f },
				{ TRACK_10,	+BM_COL_SPACING*3.5f },
				{ TRACK_11,	+BM_COL_SPACING*4.5f },
				{ TRACK_12,	+BM_COL_SPACING*6.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING },
			{ 5, 6, 7, 8, 9, Style::NO_MAPPING, Style::NO_MAPPING, 10, 10, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8,9,10,11
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_BM_SINGLE7
		&g_Games[GAME_BM],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"single7",								// m_szName
		STEPS_TYPE_BM_SINGLE7,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_8,	-BM_COL_SPACING*3.5f, "scratch" },
				{ TRACK_1,	-BM_COL_SPACING*2.0f },
				{ TRACK_2,	-BM_COL_SPACING*1.0f },
				{ TRACK_3,	-BM_COL_SPACING*0.0f },
				{ TRACK_4,	+BM_COL_SPACING*1.0f },
				{ TRACK_5,	+BM_COL_SPACING*2.0f },
				{ TRACK_6,	+BM_COL_SPACING*3.0f },
				{ TRACK_7,	+BM_COL_SPACING*4.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-BM_COL_SPACING*3.5f },
				{ TRACK_2,	-BM_COL_SPACING*2.5f },
				{ TRACK_3,	-BM_COL_SPACING*1.5f },
				{ TRACK_4,	-BM_COL_SPACING*0.5f },
				{ TRACK_5,	+BM_COL_SPACING*0.5f },
				{ TRACK_6,	+BM_COL_SPACING*1.5f },
				{ TRACK_7,	+BM_COL_SPACING*2.5f },
				{ TRACK_8,	+BM_COL_SPACING*4.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_BM_DOUBLE7
		&g_Games[GAME_BM],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double7",								// m_szName
		STEPS_TYPE_BM_DOUBLE7,				// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		16,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_8,	-BM_COL_SPACING*8.0f, "scratch" },
				{ TRACK_1,	-BM_COL_SPACING*6.5f },
				{ TRACK_2,	-BM_COL_SPACING*5.5f },
				{ TRACK_3,	-BM_COL_SPACING*4.5f },
				{ TRACK_4,	-BM_COL_SPACING*3.5f },
				{ TRACK_5,	-BM_COL_SPACING*2.5f },
				{ TRACK_6,	-BM_COL_SPACING*1.5f },
				{ TRACK_7,	-BM_COL_SPACING*0.5f },
				{ TRACK_9,	+BM_COL_SPACING*0.5f },
				{ TRACK_10,	+BM_COL_SPACING*1.5f },
				{ TRACK_11,	+BM_COL_SPACING*2.5f },
				{ TRACK_12,	+BM_COL_SPACING*3.5f },
				{ TRACK_13,	+BM_COL_SPACING*4.5f },
				{ TRACK_14,	+BM_COL_SPACING*5.5f },
				{ TRACK_15,	+BM_COL_SPACING*6.5f },
				{ TRACK_16,	+BM_COL_SPACING*8.0f, "scratch" },
			},
			{	// PLAYER_2
				{ TRACK_8,	-BM_COL_SPACING*8.0f, "scratch" },
				{ TRACK_1,	-BM_COL_SPACING*6.5f },
				{ TRACK_2,	-BM_COL_SPACING*5.5f },
				{ TRACK_3,	-BM_COL_SPACING*4.5f },
				{ TRACK_4,	-BM_COL_SPACING*3.5f },
				{ TRACK_5,	-BM_COL_SPACING*2.5f },
				{ TRACK_6,	-BM_COL_SPACING*1.5f },
				{ TRACK_7,	-BM_COL_SPACING*0.5f },
				{ TRACK_9,	+BM_COL_SPACING*0.5f },
				{ TRACK_10,	+BM_COL_SPACING*1.5f },
				{ TRACK_11,	+BM_COL_SPACING*2.5f },
				{ TRACK_12,	+BM_COL_SPACING*3.5f },
				{ TRACK_13,	+BM_COL_SPACING*4.5f },
				{ TRACK_14,	+BM_COL_SPACING*5.5f },
				{ TRACK_15,	+BM_COL_SPACING*6.5f },
				{ TRACK_16,	+BM_COL_SPACING*8.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
			{ 8, 9, 10, 11, 12, 13, 14, 15, 15, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_MANIAX_SINGLE
		&g_Games[GAME_MANIAX],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single",								// m_szName
		STEPS_TYPE_MANIAX_SINGLE,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 2, 0, 3, Style::END_MAPPING },
			{ 1, 2, 0, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_MANIAX_VERSUS
		&g_Games[GAME_MANIAX],				// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versus",								// m_szName
		STEPS_TYPE_MANIAX_SINGLE,				// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 2, 0, 3, Style::END_MAPPING },
			{ 1, 2, 0, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_MANIAX_DOUBLE
		&g_Games[GAME_MANIAX],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double",								// m_szName
		STEPS_TYPE_MANIAX_DOUBLE,				// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-MANIAX_COL_SPACING*3.5f },
				{ TRACK_2,	-MANIAX_COL_SPACING*2.5f },
				{ TRACK_3,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_4,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_5,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_6,	+MANIAX_COL_SPACING*1.5f },
				{ TRACK_7,	+MANIAX_COL_SPACING*2.5f },
				{ TRACK_8,	+MANIAX_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-MANIAX_COL_SPACING*3.5f },
				{ TRACK_2,	-MANIAX_COL_SPACING*2.5f },
				{ TRACK_3,	-MANIAX_COL_SPACING*1.5f },
				{ TRACK_4,	-MANIAX_COL_SPACING*0.5f },
				{ TRACK_5,	+MANIAX_COL_SPACING*0.5f },
				{ TRACK_6,	+MANIAX_COL_SPACING*1.5f },
				{ TRACK_7,	+MANIAX_COL_SPACING*2.5f },
				{ TRACK_8,	+MANIAX_COL_SPACING*3.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 2, 0, 3, Style::END_MAPPING },
			{ 5, 6, 4, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_SINGLE4
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"single4",								// m_szName
		STEPS_TYPE_TECHNO_SINGLE4,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_SINGLE5
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"single5",								// m_szName
		STEPS_TYPE_TECHNO_SINGLE5,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
				1, 3, 2, 0, 4, Style::END_MAPPING },
			{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
				1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_SINGLE8
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"single8",								// m_szName
		STEPS_TYPE_TECHNO_SINGLE8,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*2.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_3,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_4,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_5,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_6,	+TECHNO_COL_SPACING*2.5f },
				{ TRACK_7,	+TECHNO_COL_SPACING*3.5f },
				{ TRACK_8,	+TECHNO_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*4.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*3.5f },
				{ TRACK_3,	-TECHNO_COL_SPACING*2.5f },
				{ TRACK_4,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_5,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_6,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_7,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_8,	+TECHNO_COL_SPACING*2.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
			{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		true, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_VERSUS4
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versus4",								// m_szName
		STEPS_TYPE_TECHNO_SINGLE4,				// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		4,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_4,	TECHNO_COL_SPACING*1.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_VERSUS5
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versus5",								// m_szName
		STEPS_TYPE_TECHNO_SINGLE5,				// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
				1, 3, 2, 0, 4, Style::END_MAPPING },
			{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
				1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_VERSUS8
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		false,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"versus8",								// m_szName
		STEPS_TYPE_TECHNO_SINGLE8,				// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_VERSUS_COL_SPACING*3.5f },
				{ TRACK_2,	-TECHNO_VERSUS_COL_SPACING*2.5f },
				{ TRACK_3,	-TECHNO_VERSUS_COL_SPACING*1.5f },
				{ TRACK_4,	-TECHNO_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	+TECHNO_VERSUS_COL_SPACING*0.5f },
				{ TRACK_6,	+TECHNO_VERSUS_COL_SPACING*1.5f },
				{ TRACK_7,	+TECHNO_VERSUS_COL_SPACING*2.5f },
				{ TRACK_8,	+TECHNO_VERSUS_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_VERSUS_COL_SPACING*3.5f },
				{ TRACK_2,	-TECHNO_VERSUS_COL_SPACING*2.5f },
				{ TRACK_3,	-TECHNO_VERSUS_COL_SPACING*1.5f },
				{ TRACK_4,	-TECHNO_VERSUS_COL_SPACING*0.5f },
				{ TRACK_5,	+TECHNO_VERSUS_COL_SPACING*0.5f },
				{ TRACK_6,	+TECHNO_VERSUS_COL_SPACING*1.5f },
				{ TRACK_7,	+TECHNO_VERSUS_COL_SPACING*2.5f },
				{ TRACK_8,	+TECHNO_VERSUS_COL_SPACING*3.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
			{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
//			{ 1, 6, 4, Style::NO_MAPPING, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
//			{ 1, 6, 4, Style::NO_MAPPING, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		true, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_DOUBLE4
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double4",								// m_szName
		STEPS_TYPE_TECHNO_DOUBLE4,				// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		8,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*3.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*2.5f },
				{ TRACK_3,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_4,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_5,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_6,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_7,	+TECHNO_COL_SPACING*2.5f },
				{ TRACK_8,	+TECHNO_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*3.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*2.5f },
				{ TRACK_3,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_4,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_5,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_6,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_7,	+TECHNO_COL_SPACING*2.5f },
				{ TRACK_8,	+TECHNO_COL_SPACING*3.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 4, 7, 6, 5, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_TECHNO_DOUBLE5
		&g_Games[GAME_TECHNO],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"double5",								// m_szName
		STEPS_TYPE_TECHNO_DOUBLE5,				// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		10,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*4.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*3.5f },
				{ TRACK_3,	-TECHNO_COL_SPACING*2.5f },
				{ TRACK_4,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_5,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_6,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_7,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_8,	+TECHNO_COL_SPACING*2.5f },
				{ TRACK_9,	+TECHNO_COL_SPACING*3.5f },
				{ TRACK_10,	+TECHNO_COL_SPACING*4.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*4.5f },
				{ TRACK_2,	-TECHNO_COL_SPACING*3.5f },
				{ TRACK_3,	-TECHNO_COL_SPACING*2.5f },
				{ TRACK_4,	-TECHNO_COL_SPACING*1.5f },
				{ TRACK_5,	-TECHNO_COL_SPACING*0.5f },
				{ TRACK_6,	+TECHNO_COL_SPACING*0.5f },
				{ TRACK_7,	+TECHNO_COL_SPACING*1.5f },
				{ TRACK_8,	+TECHNO_COL_SPACING*2.5f },
				{ TRACK_9,	+TECHNO_COL_SPACING*3.5f },
				{ TRACK_10,	+TECHNO_COL_SPACING*4.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
				1, 3, 2, 0, 4, Style::END_MAPPING },
			{ Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING, Style::NO_MAPPING,
				6, 8, 7, 5, 9, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8,9
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PNM_FIVE
		&g_Games[GAME_PNM],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"pnm-five",								// m_szName
		STEPS_TYPE_PNM_FIVE,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PNM5_COL_SPACING*2.0f },
				{ TRACK_2,	-PNM5_COL_SPACING*1.0f },
				{ TRACK_3,	+PNM5_COL_SPACING*0.0f },
				{ TRACK_4,	+PNM5_COL_SPACING*1.0f },
				{ TRACK_5,	+PNM5_COL_SPACING*2.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PNM5_COL_SPACING*2.0f },
				{ TRACK_2,	-PNM5_COL_SPACING*1.0f },
				{ TRACK_3,	+PNM5_COL_SPACING*0.0f },
				{ TRACK_4,	+PNM5_COL_SPACING*1.0f },
				{ TRACK_5,	+PNM5_COL_SPACING*2.0f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, 4, Style::END_MAPPING },
			{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_PNM_NINE
		&g_Games[GAME_PNM],					// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		true,									// m_bUsedForDemonstration
		true,									// m_bUsedForHowToPlay
		"pnm-nine",								// m_szName
		STEPS_TYPE_PNM_NINE,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		9,										// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PNM9_COL_SPACING*4.0f },
				{ TRACK_2,	-PNM9_COL_SPACING*3.0f },
				{ TRACK_3,	-PNM9_COL_SPACING*2.0f },
				{ TRACK_4,	-PNM9_COL_SPACING*1.0f },
				{ TRACK_5,	+PNM9_COL_SPACING*0.0f },
				{ TRACK_6,	+PNM9_COL_SPACING*1.0f },
				{ TRACK_7,	+PNM9_COL_SPACING*2.0f },
				{ TRACK_8,	+PNM9_COL_SPACING*3.0f },
				{ TRACK_9,	+PNM9_COL_SPACING*4.0f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PNM9_COL_SPACING*4.0f },
				{ TRACK_2,	-PNM9_COL_SPACING*3.0f },
				{ TRACK_3,	-PNM9_COL_SPACING*2.0f },
				{ TRACK_4,	-PNM9_COL_SPACING*1.0f },
				{ TRACK_5,	+PNM9_COL_SPACING*0.0f },
				{ TRACK_6,	+PNM9_COL_SPACING*1.0f },
				{ TRACK_7,	+PNM9_COL_SPACING*2.0f },
				{ TRACK_8,	+PNM9_COL_SPACING*3.0f },
				{ TRACK_9,	+PNM9_COL_SPACING*4.0f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
	{	// STYLE_LIGHTS_CABINET
		&g_Games[GAME_LIGHTS],				// m_Game
		true,									// m_bUsedForGameplay
		true,									// m_bUsedForEdit
		false,									// m_bUsedForDemonstration
		false,									// m_bUsedForHowToPlay
		"cabinet",								// m_szName
		STEPS_TYPE_LIGHTS_CABINET,				// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		NUM_CABINET_LIGHTS,						// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f },
			},
		},
		{	// m_iInputColumn[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS]
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
	},
};

#define NUM_STYLES ARRAYSIZE(g_Styles)


GameManager::GameManager()
{
}

GameManager::~GameManager()
{
}

void GameManager::GetStylesForGame( const Game *pGame, vector<const Style*>& aStylesAddTo, bool editor ) const
{
	for( unsigned s=0; s<NUM_STYLES; s++ ) 
	{
		const Style* style = &g_Styles[s];
		if( style->m_pGame != pGame)
			continue;
		if( !editor && !style->m_bUsedForGameplay )	
			continue;
		if( editor && !style->m_bUsedForEdit )	
			continue;

		aStylesAddTo.push_back( style );
	}
}

const Style* GameManager::GetEditorStyleForStepsType( StepsType st ) const
{
	for( unsigned s=0; s<NUM_STYLES; s++ ) 
	{
		const Style* style = &g_Styles[s];
		if( style->m_StepsType == st && style->m_bUsedForEdit )
			return style;
	}

	ASSERT(0);	// this Game is missing a Style that can be used with the editor
	return NULL;
}


void GameManager::GetStepsTypesForGame( const Game *pGame, vector<StepsType>& aStepsTypeAddTo ) const
{
	FOREACH_StepsType( st )
	{
		bool found = false;
		for( unsigned s=0; !found && s<NUM_STYLES; s++ )
		{
			const Style* style = &g_Styles[s];
			if( style->m_pGame != pGame || style->m_StepsType != st )
				continue;

			found = true;
		}
		if( found )
			aStepsTypeAddTo.push_back( st );
	}
}

const Style* GameManager::GetDemonstrationStyleForGame( const Game *pGame ) const
{
	for( unsigned s=0; s<NUM_STYLES; s++ ) 
	{
		const Style* style = &g_Styles[s];
		if( style->m_pGame == pGame && style->m_bUsedForDemonstration )
			return style;
	}

	ASSERT(0);	// this Game is missing a Style that can be used with the demonstration
	return NULL;
}

const Style* GameManager::GetHowToPlayStyleForGame( const Game *pGame ) const
{
	for( unsigned s=0; s<NUM_STYLES; s++ ) 
	{
		const Style* style = &g_Styles[s];
		if( style->m_pGame == pGame && style->m_bUsedForHowToPlay )
			return style;
	}

	ASSERT(0);	// this Game is missing a Style that can be used with HowToPlay
	return NULL;
}

void GameManager::GetEnabledGames( vector<const Game*>& aGamesOut ) const
{
	for( int g=0; g<NUM_GAMES; g++ )
	{
		const Game *pGame = &g_Games[g];
		if( IsGameEnabled( pGame ) )
			aGamesOut.push_back( pGame );
	}
}

const Game* GameManager::GetDefaultGame() const
{
	return &g_Games[0];
}

int GameManager::GetIndexFromGame( const Game* pGame ) const
{
	for( int g=0; g<NUM_GAMES; g++ )
	{
		if( &g_Games[g] == pGame )
			return g;
	}
	ASSERT(0);
	return 0;
}

const Game* GameManager::GetGameFromIndex( int index ) const
{
	ASSERT( index >= 0 );
	ASSERT( index < NUM_GAMES );
	return &g_Games[index];
}

bool GameManager::IsGameEnabled( const Game *pGame ) const
{
	CStringArray asNoteSkins;
	NOTESKIN->GetNoteSkinNames( pGame, asNoteSkins, false ); /* don't omit default */
	return asNoteSkins.size() > 0;
}

int GameManager::StepsTypeToNumTracks( StepsType st )
{
	ASSERT_M( st < NUM_STEPS_TYPES, ssprintf("%i", st) );
	return StepsTypes[st].NumTracks;
}

StepsType GameManager::StringToStepsType( CString sStepsType )
{
	sStepsType.MakeLower();

	// HACK!  We elminitated "ez2-single-hard", but we should still handle it.
	if( sStepsType == "ez2-single-hard" )
		sStepsType = "ez2-single";

	// HACK!  "para-single" used to be called just "para"
	if( sStepsType == "para" )
		sStepsType = "para-single";

	for( int i=0; i<NUM_STEPS_TYPES; i++ )
		if( StepsTypes[i].name == sStepsType )
			return StepsType(i);
	
	// invalid StepsType
	LOG->Warn( "Invalid StepsType string '%s' encountered.  Assuming this is 'dance-single'.", sStepsType.c_str() );
	return STEPS_TYPE_DANCE_SINGLE;
}

CString GameManager::StepsTypeToString( StepsType st )
{
	ASSERT_M( st < NUM_STEPS_TYPES, ssprintf("%i", st) );
	return StepsTypes[st].name;
}

CString GameManager::StepsTypeToThemedString( StepsType st )
{
	CString s = StepsTypeToString( st );
	if( THEME->HasMetric( "StepsType", s ) )
		return THEME->GetMetric( "StepsType", s );
	else
		return s;
}

CString GameManager::StyleToThemedString( const Style* style )
{
	CString s = style->m_szName;
	s = Capitalize( s );
	if( THEME->HasMetric( "Style", s ) )
		return THEME->GetMetric( "Style", s );
	else
		return s;
}

const Game* GameManager::StringToGameType( CString sGameType )
{
	for( int i=0; i<NUM_GAMES; i++ )
		if( !sGameType.CompareNoCase(g_Games[i].m_szName) )
			return &g_Games[i];

	return NULL;
}


const Style* GameManager::GameAndStringToStyle( const Game *game, CString sStyle )
{
	for( unsigned s=0; s<NUM_STYLES; s++ ) 
	{
		const Style* style = &g_Styles[s];
		if( style->m_pGame != game )
			continue;
		if( sStyle.CompareNoCase(style->m_szName) == 0 )
			return style;
	}

	return NULL;
}


CString GameManager::GetMenuButtonSecondaryFunction( const Game *pGame, GameButton gb ) const
{
	/*
	 * Each GameButton can be used in gameplay (if any gameplay style maps to
	 * it) and/or map to a menu button (if m_DedicatedMenuButton or m_SecondaryMenuButton
	 * map to it).
	 *
	 * If a button is only used in gameplay or is only used in menus, return ""; the 
	 * primary description is sufficient.
	 *
	 * If a button is used in both gameplay and menus, return szSecondaryNames[] for
	 * the MenuButton.
	 */
	vector<const Style*> aStyles;
	this->GetStylesForGame( pGame, aStyles );
	bool bUsedInGameplay = false;
	for( unsigned i = 0; i < aStyles.size(); ++i )
	{
		const Style *pStyle = aStyles[i];
		FOREACH_GameController(gc)
		{
			const StyleInput si = pStyle->GameInputToStyleInput( GameInput(gc,gb) );
			if( si.IsValid() )
				bUsedInGameplay = true;
		}
	}

	static const char *szSecondaryNames[NUM_MENU_BUTTONS] =
	{
		"MenuLeft",
		"MenuRight",
		"MenuUp",
		"MenuDown",
		"MenuStart",
		"MenuBack",
		"Coin",
		"Operator"
	};

	FOREACH_MenuButton(m)
	{
		if( !bUsedInGameplay && pGame->m_DedicatedMenuButton[m] == gb )
			return "";
		else if( bUsedInGameplay && pGame->m_SecondaryMenuButton[m] == gb )
			return szSecondaryNames[m];
	}

	return ""; // only used in gameplay
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
