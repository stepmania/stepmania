#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: ScreenManager

 Desc: Manager/container for Screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageInputDevice.h"
#include "song.h"
#include "Notes.h"
#include "Screen.h"
#include "BitmapText.h"
#include "Quad.h"


struct MiniMenuDefinition;

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

	void PrepNewScreen( CString sClassName );
	void LoadPreppedScreen();
	void SetNewScreen( CString sClassName );
	void Prompt( ScreenMessage SM_SendWhenDone, CString sText, bool bYesNo = false, bool bDefaultAnswer = false, void(*OnYes)() = NULL, void(*OnNo)() = NULL );
	void TextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer) = NULL, void(*OnCanel)() = NULL );
	void MiniMenu( MiniMenuDefinition* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel = SM_None );
	void PopTopScreen( ScreenMessage SM );
	void SystemMessage( CString sMessage );

	void SendMessageToTopScreen( ScreenMessage SM, float fDelay );

	void RefreshCreditsMessages();

private:
	vector<Screen*> m_ScreenStack;	// bottommost to topmost
	vector<Screen*> m_ScreensToDelete;
	Screen *m_ScreenBuffered;

	BitmapText m_textStats;
	BitmapText m_textSystemMessage;
	BitmapText m_textCreditInfo[NUM_PLAYERS];

	Screen* MakeNewScreen( CString sClassName );
	void EmptyDeleteQueue();
	void SetNewScreen( Screen *pNewScreen );
};


extern ScreenManager*	SCREENMAN;	// global and accessable from anywhere in our program

#endif
