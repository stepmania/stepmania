#include "global.h"
#include "GameManager.h"
#include "GameConstantsAndTypes.h"
#include "GameInput.h"	// for GameButton constants
#include "RageLog.h"
#include "RageUtil.h"
#include "NoteSkinManager.h"
#include "RageInputDevice.h"
#include "ThemeManager.h"
#include "LightsManager.h"	// for NUM_CabinetLight
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
	// BMS reader needs 16 tracks for beat-double7
};

enum
{
	GAME_DANCE,
	GAME_PUMP,
	GAME_EZ2,
	GAME_PARA,
	GAME_DS3DDX,
	GAME_BEAT,
	GAME_MANIAX,
	GAME_TECHNO,
	GAME_POPN,
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
const int BEAT_COL_SPACING = 34;
const int MANIAX_COL_SPACING = 36;
const int TECHNO_COL_SPACING = 56;
const int TECHNO_VERSUS_COL_SPACING = 33;
const int POPN5_COL_SPACING = 32; 
const int POPN9_COL_SPACING = 32; 

enum
{
	ST_FLAGS_NONE = 0,
	ST_FLAGS_DONT_AUTOGEN = 1 << 0
};

static struct
{
	char *name;
	int NumTracks;
	uint32_t flags;
} const StepsTypes[NUM_StepsType] = {
	{ "dance-single",	4,			ST_FLAGS_NONE },
	{ "dance-double",	8,			ST_FLAGS_NONE },
	{ "dance-couple",	8,			ST_FLAGS_NONE },
	{ "dance-solo",		6,			ST_FLAGS_NONE },
	{ "dance-routine",	8,			ST_FLAGS_DONT_AUTOGEN },
	{ "pump-single",	5,			ST_FLAGS_NONE },
	{ "pump-halfdouble",	6,			ST_FLAGS_NONE },
	{ "pump-double",	10,			ST_FLAGS_NONE },
	{ "pump-couple",	10,			ST_FLAGS_NONE },
	{ "ez2-single",		5,			ST_FLAGS_NONE },	// Single: TL,LHH,D,RHH,TR
	{ "ez2-double",		10,			ST_FLAGS_NONE },	// Double: Single x2
	{ "ez2-real",		7,			ST_FLAGS_NONE },	// Real: TL,LHH,LHL,D,RHL,RHH,TR
	{ "para-single",	5,			ST_FLAGS_NONE },
	{ "para-versus",	10,			ST_FLAGS_NONE },
	{ "ds3ddx-single",	8,			ST_FLAGS_NONE },
	{ "bm-single5",		6,			ST_FLAGS_NONE },	// called "bm" for backward compat
	{ "bm-double5",		12,			ST_FLAGS_NONE },	// called "bm" for backward compat
	{ "bm-single7",		8,			ST_FLAGS_NONE },	// called "bm" for backward compat
	{ "bm-double7",		16,			ST_FLAGS_NONE },	// called "bm" for backward compat
	{ "maniax-single",	4,			ST_FLAGS_NONE },
	{ "maniax-double",	8,			ST_FLAGS_NONE },
	{ "techno-single4",	4,			ST_FLAGS_NONE },
	{ "techno-single5",	5,			ST_FLAGS_NONE },
	{ "techno-single8",	8,			ST_FLAGS_NONE },
	{ "techno-double4",	8,			ST_FLAGS_NONE },
	{ "techno-double5",	10,			ST_FLAGS_NONE },
	{ "pnm-five",		5,			ST_FLAGS_NONE },	// called "pnm" for backward compat
	{ "pnm-nine",		9,			ST_FLAGS_NONE },	// called "pnm" for backward compat
	{ "guitar-five",	5,			ST_FLAGS_NONE },
	{ "lights-cabinet",	NUM_CabinetLight,	ST_FLAGS_DONT_AUTOGEN }, // XXX disable lights autogen for now
};

//
// Important:  Every game must define the buttons: "Start", "Back", "MenuLeft", "Operator" and "MenuRight"
//
static Game g_Games[NUM_GAMES] = 
{
	{	// GAME_DANCE
		"dance",				// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"dance",				// m_szName
			NUM_DANCE_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "Left",	GameButtonType_Step,	KEY_LEFT,	KEY_KP_C4 },
				{ "Right",	GameButtonType_Step,	KEY_RIGHT,	KEY_KP_C6 },
				{ "Up",		GameButtonType_Step,	KEY_UP,		KEY_KP_C8 },
				{ "Down",	GameButtonType_Step,	KEY_DOWN,	KEY_KP_C2 },
				{ "UpLeft",	GameButtonType_Step,	NO_DEFAULT_KEY,	KEY_KP_C7 },
				{ "UpRight",	GameButtonType_Step,	NO_DEFAULT_KEY,	KEY_KP_C9 },
				{ "Start",	GameButtonType_INVALID,	KEY_ENTER,	KEY_KP_ENTER },
				{ "Select",	GameButtonType_INVALID,	KEY_BACKSLASH,	KEY_KP_C0 },
				{ "Back",	GameButtonType_INVALID,	KEY_ESC,	KEY_NUMLOCK },
				{ "MenuLeft",	GameButtonType_INVALID,	KEY_DEL,	KEY_KP_SLASH },
				{ "MenuRight",	GameButtonType_INVALID,	KEY_PGDN,	KEY_KP_ASTERISK },
				{ "MenuUp",	GameButtonType_INVALID,	KEY_HOME,	KEY_KP_HYPHEN },
				{ "MenuDown",	GameButtonType_INVALID,	KEY_END,	KEY_KP_PLUS },
				{ "Coin",	GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",	GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				DANCE_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				DANCE_BUTTON_MENURIGHT,		// MENU_BUTTON_RIGHT
				DANCE_BUTTON_MENUUP,		// MENU_BUTTON_UP
				DANCE_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				DANCE_BUTTON_START,		// MENU_BUTTON_START
				DANCE_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				DANCE_BUTTON_BACK,		// MENU_BUTTON_BACK
				DANCE_BUTTON_COIN,		// MENU_BUTTON_COIN
				DANCE_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				DANCE_BUTTON_LEFT,		// MENU_BUTTON_LEFT
				DANCE_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
				DANCE_BUTTON_UP,		// MENU_BUTTON_UP
				DANCE_BUTTON_DOWN,		// MENU_BUTTON_DOWN
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
	{	// GAME_PUMP
		"pump",					// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"pump",					// m_szName
			NUM_PUMP_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "UpLeft",	GameButtonType_Step,	KEY_Cq,		KEY_KP_C7 },
				{ "UpRight",	GameButtonType_Step,	KEY_Ce,		KEY_KP_C9 },
				{ "Center",	GameButtonType_Step,	KEY_Cs,		KEY_KP_C5 },
				{ "DownLeft",	GameButtonType_Step,	KEY_Cz,		KEY_KP_C1 },
				{ "DownRight",	GameButtonType_Step,	KEY_Cc,		KEY_KP_C3 },
				{ "Start",	GameButtonType_INVALID,	KEY_ENTER,	KEY_KP_ENTER },
				{ "Select",	GameButtonType_INVALID,	KEY_BACKSLASH,	KEY_KP_C0 },
				{ "Back",	GameButtonType_INVALID,	KEY_ESC,	KEY_NUMLOCK },
				{ "MenuLeft",	GameButtonType_INVALID,	KEY_LEFT,	NO_DEFAULT_KEY },
				{ "MenuRight",	GameButtonType_INVALID,	KEY_RIGHT,	NO_DEFAULT_KEY },
				{ "MenuUp",	GameButtonType_INVALID,	KEY_UP,		NO_DEFAULT_KEY },
				{ "MenuDown",	GameButtonType_INVALID,	KEY_DOWN,	NO_DEFAULT_KEY },
				{ "Coin",	GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",	GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				PUMP_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				PUMP_BUTTON_MENURIGHT,		// MENU_BUTTON_RIGHT
				PUMP_BUTTON_MENUUP,		// MENU_BUTTON_UP
				PUMP_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				PUMP_BUTTON_START,		// MENU_BUTTON_START
				PUMP_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				PUMP_BUTTON_BACK,		// MENU_BUTTON_BACK
				PUMP_BUTTON_COIN,		// MENU_BUTTON_COIN
				PUMP_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				PUMP_BUTTON_DOWNLEFT,		// MENU_BUTTON_LEFT
				PUMP_BUTTON_DOWNRIGHT,		// MENU_BUTTON_RIGHT
				PUMP_BUTTON_UPLEFT,		// MENU_BUTTON_UP
				PUMP_BUTTON_UPRIGHT,		// MENU_BUTTON_DOWN
				PUMP_BUTTON_CENTER,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
	{
		"ez2",					// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"ez2",					// m_szName
			NUM_EZ2_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "FootUpLeft",		GameButtonType_Step,	KEY_Cz,		NO_DEFAULT_KEY },
				{ "FootUpRight",	GameButtonType_Step,	KEY_Cb,		NO_DEFAULT_KEY },
				{ "FootDown",		GameButtonType_Step,	KEY_Cc,		NO_DEFAULT_KEY },
				{ "HandUpLeft",		GameButtonType_Step,	KEY_Cx,		NO_DEFAULT_KEY },
				{ "HandUpRight",	GameButtonType_Step,	KEY_Cv,		NO_DEFAULT_KEY },
				{ "HandLrLeft",		GameButtonType_Step,	KEY_Cs,		NO_DEFAULT_KEY },
				{ "HandLrRight",	GameButtonType_Step,	KEY_Cf,		NO_DEFAULT_KEY },
				{ "Start",		GameButtonType_INVALID,	KEY_ENTER,	NO_DEFAULT_KEY },
				{ "Select",		GameButtonType_INVALID,	KEY_BACKSLASH,	NO_DEFAULT_KEY },
				{ "Back",		GameButtonType_INVALID,	KEY_ESC,	NO_DEFAULT_KEY },
				{ "MenuLeft",		GameButtonType_INVALID,	KEY_LEFT,	NO_DEFAULT_KEY },
				{ "MenuRight",		GameButtonType_INVALID,	KEY_RIGHT,	NO_DEFAULT_KEY },
				{ "MenuUp",		GameButtonType_INVALID,	KEY_UP,		NO_DEFAULT_KEY },
				{ "MenuDown",		GameButtonType_INVALID,	KEY_DOWN,	NO_DEFAULT_KEY },
				{ "Coin",		GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",		GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				EZ2_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				EZ2_BUTTON_MENURIGHT,		// MENU_BUTTON_RIGHT
				EZ2_BUTTON_MENUUP,		// MENU_BUTTON_UP
				EZ2_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				EZ2_BUTTON_START,		// MENU_BUTTON_START
				EZ2_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				EZ2_BUTTON_BACK,		// MENU_BUTTON_BACK
				EZ2_BUTTON_COIN,		// MENU_BUTTON_COIN
				EZ2_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				EZ2_BUTTON_HANDUPLEFT,		// MENU_BUTTON_LEFT
				EZ2_BUTTON_HANDUPRIGHT,		// MENU_BUTTON_RIGHT
				EZ2_BUTTON_FOOTUPLEFT,		// MENU_BUTTON_UP
				EZ2_BUTTON_FOOTUPRIGHT,		// MENU_BUTTON_DOWN
				EZ2_BUTTON_FOOTDOWN,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W2,		// m_mapW1To
		TNS_W2,		// m_mapW2To
		TNS_W2,		// m_mapW3To
		TNS_W4,		// m_mapW4To
		TNS_Miss,	// m_mapW5To
	},
	{	// GAME_PARA
		"para",					// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"para",					// m_szName
			NUM_PARA_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "Left",	GameButtonType_Step,	KEY_Cz,		NO_DEFAULT_KEY },
				{ "UpLeft",	GameButtonType_Step,	KEY_Cx,		NO_DEFAULT_KEY },
				{ "Up",		GameButtonType_Step,	KEY_Cc,		NO_DEFAULT_KEY },
				{ "UpRight",	GameButtonType_Step,	KEY_Cv,		NO_DEFAULT_KEY },
				{ "Right",	GameButtonType_Step,	KEY_Cb,		NO_DEFAULT_KEY },
				{ "Start",	GameButtonType_INVALID,	KEY_ENTER,	NO_DEFAULT_KEY },
				{ "Select",	GameButtonType_INVALID,	KEY_BACKSLASH,	NO_DEFAULT_KEY },
				{ "Back",	GameButtonType_INVALID,	KEY_ESC,	NO_DEFAULT_KEY },
				{ "MenuLeft",	GameButtonType_INVALID,	KEY_LEFT,	NO_DEFAULT_KEY },
				{ "MenuRight",	GameButtonType_INVALID,	KEY_RIGHT,	NO_DEFAULT_KEY },
				{ "MenuUp",	GameButtonType_INVALID,	KEY_UP,		NO_DEFAULT_KEY },
				{ "MenuDown",	GameButtonType_INVALID,	KEY_DOWN,	NO_DEFAULT_KEY },
				{ "Coin",	GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",	GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				PARA_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				PARA_BUTTON_MENURIGHT,		// MENU_BUTTON_RIGHT
				PARA_BUTTON_MENUUP,		// MENU_BUTTON_UP
				PARA_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				PARA_BUTTON_START,		// MENU_BUTTON_START
				PARA_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				PARA_BUTTON_BACK,		// MENU_BUTTON_BACK
				PARA_BUTTON_COIN,		// MENU_BUTTON_COIN
				PARA_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				PARA_BUTTON_LEFT,		// MENU_BUTTON_LEFT
				PARA_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
				PARA_BUTTON_UPRIGHT,		// MENU_BUTTON_UP
				PARA_BUTTON_UPLEFT,		// MENU_BUTTON_DOWN
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
	{	// GAME_DS3DDX
		"ds3ddx",				// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"ds3ddx",				// m_szName
			NUM_DS3DDX_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "HandLeft",		GameButtonType_Step,	KEY_Ca,		NO_DEFAULT_KEY },
				{ "FootDownLeft",	GameButtonType_Step,	KEY_Cz,		NO_DEFAULT_KEY },
				{ "FootUpLeft",		GameButtonType_Step,	KEY_Cq,		NO_DEFAULT_KEY },
				{ "HandUp",		GameButtonType_Step,	KEY_Cw,		NO_DEFAULT_KEY },
				{ "HandDown",		GameButtonType_Step,	KEY_Cx,		NO_DEFAULT_KEY },
				{ "FootUpRight",	GameButtonType_Step,	KEY_Ce,		NO_DEFAULT_KEY },
				{ "FootDownRight",	GameButtonType_Step,	KEY_Cc,		NO_DEFAULT_KEY },
				{ "HandRight",		GameButtonType_Step,	KEY_Cd,		NO_DEFAULT_KEY },
				{ "Start",		GameButtonType_INVALID,	KEY_ENTER,	NO_DEFAULT_KEY },
				{ "Select",		GameButtonType_INVALID,	KEY_BACKSLASH,	NO_DEFAULT_KEY },
				{ "Back",		GameButtonType_INVALID,	KEY_ESC,	NO_DEFAULT_KEY },
				{ "MenuLeft",		GameButtonType_INVALID,	KEY_LEFT,	NO_DEFAULT_KEY },
				{ "MenuRight",		GameButtonType_INVALID,	KEY_RIGHT,	NO_DEFAULT_KEY },
				{ "MenuUp",		GameButtonType_INVALID,	KEY_UP,		NO_DEFAULT_KEY },
				{ "MenuDown",		GameButtonType_INVALID,	KEY_DOWN,	NO_DEFAULT_KEY },
				{ "Coin",		GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",		GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				DS3DDX_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				DS3DDX_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
				DS3DDX_BUTTON_MENUUP,		// MENU_BUTTON_UP
				DS3DDX_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				DS3DDX_BUTTON_START,		// MENU_BUTTON_START
				DS3DDX_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				DS3DDX_BUTTON_BACK,		// MENU_BUTTON_BACK
				DS3DDX_BUTTON_COIN,		// MENU_BUTTON_COIN
				DS3DDX_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				DS3DDX_BUTTON_HANDLEFT,		// MENU_BUTTON_LEFT
				DS3DDX_BUTTON_HANDRIGHT,	// MENU_BUTTON_RIGHT
				DS3DDX_BUTTON_HANDUP,		// MENU_BUTTON_UP
				DS3DDX_BUTTON_HANDDOWN,		// MENU_BUTTON_DOWN
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
	{	// GAME_BEAT
		"beat",					// m_szName
		true,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"beat",					// m_szName
			NUM_BEAT_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "Key1",		GameButtonType_Step,	KEY_Cm,		NO_DEFAULT_KEY },
				{ "Key2",		GameButtonType_Step,	KEY_Ck,		NO_DEFAULT_KEY },
				{ "Key3",		GameButtonType_Step,	KEY_COMMA,	NO_DEFAULT_KEY },
				{ "Key4",		GameButtonType_Step,	KEY_Cl,		NO_DEFAULT_KEY },
				{ "Key5",		GameButtonType_Step,	KEY_PERIOD,	NO_DEFAULT_KEY },
				{ "Key6",		GameButtonType_Step,	KEY_SEMICOLON,	NO_DEFAULT_KEY },
				{ "Key7",		GameButtonType_Step,	KEY_SLASH,	NO_DEFAULT_KEY },
				{ "Scratch up",		GameButtonType_Step,	KEY_LSHIFT,	NO_DEFAULT_KEY },
				{ "Scratch down",	GameButtonType_Step,	NO_DEFAULT_KEY,	NO_DEFAULT_KEY },
				{ "Start",		GameButtonType_INVALID,	KEY_ENTER,	NO_DEFAULT_KEY },
				{ "Select",		GameButtonType_INVALID,	KEY_BACKSLASH,	NO_DEFAULT_KEY },
				{ "Back",		GameButtonType_INVALID,	KEY_ESC,	NO_DEFAULT_KEY },
				{ "MenuLeft",		GameButtonType_INVALID,	KEY_LEFT,	NO_DEFAULT_KEY },
				{ "MenuRight",		GameButtonType_INVALID,	KEY_RIGHT,	NO_DEFAULT_KEY },
				{ "MenuUp",		GameButtonType_INVALID,	KEY_UP,		NO_DEFAULT_KEY },
				{ "MenuDown",		GameButtonType_INVALID,	KEY_DOWN,	NO_DEFAULT_KEY },
				{ "Coin",		GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",		GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				BEAT_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				BEAT_BUTTON_MENURIGHT,		// MENU_BUTTON_RIGHT
				BEAT_BUTTON_MENUUP,		// MENU_BUTTON_UP
				BEAT_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				BEAT_BUTTON_START,		// MENU_BUTTON_START
				BEAT_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				BEAT_BUTTON_BACK,		// MENU_BUTTON_BACK
				BEAT_BUTTON_COIN,		// MENU_BUTTON_COIN
				BEAT_BUTTON_OPERATOR,		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				BEAT_BUTTON_KEY1,		// MENU_BUTTON_LEFT
				BEAT_BUTTON_KEY3,		// MENU_BUTTON_RIGHT
				BEAT_BUTTON_SCRATCHUP,		// MENU_BUTTON_UP
				BEAT_BUTTON_SCRATCHDOWN,	// MENU_BUTTON_DOWN
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
	{	// GAME_MANIAX
		"maniax",				// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"maniax",				// m_szName
			NUM_MANIAX_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "HandUpLeft",		GameButtonType_Step,	KEY_Ca,		KEY_KP_C4 },
				{ "HandUpRight",	GameButtonType_Step,	KEY_Cs,		KEY_KP_C5 },
				{ "HandLrLeft",		GameButtonType_Step,	KEY_Cz,		KEY_KP_C1 },
				{ "HandLrRight",	GameButtonType_Step,	KEY_Cx,		KEY_KP_C2 },
				{ "Start",		GameButtonType_INVALID,	KEY_ENTER,	KEY_KP_ENTER },
				{ "Select",		GameButtonType_INVALID,	KEY_BACKSLASH,	KEY_KP_C0 },
				{ "Back",		GameButtonType_INVALID,	KEY_ESC,	KEY_NUMLOCK },
				{ "MenuLeft",		GameButtonType_INVALID,	KEY_LEFT,	KEY_KP_SLASH },
				{ "MenuRight",		GameButtonType_INVALID,	KEY_RIGHT,	KEY_KP_ASTERISK },
				{ "MenuUp",		GameButtonType_INVALID,	KEY_UP,		KEY_KP_HYPHEN },
				{ "MenuDown",		GameButtonType_INVALID,	KEY_DOWN,	KEY_KP_PLUS },
				{ "Coin",		GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",		GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				MANIAX_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				MANIAX_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
				MANIAX_BUTTON_MENUUP,		// MENU_BUTTON_UP
				MANIAX_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				MANIAX_BUTTON_START,		// MENU_BUTTON_START
				MANIAX_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				MANIAX_BUTTON_BACK,		// MENU_BUTTON_BACK
				MANIAX_BUTTON_COIN,		// MENU_BUTTON_COIN
				MANIAX_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				MANIAX_BUTTON_HANDUPLEFT,	// MENU_BUTTON_LEFT
				MANIAX_BUTTON_HANDUPRIGHT,	// MENU_BUTTON_RIGHT
				MANIAX_BUTTON_HANDLRRIGHT,	// MENU_BUTTON_UP
				MANIAX_BUTTON_HANDLRLEFT,	// MENU_BUTTON_DOWN
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
	{	// GAME_TECHNO
		"techno",				// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"techno",				// m_szName
			NUM_TECHNO_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "Left",	GameButtonType_Step,	KEY_Ca,		KEY_KP_C4 },
				{ "Right",	GameButtonType_Step,	KEY_Cd,		KEY_KP_C6 },
				{ "Up",		GameButtonType_Step,	KEY_Cw,		KEY_KP_C8 },
				{ "Down",	GameButtonType_Step,	KEY_Cx,		KEY_KP_C2 },
				{ "UpLeft",	GameButtonType_Step,	KEY_Cq,		KEY_KP_C7 },
				{ "UpRight",	GameButtonType_Step,	KEY_Ce,		KEY_KP_C9 },
				{ "Center",	GameButtonType_Step,	KEY_Cs,		KEY_KP_C5 },
				{ "DownLeft",	GameButtonType_Step,	KEY_Cz,		KEY_KP_C1 },
				{ "DownRight",	GameButtonType_Step,	KEY_Cc,		KEY_KP_C3 },
				{ "Start",	GameButtonType_INVALID,	KEY_ENTER,	KEY_KP_ENTER },
				{ "Select",	GameButtonType_INVALID,	KEY_BACKSLASH,	KEY_KP_C0 },
				{ "Back",	GameButtonType_INVALID,	KEY_ESC,	KEY_NUMLOCK },
				{ "MenuLeft",	GameButtonType_INVALID,	KEY_LEFT,	KEY_KP_SLASH },
				{ "MenuRight",	GameButtonType_INVALID,	KEY_RIGHT,	KEY_KP_ASTERISK },
				{ "MenuUp",	GameButtonType_INVALID,	KEY_UP,		KEY_KP_HYPHEN },
				{ "MenuDown",	GameButtonType_INVALID,	KEY_DOWN,	KEY_KP_PLUS },
				{ "Coin",	GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",	GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				TECHNO_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				TECHNO_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
				TECHNO_BUTTON_MENUUP,		// MENU_BUTTON_UP
				TECHNO_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				TECHNO_BUTTON_START,		// MENU_BUTTON_START
				TECHNO_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				TECHNO_BUTTON_BACK,		// MENU_BUTTON_BACK
				TECHNO_BUTTON_COIN,		// MENU_BUTTON_COIN
				TECHNO_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				TECHNO_BUTTON_LEFT,		// MENU_BUTTON_LEFT
				TECHNO_BUTTON_RIGHT,		// MENU_BUTTON_RIGHT
				TECHNO_BUTTON_UP,		// MENU_BUTTON_UP
				TECHNO_BUTTON_DOWN,		// MENU_BUTTON_DOWN
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
	{	// GAME_POPN
		"popn",					// m_szName
		true,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"popn",					// m_szName
			NUM_POPN_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "Left White",		GameButtonType_Step,	KEY_Cz,		NO_DEFAULT_KEY },
				{ "Left Yellow",	GameButtonType_Step,	KEY_Cs,		NO_DEFAULT_KEY },
				{ "Left Green",		GameButtonType_Step,	KEY_Cx,		NO_DEFAULT_KEY },
				{ "Left Blue",		GameButtonType_Step,	KEY_Cd,		NO_DEFAULT_KEY },
				{ "Red",		GameButtonType_Step,	KEY_Cc,		NO_DEFAULT_KEY },
				{ "Right Blue",		GameButtonType_Step,	KEY_Cf,		NO_DEFAULT_KEY },
				{ "Right Green",	GameButtonType_Step,	KEY_Cv,		NO_DEFAULT_KEY },
				{ "Right Yellow",	GameButtonType_Step,	KEY_Cg,		NO_DEFAULT_KEY },
				{ "Right White",	GameButtonType_Step,	KEY_Cb,		NO_DEFAULT_KEY },
				{ "Back",		GameButtonType_Step,	KEY_ENTER,	NO_DEFAULT_KEY },
				{ "Start",		GameButtonType_Step,	KEY_BACKSLASH,	NO_DEFAULT_KEY },
				{ "Select",		GameButtonType_Step,	KEY_ESC,	NO_DEFAULT_KEY },
				{ "MenuLeft",		GameButtonType_Step,	KEY_LEFT,	NO_DEFAULT_KEY },
				{ "MenuRight",		GameButtonType_Step,	KEY_RIGHT,	NO_DEFAULT_KEY },
				{ "MenuUp",		GameButtonType_Step,	KEY_UP,		NO_DEFAULT_KEY },
				{ "MenuDown",		GameButtonType_Step,	KEY_DOWN,	NO_DEFAULT_KEY },
				{ "Coin",		GameButtonType_Step,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",		GameButtonType_Step,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				POPN_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				POPN_BUTTON_MENURIGHT,		// MENU_BUTTON_RIGHT
				POPN_BUTTON_MENUUP,		// MENU_BUTTON_UP
				POPN_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				POPN_BUTTON_START,		// MENU_BUTTON_START
				POPN_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				POPN_BUTTON_BACK,		// MENU_BUTTON_START
				POPN_BUTTON_COIN,		// MENU_BUTTON_COIN
				POPN_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				POPN_BUTTON_LEFT_BLUE,		// MENU_BUTTON_LEFT
				POPN_BUTTON_RIGHT_BLUE,		// MENU_BUTTON_RIGHT
				POPN_BUTTON_LEFT_YELLOW,	// MENU_BUTTON_UP
				POPN_BUTTON_RIGHT_YELLOW,	// MENU_BUTTON_DOWN
				POPN_BUTTON_RED,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W2,		// m_mapW1To
		TNS_W2,		// m_mapW2To
		TNS_W3,		// m_mapW3To
		TNS_W3,		// m_mapW4To
		TNS_Miss,	// m_mapW5To
	},
	{	// GAME_LIGHTS
		"lights",				// m_szName
		false,					// m_bCountNotesSeparately
		{					// m_InputScheme
			"lights",				// m_szName
			NUM_LIGHTS_BUTTONS,			// m_iButtonsPerController
			{	// m_szButtonNames
				{ "MarqueeUpLeft",	GameButtonType_Step,	KEY_Cq,		NO_DEFAULT_KEY },	
				{ "MarqueeUpRight",	GameButtonType_Step,	KEY_Cw,		NO_DEFAULT_KEY },
				{ "MarqueeLrLeft",	GameButtonType_Step,	KEY_Ce,		NO_DEFAULT_KEY },
				{ "MarqueeLrRight",	GameButtonType_Step,	KEY_Cr,		NO_DEFAULT_KEY },
				{ "ButtonsLeft",	GameButtonType_Step,	KEY_Ct,		NO_DEFAULT_KEY },
				{ "ButtonsRight",	GameButtonType_Step,	KEY_Cy,		NO_DEFAULT_KEY },
				{ "BassLeft",		GameButtonType_Step,	KEY_Cu,		NO_DEFAULT_KEY },
				{ "BassRight",		GameButtonType_Step,	KEY_Ci,		NO_DEFAULT_KEY },
				{ "Start",		GameButtonType_INVALID,	KEY_ENTER,	NO_DEFAULT_KEY },
				{ "Select",		GameButtonType_INVALID,	KEY_BACKSLASH,	NO_DEFAULT_KEY },
				{ "Back",		GameButtonType_INVALID,	KEY_ESC,	NO_DEFAULT_KEY },
				{ "MenuLeft",		GameButtonType_INVALID,	KEY_DEL,	NO_DEFAULT_KEY },
				{ "MenuRight",		GameButtonType_INVALID,	KEY_PGDN,	NO_DEFAULT_KEY },
				{ "MenuUp",		GameButtonType_INVALID,	KEY_HOME,	NO_DEFAULT_KEY },
				{ "MenuDown",		GameButtonType_INVALID,	KEY_END,	NO_DEFAULT_KEY },
				{ "Coin",		GameButtonType_INVALID,	KEY_F1,		NO_DEFAULT_KEY },
				{ "Operator",		GameButtonType_INVALID,	KEY_SCRLLOCK,	NO_DEFAULT_KEY },
			},
			{	// m_DedicatedMenuButton
				LIGHTS_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
				LIGHTS_BUTTON_MENURIGHT,	// MENU_BUTTON_RIGHT
				LIGHTS_BUTTON_MENUUP,		// MENU_BUTTON_UP
				LIGHTS_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
				LIGHTS_BUTTON_START,		// MENU_BUTTON_START
				LIGHTS_BUTTON_SELECT,		// MENU_BUTTON_SELECT
				LIGHTS_BUTTON_BACK,		// MENU_BUTTON_BACK
				LIGHTS_BUTTON_COIN,		// MENU_BUTTON_COIN
				LIGHTS_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
			},
			{	// m_SecondaryMenuButton
				LIGHTS_BUTTON_MARQUEE_UP_LEFT,	// MENU_BUTTON_LEFT
				LIGHTS_BUTTON_MARQUEE_UP_RIGHT,	// MENU_BUTTON_RIGHT
				LIGHTS_BUTTON_MARQUEE_LR_LEFT,	// MENU_BUTTON_UP
				LIGHTS_BUTTON_MARQUEE_LR_RIGHT,	// MENU_BUTTON_DOWN
				GameButton_Invalid,		// MENU_BUTTON_START
				GameButton_Invalid,		// MENU_BUTTON_SELECT
				GameButton_Invalid,		// MENU_BUTTON_BACK
				GameButton_Invalid,		// MENU_BUTTON_COIN
				GameButton_Invalid,		// MENU_BUTTON_OPERATOR
			},
		},
		TNS_W1,	// m_mapW1To
		TNS_W2,	// m_mapW2To
		TNS_W3,	// m_mapW3To
		TNS_W4,	// m_mapW4To
		TNS_W5,	// m_mapW5To
	},
};

static Style g_Styles[] = 
{
	{	// STYLE_DANCE_SINGLE
		&g_Games[GAME_DANCE],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single",			// m_szName
		STEPS_TYPE_DANCE_SINGLE,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		4,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
		false, // m_bNeedsZoomOutWith2Players
		true, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_DANCE_VERSUS
		&g_Games[GAME_DANCE],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versus",			// m_szName
		STEPS_TYPE_DANCE_SINGLE,	// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		4,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f, NULL },
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
		false, // m_bLockDifficulties
	},
	{	// STYLE_DANCE_DOUBLE
		&g_Games[GAME_DANCE],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double",			// m_szName
		STEPS_TYPE_DANCE_DOUBLE,	// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 4, 7, 6, 5, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_DANCE_COUPLE
		&g_Games[GAME_DANCE],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"couple",			// m_szName
		STEPS_TYPE_DANCE_COUPLE,	// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		4,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+DANCE_COL_SPACING*1.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_5,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*1.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
		false, // m_bNeedsZoomOutWith2Players
		true, // m_bCanUseBeginnerHelper
		true, // m_bLockDifficulties
	},
	{	// STYLE_DANCE_SOLO
		&g_Games[GAME_DANCE],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"solo",				// m_szName
		STEPS_TYPE_DANCE_SOLO,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		6,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*2.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*2.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING },
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_DANCE_EDIT_COUPLE
		&g_Games[GAME_DANCE],		// m_Game
		false,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"couple-edit",			// m_szName
		STEPS_TYPE_DANCE_COUPLE,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*4.0f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*3.0f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*2.0f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*1.0f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*2.0f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*3.0f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*4.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*4.0f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*3.0f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*2.0f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*1.0f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*2.0f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*3.0f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*4.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, Style::END_MAPPING },
			{ 4, 5, 6, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
/*	{	// STYLE_DANCE_SOLO_VERSUS 
		"dance-solo-versus",		// m_szName
 		STEPS_TYPE_DANCE_SOLO,		// m_StepsType
		ONE_PLAYER_ONE_CREDIT,		// m_StyleType
		6,				// m_iColsPerPlayer
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
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING },
			{ 0, 5, 3, 2, 1, 4, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,5,1,4,2,3		// outside in
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},	*/
	{	// STYLE_DANCE_ROUTINE
		&g_Games[GAME_DANCE],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"routine",			// m_szName
		STEPS_TYPE_DANCE_ROUTINE,	// m_StepsType
		TWO_PLAYERS_SHARED_SIDES,	// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 4, 7, 6, 5, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		true, // m_bLockDifficulties
		
	},
	{	// STYLE_PUMP_SINGLE
		&g_Games[GAME_PUMP],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single",			// m_szName
		STEPS_TYPE_PUMP_SINGLE,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PUMP_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PUMP_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_PUMP_VERSUS
		&g_Games[GAME_PUMP],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versus",			// m_szName
		STEPS_TYPE_PUMP_SINGLE,		// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PUMP_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PUMP_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_PUMP_HALFDOUBLE
		&g_Games[GAME_PUMP],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"halfdouble",			// m_szName
		STEPS_TYPE_PUMP_HALFDOUBLE,	// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		6,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2.5f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_3,	-PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	+PUMP_COL_SPACING*2.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2.5f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_3,	-PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	+PUMP_COL_SPACING*2.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ Style::NO_MAPPING, 1, 0, Style::NO_MAPPING, 2, Style::END_MAPPING },
			{ 3, Style::NO_MAPPING, 5, 4, Style::NO_MAPPING, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,5,2,4,3,1
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_PUMP_DOUBLE
		&g_Games[GAME_PUMP],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double",			// m_szName
		STEPS_TYPE_PUMP_DOUBLE,		// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		10,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*4.5f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*3.5f, NULL },
				{ TRACK_3,	-PUMP_COL_SPACING*2.5f, NULL },
				{ TRACK_4,	-PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_5,	-PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_7,	+PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_8,	+PUMP_COL_SPACING*2.5f, NULL },
				{ TRACK_9,	+PUMP_COL_SPACING*3.5f, NULL },
				{ TRACK_10,	+PUMP_COL_SPACING*4.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*4.5f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*3.5f, NULL },
				{ TRACK_3,	-PUMP_COL_SPACING*2.5f, NULL },
				{ TRACK_4,	-PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_5,	-PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+PUMP_COL_SPACING*0.5f, NULL },
				{ TRACK_7,	+PUMP_COL_SPACING*1.5f, NULL },
				{ TRACK_8,	+PUMP_COL_SPACING*2.5f, NULL },
				{ TRACK_9,	+PUMP_COL_SPACING*3.5f, NULL },
				{ TRACK_10,	+PUMP_COL_SPACING*4.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 6, 8, 7, 5, 9, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3,5,7,9,6,8
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_PUMP_COUPLE
		&g_Games[GAME_PUMP],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"couple",			// m_szName
		STEPS_TYPE_PUMP_COUPLE,		// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PUMP_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_6,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_7,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_8,	+PUMP_COL_SPACING*0.0f, NULL },
				{ TRACK_9,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_10,	+PUMP_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		true, // m_bLockDifficulties
	},
	{	// STYLE_PUMP_EDIT_COUPLE
		&g_Games[GAME_PUMP],		// m_Game
		false,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"couple-edit",			// m_szName
		STEPS_TYPE_PUMP_COUPLE,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		10,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PUMP_COL_SPACING*5.0f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*4.0f, NULL },
				{ TRACK_3,	-PUMP_COL_SPACING*3.0f, NULL },
				{ TRACK_4,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_5,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_6,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_7,	+PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_8,	+PUMP_COL_SPACING*3.0f, NULL },
				{ TRACK_9,	+PUMP_COL_SPACING*4.0f, NULL },
				{ TRACK_10,	+PUMP_COL_SPACING*5.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PUMP_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PUMP_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PUMP_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PUMP_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 3, 2, 0, 4, 6, 8, 7, 5, 9, Style::END_MAPPING },
			{ 1, 3, 2, 0, 4, 6, 8, 7, 5, 9, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,2,4,1,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_EZ2_SINGLE
		&g_Games[GAME_EZ2],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single",			// m_szName
		STEPS_TYPE_EZ2_SINGLE,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+EZ2_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+EZ2_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+EZ2_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+EZ2_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_EZ2_REAL
		&g_Games[GAME_EZ2],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"real",				// m_szName
		STEPS_TYPE_EZ2_REAL,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		7,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3.0f, NULL },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, NULL },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3.0f, NULL },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, NULL },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_EZ2_SINGLE_VERSUS
		&g_Games[GAME_EZ2],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versus",			// m_szName
		STEPS_TYPE_EZ2_SINGLE,		// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+EZ2_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+EZ2_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+EZ2_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+EZ2_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+EZ2_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_EZ2_REAL_VERSUS
		&g_Games[GAME_EZ2],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versusReal",			// m_szName
		STEPS_TYPE_EZ2_REAL,		// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		7,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3.0f, NULL },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, NULL },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_REAL_COL_SPACING*3.0f, NULL },
				{ TRACK_2,	-EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_3,	-EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_4,	+EZ2_REAL_COL_SPACING*0.0f, NULL },
				{ TRACK_5,	+EZ2_REAL_COL_SPACING*1.0f, NULL }, 
				{ TRACK_6,	+EZ2_REAL_COL_SPACING*2.0f, NULL },
				{ TRACK_7,	+EZ2_REAL_COL_SPACING*3.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
			{ 0, 6, 3, 2, 4, 1, 5, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			3,0,6,2,4,1,5 // This should be from back to front: Down, UpLeft, UpRight, Lower Left Hand, Lower Right Hand, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_EZ2_DOUBLE
		&g_Games[GAME_EZ2],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double",			// m_szName
		STEPS_TYPE_EZ2_DOUBLE,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		10,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-EZ2_COL_SPACING*4.5f, NULL },
				{ TRACK_2,	-EZ2_COL_SPACING*3.5f, NULL },
				{ TRACK_3,	-EZ2_COL_SPACING*2.5f, NULL },
				{ TRACK_4,	-EZ2_COL_SPACING*1.5f, NULL },
				{ TRACK_5,	-EZ2_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+EZ2_COL_SPACING*0.5f, NULL }, 
				{ TRACK_7,	+EZ2_COL_SPACING*1.5f, NULL },  
				{ TRACK_8,	+EZ2_COL_SPACING*2.5f, NULL },
				{ TRACK_9,	+EZ2_COL_SPACING*3.5f, NULL },
				{ TRACK_10,	+EZ2_COL_SPACING*4.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-EZ2_COL_SPACING*4.5f, NULL },
				{ TRACK_2,	-EZ2_COL_SPACING*3.5f, NULL },
				{ TRACK_3,	-EZ2_COL_SPACING*2.5f, NULL },
				{ TRACK_4,	-EZ2_COL_SPACING*1.5f, NULL },
				{ TRACK_5,	-EZ2_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+EZ2_COL_SPACING*0.5f, NULL }, 
				{ TRACK_7,	+EZ2_COL_SPACING*1.5f, NULL },  
				{ TRACK_8,	+EZ2_COL_SPACING*2.5f, NULL },
				{ TRACK_9,	+EZ2_COL_SPACING*3.5f, NULL },
				{ TRACK_10,	+EZ2_COL_SPACING*4.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 4, 2, 1, 3, Style::END_MAPPING },
			{ 5, 9, 7, 6, 8, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3,7,5,9,6,8 // This should be from back to front: Down, UpLeft, UpRight, Upper Left Hand, Upper Right Hand 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_PARA_SINGLE
		&g_Games[GAME_PARA],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single",			// m_szName
		STEPS_TYPE_PARA_SINGLE,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PARA_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PARA_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PARA_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PARA_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PARA_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PARA_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_PARA_VERSUS
		&g_Games[GAME_PARA],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"versus",			// m_szName
		STEPS_TYPE_PARA_VERSUS,		// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-PARA_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PARA_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PARA_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-PARA_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+PARA_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+PARA_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+PARA_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			2,0,4,1,3 
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_DS3DDX_SINGLE
		&g_Games[GAME_DS3DDX],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single",			// m_szName
		STEPS_TYPE_DS3DDX_SINGLE,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DS3DDX_COL_SPACING*3.0f, NULL },
				{ TRACK_2,	-DS3DDX_COL_SPACING*2.0f, NULL },
				{ TRACK_3,	-DS3DDX_COL_SPACING*1.0f, NULL },
				{ TRACK_4,	-DS3DDX_COL_SPACING*0.0f, NULL },
				{ TRACK_5,	+DS3DDX_COL_SPACING*0.0f, NULL },
				{ TRACK_6,	+DS3DDX_COL_SPACING*1.0f, NULL },
				{ TRACK_7,	+DS3DDX_COL_SPACING*2.0f, NULL },
				{ TRACK_8,	+DS3DDX_COL_SPACING*3.0f , NULL},
			},
			{	// PLAYER_2
				{ TRACK_1,	-DS3DDX_COL_SPACING*3.0f, NULL },
				{ TRACK_2,	-DS3DDX_COL_SPACING*2.0f, NULL },
				{ TRACK_3,	-DS3DDX_COL_SPACING*1.0f, NULL },
				{ TRACK_4,	-DS3DDX_COL_SPACING*0.0f, NULL },
				{ TRACK_5,	+DS3DDX_COL_SPACING*0.0f, NULL },
				{ TRACK_6,	+DS3DDX_COL_SPACING*1.0f, NULL },
				{ TRACK_7,	+DS3DDX_COL_SPACING*2.0f, NULL },
				{ TRACK_8,	+DS3DDX_COL_SPACING*3.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_BEAT_SINGLE5
		&g_Games[GAME_BEAT],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single5",			// m_szName
		STEPS_TYPE_BEAT_SINGLE5,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		6,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	+BEAT_COL_SPACING*3.0f, "scratch" },
			},
			{	// PLAYER_2
				{ TRACK_1,	-BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	+BEAT_COL_SPACING*3.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_BEAT_DOUBLE
		&g_Games[GAME_BEAT],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double5",			// m_szName
		STEPS_TYPE_BEAT_DOUBLE5,	// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		12,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-BEAT_COL_SPACING*6.0f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*5.0f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*4.0f, NULL },
				{ TRACK_4,	-BEAT_COL_SPACING*3.0f, NULL },
				{ TRACK_5,	-BEAT_COL_SPACING*2.0f, NULL },
				{ TRACK_6,	-BEAT_COL_SPACING*1.5f, "scratch" },
				{ TRACK_7,	+BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_8,	+BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_9,	+BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_10,	+BEAT_COL_SPACING*3.5f, NULL },
				{ TRACK_11,	+BEAT_COL_SPACING*4.5f, NULL },
				{ TRACK_12,	+BEAT_COL_SPACING*6.0f, "scratch" },
			},
			{	// PLAYER_2
				{ TRACK_1,	-BEAT_COL_SPACING*6.0f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*5.0f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*4.0f, NULL },
				{ TRACK_4,	-BEAT_COL_SPACING*3.0f, NULL },
				{ TRACK_5,	-BEAT_COL_SPACING*2.0f, NULL },
				{ TRACK_6,	-BEAT_COL_SPACING*1.5f, "scratch" },
				{ TRACK_7,	+BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_8,	+BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_9,	+BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_10,	+BEAT_COL_SPACING*3.5f, NULL },
				{ TRACK_11,	+BEAT_COL_SPACING*4.5f, NULL },
				{ TRACK_12,	+BEAT_COL_SPACING*6.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, 4, Style::NO_MAPPING, Style::NO_MAPPING, 5, 5, Style::END_MAPPING },
			{ 5, 6, 7, 8, 9, Style::NO_MAPPING, Style::NO_MAPPING, 10, 10, Style::END_MAPPING }
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8,9,10,11
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_BEAT_SINGLE7
		&g_Games[GAME_BEAT],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"single7",			// m_szName
		STEPS_TYPE_BEAT_SINGLE7,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_8,	-BEAT_COL_SPACING*3.5f, "scratch" },
				{ TRACK_1,	-BEAT_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+BEAT_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+BEAT_COL_SPACING*2.0f, NULL },
				{ TRACK_6,	+BEAT_COL_SPACING*3.0f, NULL },
				{ TRACK_7,	+BEAT_COL_SPACING*4.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-BEAT_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+BEAT_COL_SPACING*4.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_BEAT_DOUBLE7
		&g_Games[GAME_BEAT],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double7",			// m_szName
		STEPS_TYPE_BEAT_DOUBLE7,	// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		16,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_8,	-BEAT_COL_SPACING*8.0f, "scratch" },
				{ TRACK_1,	-BEAT_COL_SPACING*6.5f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*5.5f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*4.5f, NULL },
				{ TRACK_4,	-BEAT_COL_SPACING*3.5f, NULL },
				{ TRACK_5,	-BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_6,	-BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	-BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_9,	+BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_10,	+BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_11,	+BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_12,	+BEAT_COL_SPACING*3.5f, NULL },
				{ TRACK_13,	+BEAT_COL_SPACING*4.5f, NULL },
				{ TRACK_14,	+BEAT_COL_SPACING*5.5f, NULL },
				{ TRACK_15,	+BEAT_COL_SPACING*6.5f, NULL },
				{ TRACK_16,	+BEAT_COL_SPACING*8.0f, "scratch" },
			},
			{	// PLAYER_2
				{ TRACK_8,	-BEAT_COL_SPACING*8.0f, "scratch" },
				{ TRACK_1,	-BEAT_COL_SPACING*6.5f, NULL },
				{ TRACK_2,	-BEAT_COL_SPACING*5.5f, NULL },
				{ TRACK_3,	-BEAT_COL_SPACING*4.5f, NULL },
				{ TRACK_4,	-BEAT_COL_SPACING*3.5f, NULL },
				{ TRACK_5,	-BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_6,	-BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	-BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_9,	+BEAT_COL_SPACING*0.5f, NULL },
				{ TRACK_10,	+BEAT_COL_SPACING*1.5f, NULL },
				{ TRACK_11,	+BEAT_COL_SPACING*2.5f, NULL },
				{ TRACK_12,	+BEAT_COL_SPACING*3.5f, NULL },
				{ TRACK_13,	+BEAT_COL_SPACING*4.5f, NULL },
				{ TRACK_14,	+BEAT_COL_SPACING*5.5f, NULL },
				{ TRACK_15,	+BEAT_COL_SPACING*6.5f, NULL },
				{ TRACK_16,	+BEAT_COL_SPACING*8.0f, "scratch" },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 2, 3, 4, 5, 6, 7, 0, 0, Style::END_MAPPING },
			{ 8, 9, 10, 11, 12, 13, 14, 15, 15, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_MANIAX_SINGLE
		&g_Games[GAME_MANIAX],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single",			// m_szName
		STEPS_TYPE_MANIAX_SINGLE,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		4,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 2, 0, 3, Style::END_MAPPING },
			{ 1, 2, 0, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_MANIAX_VERSUS
		&g_Games[GAME_MANIAX],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versus",			// m_szName
		STEPS_TYPE_MANIAX_SINGLE,	// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		4,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+MANIAX_COL_SPACING*1.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 2, 0, 3, Style::END_MAPPING },
			{ 1, 2, 0, 3, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0, 1, 2, 3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_MANIAX_DOUBLE
		&g_Games[GAME_MANIAX],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double",			// m_szName
		STEPS_TYPE_MANIAX_DOUBLE,	// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-MANIAX_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-MANIAX_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+MANIAX_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+MANIAX_COL_SPACING*3.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-MANIAX_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-MANIAX_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+MANIAX_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+MANIAX_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+MANIAX_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+MANIAX_COL_SPACING*3.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 2, 0, 3, Style::END_MAPPING },
			{ 5, 6, 4, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_SINGLE4
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"single4",			// m_szName
		STEPS_TYPE_TECHNO_SINGLE4,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		4,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_SINGLE5
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"single5",			// m_szName
		STEPS_TYPE_TECHNO_SINGLE5,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
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
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_SINGLE8
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"single8",			// m_szName
		STEPS_TYPE_TECHNO_SINGLE8,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_3,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_6,	+TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_7,	+TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_8,	+TECHNO_COL_SPACING*4.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*4.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_3,	-TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_4,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_5,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_7,	+TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_8,	+TECHNO_COL_SPACING*2.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
			{ 1, 6, 4, 3, 2, 5, Style::NO_MAPPING, 0, 7, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		true, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_VERSUS4
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versus4",			// m_szName
		STEPS_TYPE_TECHNO_SINGLE4,	// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		4,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 0, 3, 2, 1, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_VERSUS5
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versus5",			// m_szName
		STEPS_TYPE_TECHNO_SINGLE5,	// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+TECHNO_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+TECHNO_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+TECHNO_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
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
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_VERSUS8
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		false,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"versus8",			// m_szName
		STEPS_TYPE_TECHNO_SINGLE8,	// m_StepsType
		TWO_PLAYERS_TWO_SIDES,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_VERSUS_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-TECHNO_VERSUS_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-TECHNO_VERSUS_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-TECHNO_VERSUS_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+TECHNO_VERSUS_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+TECHNO_VERSUS_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+TECHNO_VERSUS_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+TECHNO_VERSUS_COL_SPACING*3.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_VERSUS_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-TECHNO_VERSUS_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-TECHNO_VERSUS_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-TECHNO_VERSUS_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+TECHNO_VERSUS_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+TECHNO_VERSUS_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+TECHNO_VERSUS_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+TECHNO_VERSUS_COL_SPACING*3.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
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
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_DOUBLE4
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double4",			// m_szName
		STEPS_TYPE_TECHNO_DOUBLE4,	// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		8,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+TECHNO_COL_SPACING*3.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 3, 2, 1, Style::END_MAPPING },
			{ 4, 7, 6, 5, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_TECHNO_DOUBLE5
		&g_Games[GAME_TECHNO],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"double5",			// m_szName
		STEPS_TYPE_TECHNO_DOUBLE5,	// m_StepsType
		ONE_PLAYER_TWO_SIDES,		// m_StyleType
		10,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-TECHNO_COL_SPACING*4.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_3,	-TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_4,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_5,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_7,	+TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_8,	+TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_9,	+TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_10,	+TECHNO_COL_SPACING*4.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-TECHNO_COL_SPACING*4.5f, NULL },
				{ TRACK_2,	-TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_3,	-TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_4,	-TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_5,	-TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+TECHNO_COL_SPACING*0.5f, NULL },
				{ TRACK_7,	+TECHNO_COL_SPACING*1.5f, NULL },
				{ TRACK_8,	+TECHNO_COL_SPACING*2.5f, NULL },
				{ TRACK_9,	+TECHNO_COL_SPACING*3.5f, NULL },
				{ TRACK_10,	+TECHNO_COL_SPACING*4.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
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
		false, // m_bLockDifficulties
	},
	{	// STYLE_POPN_FIVE
		&g_Games[GAME_POPN],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"popn-five",			// m_szName
		STEPS_TYPE_POPN_FIVE,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		5,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-POPN5_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-POPN5_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+POPN5_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+POPN5_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+POPN5_COL_SPACING*2.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-POPN5_COL_SPACING*2.0f, NULL },
				{ TRACK_2,	-POPN5_COL_SPACING*1.0f, NULL },
				{ TRACK_3,	+POPN5_COL_SPACING*0.0f, NULL },
				{ TRACK_4,	+POPN5_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+POPN5_COL_SPACING*2.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, 4, Style::END_MAPPING },
			{ Style::NO_MAPPING, Style::NO_MAPPING, 0, 1, 2, 3, 4, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_POPN_NINE
		&g_Games[GAME_POPN],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		true,				// m_bUsedForDemonstration
		true,				// m_bUsedForHowToPlay
		"popn-nine",			// m_szName
		STEPS_TYPE_POPN_NINE,		// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		9,				// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-POPN9_COL_SPACING*4.0f, NULL },
				{ TRACK_2,	-POPN9_COL_SPACING*3.0f, NULL },
				{ TRACK_3,	-POPN9_COL_SPACING*2.0f, NULL },
				{ TRACK_4,	-POPN9_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+POPN9_COL_SPACING*0.0f, NULL },
				{ TRACK_6,	+POPN9_COL_SPACING*1.0f, NULL },
				{ TRACK_7,	+POPN9_COL_SPACING*2.0f, NULL },
				{ TRACK_8,	+POPN9_COL_SPACING*3.0f, NULL },
				{ TRACK_9,	+POPN9_COL_SPACING*4.0f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-POPN9_COL_SPACING*4.0f, NULL },
				{ TRACK_2,	-POPN9_COL_SPACING*3.0f, NULL },
				{ TRACK_3,	-POPN9_COL_SPACING*2.0f, NULL },
				{ TRACK_4,	-POPN9_COL_SPACING*1.0f, NULL },
				{ TRACK_5,	+POPN9_COL_SPACING*0.0f, NULL },
				{ TRACK_6,	+POPN9_COL_SPACING*1.0f, NULL },
				{ TRACK_7,	+POPN9_COL_SPACING*2.0f, NULL },
				{ TRACK_8,	+POPN9_COL_SPACING*3.0f, NULL },
				{ TRACK_9,	+POPN9_COL_SPACING*4.0f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7,8
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
	{	// STYLE_LIGHTS_CABINET
		&g_Games[GAME_LIGHTS],		// m_Game
		true,				// m_bUsedForGameplay
		true,				// m_bUsedForEdit
		false,				// m_bUsedForDemonstration
		false,				// m_bUsedForHowToPlay
		"cabinet",			// m_szName
		STEPS_TYPE_LIGHTS_CABINET,	// m_StepsType
		ONE_PLAYER_ONE_SIDE,		// m_StyleType
		NUM_CabinetLight,		// m_iColsPerPlayer
		{	// m_ColumnInfo[NUM_PLAYERS][MAX_COLS_PER_PLAYER];
			{	// PLAYER_1
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f, NULL },
			},
			{	// PLAYER_2
				{ TRACK_1,	-DANCE_COL_SPACING*3.5f, NULL },
				{ TRACK_2,	-DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_3,	-DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_4,	-DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_5,	+DANCE_COL_SPACING*0.5f, NULL },
				{ TRACK_6,	+DANCE_COL_SPACING*1.5f, NULL },
				{ TRACK_7,	+DANCE_COL_SPACING*2.5f, NULL },
				{ TRACK_8,	+DANCE_COL_SPACING*3.5f, NULL },
			},
		},
		{	// m_iInputColumn[NUM_GameController][NUM_GameButton]
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, Style::END_MAPPING },
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, Style::END_MAPPING },
		},
		{	// m_iColumnDrawOrder[MAX_COLS_PER_PLAYER];
			0,1,2,3,4,5,6,7
		},
		false, // m_bNeedsZoomOutWith2Players
		false, // m_bCanUseBeginnerHelper
		false, // m_bLockDifficulties
	},
};

#define NUM_STYLES ARRAYLEN(g_Styles)


GameManager::GameManager()
{
	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "GAMEMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

GameManager::~GameManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "GAMEMAN" );
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

void GameManager::GetDemonstrationStylesForGame( const Game *pGame, vector<const Style*> &vpStylesOut ) const
{
	vpStylesOut.clear();

	for( unsigned s=0; s<NUM_STYLES; s++ ) 
	{
		const Style* style = &g_Styles[s];
		if( style->m_pGame == pGame && style->m_bUsedForDemonstration )
			vpStylesOut.push_back( style );
	}
	
	ASSERT( vpStylesOut.size()>0 );	// this Game is missing a Style that can be used with the demonstration
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
	return NOTESKIN->DoNoteSkinsExistForGame( pGame );
}

int GameManager::StepsTypeToNumTracks( StepsType st )
{
	ASSERT_M( st < NUM_StepsType, ssprintf("%i", st) );
	return StepsTypes[st].NumTracks;
}

bool GameManager::CanAutoGenStepsType( StepsType st )
{
	ASSERT_M( st < NUM_StepsType, ssprintf("%d", st) );
	return !(StepsTypes[st].flags & ST_FLAGS_DONT_AUTOGEN);
}

StepsType GameManager::StringToStepsType( RString sStepsType )
{
	sStepsType.MakeLower();

	// HACK!  We elminitated "ez2-single-hard", but we should still handle it.
	if( sStepsType == "ez2-single-hard" )
		sStepsType = "ez2-single";

	// HACK!  "para-single" used to be called just "para"
	if( sStepsType == "para" )
		sStepsType = "para-single";

	for( int i=0; i<NUM_StepsType; i++ )
		if( StepsTypes[i].name == sStepsType )
			return StepsType(i);
	
	// invalid StepsType
	LOG->Warn( "Invalid StepsType string '%s' encountered.  Assuming this is 'dance-single'.", sStepsType.c_str() );
	return STEPS_TYPE_DANCE_SINGLE;
}

RString GameManager::StepsTypeToString( StepsType st )
{
	ASSERT_M( st < NUM_StepsType, ssprintf("%i", st) );
	return StepsTypes[st].name;
}

RString GameManager::StepsTypeToLocalizedString( StepsType st )
{
	RString s = StepsTypeToString( st );
	if( THEME->HasString( "StepsType", s ) )
		return THEME->GetString( "StepsType", s );
	else
		return s;
}

RString GameManager::StyleToLocalizedString( const Style* style )
{
	RString s = style->m_szName;
	s = Capitalize( s );
	if( THEME->HasString( "Style", s ) )
		return THEME->GetString( "Style", s );
	else
		return s;
}

const Game* GameManager::StringToGameType( RString sGameType )
{
	for( int i=0; i<NUM_GAMES; i++ )
		if( !sGameType.CompareNoCase(g_Games[i].m_szName) )
			return &g_Games[i];

	return NULL;
}


const Style* GameManager::GameAndStringToStyle( const Game *game, RString sStyle )
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

// lua start
#include "LuaBinding.h"

class LunaGameManager: public Luna<GameManager>
{
public:
	static int StepsTypeToLocalizedString( T* p, lua_State *L )	{ lua_pushstring(L, p->StepsTypeToLocalizedString(Enum::Check<StepsType>(L, 1)) ); return 1; }
	static int GetFirstStepsTypeForGame( T* p, lua_State *L )
	{
		Game *pGame = Luna<Game>::check( L, 1 );

		vector<StepsType> vstAddTo;
		p->GetStepsTypesForGame( pGame, vstAddTo );
		ASSERT( !vstAddTo.empty() );
		StepsType st = vstAddTo[0];
		lua_pushnumber(L, st);
		return 1;
	}

	LunaGameManager()
	{
		ADD_METHOD( StepsTypeToLocalizedString );
		ADD_METHOD( GetFirstStepsTypeForGame );
	}
};

LUA_REGISTER_CLASS( GameManager )
// lua end


/*
 * (c) 2001-2006 Chris Danford, Glenn Maynard
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
