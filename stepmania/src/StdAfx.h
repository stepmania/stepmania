/*
-----------------------------------------------------------------------------
 File: stdafx.h

 Desc: Include file for standard system include files.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#if !defined(AFX_STDAFX_H__9FF379EB_FAE2_11D1_BFC5_D41F722B624A__INCLUDED_)
#define AFX_STDAFX_H__9FF379EB_FAE2_11D1_BFC5_D41F722B624A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Screens headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxtempl.h>		// MFC templated collections
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Screens Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//
// Rage global classes
//
#include "RageLog.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"
#include "RageSound.h"
#include "RageMusic.h"
#include "RageInput.h"
#include "RageTimer.h"
#include "RageException.h"



//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9FF379EB_FAE2_11D1_BFC5_D41F722B624A__INCLUDED_)
