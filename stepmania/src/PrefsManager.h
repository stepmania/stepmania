/*
-----------------------------------------------------------------------------
 Class: PrefsManager

 Desc: Manages all other game data that isn't handled about SongManager, 
	GameManager, or ThemeManager.  Some of this data is saved between sessions,
	for example, input mapping settings and GameOptions.  This class also has 
	temporary holders for information that passed between windows - e.g.
	ScoreSummary.

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

	ScoreSummary	m_ScoreSummary[NUM_PLAYERS];	// for passing from Dancing to Results
	SongSortOrder	m_SongSortOrder;				// used by MusicWheel and should be saved until the app exits
	int				m_iCurrentStage;				// number of stages played +1

	GameOptions		m_GameOptions;
	PlayerOptions	m_PlayerOptions[NUM_PLAYERS];
	SongOptions		m_SongOptions;


	// read and write to disk
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


protected:

	DeviceInput m_PItoDI[NUM_PADS][NUM_PAD_BUTTONS][NUM_PAD_TO_DEVICE_SLOTS];	// all the DeviceInputs that map to a PadInput

	// lookup for efficiency from a DeviceInput to a PadInput
	// This is repopulated every time m_PItoDI changes by calling UpdateTempDItoPI().
	PadInput m_tempDItoPI[NUM_INPUT_DEVICES][NUM_DEVICE_BUTTONS];
	void UpdateTempDItoPI();
};


extern PrefsManager*	PREFS;	// global and accessable from anywhere in our program


#endif