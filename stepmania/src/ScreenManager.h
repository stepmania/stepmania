#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H
/*
-----------------------------------------------------------------------------
 Class: ScreenManager

 Desc: Manager/container for Screens.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/


#include "RageInputDevice.h"
#include "ScreenMessage.h"
#include "InputFilter.h"
#include "GameInput.h"
#include "MenuInput.h"
#include "StyleInput.h"
class Screen;
struct Menu;
class ScreenSystemLayer;


class ScreenManager
{
public:
	ScreenManager();
	~ScreenManager();

	// pass these messages along to the current state
	void Update( float fDeltaTime );
	void Draw();
	void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );

	void PrepNewScreen( CString sClassName );
	void LoadPreppedScreen();
	void DeletePreppedScreen();
	void SetNewScreen( CString sClassName );
	void AddNewScreenToTop( CString sClassName );
	void Prompt( ScreenMessage SM_SendWhenDone, CString sText, bool bYesNo = false, bool bDefaultAnswer = false, void(*OnYes)() = NULL, void(*OnNo)() = NULL );
	void TextEntry( ScreenMessage SM_SendWhenDone, CString sQuestion, CString sInitialAnswer, void(*OnOK)(CString sAnswer) = NULL, void(*OnCanel)() = NULL );
	void MiniMenu( Menu* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel = SM_None );
	void PopTopScreen( ScreenMessage SM = SM_RegainingFocus );
	void SystemMessage( CString sMessage );

	void PostMessageToTopScreen( ScreenMessage SM, float fDelay );
	void SendMessageToTopScreen( ScreenMessage SM );

	void RefreshCreditsMessages();

private:
	vector<Screen*> m_ScreenStack;	// bottommost to topmost
	vector<Screen*> m_ScreensToDelete;
	Screen *m_ScreenBuffered;
	ScreenSystemLayer *m_SystemLayer;

	Screen* MakeNewScreen( CString sClassName );
	void EmptyDeleteQueue();
	void SetNewScreen( Screen *pNewScreen );
};


extern ScreenManager*	SCREENMAN;	// global and accessable from anywhere in our program

#endif
