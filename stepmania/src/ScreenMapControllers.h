/* ScreenMapControllers - Maps device input to instrument buttons. */

#ifndef SCREEN_MAP_CONTROLLERS_H
#define SCREEN_MAP_CONTROLLERS_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "InputMapper.h"
#include "ActorScroller.h"
#include "RageSound.h"

enum { NUM_SHOWN_GAME_TO_DEVICE_SLOTS = 3 };

class ScreenMapControllers : public ScreenWithMenuElements
{
public:
	ScreenMapControllers( CString sName );
	virtual void Init();
	virtual void BeginScreen();

	virtual void Update( float fDeltaTime );
	virtual void Input( const InputEventPlus &input );

	virtual void TweenOffScreen();

private:
	void BeforeChangeFocus();
	void AfterChangeFocus();
	virtual bool GenericTweenOn() const { return true; }
	void Refresh();
	
	int m_iCurController;
	int m_iCurButton;
	int m_iCurSlot;

	RageTimer m_WaitingForPress;
	DeviceInput m_DeviceIToMap;

	struct KeyToMap
	{
		GameButton m_GameButton;

		// owned by m_Line
		BitmapText	*m_textMappedTo[MAX_GAME_CONTROLLERS][NUM_SHOWN_GAME_TO_DEVICE_SLOTS];
	};
	vector<KeyToMap> m_KeysToMap;

	ActorFrame	m_Line[MAX_GAME_BUTTONS];
	ActorScroller m_LineScroller;

	RageSound m_soundChange;
	RageSound m_soundDelete;
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
