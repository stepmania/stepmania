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
#include "Transition.h"
#include "Quad.h"
#include "RandomSample.h"


#define MAX_MENU_ROWS  40


struct MenuRow
{
	const char *name;
	bool	enabled;
	int		defaultChoice;
	const char *choices[32];
};


struct MenuRowInternal
{
	CString name;
	bool    enabled;
	int		defaultChoice;
	vector<CString> choices;

	MenuRowInternal()
	{
		enabled = true;
		defaultChoice = 0;
	}

	MenuRowInternal( const MenuRow &r );
};


struct Menu
{
	CString title;
	vector<MenuRowInternal> rows;

	Menu() {}
	Menu( CString t, const MenuRow *rows );
};


class ScreenMiniMenu : public Screen
{
public:
	ScreenMiniMenu( CString sName );
	ScreenMiniMenu( Menu* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	void MenuUp( PlayerNumber pn, const InputEventType type );
	void MenuDown( PlayerNumber pn, const InputEventType type );
	void MenuLeft( PlayerNumber pn, const InputEventType type );
	void MenuRight( PlayerNumber pn, const InputEventType type );
	void MenuBack( PlayerNumber pn );
	void MenuStart( PlayerNumber pn, const InputEventType type );

	int GetGoUpSpot();		// return -1 if can't go up
	int GetGoDownSpot();	// return -1 if can't go down
	bool CanGoLeft();
	bool CanGoRight();


	void BeforeLineChanged();
	void AfterLineChanged();
	void AfterAnswerChanged();

	BGAnimation			m_Background;
	Menu				m_Def;
	BitmapText			m_textTitle;
	BitmapText			m_textLabel[MAX_MENU_ROWS];
	BitmapText			m_textAnswer[MAX_MENU_ROWS];
	int					m_iCurLine;
	int					m_iCurAnswers[MAX_MENU_ROWS];
	ScreenMessage		m_SMSendOnOK, m_SMSendOnCancel;
	Transition		m_In;
	Transition		m_Out;

public:
	static int	s_iLastLine;
	static int	s_iLastAnswers[MAX_MENU_ROWS];

};
