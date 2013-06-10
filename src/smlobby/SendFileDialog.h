#if !defined(AFX_SENDFILEDIALOG_H__6AA4B205_EDA8_4960_9424_1C5743093120__INCLUDED_)
#define AFX_SENDFILEDIALOG_H__6AA4B205_EDA8_4960_9424_1C5743093120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendFileDialog.h : header file
//

#include "resource2.h"

/////////////////////////////////////////////////////////////////////////////
// CSendFileDialog dialog

class CSendFileDialog : public CDialog
{
// Public data types
public:
	// DCC Info
	struct DCCTransferInfo
	{
		bool m_bIsSender;
		bool m_bIsConnected;
		CString m_fileName;
		CString m_directory;
		CString m_partnerName;
		unsigned long m_ulPartnerIP;
		unsigned int m_uiPort;
		unsigned long m_ulFileSize;
		unsigned long m_ulBytesSent;
		unsigned long m_ulStartTime;
		unsigned int m_uiXferRate;
		Socket m_sock;
		FILE* m_fp;

		DCCTransferInfo() : 
			m_bIsSender(false),
			m_bIsConnected(false),
			m_ulPartnerIP(0L),
			m_uiPort(0),
			m_ulFileSize(0L),
			m_ulBytesSent(0L),
			m_ulStartTime(0L),
			m_uiXferRate(0),
			m_fp(NULL)
		{}
	};

// Construction
public:
	// standard constructor
	CSendFileDialog(DCCTransferInfo dccinfo, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CSendFileDialog)
	enum { IDD = IDD_FILEXFER };
	CStatic	m_XferStatus;
	CStatic	m_XferRate;
	CStatic	m_ToFrom;
	CStatic	m_TimeLeft;
	CStatic	m_SentRecvd;
	CStatic	m_RecvrName;
	CStatic	m_FolderName;
	CStatic	m_Filesize;
	CStatic	m_FileName;
	CStatic	m_BytesSent;
	CProgressCtrl	m_ProgressFile;
	//}}AFX_DATA

	DCCTransferInfo m_dccInfo;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendFileDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	int Setup();
	static unsigned short MakePortReservation();
	static void AddTransferDialog(CSendFileDialog *ptr);
	static void FreeAnyCompletedTransfers();
	
protected:
	void FreePort(unsigned short port);
	int SetupSend();
	int SetupRecv();
	void SendSomeData();
	void RecvSomeData();

protected:
	static std::vector<HANDLE> m_hThread;
	static std::vector<unsigned short> m_usedPorts;
	static std::vector<CSendFileDialog *> m_transferDialogs;

	static const unsigned short kFirstPort;
	static const unsigned short kLastPort;

protected:
	
	// Generated message map functions
	//{{AFX_MSG(CSendFileDialog)
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDFILEDIALOG_H__6AA4B205_EDA8_4960_9424_1C5743093120__INCLUDED_)
