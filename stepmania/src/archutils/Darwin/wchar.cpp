/*
 *  wchar.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Tue Nov 25 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#define	_BSD_WCHAR_T_DEFINED_
#include <stddef.h>
#include <string>
#include <cstring>


extern "C" {
#ifndef wcslen
size_t wcslen(const wchar_t *ws)
{
    size_t n = 0;
    
    while (*(ws++) != NULL)
        ++n;
    return n;
}
#endif

#ifndef wmemchr
wchar_t *wmemchr(const wchar_t *ws, wchar_t wc, size_t n)
{
    for (unsigned i=0; i<n; ++i, ++ws)
        if (*ws == wc)
            return (wchar_t *)ws;
    return NULL;
}
#endif

#ifndef wmemcmp
int wmemcmp(const wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    for (unsigned i=0; i<n; ++i, ++ws1, ++ws2)
        if (*ws1 != *ws2)
            return *ws1 - *ws2;
    return 0;
}
#endif

#ifndef wmemcpy
wchar_t *wmemcpy(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    return (wchar_t *)memcpy(ws1, ws2, n * sizeof(wchar_t));
}
#endif

#ifndef wmemmove
wchar_t *wmemmove(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    return (wchar_t *)memmove(ws1, ws2, n * sizeof(wchar_t));
}
#endif

#ifndef wmemset
wchar_t *wmemset(wchar_t *ws , wchar_t wc, size_t n)
{
    wchar_t *temp = ws;
    for (unsigned i=0; i<n; ++i, ++temp)
        *temp = wc;
    return ws;
}
#endif
}

#ifndef wstring
template class std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >;
#endif
