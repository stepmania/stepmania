/* ScreenMapControllers2 - Maps device input to game buttons. */

#ifndef SCREEN_MAP_CONTROLLERS2_H
#define SCREEN_MAP_CONTROLLERS2_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "InputMapper.h"
#include "ActorScroller.h"
#include "RageSound.h"
#include "EnumHelper.h"
#include "ThemeMetric.h"

/* old inputmapper consts:
const int NUM_GAME_TO_DEVICE_SLOTS	= 5;	// five device inputs may map to one game input
const int NUM_SHOWN_GAME_TO_DEVICE_SLOTS = 3;
const int NUM_USER_GAME_TO_DEVICE_SLOTS = 2;
*/
const int MAPPING_LINE = NUM_SHOWN_GAME_TO_DEVICE_SLOTS+1; // +1 for "Add Mapping"

enum MapControlsState
{
	MCS_Button,		// which button to edit mappings for
	MCS_EditMappings,		// player is editing mappings
	NUM_MapControlsState,
	MapControlsState_Invalid
};
const RString& MapControlsStateToString( MapControlsState mct );
LuaDeclareType( MapControlsState );

class ScreenMapControllers2 : public ScreenWithMenuElements
{
public:
	ScreenMapControllers2();
	virtual void Init();
	virtual void BeginScreen();
	//virtual void EndScreen();

	virtual void Update( float fDeltaTime );
	virtual bool Input( const InputEventPlus &input );

	virtual bool MenuUp( const InputEventPlus &input );
	virtual bool MenuDown( const InputEventPlus &input );
	virtual bool MenuStart( const InputEventPlus &input );
	virtual void TweenOnScreen();

	MapControlsState GetMapControlsState(){ return m_MapControlsState; }
	GameButton GetEditingGameButton() { return m_EditingButton; }
	int GetInputIndex(){ return m_iInputChoice; }
	int GetNumInputs(){ return m_KeysToMap.size(); }
	int GetMappingIndex(PlayerNumber pn){ return m_iMappingChoice[pn]; }

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	virtual void HandleMessage( const Message &msg );

	void ChangeActivePlayer();
	void ChangeInputSelection(int iDir);
	void UpdateMappingScrollers();
	void Refresh();

	MapControlsState m_MapControlsState;
	RageTimer m_WaitingForPress;
	DeviceInput m_DeviceIToMap;

	struct KeyToMap
	{
		GameButton	m_GameButton;
		BitmapText*	m_textLabel;
	};
	vector<KeyToMap> m_KeysToMap;

	BitmapText m_textDevices;
	//vector<BitmapText> m_textMappingLabels[NUM_PLAYERS];
	BitmapText m_textAddMapping[NUM_PLAYERS];

	// possible inputs
	int m_iInputChoice;
	ActorScroller	m_InputScroller;
	ActorFrame	m_InputLine[NUM_GameButton];
	ThemeMetric<bool>	WRAP_INPUT_SCROLLER;
	ThemeMetric<bool>	LOOP_INPUT_SCROLLER;
	ThemeMetric<float>	INPUT_SCROLLER_SECONDS_PER_ITEM;
	ThemeMetric<float>	INPUT_SCROLLER_NUM_ITEMS_TO_DRAW;
	ThemeMetric<LuaReference> INPUT_SCROLLER_TRANSFORM;
	ThemeMetric<int>	INPUT_SCROLLER_SUBDIVISIONS;

	// per-player button mappings.
	GameButton m_EditingButton;
	PlayerNumber m_ActivePlayerMapping;
	int m_iMappingChoice[NUM_PLAYERS];
	int m_iNumMappingRows[NUM_PLAYERS];
	ActorScroller	m_MappingScroller[NUM_PLAYERS];
	ActorFrame	m_MappingLine[MAPPING_LINE];
	ThemeMetric<bool>	WRAP_MAPPING_SCROLLER;
	ThemeMetric<bool>	LOOP_MAPPING_SCROLLER;
	ThemeMetric<float>	MAPPING_SCROLLER_SECONDS_PER_ITEM;
	ThemeMetric<float>	MAPPING_SCROLLER_NUM_ITEMS_TO_DRAW;
	ThemeMetric<LuaReference> MAPPING_SCROLLER_TRANSFORM;
	ThemeMetric<int>	MAPPING_SCROLLER_SUBDIVISIONS;

	// should be handled in lua
	//BitmapText m_textLabel[NUM_GameController];

	RageSound m_soundChange;
	RageSound m_soundDelete;
};

#endif

/*
 * (c) 2011 AJ Kelly
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
