#include "global.h"
#include "RageUtil_CharConversions.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageString.hpp"
#include "RageUnicode.hpp"

using std::vector;
using std::wstring;

#if defined(_WINDOWS)

#include "archutils/Win32/ErrorStrings.h"
#include <windows.h>

/* Convert from the given codepage to UTF-8.  Return true if successful. */
static bool CodePageConvert( std::string &sText, int iCodePage )
{
	int iSize = MultiByteToWideChar( iCodePage, MB_ERR_INVALID_CHARS, sText.data(), sText.size(), nullptr, 0 );
	if( iSize == 0 )
	{
		LOG->Trace( "%s\n", werr_format(GetLastError(), "err: ").c_str() );
		return false; /* error */
	}

	wstring sOut;
	sOut.append( iSize, ' ' );
	/* Nonportable: */
	iSize = MultiByteToWideChar( iCodePage, MB_ERR_INVALID_CHARS, sText.data(), sText.size(), (wchar_t *) sOut.data(), iSize );
	ASSERT( iSize != 0 );

	sText = WStringToString( sOut );
	return true;
}

static bool AttemptEnglishConversion( std::string &sText ) { return CodePageConvert( sText, 1252 ); }
static bool AttemptKoreanConversion( std::string &sText ) { return CodePageConvert( sText, 949 ); }
static bool AttemptJapaneseConversion( std::string &sText ) { return CodePageConvert( sText, 932 ); }

#elif defined(HAVE_ICONV)
#include <errno.h>
#include <iconv.h>

static bool ConvertFromCharset( std::string &sText, const char *szCharset )
{
	iconv_t converter = iconv_open( "UTF-8", szCharset );
	if( converter == (iconv_t) -1 )
	{
		LOG->MapLog( fmt::sprintf("conv %s", szCharset), "iconv_open(%s): %s", szCharset, strerror(errno) );
		return false;
	}

	/* Copy the string into a char* for iconv */
	ICONV_CONST char *szTextIn = const_cast<ICONV_CONST char*>( sText.data() );
	size_t iInLeft = sText.size();

	/* Create a new string with enough room for the new conversion */
	std::string sBuf;
	sBuf.resize( sText.size() * 5 );

	char *sTextOut = const_cast<char*>( sBuf.data() );
	size_t iOutLeft = sBuf.size();
	size_t size = iconv( converter, &szTextIn, &iInLeft, &sTextOut, &iOutLeft );

	iconv_close( converter );

	if( size == (size_t)(-1) )
	{
		LOG->Trace( "%s\n", strerror( errno ) );
		return false; /* Returned an error */
	}

	if( iInLeft != 0 )
	{
		LOG->Warn( "iconv(UTF-8,%s) for \"%s\": whole buffer not converted (%i left)", szCharset, sText.c_str(), int(iInLeft) );
		return false;
	}

	if( sBuf.size() == iOutLeft )
		return false; /* Conversion failed */

	sBuf.resize( sBuf.size()-iOutLeft );

	sText = sBuf;
	return true;
}

static bool AttemptEnglishConversion( std::string &sText ) { return ConvertFromCharset( sText, "CP1252" ); }
static bool AttemptKoreanConversion( std::string &sText ) { return ConvertFromCharset( sText, "CP949" ); }
static bool AttemptJapaneseConversion( std::string &sText ) { return ConvertFromCharset( sText, "CP932" ); }

#elif defined(MACOSX)
#include <CoreFoundation/CoreFoundation.h>

static bool ConvertFromCP( std::string &sText, int iCodePage )
{
	CFStringEncoding encoding = CFStringConvertWindowsCodepageToEncoding( iCodePage );

	if( encoding == kCFStringEncodingInvalidId )
		return false;

	CFStringRef old = CFStringCreateWithCString( kCFAllocatorDefault, sText.c_str(), encoding );

	if( old == nullptr )
		return false;
	const size_t size = CFStringGetMaximumSizeForEncoding( CFStringGetLength(old), kCFStringEncodingUTF8 );

	char *buf = new char[size+1];
	buf[0] = '\0';
	bool result = CFStringGetCString( old, buf, size, kCFStringEncodingUTF8 );
	sText = buf;
	delete[] buf;
	CFRelease( old );
	return result;
}

static bool AttemptEnglishConversion( std::string &sText ) { return ConvertFromCP( sText, 1252 ); }
static bool AttemptKoreanConversion( std::string &sText ) { return ConvertFromCP( sText, 949 ); }
static bool AttemptJapaneseConversion( std::string &sText ) { return ConvertFromCP( sText, 932 ); }

#else

/* No converters are available, so all fail--we only accept UTF-8. */
static bool AttemptEnglishConversion( std::string &sText ) { return false; }
static bool AttemptKoreanConversion( std::string &sText ) { return false; }
static bool AttemptJapaneseConversion( std::string &sText ) { return false; }

#endif

bool ConvertString( std::string &str, const std::string &encodings )
{
	if( str.empty() )
		return true;

	auto lst = Rage::split( encodings, "," );

	for(unsigned i = 0; i < lst.size(); ++i)
	{
		if( lst[i] == "utf-8" )
		{
			/* Is the string already valid utf-8? */
			if( Rage::utf8_is_valid(str) )
				return true;
			continue;
		}
		if( lst[i] == "english" )
		{
			if( AttemptEnglishConversion(str) )
				return true;
			continue;
		}

		if( lst[i] == "japanese" )
		{
			if( AttemptJapaneseConversion(str) )
				return true;
			continue;
		}

		if( lst[i] == "korean" )
		{
			if( AttemptKoreanConversion(str) )
				return true;
			continue;
		}

		RageException::Throw( "Unexpected conversion string \"%s\" (string \"%s\").",
				      lst[i].c_str(), str.c_str() );
	}

	return false;
}

/* Written by Glenn Maynard.  In the public domain; there are so many
 * simple conversion interfaces that restricting them is silly. */
