#include "global.h"
#include "InputMapper.h"
#include "IniFile.h"
#include "MessageManager.h"
#include "RageLog.h"
#include "InputFilter.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageInput.h"
#include "SpecialFiles.h"
#include "LocalizedString.h"

static Preference<RString> g_sLastSeenInputDevices( "LastSeenInputDevices", "" );
static Preference<bool> g_bAutoMapOnJoyChange( "AutoMapOnJoyChange", true );

namespace
{
	// lookup for efficiency from a DeviceInput to a GameInput
	// This is repopulated every time m_PItoDI changes by calling UpdateTempDItoPI().
	map<DeviceInput, GameInput> g_tempDItoGI;

	PlayerNumber g_JoinControllers;
};

InputMapper*	INPUTMAPPER = NULL;	// global and accessable from anywhere in our program

static GameButton g_DedicatedMenuButtons[NUM_MenuButton] =
{
	GAME_BUTTON_MENULEFT,		// MENU_BUTTON_LEFT
	GAME_BUTTON_MENURIGHT,		// MENU_BUTTON_RIGHT
	GAME_BUTTON_MENUUP,		// MENU_BUTTON_UP
	GAME_BUTTON_MENUDOWN,		// MENU_BUTTON_DOWN
	GAME_BUTTON_START,		// MENU_BUTTON_START
	GAME_BUTTON_SELECT,		// MENU_BUTTON_SELECT
	GAME_BUTTON_BACK,		// MENU_BUTTON_BACK
	GAME_BUTTON_COIN,		// MENU_BUTTON_COIN
	GAME_BUTTON_OPERATOR		// MENU_BUTTON_OPERATOR
};

InputMapper::InputMapper()
{
	g_JoinControllers = PLAYER_INVALID;
	m_pInputScheme = NULL;
}


InputMapper::~InputMapper()
{
	SaveMappingsToDisk();
	g_tempDItoGI.clear();
}

void InputMapper::ClearAllMappings()
{
	FOREACH_GameController( i )
		FOREACH_GameButton(j)
			for( int k=0; k<NUM_GAME_TO_DEVICE_SLOTS; k++ )
				m_GItoDI[i][j][k].MakeInvalid();
	UpdateTempDItoGI();
}

void InputMapper::AddDefaultMappingsForCurrentGameIfUnmapped()
{
	// Clear default mappings.  Default mappings are in the third slot.
	FOREACH_GameController( i )
		FOREACH_GameButton(j)
			ClearFromInputMap( GameInput(i, j), 2 );

	FOREACH_GameController( c )
	{
		FOREACH_GameButtonInScheme( m_pInputScheme, b )
		{
			DeviceButton key = m_pInputScheme->m_GameButtonInfo[b].m_iDefaultKeyboardKey[c];
			if( key == NO_DEFAULT_KEY )
				continue;
			DeviceInput DeviceI( DEVICE_KEYBOARD, key );
			GameInput GameI( c, b );
			if( !IsMapped(DeviceI) )	// if this key isn't already being used by another user-made mapping
				SetInputMap( DeviceI, GameI, 2 );   
		}
	}
}

struct AutoJoyMapping
{
	const char *szGame;
	const char *szDriverRegex;	// reported by InputHandler
	const char *szControllerName;	// the product name of the controller
	struct InputMapper::Mapping maps[32];
};
#define END_MARKER	{-1, DeviceButton_Invalid, -1, false },	// end marker
const AutoJoyMapping g_AutoJoyMappings[] = 
{
	{
		"dance",
		"GIC USB Joystick",
		"Boom USB convertor (black/gray)",
		{
			{ 0, JOY_BUTTON_16,	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_14,	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_13,	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_15,	DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_12,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"4 axis 16 button joystick",
		"EMS USB2",
		{
			{ 0, JOY_BUTTON_16,	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_14,	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_13,	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_15,	DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"GamePad Pro USB ",	// yes, there is a space at the end
		"GamePad Pro USB",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,	false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_5,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_6,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"SideWinder Game Pad USB version 1.0",
		"SideWinder Game Pad USB",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,	false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_5,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"4 axis 12 button joystick with hat switch",
		"Super Joy Box 5",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,	false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"MP-8866 Dual USB Joypad",
		"Super Dual Box",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,	false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_5,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_6,	DANCE_BUTTON_UPRIGHT,	false },
			{ 1, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },
			{ 1, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"NTPAD",
		"NTPAD",
		{
			{ 0, JOY_BUTTON_13,	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_15,	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_16,	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_14,	DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_5,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_6,	DANCE_BUTTON_UPRIGHT,	false },
			{ 1, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },
			{ 1, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"Psx Gamepad",
		"PSXPAD",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,	false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_5,	DANCE_BUTTON_UPRIGHT,	false },
			{ 1, JOY_BUTTON_8,	DANCE_BUTTON_UPLEFT,	false },
			{ 1, JOY_BUTTON_6,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"XBOX Gamepad Plugin V0.01",
		"X-Box gamepad",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,	false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_DOWN,	false },	// A
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_RIGHT,	false },	// B
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_LEFT,	false },	// X
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_UP,	false },	// Y
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,	false },	// L shoulder
			{ 0, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,	false },	// R shoulder
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_BACK,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"0b43:0003", // The EMS USB2 doesn't provide a model string, so Linux 
			     // just gives us the VendorID and ModelID in hex.
		"EMS USB2",
		{
			// Player 1.
			{ 0, JOY_BUTTON_16,	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_14,	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_13,	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_15,	DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_4,	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_2,	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_1,	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_3,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPLEFT,    false },
			{ 0, JOY_BUTTON_8,	DANCE_BUTTON_UPRIGHT,   false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_SELECT,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_5,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_6,	GAME_BUTTON_COIN,	false },
			// Player 2.
			{ 0, JOY_BUTTON_32,	DANCE_BUTTON_LEFT,	true },
			{ 0, JOY_BUTTON_30,	DANCE_BUTTON_RIGHT,	true },
			{ 0, JOY_BUTTON_29,	DANCE_BUTTON_UP,	true },
			{ 0, JOY_BUTTON_31,	DANCE_BUTTON_DOWN,	true },
			{ 1, JOY_BUTTON_20,	DANCE_BUTTON_LEFT,	true },
			{ 1, JOY_BUTTON_18,	DANCE_BUTTON_RIGHT,	true },
			{ 1, JOY_BUTTON_17,	DANCE_BUTTON_UP,	true },
			{ 1, JOY_BUTTON_19,	DANCE_BUTTON_DOWN,	true },
			{ 0, JOY_BUTTON_23,	DANCE_BUTTON_UPRIGHT,	true },
			{ 0, JOY_BUTTON_24,	DANCE_BUTTON_UPLEFT,	true },
			{ 0, JOY_BUTTON_25,	GAME_BUTTON_SELECT,	true },
			{ 0, JOY_BUTTON_26,	GAME_BUTTON_START,	true },
			{ 0, JOY_BUTTON_21,	GAME_BUTTON_BACK,	true },
			{ 0, JOY_BUTTON_22,	GAME_BUTTON_COIN,	true },
			END_MARKER
		}
	},
	{
		"dance",
		"Dance ", //Notice extra space at end
		"LevelSix USB Pad (DDR638)",	// "DDR638" is the model number of the pad
		{
			{ 0, JOY_BUTTON_1,	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_2,	DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_3,	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_4,	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_7,	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_8,	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"SmartJoy PLUS Adapter",
		"SmartJoy PLUS Adapter",
		{
			{ 0, JOY_LEFT,		/* dpad L */	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_RIGHT,		/* dpad R */	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_UP,		/* dpad U */	DANCE_BUTTON_UP,	false },
			{ 0, JOY_DOWN,		/* dpad D */	DANCE_BUTTON_DOWN,	false },
			{ 1, JOY_BUTTON_4,	/* Square */	DANCE_BUTTON_LEFT,	false },
			{ 1, JOY_BUTTON_2,	/* Circle */	DANCE_BUTTON_RIGHT,	false },
			{ 1, JOY_BUTTON_1,	/* Tri */	DANCE_BUTTON_UP,	false },
			{ 1, JOY_BUTTON_3,	/* X */		DANCE_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_7,	/* L1 */	DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_8,	/* R1 */	DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_10,	/* Select */	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_9,	/* Start */	GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_5,	/* R1 */	GAME_BUTTON_SELECT,	false },
			{ 0, JOY_BUTTON_6,	/* R2 */	GAME_BUTTON_COIN,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"RedOctane USB Pad|XBOX DDR", // "RedOctane USB Pad" is Ignition 3s and newer Afterburners.  "XBOX DDR" is older Afterburners.
		"RedOctane Ignition 3 or Afterburner",
		{
			{ 0, JOY_BUTTON_1,	/* dpad L */	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_4,	/* dpad R */	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_3,	/* dpad U */	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_2,	/* dpad D */	DANCE_BUTTON_DOWN,	false },
			//{ 0, JOY_BUTTON_5,	/* Tri */	GAME_BUTTON_BACK,	false },
			//{ 0, JOY_BUTTON_6,	/* Square */	GAME_BUTTON_BACK,	false },
			//{ 1, JOY_BUTTON_7,	/* X */		GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_8,	/* O */		GAME_BUTTON_START,	false },
			{ 1, JOY_BUTTON_9,	/* Start */	GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_10,	/* Sel */	GAME_BUTTON_BACK,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"Joypad to USB converter",
		"EMS Trio Linker",
		{
			{ 0, JOY_BUTTON_16,	/* dpad L */	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_14,	/* dpad R */	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_13,	/* dpad U */	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_15,	/* dpad D */	DANCE_BUTTON_DOWN,	false },
			//{ 0, JOY_BUTTON_5,	/* Tri */	GAME_BUTTON_BACK,	false },
			//{ 0, JOY_BUTTON_6,	/* Square */	GAME_BUTTON_BACK,	false },
			//{ 1, JOY_BUTTON_7,	/* X */		GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_2,	/* O */		GAME_BUTTON_START,	false },
			{ 1, JOY_BUTTON_10,	/* Start */	GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_9,	/* Sel */	GAME_BUTTON_BACK,	false },
			END_MARKER
		}
	},
	{
		"dance",
		"Positive Gaming Impact USB pad",
		"Positive Gaming Impact USB pad",
		{
			{ 0, JOY_BUTTON_1,	/* dpad L */	DANCE_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_4,	/* dpad R */	DANCE_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_3,	/* dpad U */	DANCE_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_2,	/* dpad D */	DANCE_BUTTON_DOWN,	false },
			//{ 0, JOY_BUTTON_5,	/* Tri */	GAME_BUTTON_BACK,	false },
			//{ 0, JOY_BUTTON_6,	/* Square */	GAME_BUTTON_BACK,	false },
			//{ 1, JOY_BUTTON_7,	/* X */		GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_8,	/* O */		GAME_BUTTON_START,	false },
			{ 1, JOY_BUTTON_9,	/* Start */	GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_10,	/* Sel */	GAME_BUTTON_BACK,	false },
			END_MARKER
		}
	},
	{
		"pump",
		"Pump USB",
		"Pump USB pad",
		{
			{ 0, JOY_BUTTON_1,	PUMP_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_2,	PUMP_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_3,	PUMP_BUTTON_CENTER,	false },
			{ 0, JOY_BUTTON_4,	PUMP_BUTTON_DOWNLEFT,	false },
			{ 0, JOY_BUTTON_5,	PUMP_BUTTON_DOWNRIGHT,	false },
			{ 0, JOY_BUTTON_6,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_7,	PUMP_BUTTON_UPLEFT,	true },
			{ 0, JOY_BUTTON_8,	PUMP_BUTTON_UPRIGHT,	true },
			{ 0, JOY_BUTTON_9,	PUMP_BUTTON_CENTER,	true },
			{ 0, JOY_BUTTON_10,	PUMP_BUTTON_DOWNLEFT,	true },
			{ 0, JOY_BUTTON_11,	PUMP_BUTTON_DOWNRIGHT,	true },
			END_MARKER
		}
	},
	{
		"para",
		"ParaParaParadise Controller",
		"ParaParaParadise Controller",
		{
			{ 0, JOY_BUTTON_5,	PARA_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_4,	PARA_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_3,	PARA_BUTTON_UP,		false },
			{ 0, JOY_BUTTON_2,	PARA_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_1,	PARA_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			{ 0, JOY_BUTTON_11,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_12,	GAME_BUTTON_MENULEFT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_MENURIGHT,	false },
			END_MARKER
		}
	},
	{
		"techno",
		"Dance ", //Notice the extra space at end
		"LevelSix USB Pad (DDR638)",	// "DDR638" is the model number of the pad
		{
			{ 0, JOY_BUTTON_1,	TECHNO_BUTTON_UP,	false },
			{ 0, JOY_BUTTON_2,	TECHNO_BUTTON_DOWN,	false },
			{ 0, JOY_BUTTON_3,	TECHNO_BUTTON_LEFT,	false },
			{ 0, JOY_BUTTON_4,	TECHNO_BUTTON_RIGHT,	false },
			{ 0, JOY_BUTTON_5,	TECHNO_BUTTON_DOWNRIGHT,false },
			{ 0, JOY_BUTTON_6,	TECHNO_BUTTON_DOWNLEFT,	false },
			{ 0, JOY_BUTTON_7,	TECHNO_BUTTON_UPRIGHT,	false },
			{ 0, JOY_BUTTON_8,	TECHNO_BUTTON_UPLEFT,	false },
			{ 0, JOY_BUTTON_9,	GAME_BUTTON_BACK,	false },
			{ 0, JOY_BUTTON_10,	GAME_BUTTON_START,	false },
			END_MARKER
		}
	},
};

void InputMapper::Unmap( InputDevice id )
{
	FOREACH_GameController( i )
	{
		FOREACH_GameButton(j)
		{
			for( int k=0; k<NUM_USER_GAME_TO_DEVICE_SLOTS; k++ )
			{
				DeviceInput &di = m_GItoDI[i][j][k];
				if( di.device == id )
					di.MakeInvalid();
			}
		}
	}

	UpdateTempDItoGI();
}

void InputMapper::ApplyMapping( const Mapping *maps, GameController gc, InputDevice id )
{
	for( int k=0; !maps[k].IsEndMarker(); k++ )
	{
		GameController map_gc = gc;
		if( maps[k].SecondController )
		{
			map_gc = (GameController)(map_gc+1);

			/* If that pushed it over, then it's a second controller for a joystick
			 * that's already a second controller, so we'll just ignore it.  (This
			 * can happen if eg. two primary Pump pads are connected.) */
			if( map_gc >= NUM_GameController )
				continue;
		}

		DeviceInput di( id, maps[k].deviceButton );
		GameInput gi( map_gc, maps[k].gb );
		SetInputMap( di, gi, maps[k].iSlotIndex );
	}
}

void InputMapper::AutoMapJoysticksForCurrentGame()
{
	vector<InputDeviceInfo> vDevices;
	INPUTMAN->GetDevicesAndDescriptions(vDevices);

	int iNumJoysticksMapped = 0;

	for( unsigned i=0; i<vDevices.size(); i++ )
	{
		InputDevice id = vDevices[i].id;
		const RString &sDescription = vDevices[i].sDesc;
		for( unsigned j=0; j<ARRAYLEN(g_AutoJoyMappings); j++ )
		{
			const AutoJoyMapping& mapping = g_AutoJoyMappings[j];

			if( RString(m_pInputScheme->m_szName).CompareNoCase(mapping.szGame) )
				continue;	// games don't match

			RString sDriverRegex = mapping.szDriverRegex;
			Regex regex( sDriverRegex );
			if( !regex.Compare(sDescription) )
				continue;	// driver names don't match

			//
			// We have a mapping for this joystick
			//
			GameController gc = (GameController)iNumJoysticksMapped;
			if( gc >= NUM_GameController )
				break;	// stop mapping.  We already mapped one device for each game controller.

			LOG->Info( "Applying default joystick mapping #%d for device '%s' (%s)",
				iNumJoysticksMapped+1, mapping.szDriverRegex, mapping.szControllerName );

			Unmap( id );
			ApplyMapping( mapping.maps, gc, id );

			iNumJoysticksMapped++;
		}
	}
}

void InputMapper::SetInputScheme( const InputScheme *pInputScheme )
{
	m_pInputScheme = pInputScheme;

	ReadMappingsFromDisk();
}

const InputScheme *InputMapper::GetInputScheme() const
{
	return m_pInputScheme;
}

static const RString DEVICE_INPUT_SEPARATOR = ":";	// this isn't used in any key names

void InputMapper::ReadMappingsFromDisk()
{
	ClearAllMappings();

	IniFile ini;
	if( !ini.ReadFile( SpecialFiles::KEYMAPS_PATH ) )
		LOG->Trace( "Couldn't open mapping file \"%s\": %s.",
					SpecialFiles::KEYMAPS_PATH.c_str(), ini.GetError().c_str() );

	const XNode *Key = ini.GetChild( m_pInputScheme->m_szName );

	if( Key  )
	{
		FOREACH_CONST_Attr( Key, i )
		{
			const RString &name = i->first;
			RString value;
			i->second->GetValue( value );

			GameInput GameI;
			GameI.FromString( m_pInputScheme, name );

			vector<RString> sDeviceInputStrings;
			split( value, DEVICE_INPUT_SEPARATOR, sDeviceInputStrings, false );

			for( unsigned i=0; i<sDeviceInputStrings.size() && i<unsigned(NUM_GAME_TO_DEVICE_SLOTS); i++ )
			{
				DeviceInput DeviceI;
				DeviceI.FromString( sDeviceInputStrings[i] );
				if( DeviceI.IsValid() )
					SetInputMap( DeviceI, GameI, i );
			}
		}
	}

	AddDefaultMappingsForCurrentGameIfUnmapped();
}

void InputMapper::SaveMappingsToDisk()
{
	IniFile ini;
	ini.ReadFile( SpecialFiles::KEYMAPS_PATH );
	
	// erase the key so that we overwrite everything for this game
	ini.DeleteKey( m_pInputScheme->m_szName );

	XNode *pKey = ini.GetChild( m_pInputScheme->m_szName );
	if( pKey != NULL )
		ini.RemoveChild( pKey );
	pKey = ini.AppendChild( m_pInputScheme->m_szName );

	// iterate over our input map and write all mappings to the ini file
	FOREACH_GameController( i )
	{
		FOREACH_GameButtonInScheme( m_pInputScheme, j )
		{
			GameInput GameI( i, j );
			RString sNameString = GameI.ToString( m_pInputScheme );
			
			vector<RString> asValues;
			for( int slot = 0; slot < NUM_USER_GAME_TO_DEVICE_SLOTS; ++slot )	// don't save data from the last (keyboard automap) slot
				asValues.push_back( m_GItoDI[i][j][slot].ToString() );
		
			while( asValues.size() && asValues.back() == "" )
				asValues.erase( asValues.begin()+asValues.size()-1 );
			
			RString sValueString = join( DEVICE_INPUT_SEPARATOR, asValues );

			pKey->AppendAttr( sNameString, sValueString );
		}
	}

	ini.WriteFile( SpecialFiles::KEYMAPS_PATH );
}

static LocalizedString CONNECTED			( "InputMapper", "Connected" );
static LocalizedString DISCONNECTED			( "InputMapper", "Disconnected" );
static LocalizedString AUTOMAPPING_ALL_JOYSTICKS	( "InputMapper", "Auto-mapping all joysticks." );
bool InputMapper::CheckForChangedInputDevicesAndRemap( RString &sMessageOut )
{
	// Only check for changes in joysticks since that's all we know how to remap.

	// update last seen joysticks
	vector<InputDeviceInfo> vDevices;
	INPUTMAN->GetDevicesAndDescriptions( vDevices );

	// Strip non-joysticks.
	vector<RString> vsLastSeenJoysticks;
	split( g_sLastSeenInputDevices, ",", vsLastSeenJoysticks );

	vector<RString> vsCurrent;
	vector<RString> vsCurrentJoysticks;
	for( int i=vDevices.size()-1; i>=0; i-- )
	{
		vsCurrent.push_back( vDevices[i].sDesc );
		if( IsJoystick(vDevices[i].id) )
		{
			vsCurrentJoysticks.push_back( vDevices[i].sDesc );
		}
		else
		{
			vector<RString>::iterator iter = find( vsLastSeenJoysticks.begin(), vsLastSeenJoysticks.end(), vDevices[i].sDesc );
			if( iter != vsLastSeenJoysticks.end() )
				vsLastSeenJoysticks.erase( iter );
		}
	}


	bool bJoysticksChanged = vsCurrentJoysticks != vsLastSeenJoysticks;
	if( !bJoysticksChanged )
		return false;

	vector<RString> vsConnects, vsDisconnects;
	GetConnectsDisconnects( vsLastSeenJoysticks, vsCurrentJoysticks, vsDisconnects, vsConnects );

	sMessageOut = RString();
	if( !vsConnects.empty() )
		sMessageOut += CONNECTED.GetValue()+": " + join( "\n", vsConnects ) + "\n";
	if( !vsDisconnects.empty() )
		sMessageOut += DISCONNECTED.GetValue()+": " + join( "\n", vsDisconnects ) + "\n";

	if( g_bAutoMapOnJoyChange )
	{
		sMessageOut += AUTOMAPPING_ALL_JOYSTICKS.GetValue();
		AutoMapJoysticksForCurrentGame();
		SaveMappingsToDisk();
		MESSAGEMAN->Broadcast( Message_AutoJoyMappingApplied );
	}

	LOG->Info( sMessageOut );

	g_sLastSeenInputDevices.Set( join(",",vsCurrent) );
	PREFSMAN->SavePrefsToDisk();

	return true;
}

void InputMapper::SetInputMap( const DeviceInput &DeviceI, const GameInput &GameI, int iSlotIndex )
{
	// remove the old input
	ClearFromInputMap( DeviceI );
	ClearFromInputMap( GameI, iSlotIndex );
	
	ASSERT_M( GameI.controller < NUM_GameController,
		  ssprintf("controller: %u >= %u", GameI.controller, NUM_GameController) );
	ASSERT_M( GameI.button < NUM_GameButton,
		  ssprintf("button: %u >= %u", GameI.button, NUM_GameButton) );
	ASSERT_M( iSlotIndex < NUM_GAME_TO_DEVICE_SLOTS,
		  ssprintf("slot: %u >= %u", iSlotIndex, NUM_GAME_TO_DEVICE_SLOTS) );
	m_GItoDI[GameI.controller][GameI.button][iSlotIndex] = DeviceI;


	UpdateTempDItoGI();
}

void InputMapper::ClearFromInputMap( const DeviceInput &DeviceI )
{
	// search for where this DeviceI maps to

	FOREACH_GameController( p )
	{
		FOREACH_GameButton(b)
		{
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ )
			{
				if( m_GItoDI[p][b][s] == DeviceI )
					m_GItoDI[p][b][s].MakeInvalid();
			}
		}
	}
	
	UpdateTempDItoGI();
}

bool InputMapper::ClearFromInputMap( const GameInput &GameI, int iSlotIndex )
{
	if( !GameI.IsValid() )
		return false;

	DeviceInput &di = m_GItoDI[GameI.controller][GameI.button][iSlotIndex];
	if( !di.IsValid() )
		return false;
	di.MakeInvalid();

	UpdateTempDItoGI();
	return true;
}

bool InputMapper::IsMapped( const DeviceInput &DeviceI )
{
	return g_tempDItoGI.find(DeviceI) != g_tempDItoGI.end();
}

void InputMapper::UpdateTempDItoGI()
{
	// repopulate g_tempDItoGI
	g_tempDItoGI.clear();
	FOREACH_GameController( n )
	{
		FOREACH_GameButton( b )
		{
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ )
			{
				const DeviceInput &DeviceI = m_GItoDI[n][b][s];
				if( !DeviceI.IsValid() )
					continue;

				g_tempDItoGI[DeviceI] = GameInput( n, b );
			}
		}
	}
}

bool InputMapper::DeviceToGame( const DeviceInput &DeviceI, GameInput& GameI ) // return true if there is a mapping from device to pad
{
	GameI = g_tempDItoGI[DeviceI];
	return GameI.controller != GameController_Invalid;
}

bool InputMapper::GameToDevice( const GameInput &GameI, int iSlotNum, DeviceInput& DeviceI )	// return true if there is a mapping from pad to device
{
	DeviceI = m_GItoDI[GameI.controller][GameI.button][iSlotNum];
	return DeviceI.device != InputDevice_Invalid;
}

PlayerNumber InputMapper::ControllerToPlayerNumber( GameController controller )
{
	if( g_JoinControllers != PLAYER_INVALID )
		return g_JoinControllers;
	else
		return (PlayerNumber) controller;
}

MenuButton InputMapper::GameToMenu( const GameInput &GameI )
{
	return m_pInputScheme->GameInputToMenuButton( GameI );
}

/* If set (not PLAYER_INVALID), inputs from both GameControllers will be mapped
 * to the specified player.  If PLAYER_INVALID, GameControllers will be mapped
 * individually. */
void InputMapper::SetJoinControllers( PlayerNumber pn )
{
	g_JoinControllers = pn;
}


void InputMapper::MenuToGame( MenuButton MenuI, PlayerNumber pn, GameInput GameIout[4] )
{
	if( g_JoinControllers != PLAYER_INVALID )
		pn = PLAYER_INVALID;

	m_pInputScheme->MenuButtonToGameInputs( MenuI, pn, GameIout );
}


bool InputMapper::IsBeingPressed( const GameInput &GameI, MultiPlayer mp, const DeviceInputList *pButtonState )
{
	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;

		if( GameToDevice( GameI, i, DeviceI ) )
		{
			if( mp != MultiPlayer_Invalid )
				DeviceI.device = MultiPlayerToInputDevice(mp);
			if( INPUTFILTER->IsBeingPressed(DeviceI, pButtonState) )
				return true;
		}
	}

	return false;
}

bool InputMapper::IsBeingPressed( MenuButton MenuI, PlayerNumber pn )
{
	GameInput GameI[4];
	MenuToGame( MenuI, pn, GameI );
	for( int i=0; i<4; i++ )
		if( GameI[i].IsValid()  &&  IsBeingPressed(GameI[i]) )
			return true;

	return false;
}

void InputMapper::RepeatStopKey( const GameInput &GameI )
{
	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;

		if( GameToDevice( GameI, i, DeviceI ) )
			INPUTFILTER->RepeatStopKey( DeviceI );
	}
}

void InputMapper::RepeatStopKey( MenuButton MenuI, PlayerNumber pn )
{
	GameInput GameI[4];
	MenuToGame( MenuI, pn, GameI );
	for( int i=0; i<4; i++ )
		if( GameI[i].IsValid() )
			RepeatStopKey( GameI[i] );
}

float InputMapper::GetSecsHeld( const GameInput &GameI, MultiPlayer mp )
{
	float fMaxSecsHeld = 0;

	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;
		if( GameToDevice( GameI, i, DeviceI ) )
		{
			if( mp != MultiPlayer_Invalid )
				DeviceI.device = MultiPlayerToInputDevice(mp);
			fMaxSecsHeld = max( fMaxSecsHeld, INPUTFILTER->GetSecsHeld(DeviceI) );
		}
	}

	return fMaxSecsHeld;
}

float InputMapper::GetSecsHeld( MenuButton MenuI, PlayerNumber pn )
{
	float fMaxSecsHeld = 0;

	GameInput GameI[4];
	MenuToGame( MenuI, pn, GameI );
	for( int i=0; i<4; i++ )
		if( GameI[i].IsValid() )
			fMaxSecsHeld = max( fMaxSecsHeld, GetSecsHeld(GameI[i]) );

	return fMaxSecsHeld;
}

void InputMapper::ResetKeyRepeat( const GameInput &GameI )
{
	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;
		if( GameToDevice( GameI, i, DeviceI ) )
			INPUTFILTER->ResetKeyRepeat( DeviceI );
	}
}

void InputMapper::ResetKeyRepeat( MenuButton MenuI, PlayerNumber pn )
{
	GameInput GameI[4];
	MenuToGame( MenuI, pn, GameI );
	for( int i=0; i<4; i++ )
		if( GameI[i].IsValid() )
			ResetKeyRepeat( GameI[i] );
}

InputDevice InputMapper::MultiPlayerToInputDevice( MultiPlayer mp )
{
	if( mp == MultiPlayer_Invalid )
		return InputDevice_Invalid;
	return enum_add2( DEVICE_JOY1, mp );
}

MultiPlayer InputMapper::InputDeviceToMultiPlayer( InputDevice id )
{
	if( id == InputDevice_Invalid )
		return MultiPlayer_Invalid;
	return enum_add2( MultiPlayer_1, id - DEVICE_JOY1 );
}

GameButton InputScheme::ButtonNameToIndex( const RString &sButtonName ) const
{
	for( int i=0; i<m_iButtonsPerController; i++ ) 
		if( stricmp(m_GameButtonInfo[i].m_szName, sButtonName) == 0 )
			return i;

	return GameButton_Invalid;
}

MenuButton InputScheme::GameInputToMenuButton( GameInput GameI ) const
{
	FOREACH_MenuButton(i)
		if( g_DedicatedMenuButtons[i] == GameI.button )
			return i;

	if( !PREFSMAN->m_bOnlyDedicatedMenuButtons )
	{
		FOREACH_MenuButton(i)
			if( m_SecondaryMenuButton[i] == GameI.button )
				return i;
	}

	return MenuButton_Invalid;	// invalid GameInput
}

void InputScheme::MenuButtonToGameInputs( MenuButton MenuI, PlayerNumber pn, GameInput GameIout[4] ) const
{
	ASSERT( MenuI != MenuButton_Invalid );

	GameIout[0].MakeInvalid();	// initialize
	GameIout[1].MakeInvalid();	
	GameIout[2].MakeInvalid();	
	GameIout[3].MakeInvalid();	

	vector<GameController> controller;
	if( pn == PLAYER_INVALID )
	{
		controller.push_back( GAME_CONTROLLER_1 );
		controller.push_back( GAME_CONTROLLER_2 );
	}
	else
	{
		controller.push_back( (GameController)pn );
	}

	GameButton button[2] = { g_DedicatedMenuButtons[MenuI], m_SecondaryMenuButton[MenuI] };
	int iNumButtonsUsing = PREFSMAN->m_bOnlyDedicatedMenuButtons ? 1 : 2;

	int iOut = 0;
	for( size_t i=0; i<controller.size(); i++ )
	{
		for( int j=0; j<iNumButtonsUsing; j++ )
		{
			if( button[j] == GameButton_Invalid )
				continue;
			GameIout[iOut].controller = controller[i];
			GameIout[iOut].button = button[j];
			++iOut;
		}
	}
}

MenuButton InputScheme::GetMenuButtonSecondaryFunction( GameButton gb ) const
{
	FOREACH_MenuButton(mb)
		if( m_SecondaryMenuButton[mb] == gb )
			return mb;

	return MenuButton_Invalid;
}

/*
 * (c) 2001-2003 Chris Danford
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
