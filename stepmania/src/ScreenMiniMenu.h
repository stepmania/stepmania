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

	MenuRow( const char * n, bool e, const char * c0=NULL, const char * c1=NULL, const char * c2=NULL, const char * c3=NULL, const char * c4=NULL, const char * c5=NULL, const char * c6=NULL, const char * c7=NULL, const char * c8=NULL, const char * c9=NULL, const char * c10=NULL, const char * c11=NULL, const char * c12=NULL, const char * c13=NULL, const char * c14=NULL )
	{
		name = n;
		enabled = e;
		defaultChoice = 0;
#define PUSH( c )	if(c!=NULL) choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);
#undef PUSH
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
#define PUSH( r )	if(r.name!="" || !r.choices.empty()) rows.push_back(r);
		PUSH(r0);PUSH(r1);PUSH(r2);PUSH(r3);PUSH(r4);PUSH(r5);PUSH(r6);PUSH(r7);PUSH(r8);PUSH(r9);PUSH(r10);PUSH(r11);PUSH(r12);PUSH(r13);PUSH(r14);PUSH(r15);PUSH(r16);PUSH(r17);PUSH(r18);PUSH(r19);
#undef PUSH
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
