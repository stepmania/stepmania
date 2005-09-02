/* StdString - std::string convenience wrapper. */

// =============================================================================
//  FILE:  StdString.h
//  AUTHOR:	Joe O'Leary (with outside help noted in comments)
//  REMARKS:
//		This header file declares the CStdStr template.  This template derives
//		the Standard C++ Library basic_string<> template and add to it the
//		the following conveniences:
//			- The full MFC CString set of functions (including implicit cast)
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
//		improve the CString facade.
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
//
//	REVISION HISTORY
//    2002-JUN-26 - Thanks to Arnt Witteveen for pointing out that I was using
//                  static_cast<> in a place in which I should have been using
//                  reinterpret_cast<> (the ctor for unsigned char strings).
//                  That's what happens when I don't unit-test properly!
//                  Arnt also noticed that CString was silently correcting the
//                  'nCount' argument to Left() and Right() where CStdString was
//                  not (and crashing if it was bad).  That is also now fixed!
//	  2002-FEB-25 - Thanks to Tim Dowty for pointing out (and giving me the fix
//					for) a conversion problem with non-ASCII MBCS characters.
//					CStdString is now used in my favorite commercial MP3 player!
//	  2001-DEC-06 - Thanks to Wang Haifeng for spotting a problem in one of the
//					assignment operators (for _bstr_t) that would cause compiler
//					errors when refcounting protection was turned off.
//	  2001-NOV-27 - Remove calls to operator!= which involve reverse_iterators
//					due to a conflict with the rel_ops operator!=.  Thanks to
//					John James for pointing this out.
//    2001-OCT-29 - Added a minor range checking fix for the Mid function to
//					make it as forgiving as CString's version is.  Thanks to
//					Igor Kholodov for noticing this.  
//				  - Added a specialization of std::swap for CStdString.  Thanks
//					to Mike Crusader for suggesting this!  It's commented out
//					because you're not supposed to inject your own code into the
//					'std' namespace.  But if you don't care about that, it's
//					there if you want it
//				  - Thanks to Jason Mills for catching a case where CString was
//					more forgiving in the Delete() function than I was.
//	  2001-JUN-06 - I was violating the Standard name lookup rules stated
//					in [14.6.2(3)].  None of the compilers I've tried so
//					far apparently caught this but HP-UX aCC 3.30 did.  The
//					fix was to add 'this->' prefixes in many places.
//					Thanks to Farrokh Nejadlotfi for this!
//
//	  2001-APR-27 - StreamLoad was calculating the number of BYTES in one
//					case, not characters.  Thanks to Pablo Presedo for this.
//
//    2001-FEB-23 - Replace() had a bug which caused infinite loops if the
//					source string was empty.  Fixed thanks to Eric Nitzsche.
//
//    2001-FEB-23 - Scott Hathaway was a huge help in providing me with the
//					ability to build CStdString on Sun Unix systems.  He
//					sent me detailed build reports about what works and what
//					does not.  If CStdString compiles on your Unix box, you
//					can thank Scott for it.
//
//	  2000-DEC-29 - Joldakowski noticed one overload of Insert failed to do
//					range check as CString's does.  Now fixed -- thanks!
//
//	  2000-NOV-07 - Aaron pointed out that I was calling static member
//					functions of char_traits via a temporary.  This was not
//					technically wrong, but it was unnecessary and caused
//					problems for poor old buggy VC5.  Thanks Aaron!
//
//	  2000-JUL-11 - Joe Woodbury noted that the CString::Find docs don't match
//					what the CString::Find code really ends up doing.   I was
//					trying to match the docs.  Now I match the CString code
//				  - Joe also caught me truncating strings for GetBuffer() calls
//					when the supplied length was less than the current length.
//
//	  2000-MAY-25 - Better support for STLPORT's Standard library distribution
//				  - Got rid of the NSP macro - it interfered with Koenig lookup
//				  - Thanks to Joe Woodbury for catching a TrimLeft() bug that
//					I introduced in January.  Empty strings were not getting
//					trimmed
//
//	  2000-APR-17 - Thanks to Joe Vitaterna for pointing out that ReverseFind
//					is supposed to be a const function.
//
//	  2000-MAR-07 - Thanks to Ullrich Pollähne for catching a range bug in one
//					of the overloads of assign.
//
//    2000-FEB-01 - You can now use CStdString on the Mac with CodeWarrior!
//					Thanks to Todd Heckel for helping out with this.
//
//	  2000-JAN-23 - Thanks to Jim Cline for pointing out how I could make the
//					Trim() function more efficient.
//				  - Thanks to Jeff Kohn for prompting me to find and fix a typo
//					in one of the addition operators that takes _bstr_t.
//				  - Got rid of the .CPP file -  you only need StdString.h now!
//
//	  1999-DEC-22 - Thanks to Greg Pickles for helping me identify a problem
//					with my implementation of CStdString::FormatV in which
//					resulting string might not be properly NULL terminated.
//
//	  1999-DEC-06 - Chris Conti pointed yet another basic_string<> assignment
//					bug that MS has not fixed.  CStdString did nothing to fix
//					it either but it does now!  The bug was: create a string
//					longer than 31 characters, get a pointer to it (via c_str())
//					and then assign that pointer to the original string object.
//					The resulting string would be empty.  Not with CStdString!
//
//	  1999-OCT-06 - BufferSet was erasing the string even when it was merely
//					supposed to shrink it.  Fixed.  Thanks to Chris Conti.
//				  - Some of the Q172398 fixes were not checking for assignment-
//					to-self.  Fixed.  Thanks to Baptiste Lepilleur.
//
//	  1999-AUG-20 - Improved Load() function to be more efficient by using 
//					SizeOfResource().  Thanks to Rich Zuris for this.
//				  - Corrected resource ID constructor, again thanks to Rich.
//				  - Fixed a bug that occurred with UNICODE characters above
//					the first 255 ANSI ones.  Thanks to Craig Watson. 
//				  - Added missing overloads of TrimLeft() and TrimRight().
//					Thanks to Karim Ratib for pointing them out
//
//	  1999-JUL-21 - Made all calls to GetBuf() with no args check length first.
//
//	  1999-JUL-10 - Improved MFC/ATL independence of conversion macros
//				  - Added SS_NO_REFCOUNT macro to allow you to disable any
//					reference-counting your basic_string<> impl. may do.
//				  - Improved ReleaseBuffer() to be as forgiving as CString.
//					Thanks for Fan Xia for helping me find this and to
//					Matthew Williams for pointing it out directly.
//
//	  1999-JUL-06 - Thanks to Nigel Nunn for catching a very sneaky bug in
//					ToLower/ToUpper.  They should call GetBuf() instead of
//					data() in order to ensure the changed string buffer is not
//					reference-counted (in those implementations that refcount).
//
//	  1999-JUL-01 - Added a true CString facade.  Now you can use CStdString as
//					a drop-in replacement for CString.  If you find this useful,
//					you can thank Chris Sells for finally convincing me to give
//					in and implement it.
//				  - Changed operators << and >> (for MFC CArchive) to serialize
//					EXACTLY as CString's do.  So now you can send a CString out
//					to a CArchive and later read it in as a CStdString.   I have
//					no idea why you would want to do this but you can. 
//
//	  1999-JUN-21 - Changed the CStdString class into the CStdStr template.
//				  - Fixed FormatV() to correctly decrement the loop counter.
//					This was harmless bug but a bug nevertheless.  Thanks to
//					Chris (of Melbsys) for pointing it out
//				  - Changed Format() to try a normal stack-based array before
//					using to _alloca().
//				  - Updated the text conversion macros to properly use code
//					pages and to fit in better in MFC/ATL builds.  In other
//					words, I copied Microsoft's conversion stuff again. 
//				  - Added equivalents of CString::GetBuffer, GetBufferSetLength
//				  - a Trim() function that combines TrimRight() and TrimLeft().
//
//	  1999-MAR-13 - Corrected the "NotSpace" functional object to use _istpace()
//					instead of _isspace()   Thanks to Dave Plummer for this.
//
//	  1999-FEB-26 - Removed errant line (left over from testing) that #defined
//					_MFC_VER.  Thanks to John C Sipos for noticing this.
//
//	  1999-FEB-03 - Fixed a bug in a rarely-used overload of operator+() that
//					caused infinite recursion and stack overflow
//				  - Added member functions to simplify the process of
//					persisting CStdStrings to/from DCOM IStream interfaces 
//				  - Added functional objects (e.g. StdStringLessNoCase) that
//					allow CStdStrings to be used as keys STL map objects with
//					case-insensitive comparison 
//				  - Added array indexing operators (i.e. operator[]).  I
//					originally assumed that these were unnecessary and would be
//					inherited from basic_string.  However, without them, Visual
//					C++ complains about ambiguous overloads when you try to use
//					them.  Thanks to Julian Selman to pointing this out. 
//
//	  1998-FEB-?? - Added overloads of assign() function to completely account
//					for Q172398 bug.  Thanks to "Pete the Plumber" for this
//
//	  1998-FEB-?? - Initial submission
//
// =============================================================================

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

// -----------------------------------------------------------------------------
// sslen: strlen/wcslen wrappers
// -----------------------------------------------------------------------------
template<typename CT> inline int sslen(const CT* pT)
{
	return 0 == pT ? 0 : std::basic_string<CT>::traits_type::length(pT);
}
inline int sslen(const std::string& s)
{
	return s.length();
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
	// Watch out for NULLs, as always.

	if ( 0 == pA )
	{
		sDst.erase();
	}

#if defined(HAVE_ASSIGN_FIX)
	// If pA actually points to part of sDst, we must NOT erase(), but
	// rather take a substring

	else if ( pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.size() )
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
#else
	else
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
	if ( &sDst == &sSrc )
		sDst.reserve(2*sDst.size());

	sDst.append(sSrc.c_str());
}
inline void	ssadd(std::string& sDst, PCSTR pA)
{
	if ( pA )
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
// -----------------------------------------------------------------------------
//  vsprintf/vswprintf or _vsnprintf/_vsnwprintf equivalents.  In standard
//  builds we can't use _vsnprintf/_vsnwsprintf because they're MS extensions.
// -----------------------------------------------------------------------------
	inline int ssvsprintf(PSTR pA, size_t nCount, PCSTR pFmtA, va_list vl)
	{
#if defined(WIN32)
		return _vsnprintf(pA, nCount, pFmtA, vl);
#else
		return vsnprintf(pA, nCount, pFmtA, vl);
#endif
	}


//			Now we can define the template (finally!)
// =============================================================================
// TEMPLATE: CStdStr
//		template<typename CT> class CStdStr : public std::basic_string<CT>
//
// REMARKS:
//		This template derives from basic_string<CT> and adds some MFC CString-
//		like functionality
//
//		Basically, this is my attempt to make Standard C++ library strings as
//		easy to use as the MFC CString class.
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

	CStdStr(const std::string& str)
	{
		ssasn(*this, str);
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
	// -------------------------------------------------------------------------
	MYTYPE& ToUpper()
	{
		//  Strictly speaking, this would be about the most portable way

		//	std::transform(begin(),
		//				   end(),
		//				   begin(),
		//				   std::bind2nd(SSToUpper<CT>(), std::locale()));

		// But practically speaking, this works faster

		if ( !this->empty() )
			ssupr(GetBuf(), this->size());

		return *this;
	}



	MYTYPE& ToLower()
	{
		//  Strictly speaking, this would be about the most portable way

		//	std::transform(begin(),
		//				   end(),
		//				   begin(),
		//				   std::bind2nd(SSToLower<CT>(), std::locale()));

		// But practically speaking, this works faster

		if ( !this->empty() )
			sslwr(GetBuf(), this->size());

		return *this;
	}



	// -------------------------------------------------------------------------
	// CStdStr -- Direct access to character buffer.  In the MS' implementation,
	// the at() function that we use here also calls _Freeze() providing us some
	// protection from multithreading problems associated with ref-counting.
	// -------------------------------------------------------------------------
	CT* GetBuf(int nMinLen=-1)
	{
		if ( static_cast<int>(this->size()) < nMinLen )
			this->resize(static_cast<MYSIZE>(nMinLen));

		return this->empty() ? const_cast<CT*>(this->data()) : &(this->at(0));
	}

	void RelBuf(int nNewLen=-1)
	{
		this->resize(static_cast<MYSIZE>(nNewLen > -1 ? nNewLen : sslen(this->c_str())));
	}

	// -------------------------------------------------------------------------
	// FUNCTION:  CStdStr::Format
	//		void _cdecl Formst(CStdStringA& PCSTR szFormat, ...)
	//		void _cdecl Format(PCSTR szFormat);
	//           
	// DESCRIPTION:
	//		This function does sprintf/wsprintf style formatting on CStdStringA
	//		objects.  It looks a lot like MFC's CString::Format.  Some people
	//		might even call this identical.  Fortunately, these people are now
	//		dead.
	//
	// PARAMETERS: 
	//		nId - ID of string resource holding the format string
	//		szFormat - a PCSTR holding the format specifiers
	//		argList - a va_list holding the arguments for the format specifiers.
	//
	// RETURN VALUE:  None.
	// -------------------------------------------------------------------------
	// formatting (using wsprintf style formatting)

    // If they want a Format() function that safely handles string objects
    // without casting
 

	void Format(const CT* szFmt, ...)
	{
		va_list argList;
		va_start(argList, szFmt);
		FormatV(szFmt, argList);
		va_end(argList);
	}

	#define MAX_FMT_TRIES		5	 // #of times we try 
	#define FMT_BLOCK_SIZE		2048 // # of bytes to increment per try
	#define BUFSIZE_1ST	256
	#define BUFSIZE_2ND 512
	#define STD_BUF_SIZE		1024

	// -------------------------------------------------------------------------
	// FUNCTION:  FormatV
	//		void FormatV(PCSTR szFormat, va_list, argList);
	//           
	// DESCRIPTION:
	//		This function formats the string with sprintf style format-specs. 
	//		It makes a general guess at required buffer size and then tries
	//		successively larger buffers until it finds one big enough or a
	//		threshold (MAX_FMT_TRIES) is exceeded.
	//
	// PARAMETERS: 
	//		szFormat - a PCSTR holding the format of the output
	//		argList - a Microsoft specific va_list for variable argument lists
	//
	// RETURN VALUE: 
	// -------------------------------------------------------------------------

	void FormatV(const CT* szFormat, va_list argList)
	{
	#if defined(WIN32) && !defined(__MINGW32__)
		CT* pBuf			= NULL;
		int nChars			= 1;
		int nUsed			= 0;
		size_type nActual	= 0;
		int nTry			= 0;

		do	
		{
			// Grow more than linearly (e.g. 512, 1536, 3072, etc)

			nChars			+= ((nTry+1) * FMT_BLOCK_SIZE);
			pBuf			= reinterpret_cast<CT*>(_alloca(sizeof(CT)*nChars));
			nUsed			= ssvsprintf(pBuf, nChars-1, szFormat, argList);

			// Ensure proper NULL termination.

			nActual			= nUsed == -1 ? nChars-1 : min(nUsed, nChars-1);
			pBuf[nActual+1]= '\0';
		} while ( nUsed < 0 && nTry++ < MAX_FMT_TRIES );

		// assign whatever we managed to format

		this->assign(pBuf, nActual);
	#else
		static bool bExactSizeSupported;
		static bool bInitialized = false;
		if( !bInitialized )
		{
			/* Some systems return the actual size required when snprintf
			 * doesn't have enough space.  This lets us avoid wasting time
			 * iterating, and wasting memory. */
			bInitialized = true;
			char ignore;
			bExactSizeSupported = ( snprintf( &ignore, 0, "Hello World" ) == 11 );
		}

		if( bExactSizeSupported )
		{
			va_list tmp;
			va_copy( tmp, argList );
			char ignore;
			int iNeeded = ssvsprintf( &ignore, 0, szFormat, tmp );
			va_end(tmp);

			char *buf = GetBuffer( iNeeded+1 );
			ssvsprintf( buf, iNeeded+1, szFormat, argList );
			ReleaseBuffer( iNeeded );
			return;
		}

		int nChars			= FMT_BLOCK_SIZE;
		int nTry			= 1;
		do	
		{
			// Grow more than linearly (e.g. 512, 1536, 3072, etc)
			char *buf = GetBuffer(nChars);
			int nUsed = ssvsprintf(buf, nChars-1, szFormat, argList);

			if(nUsed == -1)
			{
				nChars += ((nTry+1) * FMT_BLOCK_SIZE);
				ReleaseBuffer();
				continue;
			}

			/* OK */
			ReleaseBuffer(nUsed);
			break;
		} while ( nTry++ < MAX_FMT_TRIES );
	#endif
	}
	

	// -------------------------------------------------------------------------
	// CString Facade Functions:
	//
	// The following methods are intended to allow you to use this class as a
	// drop-in replacement for CString.
	// -------------------------------------------------------------------------
	int CompareNoCase(PCMYSTR szThat)	const
	{
		return ssicmp(this->c_str(), szThat);
	}

	int Find(CT ch) const
	{
		MYSIZE nIdx	= this->find_first_of(ch);
		return MYBASE::npos == nIdx  ? -1 : static_cast<int>(nIdx);
	}

	int Find(PCMYSTR szSub) const
	{
		MYSIZE nIdx	= this->find(szSub);
		return MYBASE::npos == nIdx ? -1 : static_cast<int>(nIdx);
	}

	int Find(CT ch, int nStart) const
	{
		// CString::Find docs say add 1 to nStart when it's not zero
		// CString::Find code doesn't do that however.  We'll stick
		// with what the code does

		MYSIZE nIdx	= this->find_first_of(ch, static_cast<MYSIZE>(nStart));
		return MYBASE::npos == nIdx ? -1 : static_cast<int>(nIdx);
	}

	int Find(PCMYSTR szSub, int nStart) const
	{
		// CString::Find docs say add 1 to nStart when it's not zero
		// CString::Find code doesn't do that however.  We'll stick
		// with what the code does

		MYSIZE nIdx	= this->find(szSub, static_cast<MYSIZE>(nStart));
		return MYBASE::npos == nIdx ? -1 : static_cast<int>(nIdx);
	}

	// -------------------------------------------------------------------------
	// GetXXXX -- Direct access to character buffer
	// -------------------------------------------------------------------------
	CT* GetBuffer(int nMinLen=-1)
	{
		return GetBuf(nMinLen);
	}

	// GetLength() -- MFC docs say this is the # of BYTES but
	// in truth it is the number of CHARACTERs (chars or wchar_ts)
	int GetLength() const
	{
		return static_cast<int>(this->length());
	}

	
	int Insert(int nIdx, CT ch)
	{
		if ( static_cast<MYSIZE>(nIdx) > this->size() -1 )
			this->append(1, ch);
		else
			this->insert(static_cast<MYSIZE>(nIdx), 1, ch);

		return GetLength();
	}
	int Insert(int nIdx, PCMYSTR sz)
	{
		if ( nIdx >= this->size() )
			this->append(sz, sslen(sz));
		else
			this->insert(static_cast<MYSIZE>(nIdx), sz);

		return GetLength();
	}

	MYTYPE Left(int nCount) const
	{
        // Range check the count.

		nCount = max(0, min(nCount, static_cast<int>(this->size())));
		return this->substr(0, static_cast<MYSIZE>(nCount)); 
	}

	void MakeLower()
	{
		ToLower();
	}

	void MakeReverse()
	{
		std::reverse(this->begin(), this->end());
	}

	void MakeUpper()
	{ 
		ToUpper();
	}

	void ReleaseBuffer(int nNewLen=-1)
	{
		RelBuf(nNewLen);
	}

	int Remove(CT ch)
	{
		MYSIZE nIdx		= 0;
		int nRemoved	= 0;
		while ( (nIdx=this->find_first_of(ch)) != MYBASE::npos )
		{
			this->erase(nIdx, 1);
			nRemoved++;
		}
		return nRemoved;
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
			replace(this->begin()+nIdx, this->begin()+nIdx+nOldLen, szRealNew);
			nReplaced++;
			nIdx += nNewLen;
		}
		return nReplaced;
	}

	int ReverseFind(CT ch) const
	{
		MYSIZE nIdx	= this->find_last_of(ch);
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
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



// -----------------------------------------------------------------------------
// CStdStr friend addition functions defined as inline
// -----------------------------------------------------------------------------




// -----------------------------------------------------------------------------
// These versions of operator+ provided by Scott Hathaway in order to allow
// CStdString to build on Sun Unix systems.
// -----------------------------------------------------------------------------

#if defined(__SUNPRO_CC_COMPAT) || defined(__SUNPRO_CC)

inline
CStdStr<char> operator+(const  CStdStr<char>& str1, const  CStdStr<char>& str2)
{
	CStdStr<char> strRet(str1);
	strRet.append(str2);
	return strRet;
}

inline
CStdStr<char> operator+(const  CStdStr<char>& str, char t)
{
	// this particular overload is needed for disabling reference counting
	// though it's only an issue from line 1 to line 2

	CStdStr<char> strRet(str);	// 1
	strRet.append(1, t);				// 2
	return strRet;
}

inline
CStdStr<char> operator+(const  CStdStr<char>& str, PCSTR pA)
{
	return CStdStr<char>(str) + CStdStr<char>(pA);
}

inline
CStdStr<char> operator+(PCSTR pA, const  CStdStr<char>& str)
{
	CStdStr<char> strRet(pA);
	strRet.append(str);
	return strRet;
}

#endif // defined(__SUNPRO_CC_COMPAT) || defined(__SUNPRO_CC)


// =============================================================================
//						END OF CStdStr INLINE FUNCTION DEFINITIONS
// =============================================================================

//	Now typedef our class names based upon this humongous template

typedef CStdStr<char>		CStdStringA;	// a better std::string



// -----------------------------------------------------------------------------
// HOW TO EXPORT CSTDSTRING FROM A DLL
//
// If you want to export CStdStringA and CStdStringW from a DLL, then all you
// need to
//		1.	make sure that all components link to the same DLL version
//			of the CRT (not the static one).
//		2.	Uncomment the 3 lines of code below
//		3.	#define 2 macros per the instructions in MS KnowledgeBase
//			article Q168958.  The macros are:
//
//		MACRO		DEFINTION WHEN EXPORTING		DEFINITION WHEN IMPORTING
//		-----		------------------------		-------------------------
//		SSDLLEXP	(nothing, just #define it)		extern
//		SSDLLSPEC	__declspec(dllexport)			__declspec(dllimport)
//
//		Note that these macros must be available to ALL clients who want to 
//		link to the DLL and use the class.  If they 
// -----------------------------------------------------------------------------
//#pragma warning(disable:4231) // non-standard extension ("extern template")
//	SSDLLEXP template class SSDLLSPEC CStdStr<char>;
//	SSDLLEXP template class SSDLLSPEC CStdStr<wchar_t>;


// In MFC builds, define some global serialization operators
// Special operators that allow us to serialize CStdStrings to CArchives.
// Note that we use an intermediate CString object in order to ensure that
// we use the exact same format.

#ifdef _MFC_VER
	inline CArchive& AFXAPI operator<<(CArchive& ar, const CStdStringA& strA)
	{
		CString strTemp	= strA;
		return ar << strTemp;
	}

	inline CArchive& AFXAPI operator>>(CArchive& ar, CStdStringA& strA)
	{
		CString strTemp;
		ar >> strTemp;
		strA = strTemp;
		return ar;
	}
#endif	// #ifdef _MFC_VER -- (i.e. is this MFC?)

// Define friendly names for some of these functions

	#define CStdString				CStdStringA

// ...and some shorter names for the space-efficient

#define WUFmtA						WUFormatA
#define	WUFmtW						WUFormatW
#define WUFmt						WUFormat


// -----------------------------------------------------------------------------
// FUNCTIONAL COMPARATORS:
// REMARKS:
//		These structs are derived from the std::binary_function template.  They
//		give us functional classes (which may be used in Standard C++ Library
//		collections and algorithms) that perform case-insensitive comparisons of
//		CStdString objects.  This is useful for maps in which the key may be the
//		 proper string but in the wrong case.
// -----------------------------------------------------------------------------
#define StdStringLessNoCaseW		SSLNCW	// avoid VC compiler warning 4786
#define StdStringEqualsNoCaseW		SSENCW		
#define StdStringLessNoCaseA		SSLNCA		
#define StdStringEqualsNoCaseA		SSENCA		

#define StdStringLessNoCase			SSLNCA
#define StdStringEqualsNoCase		SSENCA

struct StdStringLessNoCaseA
	: std::binary_function<CStdStringA, CStdStringA, bool>
{
	inline
	bool operator()(const CStdStringA& sLeft, const CStdStringA& sRight) const
	{ return ssicmp(sLeft.c_str(), sRight.c_str()) < 0; }
};
struct StdStringEqualsNoCaseA
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

#if defined(_MSC_VER) && (_MSC_VER > 1100)
	#pragma warning (pop)
#endif

#define CString CStdString
#define CStringArray vector<CString>

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
