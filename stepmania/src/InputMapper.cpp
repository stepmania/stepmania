#include "stdafx.h"
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


InputMapper*	INPUTMAPPER = NULL;	// global and accessable from anywhere in our program


InputMapper::InputMapper()
{
	m_sCurrentGame = GAMEMAN->GetCurrentGameDef()->m_szName;

	ReadMappingsFromDisk();
}


InputMapper::~InputMapper()
{
	// don't worry about releasing the Song array.  Let the OS do it :-)
	SaveMappingsToDisk();
}


void InputMapper::ReadMappingsFromDisk()
{
	ASSERT( GAMEMAN != NULL );

	IniFile ini;
	ini.SetPath( m_sCurrentGame + ".ini" );
	if( !ini.ReadFile() ) {
		return;		// load nothing
		//FatalError( "could not read config file" );
	}

	CMapStringToString* pKey = ini.GetKeyPointer("Input");
	CString name_string, value_string;

	if( pKey != NULL )
	{
		for( POSITION pos = pKey->GetStartPosition(); pos != NULL; )
		{
			pKey->GetNextAssoc( pos, name_string, value_string );

			GameInput GameI;
			GameI.fromString(name_string);

			CStringArray sDeviceInputStrings;
			split( value_string, ",", sDeviceInputStrings, false );

			for( int i=0; i<sDeviceInputStrings.GetSize() && i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
			{
				DeviceInput DeviceI;
				DeviceI.fromString( sDeviceInputStrings[i] );
				if( !DeviceI.IsBlank() )
					SetInputMap( DeviceI, GameI, i );
			}
		}
	}
}


void InputMapper::SaveMappingsToDisk()
{
	IniFile ini;
	ini.SetPath( m_sCurrentGame + ".ini" );
//	ini.ReadFile();		// don't read the file so that we overwrite everything there


	// iterate over our input map and write all mappings to the ini file
	for( int i=0; i<MAX_INSTRUMENTS; i++ )
	{
		for( int j=0; j<MAX_INSTRUMENT_BUTTONS; j++ )
		{
			CString sNameString, sValueString;
			
			GameInput GameI( (InstrumentNumber)i, (InstrumentButton)j );
			sNameString = GameI.toString();
			sValueString = ssprintf( "%s,%s,%s", 
				m_GItoDI[i][j][0].toString(), m_GItoDI[i][j][1].toString(), m_GItoDI[i][j][2].toString() );
			
			ini.SetValue( "Input", sNameString, sValueString );
		}
	}

	ini.WriteFile();
}


///////////////////////////////////////
// Input mapping stuff
///////////////////////////////////////

void InputMapper::SetInputMap( DeviceInput DeviceI, GameInput GameI, int iSlotIndex, bool bOverrideHardCoded )
{
	// remove the old input
	ClearFromInputMap( DeviceI );
	ClearFromInputMap( GameI, iSlotIndex );

	m_GItoDI[GameI.number][GameI.button][iSlotIndex] = DeviceI;


	UpdateTempDItoGI();
}

void InputMapper::ClearFromInputMap( DeviceInput DeviceI )
{
	// search for where this DeviceI maps to

	for( int p=0; p<MAX_INSTRUMENTS; p++ )
	{
		for( int b=0; b<MAX_INSTRUMENT_BUTTONS; b++ )
		{
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ )
			{
				if( m_GItoDI[p][b][s] == DeviceI )
					m_GItoDI[p][b][s].MakeBlank();
			}
		}
	}
	
	UpdateTempDItoGI();
}

void InputMapper::ClearFromInputMap( GameInput GameI, int iSlotIndex )
{
	if( GameI.IsBlank() )
		return;

	m_GItoDI[GameI.number][GameI.button][iSlotIndex].MakeBlank();

	UpdateTempDItoGI();
}

void InputMapper::UpdateTempDItoGI()
{
	// clear out m_tempDItoGI
	for( int d=0; d<NUM_INPUT_DEVICES; d++ )
	{
		for( int b=0; b<NUM_DEVICE_BUTTONS; b++ )
		{
			m_tempDItoGI[d][b].MakeBlank();
		}
	}


	// repopulate m_tempDItoGI
	for( int n=0; n<MAX_INSTRUMENTS; n++ )
	{
		for( int b=0; b<MAX_INSTRUMENT_BUTTONS; b++ )
		{
			for( int s=0; s<NUM_GAME_TO_DEVICE_SLOTS; s++ )
			{
				GameInput GameI( (InstrumentNumber)n, (InstrumentButton)b );
				DeviceInput DeviceI = m_GItoDI[n][b][s];

				if( DeviceI.IsBlank() )
					continue;

				m_tempDItoGI[DeviceI.device][DeviceI.button] = GameI;
			}
		}
	}
}

bool InputMapper::DeviceToGame( DeviceInput DeviceI, GameInput& GameI ) // return true if there is a mapping from device to pad
{
	GameI = m_tempDItoGI[DeviceI.device][DeviceI.button];
	return GameI.number != PLAYER_NONE;
}

bool InputMapper::GameToDevice( GameInput GameI, int iSoltNum, DeviceInput& DeviceI )	// return true if there is a mapping from pad to device
{
	DeviceI = m_GItoDI[GameI.number][GameI.button][iSoltNum];
	return DeviceI.device != DEVICE_NONE;
}

struct HardCodedMenuKey
{
	InputDevice device;
	int device_button;
	PlayerNumber player_no;
	MenuButton menu_button;
};

const HardCodedMenuKey g_HardCodedMenuKeys[] =
{
	{ DEVICE_KEYBOARD,	DIK_UP,		PLAYER_2, MENU_BUTTON_UP },
	{ DEVICE_KEYBOARD,	DIK_DOWN,	PLAYER_2, MENU_BUTTON_DOWN },
	{ DEVICE_KEYBOARD,	DIK_LEFT,	PLAYER_2, MENU_BUTTON_LEFT },
	{ DEVICE_KEYBOARD,	DIK_RIGHT,	PLAYER_2, MENU_BUTTON_RIGHT },
	{ DEVICE_KEYBOARD,	DIK_RETURN,	PLAYER_2, MENU_BUTTON_START },
	{ DEVICE_KEYBOARD,	DIK_ESCAPE,	PLAYER_2, MENU_BUTTON_BACK },
};
const int NUM_HARD_CODED_MENU_KEYS = sizeof(g_HardCodedMenuKeys) / sizeof(HardCodedMenuKey);

MenuInput InputMapper::DeviceToMenu( DeviceInput DeviceI )
{
	for( int i=0; i<NUM_HARD_CODED_MENU_KEYS; i++ )
	{
		if( g_HardCodedMenuKeys[i].device == DeviceI.device  &&
			g_HardCodedMenuKeys[i].device_button == DeviceI.button )
		{
			return MenuInput( g_HardCodedMenuKeys[i].player_no, g_HardCodedMenuKeys[i].menu_button );
		}
	}

	return MenuInput( PLAYER_NONE, MENU_BUTTON_NONE );
}

DeviceInput InputMapper::MenuToDevice( MenuInput MenuI )
{
	for( int i=0; i<NUM_HARD_CODED_MENU_KEYS; i++ )
	{
		if( g_HardCodedMenuKeys[i].player_no == MenuI.player  &&
			g_HardCodedMenuKeys[i].menu_button == MenuI.button )
		{
			return DeviceInput( g_HardCodedMenuKeys[i].device, g_HardCodedMenuKeys[i].device_button );
		}
	}

	return DeviceInput( DEVICE_NONE, -1 );
}

void InputMapper::GameToStyle( GameInput GameI, StyleInput &StyleI )
{
	StyleDef* pStyleDef = GAMEMAN->GetCurrentStyleDef();
	StyleI = pStyleDef->GameInputToStyleInput( GameI );
}

void InputMapper::GameToMenu( GameInput GameI, MenuInput &MenuI )
{
	GameDef* pGameDef = GAMEMAN->GetCurrentGameDef();
	MenuI = pGameDef->GameInputToMenuInput( GameI );
}

void InputMapper::StyleToGame( StyleInput StyleI, GameInput &GameI )
{
	StyleDef* pStyleDef = GAMEMAN->GetCurrentStyleDef();
	GameI = pStyleDef->StyleInputToGameInput( StyleI );
}

void InputMapper::MenuToGame( MenuInput MenuI, GameInput &GameI )
{
	GameDef* pGameDef = GAMEMAN->GetCurrentGameDef();
	GameI = pGameDef->MenuInputToGameInput( MenuI );
}

bool InputMapper::IsButtonDown( GameInput GameI )
{
	for( int i=0; i<NUM_GAME_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput DeviceI;

		if( GameToDevice( GameI, i, DeviceI ) )
		{
			if( INPUTMAN->IsBeingPressed( DeviceI ) )
				return true;
		}
	}

	return false;
}

bool InputMapper::IsButtonDown( MenuInput MenuI )
{
	GameInput GameI;
	MenuToGame( MenuI, GameI );
	return IsButtonDown( GameI );
}


bool InputMapper::IsButtonDown( StyleInput StyleI )
{
	GameInput GameI;
	StyleToGame( StyleI, GameI );
	return IsButtonDown( GameI );
}



