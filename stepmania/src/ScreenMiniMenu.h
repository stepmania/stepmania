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


#define MAX_MENU_ROWS  20


struct MenuRow
{
	CString name;
	bool	enabled;
	int		defaultChoice;
	vector<CString> choices;

	MenuRow()
	{
		enabled = true;
		defaultChoice = 0;
	}

	MenuRow( CString n, bool e, CString c0="", CString c1="", CString c2="", CString c3="", CString c4="", CString c5="", CString c6="", CString c7="", CString c8="", CString c9="", CString c10="", CString c11="", CString c12="", CString c13="", CString c14="" )
	{
		name = n;
		enabled = e;
		defaultChoice = 0;
#define PUSH( c )	if(c!="") choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);
		printf( "choices.size = %u", choices.size() );
	}
};


struct Menu
{
	CString title;
	vector<MenuRow> rows;

	Menu() {}

	Menu( CString t, MenuRow r0, MenuRow r1=MenuRow(), MenuRow r2=MenuRow(), MenuRow r3=MenuRow(), MenuRow r4=MenuRow(), MenuRow r5=MenuRow(), MenuRow r6=MenuRow(), MenuRow r7=MenuRow(), MenuRow r8=MenuRow(), MenuRow r9=MenuRow(), MenuRow r10=MenuRow(), MenuRow r11=MenuRow(), MenuRow r12=MenuRow(), MenuRow r13=MenuRow(), MenuRow r14=MenuRow(), MenuRow r15=MenuRow(), MenuRow r16=MenuRow(), MenuRow r17=MenuRow(), MenuRow r18=MenuRow(), MenuRow r19=MenuRow() )
	{
		title = t;
#define PUSH2( r )	if(r.name!="" || !r.choices.empty()) rows.push_back(r);
		PUSH2(r0);PUSH2(r1);PUSH2(r2);PUSH2(r3);PUSH2(r4);PUSH2(r5);PUSH2(r6);PUSH2(r7);PUSH2(r8);PUSH2(r9);PUSH2(r10);PUSH2(r11);PUSH2(r12);PUSH2(r13);PUSH2(r14);PUSH2(r15);PUSH2(r16);PUSH2(r17);PUSH2(r18);PUSH2(r19);
	}
};


class ScreenMiniMenu : public Screen
{
public:
	ScreenMiniMenu();
	ScreenMiniMenu( Menu* pDef, ScreenMessage SM_SendOnOK, ScreenMessage SM_SendOnCancel );

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

	Menu				m_Def;
	TransitionFade		m_Fade;
	BitmapText			m_textTitle;
	BitmapText			m_textLabel[MAX_MENU_ROWS];
	BitmapText			m_textAnswer[MAX_MENU_ROWS];
	int					m_iCurLine;
	int					m_iCurAnswers[MAX_MENU_ROWS];
	ScreenMessage		m_SMSendOnOK, m_SMSendOnCancel;

public:
	static int	s_iLastLine;
	static int	s_iLastAnswers[MAX_MENU_ROWS];

};

