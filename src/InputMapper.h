#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

#include "RageInputDevice.h"
#include "GameInput.h"
#include "PlayerNumber.h"

#include <vector>


struct Game;

const int NUM_GAME_TO_DEVICE_SLOTS	= 5;	// five device inputs may map to one game input
const int NUM_SHOWN_GAME_TO_DEVICE_SLOTS = 3;
const int NUM_USER_GAME_TO_DEVICE_SLOTS = 2;
extern const RString DEVICE_INPUT_SEPARATOR;

struct AutoMappingEntry
{
	AutoMappingEntry( int i, DeviceButton db, GameButton gb, bool b ):
		m_iSlotIndex(i), m_deviceButton(db),
		m_gb(gb), m_bSecondController(b) {}
	AutoMappingEntry(): m_iSlotIndex(-1),
		m_deviceButton(DeviceButton_Invalid), m_gb(GameButton_Invalid),
		m_bSecondController(false) {}
	bool IsEmpty() const { return m_deviceButton == DeviceButton_Invalid  &&  m_gb == GameButton_Invalid; }

	int m_iSlotIndex;
	DeviceButton m_deviceButton;
	GameButton m_gb; // GameButton_Invalid means unmap this button
	/* If this is true, this is an auxilliary mapping assigned to the second
	 * player. If two of the same device are found, and the device has secondary
	 * entries, the later entries take precedence. This way, if a Pump pad is
	 * found, it'll map P1 to the primary pad and P2 to the secondary pad. (We
	 * can't tell if a slave pad is actually there.) Then, if a second primary
	 * is found (DEVICE_PUMP2), 2P will be mapped to it. */
	bool m_bSecondController;
};

struct AutoMappings
{
	AutoMappings(
		RString s1,
		RString s2,
		RString s3,
		AutoMappingEntry im0 = AutoMappingEntry(),
		AutoMappingEntry im1 = AutoMappingEntry(),
		AutoMappingEntry im2 = AutoMappingEntry(),
		AutoMappingEntry im3 = AutoMappingEntry(),
		AutoMappingEntry im4 = AutoMappingEntry(),
		AutoMappingEntry im5 = AutoMappingEntry(),
		AutoMappingEntry im6 = AutoMappingEntry(),
		AutoMappingEntry im7 = AutoMappingEntry(),
		AutoMappingEntry im8 = AutoMappingEntry(),
		AutoMappingEntry im9 = AutoMappingEntry(),
		AutoMappingEntry im10 = AutoMappingEntry(),
		AutoMappingEntry im11 = AutoMappingEntry(),
		AutoMappingEntry im12 = AutoMappingEntry(),
		AutoMappingEntry im13 = AutoMappingEntry(),
		AutoMappingEntry im14 = AutoMappingEntry(),
		AutoMappingEntry im15 = AutoMappingEntry(),
		AutoMappingEntry im16 = AutoMappingEntry(),
		AutoMappingEntry im17 = AutoMappingEntry(),
		AutoMappingEntry im18 = AutoMappingEntry(),
		AutoMappingEntry im19 = AutoMappingEntry(),
		AutoMappingEntry im20 = AutoMappingEntry(),
		AutoMappingEntry im21 = AutoMappingEntry(),
		AutoMappingEntry im22 = AutoMappingEntry(),
		AutoMappingEntry im23 = AutoMappingEntry(),
		AutoMappingEntry im24 = AutoMappingEntry(),
		AutoMappingEntry im25 = AutoMappingEntry(),
		AutoMappingEntry im26 = AutoMappingEntry(),
		AutoMappingEntry im27 = AutoMappingEntry(),
		AutoMappingEntry im28 = AutoMappingEntry(),
		AutoMappingEntry im29 = AutoMappingEntry(),
		AutoMappingEntry im30 = AutoMappingEntry(),
		AutoMappingEntry im31 = AutoMappingEntry(),
		AutoMappingEntry im32 = AutoMappingEntry(),
		AutoMappingEntry im33 = AutoMappingEntry(),
		AutoMappingEntry im34 = AutoMappingEntry(),
		AutoMappingEntry im35 = AutoMappingEntry(),
		AutoMappingEntry im36 = AutoMappingEntry(),
		AutoMappingEntry im37 = AutoMappingEntry(),
		AutoMappingEntry im38 = AutoMappingEntry(),
		AutoMappingEntry im39 = AutoMappingEntry() )
	: m_sGame(s1), m_sDriverRegex(s2), m_sControllerName(s3), m_vMaps()
	{
#define PUSH( im )	if(!im.IsEmpty()) m_vMaps.push_back(im);
		PUSH(im0);PUSH(im1);PUSH(im2);PUSH(im3);PUSH(im4);PUSH(im5);PUSH(im6);PUSH(im7);PUSH(im8);PUSH(im9);PUSH(im10);PUSH(im11);PUSH(im12);PUSH(im13);PUSH(im14);PUSH(im15);PUSH(im16);PUSH(im17);PUSH(im18);PUSH(im19);
		PUSH(im20);PUSH(im21);PUSH(im22);PUSH(im23);PUSH(im24);PUSH(im25);PUSH(im26);PUSH(im27);PUSH(im28);PUSH(im29);PUSH(im30);PUSH(im31);PUSH(im32);PUSH(im33);PUSH(im34);PUSH(im35);PUSH(im36);PUSH(im37);PUSH(im38);PUSH(im39);
#undef PUSH
	}

	// Strings used by automatic joystick mappings.
	RString m_sGame;			// only used
	RString m_sDriverRegex;		// reported by InputHandler
	RString m_sControllerName;	// the product name of the controller

	std::vector<AutoMappingEntry> m_vMaps;
};

class InputScheme
{
public:
	const char	*m_szName;
	int		m_iButtonsPerController;
	struct GameButtonInfo
	{
		const char	*m_szName;	// The name used by the button graphics system.  e.g. "left", "right", "middle C", "snare"
		GameButton	m_SecondaryMenuButton;
	};
	// Data for each Game-specific GameButton. This starts at GAME_BUTTON_NEXT.
	GameButtonInfo m_GameButtonInfo[NUM_GameButton];
	const AutoMappings *m_pAutoMappings;

	GameButton ButtonNameToIndex( const RString &sButtonName ) const;
	GameButton GameButtonToMenuButton( GameButton gb ) const;
	void MenuButtonToGameInputs( GameButton MenuI, PlayerNumber pn, std::vector<GameInput> &GameIout ) const;
	void MenuButtonToGameButtons( GameButton MenuI, std::vector<GameButton> &aGameButtons ) const;
	const GameButtonInfo *GetGameButtonInfo( GameButton gb ) const;
	const char *GetGameButtonName( GameButton gb ) const;
};
/** @brief A special foreach loop to handle the various GameButtons. */
#define FOREACH_GameButtonInScheme( s, var )	for( GameButton var=(GameButton)0; var<s->m_iButtonsPerController; enum_add<GameButton>( var, +1 ) )

class InputMappings
{
public:
	// only filled for automappings
	RString m_sDeviceRegex;
	RString m_sDescription;

	// map from a GameInput to multiple DeviceInputs
	DeviceInput m_GItoDI[NUM_GameController][NUM_GameButton][NUM_GAME_TO_DEVICE_SLOTS];

	void Clear();
	void Unmap( InputDevice id );
	void WriteMappings( const InputScheme *pInputScheme, RString sFilePath );
	void ReadMappings( const InputScheme *pInputScheme, RString sFilePath, bool bIsAutoMapping );
	void SetInputMap( const DeviceInput &DeviceI, const GameInput &GameI, int iSlotIndex );

	void ClearFromInputMap( const DeviceInput &DeviceI );
	bool ClearFromInputMap( const GameInput &GameI, int iSlotIndex );
};
/** @brief Holds user-chosen input preferences and saves it between sessions. */
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
	void ResetMappingsToDefault();
	void CheckButtonAndAddToReason(GameButton menu, std::vector<RString>& full_reason, RString const& sub_reason);
	void SanityCheckMappings(std::vector<RString>& reason);

	void ClearAllMappings();

	void SetInputMap( const DeviceInput &DeviceI, const GameInput &GameI, int iSlotIndex );
	void ClearFromInputMap( const DeviceInput &DeviceI );
	bool ClearFromInputMap( const GameInput &GameI, int iSlotIndex );

	void AddDefaultMappingsForCurrentGameIfUnmapped();
	void AutoMapJoysticksForCurrentGame();
	bool CheckForChangedInputDevicesAndRemap( RString &sMessageOut );

	bool IsMapped( const DeviceInput &DeviceI ) const;

	bool DeviceToGame( const DeviceInput &DeviceI, GameInput& GameI ) const;	// return true if there is a mapping from device to pad
	bool GameToDevice( const GameInput &GameI, int iSlotNum, DeviceInput& DeviceI ) const;	// return true if there is a mapping from pad to device

	GameButton GameButtonToMenuButton( GameButton gb ) const;
	void MenuToGame( GameButton MenuI, PlayerNumber pn, std::vector<GameInput> &GameIout ) const;
	PlayerNumber ControllerToPlayerNumber( GameController controller ) const;

	float GetSecsHeld( const GameInput &GameI, MultiPlayer mp = MultiPlayer_Invalid ) const;
	float GetSecsHeld( GameButton MenuI, PlayerNumber pn ) const;

	bool IsBeingPressed( const GameInput &GameI, MultiPlayer mp = MultiPlayer_Invalid, const DeviceInputList *pButtonState = nullptr ) const;
	bool IsBeingPressed( GameButton MenuI, PlayerNumber pn ) const;
	bool IsBeingPressed(const std::vector<GameInput>& GameI, MultiPlayer mp = MultiPlayer_Invalid, const DeviceInputList *pButtonState = nullptr ) const;

	void ResetKeyRepeat( const GameInput &GameI );
	void ResetKeyRepeat( GameButton MenuI, PlayerNumber pn );

	void RepeatStopKey( const GameInput &GameI );
	void RepeatStopKey( GameButton MenuI, PlayerNumber pn );

	float GetLevel( const GameInput &GameI ) const;
	float GetLevel( GameButton MenuI, PlayerNumber pn ) const;

	static InputDevice MultiPlayerToInputDevice( MultiPlayer mp );
	static MultiPlayer InputDeviceToMultiPlayer( InputDevice id );

	void Unmap( InputDevice device );
	void ApplyMapping( const std::vector<AutoMappingEntry> &vMmaps, GameController gc, InputDevice id );

protected:
	InputMappings m_mappings;

	void UpdateTempDItoGI();
	const InputScheme *m_pInputScheme;

private:
	InputMapper(const InputMapper& rhs);
	InputMapper& operator=(const InputMapper& rhs);
};

extern InputMapper*	INPUTMAPPER;	// global and accessible from anywhere in our program

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
