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
#pragma warning (disable : 4512) // assignment operator could not be generated (so?)
/* "unreferenced formal parameter"; we *want* that in many cases */
#pragma warning (disable : 4100)
/* "case 'aaa' is not a valid value for switch of enum 'bbb'
 * Actually, this is a valid warning, but we do it all over the
 * place, eg. with ScreenMessages.  Those should be fixed, but later. XXX */
#pragma warning (disable : 4063)
#pragma warning (disable : 4127)
#endif

#undef min
#undef max
#define NOMINMAX /* make sure Windows doesn't try to define this */



/* Make sure everyone has min and max: */
#include <algorithm>

/* Everything will need string for one reason or another: */
#include <string>

/* And vector: */
#include <vector>

using namespace std;

/* Assertion that sets an optional message and brings up the crash handler, so
 * we get a backtrace.  This should probably be used instead of throwing an
 * exception in most cases we expect never to happen (but not in cases that
 * we do expect, such as d3d init failure.) */
#include "crash.h"
#define RAGE_ASSERT_M(COND, MESSAGE) { if(!(COND)) { VDCHECKPOINT_M(MESSAGE); *(char*)0=0; } }
#define RAGE_ASSERT(COND) RAGE_ASSERT_M((COND), "Assertion failure")

/* Make this the default assert handler. */
#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT RAGE_ASSERT

#if 1

#include "StdString.h"

/* Use CStdString: */
#define CString CStdString
#else

/* Wrapper to use getline() on MFC strings. */
template<class _E, class _Tr> inline
basic_istream<_E, _Tr>& getline(basic_istream<_E, _Tr> &I,
	CString &X) {
	string str;

	basic_istream<_E, _Tr> &ret = getline(I, str);
	X = str.c_str();
	return ret;
}

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

#endif

#endif

#if defined(_MSC_VER) && _MSC_VER  < 1300 /* VC6, not VC7 */

/* VC6's <algorithm> is doesn't actually define min and max. */
template<class T>
inline const T& max(const T &a, const T &b)
{ return a < b? b:a; }
template<class T, class P>
inline const T& max(const T &a, const T &b, P Pr)
{ return Pr(a, b)? b:a; }
template<class T>
inline const T& min(const T &a, const T &b)
{ return b < a? b:a; }
template<class T, class P>
inline const T& min(const T &a, const T &b, P Pr)
{ return Pr(b, a)? b:a; }

#endif

template <class Type, class IGNOREME>
class StdCArray: public std::vector<Type> {};
#define CArray StdCArray
#define CStringArray vector<CString>

/* Include this here to make sure our assertion handler is always
 * used.  (This file is a dependency of most everything anyway,
 * so there's no real problem putting it here.) */
#include "RageException.h"

/* Don't include our own headers here, since they tend to change
 * often. */

#endif
