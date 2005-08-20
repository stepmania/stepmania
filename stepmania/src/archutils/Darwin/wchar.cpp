#include <AvailabilityMacros.h>

/* The Mac OS X 10.2.8 sdk is lacking wchar.h and all that goes with it.
 * wchar_t is a fundamental type in c++ but is essentially useless with this
 * sdk. As a result, this file contains all of the code required to actually
 * use wchar_t, at least in StepMania. If this file is compiled with
 * MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3 then the included
 * wchar_t (and friends) is used.
 *
 * For maximum usability, this hack must be maintained as long as 10.2.8 is
 * required to be supported.
 */
#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_3)
#include <sys/cdefs.h>
#include <cstddef>
#include <string>
#include <cstring>

#define WEOF -1
__BEGIN_DECLS
size_t wcslen(const wchar_t *ws)
{
    size_t n = 0;
    
    while (*(ws++) != 0)
        ++n;
    return n;
}

wchar_t *wmemchr(const wchar_t *ws, wchar_t wc, size_t n)
{
    for (unsigned i=0; i<n; ++i, ++ws)
        if (*ws == wc)
            return (wchar_t *)ws;
    return NULL;
}

int wmemcmp(const wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    for (unsigned i=0; i<n; ++i, ++ws1, ++ws2)
        if (*ws1 != *ws2)
            return *ws1 - *ws2;
    return 0;
}

wchar_t *wmemcpy(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    return (wchar_t *)memcpy(ws1, ws2, n * sizeof(wchar_t));
}

wchar_t *wmemmove(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
    return (wchar_t *)memmove(ws1, ws2, n * sizeof(wchar_t));
}

wchar_t *wmemset(wchar_t *ws , wchar_t wc, size_t n)
{
    wchar_t *temp = ws;
    for (unsigned i=0; i<n; ++i, ++temp)
        *temp = wc;
    return ws;
}
__END_DECLS

// copy and paste for the most part
namespace std
{
	typedef wchar_t wint_t;
	typedef wint_t wstreampos;

	template<>
	struct char_traits<wchar_t>
	{
		typedef wchar_t       char_type;
		typedef wint_t        int_type;
		typedef streamoff     off_type;
		typedef wstreampos    pos_type;
		typedef mbstate_t     state_type;
		
		// gcc is being stupid and not allowing me to define it here
		static void assign(char_type& __c1, const char_type& __c2);
		
		static bool eq(const char_type& __c1, const char_type& __c2)
		{
			return __c1 == __c2;
		}
		
		static bool lt(const char_type& __c1, const char_type& __c2)
		{
			return __c1 < __c2;
		}
		
		static int compare(const char_type* __s1, const char_type* __s2,
						   size_t __n)
		{
			return wmemcmp(__s1, __s2, __n);
		}
		
		// same here
		static size_t length(const char_type* __s);
		
		static const char_type* find(const char_type* __s, size_t __n,
									 const char_type& __a)
		{
			return wmemchr(__s, __a, __n);
		}
		
		static char_type* move(char_type* __s1, const char_type* __s2,
							   int_type __n)
		{
			return wmemmove(__s1, __s2, __n);
		}
		
		// and here
		static char_type* copy(char_type* __s1, const char_type* __s2,
							   size_t __n);
		
		static char_type* assign(char_type* __s, size_t __n, char_type __a)
		{
			return wmemset(__s, __a, __n);
		}
	
		static char_type to_char_type(const int_type& __c)
		{
			return char_type(__c);
		}
		
		static int_type to_int_type(const char_type& __c)
		{
			return int_type(__c);
		}
		
		static bool eq_int_type(const int_type& __c1, const int_type& __c2)
		{
			return __c1 == __c2;
		}
		
		static int_type eof()
		{
			return static_cast<int_type>(WEOF);
		}
		
		static int_type not_eof(const int_type& __c)
		{
			return eq_int_type(__c, eof()) ? 0 : __c;
		}
	};
	// So, let's just do it here
	void char_traits<wchar_t>::assign(wchar_t& __c1, wchar_t const& __c2)
	{
		__c1 = __c2;
	}
	
	size_t char_traits<wchar_t>::length(const char_type* __s)
	{
		return wcslen(__s);
	}
	
	// This return type should be char_traits<wchar_t>::char_type*. wchar_t
	// is shorter.
	wchar_t* char_traits<wchar_t>::copy(char_type* __s1, const char_type* __s2,
										size_t __n)
	{
		return wmemcpy(__s1, __s2, __n);
	}
}

template class std::basic_string<wchar_t>;
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
