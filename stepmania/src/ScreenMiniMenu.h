/* ScreenMiniMenu - Displays a simple menu over the top of another screen. */

#ifndef SCREEN_MINI_MENU_H
#define SCREEN_MINI_MENU_H

#include "ScreenOptions.h"

typedef bool (*MenuRowUpdateEnabled)();

struct MenuRowDef
{
	int				iRowCode;
	CString			sName;
	bool			bEnabled;
	MenuRowUpdateEnabled pfnEnabled;	// if ! NULL, used to fill bEnabled
	EditMode		emShowIn;
	int				iDefaultChoice;
	vector<CString> choices;

	MenuRowDef() {}
	MenuRowDef( int r, CString n, MenuRowUpdateEnabled pe, EditMode s, int d, const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL )
	{
		iRowCode = r; 
		sName = n; 
		bEnabled = true; 
		pfnEnabled = pe; 
		emShowIn = s; 
		iDefaultChoice = d;
#define PUSH( c )	if(c) choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}
	MenuRowDef( int r, CString n, bool e, EditMode s, int d, const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL )
	{
		iRowCode = r; 
		sName = n; 
		bEnabled = e; 
		pfnEnabled = NULL; 
		emShowIn = s; 
		iDefaultChoice = d;
#define PUSH( c )	if(c) choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}

	bool SetDefaultChoiceIfPresent( CString sChoice )
	{
		iDefaultChoice = 0;
		FOREACH_CONST( CString, choices, s )
		{
			if( sChoice == *s )
			{
				iDefaultChoice = s - choices.begin();
				return true;
			}
		}
		return false;
	}
};

struct MenuDef
{
	CString sClassName;
	vector<MenuRowDef> rows;

	MenuDef( CString c, MenuRowDef r0=MenuRowDef(), MenuRowDef r1=MenuRowDef(), MenuRowDef r2=MenuRowDef(), MenuRowDef r3=MenuRowDef(), MenuRowDef r4=MenuRowDef(), MenuRowDef r5=MenuRowDef(), MenuRowDef r6=MenuRowDef(), MenuRowDef r7=MenuRowDef(), MenuRowDef r8=MenuRowDef(), MenuRowDef r9=MenuRowDef(), MenuRowDef r10=MenuRowDef(), MenuRowDef r11=MenuRowDef(), MenuRowDef r12=MenuRowDef(), MenuRowDef r13=MenuRowDef(), MenuRowDef r14=MenuRowDef(), MenuRowDef r15=MenuRowDef(), MenuRowDef r16=MenuRowDef(), MenuRowDef r17=MenuRowDef(), MenuRowDef r18=MenuRowDef(), MenuRowDef r19=MenuRowDef(), MenuRowDef r20=MenuRowDef(), MenuRowDef r21=MenuRowDef(), MenuRowDef r22=MenuRowDef(), MenuRowDef r23=MenuRowDef(), MenuRowDef r24=MenuRowDef(), MenuRowDef r25=MenuRowDef(), MenuRowDef r26=MenuRowDef(), MenuRowDef r27=MenuRowDef(), MenuRowDef r28=MenuRowDef(), MenuRowDef r29=MenuRowDef() )
	{
		sClassName = c;
#define PUSH( r )	if(!r.sName.empty()) rows.push_back(r);
		PUSH(r0);PUSH(r1);PUSH(r2);PUSH(r3);PUSH(r4);PUSH(r5);PUSH(r6);PUSH(r7);PUSH(r8);PUSH(r9);PUSH(r10);PUSH(r11);PUSH(r12);PUSH(r13);PUSH(r14);PUSH(r15);PUSH(r16);PUSH(r17);PUSH(r18);PUSH(r19);PUSH(r20);PUSH(r21);PUSH(r22);PUSH(r23);PUSH(r24);PUSH(r25);PUSH(r26);PUSH(r27);PUSH(r28);PUSH(r29);
#undef PUSH
	}
};


class ScreenMiniMenu : public ScreenOptions
{
public:
	static void MiniMenu( MenuDef* pDef, ScreenMessage smSendOnOK, ScreenMessage smSendOnCancel = SM_None, float fX = 0, float fY = 0 );

	ScreenMiniMenu( CString sScreenClass );
	void LoadMenu( const MenuDef* pDef );
	void HandleScreenMessage( const ScreenMessage SM );

	void SetOKMessage( ScreenMessage SM_SendOnOK ) { m_SMSendOnOK = SM_SendOnOK; }
	void SetCancelMessage( ScreenMessage SM_SendOnCancel ) { m_SMSendOnCancel = SM_SendOnCancel; }

protected:
	virtual void OnChange( PlayerNumber pn );
	virtual void ImportOptions( int iRow, const vector<PlayerNumber> &vpns );
	virtual void ExportOptions( int iRow, const vector<PlayerNumber> &vpns );
	
	ScreenMessage		m_SMSendOnOK;
	ScreenMessage		m_SMSendOnCancel;

	vector<MenuRowDef> m_vMenuRows;

public:
	static bool s_bCancelled;
	static int	s_iLastRowCode;
	static vector<int>	s_viLastAnswers;
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
