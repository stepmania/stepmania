#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: See Header.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PrefsManager.h"
#include "IniFile.h"


PrefsManager*	PREFS = NULL;	// global and accessable from anywhere in our program




PrefsManager::PrefsManager()
{
	m_GameMode = MODE_SINGLE;
	m_SongSortOrder = SORT_GROUP;
	m_iCurrentStage = 1;
	m_ThemeName = "default";

	ReadPrefsFromDisk();
	SetHardCodedButtons();
}


PrefsManager::~PrefsManager()
{
	// don't worry about releasing the Song array.  Let the OS do it :-)
	SavePrefsToDisk();
}


void PrefsManager::ReadPrefsFromDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
	if( !ini.ReadFile() ) {
		return;		// load nothing
		//RageError( "could not read config file" );
	}

	CMapStringToString* pKey = ini.GetKeyPointer("Input");
	CString name_string, value_string;

	if( pKey != NULL )
	{
		for( POSITION pos = pKey->GetStartPosition(); pos != NULL; )
		{
			pKey->GetNextAssoc( pos, name_string, value_string );

			PadInput pi;
			pi.fromString(name_string);

			CStringArray sDeviceInputStrings;
			split( value_string, ",", sDeviceInputStrings, false );

			for( int i=0; i<sDeviceInputStrings.GetSize() && i<NUM_PAD_TO_DEVICE_SLOTS; i++ )
			{
				DeviceInput di;
				di.fromString( sDeviceInputStrings[i] );
				if( !di.IsBlank() )
					SetInputMap( di, pi, i );
			}
		}
	}

	this->SetHardCodedButtons();


	pKey = ini.GetKeyPointer( "GameOptions" );
	if( pKey )
	{
		for( POSITION pos = pKey->GetStartPosition(); pos != NULL; )
		{
			pKey->GetNextAssoc( pos, name_string, value_string );

			if( name_string == "Windowed" )				m_GameOptions.m_bWindowed		= ( value_string == "1" );
			if( name_string == "Resolution" )			m_GameOptions.m_iResolution		= atoi( value_string );
			if( name_string == "DisplayColor" )			m_GameOptions.m_iDisplayColor	= atoi( value_string );
			if( name_string == "TextureColor" )			m_GameOptions.m_iTextureColor	= atoi( value_string );
			if( name_string == "FilterTextures" )		m_GameOptions.m_bFilterTextures	= ( value_string == "1" );
			if( name_string == "Shadows" )				m_GameOptions.m_bShadows		= ( value_string == "1" );
			if( name_string == "IgnoreJoyAxes" )		m_GameOptions.m_bIgnoreJoyAxes	= ( value_string == "1" );
			if( name_string == "ShowFPS" )				m_GameOptions.m_bShowFPS		= ( value_string == "1" );
			if( name_string == "UseRandomVis" )			m_GameOptions.m_bUseRandomVis	= ( value_string == "1" );
			if( name_string == "SkipCaution" )			m_GameOptions.m_bSkipCaution	= ( value_string == "1" );
			if( name_string == "Announcer" )			m_GameOptions.m_bAnnouncer		= ( value_string == "1" );
		}
	}

}


void PrefsManager::SavePrefsToDisk()
{
	IniFile ini;
	ini.SetPath( "StepMania.ini" );
//	ini.ReadFile();		// don't read the file so that we overwrite everything there


	// iterate over our input map and write all mappings to the ini file
	for( int i=0; i<NUM_PADS; i++ )
	{
		for( int j=0; j<NUM_PAD_BUTTONS; j++ )
		{
			CString sNameString, sValueString;
			
			PadInput pi( (PadNumber)i, (PadButton)j );
			sNameString = pi.toString();
			sValueString = ssprintf( "%s,%s,%s", 
				m_PItoDI[i][j][0].toString(), m_PItoDI[i][j][1].toString(), m_PItoDI[i][j][2].toString() );
			
			ini.SetValue( "Input", sNameString, sValueString );
		}
	}

	// save the GameOptions
	ini.SetValue( "GameOptions", "Windowed",		m_GameOptions.m_bWindowed ? "1":"0" );
	ini.SetValue( "GameOptions", "Resolution",		ssprintf("%d", m_GameOptions.m_iResolution) );
	ini.SetValue( "GameOptions", "DisplayColor",	ssprintf("%d", m_GameOptions.m_iDisplayColor) );
	ini.SetValue( "GameOptions", "TextureColor",	ssprintf("%d", m_GameOptions.m_iTextureColor) );
	ini.SetValue( "GameOptions", "FilterTextures",	m_GameOptions.m_bFilterTextures ? "1":"0" );
	ini.SetValue( "GameOptions", "Shadows",			m_GameOptions.m_bShadows ? "1":"0" );
	ini.SetValue( "GameOptions", "IgnoreJoyAxes",	m_GameOptions.m_bIgnoreJoyAxes ? "1":"0" );
	ini.SetValue( "GameOptions", "ShowFPS",			m_GameOptions.m_bShowFPS ? "1":"0" );
	ini.SetValue( "GameOptions", "UseRandomVis",	m_GameOptions.m_bUseRandomVis ? "1":"0" );
	ini.SetValue( "GameOptions", "SkipCaution",		m_GameOptions.m_bSkipCaution ? "1":"0" );
	ini.SetValue( "GameOptions", "Announcer",		m_GameOptions.m_bAnnouncer ? "1":"0" );



	ini.WriteFile();
}


///////////////////////////////////////
// Input mapping stuff
///////////////////////////////////////

void PrefsManager::SetInputMap( DeviceInput di, PadInput pi, int iSlotIndex, bool bOverrideHardCoded )
{
	if( IsAHardCodedDeviceInput(di) && !bOverrideHardCoded )
		return;		// don't allow hard coded inputs to be overwritten

	// remove the old input
	ClearFromInputMap( di );
	ClearFromInputMap( pi, iSlotIndex );

	m_PItoDI[pi.pad_no][pi.button][iSlotIndex] = di;


	UpdateTempDItoPI();
}

void PrefsManager::ClearFromInputMap( DeviceInput di )
{
	// search for where this di maps to

	for( int p=0; p<NUM_PADS; p++ )
	{
		for( int b=0; b<NUM_PAD_BUTTONS; b++ )
		{
			for( int s=0; s<NUM_PAD_TO_DEVICE_SLOTS; s++ )
			{
				if( m_PItoDI[p][b][s] == di )
					m_PItoDI[p][b][s].MakeBlank();
			}
		}
	}
	
	UpdateTempDItoPI();
}

void PrefsManager::ClearFromInputMap( PadInput pi, int iSlotIndex )
{
	if( pi.IsBlank() )
		return;

	m_PItoDI[pi.pad_no][pi.button][iSlotIndex].MakeBlank();

	UpdateTempDItoPI();
}

void PrefsManager::UpdateTempDItoPI()
{
	// clear out m_tempDItoPI
	for( int d=0; d<NUM_INPUT_DEVICES; d++ )
	{
		for( int b=0; b<NUM_DEVICE_BUTTONS; b++ )
		{
			m_tempDItoPI[d][b].MakeBlank();
		}
	}


	// repopulate m_tempDItoPI
	for( int p=0; p<NUM_PADS; p++ )
	{
		for( int b=0; b<NUM_PAD_BUTTONS; b++ )
		{
			for( int s=0; s<NUM_PAD_TO_DEVICE_SLOTS; s++ )
			{
				PadInput PadI( (PadNumber)p, (PadButton)b );
				DeviceInput DeviceI = m_PItoDI[p][b][s];

				if( DeviceI.IsBlank() )
					continue;

				m_tempDItoPI[DeviceI.device][DeviceI.button] = PadI;
			}
		}
	}
}


const PadInput g_HardCodedPadInputs[] = {
	PadInput(PAD_1,BUTTON_LEFT),
	PadInput(PAD_1,BUTTON_RIGHT),
	PadInput(PAD_1,BUTTON_UP),
	PadInput(PAD_1,BUTTON_DOWN),
	PadInput(PAD_1,BUTTON_BACK),
	PadInput(PAD_1,BUTTON_NEXT),
};
const int NUM_HARD_CODED_INPUTS = sizeof(g_HardCodedPadInputs) / sizeof(PadInput);

const DeviceInput g_HardCodedDeviceInputs[NUM_HARD_CODED_INPUTS] = {
	DeviceInput(DEVICE_KEYBOARD,DIK_LEFT),
	DeviceInput(DEVICE_KEYBOARD,DIK_RIGHT),
	DeviceInput(DEVICE_KEYBOARD,DIK_UP),
	DeviceInput(DEVICE_KEYBOARD,DIK_DOWN),
	DeviceInput(DEVICE_KEYBOARD,DIK_ESCAPE),
	DeviceInput(DEVICE_KEYBOARD,DIK_RETURN),
};

void PrefsManager::SetHardCodedButtons()
{
	for( int i=0; i<NUM_HARD_CODED_INPUTS; i++ )
	{
		SetInputMap( g_HardCodedDeviceInputs[i], g_HardCodedPadInputs[i], NUM_PAD_TO_DEVICE_SLOTS-1, true );	// always go in the last slot
	}
}

bool PrefsManager::IsAHardCodedDeviceInput( DeviceInput di )
{
	for( int i=0; i<NUM_HARD_CODED_INPUTS; i++ )
	{
		if( di == g_HardCodedDeviceInputs[i] )
			return true;
	}
	
	return false;
}


bool PrefsManager::DeviceToPad( DeviceInput di, PadInput& pi ) // return true if there is a mapping from device to pad
{
	pi = m_tempDItoPI[di.device][di.button];
	return pi.pad_no != PAD_NONE;
}

bool PrefsManager::PadToDevice( PadInput pi, int iSoltNum, DeviceInput& di )	// return true if there is a mapping from pad to device
{
	di = m_PItoDI[pi.pad_no][pi.button][iSoltNum];
	return di.device != DEVICE_NONE;
}

void PrefsManager::PadToPlayer( PadInput PadI, PlayerInput &PlayerI )
{
	PlayerI.player_no = PLAYER_NONE;
	PlayerI.step = STEP_NONE;

	switch( m_GameMode )
	{
	case MODE_SINGLE:
	case MODE_SOLO:
		if( PadI.pad_no == PAD_1 )
			PlayerI.player_no = PLAYER_1;
		else
			PlayerI.player_no = PLAYER_NONE;
		
		if ( PadI.button == BUTTON_LEFT )		PlayerI.step |= STEP_PAD1_LEFT;
		if ( PadI.button == BUTTON_UPLEFT )		PlayerI.step |= STEP_PAD1_UPLEFT;
		if ( PadI.button == BUTTON_DOWN )		PlayerI.step |= STEP_PAD1_DOWN;
		if ( PadI.button == BUTTON_UP )			PlayerI.step |= STEP_PAD1_UP;
		if ( PadI.button == BUTTON_UPRIGHT )	PlayerI.step |= STEP_PAD1_UPRIGHT;
		if ( PadI.button == BUTTON_RIGHT )		PlayerI.step |= STEP_PAD1_RIGHT;
	
		break;
	case MODE_VERSUS:
	case MODE_COUPLE:
		if( PadI.pad_no == PAD_1 )
			PlayerI.player_no = PLAYER_1;
		else if( PadI.pad_no == PAD_2 )
			PlayerI.player_no = PLAYER_2;
		else
			PlayerI.player_no = PLAYER_NONE;

		if ( PadI.button == BUTTON_LEFT )		PlayerI.step |= STEP_PAD1_LEFT;
		if ( PadI.button == BUTTON_UPLEFT )		PlayerI.step |= STEP_PAD1_UPLEFT;
		if ( PadI.button == BUTTON_DOWN )		PlayerI.step |= STEP_PAD1_DOWN;
		if ( PadI.button == BUTTON_UP )			PlayerI.step |= STEP_PAD1_UP;
		if ( PadI.button == BUTTON_UPRIGHT )	PlayerI.step |= STEP_PAD1_UPRIGHT;
		if ( PadI.button == BUTTON_RIGHT )		PlayerI.step |= STEP_PAD1_RIGHT;
		
		break;
	case MODE_DOUBLE:

		switch( PadI.pad_no )
		{
		case PAD_1:	
			PlayerI.player_no = PLAYER_1;
			if ( PadI.button == BUTTON_LEFT )	PlayerI.step |= STEP_PAD1_LEFT;
			if ( PadI.button == BUTTON_DOWN )	PlayerI.step |= STEP_PAD1_DOWN;
			if ( PadI.button == BUTTON_UP )		PlayerI.step |= STEP_PAD1_UP;
			if ( PadI.button == BUTTON_RIGHT )	PlayerI.step |= STEP_PAD1_RIGHT;
			break;
		case PAD_2:	
			PlayerI.player_no = PLAYER_1;
			if ( PadI.button == BUTTON_LEFT )	PlayerI.step |= STEP_PAD2_LEFT;
			if ( PadI.button == BUTTON_DOWN )	PlayerI.step |= STEP_PAD2_DOWN;
			if ( PadI.button == BUTTON_UP )		PlayerI.step |= STEP_PAD2_UP;
			if ( PadI.button == BUTTON_RIGHT )	PlayerI.step |= STEP_PAD2_RIGHT;
			break;
		case PAD_NONE:
			PlayerI.player_no = PLAYER_NONE;
			break;
		}

		break;
	default:
		ASSERT( false );		// invalid GameMode
	}
}

void PrefsManager::PlayerToPad(  PlayerInput PlayerI, PadInput &PadI )
{
	PadI.pad_no = PAD_NONE;
	PadI.button = BUTTON_NONE;

	switch( m_GameMode )
	{
	case MODE_SINGLE:
	case MODE_SOLO:
	case MODE_VERSUS:
	case MODE_COUPLE:
		PadI.pad_no = (PadNumber)PlayerI.player_no;
		
		if ( PlayerI.step == STEP_PAD1_LEFT )		PadI.button = BUTTON_LEFT;
		if ( PlayerI.step == STEP_PAD1_UPLEFT )		PadI.button = BUTTON_UPLEFT;
		if ( PlayerI.step == STEP_PAD1_DOWN )		PadI.button = BUTTON_DOWN;
		if ( PlayerI.step == STEP_PAD1_UP )			PadI.button = BUTTON_UP;
		if ( PlayerI.step == STEP_PAD1_UPRIGHT )	PadI.button = BUTTON_UPRIGHT;
		if ( PlayerI.step == STEP_PAD1_RIGHT )		PadI.button = BUTTON_RIGHT;
	
		break;
	case MODE_DOUBLE:
		switch( PlayerI.step )
		{
		case STEP_PAD1_LEFT:	PadI.pad_no = PAD_1; PadI.button = BUTTON_LEFT;		break; 	
		case STEP_PAD1_DOWN:	PadI.pad_no = PAD_1; PadI.button = BUTTON_DOWN;		break; 	
		case STEP_PAD1_UP:		PadI.pad_no = PAD_1; PadI.button = BUTTON_UP;		break; 	
		case STEP_PAD1_RIGHT:	PadI.pad_no = PAD_1; PadI.button = BUTTON_RIGHT;	break; 	
		case STEP_PAD2_LEFT:	PadI.pad_no = PAD_2; PadI.button = BUTTON_LEFT;		break; 	
		case STEP_PAD2_DOWN:	PadI.pad_no = PAD_2; PadI.button = BUTTON_DOWN;		break; 	
		case STEP_PAD2_UP:		PadI.pad_no = PAD_2; PadI.button = BUTTON_UP;		break; 	
		case STEP_PAD2_RIGHT:	PadI.pad_no = PAD_2; PadI.button = BUTTON_RIGHT;	break; 	
		}

		break;
	default:
		ASSERT( false );		// invalid GameMode
	}
}

bool PrefsManager::IsButtonDown( PadInput pi )
{
	for( int i=0; i<NUM_PAD_TO_DEVICE_SLOTS; i++ )
	{
		DeviceInput di;

		if( PadToDevice( pi, i, di ) )
		{
			if( INPUT->IsBeingPressed( di ) )
				return true;
		}
	}

	return false;
}

bool PrefsManager::IsButtonDown( PlayerInput PlayerI )
{
	PadInput PadI;
	PlayerToPad( PlayerI, PadI );
	return IsButtonDown( PadI );
}



