#include "global.h"
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
#include "Game.h"
#include "Style.h"


InputMapper*	INPUTMAPPER = NULL;	// global and accessable from anywhere in our program

#define KEYMAPS_PATH "Data/Keymaps.ini"

InputMapper::InputMapper()
{
	ReadMappingsFromDisk();
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

	const Game* pGame = GAMESTATE->GetCurrentGame();
	for( int c=0; c<MAX_GAME_CONTROLLERS; c++ )
	{
		for( int b=0; b<pGame->m_iButtonsPerController; b++ )
		{
			int key = pGame->m_iDefaultKeyboardKey[c][b];
			if( key == NO_DEFAULT_KEY )
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
	const char *szGame;
	const char *szDriverRegex;	// reported by InputHandler
	const char *szControllerName;	// the product name of the controller
	struct InputMapper::Mapping maps[32];
};
#define END_MARKER	{-1, -1, -1, false },	// end marker
const AutoJoyMapping g_AutoJoyMappings[] = 
{
	{
		"dance",
		"GIC USB Joystick",
		"Boom USB convertor (black/gray)",
		{
			{ 0, JOY_16,		DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_14,		DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_13,		DANCE_BUTTON_UP,		false },
			{ 0, JOY_15,		DANCE_BUTTON_DOWN,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"4 axis 16 button joystick",
		"PC Magic Box",
		{
			{ 0, JOY_16,		DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_14,		DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_13,		DANCE_BUTTON_UP,		false },
			{ 0, JOY_15,		DANCE_BUTTON_DOWN,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"GamePad Pro USB ",	// yes, there is a space at the end
		"GamePad Pro USB",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,		false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,		false },
			{ 1, JOY_1,			DANCE_BUTTON_LEFT,		false },
			{ 1, JOY_3,			DANCE_BUTTON_RIGHT,		false },
			{ 1, JOY_4,			DANCE_BUTTON_UP,		false },
			{ 1, JOY_2,			DANCE_BUTTON_DOWN,		false },
			{ 0, JOY_5,			DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_6,			DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_9,			DANCE_BUTTON_BACK,		false },
			{ 0, JOY_10,		DANCE_BUTTON_START,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"SideWinder Game Pad USB version 1.0",
		"SideWinder Game Pad USB",
		{
			{ 0, JOY_LEFT,		DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_RIGHT,		DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_UP,		DANCE_BUTTON_UP,		false },
			{ 0, JOY_DOWN,		DANCE_BUTTON_DOWN,		false },
			{ 1, JOY_4,			DANCE_BUTTON_LEFT,		false },
			{ 1, JOY_2,			DANCE_BUTTON_RIGHT,		false },
			{ 1, JOY_5,			DANCE_BUTTON_UP,		false },
			{ 1, JOY_1,			DANCE_BUTTON_DOWN,		false },
			{ 0, JOY_7,			DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_8,			DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_9,			DANCE_BUTTON_BACK,		false },
			{ 0, JOY_10,		DANCE_BUTTON_START,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"4 axis 12 button joystick with hat switch",
		"Super Joy Box 5",
		{
			{ 0, JOY_HAT_LEFT,	DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_HAT_RIGHT,	DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_HAT_UP,	DANCE_BUTTON_UP,		false },
			{ 0, JOY_HAT_DOWN,	DANCE_BUTTON_DOWN,		false },
			{ 1, JOY_4,			DANCE_BUTTON_LEFT,		false },
			{ 1, JOY_2,			DANCE_BUTTON_RIGHT,		false },
			{ 1, JOY_1,			DANCE_BUTTON_UP,		false },
			{ 1, JOY_3,			DANCE_BUTTON_DOWN,		false },
			{ 0, JOY_5,			DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_6,			DANCE_BUTTON_UPRIGHT,	false },
			{ 1, JOY_7,			DANCE_BUTTON_UPLEFT,	false },
			{ 1, JOY_8,			DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_10,		DANCE_BUTTON_BACK,		false },
			{ 0, JOY_9,			DANCE_BUTTON_START,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"MP-8866 Dual USB Joypad",
		"Super Dual Box",
		{
			{ 0, JOY_HAT_LEFT,	DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_HAT_RIGHT,	DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_HAT_UP,	DANCE_BUTTON_UP,		false },
			{ 0, JOY_HAT_DOWN,	DANCE_BUTTON_DOWN,		false },
			{ 1, JOY_4,			DANCE_BUTTON_LEFT,		false },
			{ 1, JOY_2,			DANCE_BUTTON_RIGHT,		false },
			{ 1, JOY_1,			DANCE_BUTTON_UP,		false },
			{ 1, JOY_3,			DANCE_BUTTON_DOWN,		false },
			{ 0, JOY_5,			DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_6,			DANCE_BUTTON_UPRIGHT,	false },
			{ 1, JOY_7,			DANCE_BUTTON_UPLEFT,	false },
			{ 1, JOY_8,			DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_10,		DANCE_BUTTON_BACK,		false },
			{ 0, JOY_9,			DANCE_BUTTON_START,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"NTPAD",
		"NTPAD",
		{
			{ 0, JOY_13,	DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_15,	DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_16,	DANCE_BUTTON_UP,		false },
			{ 0, JOY_14,	DANCE_BUTTON_DOWN,		false },
			{ 1, JOY_1,		DANCE_BUTTON_LEFT,		false },
			{ 1, JOY_3,		DANCE_BUTTON_RIGHT,		false },
			{ 1, JOY_4,		DANCE_BUTTON_UP,		false },
			{ 1, JOY_2,		DANCE_BUTTON_DOWN,		false },
			{ 0, JOY_5,		DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_6,		DANCE_BUTTON_UPRIGHT,	false },
			{ 1, JOY_7,		DANCE_BUTTON_UPLEFT,	false },
			{ 1, JOY_8,		DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_9,		DANCE_BUTTON_BACK,		false },
			{ 0, JOY_10,	DANCE_BUTTON_START,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"Psx Gamepad",
		"PSXPAD",
		{
			{ 0, JOY_LEFT,	DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_RIGHT,	DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_UP,	DANCE_BUTTON_UP,		false },
			{ 0, JOY_DOWN,	DANCE_BUTTON_DOWN,		false },
			{ 1, JOY_2,		DANCE_BUTTON_LEFT,		false },
			{ 1, JOY_1,		DANCE_BUTTON_RIGHT,		false },
			{ 1, JOY_4,		DANCE_BUTTON_UP,		false },
			{ 1, JOY_3,		DANCE_BUTTON_DOWN,		false },
			{ 0, JOY_7,		DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_5,		DANCE_BUTTON_UPRIGHT,	false },
			{ 1, JOY_8,		DANCE_BUTTON_UPLEFT,	false },
			{ 1, JOY_6,		DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_10,	DANCE_BUTTON_BACK,		false },
			{ 0, JOY_9,		DANCE_BUTTON_START,		false },
			END_MARKER
		}
	},
	{
		"dance",
		"XBOX Gamepad Plugin V0.01",
		"X-Box gamepad",
		{
			{ 0, JOY_HAT_LEFT,	DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_HAT_RIGHT,	DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_HAT_UP,	DANCE_BUTTON_UP,		false },
			{ 0, JOY_HAT_DOWN,	DANCE_BUTTON_DOWN,		false },
			{ 1, JOY_1,			DANCE_BUTTON_DOWN,		false },	// A
			{ 1, JOY_2,			DANCE_BUTTON_RIGHT,		false },	// B
			{ 1, JOY_3,			DANCE_BUTTON_LEFT,		false },	// X
			{ 1, JOY_4,			DANCE_BUTTON_UP,		false },	// Y
			{ 0, JOY_7,			DANCE_BUTTON_UPLEFT,	false },	// L shoulder
			{ 0, JOY_8,			DANCE_BUTTON_UPRIGHT,	false },	// R shoulder
			{ 0, JOY_9,			DANCE_BUTTON_START,		false },
			{ 0, JOY_10,		DANCE_BUTTON_BACK,		false },
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
			{ 0, JOY_13,		DANCE_BUTTON_UP,		false },
			{ 0, JOY_16,		DANCE_BUTTON_LEFT,		false },
			{ 0, JOY_14,		DANCE_BUTTON_RIGHT,		false },
			{ 0, JOY_15,		DANCE_BUTTON_DOWN,		false },
			{ 0, JOY_3,			DANCE_BUTTON_UPLEFT,	false },
			{ 0, JOY_2,			DANCE_BUTTON_UPRIGHT,	false },
			{ 0, JOY_10,		DANCE_BUTTON_START,		false },
			// Player 2.
			{ 0, JOY_29,		DANCE_BUTTON_UP,		true },
			{ 0, JOY_30,		DANCE_BUTTON_RIGHT,		true },
			{ 0, JOY_31,		DANCE_BUTTON_DOWN,		true },
			{ 0, JOY_32,		DANCE_BUTTON_LEFT,		true },
			{ 0, JOY_18,		DANCE_BUTTON_UPRIGHT,	true },
			{ 0, JOY_19,		DANCE_BUTTON_UPLEFT,	true },
			{ 0, JOY_26,		DANCE_BUTTON_START,		true },
			END_MARKER
		}
	},
	{
		"pump",
		"Pump USB",
		"Pump USB pad",
		{
			{ 0, PUMP_UL,		PUMP_BUTTON_UPLEFT,		false },
			{ 0, PUMP_UR,		PUMP_BUTTON_UPRIGHT,	false },
			{ 0, PUMP_MID,		PUMP_BUTTON_CENTER,		false },
			{ 0, PUMP_DL,		PUMP_BUTTON_DOWNLEFT,	false },
			{ 0, PUMP_DR,		PUMP_BUTTON_DOWNRIGHT,	false },
			{ 0, PUMP_ESCAPE,	PUMP_BUTTON_BACK,		false },
			{ 0, PUMP_2P_UL,	PUMP_BUTTON_UPLEFT,		true },
			{ 0, PUMP_2P_UR,	PUMP_BUTTON_UPRIGHT,	true },
			{ 0, PUMP_2P_MID,	PUMP_BUTTON_CENTER,		true },
			{ 0, PUMP_2P_DL,	PUMP_BUTTON_DOWNLEFT,	true },
			{ 0, PUMP_2P_DR,	PUMP_BUTTON_DOWNRIGHT,	true },
			END_MARKER
		}
	},
};

void InputMapper::ApplyMapping( const Mapping *maps, GameController gc, InputDevice device )
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
			if( map_gc >= GAME_CONTROLLER_INVALID )
				continue;
		}

		DeviceInput di( device, maps[k].deviceButton );
		GameInput gi( map_gc, maps[k].gb );
		SetInputMap( di, gi, maps[k].iSlotIndex );
	}
}

void InputMapper::AutoMapJoysticksForCurrentGame()
{
	vector<InputDevice> vDevices;
	vector<CString> vDescriptions;
	INPUTMAN->GetDevicesAndDescriptions(vDevices,vDescriptions);

	int iNumJoysticksMapped = 0;

	const Game* pGame = GAMESTATE->m_pCurGame;

	for( unsigned i=0; i<vDevices.size(); i++ )
	{
		InputDevice device = vDevices[i];
		CString sDescription = vDescriptions[i];
		for( unsigned j=0; j<ARRAYSIZE(g_AutoJoyMappings); j++ )
		{
			const AutoJoyMapping& mapping = g_AutoJoyMappings[j];

			if( pGame != GAMEMAN->StringToGameType(mapping.szGame) )
				continue;	// games don't match

			CString sDriverRegex = mapping.szDriverRegex;
			Regex regex( sDriverRegex );
			if( !regex.Compare(sDescription) )
				continue;	// driver names don't match

			//
			// We have a mapping for this joystick
			//
			GameController gc = (GameController)iNumJoysticksMapped;
			if( gc >= GAME_CONTROLLER_INVALID )
				break;	// stop mapping.  We already mapped one device for each game controller.

			LOG->Info( "Applying default joystick mapping #%d for device '%s' (%s)",
				iNumJoysticksMapped+1, mapping.szDriverRegex, mapping.szControllerName );

			ApplyMapping( mapping.maps, gc, device );

			iNumJoysticksMapped++;
		}
	}
}

void InputMapper::ReadMappingsFromDisk()
{
	ASSERT( GAMEMAN != NULL );

	ClearAllMappings();

	IniFile ini;
	if( !ini.ReadFile( KEYMAPS_PATH ) )
		LOG->Trace( "Couldn't open mapping file \"%s\": %s.", KEYMAPS_PATH, ini.GetError().c_str() );

	const IniFile::key *Key = ini.GetKey( GAMESTATE->GetCurrentGame()->m_szName );

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
	ini.ReadFile( KEYMAPS_PATH );
	
	// erase the key so that we overwrite everything for this game
	ini.DeleteKey( GAMESTATE->GetCurrentGame()->m_szName );

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
			
			ini.SetValue( GAMESTATE->GetCurrentGame()->m_szName, sNameString, sValueString );
		}
	}

	ini.WriteFile( KEYMAPS_PATH );
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
		for( int b=0; b<NUM_DEVICE_BUTTONS[d]; b++ )
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
	if( GAMESTATE->m_pCurStyle == NULL )
	{
		StyleI.MakeInvalid();
		return;
	}

	StyleI = GAMESTATE->m_pCurStyle->GameInputToStyleInput( GameI );
}

void InputMapper::GameToMenu( GameInput GameI, MenuInput &MenuI )
{
	const Game* pGame = GAMESTATE->GetCurrentGame();
	MenuI = pGame->GameInputToMenuInput( GameI );
}

void InputMapper::StyleToGame( StyleInput StyleI, GameInput &GameI )
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle();
	GameI = pStyle->StyleInputToGameInput( StyleI );
}


void InputMapper::MenuToGame( MenuInput MenuI, GameInput GameIout[4] )
{
	const Game* pGame = GAMESTATE->GetCurrentGame();
	pGame->MenuInputToGameInput( MenuI, GameIout );
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

void InputMapper::ResetKeyRepeat( GameInput GameI )
{
	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;
		if( GameToDevice( GameI, i, DeviceI ) )
			INPUTFILTER->ResetKeyRepeat( DeviceI );
	}
}

void InputMapper::ResetKeyRepeat( MenuInput MenuI )
{
	GameInput GameI[4];
	MenuToGame( MenuI, GameI );
	for( int i=0; i<4; i++ )
		if( GameI[i].IsValid() )
			ResetKeyRepeat( GameI[i] );
}

void InputMapper::ResetKeyRepeat( StyleInput StyleI )
{
	GameInput GameI;
	StyleToGame( StyleI, GameI );
	ResetKeyRepeat( GameI );
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
