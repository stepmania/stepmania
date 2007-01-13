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
	{ "lights-cabinet",	NUM_CabinetLight,	ST_FLAGS_DONT_AUTOGEN }, // XXX disable lights autogen for now
};


//
// Important:  Every game must define the buttons: "Start", "Back", "MenuLeft", "Operator" and "MenuRight"
//
const InputMapping g_AutoKeyMappings_Dance[] = 
{
	{ 0, KEY_DEL,		GAME_BUTTON_MENULEFT,		false },
	{ 0, KEY_PGDN,		GAME_BUTTON_MENURIGHT,		false },
	{ 0, KEY_HOME,		GAME_BUTTON_MENUUP,		false },
	{ 0, KEY_END,		GAME_BUTTON_MENUDOWN,		false },
	{ 0, KEY_LEFT,		DANCE_BUTTON_LEFT,		false },
	{ 0, KEY_RIGHT,		DANCE_BUTTON_RIGHT,		false },
	{ 0, KEY_UP,		DANCE_BUTTON_UP,		false },
	{ 0, KEY_DOWN,		DANCE_BUTTON_DOWN,		false },
	{ 0, KEY_KP_SLASH,	GAME_BUTTON_MENULEFT,		true },
	{ 0, KEY_KP_ASTERISK,	GAME_BUTTON_MENURIGHT,		true },
	{ 0, KEY_KP_HYPHEN,	GAME_BUTTON_MENUUP,		true },
	{ 0, KEY_KP_PLUS,	GAME_BUTTON_MENUDOWN,		true },
	{ 0, KEY_KP_C4,		DANCE_BUTTON_LEFT,		true },
	{ 0, KEY_KP_C6,		DANCE_BUTTON_RIGHT,		true },
	{ 0, KEY_KP_C7,		DANCE_BUTTON_UP,		true },
	{ 0, KEY_KP_C9,		DANCE_BUTTON_DOWN,		true },
	{ 0, KEY_KP_C7,		DANCE_BUTTON_UPLEFT,		true },
	{ 0, KEY_KP_C9,		DANCE_BUTTON_UPRIGHT,		true },
	InputMapping_END
};

static const Game g_Game_Dance = 
{
	"dance",					// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"dance",				// m_szName
		NUM_DANCE_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left",		MENU_BUTTON_LEFT },
			{ "Right",		MENU_BUTTON_RIGHT },
			{ "Up",			MENU_BUTTON_UP },
			{ "Down",		MENU_BUTTON_DOWN },
			{ "UpLeft",		MenuButton_Invalid },
			{ "UpRight",		MenuButton_Invalid },
		},
		g_AutoKeyMappings_Dance
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Pump[] = 
{
	{ 0, KEY_Cq,		PUMP_BUTTON_UPLEFT,		false },
	{ 0, KEY_Ce,		PUMP_BUTTON_UPRIGHT,		false },
	{ 0, KEY_Cs,		PUMP_BUTTON_CENTER,		false },
	{ 0, KEY_Cz,		PUMP_BUTTON_DOWNLEFT,		false },
	{ 0, KEY_Cc,		PUMP_BUTTON_DOWNRIGHT,		false },
	{ 0, KEY_KP_C7,		PUMP_BUTTON_UPLEFT,		true },
	{ 0, KEY_KP_C9,		PUMP_BUTTON_UPRIGHT,		true },
	{ 0, KEY_KP_C5,		PUMP_BUTTON_CENTER,		true },
	{ 0, KEY_KP_C1,		PUMP_BUTTON_DOWNLEFT,		true },
	{ 0, KEY_KP_C3,		PUMP_BUTTON_DOWNRIGHT,		true },

	// unmap confusing default MenuButtons
	{ 0, KEY_KP_C8,		GameButton_Invalid,		false },
	{ 0, KEY_KP_C2,		GameButton_Invalid,		false },
	{ 0, KEY_KP_C4,		GameButton_Invalid,		false },
	{ 0, KEY_KP_C6,		GameButton_Invalid,		false },

	InputMapping_END
};

static const Game g_Game_Pump = 
{
	"pump",						// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"pump",					// m_szName
		NUM_PUMP_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "UpLeft",		MENU_BUTTON_UP },
			{ "UpRight",		MENU_BUTTON_DOWN },
			{ "Center",		MENU_BUTTON_START },
			{ "DownLeft",		MENU_BUTTON_LEFT },
			{ "DownRight",		MENU_BUTTON_RIGHT },
		},
		g_AutoKeyMappings_Pump
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Ez2[] = 
{
	{ 0, KEY_Cz,		EZ2_BUTTON_FOOTUPLEFT,		false },
	{ 0, KEY_Cb,		EZ2_BUTTON_FOOTUPRIGHT,		false },
	{ 0, KEY_Cc,		EZ2_BUTTON_FOOTDOWN,		false },
	{ 0, KEY_Cx,		EZ2_BUTTON_HANDUPLEFT,		false },
	{ 0, KEY_Cv,		EZ2_BUTTON_HANDUPRIGHT,		false },
	{ 0, KEY_Cs,		EZ2_BUTTON_HANDLRLEFT,		false },
	{ 0, KEY_Cf,		EZ2_BUTTON_HANDLRRIGHT,		false },
	InputMapping_END
};

static const Game g_Game_Ez2 = 
{
	"ez2",						// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"ez2",					// m_szName
		NUM_EZ2_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "FootUpLeft",		MENU_BUTTON_UP },
			{ "FootUpRight",	MENU_BUTTON_DOWN },
			{ "FootDown",		MENU_BUTTON_START },
			{ "HandUpLeft",		MENU_BUTTON_LEFT },
			{ "HandUpRight",	MENU_BUTTON_RIGHT },
			{ "HandLrLeft",		MenuButton_Invalid },
			{ "HandLrRight",	MenuButton_Invalid },
		},
		g_AutoKeyMappings_Ez2
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W2,		// m_mapW1To
	TNS_W2,		// m_mapW2To
	TNS_W2,		// m_mapW3To
	TNS_W4,		// m_mapW4To
	TNS_Miss,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Para[] = 
{
	{ 0, KEY_Cz,		PARA_BUTTON_LEFT,		false },
	{ 0, KEY_Cx,		PARA_BUTTON_UPLEFT,		false },
	{ 0, KEY_Cc,		PARA_BUTTON_UP,			false },
	{ 0, KEY_Cv,		PARA_BUTTON_UPRIGHT,		false },
	{ 0, KEY_Cb,		PARA_BUTTON_RIGHT,		false },
	InputMapping_END
};

static const Game g_Game_Para = 
{
	"para",						// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"para",					// m_szName
		NUM_PARA_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left",		MENU_BUTTON_LEFT },
			{ "UpLeft",		MENU_BUTTON_DOWN },
			{ "Up",			MenuButton_Invalid },
			{ "UpRight",		MENU_BUTTON_UP },
			{ "Right",		MENU_BUTTON_RIGHT },
		},
		g_AutoKeyMappings_Para
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_DS3DDX[] = 
{
	{ 0, KEY_Ca,		DS3DDX_BUTTON_HANDLEFT,		false },
	{ 0, KEY_Cz,		DS3DDX_BUTTON_FOOTDOWNLEFT,	false },
	{ 0, KEY_Cq,		DS3DDX_BUTTON_FOOTUPLEFT,	false },
	{ 0, KEY_Cw,		DS3DDX_BUTTON_HANDUP,		false },
	{ 0, KEY_Cx,		DS3DDX_BUTTON_HANDDOWN,		false },
	{ 0, KEY_Ce,		DS3DDX_BUTTON_FOOTUPRIGHT,	false },
	{ 0, KEY_Cc,		DS3DDX_BUTTON_FOOTDOWNRIGHT,	false },
	{ 0, KEY_Cd,		DS3DDX_BUTTON_HANDRIGHT,	false },
	InputMapping_END
};

static const Game g_Game_DS3DDX = 
{
	"ds3ddx",					// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"ds3ddx",				// m_szName
		NUM_DS3DDX_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "HandLeft",		MENU_BUTTON_LEFT },
			{ "FootDownLeft",	MenuButton_Invalid },
			{ "FootUpLeft",		MenuButton_Invalid },
			{ "HandUp",		MENU_BUTTON_UP },
			{ "HandDown",		MENU_BUTTON_DOWN },
			{ "FootUpRight",	MenuButton_Invalid },
			{ "FootDownRight",	MenuButton_Invalid },
			{ "HandRight",		MENU_BUTTON_RIGHT },
		},
		g_AutoKeyMappings_DS3DDX
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Beat[] = 
{
	{ 0, KEY_Cm,		BEAT_BUTTON_KEY1,		false },
	{ 0, KEY_Ck,		BEAT_BUTTON_KEY2,		false },
	{ 0, KEY_COMMA,		BEAT_BUTTON_KEY3,		false },
	{ 0, KEY_Cl,		BEAT_BUTTON_KEY4,		false },
	{ 0, KEY_PERIOD,	BEAT_BUTTON_KEY5,		false },
	{ 0, KEY_SEMICOLON,	BEAT_BUTTON_KEY6,		false },
	{ 0, KEY_SLASH,		BEAT_BUTTON_KEY7,		false },
	{ 0, KEY_LSHIFT,	BEAT_BUTTON_SCRATCHUP,		false },
	InputMapping_END
};

static const Game g_Game_Beat = 
{
	"beat",						// m_szName
	true,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"beat",					// m_szName
		NUM_BEAT_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Key1",		MENU_BUTTON_LEFT },
			{ "Key2",		MenuButton_Invalid },
			{ "Key3",		MENU_BUTTON_RIGHT },
			{ "Key4",		MenuButton_Invalid },
			{ "Key5",		MenuButton_Invalid },
			{ "Key6",		MenuButton_Invalid },
			{ "Key7",		MenuButton_Invalid },
			{ "Scratch up",		MENU_BUTTON_UP },
			{ "Scratch down",	MENU_BUTTON_DOWN },
		},
		g_AutoKeyMappings_Beat
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Maniax[] = 
{
	{ 0, KEY_Ca,		MANIAX_BUTTON_HANDUPLEFT,	false },
	{ 0, KEY_Cs,		MANIAX_BUTTON_HANDUPRIGHT,	false },
	{ 0, KEY_Cz,		MANIAX_BUTTON_HANDLRLEFT,	false },
	{ 0, KEY_Cx,		MANIAX_BUTTON_HANDLRRIGHT,	false },
	{ 0, KEY_KP_C4,		MANIAX_BUTTON_HANDUPLEFT,	true },
	{ 0, KEY_KP_C5,		MANIAX_BUTTON_HANDUPRIGHT,	true },
	{ 0, KEY_KP_C1,		MANIAX_BUTTON_HANDLRLEFT,	true },
	{ 0, KEY_KP_C2,		MANIAX_BUTTON_HANDLRRIGHT,	true },
	InputMapping_END
};

static const Game g_Game_Maniax = 
{
	"maniax",					// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"maniax",				// m_szName
		NUM_MANIAX_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "HandUpLeft",		MENU_BUTTON_LEFT },
			{ "HandUpRight",	MENU_BUTTON_RIGHT },
			{ "HandLrLeft",		MENU_BUTTON_DOWN },
			{ "HandLrRight",	MENU_BUTTON_UP },
		},
		g_AutoKeyMappings_Maniax
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Techno[] = 
{
	{ 0, KEY_Ca,		TECHNO_BUTTON_LEFT,	false },
	{ 0, KEY_Cd,		TECHNO_BUTTON_RIGHT,	false },
	{ 0, KEY_Cw,		TECHNO_BUTTON_UP,	false },
	{ 0, KEY_Cx,		TECHNO_BUTTON_DOWN,	false },
	{ 0, KEY_Cq,		TECHNO_BUTTON_UPLEFT,	false },
	{ 0, KEY_Ce,		TECHNO_BUTTON_UPRIGHT,	false },
	{ 0, KEY_Cs,		TECHNO_BUTTON_CENTER,	false },
	{ 0, KEY_Cz,		TECHNO_BUTTON_DOWNLEFT,	false },
	{ 0, KEY_Cc,		TECHNO_BUTTON_DOWNRIGHT,false },
	{ 0, KEY_KP_C4,		TECHNO_BUTTON_LEFT,	true },
	{ 0, KEY_KP_C6,		TECHNO_BUTTON_RIGHT,	true },
	{ 0, KEY_KP_C8,		TECHNO_BUTTON_UP,	true },
	{ 0, KEY_KP_C2,		TECHNO_BUTTON_DOWN,	true },
	{ 0, KEY_KP_C7,		TECHNO_BUTTON_UPLEFT,	true },
	{ 0, KEY_KP_C9,		TECHNO_BUTTON_UPRIGHT,	true },
	{ 0, KEY_KP_C5,		TECHNO_BUTTON_CENTER,	true },
	{ 0, KEY_KP_C1,		TECHNO_BUTTON_DOWNLEFT,	true },
	{ 0, KEY_KP_C3,		TECHNO_BUTTON_DOWNRIGHT,true },
	InputMapping_END
};

static const Game g_Game_Techno = 
{
	"techno",					// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"techno",				// m_szName
		NUM_TECHNO_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left",		MENU_BUTTON_LEFT },
			{ "Right",		MENU_BUTTON_RIGHT },
			{ "Up",			MENU_BUTTON_UP },
			{ "Down",		MENU_BUTTON_DOWN },
			{ "UpLeft",		MenuButton_Invalid },
			{ "UpRight",		MenuButton_Invalid },
			{ "Center",		MenuButton_Invalid },
			{ "DownLeft",		MenuButton_Invalid },
			{ "DownRight",		MenuButton_Invalid },
		},
		g_AutoKeyMappings_Techno
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Popn[] = 
{
	{ 0, KEY_Cz,		POPN_BUTTON_LEFT_WHITE,	false },
	{ 0, KEY_Cs,		POPN_BUTTON_LEFT_YELLOW,false },
	{ 0, KEY_Cx,		POPN_BUTTON_LEFT_GREEN,	false },
	{ 0, KEY_Cd,		POPN_BUTTON_LEFT_BLUE,	false },
	{ 0, KEY_Cc,		POPN_BUTTON_RED,	false },
	{ 0, KEY_Cf,		POPN_BUTTON_RIGHT_BLUE,	false },
	{ 0, KEY_Cv,		POPN_BUTTON_RIGHT_GREEN,false },
	{ 0, KEY_Cg,		POPN_BUTTON_RIGHT_YELLOW,false },
	{ 0, KEY_Cb,		POPN_BUTTON_RIGHT_WHITE,false },
	InputMapping_END
};

static const Game g_Game_Popn = 
{
	"popn",						// m_szName
	true,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"popn",					// m_szName
		NUM_POPN_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "Left White",		MenuButton_Invalid },
			{ "Left Yellow",	MENU_BUTTON_UP },
			{ "Left Green",		MenuButton_Invalid },
			{ "Left Blue",		MENU_BUTTON_LEFT },
			{ "Red",		MENU_BUTTON_START },
			{ "Right Blue",		MENU_BUTTON_RIGHT },
			{ "Right Green",	MenuButton_Invalid },
			{ "Right Yellow",	MENU_BUTTON_DOWN },
			{ "Right White",	MenuButton_Invalid },
		},
		g_AutoKeyMappings_Popn
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W2,		// m_mapW1To
	TNS_W2,		// m_mapW2To
	TNS_W3,		// m_mapW3To
	TNS_W3,		// m_mapW4To
	TNS_Miss,	// m_mapW5To
};

const InputMapping g_AutoKeyMappings_Lights[] = 
{
	{ 0, KEY_Cq,		LIGHTS_BUTTON_MARQUEE_UP_LEFT,	false },
	{ 0, KEY_Cw,		LIGHTS_BUTTON_MARQUEE_UP_RIGHT,	false },
	{ 0, KEY_Ce,		LIGHTS_BUTTON_MARQUEE_LR_LEFT,	false },
	{ 0, KEY_Cr,		LIGHTS_BUTTON_MARQUEE_LR_RIGHT,	false },
	{ 0, KEY_Ct,		LIGHTS_BUTTON_BUTTONS_LEFT,	false },
	{ 0, KEY_Cy,		LIGHTS_BUTTON_BUTTONS_RIGHT,	false },
	{ 0, KEY_Cu,		LIGHTS_BUTTON_BASS_LEFT,	false },
	{ 0, KEY_Ci,		LIGHTS_BUTTON_BASS_RIGHT,	false },
	InputMapping_END
};

static const Game g_Game_Lights = 
{
	"lights",					// m_szName
	false,						// m_bCountNotesSeparately
	false,						// m_bAllowHopos
	{						// m_InputScheme
		"lights",				// m_szName
		NUM_LIGHTS_BUTTONS,			// m_iButtonsPerController
		{	// m_szButtonNames
			{ "MarqueeUpLeft",	MENU_BUTTON_LEFT },
			{ "MarqueeUpRight",	MENU_BUTTON_RIGHT },
			{ "MarqueeLrLeft",	MENU_BUTTON_UP },
			{ "MarqueeLrRight",	MENU_BUTTON_DOWN },
			{ "ButtonsLeft",	MenuButton_Invalid },
			{ "ButtonsRight",	MenuButton_Invalid },
			{ "BassLeft",		MenuButton_Invalid },
			{ "BassRight",		MenuButton_Invalid },
		},
		g_AutoKeyMappings_Lights
	},
	{
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
		{ GameButtonType_Step },
	},
	TNS_W1,	// m_mapW1To
	TNS_W2,	// m_mapW2To
	TNS_W3,	// m_mapW3To
	TNS_W4,	// m_mapW4To
	TNS_W5,	// m_mapW5To
};

static const Game *g_Games[] = 
{
	&g_Game_Dance,
	&g_Game_Pump,
	&g_Game_Ez2,
	&g_Game_Para,
	&g_Game_DS3DDX,
	&g_Game_Beat,
	&g_Game_Maniax,
	&g_Game_Techno,
	&g_Game_Popn,
	&g_Game_Lights,
};

static Style g_Styles[] = 
{
	{	// STYLE_DANCE_SINGLE
		&g_Game_Dance,			// m_Game
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
		&g_Game_Dance,			// m_Game
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
		&g_Game_Dance,			// m_Game
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
		&g_Game_Dance,			// m_Game
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
		&g_Game_Dance,			// m_Game
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
		&g_Game_Dance,			// m_Game
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
		&g_Game_Dance,			// m_Game
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
		&g_Game_Pump,		// m_Game
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
		&g_Game_Pump,		// m_Game
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
		&g_Game_Pump,		// m_Game
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
		&g_Game_Pump,		// m_Game
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
		&g_Game_Pump,		// m_Game
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
		&g_Game_Pump,		// m_Game
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
		&g_Game_Ez2,		// m_Game
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
		&g_Game_Ez2,		// m_Game
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
		&g_Game_Ez2,		// m_Game
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
		&g_Game_Ez2,		// m_Game
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
		&g_Game_Ez2,		// m_Game
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
		&g_Game_Para,		// m_Game
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
		&g_Game_Para,		// m_Game
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
		&g_Game_DS3DDX,		// m_Game
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
		&g_Game_Beat,		// m_Game
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
		&g_Game_Beat,		// m_Game
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
		&g_Game_Beat,		// m_Game
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
		&g_Game_Beat,		// m_Game
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
		&g_Game_Maniax,		// m_Game
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
		&g_Game_Maniax,		// m_Game
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
		&g_Game_Maniax,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Techno,		// m_Game
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
		&g_Game_Popn,		// m_Game
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
		&g_Game_Popn,		// m_Game
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
		&g_Game_Lights,		// m_Game
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
	for( size_t g=0; g<ARRAYSIZE(g_Games); ++g )
	{
		const Game *pGame = g_Games[g];
		if( IsGameEnabled( pGame ) )
			aGamesOut.push_back( pGame );
	}
}

const Game* GameManager::GetDefaultGame() const
{
	return g_Games[0];
}

int GameManager::GetIndexFromGame( const Game* pGame ) const
{
	for( size_t g=0; g<ARRAYSIZE(g_Games); ++g )
	{
		if( g_Games[g] == pGame )
			return g;
	}
	ASSERT(0);
	return 0;
}

const Game* GameManager::GetGameFromIndex( int index ) const
{
	ASSERT( index >= 0 );
	ASSERT( index < (int) ARRAYSIZE(g_Games) );
	return g_Games[index];
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
	for( size_t i=0; i<ARRAYSIZE(g_Games); ++i )
		if( !sGameType.CompareNoCase(g_Games[i]->m_szName) )
			return g_Games[i];

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
