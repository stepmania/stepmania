/*
-----------------------------------------------------------------------------
 File: stdafx.h

 Desc: Include file for standard system include files.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef STDAFX_H
#define STDAFX_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff

#if defined(_MSC_VER) && (_MSC_VER > 1100)
#pragma warning (disable : 4786) // turn off broken debugger warning
/* "unreferenced formal parameter"; we *want* that in many cases */
#pragma warning (disable : 4100)
/* "case 'aaa' is not a valid value for switch of enum 'bbb'
 * Actually, this is a valid warning, but we do it all over the
 * place, eg. with ScreenMessages.  Those should be fixed, but later. XXX */
#pragma warning (disable : 4063)
#pragma warning (disable : 4127)
#endif

#include <afxwin.h>         // MFC core and standard components

#include <d3d8.h>
#include <d3dx8math.h>

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION  0x0800
#endif
#include <dinput.h>

#include "STDCarray.h"

#if 0
#include "StdString.h"

/* Use CStdString: */
#define CString CStdString
#define CStringArray StdCArray<CString,CString>
#else

/* Arg.  VC7's CString has GetString(), an equivalent of c_str().
 * VC6 doesn't have that.  We need it to transition to std::string
 * sanely.  So, sneakily add it.  This goes away when we finish
 * transitioning. */
#if _MSC_VER < 1300 /* VC6, not VC7 */
class CStringTemp: public CString {
public:
	CStringTemp() {}
	CStringTemp(const char *s): CString(s) { }
	CStringTemp(const CString &s): CString(s) {}

	const char *GetString() const { return (const char *) *this; }
};
#define CString CStringTemp
#define CStringArray CArray<CStringTemp,CStringTemp>
#endif

#endif

/* Include this here to make sure our assertion handler is always
 * used.  (This file is a dependency of most everything anyway,
 * so there's no real problem putting it here.) */
#include "RageException.h"

/* Don't include our own headers here, since they tend to change
 * often. */

#endif
