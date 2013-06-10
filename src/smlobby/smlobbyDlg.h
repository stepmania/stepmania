// smlobbyDlg.h : header file
//

#if !defined(AFX_SmlobbyDLG_H__B8E7A228_8094_4516_894C_9E613B74B27F__INCLUDED_)
#define AFX_SmlobbyDLG_H__B8E7A228_8094_4516_894C_9E613B74B27F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "irc.h"
#include "EditChat.h"
#include <vector> 

using namespace irc;

/////////////////////////////////////////////////////////////////////////////
// CSmlobbyDlg dialog

class CSmlobbyDlg : public CDialog, public CIrcDefaultMonitor
{
// Construction
public:
	CSmlobbyDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSmlobbyDlg)
	enum { IDD = IDD_SMLOBBY_DIALOG };
	CButton	m_buttonStartGame;
	CStatic	m_staticGameName;
	CStatic	m_staticSelectMusic;
	CButton	m_frameNewGame;
	CButton	m_buttonCreateGame;
	CComboBox	m_comboMusic;
	CEdit	m_editGameName;
	CEdit	m_editGameInfo;
	CListBox	m_listUsers;
	CEditChat	m_editEntry;
	CEdit		m_editChatMessages;
	CListBox	m_listGames;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmlobbyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	bool m_bWantToJoin;

	// Generated message map functions
	//{{AFX_MSG(CSmlobbyDlg)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	virtual BOOL OnInitDialog();
	virtual void OnPaint();
	virtual void OnDestroy();
	afx_msg void OnDblclkListGames();
	afx_msg void OnButtonCreateGame();
	afx_msg void OnSelchangeListGames();
	afx_msg void OnButtonBeginGame();
	afx_msg void OnRefreshGameList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	bool OnIrc_YOURHOST(const CIrcMessage* pmsg);
	bool OnIrc_NICK(const CIrcMessage* pmsg);
	bool OnIrc_PRIVMSG(const CIrcMessage* pmsg);
	bool OnIrc_JOIN(const CIrcMessage* pmsg);
	bool OnIrc_PART(const CIrcMessage* pmsg);
	bool OnIrc_KICK(const CIrcMessage* pmsg);
	bool OnIrc_MODE(const CIrcMessage* pmsg);
	bool OnIrc_QUIT(const CIrcMessage *pmsg);
	bool OnIrc_RPL_LISTSTART(const CIrcMessage *pmsg);
	bool OnIrc_RPL_LIST(const CIrcMessage *pmsg);
	bool OnIrc_RPL_TOPIC(const CIrcMessage *pmsg);
	bool OnIrc_RPL_NAMREPLY(const CIrcMessage *pmsg);
	bool OnIrc_DCC_SEND(const CIrcMessage *pmsg);
	bool OnIrc_DCC_RECV(const CIrcMessage *pmsg);
	bool OnIrc_DDR_GAME_START(const CIrcMessage *pmsg);
	bool OnIrc_RPL_ENDOFMOTD(const CIrcMessage *pmsg);
	bool OnIrc_IgnoreMesg(const CIrcMessage *pmsg)		{ return true; }

	virtual void OnIrcDefault(const CIrcMessage* pmsg);
	virtual void OnIrcDisconnected();

	void UpdateChatMessages( const CIrcMessage* p );
	bool IsUniqueGameName(const CString GameName);
	int FindSongNameFromHash(unsigned long hash);
	CString SelectFolder();
	bool WinExec(String sCmdLine);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SmlobbyDLG_H__B8E7A228_8094_4516_894C_9E613B74B27F__INCLUDED_)
