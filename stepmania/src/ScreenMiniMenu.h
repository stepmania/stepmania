/*
-----------------------------------------------------------------------------
 Class: ScreenMiniMenu

 Desc: Displays a prompt over the top of another screen.  Must use by calling
	SCREENMAN->AddScreenToTop( new ScreenMiniMenu(...) );

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "Quad.h"
#include "RandomSample.h"

#define MAX_MINI_MENU_LINES  40
#define MAX_OPTIONS_PER_LINE  20

struct MiniMenuDefinition 
{
	char szTitle[64];
	int iNumLines;
	struct MiniMenuLine
	{
		char szLabel[40];
		bool bEnabled;
		int iNumOptions;
		int iDefaultOption;
		char szOptionsText[MAX_OPTIONS_PER_LINE][256];
	} lines[MAX_MINI_MENU_LINES];
};


class ScreenMiniMenu : public Screen
{
public:
	ScreenMiniMenu();
	ScreenMiniMenu( MiniMenuDefinition* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );
	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );

	int GetGoUpSpot();		// return -1 if can't go up
	int GetGoDownSpot();	// return -1 if can't go down
	bool CanGoLeft();
	bool CanGoRight();


	void BeforeLineChanged();
	void AfterLineChanged();
	void AfterAnswerChanged();

	MiniMenuDefinition	m_Def;
	TransitionFade		m_Fade;
	BitmapText			m_textTitle;
	BitmapText			m_textLabel[MAX_MINI_MENU_LINES];
	BitmapText			m_textAnswer[MAX_MINI_MENU_LINES];
	int					m_iCurLine;
	int					m_iCurAnswers[MAX_MINI_MENU_LINES];
	ScreenMessage		m_SMSendOnOK, m_SMSendOnCancel;

public:
	static int	s_iLastLine;
	static int	s_iLastAnswers[MAX_MINI_MENU_LINES];

};

