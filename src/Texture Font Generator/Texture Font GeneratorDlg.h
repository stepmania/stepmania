#pragma once
#include <afxwin.h>
#include <afxcmn.h>

#include "TextureFont.h"


class CTextureFontGeneratorDlg : public CDialog
{
	DECLARE_DYNAMIC(CTextureFontGeneratorDlg);

public:
	CTextureFontGeneratorDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CTextureFontGeneratorDlg();

	enum { IDD = IDD_TEXTUREFONTGENERATOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HICON m_hIcon;
	CFont m_Font;

	BOOL CanExit();

	bool m_bUpdateFontNeeded;
	bool m_bUpdateFontViewAndCloseUpNeeded;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	DECLARE_MESSAGE_MAP()

	void UpdateFontViewAndCloseUp();
	void UpdateFont( bool bSavingDoubleRes );
	void UpdateCloseUp();

public:
	vector<FontPageDescription> m_PagesToGenerate;

	afx_msg void OnCbnSelchangeShownPage();
	afx_msg void OnCbnSelchangeFamilyList();
	afx_msg void OnEnChangeFontSize();
	afx_msg void OnStyleAntialiased();
	afx_msg void OnStyleBold();
	afx_msg void OnStyleItalic();
	afx_msg void OnDeltaposSpinTop(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinBaseline(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangePadding();
	afx_msg void OnFileSave();
	afx_msg void OnFileExit();

	static int CALLBACK EnumFontFamiliesCallback( const LOGFONTA *pLogicalFontData, const TEXTMETRICA *pPhysicalFontData, DWORD FontType, LPARAM lParam );

	CStatic m_FontView;
	CComboBox m_ShownPage;
	CComboBox m_FamilyList;
	CStatic m_TextOverlap;
	CEdit m_FontSize;
	CEdit m_Padding;
	CStatic m_ErrorOrWarning;
	CStatic m_CloseUp;
	CSpinButtonCtrl m_SpinTop;
	CSpinButtonCtrl m_SpinBaseline;
	CStatic m_FontType;
	afx_msg void OnOptionsDoubleres();
	afx_msg void OnOptionsExportstroketemplates();
	afx_msg void OnOptionsNumbersonly();
};

/*
 * Copyright (c) 2003-2007 Glenn Maynard
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
