#ifndef EnumHelper_H
#define EnumHelper_H

#define FOREACH_ENUM( e, max, var )	for( e var=(e)0; var<max; ((int&)var)++ )


static const CString EMPTY_STRING;

#define XToString(X)	\
	const CString& X##ToString( X x ) \
	{	\
		if( x == ARRAYSIZE(X##Names)+1 ) 	\
			return EMPTY_STRING;	\
		ASSERT(unsigned(x) < ARRAYSIZE(X##Names));	\
		return X##Names[x];	\
	}

#define XToThemedString(X)	\
	CString X##ToThemedString( X x ) \
	{	\
		return THEME->GetMetric( #X, X##ToString(x) );	\
	}

#define StringToX(X)	\
	X StringTo##X( const CString& s ) \
	{	\
		CString s2 = s;	\
		s2.MakeLower();	\
        unsigned i; \
		for( i = 0; i < ARRAYSIZE(X##Names); ++i )	\
			if( !s2.CompareNoCase(X##Names[i]) )	\
				return (X)i;	\
		return (X)(i+1); /*invalid*/	\
	}

#endif

/*
 * (c) 2004 Chris Danford
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
