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


InputMapper*	INPUTMAPPER = NULL;	// global and accessable from anywhere in our program


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
	const char *sDeviceDescription;
	bool bIgnoreAxes;
	int numMappings;
	struct {
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
	} mapping[32];
};
const AutoJoyMapping g_AutoJoyMappings[] = 
{
	{
		GAME_DANCE,
		"GIC USB Joystick",
		false,
		4,
		{
			{ JOY_16, DANCE_BUTTON_LEFT },
			{ JOY_14, DANCE_BUTTON_RIGHT },
			{ JOY_13, DANCE_BUTTON_UP },
			{ JOY_15, DANCE_BUTTON_DOWN },
		}
	},
	{
		GAME_DANCE,
		"4 axis 16 button joystick",	// likely a PC Magic Box
		false,
		4,
		{
			{ JOY_16, DANCE_BUTTON_LEFT },
			{ JOY_14, DANCE_BUTTON_RIGHT },
			{ JOY_13, DANCE_BUTTON_UP },
			{ JOY_15, DANCE_BUTTON_DOWN },
		}
	},
	{
		GAME_DANCE,
		"GamePad Pro USB ",	// yes, there is a space at the end
		false,
		12,
		{
			{ JOY_LEFT,		DANCE_BUTTON_LEFT },
			{ JOY_RIGHT,	DANCE_BUTTON_RIGHT },
			{ JOY_UP,		DANCE_BUTTON_UP },
			{ JOY_DOWN,		DANCE_BUTTON_DOWN },
			{ JOY_1,		DANCE_BUTTON_LEFT },
			{ JOY_3,		DANCE_BUTTON_RIGHT },
			{ JOY_4,		DANCE_BUTTON_UP },
			{ JOY_2,		DANCE_BUTTON_DOWN },
			{ JOY_5,		DANCE_BUTTON_UPLEFT },
			{ JOY_6,		DANCE_BUTTON_UPRIGHT },
			{ JOY_9,		DANCE_BUTTON_BACK },
			{ JOY_10,		DANCE_BUTTON_START },
		}
	},
	{
		GAME_PUMP,
		"Pump USB",
		false,
		11,
		{
			{ PUMP_UL,		PUMP_BUTTON_UPLEFT },
			{ PUMP_UR,		PUMP_BUTTON_UPRIGHT },
			{ PUMP_MID,		PUMP_BUTTON_CENTER },
			{ PUMP_DL,		PUMP_BUTTON_DOWNLEFT },
			{ PUMP_DR,		PUMP_BUTTON_DOWNRIGHT },
			{ PUMP_ESCAPE,	PUMP_BUTTON_BACK },
			{ PUMP_2P_UL,	PUMP_BUTTON_UPLEFT,		true },
			{ PUMP_2P_UR,	PUMP_BUTTON_UPRIGHT,	true },
			{ PUMP_2P_MID,	PUMP_BUTTON_CENTER,		true },
			{ PUMP_2P_DL,	PUMP_BUTTON_DOWNLEFT,	true },
			{ PUMP_2P_DR,	PUMP_BUTTON_DOWNRIGHT,	true },
		}
	},
};
const int NUM_AUTO_JOY_MAPPINGS = sizeof(g_AutoJoyMappings) / sizeof(g_AutoJoyMappings[0]);

void InputMapper::AutoMapJoysticksForCurrentGame()
{
	vector<InputDevice> vDevices;
	vector<CString> vDescriptions;
	PREFSMAN->m_bIgnoreJoyAxes = false;
	INPUTMAN->GetDevicesAndDescriptions(vDevices,vDescriptions);
	for( unsigned i=0; i<vDevices.size(); i++ )
	{
		InputDevice device = vDevices[i];
		CString sDescription = vDescriptions[i];
		for( unsigned j=0; j<NUM_AUTO_JOY_MAPPINGS; j++ )
		{
			const AutoJoyMapping& mapping = g_AutoJoyMappings[j];

			if( sDescription == mapping.sDeviceDescription )
			{
				PREFSMAN->m_bIgnoreJoyAxes |= mapping.bIgnoreAxes;

				GameController gc = GAME_CONTROLLER_INVALID;
				switch( device )
				{
				case DEVICE_JOY1:
				case DEVICE_JOY3:
				case DEVICE_PUMP1:
					gc = GAME_CONTROLLER_1;	
					break;
				case DEVICE_JOY2:
				case DEVICE_JOY4:
				case DEVICE_PUMP2:
					gc = GAME_CONTROLLER_2;	
					break;
				}
				if( gc == GAME_CONTROLLER_INVALID )
					continue;

				for( int k=0; k<mapping.numMappings; k++ )
				{
					if( mapping.mapping[k].SecondController && gc == GAME_CONTROLLER_1 )
						gc = GAME_CONTROLLER_2;

					DeviceInput di( device, mapping.mapping[k].deviceButton );
					GameInput gi( gc, mapping.mapping[k].gb );
					SetInputMap( di, gi, 1 );
				}
				break;
			}
		}
	}
}

void InputMapper::ReadMappingsFromDisk()
{
	ASSERT( GAMEMAN != NULL );

	ClearAllMappings();

	IniFile ini;
	ini.SetPath( "Data/Keymaps.ini" );
	if( !ini.ReadFile() )
		LOG->Warn( "could not input mapping file \"Keymaps.ini\"." );

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
	ini.SetPath( "Data/Keymaps.ini" );
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
