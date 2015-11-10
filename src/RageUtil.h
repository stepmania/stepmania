/** @brief RageUtil - Miscellaneous helper macros and functions. */

#ifndef RAGE_UTIL_H
#define RAGE_UTIL_H

#include <map>
#include <vector>
#include <sstream>

#include "RageMath.hpp"

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

// TODO: Rename this to TryClamp or something a little more accurate
// so as to not be confusing.
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
	x = fmodf(x,n);
}

inline float fracf( float f ) { return f - truncf(f); }

template<class T>
void CircularShift( std::vector<T> &v, int dist )
{
	for( int i = abs(dist); i>0; i-- )
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


/* We only have unsigned swaps; byte swapping a signed value doesn't make sense.
 *
 * Platform-specific, optimized versions are defined in arch_setup, with the names
 * ArchSwap32, ArchSwap24, and ArchSwap16; we define them to their real names here,
 * to force inclusion of this file when swaps are in use (to prevent different dependencies
 * on different systems).
 */
#ifdef HAVE_BYTE_SWAPS
#define Swap32 ArchSwap32
#define Swap24 ArchSwap24
#define Swap16 ArchSwap16
#else
inline uint32_t Swap32( uint32_t n )
{
	return (n >> 24) |
		((n >>  8) & 0x0000FF00) |
		((n <<  8) & 0x00FF0000) |
		(n << 24);
}

inline uint32_t Swap24( uint32_t n )
{
	return Swap32( n ) >> 8; // xx223344 -> 443322xx -> 00443322
}

inline uint16_t Swap16( uint16_t n )
{
	return (n >>  8) | (n <<  8);
}
#endif

#if defined(ENDIAN_LITTLE)
inline uint32_t Swap32LE( uint32_t n ) { return n; }
inline uint32_t Swap24LE( uint32_t n ) { return n; }
inline uint16_t Swap16LE( uint16_t n ) { return n; }
inline uint32_t Swap32BE( uint32_t n ) { return Swap32( n ); }
inline uint32_t Swap24BE( uint32_t n ) { return Swap24( n ); }
inline uint16_t Swap16BE( uint16_t n ) { return Swap16( n ); }
#else
inline uint32_t Swap32BE( uint32_t n ) { return n; }
inline uint32_t Swap24BE( uint32_t n ) { return n; }
inline uint16_t Swap16BE( uint16_t n ) { return n; }
inline uint32_t Swap32LE( uint32_t n ) { return Swap32( n ); }
inline uint32_t Swap24LE( uint32_t n ) { return Swap24( n ); }
inline uint16_t Swap16LE( uint16_t n ) { return Swap16( n ); }
#endif

struct MersenneTwister
{
	MersenneTwister( int iSeed = 0 ); // 0 = time()
	int operator()(); // returns [0,2^31-1]
	int operator()( int n ) // returns [0,n)
	{
		return (*this)() % n;
	}

	void Reset( int iSeed );

private:
	static int Temper( int iValue );
	void GenerateValues();

	int m_Values[624];
	int m_iNext;
};
typedef MersenneTwister RandomGen;

extern RandomGen g_RandomNumberGenerator;

/**
 * @brief Generate a random float between 0 inclusive and 1 exclusive.
 * @return the random float.
 */
inline float RandomFloat()
{
	return g_RandomNumberGenerator() / 2147483648.0f;
}

/**
 * @brief Return a float between the low and high values.
 * @param fLow the low value, inclusive.
 * @param fHigh the high value, inclusive.
 * @return the random float.
 */
inline float RandomFloat( float fLow, float fHigh )
{
  return Rage::scale( RandomFloat(), 0.0f, 1.0f, fLow, fHigh );
}

// Returns an integer between nLow and nHigh inclusive
inline int RandomInt( int nLow, int nHigh )
{
	return int( g_RandomNumberGenerator(nHigh - nLow + 1) + nLow );
}

// Returns an integer between 0 and n-1 inclusive (replacement for rand() % n).
inline int RandomInt( int n )
{
	return int( g_RandomNumberGenerator(n) );
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
	return ceilf( i/iInterval ) * iInterval;
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
RString BinaryToHex( const void *pData_, int iNumBytes );
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

RString unique_name(RString const& type);

struct tm GetLocalTime();

/*
 * Splits a Path into 4 parts (Directory, Drive, Filename, Extention).  Supports UNC path names.
 * If Path is a directory (eg. c:\games\stepmania"), append a slash so the last
 * element will end up in Dir, not FName: "c:\games\stepmania\".
 * */
void splitpath( const RString &Path, RString &Dir, RString &Filename, RString &Ext );

RString SetExtension( const RString &path, const RString &ext );
RString GetExtension( const RString &sPath );
RString GetFileNameWithoutExtension( const RString &sPath );
void MakeValidFilename( RString &sName );

bool FindFirstFilenameContaining(
	std::vector<std::string> const & filenames, std::string& out,
	std::vector<std::string> const & starts_with,
	std::vector<std::string> const & contains, std::vector<std::string> const & ends_with);

/**
 * @brief Have a standard way of converting Strings to integers.
 * @param sString the string to convert.
 * @return the integer we are after. */
int StringToInt( const RString &sString );
/**
 * @brief Have a standard way of converting integers to Strings.
 * @param iNum the integer to convert.
 * @return the string we are after. */
RString IntToString( const int &iNum );
float StringToFloat( const RString &sString );
RString FloatToString( const float &num );
bool StringToFloat( const RString &sString, float &fOut );
// Better than IntToString because you can check for success.
template<class T>
inline bool operator>>(const RString& lhs, T& rhs)
{
	return !!(std::istringstream(lhs) >> rhs);
}

RString WStringToRString( const std::wstring &sString );
RString WcharToUTF8( wchar_t c );
std::wstring RStringToWstring( const RString &sString );

struct LanguageInfo
{
	std::string isoCode;
	std::string englishName;
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

// These methods escapes a string for saving in a .sm or .crs file
RString SmEscape( const RString &sUnescaped );
RString SmEscape( const char *cUnescaped, int len );

// These methods "escape" a string for .dwi by turning = into -, ] into I, etc.  That is "lossy".
RString DwiEscape( const RString &sUnescaped );
RString DwiEscape( const char *cUnescaped, int len );

RString GetCwd();

void SetCommandlineArguments( int argc, char **argv );
void GetCommandLineArguments( int &argc, char **&argv );
bool GetCommandlineArgument( const RString &option, RString *argument=nullptr, int iIndex=0 );
extern int g_argc;
extern char **g_argv;

void CRC32( unsigned int &iCRC, const void *pBuffer, size_t iSize );
unsigned int GetHashForString( const RString &s );
unsigned int GetHashForFile( const RString &sPath );
unsigned int GetHashForDirectory( const RString &sDir );	// a hash value that remains the same as long as nothing in the directory has changed
bool DirectoryIsEmpty( const RString &sPath );

bool CompareRStringsAsc( const RString &sStr1, const RString &sStr2 );
bool CompareRStringsDesc( const RString &sStr1, const RString &sStr2 );
void SortRStringArray( std::vector<std::string> &asAddTo, const bool bSortAscending = true );

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
bool CalcLeastSquares( const std::vector< std::pair<float, float> > &vCoordinates,
                       float &fSlope, float &fIntercept, float &fError );

/*
 * This method throws away any points that are more than fCutoff away from
 * the line defined by fSlope and fIntercept.
 */
void FilterHighErrorPoints( std::vector< std::pair<float, float> > &vCoordinates,
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

void StripCrnl( RString &sStr );
RString URLEncode( const RString &sStr );

void StripCvsAndSvn( std::vector<std::string> &vs ); // Removes various versioning system metafolders.
void StripMacResourceForks( std::vector<std::string> &vs ); // Removes files starting with "._"

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


void ReplaceEntityText( std::string &sText, std::map<std::string,std::string> const &m );
void ReplaceEntityText( std::string &sText, std::map<char,std::string> const &m );
void Replace_Unicode_Markers( std::string &Text );
RString WcharDisplayText( wchar_t c );

RString Capitalize( const RString &s );

#if defined(HAVE_UNISTD_H)
#include <unistd.h> /* correct place with correct definitions */
#endif

extern unsigned char g_UpperCase[256];
extern unsigned char g_LowerCase[256];

/* Compatibility/convenience shortcuts. These are actually defined in RageFileManager.h, but
 * declared here since they're used in many places. */
void GetDirListing( std::string const &sPath, std::vector<std::string> &AddTo, bool bOnlyDirs=false, bool bReturnPathToo=false );
void GetDirListingRecursive( std::string const &sDir, std::string const &sMatch, std::vector<std::string> &AddTo );	/* returns path too */
void GetDirListingRecursive( RageFileDriver *prfd, std::string const &sDir, std::string const &sMatch, std::vector<std::string> &AddTo );	/* returns path too */
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
	template<> inline bool FromString<std::string> ( RString const &sValue, std::string &outParam)
	{
		outParam = sValue;
		return true;
	}
	template<> inline RString ToString<std::string>( std::string const &value )
	{
		return value;
	}
}

class RageFileBasic;
bool FileCopy( const RString &sSrcFile, const RString &sDstFile );
bool FileCopy( RageFileBasic &in, RageFileBasic &out, RString &sError, bool *bReadError = nullptr );

template<class T>
void GetAsNotInBs( const std::vector<T> &as, const std::vector<T> &bs, std::vector<T> &difference )
{
	std::vector<T> bsUnmatched = bs;
	// Cannot use FOREACH_CONST here because std::vector<T>::const_iterator is an implicit type.
	for (typename std::vector<T>::const_iterator a = as.begin(); a != as.end(); ++a )
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
