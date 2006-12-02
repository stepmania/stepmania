/* StdString - std::string convenience wrapper. */

// =============================================================================
//  FILE:  StdString.h
//  AUTHOR:	Joe O'Leary (with outside help noted in comments)
//  REMARKS:
//		This header file declares the CStdStr template.  This template derives
//		the Standard C++ Library basic_string<> template and add to it the
//		the following conveniences:
//			- The full MFC RString set of functions (including implicit cast)
//			- writing to/reading from COM IStream interfaces
//			- Functional objects for use in STL algorithms
//
//		From this template, we intstantiate two classes:  CStdStringA and
//		CStdStringW.  The name "CStdString" is just a #define of one of these,
//		based upone the _UNICODE macro setting
//
//		This header also declares our own version of the MFC/ATL UNICODE-MBCS
//		conversion macros.  Our version looks exactly like the Microsoft's to
//		facilitate portability.
//
//	NOTE:
//		If you you use this in an MFC or ATL build, you should include either
//		afx.h or atlbase.h first, as appropriate.
//
//	PEOPLE WHO HAVE CONTRIBUTED TO THIS CLASS:
//
//		Several people have helped me iron out problems and othewise improve
//		this class.  OK, this is a long list but in my own defense, this code
//		has undergone two major rewrites.  Many of the improvements became
//		necessary after I rewrote the code as a template.  Others helped me
//		improve the RString facade.
//
//		Anyway, these people are (in chronological order):
//
//			- Pete the Plumber (???)
//			- Julian Selman
//			- Chris (of Melbsys)
//			- Dave Plummer
//			- John C Sipos
//			- Chris Sells
//			- Nigel Nunn
//			- Fan Xia
//			- Matthew Williams
//			- Carl Engman
//			- Mark Zeren
//			- Craig Watson
//			- Rich Zuris
//			- Karim Ratib
//			- Chris Conti
//			- Baptiste Lepilleur
//			- Greg Pickles
//			- Jim Cline
//			- Jeff Kohn
//			- Todd Heckel
//			- Ullrich Pollähne
//			- Joe Vitaterna
//			- Joe Woodbury
//			- Aaron (no last name)
//			- Joldakowski (???)
//			- Scott Hathaway
//			- Eric Nitzche
//			- Pablo Presedo
//			- Farrokh Nejadlotfi
//			- Jason Mills
//			- Igor Kholodov
//			- Mike Crusader
//			- John James
//			- Wang Haifeng
//			- Tim Dowty
//          - Arnt Witteveen

// Turn off browser references
// Turn off unavoidable compiler warnings


#if defined(_MSC_VER) && (_MSC_VER > 1100)
	#pragma component(browser, off, references, "CStdString")
	#pragma warning (push)
	#pragma warning (disable : 4290) // C++ Exception Specification ignored
	#pragma warning (disable : 4127) // Conditional expression is constant
	#pragma warning (disable : 4097) // typedef name used as synonym for class name
	#pragma warning (disable : 4512) // assignment operator could not be generated
#endif

#ifndef STDSTRING_H
#define STDSTRING_H

// If they want us to use only standard C++ stuff (no Win32 stuff)

typedef const char*		PCSTR;
typedef char*			PSTR;

// Standard headers needed
#include <string>			// basic_string
#include <algorithm>		// for_each, etc.
#include <functional>		// for StdStringLessNoCase, et al

#if defined(WIN32)
#include <malloc.h>			// _alloca
#endif

#include <cstdarg>

#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstdarg>

// a very shorthand way of applying the fix for KB problem Q172398
// (basic_string assignment bug)

#if defined ( _MSC_VER ) && ( _MSC_VER < 1200 )
	#define HAVE_ASSIGN_FIX
	#define Q172398(x) (x).erase()
#else
	#define Q172398(x)
#endif

/* In RageUtil: */
void MakeUpper( char *p, size_t iLen );
void MakeLower( char *p, size_t iLen );
void MakeUpper( wchar_t *p, size_t iLen );
void MakeLower( wchar_t *p, size_t iLen );

// =============================================================================
// INLINE FUNCTIONS ON WHICH CSTDSTRING RELIES
//
// Usually for generic text mapping, we rely on preprocessor macro definitions
// to map to string functions.  However the CStdStr<> template cannot use
// macro-based generic text mappings because its character types do not get
// resolved until template processing which comes AFTER macro processing.  In
// other words, UNICODE is of little help to us in the CStdStr template
//
// Therefore, to keep the CStdStr declaration simple, we have these inline
// functions.  The template calls them often.  Since they are inline (and NOT
// exported when this is built as a DLL), they will probably be resolved away
// to nothing. 
//
// Without these functions, the CStdStr<> template would probably have to broken
// out into two, almost identical classes.  Either that or it would be a huge,
// convoluted mess, with tons of "if" statements all over the place checking the
// size of template parameter CT.
// 
// In several cases, you will see two versions of each function.  One version is
// the more portable, standard way of doing things, while the other is the
// non-standard, but often significantly faster Visual C++ way.
// =============================================================================
namespace StdString
{

// -----------------------------------------------------------------------------
// sslen: strlen/wcslen wrappers
// -----------------------------------------------------------------------------
template<typename CT> inline int sslen(const CT* pT)
{
	return 0 == pT ? 0 : std::basic_string<CT>::traits_type::length(pT);
}

// -----------------------------------------------------------------------------
// sstolower/sstoupper -- convert characters to upper/lower case
// -----------------------------------------------------------------------------
//inline char sstoupper(char ch)		{ return (char)::toupper(ch); }
//inline char sstolower(char ch)		{ return (char)::tolower(ch); }
/* Our strings are UTF-8; instead of having to play around with locales,
 * let's just manually toupper ASCII only.  If we really want to play with
 * Unicode cases, we can do it ourself in RageUtil. */
inline char sstoupper(char ch)		{ return (ch >= 'a' && ch <= 'z')? char(ch + 'A' - 'a'): ch; }
inline char sstolower(char ch)		{ return (ch >= 'A' && ch <= 'Z')? char(ch + 'a' - 'A'): ch; }

// -----------------------------------------------------------------------------
// ssasn: assignment functions -- assign "sSrc" to "sDst"
// -----------------------------------------------------------------------------
typedef std::string::size_type		SS_SIZETYPE; // just for shorthand, really
typedef std::string::pointer		SS_PTRTYPE;  

inline void	ssasn(std::string& sDst, const std::string& sSrc)
{
	if ( sDst.c_str() != sSrc.c_str() )
	{
		sDst.erase();
		sDst.assign(sSrc);
	}
}
inline void	ssasn(std::string& sDst, PCSTR pA)
{
#if defined(HAVE_ASSIGN_FIX)
	// If pA actually points to part of sDst, we must NOT erase(), but
	// rather take a substring

	if ( pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.size() )
	{
		sDst =sDst.substr(static_cast<SS_SIZETYPE>(pA-sDst.c_str()));
	}

	// Otherwise (most cases) apply the assignment bug fix, if applicable
	// and do the assignment

	else
	{
		Q172398(sDst);
		sDst.assign(pA);
	}
	else
#else
		sDst.assign(pA);
#endif
}
inline void ssasn(std::string& sDst, const int nNull)
{
	sDst.erase();
}	
#undef StrSizeType


// -----------------------------------------------------------------------------
// ssadd: string object concatenation -- add second argument to first
// -----------------------------------------------------------------------------
inline void	ssadd(std::string& sDst, const std::string& sSrc)
{
	sDst.append(sSrc.c_str());
}
inline void	ssadd(std::string& sDst, PCSTR pA)
{
	// If the string being added is our internal string or a part of our
	// internal string, then we must NOT do any reallocation without
	// first copying that string to another object (since we're using a
	// direct pointer)

	if ( pA >= sDst.c_str() && pA <= sDst.c_str()+sDst.length())
	{
		if ( sDst.capacity() <= sDst.size()+sslen(pA) )
			sDst.append(std::string(pA));
		else
			sDst.append(pA);
	}
	else
	{
		sDst.append(pA); 
	}
}

// -----------------------------------------------------------------------------
// ssicmp: comparison (case insensitive )
// -----------------------------------------------------------------------------
	template<typename CT>
	inline int ssicmp(const CT* pA1, const CT* pA2)
	{
		CT f;
		CT l;

		do 
		{
			f = sstolower(*(pA1++));
			l = sstolower(*(pA2++));
		} while ( (f) && (f == l) );

		return (int)(f - l);
	}

// -----------------------------------------------------------------------------
// ssupr/sslwr: Uppercase/Lowercase conversion functions
// -----------------------------------------------------------------------------
#if 0
	template<typename CT>
	inline void sslwr(CT* pT, size_t nLen)
	{
		for ( CT* p = pT; static_cast<size_t>(p - pT) < nLen; ++p)
			*p = (CT)sstolower(*p);
	}
	template<typename CT>
	inline void ssupr(CT* pT, size_t nLen)
	{
		for ( CT* p = pT; static_cast<size_t>(p - pT) < nLen; ++p)
			*p = (CT)sstoupper(*p);
	}
#endif

	inline void sslwr(char *pT, size_t nLen)
	{
		MakeLower( pT, nLen );
	}
	inline void ssupr(char *pT, size_t nLen)
	{
		MakeUpper( pT, nLen );
	}
	inline void sslwr(wchar_t *pT, size_t nLen)
	{
		MakeLower( pT, nLen );
	}
	inline void ssupr(wchar_t *pT, size_t nLen)
	{
		MakeUpper( pT, nLen );
	}

#if defined(WIN32)
#define vsnprintf _vsnprintf
#endif

//			Now we can define the template (finally!)
// =============================================================================
// TEMPLATE: CStdStr
//		template<typename CT> class CStdStr : public std::basic_string<CT>
//
// REMARKS:
//		This template derives from basic_string<CT> and adds some MFC RString-
//		like functionality
//
//		Basically, this is my attempt to make Standard C++ library strings as
//		easy to use as the MFC RString class.
//
//		Note that although this is a template, it makes the assumption that the
//		template argument (CT, the character type) is either char or wchar_t.  
// =============================================================================

//#define CStdStr _SS	// avoid compiler warning 4786
template<typename CT>
class CStdStr;

template<typename CT>
inline
CStdStr<CT> operator+(const  CStdStr<CT>& str1, const  CStdStr<CT>& str2)
{
	CStdStr<CT> strRet(str1);
	strRet.append(str2);
	return strRet;
}

template<typename CT>	
inline
CStdStr<CT> operator+(const  CStdStr<CT>& str, CT t)
{
	// this particular overload is needed for disabling reference counting
	// though it's only an issue from line 1 to line 2

	CStdStr<CT> strRet(str);	// 1
	strRet.append(1, t);				// 2
	return strRet;
}

template<typename CT>
inline
CStdStr<CT> operator+(const  CStdStr<CT>& str, PCSTR pA)
{
	return CStdStr<CT>(str) + CStdStr<CT>(pA);
}

template<typename CT>
inline
CStdStr<CT> operator+(PCSTR pA, const  CStdStr<CT>& str)
{
	CStdStr<CT> strRet(pA);
	strRet.append(str);
	return strRet;
}



template<typename CT>
class CStdStr : public std::basic_string<CT>
{
	// Typedefs for shorter names.  Using these names also appears to help
	// us avoid some ambiguities that otherwise arise on some platforms

	typedef typename std::basic_string<CT>		MYBASE;	 // my base class
	typedef CStdStr<CT>							MYTYPE;	 // myself
	typedef typename MYBASE::const_pointer		PCMYSTR; // PCSTR 
	typedef typename MYBASE::pointer			PMYSTR;	 // PSTR
	typedef typename MYBASE::iterator			MYITER;  // my iterator type
	typedef typename MYBASE::const_iterator		MYCITER; // you get the idea...
	typedef typename MYBASE::reverse_iterator	MYRITER;
	typedef typename MYBASE::size_type			MYSIZE;   
	typedef typename MYBASE::value_type			MYVAL; 
	typedef typename MYBASE::allocator_type		MYALLOC;
	
public:

	// CStdStr inline constructors
	CStdStr()
	{
	}

	CStdStr(const MYTYPE& str) : MYBASE(str)
	{
	}

	CStdStr(const std::string& str): MYBASE(str)
	{
	}

	CStdStr(PCMYSTR pT, MYSIZE n) : MYBASE(pT, n)
	{
	}

	CStdStr(PCSTR pA)
	{
		*this = pA;
	}

	CStdStr(MYCITER first, MYCITER last)
		: MYBASE(first, last)
	{
	}

	CStdStr(MYSIZE nSize, MYVAL ch, const MYALLOC& al=MYALLOC())
		: MYBASE(nSize, ch, al)
	{
	}

	// CStdStr inline assignment operators -- the ssasn function now takes care
	// of fixing  the MSVC assignment bug (see knowledge base article Q172398).
	MYTYPE& operator=(const MYTYPE& str)
	{ 
		ssasn(*this, str); 
		return *this;
	}

	MYTYPE& operator=(const std::string& str)
	{
		ssasn(*this, str);
		return *this;
	}

	MYTYPE& operator=(PCSTR pA)
	{
		ssasn(*this, pA);
		return *this;
	}

	MYTYPE& operator=(CT t)
	{
		Q172398(*this);
		this->assign(1, t);
		return *this;
	}

	// Overloads  also needed to fix the MSVC assignment bug (KB: Q172398)
	//  *** Thanks to Pete The Plumber for catching this one ***
	// They also are compiled if you have explicitly turned off refcounting
	#if ( defined(_MSC_VER) && ( _MSC_VER < 1200 ) )

		MYTYPE& assign(const MYTYPE& str)
		{
			ssasn(*this, str);
			return *this;
		}

		MYTYPE& assign(const MYTYPE& str, MYSIZE nStart, MYSIZE nChars)
		{
			// This overload of basic_string::assign is supposed to assign up to
			// <nChars> or the NULL terminator, whichever comes first.  Since we
			// are about to call a less forgiving overload (in which <nChars>
			// must be a valid length), we must adjust the length here to a safe
			// value.  Thanks to Ullrich Pollähne for catching this bug

			nChars		= min(nChars, str.length() - nStart);

			// Watch out for assignment to self

			if ( this == &str )
			{
				MYTYPE strTemp(str.c_str()+nStart, nChars);
				MYBASE::assign(strTemp);
			}
			else
			{
				Q172398(*this);
				MYBASE::assign(str.c_str()+nStart, nChars);
			}
			return *this;
		}

		MYTYPE& assign(const MYBASE& str)
		{
			ssasn(*this, str);
			return *this;
		}

		MYTYPE& assign(const MYBASE& str, MYSIZE nStart, MYSIZE nChars)
		{
			// This overload of basic_string::assign is supposed to assign up to
			// <nChars> or the NULL terminator, whichever comes first.  Since we
			// are about to call a less forgiving overload (in which <nChars>
			// must be a valid length), we must adjust the length here to a safe
			// value. Thanks to Ullrich Pollähne for catching this bug

			nChars		= min(nChars, str.length() - nStart);

			// Watch out for assignment to self

			if ( this == &str )	// watch out for assignment to self
			{
				MYTYPE strTemp(str.c_str() + nStart, nChars);
				MYBASE::assign(strTemp);
			}
			else
			{
				Q172398(*this);
				MYBASE::assign(str.c_str()+nStart, nChars);
			}
			return *this;
		}

		MYTYPE& assign(const CT* pC, MYSIZE nChars)
		{
			// Q172398 only fix -- erase before assigning, but not if we're
			// assigning from our own buffer

	#if defined ( _MSC_VER ) && ( _MSC_VER < 1200 )
			if ( !this->empty() &&
				( pC < this->data() || pC > this->data() + this->capacity() ) )
			{
				this->erase();
			}
	#endif
			MYBASE::assign(pC, nChars);
			return *this;
		}

		MYTYPE& assign(MYSIZE nChars, MYVAL val)
		{
			Q172398(*this);
			MYBASE::assign(nChars, val);
			return *this;
		}

		MYTYPE& assign(const CT* pT)
		{
			return this->assign(pT, MYBASE::traits_type::length(pT));
		}

		MYTYPE& assign(MYCITER iterFirst, MYCITER iterLast)
		{
	#if defined ( _MSC_VER ) && ( _MSC_VER < 1200 ) 
			// Q172398 fix.  don't call erase() if we're assigning from ourself
			if ( iterFirst < this->begin() || iterFirst > this->begin() + this->size() )
				this->erase()
	#endif
				this->replace(this->begin(), this->end(), iterFirst, iterLast);
			return *this;
		}
	#endif

	/* VC6 string is missing clear(). */
	#if defined(_MSC_VER) && ( _MSC_VER < 1300 )	/* VC6, not VC7 */
	void clear()
	{
		this->erase();
	}
	#endif

	// -------------------------------------------------------------------------
	// CStdStr inline concatenation.
	// -------------------------------------------------------------------------
	MYTYPE& operator+=(const MYTYPE& str)
	{
		ssadd(*this, str);
		return *this;
	}

	MYTYPE& operator+=(const std::string& str)
	{
		ssadd(*this, str);
		return *this; 
	}

	MYTYPE& operator+=(PCSTR pA)
	{
		ssadd(*this, pA);
		return *this;
	}

	MYTYPE& operator+=(CT t)
	{
		this->append(1, t);
		return *this;
	}


	// addition operators -- global friend functions.

#if defined(_MSC_VER) && _MSC_VER < 1300 /* VC6, not VC7 */
/* work around another stupid vc6 bug */
#define EMP_TEMP
#else
#define EMP_TEMP <>
#endif
	friend	MYTYPE	operator+ EMP_TEMP(const MYTYPE& str1,	const MYTYPE& str2);
	friend	MYTYPE	operator+ EMP_TEMP(const MYTYPE& str,	CT t);
	friend	MYTYPE	operator+ EMP_TEMP(const MYTYPE& str,	PCSTR sz);
	friend	MYTYPE	operator+ EMP_TEMP(PCSTR pA,				const MYTYPE& str);

	// -------------------------------------------------------------------------
	// Case changing functions
	// -------------------------------------------------------------------------
	MYTYPE& MakeUpper()
	{
		if ( !this->empty() )
			ssupr(GetBuffer(), this->size());

		return *this;
	}

	MYTYPE& MakeLower()
	{
		if ( !this->empty() )
			sslwr(GetBuffer(), this->size());

		return *this;
	}



	// -------------------------------------------------------------------------
	// CStdStr -- Direct access to character buffer.  In the MS' implementation,
	// the at() function that we use here also calls _Freeze() providing us some
	// protection from multithreading problems associated with ref-counting.
	// -------------------------------------------------------------------------
	CT* GetBuffer(int nMinLen=-1)
	{
		if ( static_cast<int>(this->size()) < nMinLen )
			this->resize(static_cast<MYSIZE>(nMinLen));

		return this->empty() ? const_cast<CT*>(this->data()) : &(this->at(0));
	}

	void ReleaseBuffer(int nNewLen=-1)
	{
		this->resize(static_cast<MYSIZE>(nNewLen > -1 ? nNewLen : sslen(this->c_str())));
	}

	

	// -------------------------------------------------------------------------
	// RString Facade Functions:
	//
	// The following methods are intended to allow you to use this class as a
	// drop-in replacement for CString.
	// -------------------------------------------------------------------------
	int CompareNoCase(PCMYSTR szThat)	const
	{
		return ssicmp(this->c_str(), szThat);
	}

	bool EqualsNoCase(PCMYSTR szThat)	const
	{
		return CompareNoCase(szThat) == 0;
	}

	MYTYPE Left(int nCount) const
	{
		// Range check the count.

		nCount = max(0, min(nCount, static_cast<int>(this->size())));
		return this->substr(0, static_cast<MYSIZE>(nCount)); 
	}

	int Replace(CT chOld, CT chNew)
	{
		int nReplaced	= 0;
		for ( MYITER iter=this->begin(); iter != this->end(); iter++ )
		{
			if ( *iter == chOld )
			{
				*iter = chNew;
				nReplaced++;
			}
		}
		return nReplaced;
	}

	int Replace(PCMYSTR szOld, PCMYSTR szNew)
	{
		int nReplaced		= 0;
		MYSIZE nIdx			= 0;
		MYSIZE nOldLen		= sslen(szOld);
		if ( 0 == nOldLen )
			return 0;

		static const CT ch	= CT(0);
		MYSIZE nNewLen		= sslen(szNew);
		PCMYSTR szRealNew	= szNew == 0 ? &ch : szNew;

		while ( (nIdx=this->find(szOld, nIdx)) != MYBASE::npos )
		{
			MYBASE::replace(this->begin()+nIdx, this->begin()+nIdx+nOldLen, szRealNew);
			nReplaced++;
			nIdx += nNewLen;
		}
		return nReplaced;
	}

	MYTYPE Right(int nCount) const
	{
		// Range check the count.

		nCount = max(0, min(nCount, static_cast<int>(this->size())));
		return this->substr(this->size()-static_cast<MYSIZE>(nCount));
	}

	// Array-indexing operators.  Required because we defined an implicit cast
	// to operator const CT* (Thanks to Julian Selman for pointing this out)
	CT& operator[](int nIdx)
	{
		return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
	}

	const CT& operator[](int nIdx) const
	{
		return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
	}

	CT& operator[](unsigned int nIdx)
	{
		return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
	}

	const CT& operator[](unsigned int nIdx) const
	{
		return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
	}

	CT& operator[](long unsigned int nIdx){
	  return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
	}
       
	const CT& operator[](long unsigned int nIdx) const {
	  return MYBASE::operator[](static_cast<MYSIZE>(nIdx));
	}
#ifndef SS_NO_IMPLICIT_CASTS
	operator const CT*() const
	{
		return this->c_str();
	}
#endif
};


// =============================================================================
//						END OF CStdStr INLINE FUNCTION DEFINITIONS
// =============================================================================

//	Now typedef our class names based upon this humongous template

typedef CStdStr<char>		CStdStringA;	// a better std::string
#define CStdString				CStdStringA


// -----------------------------------------------------------------------------
// FUNCTIONAL COMPARATORS:
// REMARKS:
//		These structs are derived from the std::binary_function template.  They
//		give us functional classes (which may be used in Standard C++ Library
//		collections and algorithms) that perform case-insensitive comparisons of
//		CStdString objects.  This is useful for maps in which the key may be the
//		 proper string but in the wrong case.
// -----------------------------------------------------------------------------

#define StdStringLessNoCase		SSLNCA
#define StdStringEqualsNoCase		SSENCA

struct StdStringLessNoCase
	: std::binary_function<CStdStringA, CStdStringA, bool>
{
	inline
	bool operator()(const CStdStringA& sLeft, const CStdStringA& sRight) const
	{ return ssicmp(sLeft.c_str(), sRight.c_str()) < 0; }
};
struct StdStringEqualsNoCase
	: std::binary_function<CStdStringA, CStdStringA, bool>
{
	inline
	bool operator()(const CStdStringA& sLeft, const CStdStringA& sRight) const
	{ return ssicmp(sLeft.c_str(), sRight.c_str()) == 0; }
};

// These std::swap specializations come courtesy of Mike Crusader. 

//namespace std
//{
//	inline void swap(CStdStringA& s1, CStdStringA& s2) throw()
//	{
//		s1.swap(s2);
//	}
//}

}	// namespace StdString

#if defined(_MSC_VER) && (_MSC_VER > 1100)
	#pragma warning (pop)
#endif

#endif	// #ifndef STDSTRING_H

/*
 * COPYRIGHT:
 *	1999 Joseph M. O'Leary.  This code is free.  Use it anywhere you want.
 *	Rewrite it, restructure it, whatever.  Please don't blame me if it makes
 *	your $30 billion dollar satellite explode in orbit.  If you redistribute
 *	it in any form, I'd appreciate it if you would leave this notice here.
 *
 *	If you find any bugs, please let me know:
 *
 *			jmoleary@earthlink.net
 *			http://home.earthlink.net/~jmoleary
 */
