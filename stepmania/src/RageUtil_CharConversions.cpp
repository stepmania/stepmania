#include "global.h"
#include "RageUtil_CharConversions.h"

#include "RageUtil.h"
#include "RageLog.h"

#if defined(_WINDOWS)

#include "windows.h"

/* Convert from the given codepage to UTF-8.  Return true if successful. */
static bool CodePageConvert(CString &txt, int cp)
{
	if( txt.size() == 0 )
		return true;

	int size = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, txt.data(), txt.size(), NULL, 0);
	if( size == 0 )
	{
		LOG->Trace("%s\n", werr_ssprintf(GetLastError(), "err: ").c_str());
		return false; /* error */
	}

	wstring out;
	out.append(size, ' ');
	/* Nonportable: */
	size = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, txt.data(), txt.size(), (wchar_t *) out.data(), size);
	ASSERT( size != 0 );

	txt = WStringToCString(out);
	return true;
}

static bool AttemptEnglishConversion( CString &txt ) { return CodePageConvert( txt, 1252 ); }
static bool AttemptKoreanConversion( CString &txt ) { return CodePageConvert( txt, 949 ); }
static bool AttemptJapaneseConversion( CString &txt ) { return CodePageConvert( txt, 932 ); }

#elif defined(HAVE_ICONV)
#include <errno.h>
#include <iconv.h>

static bool ConvertFromCharset( CString &txt, const char *charset )
{
	if ( txt.size() == 0 )
		return true;

	iconv_t converter = iconv_open( "UTF-8", charset );
	if( converter == (iconv_t) -1 )
	{
		LOG->MapLog( ssprintf("conv %s", charset), "iconv_open(%s): %s", charset, strerror(errno) );
		return false;
	}

	/* Copy the string into a char* for iconv */
	char *txtin = new char[ txt.size() + 1 ];
	strcpy( txtin, txt );
	size_t inleft = strlen( txtin );

	/* Create a new string with enough room for the new conversion */
	char *txtout = new char[ (txt.size() + 1) * 5 ];
	txtout = (char *)memset( txtout, 0, (txt.size() + 1) * 5 );
	size_t outleft = inleft * 5;
	size_t outsize = outleft;

	size_t size = iconv( converter, &txtin, &inleft, &txtout, &outleft );

	/* Redirect the output pointer back to the beginning of the string */
	outsize -= outleft;
	txtout -= outsize;
	iconv_close( converter );

	if( size == (size_t)(-1) )
	{
		LOG->Trace( "%s\n", strerror( errno ) );
		return false; /* Returned an error */
	}
	if( !strlen(txtout) )
		return false; /* Conversion failed */

	txt = txtout;
	return true;
}

static bool AttemptEnglishConversion( CString &txt ) { return ConvertFromCharset( txt, "CP1252" ); }
static bool AttemptKoreanConversion( CString &txt ) { return ConvertFromCharset( txt, "CP949" ); }
static bool AttemptJapaneseConversion( CString &txt ) { return ConvertFromCharset( txt, "CP932" ); }

#else

/* No converters are available, so all fail--we only accept UTF-8. */
static bool AttemptEnglishConversion( CString &txt ) { return false; }
static bool AttemptKoreanConversion( CString &txt ) { return false; }
static bool AttemptJapaneseConversion( CString &txt ) { return false; }

#endif

bool ConvertString(CString &str, const CString &encodings)
{
	CStringArray lst;
	split(encodings, ",", lst);

	for(unsigned i = 0; i < lst.size(); ++i)
	{
		if( lst[i] == "utf-8" )
		{
			/* Is the string already valid utf-8? */
			if( utf8_is_valid(str) )
				return true;
			continue;
		}
		if( lst[i] == "english" )
		{
			if(AttemptEnglishConversion(str))
				return true;
			continue;
		}

		if( lst[i] == "japanese" )
		{
			if(AttemptJapaneseConversion(str))
				return true;
			continue;
		}

		if( lst[i] == "korean" )
		{
			if(AttemptKoreanConversion(str))
				return true;
			continue;
		}

		RageException::Throw( "Unexpected conversion string \"%s\" (string \"%s\")",
						lst[i].c_str(), str.c_str() );
	}

	return false;
}

/* Written by Glenn Maynard.  In the public domain; there are so many
 * simple conversion interfaces that restricting them is silly. */
