#pragma once
/*
-----------------------------------------------------------------------------
 Class: InputMapper

 Desc: Holds user-chosen preferences and saves it between sessions.  This class 
    also has temporary holders for information that passed between windows - e.g.
	GameplayStatistics.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageInput.h"
#include "GameInput.h"
#include "MenuInput.h"
#include "StyleInput.h"
#include "GameConstantsAndTypes.h"



const int NUM_GAME_TO_DEVICE_SLOTS	= 3;	// three device inputs may map to one game input


class InputMapper
{
public:
	InputMapper();
	~InputMapper();

	void SwitchGame( CString sNewGame )
	{
		SaveMappingsToDisk();
		m_sCurrentGame = sNewGame;
		ReadMappingsFromDisk();
	};

	void ReadMappingsFromDisk();
	void SaveMappingsToDisk();


	// input mapping stuff
	void SetInputMap( DeviceInput DeviceI, GameInput GameI, int iSlotIndex, bool bOverrideHardCoded = false );
	void ClearFromInputMap( DeviceInput DeviceI );
	void ClearFromInputMap( GameInput GameI, int iSlotIndex );
	
	bool DeviceToGame( DeviceInput DeviceI, GameInput& GameI );	// return true if there is a mapping from device to pad
	bool GameToDevice( GameInput GameI, int iSoltNum, DeviceInput& DeviceI );	// return true if there is a mapping from pad to device

	MenuInput DeviceToMenu( DeviceInput DeviceI );
	DeviceInput MenuToDevice( MenuInput MenuI );

	void GameToStyle( GameInput GameI, StyleInput &StyleI );
	void StyleToGame( StyleInput StyleI, GameInput &GameI );

	void GameToMenu( GameInput GameI, MenuInput &MenuI );
	void MenuToGame( MenuInput MenuI, GameInput &GameI );

	bool IsButtonDown( GameInput GameI );
	bool IsButtonDown( MenuInput MenuI );
	bool IsButtonDown( StyleInput StyleI );


protected:
	CString m_sCurrentGame;

	// all the DeviceInputs that map to a GameInput
	DeviceInput m_GItoDI[MAX_INSTRUMENTS][MAX_INSTRUMENT_BUTTONS][NUM_GAME_TO_DEVICE_SLOTS];

	// lookup for efficiency from a DeviceInput to a GameInput
	// This is repopulated every time m_PItoDI changes by calling UpdateTempDItoPI().
	GameInput m_tempDItoGI[NUM_INPUT_DEVICES][NUM_DEVICE_BUTTONS];
	void UpdateTempDItoGI();
};


extern InputMapper*	INPUTMAPPER;	// global and accessable from anywhere in our program
