/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: Manages input mapping, saves preferences, and holds data that is passed 
	between Windows.

 Copyright (c) 2001-2002 by the names listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _PrefsManager_H_
#define _PrefsManager_H_


#include "RageInput.h"
#include "PadInput.h"
#include "PlayerInput.h"
#include "GameTypes.h"



const int NUM_PAD_TO_DEVICE_SLOTS	= 3;	// three device inputs may map to one pad input


class Song;

class PrefsManager
{
public:
	PrefsManager();
	~PrefsManager();

	GameMode m_GameMode;

	ScoreSummary scoreSummaryPlayer[NUM_PLAYERS];

	SongSortOrder	m_SongSortOrder;

	int				m_iCurrentStage;

	GameOptions		m_GameOptions;
	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];
	SongOptions		m_SongOptions;

	bool IsPlayerEnabled( PlayerNumber PlayerNo )
	{
		switch( m_GameMode )
		{
		case MODE_VERSUS:
		case MODE_COUPLE:
			if( PlayerNo == PLAYER_1 )
				return true;
			if( PlayerNo == PLAYER_2 )
				return true;
			break;
		case MODE_SINGLE:
		case MODE_SOLO:
		case MODE_DOUBLE:
			if( PlayerNo == PLAYER_1 )
				return true;
			if( PlayerNo == PLAYER_2 )
				return false;
			break;
		default:
			ASSERT( false );	// invalid game mode
		}
		return false;
	};

	// prefs file stuff
	void ReadPrefsFromDisk();
	void SavePrefsToDisk();


	// input mapping stuff
	void SetInputMap( DeviceInput di, PadInput pi, int iSlotIndex, bool bOverrideHardCoded = false );
	void ClearFromInputMap( DeviceInput di );
	void ClearFromInputMap( PadInput pi, int iSlotIndex );
	
	void SetHardCodedButtons();
	bool IsAHardCodedDeviceInput( DeviceInput di );

	bool DeviceToPad( DeviceInput di, PadInput& pi );	// return true if there is a mapping from device to pad
	bool PadToDevice( PadInput pi, int iSoltNum, DeviceInput& di );	// return true if there is a mapping from pad to device

	void PadToPlayer( PadInput PadI, PlayerInput &PlayerI );
	void PlayerToPad(  PlayerInput PlayerI, PadInput &PadI );

	bool IsButtonDown( PadInput pi );
	bool IsButtonDown( PlayerInput PlayerI );

	// theme stuff
	CString m_ThemeName;




protected:

	DeviceInput m_PItoDI[NUM_PADS][NUM_PAD_BUTTONS][NUM_PAD_TO_DEVICE_SLOTS];	// all the DeviceInputs that map to a PadInput

	// lookup for efficiency from a DeviceInput to a PadInput
	// This is repopulated every time m_PItoDI changes by calling UpdateTempDItoPI().
	PadInput m_tempDItoPI[NUM_INPUT_DEVICES][NUM_DEVICE_BUTTONS];
	void UpdateTempDItoPI();
};


extern PrefsManager*	PREFS;	// global and accessable from anywhere in our program


#endif