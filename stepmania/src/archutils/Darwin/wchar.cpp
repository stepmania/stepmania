/*
 *  wchar.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Tue Nov 25 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

/*
 * Since I cannot use the <wchar.h> header beacuse Apple decided to be really
 * stupid when it released the old SDKs, I'm going to write really bad
 * implementations for a few functions since they are not actually used
 * anywhere in StepMania (I hope). If the need arises, I can write real
 * real functions for these.
 */

#define	_BSD_WCHAR_T_DEFINED_
#include <stddef.h>

#define CRASH(x) *(char*)0=0; return x
extern "C" {
#ifndef wcslen
size_t wcslen(const wchar_t *w) { CRASH(0); }
#endif

#ifndef wmemchr
wchar_t *wmemchr(const wchar_t *w1, wchar_t w2, size_t s) { CRASH(NULL); }
#endif

#ifndef wmemcmp
int wmemcmp(const wchar_t *w1, const wchar_t *w2, size_t s) { CRASH(0); }
#endif

#ifndef wmemcpy
wchar_t *wmemcpy(wchar_t *dst, const wchar_t *src, size_t s) { CRASH(NULL); }
#endif

#ifndef wmemmove
wchar_t *wmemmove(wchar_t *dst, const wchar_t *src, size_t s) { CRASH(NULL); }
#endif

#ifndef wmemset
wchar_t *wmemset(wchar_t *w1 , wchar_t w, size_t s) { CRASH(NULL); }
#endif
}
