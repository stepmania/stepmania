#ifndef ARCH_SETUP_WINDOWS_H
#define ARCH_SETUP_WINDOWS_H

/* Fix VC breakage. */
#define PATH_MAX _MAX_PATH

#if _MSC_VER < 1300 /* VC6, not VC7 */
#define NEED_MINMAX_TEMPLATES 1
#endif

/* Don't include windows.h everywhere; when we do eventually include it, use this: */
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#endif

#include <direct.h> /* has stuff that should be in unistd.h */
#include <wchar.h> /* needs to be included before our fixes below */

#define getcwd _getcwd
#define wgetcwd _wgetcwd
#define chdir _chdir
#define wchdir _wchdir
#define alloca _alloca
#define stat _stat
#define lstat _stat
/* mkdir is missing the mode arg */
#define mkdir(p,m) mkdir(p)


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

/* For RageLog */
#define HAVE_VERSION_INFO

#ifdef _XBOX
#include <xtl.h>
#include <xgraphics.h>
#include <stdio.h>
#endif

#endif

