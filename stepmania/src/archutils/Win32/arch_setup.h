#ifndef ARCH_SETUP_WINDOWS_H
#define ARCH_SETUP_WINDOWS_H

/* Fix VC breakage. */
#define PATH_MAX _MAX_PATH

#if _MSC_VER < 1300 /* VC6, not VC7 */
#define NEED_MINMAX_TEMPLATES 1
#endif

/* Don't include windows.h everywhere; when we do eventually include it, use this: */
#define WIN32_LEAN_AND_MEAN

/* If this isn't defined to 0, VC fails to define things like stat and alloca. */
#define __STDC__ 0

#include <direct.h> /* has stuff that should be in unistd.h */
#include <wchar.h> /* needs to be included before our fixes below */

#define lstat stat
#define fsync _commit
/* mkdir is missing the mode arg */
#define mkdir(p,m) mkdir(p)

/* Missing stdint types: */
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
static inline int64_t llabs( int64_t i ) { return i >= 0? i: -i; }

#pragma warning (disable : 4201) // nonstandard extension used : nameless struct/union (Windows headers do this)
#pragma warning (disable : 4786) // turn off broken debugger warning
#pragma warning (disable : 4512) // assignment operator could not be generated (so?)
/* "unreferenced formal parameter"; we *want* that in many cases */
#pragma warning (disable : 4100)
/* "case 'aaa' is not a valid value for switch of enum 'bbb'
 * Actually, this is a valid warning, but we do it all over the
 * place, eg. with ScreenMessages.  Those should be fixed, but later. XXX */
#pragma warning (disable : 4063)
#pragma warning (disable : 4127)
#pragma warning (disable : 4786) /* VC6: identifier was truncated to '255' characters in the debug information */

#undef min
#undef max
#define NOMINMAX /* make sure Windows doesn't try to define this */

/* Windows is missing some basic math functions: */
#define NEED_TRUNCF
#define NEED_ROUNDF
#define MISSING_STDINT_H

/* For RageLog */
#define HAVE_VERSION_INFO

#ifdef _XBOX
#include <xtl.h>
#include <xgraphics.h>
#include <stdio.h>
#endif

#endif

