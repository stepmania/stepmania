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



//
// An enum class with reflection-like functionality.
// String literals are stored in GetString instead of a static member so that 
// the literals can be defined in the header.
//
#define BEGIN_ENUM( name )	\
class name {				\
public:						\
	enum Value {			\

#define MIDDLE_ENUM( name )										\
		NUM,													\
		INVALID = -1,											\
	};															\
protected:														\
	Value m_value;												\
	CCStringRef GetString(int i) const {						\
		const CString s[NUM] = {								\

#define END_ENUM( name )										\
		}; return s[i]; }										\
public:															\
	name( Value value )						{ m_value=value; }			\
	name()									{ MakeInvalid(); }			\
	name( int value )						{ m_value=(Value)value; }		\
	operator int&()							{ return (int&)m_value; }	\
	operator size_t() const					{ return (size_t)m_value; }	\
	static size_t Num()						{ return (size_t)NUM; }		\
	name& operator=(Value val)				{ m_value = val; return *this; }	\
	bool operator<(const name& other) const	{ return m_value < other.m_value; }		\
	void MakeInvalid()						{ m_value=INVALID; }		\
	bool IsValid() const					{ return m_value>=0 && m_value<NUM; }	\
	CCStringRef ToString() const			{ ASSERT(IsValid()); return GetString(m_value); }	\
	CString ToThemedString() const			{ return GetThemedString(#name,ToString()); }	\
	void FromString( CString s )								\
	{															\
		for( int i=0; i<NUM; i++ )								\
		{														\
			if( stricmp(s,GetString(i))==0 )					\
			{													\
				m_value = (Value)i;								\
				return;											\
			}													\
		}														\
		MakeInvalid();											\
	}															\
};																\

#define FOREACH( name, var )	for( name var(0); (int)var<(int)name::NUM; var++ )

CString GetThemedString( CCStringRef sClass, CCStringRef sValue );

#endif
