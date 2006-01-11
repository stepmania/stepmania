/* ControllerStateDisplay - Show the button state of a controller. */

#ifndef ControllerStateDisplay_H
#define ControllerStateDisplay_H

#include "ActorFrame.h"
#include "ActorUtil.h"
#include "PlayerNumber.h"
#include "Sprite.h"
#include "RageInput.h"

enum ControllerStateButton
{
	ControllerStateButton_Up,
	ControllerStateButton_Down,
	ControllerStateButton_Left,
	ControllerStateButton_Right,
	NUM_ControllerStateButton
};


class ControllerStateDisplay : public ActorFrame
{
public:
	ControllerStateDisplay();
	void Load( MultiPlayer mp );
	virtual void Update( float fDelta );
	bool IsLoaded() const { return m_bIsLoaded; }

protected:
	bool m_bIsLoaded;
	Sprite m_sprFrame;
	struct Button
	{
		Sprite spr;
		DeviceInput di;
	};
	Button m_Buttons[NUM_ControllerStateButton];
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
