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
