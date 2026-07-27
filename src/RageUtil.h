/** @brief RageUtil - Miscellaneous helper macros and functions. */

#ifndef RAGE_UTIL_H
#define RAGE_UTIL_H

#include "global.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
#include <sstream>
#include <vector>

class RageFileDriver;

/** @brief Safely delete pointers. */
#define SAFE_DELETE(p)       do { delete (p);     (p)=nullptr; } while( false )
/** @brief Safely delete array pointers. */
#define SAFE_DELETE_ARRAY(p) do { delete[] (p);   (p)=nullptr; } while( false )

/** @brief Zero out the memory. */
#define ZERO(x)	memset(&(x), 0, sizeof(x))
/** @brief Copy from a to b. */
#define COPY(a,b) do { ASSERT(sizeof(a)==sizeof(b)); memcpy(&(a), &(b), sizeof(a)); } while( false )
/** @brief Get the length of the array. */
#define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))

extern const RString CUSTOM_SONG_PATH;

/**
 * @brief Scales x so that l1 corresponds to l2 and h1 corresponds to h2.
 *
 * This does not modify x, so it MUST assign the result to something!
 * Do the multiply before the divide to that integer scales have more precision.
 *
 * One such example: SCALE(x, 0, 1, L, H); interpolate between L and H.
 */
#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) * ((h2) - (l2)) / ((h1) - (l1)) + (l2))

template<typename T, typename U>
inline U lerp( T x, U l, U h )
{
	return U(x * (h - l) + l);
}

template<typename T, typename U, typename V>
inline bool CLAMP(T& x, U l, V h)
{
	if(x > static_cast<T>(h)) { x= static_cast<T>(h); return true; }
	else if(x < static_cast<T>(l)) { x= static_cast<T>(l); return true; }
	return false;
}

template<class T>
inline bool ENUM_CLAMP( T &x, T l, T h )
{
	if (x > h)	{ x = h; return true; }
	else if (x < l) { x = l; return true; }
	return false;
}

inline void wrap( int &x, int n )
{
	if (x<0)
		x += ((-x/n)+1)*n;
	x %= n;
}
inline void wrap( unsigned &x, unsigned n )
{
	x %= n;
}
inline void wrap( float &x, float n )
{
	if (x<0)
		x += std::trunc(((-x/n)+1))*n;
	x = std::fmod(x,n);
}

inline float fracf( float f ) { return f - std::trunc(f); }

template<class T>
void CircularShift( std::vector<T> &v, int dist )
{
	for( int i = std::abs(dist); i>0; i-- )
	{
		if( dist > 0 )
		{
			T t = v[0];
			v.erase( v.begin() );
			v.push_back( t );
		}
		else
		{
			T t = v.back();
			v.erase( v.end()-1 );
			v.insert( v.begin(), t );
		}
	}
}

template<typename Type, typename Ret>
static Ret *CreateClass() { return new Type; }

/*
 * Helper function to remove all objects from an STL container for which the
 * Predicate pred is true. If you want to remove all objects for which the predicate
 * returns false, wrap the predicate with not1().
 */
template<typename Container, typename Predicate>
inline void RemoveIf( Container& c, Predicate p )
{
	c.erase( remove_if(c.begin(), c.end(), p), c.end() );
}
template<typename Container, typename Value>
inline void RemoveIfEqual( Container &c, const Value &v )
{
	c.erase( remove(c.begin(), c.end(), v), c.end() );
}

/* Helper for ConvertValue(). */
template<typename TO, typename FROM>
struct ConvertValueHelper
{
	explicit ConvertValueHelper( FROM *pVal ): m_pFromValue(pVal)
	{
		m_ToValue = static_cast<TO>( *m_pFromValue );
	}

	~ConvertValueHelper()
	{
		*m_pFromValue = static_cast<FROM>( m_ToValue );
	}

	TO &operator *() { return m_ToValue; }
	operator TO *() { return &m_ToValue; }

private:
	FROM *m_pFromValue;
	TO m_ToValue;
};

/* Safely temporarily convert between types.  For example,
 *
 * float f = 10.5;
 * *ConvertValue<int>(&f) = 12;
 */
template<typename TO, typename FROM>
ConvertValueHelper<TO, FROM> ConvertValue( FROM *pValue )
{
	return ConvertValueHelper<TO, FROM>( pValue );
}

/* Safely add an integer to an enum.
 *
 * This is illegal:
 *
 *  ((int&)val) += iAmt;
 *
 * It breaks aliasing rules; the compiler is allowed to assume that "val" doesn't
 * change (unless it's declared volatile), and in some cases, you'll end up getting
 * old values for "val" following the add.  (What's probably really happening is
 * that the memory location is being added to, but the value is stored in a register,
 * and breaking aliasing rules means the compiler doesn't know that the register
 * value is invalid.)
 */
template<typename T>
static inline void enum_add( T &val, int iAmt )
{
	val = static_cast<T>( val + iAmt );
}

template<typename T>
static inline T enum_add2( T val, int iAmt )
{
	return static_cast<T>( val + iAmt );
}

template<typename T>
static inline T enum_cycle( T val, int iMax, int iAmt = 1 )
{
	int iVal = val + iAmt;
	iVal %= iMax;
	return static_cast<T>( iVal );
}

namespace Endian
{
	// When std::endian is supported by all desired compilers, we can eliminate the #ifdefs
	// (At least) the current compiler used for the Ubuntu 20.04 build does not support this.
#if defined(__BYTE_ORDER__)
	inline constexpr bool little = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
	inline constexpr bool big    = __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__;
#elif defined(_WIN32)
	inline constexpr bool little = true;
	inline constexpr bool big    = false;
#else
#error "unknown byte order"
#endif
}

#ifdef _WIN32
#define Swap32(n) _byteswap_ulong(n)
#define Swap24(n) _byteswap_ulong(n) >> 8
#define Swap16(n) _byteswap_ushort(n)
#else
#define Swap32(n) __builtin_bswap32(n)
#define Swap24(n) __builtin_bswap32(n) >> 8
#define Swap16(n) __builtin_bswap16(n)
#endif

inline std::uint32_t Swap32LE( std::uint32_t n ) { return Endian::little ? n : Swap32( n ); }
inline std::uint32_t Swap24LE( std::uint32_t n ) { return Endian::little ? n : Swap24( n ); }
inline std::uint16_t Swap16LE( std::uint16_t n ) { return Endian::little ? n : Swap16( n ); }
inline std::uint32_t Swap32BE( std::uint32_t n ) { return Endian::big    ? n : Swap32( n ); }
inline std::uint32_t Swap24BE( std::uint32_t n ) { return Endian::big    ? n : Swap24( n ); }
inline std::uint16_t Swap16BE( std::uint16_t n ) { return Endian::big    ? n : Swap16( n ); }

class MersenneTwister : public std::mt19937
{
public:
	MersenneTwister( int iSeed = 0 ) : std::mt19937( iSeed == 0 ? time( nullptr ) : iSeed ) {}
};

typedef MersenneTwister RandomGen;

extern RandomGen g_RandomNumberGenerator;

/**
 * @brief Return a float between the low and high values.
 * @param fLow the low value, inclusive.
 * @param fHigh the high value, inclusive.
 * @return the random float.
 */
inline float RandomFloat( float fLow, float fHigh )
{
	std::uniform_real_distribution<> dist( fLow, fHigh );
	return dist( g_RandomNumberGenerator );
}

/**
 * @brief Generate a random float between 0 inclusive and 1 exclusive.
 * @return the random float.
 */
inline float RandomFloat()
{
	return RandomFloat( 0, 1 );
}

// Returns an integer between nLow and nHigh inclusive
inline int RandomInt( int nLow, int nHigh )
{
	std::uniform_int_distribution<> dist( nLow, nHigh );
	return dist( g_RandomNumberGenerator );
}

// Returns an integer between 0 and n-1 inclusive (replacement for rand() % n).
inline int RandomInt( int n )
{
	return RandomInt( 0, n - 1 );
}


// Simple function for generating random numbers
inline float randomf( const float low=-1.0f, const float high=1.0f )
{
	return RandomFloat( low, high );
}

/* return f rounded to the nearest multiple of fRoundInterval */
inline float Quantize( const float f, const float fRoundInterval )
{
	return int( (f + fRoundInterval/2)/fRoundInterval ) * fRoundInterval;
}

inline int Quantize( const int i, const int iRoundInterval )
{
	return int( (i + iRoundInterval/2)/iRoundInterval ) * iRoundInterval;
}

/* return f truncated to the nearest multiple of fTruncInterval */
inline float ftruncf( const float f, const float fTruncInterval )
{
	return int( (f)/fTruncInterval ) * fTruncInterval;
}

/* Return i rounded up to the nearest multiple of iInterval. */
inline int QuantizeUp( int i, int iInterval )
{
	return int( (i+iInterval-1)/iInterval ) * iInterval;
}

inline float QuantizeUp( float i, float iInterval )
{
	return std::ceil( i/iInterval ) * iInterval;
}

/* Return i rounded down to the nearest multiple of iInterval. */
inline int QuantizeDown( int i, int iInterval )
{
	return int( (i-iInterval+1)/iInterval ) * iInterval;
}

inline float QuantizeDown( float i, float iInterval )
{
	return std::floor( i/iInterval ) * iInterval;
}

// Move val toward other_val by to_move.
void fapproach( float& val, float other_val, float to_move );

/* Return a positive x mod y. */
float fmodfp( float x, float y );

int power_of_two( int input );
bool IsAnInt( const RString &s );
bool IsHexVal( const RString &s );
RString BinaryToHex( const void *pData_, std::size_t iNumBytes );
RString BinaryToHex( const RString &sString );
bool HexToBinary( const RString &s, unsigned char *stringOut );
bool HexToBinary( const RString &s, RString *sOut );
float HHMMSSToSeconds( const RString &sHMS );
RString SecondsToHHMMSS( float fSecs );
RString SecondsToMSSMsMs( float fSecs );
RString SecondsToMMSSMsMs( float fSecs );
RString SecondsToMMSSMsMsMs( float fSecs );
RString SecondsToMSS( float fSecs );
RString SecondsToMMSS( float fSecs );
RString PrettyPercent( float fNumerator, float fDenominator );
inline RString PrettyPercent( int fNumerator, int fDenominator ) { return PrettyPercent( float(fNumerator), float(fDenominator) ); }
RString Commify( int iNum );
RString Commify(const RString& num, const RString& sep= ",", const RString& dot= ".");
RString FormatNumberAndSuffix( int i );


struct tm GetLocalTime();

RString ssprintf( const char *fmt, ...) PRINTF(1,2);
RString vssprintf( const char *fmt, va_list argList );
RString ConvertI64FormatString( const RString &sStr );

/*
 * Splits a Path into 4 parts (Directory, Drive, Filename, Extention).  Supports UNC path names.
 * If Path is a directory (eg. c:\games\stepmania"), append a slash so the last
 * element will end up in Dir, not FName: "c:\games\stepmania\".
 * */
void splitpath( const RString &Path, RString &Dir, RString &Filename, RString &Ext );
RString custom_songify_path(RString const& path);

RString SetExtension( const RString &path, const RString &ext );
RString GetExtension( const RString &sPath );
RString GetFileNameWithoutExtension( const RString &sPath );
void MakeValidFilename( RString &sName );

bool FindFirstFilenameContaining(
	const std::vector<RString>& filenames, RString& out,
	const std::vector<RString>& starts_with,
	const std::vector<RString>& contains, const std::vector<RString>& ends_with);

extern const wchar_t INVALID_CHAR;

int utf8_get_char_len( char p );
bool utf8_to_wchar( const char *s, std::size_t iLength, unsigned &start, wchar_t &ch );
bool utf8_to_wchar_ec( const RString &s, unsigned &start, wchar_t &ch );
void wchar_to_utf8( wchar_t ch, RString &out );
wchar_t utf8_get_char( const RString &s );
bool utf8_is_valid( const RString &s );
void utf8_remove_bom( RString &s );
void MakeUpper( char *p, std::size_t iLen );
void MakeLower( char *p, std::size_t iLen );
void MakeUpper( wchar_t *p, std::size_t iLen );
void MakeLower( wchar_t *p, std::size_t iLen );

// TODO: Have the three functions below be moved to better locations.
float StringToFloat( const RString &sString );
bool StringToFloat( const RString &sString, float &fOut );
// Better than IntToString because you can check for success.
template<class T>
inline bool operator>>(const RString& lhs, T& rhs)
{
	return !!(std::istringstream(lhs) >> rhs);
}

// Exception-safe wrappers around stoi and friends
// Additional argument exceptVal will be returned if the conversion couldn't be performed
int StringToInt( const std::string& str, std::size_t* pos = 0, int base = 10, int exceptVal = 0 );
long StringToLong( const std::string& str, std::size_t* pos = 0, int base = 10, long exceptVal = 0 );
long long StringToLLong( const std::string& str, std::size_t* pos = 0, int base = 10, long long exceptVal = 0 );

RString WStringToRString( const std::wstring &sString );
RString WcharToUTF8( wchar_t c );
std::wstring RStringToWstring( const RString &sString );

struct LanguageInfo
{
	const char *szIsoCode;
	const char *szEnglishName;
};
void GetLanguageInfos( std::vector<const LanguageInfo*> &vAddTo );
const LanguageInfo *GetLanguageInfo( const RString &sIsoCode );
RString GetLanguageNameFromISO639Code( RString sName );

// Splits a RString into an std::vector<RString> according the Delimitor.
void split( const RString &sSource, const RString &sDelimitor, std::vector<RString>& asAddIt, const bool bIgnoreEmpty = true );
void split( const std::wstring &sSource, const std::wstring &sDelimitor, std::vector<std::wstring> &asAddIt, const bool bIgnoreEmpty = true );

/* In-place split. */
void split( const RString &sSource, const RString &sDelimitor, int &iBegin, int &iSize, const bool bIgnoreEmpty = true );
void split( const std::wstring &sSource, const std::wstring &sDelimitor, int &iBegin, int &iSize, const bool bIgnoreEmpty = true );

/* In-place split of partial string. */
void split( const RString &sSource, const RString &sDelimitor, int &iBegin, int &iSize, int iLen, const bool bIgnoreEmpty ); /* no default to avoid ambiguity */
void split( const std::wstring &sSource, const std::wstring &sDelimitor, int &iBegin, int &iSize, int iLen, const bool bIgnoreEmpty );

// Joins a std::vector<RString> to create a RString according the Deliminator.
RString join( const RString &sDelimitor, const std::vector<RString>& sSource );
RString join( const RString &sDelimitor, std::vector<RString>::const_iterator begin, std::vector<RString>::const_iterator end );

// These methods escapes a string for saving in a .sm or .crs file
RString SmEscape(const RString &sUnescaped, const std::vector<char> charsToEscape = {'\\', ':', ';'});
RString SmEscape( const char *cUnescaped, int len, const std::vector<char> charsToEscape = {'\\', ':', ';'} );
// Escapes each element in a std::vector<RString>, returns a new vector
std::vector<RString> SmEscape(const std::vector<RString> &vUnescaped, const std::vector<char> charsToEscape = {'\\', ':', ';'});

RString SmUnescape( const RString &sEscaped );

// These methods "escape" a string for .dwi by turning = into -, ] into I, etc.  That is "lossy".
RString DwiEscape( const RString &sUnescaped );
RString DwiEscape( const char *cUnescaped, int len );

RString GetCwd();

void SetCommandlineArguments( int argc, char **argv );
void GetCommandLineArguments( int &argc, char **&argv );
bool GetCommandlineArgument( const RString &option, RString *argument=nullptr, int iIndex=0 );
extern int g_argc;
extern char **g_argv;

void CRC32( unsigned int &iCRC, const void *pBuffer, std::size_t iSize );
unsigned int GetHashForString( const RString &s );
unsigned int GetHashForFile( const RString &sPath );
unsigned int GetHashForDirectory( const RString &sDir );	// a hash value that remains the same as long as nothing in the directory has changed
bool DirectoryIsEmpty( const RString &sPath );

bool CompareRStringsAsc( const RString &sStr1, const RString &sStr2 );
bool CompareRStringsDesc( const RString &sStr1, const RString &sStr2 );
void SortRStringArray( std::vector<RString> &asAddTo, const bool bSortAscending = true );

/* Find the mean and standard deviation of all numbers in [start,end). */
float calc_mean( const float *pStart, const float *pEnd );
/* When bSample is true, it calculates the square root of an unbiased estimator for the population
 * variance. Note that this is not an unbiased estimator for the population standard deviation but
 * it is close and an unbiased estimator is complicated (apparently).
 * When the entire population is known, bSample should be false to calculate the exact standard
 * deviation. */
float calc_stddev( const float *pStart, const float *pEnd, bool bSample = false );

/*
 * Find the slope, intercept, and error of a linear least squares regression
 * of the points given.  Error is returned as the sqrt of the average squared
 * Y distance from the chosen line.
 * Returns true on success, false on failure.
 */
bool CalcLeastSquares( const std::vector<std::pair<float, float> > &vCoordinates,
                       float &fSlope, float &fIntercept, float &fError );

/*
 * This method throws away any points that are more than fCutoff away from
 * the line defined by fSlope and fIntercept.
 */
void FilterHighErrorPoints( std::vector<std::pair<float, float> > &vCoordinates,
                            float fSlope, float fIntercept, float fCutoff );

template<class T1, class T2>
int FindIndex( T1 begin, T1 end, const T2 *p )
{
	T1 iter = find( begin, end, p );
	if( iter == end )
		return -1;
	return iter - begin;
}

/* Useful for objects with no operator-, eg. map::iterator (more convenient than advance). */
template<class T>
inline T Increment( T a ) { ++a; return a; }
template<class T>
inline T Decrement( T a ) { --a; return a; }

void TrimLeft( RString &sStr, const char *szTrim = "\r\n\t " );
void TrimRight( RString &sStr, const char *szTrim = "\r\n\t " );
void Trim( RString &sStr, const char *szTrim = "\r\n\t " );
void StripCrnl( RString &sStr );
bool BeginsWith( const RString &sTestThis, const RString &sBeginning );
bool EndsWith( const RString &sTestThis, const RString &sEnding );
RString URLEncode( const RString &sStr );

void StripCvsAndSvn( std::vector<RString> &vs ); // Removes various versioning system metafolders.
void StripMacResourceForks( std::vector<RString> &vs ); // Removes files starting with "._"

RString DerefRedir( const RString &sPath );
bool GetFileContents( const RString &sPath, RString &sOut, bool bOneLine = false );
bool GetFileContents( const RString &sFile, std::vector<RString> &asOut );

class Regex
{
public:
	Regex( const RString &sPat = "" );
	Regex( const Regex &rhs );
	Regex &operator=( const Regex &rhs );
	~Regex();
	bool IsSet() const { return !m_sPattern.empty(); }
	void Set( const RString &str );
	bool Compare( const RString &sStr );
	bool Compare( const RString &sStr, std::vector<RString> &asMatches );
	bool Replace( const RString &sReplacement, const RString &sSubject, RString &sOut );

private:
	void Compile();
	void Release();

	void *m_pReg;
	unsigned m_iBackrefs;
	RString m_sPattern;
};


void ReplaceEntityText( RString &sText, const std::map<RString,RString> &m );
void ReplaceEntityText( RString &sText, const std::map<char,RString> &m );
void Replace_Unicode_Markers( RString &Text );
RString WcharDisplayText( wchar_t c );

RString Basename( const RString &dir );
RString Dirname( const RString &dir );
RString Capitalize( const RString &s );

#if defined(HAVE_UNISTD_H)
#include <unistd.h> /* correct place with correct definitions */
#endif

extern unsigned char g_UpperCase[256];
extern unsigned char g_LowerCase[256];

/* ASCII-only case insensitivity. */
struct char_traits_char_nocase: public std::char_traits<char>
{
	static inline bool eq( char c1, char c2 )
	{ return g_UpperCase[(unsigned char)c1] == g_UpperCase[(unsigned char)c2]; }

	static inline bool ne( char c1, char c2 )
	{ return g_UpperCase[(unsigned char)c1] != g_UpperCase[(unsigned char)c2]; }

	static inline bool lt( char c1, char c2 )
	{ return g_UpperCase[(unsigned char)c1] < g_UpperCase[(unsigned char)c2]; }

	static int compare( const char* s1, const char* s2, std::size_t n )
	{
		int ret = 0;
		while( n-- )
		{
			ret = fasttoupper(*s1++) - fasttoupper(*s2++);
			if( ret != 0 )
				break;
		}
		return ret;
	}

	static inline char fasttoupper(char a)
	{
		return g_UpperCase[(unsigned char)a];
	}

	static const char *find( const char* s, int n, char a )
	{
		a = fasttoupper(a);
		while( n-- > 0 && fasttoupper(*s) != a )
			++s;

		if(fasttoupper(*s) == a)
			return s;
		return nullptr;
	}
};
typedef std::basic_string<char,char_traits_char_nocase> istring;

/* Compatibility/convenience shortcuts. These are actually defined in RageFileManager.h, but
 * declared here since they're used in many places. */
void GetDirListing( const RString &sPath, std::vector<RString> &AddTo, bool bOnlyDirs=false, bool bReturnPathToo=false );
void GetDirListingRecursive( const RString &sDir, const RString &sMatch, std::vector<RString> &AddTo );	/* returns path too */
void GetDirListingRecursive( RageFileDriver *prfd, const RString &sDir, const RString &sMatch, std::vector<RString> &AddTo );	/* returns path too */
bool DeleteRecursive( const RString &sDir );	/* delete the dir and all files/subdirs inside it */
bool DeleteRecursive( RageFileDriver *prfd, const RString &sDir );	/* delete the dir and all files/subdirs inside it */
bool DoesFileExist( const RString &sPath );
bool IsAFile( const RString &sPath );
bool IsADirectory( const RString &sPath );
int GetFileSizeInBytes( const RString &sFilePath );

// call FixSlashesInPlace on any path that came from the user
void FixSlashesInPlace( RString &sPath );
void CollapsePath( RString &sPath, bool bRemoveLeadingDot=false );

/** @brief Utilities for converting the RStrings. */
namespace StringConversion
{
	template<typename T>
	bool FromString( const RString &sValue, T &out );

	template<typename T>
	RString ToString( const T &value );

	template<> inline bool FromString<RString>( const RString &sValue, RString &out ) { out = sValue; return true; }
	template<> inline RString ToString<RString>( const RString &value ) { return value; }
}

class RageFileBasic;
bool FileCopy( const RString &sSrcFile, const RString &sDstFile );
bool FileCopy( RageFileBasic &in, RageFileBasic &out, RString &sError, bool *bReadError = nullptr );

template<class T>
void GetAsNotInBs( const std::vector<T> &as, const std::vector<T> &bs, std::vector<T> &difference )
{
	std::vector<T> bsUnmatched = bs;
	// Cannot use FOREACH_CONST here because std::vector<T>::const_iterator is an implicit type.
	for( typename std::vector<T>::const_iterator a = as.begin(); a != as.end(); ++a )
	{
		typename std::vector<T>::iterator iter = find( bsUnmatched.begin(), bsUnmatched.end(), *a );
		if( iter != bsUnmatched.end() )
			bsUnmatched.erase( iter );
		else
			difference.push_back( *a );
	}
}

template<class T>
void GetConnectsDisconnects( const std::vector<T> &before, const std::vector<T> &after, std::vector<T> &disconnects, std::vector<T> &connects )
{
	GetAsNotInBs( before, after, disconnects );
	GetAsNotInBs( after, before, connects );
}


#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2005
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
