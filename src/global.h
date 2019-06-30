#ifndef GLOBAL_H
#define GLOBAL_H

#if defined(CMAKE_POWERED)
#include "config.hpp"
#elif defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/** @brief This macro is for INT8_MIN, etc. */
#define __STDC_LIMIT_MACROS
/** @brief This macro is for INT64_C, etc. */
#define __STDC_CONSTANT_MACROS

/* Platform-specific fixes. */
#if defined(WIN32)
#include "archutils/Win32/arch_setup.h"
#elif defined(PBBUILD)
#include "archutils/Darwin/arch_setup.h"
#elif defined(UNIX)
#include "archutils/Unix/arch_setup.h"
#endif

/* Make sure everyone has min and max: */
#include <algorithm>

/* Everything will need string for one reason or another: */
#include <string>

/* And vector: */
#include <vector>

#if defined(HAVE_STDINT_H) /* need to define int64_t if so */
#include <stdint.h>
#endif
#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif

/* Branch optimizations: */
#if defined(__GNUC__)
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#if defined(NEED_CSTDLIB_WORKAROUND)
#define llabs ::llabs
#endif

using namespace std;

#ifdef ASSERT
#undef ASSERT
#endif

/** @brief RageThreads defines (don't pull in all of RageThreads.h here) */
namespace Checkpoints
{
	void SetCheckpoint( const char *file, int line, const char *message );
}
/** @brief Set a checkpoint with no message. */
#define CHECKPOINT (Checkpoints::SetCheckpoint(__FILE__, __LINE__, nullptr))
/** @brief Set a checkpoint with a specified message. */
#define CHECKPOINT_M(m) (Checkpoints::SetCheckpoint(__FILE__, __LINE__, m))


/**
 * @brief Define a macro to tell the compiler that a function doesn't return.
 *
 * This just improves compiler warnings.  This should be placed near the 
 * beginning of the function prototype (although it looks better near the end,
 * VC only accepts it at the beginning). */
#if defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#elif defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5))
#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif

/**
 * @brief A crash has occurred, and we're not getting out of it easily.
 *
 * For most users, this will result in a crashinfo.txt file being generated.
 * For anyone that is using a debug build, a debug break will be thrown to
 * allow viewing the current process.
 * @param reason the crash reason as determined by prior function calls.
 * @return nothing: there is no escape without quitting the program.
 */
void NORETURN sm_crash( const char *reason = "Internal error" );

/**
 * @brief Assertion that sets an optional message and brings up the crash 
 * handler, so we get a backtrace.
 * 
 * This should probably be used instead of throwing an exception in most 
 * cases we expect never to happen (but not in cases that we do expect, 
 * such as DSound init failure.) */
#define FAIL_M(MESSAGE) do { CHECKPOINT_M(MESSAGE); sm_crash(MESSAGE); } while(0)
#define ASSERT_M(COND, MESSAGE) do { if(unlikely(!(COND))) { FAIL_M(MESSAGE); } } while(0)


#if !defined(CO_EXIST_WITH_MFC)
#define ASSERT(COND) ASSERT_M((COND), "Assertion '" #COND "' failed")
#endif

/** @brief Use this to catch switching on invalid values */
#define DEFAULT_FAIL(i) 	default: FAIL_M( ssprintf("%s = %i", #i, (i)) )

void ShowWarningOrTrace( const char *file, int line, const char *message, bool bWarning ); // don't pull in LOG here
#define WARN(MESSAGE) (ShowWarningOrTrace(__FILE__, __LINE__, MESSAGE, true))
#if !defined(CO_EXIST_WITH_MFC)
#define TRACE(MESSAGE) (ShowWarningOrTrace(__FILE__, __LINE__, MESSAGE, false))
#endif

#ifdef DEBUG
// No reason to kill the program. A lot of these don't produce a crash in NDEBUG so why stop?
// TODO: These should have something you can hook a breakpoint on.
#define DEBUG_ASSERT_M(COND,MESSAGE) if(unlikely(!(COND))) WARN(MESSAGE)
#define DEBUG_ASSERT(COND) DEBUG_ASSERT_M(COND,"Debug assert failed")
#else
/** @brief A dummy define to keep things going smoothly. */
#define DEBUG_ASSERT(x)
/** @brief A dummy define to keep things going smoothly. */
#define DEBUG_ASSERT_M(x,y)
#endif

/* Use SM_UNIQUE_NAME to get the line number concatenated to x. This is useful for
 * generating unique identifiers in other macros.  */
#define SM_UNIQUE_NAME3(x,line) x##line
#define SM_UNIQUE_NAME2(x,line) SM_UNIQUE_NAME3(x, line)
#define SM_UNIQUE_NAME(x) SM_UNIQUE_NAME2(x, __LINE__)	

template <bool> struct CompileAssert;
template <> struct CompileAssert<true> { };
template<int> struct CompileAssertDecl { };
#define COMPILE_ASSERT(COND) typedef CompileAssertDecl< sizeof(CompileAssert<!!(COND)>) > CompileAssertInst

#include "StdString.h"
/** @brief Use RStrings throughout the program. */
typedef StdString::CStdString RString;

#include "RageException.h"

/* Define a few functions if necessary */
#include <cmath>

/* Don't include our own headers here, since they tend to change often. */

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
 * @section LICENSE
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
