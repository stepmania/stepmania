/* ControllerStateDisplay - Show the button state of a controller. */

#ifndef ControllerStateDisplay_H
#define ControllerStateDisplay_H

#include "ActorFrame.h"
#include "ActorUtil.h"
#include "PlayerNumber.h"
#include "AutoActor.h"
#include "RageInput.h"
#include "GameInput.h"

enum ControllerStateButton
{
	ControllerStateButton_UpLeft,
	ControllerStateButton_UpRight,
	ControllerStateButton_Center,
	ControllerStateButton_DownLeft,
	ControllerStateButton_DownRight,
	NUM_ControllerStateButton
};

/*
enum ControllerDanceButton
{
	ControllerDanceButton_Up,
	ControllerDanceButton_Down,
	ControllerDanceButton_Left,
	ControllerDanceButton_Right,
	ControllerPumpButton_UpLeft,
	ControllerPumpButton_UpRight,
	NUM_ControllerDanceButton
};

enum ControllerPumpButton
{
	ControllerPumpButton_UpLeft,
	ControllerPumpButton_UpRight,
	ControllerPumpButton_Center,
	ControllerPumpButton_DownLeft,
	ControllerPumpButton_DownRight,
	NUM_ControllerPumpButton
};

enum ControllerKB7Button
{
	ControllerKB7Button_Key1,
	ControllerKB7Button_Key2,
	ControllerKB7Button_Key3,
	ControllerKB7Button_Key4,
	ControllerKB7Button_Key5,
	ControllerKB7Button_Key6,
	ControllerKB7Button_Key7,
	NUM_ControllerKB7Button
};
*/


class ControllerStateDisplay : public ActorFrame
{
public:
	ControllerStateDisplay();
	void LoadMultiPlayer( RString sType, MultiPlayer mp );
	void LoadGameController( RString sType, GameController gc );
	virtual void Update( float fDelta );
	bool IsLoaded() const { return m_bIsLoaded; }

	virtual ControllerStateDisplay *Copy() const;

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	void LoadInternal( RString sType, MultiPlayer mp, GameController gc );
	MultiPlayer m_mp;

	bool m_bIsLoaded;
	AutoActor m_sprFrame;
	struct Button
	{
		Button()
		{
			di.MakeInvalid();
			gi.MakeInvalid();
		}

		AutoActor spr;
		DeviceInput di;
		GameInput gi;
	};
	Button m_Buttons[NUM_ControllerStateButton];

	InputDeviceState m_idsLast;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
