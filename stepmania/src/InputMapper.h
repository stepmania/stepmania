/* InputMapper - Holds user-chosen input preferences and saves it between sessions. */

#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

#include "RageInputDevice.h"
#include "GameInput.h"
#include "MenuInput.h"
#include "GameConstantsAndTypes.h"

const int NUM_GAME_TO_DEVICE_SLOTS	= 5;	// five device inputs may map to one game input
const int NUM_SHOWN_GAME_TO_DEVICE_SLOTS = 3;
const int NUM_USER_GAME_TO_DEVICE_SLOTS = 2;
#define NO_DEFAULT_KEY DeviceButton_Invalid

class InputScheme
{
public:
	const char	*m_szName;
	int		m_iButtonsPerController;
	char		m_szButtonNames[NUM_GameButton][60];	// The name used by the button graphics system.  e.g. "left", "right", "middle C", "snare"
	GameButton	m_DedicatedMenuButton[NUM_MenuButton];
	GameButton	m_SecondaryMenuButton[NUM_MenuButton];
	DeviceButton	m_iDefaultKeyboardKey[NUM_GameController][NUM_GameButton];	// default keyboard keys only have an effect the current game is this game

	GameButton ButtonNameToIndex( const RString &sButtonName ) const;
	MenuButton GameInputToMenuButton( GameInput GameI ) const;
	void MenuButtonToGameInputs( MenuButton MenuI, PlayerNumber pn, GameInput GameIout[4] ) const;
};

class InputMapper
{
public:
	InputMapper();
	~InputMapper();

	void SetInputScheme( const InputScheme *pInputScheme );
	const InputScheme *GetInputScheme() const;
	void SetJoinControllers( PlayerNumber pn );

	void ReadMappingsFromDisk();
	void SaveMappingsToDisk();

	void ClearAllMappings();

	void SetInputMap( const DeviceInput &DeviceI, const GameInput &GameI, int iSlotIndex );
	void ClearFromInputMap( const DeviceInput &DeviceI );
	bool ClearFromInputMap( const GameInput &GameI, int iSlotIndex );

	void AddDefaultMappingsForCurrentGameIfUnmapped();
	void AutoMapJoysticksForCurrentGame();
	bool CheckForChangedInputDevicesAndRemap( RString &sMessageOut );

	bool IsMapped( const DeviceInput &DeviceI );
	
	bool DeviceToGame( const DeviceInput &DeviceI, GameInput& GameI );	// return true if there is a mapping from device to pad
	bool GameToDevice( const GameInput &GameI, int iSlotNum, DeviceInput& DeviceI );	// return true if there is a mapping from pad to device

	MenuButton GameToMenu( const GameInput &GameI );
	void MenuToGame( MenuButton MenuI, PlayerNumber pn, GameInput GameIout[4] );
	PlayerNumber ControllerToPlayerNumber( GameController controller );

	float GetSecsHeld( const GameInput &GameI, MultiPlayer mp = MultiPlayer_Invalid );
	float GetSecsHeld( MenuButton MenuI, PlayerNumber pn );

	bool IsBeingPressed( const GameInput &GameI, MultiPlayer mp = MultiPlayer_Invalid, const DeviceInputList *pButtonState = NULL );
	bool IsBeingPressed( MenuButton MenuI, PlayerNumber pn );

	void ResetKeyRepeat( const GameInput &GameI );
	void ResetKeyRepeat( MenuButton MenuI, PlayerNumber pn );

	void RepeatStopKey( const GameInput &GameI );
	void RepeatStopKey( MenuButton MenuI, PlayerNumber pn );

	struct Mapping
	{
		bool IsEndMarker() const { return iSlotIndex==-1; }

		int iSlotIndex;	// -1 == end marker
		DeviceButton deviceButton;
		GameButton gb;

		/*
		 * If this is true, this is an auxilliary mapping assigned to the second
		 * player.  If two of the same device are found, and the device has secondary
		 * entries, the later entries take precedence.  This way, if a Pump pad is
		 * found, it'll map P1 to the primary pad and P2 to the secondary pad.
		 * (We can't tell if a slave pad is actually there.)  Then, if a second primary
		 * is found (DEVICE_PUMP2), 2P will be mapped to it. 
		 */
		bool SecondController;
	};

	static InputDevice MultiPlayerToInputDevice( MultiPlayer mp );
	static MultiPlayer InputDeviceToMultiPlayer( InputDevice id );

	void Unmap( InputDevice device );
	void ApplyMapping( const Mapping *maps, GameController gc, InputDevice device );

protected:
	// all the DeviceInputs that map to a GameInput
	DeviceInput m_GItoDI[NUM_GameController][NUM_GameButton][NUM_GAME_TO_DEVICE_SLOTS];

	void UpdateTempDItoGI();
	const InputScheme *m_pInputScheme;
};


extern InputMapper*	INPUTMAPPER;	// global and accessable from anywhere in our program


#endif

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
