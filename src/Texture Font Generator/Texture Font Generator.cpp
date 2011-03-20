// Texture Font Generator.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Texture Font Generator.h"
#include "Texture Font GeneratorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
 * TODO:
 *
 * allow importing existing fonts (open .ini)
 * allow selectively creating font pages
 * allow creating font page types
 * tag font page names; "rage numbers=0"
 * if only exporting one font page, don't include the name; "_times 16pt.png", not "_times 16pt [main].png"
 * include font properties in the default filename "_times 16pt bold [main].png"
 * allow selecting points or pixels; include this in the defualt filename: "16pt" "16px"
 * separate m_BoundingRect for each page
 *  - optimize: if we have a few big characters, put them in a separate font page, so we can
 *    pack the smaller ones tighter
 * "always expand to power of two" (enable or disable packing)?
 * "white on black" vs. "opaque on transparent"?
 */

BEGIN_MESSAGE_MAP(CTextureFontGeneratorApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


CTextureFontGeneratorApp::CTextureFontGeneratorApp()
{
}


// The one and only CTextureFontGeneratorApp object

CTextureFontGeneratorApp theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x9BFC6AC5, 0x5A9, 0x4468, { 0xB0, 0x9, 0xA9, 0xE6, 0xC2, 0x59, 0xDF, 0xD8 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


BOOL CTextureFontGeneratorApp::InitInstance()
{
	CWinApp::InitInstance();

	// Initialize OLE libraries
	if( !AfxOleInit() )
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Parse command line for automation or reg/unreg switches.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// App was launched with /Embedding or /Automation switch.
	// Run app as automation server.
	if( cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated )
	{
		// Register class factories via CoRegisterClassObject().
		COleTemplateServer::RegisterAll();
	}
	// App was launched with /Unregserver or /Unregister switch.  Remove
	// entries from the registry.
	else if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister)
	{
		COleObjectFactory::UpdateRegistryAll(FALSE);
		AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor);
		return FALSE;
	}
	// App was launched standalone or with other switches (e.g. /Register
	// or /Regserver).  Update registry entries, including typelibrary.
	else
	{
		COleObjectFactory::UpdateRegistryAll();
		AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid);
		if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppRegister)
			return FALSE;
	}

	m_hAccelerators = LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR) );

	CTextureFontGeneratorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CTextureFontGeneratorApp::ProcessMessageFilter( int code, LPMSG lpMsg )
{
	if( m_hAccelerators )
	{
		if (::TranslateAccelerator(m_pMainWnd->m_hWnd, m_hAccelerators, lpMsg)) 
			return(TRUE);
	}
	
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}


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
