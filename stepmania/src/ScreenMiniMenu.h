/* ScreenMiniMenu - Displays a prompt over the top of another screen.  Call SCREENMAN->AddScreenToTop( new ScreenMiniMenu(...) ); */

#ifndef SCREEN_MINI_MENU_H
#define SCREEN_MINI_MENU_H

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

	void SetDefaultChoiceIfPresent( const CString &s );
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

#endif

/*
 * (c) 2003-2004 Chris Danford
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
