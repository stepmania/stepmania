#pragma once
/*
-----------------------------------------------------------------------------
 Class: ScreenManager

 Desc: Manager/container for Screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageInput.h"
#include "Song.h"
#include "Notes.h"
#include "Screen.h"
#include "BitmapText.h"
#include "Quad.h"


class ScreenManager
{
public:
	ScreenManager();
	~ScreenManager();

	// pass these messages along to the current state
	void Restore();
	void Invalidate();
	void Update( float fDeltaTime );
	void Draw();
	void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );

	void SetNewScreen( Screen *pNewScreen );
	void AddScreenToTop( Screen *pNewScreen );
	void PopTopScreen( ScreenMessage SM_SendToNewTopScreen );

	void SystemMessage( CString sMessage );

	void SendMessageToTopScreen( ScreenMessage SM, float fDelay );

//	void OverrideCreditsMessage( PlayerNumber p, CString sNewString );
	void RefreshCreditsMessages();

private:
	CArray<Screen*, Screen*&> m_ScreenStack;
	CArray<Screen*, Screen*&> m_ScreensToDelete;

	BitmapText m_textStats;
	BitmapText m_textSystemMessage;
	BitmapText m_textCreditInfo[NUM_PLAYERS];

	void EmptyDeleteQueue();
};


extern ScreenManager*	SCREENMAN;	// global and accessable from anywhere in our program
