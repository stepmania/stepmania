#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: InputMapper

 Desc: See Header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "InputMapper.h"
#include "IniFile.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageLog.h"
#include "InputFilter.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageInput.h"
#include "arch/arch.h"


InputMapper*	INPUTMAPPER = NULL;	// global and accessable from anywhere in our program

#define KEYMAPS_PATH BASE_PATH "Data" SLASH "Keymaps.ini"

InputMapper::InputMapper()
{
}


InputMapper::~InputMapper()
{
	SaveMappingsToDisk();
}

void InputMapper::ClearAllMappings()
{
	for( int i=0; i<MAX_GAME_CONTROLLERS; i++ )
		for( int j=0; j<MAX_GAME_BUTTONS; j++ )
			for( int k=0; k<NUM_GAME_TO_DEVICE_SLOTS; k++ )
				m_GItoDI[i][j][k].MakeInvalid();
}

void InputMapper::AddDefaultMappingsForCurrentGameIfUnmapped()
{
	// Clear default mappings.  Default mappings are in the third slot.
	for( int i=0; i<MAX_GAME_CONTROLLERS; i++ )
		for( int j=0; j<MAX_GAME_BUTTONS; j++ )
			ClearFromInputMap( GameInput((GameController)i,(GameButton)j), 2 );

	GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	for( int c=0; c<MAX_GAME_CONTROLLERS; c++ )
	{
		for( int b=0; b<pGameDef->m_iButtonsPerController; b++ )
		{
			int key = pGameDef->m_iDefaultKeyboardKey[c][b];
			if( key == -1 )	// "no key" marker"
				continue;
			DeviceInput DeviceI( DEVICE_KEYBOARD, key );
			GameInput GameI( (GameController)c, (GameButton)b );
			if( !IsMapped(DeviceI) )	// if this key isn't already being used by another user-made mapping
				SetInputMap( DeviceI, GameI, 2 );   
		}
	}
}

struct AutoJoyMapping
{
	Game game;
	const char *szDeviceDescription;	// reported by InputHandler
	const char *szControllerName;	// the product name of the controller
	bool bIgnoreAxes;
	struct {
		int iSlotIndex;	// -1 == end marker
		int deviceButton;
		GameButton gb;
		/* If this is true, this is an auxilliary mapping assigned to the second
		 * player.  If two of the same device are found, and the device has secondary
		 * entries, the later entries take precedence.  This way, if a Pump pad is
		 * found, it'll map P1 to the primary pad and P2 to the secondary pad.
		 * (We can't tell if a slave pad is actually there.)  Then, if a second primary
		 * is found (DEVICE_PUMP2), 2P will be mapped to it. 
		 *
		 * This isn't well-tested; I only have one Pump pad. */
		bool SecondController;
	} map[32];
};
const AutoJoyMapping g_AutoJoyMappings[] = 
{
	{
		GAME_DANCE,
		"GIC USB Joystick",
		"Boom USB convertor (black/gray)",
		false,
		{
			{ 0, JOY_16, DANCE_BUTTON_LEFT },
			{ 0, JOY_14, DANCE_BUTTON_RIGHT },
			{ 0, JOY_13, DANCE_BUTTON_UP },
			{ 0, JOY_15, DANCE_BUTTON_DOWN },
			{-1, NULL,		NULL },
		}
	},
	{
		GAME_DANCE,
		"4 axis 16 button joystick",
		"PC Magic Box",
		false,
		{
			{ 0, JOY_16, DANCE_BUTTON_LEFT },
			{ 0, JOY_14, DANCE_BUTTON_RIGHT },
			{ 0, JOY_13, DANCE_BUTTON_UP },
			{ 0, JOY_15, DANCE_BUTTON_DOWN },
			{-1, NULL,		NULL },
		}
	},
	{
		GAME_DANCE,
		"GamePad Pro USB ",	// yes, there is a space at the end
		"GamePad Pro USB",
		false,
		{
			{ 0, JOY_LEFT,	DANCE_BUTTON_LEFT },
			{ 0, JOY_RIGHT,	DANCE_BUTTON_RIGHT },
			{ 0, JOY_UP,	DANCE_BUTTON_UP },
			{ 0, JOY_DOWN,	DANCE_BUTTON_DOWN },
			{ 1, JOY_1,		DANCE_BUTTON_LEFT },
			{ 1, JOY_3,		DANCE_BUTTON_RIGHT },
			{ 1, JOY_4,		DANCE_BUTTON_UP },
			{ 1, JOY_2,		DANCE_BUTTON_DOWN },
			{ 0, JOY_5,		DANCE_BUTTON_UPLEFT },
			{ 0, JOY_6,		DANCE_BUTTON_UPRIGHT },
			{ 0, JOY_9,		DANCE_BUTTON_BACK },
			{ 0, JOY_10,	DANCE_BUTTON_START },
			{-1, NULL,		NULL },
		}
	},
	{
		GAME_DANCE,
		"4 axis 12 button joystick with hat switch",
		"Super Joy Box 5",
		false,
		{
			{ 0, JOY_HAT_LEFT,	DANCE_BUTTON_LEFT },
			{ 0, JOY_HAT_RIGHT,	DANCE_BUTTON_RIGHT },
			{ 0, JOY_HAT_UP,	DANCE_BUTTON_UP },
			{ 0, JOY_HAT_DOWN,	DANCE_BUTTON_DOWN },
			{ 1, JOY_4,			DANCE_BUTTON_LEFT },
			{ 1, JOY_2,			DANCE_BUTTON_RIGHT },
			{ 1, JOY_1,			DANCE_BUTTON_UP },
			{ 1, JOY_3,			DANCE_BUTTON_DOWN },
			{ 0, JOY_5,			DANCE_BUTTON_UPLEFT },
			{ 0, JOY_6,			DANCE_BUTTON_UPRIGHT },
			{ 1, JOY_7,			DANCE_BUTTON_UPLEFT },
			{ 1, JOY_8,			DANCE_BUTTON_UPRIGHT },
			{ 0, JOY_10,		DANCE_BUTTON_BACK },
			{ 0, JOY_9,			DANCE_BUTTON_START },
			{-1, NULL,			NULL },
		}
	},
	{
		GAME_DANCE,
		"MP-8866 Dual USB Joypad",
		"Super Dual Box",
		false,
		{
			{ 0, JOY_HAT_LEFT,	DANCE_BUTTON_LEFT },
			{ 0, JOY_HAT_RIGHT,	DANCE_BUTTON_RIGHT },
			{ 0, JOY_HAT_UP,	DANCE_BUTTON_UP },
			{ 0, JOY_HAT_DOWN,	DANCE_BUTTON_DOWN },
			{ 1, JOY_4,			DANCE_BUTTON_LEFT },
			{ 1, JOY_2,			DANCE_BUTTON_RIGHT },
			{ 1, JOY_1,			DANCE_BUTTON_UP },
			{ 1, JOY_3,			DANCE_BUTTON_DOWN },
			{ 0, JOY_5,			DANCE_BUTTON_UPLEFT },
			{ 0, JOY_6,			DANCE_BUTTON_UPRIGHT },
			{ 1, JOY_7,			DANCE_BUTTON_UPLEFT },
			{ 1, JOY_8,			DANCE_BUTTON_UPRIGHT },
			{ 0, JOY_10,		DANCE_BUTTON_BACK },
			{ 0, JOY_9,			DANCE_BUTTON_START },
			{-1, NULL,			NULL },
		}
	},
	{
		GAME_DANCE,
		"XBOX Gamepad Plugin V0.01",
		"X-Box gamepad",
		false,
		{
			{ 0, JOY_HAT_LEFT,	DANCE_BUTTON_LEFT },
			{ 0, JOY_HAT_RIGHT,	DANCE_BUTTON_RIGHT },
			{ 0, JOY_HAT_UP,	DANCE_BUTTON_UP },
			{ 0, JOY_HAT_DOWN,	DANCE_BUTTON_DOWN },
			{ 1, JOY_1,			DANCE_BUTTON_DOWN },	// A
			{ 1, JOY_2,			DANCE_BUTTON_RIGHT },	// B
			{ 1, JOY_3,			DANCE_BUTTON_LEFT },	// X
			{ 1, JOY_4,			DANCE_BUTTON_UP },		// Y
			{ 0, JOY_7,			DANCE_BUTTON_UPLEFT },	// L shoulder
			{ 0, JOY_8,			DANCE_BUTTON_UPRIGHT },	// R shoulder
			{ 0, JOY_9,			DANCE_BUTTON_START },
			{ 0, JOY_10,		DANCE_BUTTON_BACK },
			{-1, NULL,			NULL },
		}
	},
	{
		GAME_PUMP,
		"Pump USB",
		"Pump USB pad",
		false,
		{
			{ 0, PUMP_UL,		PUMP_BUTTON_UPLEFT },
			{ 0, PUMP_UR,		PUMP_BUTTON_UPRIGHT },
			{ 0, PUMP_MID,		PUMP_BUTTON_CENTER },
			{ 0, PUMP_DL,		PUMP_BUTTON_DOWNLEFT },
			{ 0, PUMP_DR,		PUMP_BUTTON_DOWNRIGHT },
			{ 0, PUMP_ESCAPE,	PUMP_BUTTON_BACK },
			{ 0, PUMP_2P_UL,	PUMP_BUTTON_UPLEFT,		true },
			{ 0, PUMP_2P_UR,	PUMP_BUTTON_UPRIGHT,	true },
			{ 0, PUMP_2P_MID,	PUMP_BUTTON_CENTER,		true },
			{ 0, PUMP_2P_DL,	PUMP_BUTTON_DOWNLEFT,	true },
			{ 0, PUMP_2P_DR,	PUMP_BUTTON_DOWNRIGHT,	true },
			{-1, NULL,			NULL },
		}
	},
};

void InputMapper::AutoMapJoysticksForCurrentGame()
{
	vector<InputDevice> vDevices;
	vector<CString> vDescriptions;
	PREFSMAN->m_bIgnoreJoyAxes = false;
	INPUTMAN->GetDevicesAndDescriptions(vDevices,vDescriptions);

	int iNumJoysticksMapped = 0;

	for( unsigned i=0; i<vDevices.size(); i++ )
	{
		InputDevice device = vDevices[i];
		CString sDescription = vDescriptions[i];
		for( int j=0; j<ARRAYSIZE(g_AutoJoyMappings); j++ )
		{
			const AutoJoyMapping& mapping = g_AutoJoyMappings[j];

			if( sDescription == mapping.szDeviceDescription )
			{
				//
				// We have a mapping for this joystick
				//
				GameController gc = (GameController)iNumJoysticksMapped;
				if( gc >= GAME_CONTROLLER_INVALID )
					break;	// stop mapping.  We already mapped one device for each game controller.

				LOG->Info( "Applying default joystick mapping #%d for device '%s' (%s)",
					iNumJoysticksMapped+1, mapping.szDeviceDescription, mapping.szControllerName );

				PREFSMAN->m_bIgnoreJoyAxes |= mapping.bIgnoreAxes;

				for( int k=0; mapping.map[k].iSlotIndex != -1; k++ )
				{
					if( mapping.map[k].SecondController )
						gc = GAME_CONTROLLER_2;

					DeviceInput di( device, mapping.map[k].deviceButton );
					GameInput gi( gc, mapping.map[k].gb );
					SetInputMap( di, gi, mapping.map[k].iSlotIndex );
				}

				iNumJoysticksMapped++;
			}
		}
	}
}

void InputMapper::ReadMappingsFromDisk()
{
	ASSERT( GAMEMAN != NULL );

	ClearAllMappings();

	IniFile ini;
	ini.SetPath( KEYMAPS_PATH );
	if( !ini.ReadFile() )
		LOG->Warn( "could not input mapping file '%s'.", KEYMAPS_PATH );

	const IniFile::key *Key = ini.GetKey( GAMESTATE->GetCurrentGameDef()->m_szName );

	if( Key  )
	{
		for( IniFile::key::const_iterator i = Key->begin(); 
			i != Key->end(); ++i )
		{
			CString name = i->first;
			CString value = i->second;

			GameInput GameI;
			GameI.fromString( name );

			CStringArray sDeviceInputStrings;
			split( value, ",", sDeviceInputStrings, false );

			for( unsigned i=0; i<sDeviceInputStrings.size() && i<unsigned(NUM_GAME_TO_DEVICE_SLOTS); i++ )
			{
				DeviceInput DeviceI;
				DeviceI.fromString( sDeviceInputStrings[i] );
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
	ini.SetPath( KEYMAPS_PATH );
	ini.ReadFile();
	
	// erase the key so that we overwrite everything for this game
	ini.DeleteKey( GAMESTATE->GetCurrentGameDef()->m_szName );

	// iterate over our input map and write all mappings to the ini file
	for( int i=0; i<MAX_GAME_CONTROLLERS; i++ )
	{
		for( int j=0; j<MAX_GAME_BUTTONS; j++ )
		{
			CString sNameString, sValueString;
			
			GameInput GameI( (GameController)i, (GameButton)j );
			sNameString = GameI.toString();
			sValueString = ssprintf( "%s,%s,%s", 
				m_GItoDI[i][j][0].toString().c_str(), m_GItoDI[i][j][1].toString().c_str(), m_GItoDI[i][j][2].toString().c_str() );
			
			ini.SetValue( GAMESTATE->GetCurrentGameDef()->m_szName, sNameString, sValueString );
		}
	}

	ini.WriteFile();
}


void InputMapper::SetInputMap( DeviceInput DeviceI, GameInput GameI, int iSlotIndex )
{
	// remove the old input
	ClearFromInputMap( DeviceI );
	ClearFromInputMap( GameI, iSlotIndex );

	m_GItoDI[GameI.controller][GameI.button][iSlotIndex] = DeviceI;


	UpdateTempDItoGI();
}

void InputMapper::ClearFromInputMap( DeviceInput DeviceI )
{
	// search for where this DeviceI maps to

	for( int p=0; p<MAX_GAME_CONTROLLERS; p++ )
	{
		for( int b=0; b<MAX_GAME_BUTTONS; b++ )
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

void InputMapper::ClearFromInputMap( GameInput GameI, int iSlotIndex )
{
	if( !GameI.IsValid() )
		return;

	m_GItoDI[GameI.controller][GameI.button][iSlotIndex].MakeInvalid();

	UpdateTempDItoGI();
}

bool InputMapper::IsMapped( DeviceInput DeviceI )
{
	return m_tempDItoGI[DeviceI.device][DeviceI.button].IsValid();
}

bool InputMapper::IsMapped( GameInput GameI )
{
	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
		if( m_GItoDI[GameI.controller][GameI.button][i].IsValid() )
			return true;

	return false;
}


void InputMapper::UpdateTempDItoGI()
{
	// clear out m_tempDItoGI
	for( int d=0; d<NUM_INPUT_DEVICES; d++ )
	{
		for( int b=0; b<NUM_DEVICE_BUTTONS; b++ )
		{
			m_tempDItoGI[d][b].MakeInvalid();
		}
	}


	// repopulate m_tempDItoGI
	for( int n=0; n<MAX_GAME_CONTROLLERS; n++ )
	{
		for( int b=0; b<MAX_GAME_BUTTONS; b++ )
		{
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ )
			{
				GameInput GameI( (GameController)n, (GameButton)b );
				DeviceInput DeviceI = m_GItoDI[n][b][s];

				if( DeviceI.IsValid() )
					m_tempDItoGI[DeviceI.device][DeviceI.button] = GameI;
			}
		}
	}
}

bool InputMapper::DeviceToGame( DeviceInput DeviceI, GameInput& GameI ) // return true if there is a mapping from device to pad
{
	GameI = m_tempDItoGI[DeviceI.device][DeviceI.button];
	return GameI.controller != GAME_CONTROLLER_INVALID;
}

bool InputMapper::GameToDevice( GameInput GameI, int iSoltNum, DeviceInput& DeviceI )	// return true if there is a mapping from pad to device
{
	DeviceI = m_GItoDI[GameI.controller][GameI.button][iSoltNum];
	return DeviceI.device != DEVICE_NONE;
}

void InputMapper::GameToStyle( GameInput GameI, StyleInput &StyleI )
{
	if( GAMESTATE->m_CurStyle == STYLE_INVALID )
	{
		StyleI.MakeInvalid();
		return;
	}

	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	StyleI = pStyleDef->GameInputToStyleInput( GameI );
}

void InputMapper::GameToMenu( GameInput GameI, MenuInput &MenuI )
{
	const GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	MenuI = pGameDef->GameInputToMenuInput( GameI );
}

void InputMapper::StyleToGame( StyleInput StyleI, GameInput &GameI )
{
	const StyleDef* pStyleDef = GAMESTATE->GetCurrentStyleDef();
	GameI = pStyleDef->StyleInputToGameInput( StyleI );
}


void InputMapper::MenuToGame( MenuInput MenuI, GameInput GameIout[4] )
{
	GameDef* pGameDef = GAMESTATE->GetCurrentGameDef();
	pGameDef->MenuInputToGameInput( MenuI, GameIout );
}


bool InputMapper::IsButtonDown( GameInput GameI )
{
	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;

		if( GameToDevice( GameI, i, DeviceI ) )
			if( INPUTFILTER->IsBeingPressed( DeviceI ) )
				return true;
	}

	return false;
}

bool InputMapper::IsButtonDown( MenuInput MenuI )
{
	GameInput GameI[4];
	MenuToGame( MenuI, GameI );
	for( int i=0; i<4; i++ )
		if( GameI[i].IsValid()  &&  IsButtonDown(GameI[i]) )
			return true;

	return false;
}

bool InputMapper::IsButtonDown( StyleInput StyleI )
{
	GameInput GameI;
	StyleToGame( StyleI, GameI );
	return IsButtonDown( GameI );
}


float InputMapper::GetSecsHeld( GameInput GameI )
{
	float fMaxSecsHeld = 0;

	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;

		if( GameToDevice( GameI, i, DeviceI ) )
			fMaxSecsHeld = max( fMaxSecsHeld, INPUTFILTER->GetSecsHeld(DeviceI) );
	}

	return fMaxSecsHeld;
}

float InputMapper::GetSecsHeld( MenuInput MenuI )
{
	float fMaxSecsHeld = 0;

	GameInput GameI[4];
	MenuToGame( MenuI, GameI );
	for( int i=0; i<4; i++ )
		if( GameI[i].IsValid() )
			fMaxSecsHeld = max( fMaxSecsHeld, GetSecsHeld(GameI[i]) );

	return fMaxSecsHeld;
}

float InputMapper::GetSecsHeld( StyleInput StyleI )
{
	GameInput GameI;
	StyleToGame( StyleI, GameI );
	return GetSecsHeld( GameI );
}
