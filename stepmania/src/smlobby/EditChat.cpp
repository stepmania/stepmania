// EditChat.cpp : implementation file
//

#include "stdafx.h"
#include "smlobby.h"
#include "EditChat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//external object
extern irc::CIrcSession g_ircSession;

/////////////////////////////////////////////////////////////////////////////
// CEditChat

CEditChat::CEditChat()
{
}

CEditChat::~CEditChat()
{
}


BEGIN_MESSAGE_MAP(CEditChat, CEdit)
	//{{AFX_MSG_MAP(CEditChat)
	ON_WM_GETDLGCODE()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditChat message handlers

UINT CEditChat::OnGetDlgCode() 
{
	UINT code = CEdit::OnGetDlgCode(); 

	code |= DLGC_WANTMESSAGE; 

	return code; 
}

void CEditChat::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
		//User presses the return key
		case VK_RETURN:
			SendChatMesg();
			break;
	}
	
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CEditChat::SendChatMesg()
{
	CString s, name;
	bool isIncoming = false;

	this->GetWindowText(s);

	if( s.GetLength() == 0 ) return;

	if( s[0] != '/' )
	{
		//Make sure were in a chat room
		if( g_ircSession.GetInfo().sCurrentChatRoom.length() == 0)
		{
			s = "You are not currently in a chat room";
			name = g_ircSession.GetInfo().sNick.c_str();
			isIncoming = true;
		}
		else
		{
			name = g_ircSession.GetInfo().sCurrentChatRoom.c_str();
		}

		s = "PRIVMSG " + name + " :" + s;
	}
	else
	{
		s = s.Mid(1);
	}

	if( g_ircSession )
		g_ircSession << irc::CIrcMessage(s, isIncoming);

	this->SetWindowText(_T(""));
}