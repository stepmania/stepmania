/*
-----------------------------------------------------------------------------
 Class: ScreenMapControllers

 Desc: Where the player maps device input to instrument buttons.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "PrefsManager.h"
#include "GrayArrow.h"
#include "InputMapper.h"
#include "MenuElements.h"


class ScreenMapControllers : public Screen
{
public:
	ScreenMapControllers();
	virtual ~ScreenMapControllers();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	void KeyLeft();
	void KeyRight();
	void KeyUp();
	void KeyDown();
	void KeyBack();
	void KeyStart();

	void Refresh();


	int m_iCurController;
	int m_iCurButton;
	int m_iCurSlot;
	bool m_bWaitingForPress;

	BitmapText	m_textError;
	BitmapText	m_textName[MAX_GAME_BUTTONS];
	BitmapText	m_textName2[MAX_GAME_BUTTONS];
	BitmapText	m_textMappedTo[MAX_GAME_CONTROLLERS][MAX_GAME_BUTTONS][NUM_GAME_TO_DEVICE_SLOTS];

	MenuElements m_Menu;

};

