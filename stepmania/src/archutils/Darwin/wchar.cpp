#include <bits/c++config.h>
#include <string>

extern "C"
{
#if ! _GLIBCPP_HAVE_WCLEN
size_t wcslen(const wchar_t *ws)
{
		size_t n = 0;
		
		while (*(ws++) != L'\0')
			++n;
		return n;
}
#endif

#if ! _GLIBCPP_HAVE_WMEMCHR
wchar_t *wmemchr(const wchar_t *ws, wchar_t wc, size_t n)
{
    for (unsigned i=0; i<n; ++i, ++ws)
        if (*ws == wc)
            return (wchar_t *)ws;
    return NULL;
}
#endif

#if ! _GLIBCPP_HAVE_WMEMCMP
int wmemcmp(const wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    for (unsigned i=0; i<n; ++i, ++ws1, ++ws2)
        if (*ws1 != *ws2)
            return *ws1 - *ws2;
    return 0;
}
#endif

#if ! _GLIBCPP_HAVE_WMEMCPY
wchar_t *wmemcpy(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    return (wchar_t *)memcpy(ws1, ws2, n * sizeof(wchar_t));
}
#endif

#if ! _GLIBCPP_HAVE_WMEMMOVE
wchar_t *wmemmove(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    return (wchar_t *)memmove(ws1, ws2, n * sizeof(wchar_t));
}
#endif

#if ! _GLIBCPP_HAVE_WMEMSET
wchar_t *wmemset(wchar_t *ws , wchar_t wc, size_t n)
{
    wchar_t *temp = ws;
    for (unsigned i=0; i<n; ++i, ++temp)
        *temp = wc;
    return ws;
}
#endif
}

#if 0
/* This has nothing to do with this file, but I might as well put it here
 * rather than make a new file just for it.
 */
#include <iostream>
#include <fstream>
using namespace std;

template basic_istream<char, char_traits<char> >&
    basic_istream<char, char_traits<char> >::seekg(fpos<__mbstate_t>);
template streampos
	basic_streambuf<char, char_traits<char> >::pubseekpos(fpos<__mbstate_t>,
														  _Ios_Openmode);
#endif

/*
 * (c) 2003-2005 Steve Checkoway
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
