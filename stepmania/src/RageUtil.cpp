#include "global.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "RageLog.h"
#include "RageFile.h"
#include "Foreach.h"
#include "LocalizedString.h"
#include "LuaManager.h"
#include <float.h>

#include <numeric>
#include <ctime>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <float.h>

int randseed = time(NULL);

// From "Numerical Recipes in C".
float RandomFloat( int &seed )
{
	const int MASK = 123459876;
	seed ^= MASK;

	const int IA = 16807;
	const int IM = 2147483647;

	const int IQ = 127773;
	const int IR = 2836;

	long k = seed / IQ;
	seed = IA*(seed-k*IQ)-IR*k;
	if( seed < 0 )
		seed += IM;

	const float AM = .999999f / IM;
	float ans = AM * seed;

	seed ^= MASK;
	return ans;
}

RandomGen::RandomGen( unsigned long seed_ )
{
	seed = seed_;
	if(seed == 0)
		seed = time(NULL);
}

int RandomGen::operator() ( int n )
{
	float f = RandomFloat(seed) * (n);
	int ans = int(f);
	return ans;
}


void fapproach( float& val, float other_val, float to_move )
{
	ASSERT_M( to_move >= 0, ssprintf("to_move: %f", to_move) );
	if( val == other_val )
		return;
	float fDelta = other_val - val;
	float fSign = fDelta / fabsf( fDelta );
	float fToMove = fSign*to_move;
	if( fabsf(fToMove) > fabsf(fDelta) )
		fToMove = fDelta;	// snap
	val += fToMove;
}

/* Return a positive x mod y. */
float fmodfp(float x, float y)
{
	x = fmodf(x, y);	/* x is [-y,y] */
	x += y;				/* x is [0,y*2] */
	x = fmodf(x, y);	/* x is [0,y] */
	return x;
}

int power_of_two( int iInput )
{
	int iValue = 1;

	while ( iValue < iInput )
		iValue <<= 1;

	return iValue;
}

bool IsAnInt( const RString &s )
{
	if( !s.size() )
		return false;

	for( int i=0; s[i]; i++ )
		if( s[i] < '0' || s[i] > '9' )
			return false;

	return true;
}

bool IsHexVal( const RString &s )
{
	if( !s.size() )
		return false;

	for( int i=0; s[i]; i++ )
		if( !(s[i] >= '0' && s[i] <= '9') && 
			!(toupper(s[i]) >= 'A' && toupper(s[i]) <= 'F'))
			return false;

	return true;
}

RString BinaryToHex( const unsigned char *string, int iNumBytes )
{
       RString s;
       for( int i=0; i<iNumBytes; i++ )
       {
               unsigned val = string[i];
               s += ssprintf( "%02x", val );
       }
       return s;
}

bool HexToBinary( const RString &s, unsigned char *stringOut )
{
       if( !IsHexVal(s) )
               return false;

       for( int i=0; true; i++ )
       {
               if( (int)s.size() <= i*2 )
                       break;
               RString sByte = s.substr( i*2, 2 );

               uint8_t val = 0;
               if( sscanf( sByte, "%hhx", &val ) != 1 )
                       return false;
               stringOut[i] = val;
       }
       return true;
}

float HHMMSSToSeconds( const RString &sHHMMSS )
{
	vector<RString> arrayBits;
	split( sHHMMSS, ":", arrayBits, false );

	while( arrayBits.size() < 3 )
		arrayBits.insert(arrayBits.begin(), "0" );	// pad missing bits

	float fSeconds = 0;
	fSeconds += atoi( arrayBits[0] ) * 60 * 60;
	fSeconds += atoi( arrayBits[1] ) * 60;
	fSeconds += StringToFloat( arrayBits[2] );

	return fSeconds;
}

RString SecondsToHHMMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	RString sReturn = ssprintf( "%02d:%02d:%02d", iMinsDisplay/60, iMinsDisplay%60, iSecsDisplay );
	return sReturn;
}

RString SecondsToMMSSMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	RString sReturn = ssprintf( "%02d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

RString SecondsToMSSMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	RString sReturn = ssprintf( "%01d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

RString SecondsToMMSSMsMsMs( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 1000 );
	RString sReturn = ssprintf( "%02d:%02d.%03d", iMinsDisplay, iSecsDisplay, min(999,iLeftoverDisplay) );
	return sReturn;
}

RString PrettyPercent( float fNumerator, float fDenominator)
{
	return ssprintf("%0.2f%%",fNumerator/fDenominator*100);
}

RString Commify( int iNum ) 
{
	RString sNum = ssprintf("%d",iNum);
	RString sReturn;
	for( unsigned i=0; i<sNum.length(); i++ )
	{
		char cDigit = sNum[sNum.length()-1-i];
		if( i!=0 && i%3 == 0 )
			sReturn = ',' + sReturn;
		sReturn = cDigit + sReturn;
	}
	return sReturn;
}

static LocalizedString NUM_PREFIX	( "RageUtil", "NumPrefix" );
static LocalizedString NUM_ST		( "RageUtil", "NumSt" );
static LocalizedString NUM_ND		( "RageUtil", "NumNd" );
static LocalizedString NUM_RD		( "RageUtil", "NumRd" );
static LocalizedString NUM_TH		( "RageUtil", "NumTh" );
RString FormatNumberAndSuffix( int i )
{
	RString sSuffix;
	switch( i%10 )
	{
	case 1:		sSuffix = NUM_ST; break;
	case 2:		sSuffix = NUM_ND; break;
	case 3:		sSuffix = NUM_RD; break;
	default:	sSuffix = NUM_TH; break;
	}

	// "11th", "113th", etc.
	if( ((i%100) / 10) == 1 )
		sSuffix = NUM_TH;

	return NUM_PREFIX.GetValue() + ssprintf("%i", i) + sSuffix;
}

struct tm GetLocalTime()
{
	const time_t t = time(NULL);
	struct tm tm;
	localtime_r( &t, &tm );
	return tm;
}


RString ssprintf( const char *fmt, ...)
{
	va_list	va;
	va_start(va, fmt);
	return vssprintf(fmt, va);
}

#define FMT_BLOCK_SIZE		2048 // # of bytes to increment per try

RString vssprintf( const char *szFormat, va_list argList )
{
	RString sStr;

#if defined(WIN32) && !defined(__MINGW32__)
	char *pBuf = NULL;
	int iChars = 1;
	int iUsed = 0;
	int iTry = 0;

	do	
	{
		// Grow more than linearly (e.g. 512, 1536, 3072, etc)
		iChars += iTry * FMT_BLOCK_SIZE;
		pBuf = (char*) _alloca( sizeof(char)*iChars );
		iUsed = vsnprintf( pBuf, iChars-1, szFormat, argList );
		++iTry;
	} while( iUsed < 0 );

	// assign whatever we managed to format
	sStr.assign( pBuf, iUsed );
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
		int iNeeded = vsnprintf( &ignore, 0, szFormat, tmp );
		va_end(tmp);

		char *buf = sStr.GetBuffer( iNeeded+1 );
		vsnprintf( buf, iNeeded+1, szFormat, argList );
		sStr.ReleaseBuffer( iNeeded );
		return sStr;
	}

	int iChars = FMT_BLOCK_SIZE;
	int iTry = 1;
	while( 1 )
	{
		// Grow more than linearly (e.g. 512, 1536, 3072, etc)
		char *buf = sStr.GetBuffer(iChars);
		int iUsed = vsnprintf(buf, iChars-1, szFormat, argList);

		if( iUsed == -1 )
		{
			iChars += ((iTry+1) * FMT_BLOCK_SIZE);
			sStr.ReleaseBuffer();
			++iTry;
			continue;
		}

		/* OK */
		sStr.ReleaseBuffer(iUsed);
		break;
	}
#endif
	return sStr;
}

/* ISO-639-1 codes: http://www.loc.gov/standards/iso639-2/langcodes.html
 * native forms: http://people.w3.org/rishida/names/languages.html
 * We don't use 3-letter codes, so we don't bother supporting them. */
static const LanguageInfo g_langs[] =
{
	{"aa", "Afar", ""},
	{"ab", "Abkhazian", "аҧсуа бызшәа"},
	{"af", "Afrikaans", "Afrikaans"},
	{"am", "Amharic", ""},
	{"ar", "Arabic", "العربية"},
	{"as", "Assamese", ""},
	{"ay", "Aymara", ""},
	{"az", "Azerbaijani", "azərbaycan"},
	{"ba", "Bashkir", ""},
	{"be", "Byelorussian", "Беларуская"},
	{"bg", "Bulgarian", "Български"},
	{"bh", "Bihari", ""},
	{"bi", "Bislama", ""},
	{"bn", "Bengali", ""},
	{"bo", "Tibetan", ""},
	{"br", "Breton", ""},
	{"ca", "Catalan", "Català"},
	{"co", "Corsican", ""},
	{"cs", "Czech", "čeština"},
	{"cy", "Welsh", "Cymraeg"},
	{"da", "Danish", "Dansk"},
	{"de", "German", "Deutsch"},
	{"dz", "Bhutani", ""},
	{"el", "Greek", "Ελληνικά"},
	{"en", "English", "English"},
	{"eo", "Esperanto", ""},
	{"es", "Spanish", "Español"},
	{"et", "Estonian", "Eesti"},
	{"eu", "Basque", "euskera"},
	{"fa", "Persian", "فارسی"},
	{"fi", "Finnish", "Suomi"},
	{"fj", "Fiji", ""},
	{"fo", "Faeroese", ""},
	{"fr", "French", "Français"},
	{"fy", "Frisian", ""},
	{"ga", "Irish", "Gaeilge"},
	{"gd", "Gaelic", ""},
	{"gl", "Galician", "Galego"},
	{"gn", "Guarani", ""},
	{"gu", "Gujarati", ""},
	{"ha", "Hausa", "Hausa"},
	{"he", "Hebrew", "עברית"},
	{"hi", "Hindi", "हिंदी"},
	{"hr", "Croatian", "Hrvatski"},
	{"hu", "Hungarian", "Magyar"},
	{"hy", "Armenian", "Հայերեն"},
	{"ia", "Interlingua", ""},
	{"id", "Indonesian", "Bahasa indonesia"},
	{"ie", "Interlingue", ""},
	{"ik", "Inupiak", ""},
	{"in", "Indonesian", "Bahasa indonesia"}, // compatibility
	{"is", "Icelandic", "Íslenska"},
	{"it", "Italian", "Italiano"},
	{"iw", "Hebrew", "עברית"}, // compatibility
	{"ja", "Japanese", "日本語"},
	{"ji", "Yiddish", ""}, // compatibility
	{"jw", "Javanese", ""},
	{"ka", "Georgian", ""},
	{"kk", "Kazakh", "Қазақ"},
	{"kl", "Greenlandic", ""},
	{"km", "Cambodian", ""},
	{"kn", "Kannada", "ಕನ್ನಡ"},
	{"ko", "Korean", "한국어"},
	{"ks", "Kashmiri", ""},
	{"ku", "Kurdish", ""},
	{"ky", "Kirghiz", "Кыргыз"},
	{"la", "Latin", ""},
	{"ln", "Lingala", ""},
	{"lo", "Laothian", ""},
	{"lt", "Lithuanian", "Lietuviškai"},
	{"lv", "Latvian", "Latviešu"},
	{"mg", "Malagasy", ""},
	{"mi", "Maori", ""},
	{"mk", "Macedonian", "Македонски"},
	{"ml", "Malayalam", ""},
	{"mn", "Mongolian", ""},
	{"mo", "Moldavian", ""},
	{"mr", "Marathi", ""},
	{"ms", "Malay", "Bahasa melayu"},
	{"mt", "Maltese", "Malti"},
	{"my", "Burmese", ""},
	{"na", "Nauru", ""},
	{"ne", "Nepali", ""},
	{"nl", "Dutch", "Nederlands"},
	{"no", "Norwegian", "Norsk"},
	{"oc", "Occitan", ""},
	{"om", "Oromo", ""},
	{"or", "Oriya", ""},
	{"pa", "Punjabi", ""},
	{"pl", "Polish", "Polski"},
	{"ps", "Pashto", "پښتو"},
	{"pt", "Portuguese", "português"},
	{"qu", "Quechua", ""},
	{"rm", "Rhaeto-Romance", ""},
	{"rn", "Kirundi", "Kirundi"},
	{"ro", "Romanian", "Română"},
	{"ru", "Russian", "Pyccĸий"},
	{"rw", "Kinyarwanda", "Kinyarwanda"},
	{"sa", "Sanskrit", ""},
	{"sd", "Sindhi", ""},
	{"sg", "Sangro", ""},
	{"sh", "Serbo-Croatian", ""},
	{"si", "Singhalese", ""},
	{"sk", "Slovak", "Slovenčina"},
	{"sl", "Slovenian", "Slovenščina"},
	{"sm", "Samoan", ""},
	{"sn", "Shona", ""},
	{"so", "Somali", "Somali"},
	{"sq", "Albanian", "Shqip"},
	{"sr", "Serbian", "Srpski"},
	{"ss", "Siswati", ""},
	{"st", "Sesotho", ""},
	{"su", "Sudanese", ""},
	{"sv", "Swedish", "svenska"},
	{"sw", "Swahili", "Kiswahili"},
	{"ta", "Tamil", ""},
	{"te", "Tegulu", "తెలుగు"},
	{"tg", "Tajik", ""},
	{"th", "Thai", "ภาษาไทย"},
	{"ti", "Tigrinya", ""},
	{"tk", "Turkmen", ""},
	{"tl", "Tagalog", ""},
	{"tn", "Setswana", ""},
	{"to", "Tonga", ""},
	{"tr", "Turkish", "Tϋrkçe"},
	{"ts", "Tsonga", ""},
	{"tt", "Tatar", ""},
	{"tw", "Twi", ""},
	{"uk", "Ukrainian", "Українська"},
	{"ur", "Urdu", "اردو"},
	{"uz", "Uzbek", "o'zbek"},
	{"vi", "Vietnamese", "Tiếng Việt"},
	{"vo", "Volapuk", ""},
	{"wo", "Wolof", "Wolof"},
	{"xh", "Xhosa", "isiXhosa"},
	{"yi", "Yiddish", ""},
	{"yo", "Yoruba", "Yorùbá"},
	{"zh", "Chinese", "中文"},
	{"zu", "Zulu", "isiZulu"},
};

void GetLanguageInfos( vector<const LanguageInfo*> &vAddTo )
{
	for( unsigned i=0; i<ARRAYLEN(g_langs); ++i )
	{
		// Only use languages in the intersection of Windows languages
		// and languages that had native forms on the site listed above.
		// Obscure languages are dropped so that they don't clutter up the 
		// smtools UI.
		if( g_langs[i].szNativeName == RString() )
			continue;
		vAddTo.push_back( &g_langs[i] );
	}
}

const LanguageInfo *GetLanguageInfo( const RString &sIsoCode )
{
	for( unsigned i=0; i<ARRAYLEN(g_langs); ++i )
	{
		if( sIsoCode.EqualsNoCase(g_langs[i].szIsoCode) )
			return &g_langs[i];
	}

	return NULL;
}

RString join( const RString &sDeliminator, const vector<RString> &sSource)
{
	if( sSource.empty() )
		return RString();

	RString sTmp;

	for( unsigned iNum = 0; iNum < sSource.size()-1; iNum++ )
	{
		sTmp += sSource[iNum];
		sTmp += sDeliminator;
	}
	sTmp += sSource.back();
	return sTmp;
}

RString join( const RString &sDelimitor, vector<RString>::const_iterator begin, vector<RString>::const_iterator end )
{
	if( begin == end )
		return RString();

	RString sRet;
	while( begin != end )
	{
		sRet += *begin;
		++begin;
		if( begin != end )
			sRet += sDelimitor;
	}

	return sRet;
}

template <class S>
static int DelimitorLength( const S &Delimitor )
{
	return Delimitor.size();
}

static int DelimitorLength( char Delimitor )
{
	return 1;
}

static int DelimitorLength( wchar_t Delimitor )
{
	return 1;
}

template <class S, class C>
void do_split( const S &Source, const C Delimitor, vector<S> &AddIt, const bool bIgnoreEmpty )
{
	/* Short-circuit if the source is empty; we want to return an empty vector if
	 * the string is empty, even if bIgnoreEmpty is true. */
	if( Source.empty() )
		return;

	size_t startpos = 0;

	do {
		size_t pos;
		pos = Source.find( Delimitor, startpos );
		if( pos == Source.npos )
			pos = Source.size();

		if( pos-startpos > 0 || !bIgnoreEmpty )
		{
			/* Optimization: if we're copying the whole string, avoid substr; this
			 * allows this copy to be refcounted, which is much faster. */
			if( startpos == 0 && pos-startpos == Source.size() )
				AddIt.push_back(Source);
			else
			{
				const S AddRString = Source.substr(startpos, pos-startpos);
				AddIt.push_back(AddRString);
			}
		}

		startpos = pos+DelimitorLength(Delimitor);
	} while ( startpos <= Source.size() );
}


void split( const RString &sSource, const RString &sDelimitor, vector<RString> &asAddIt, const bool bIgnoreEmpty )
{
	if( sDelimitor.size() == 1 )
		do_split( sSource, sDelimitor[0], asAddIt, bIgnoreEmpty );
	else
		do_split( sSource, sDelimitor, asAddIt, bIgnoreEmpty );
}

void split( const wstring &sSource, const wstring &sDelimitor, vector<wstring> &asAddIt, const bool bIgnoreEmpty )
{
	if( sDelimitor.size() == 1 )
		do_split( sSource, sDelimitor[0], asAddIt, bIgnoreEmpty );
	else
		do_split( sSource, sDelimitor, asAddIt, bIgnoreEmpty );
}

/* Use:

RString str="a,b,c";
int start = 0, size = -1;
while( 1 )
{
	do_split( str, ",", start, size );
	if( start == str.size() )
		break;
	str[start] = 'Q';
}

 */

template <class S>
void do_split( const S &Source, const S &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	if( size != -1 )
	{
		/* Start points to the beginning of the last delimiter.  Move it up. */
		begin += size+Delimitor.size();
		begin = min( begin, len );
	}

	size = 0;

	if( bIgnoreEmpty )
	{
		/* Skip delims. */
		while( begin + Delimitor.size() < Source.size() &&
			!Source.compare( begin, Delimitor.size(), Delimitor ) )
			++begin;
	}

	/* Where's the string function to find within a substring?  C++ strings apparently
	 * are missing that ... */
	size_t pos;
	if( Delimitor.size() == 1 )
		pos = Source.find( Delimitor[0], begin );
	else
		pos = Source.find( Delimitor, begin );
	if( pos == Source.npos || (int) pos > len )
		pos = len;
	size = pos - begin;
}

void split( const RString &Source, const RString &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );

}

void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, int len, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, len, bIgnoreEmpty );
}

void split( const RString &Source, const RString &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
}

void split( const wstring &Source, const wstring &Delimitor, int &begin, int &size, const bool bIgnoreEmpty )
{
	do_split( Source, Delimitor, begin, size, Source.size(), bIgnoreEmpty );
}



/*
 * foo\fum\          -> "foo\fum\", "", ""
 * c:\foo\bar.txt    -> "c:\foo\", "bar", ".txt"
 * \\foo\fum         -> "\\foo\", "fum", ""
 */
void splitpath( const RString &sPath, RString &sDir, RString &sFilename, RString &sExt )
{
	sDir = sFilename = sExt = "";

	vector<RString> asMatches;

	/*
	 * One level of escapes for the regex, one for C. Ew. 
	 * This is really:
	 * ^(.*[\\/])?(.*)$ 
	 */
	static Regex sep("^(.*[\\\\/])?(.*)$");
	bool bCheck = sep.Compare( sPath, asMatches );
	ASSERT( bCheck );

	sDir = asMatches[0];
	const RString sBase = asMatches[1];

	/* ^(.*)(\.[^\.]+)$ */
	static Regex SplitExt("^(.*)(\\.[^\\.]+)$");
	if( SplitExt.Compare(sBase, asMatches) )
	{
		sFilename = asMatches[0];
		sExt = asMatches[1];
	}
	else
	{
		sFilename = sBase;
	}
}


/* "foo.bar", "baz" -> "foo.baz"
 * "foo", "baz" -> "foo.baz"
 * "foo.bar", "" -> "foo" */
RString SetExtension( const RString &sPath, const RString &sExt )
{
	RString sDir, sFileName, sOldExt;
	splitpath( sPath, sDir, sFileName, sOldExt );
	return sDir + sFileName + (sExt.size()? ".":"") + sExt;
}

RString GetExtension( const RString &sPath )
{
	size_t pos = sPath.rfind( '.' );
	if( pos == sPath.npos )
		return RString();

	size_t slash = sPath.find( '/', pos );
	if( slash != sPath.npos )
		return RString(); /* rare: path/dir.ext/fn */

	return sPath.substr( pos+1, sPath.size()-pos+1 );
}

RString GetFileNameWithoutExtension( const RString &sPath )
{
	RString sThrowAway, sFName;
	splitpath( sPath, sThrowAway, sFName, sThrowAway );
	return sFName;
}

void MakeValidFilename( RString &sName )
{
	wstring wsName = RStringToWstring( sName );
	wstring wsInvalid = L"/\\:*?\"<>|";
	for( unsigned i = 0; i < wsName.size(); ++i )
	{
		wchar_t w = wsName[i];
		if( w >= 32 &&
			w < 126 &&
			wsInvalid.find_first_of(w) == wsInvalid.npos )
			continue;

		if( w == L'"' )
		{
			wsName[i] = L'\'';
			continue;
		}

		/*
		 * We could replace with closest matches in ASCII: convert the character to UTF-8
		 * NFD (decomposed) (maybe NFKD?), and see if the first character is ASCII.
		 *
		 * This is useless for non-Western languages, since we'll replace the whole filename.
		 */
		wsName[i] = '_';
	}

	sName = WStringToRString( wsName );
}

int g_argc = 0;
char **g_argv = NULL;

void SetCommandlineArguments( int argc, char **argv )
{
	g_argc = argc;
	g_argv = argv;
}

/*
 * Search for the commandline argument given; eg. "test" searches for the
 * option "--test".  All commandline arguments are getopt_long style: --foo;
 * short arguments (-x) are not supported.  (These are not intended for
 * common, general use, so having short options isn't currently needed.)
 * If argument is non-NULL, accept an argument.
 */
bool GetCommandlineArgument( const RString &option, RString *argument, int iIndex )
{
	const RString optstr = "--" + option;
	
	for( int arg = 1; arg < g_argc; ++arg )
	{
		const RString CurArgument = g_argv[arg];

		const size_t i = CurArgument.find( "=" );
		RString CurOption = CurArgument.substr(0,i);
		if( CurOption.CompareNoCase(optstr) )
			continue; /* no match */

		/* Found it. */
		if( iIndex )
		{
			--iIndex;
			continue;
		}

		if( argument )
		{
			if( i != RString::npos )
				*argument = CurArgument.substr( i+1 );
			else
				*argument = "";
		}
		
		return true;
	}

	return false;
}

RString GetCwd()
{
#ifdef _XBOX
	return SYS_BASE_PATH;
#else
	char buf[PATH_MAX];
	bool ret = getcwd(buf, PATH_MAX) != NULL;
	ASSERT(ret);
	return buf;
#endif
}

/*
 * Calculate a standard CRC32.  iCRC should be initialized to 0.
 * References:
 *   http://www.theorem.com/java/CRC32.java,
 *   http://www.faqs.org/rfcs/rfc1952.html
 */
void CRC32( unsigned int &iCRC, const void *pVoidBuffer, size_t iSize )
{
	static unsigned tab[256];
	static bool initted = false;
	if( !initted )
	{
		initted = true;
		const unsigned POLY = 0xEDB88320;

		for( int i = 0; i < 256; ++i )
		{
			tab[i] = i;
			for( int j = 0; j < 8; ++j )
			{
				if( tab[i] & 1 )
					tab[i] = (tab[i] >> 1) ^ POLY;
				else
					tab[i] >>= 1;
			}
		}
	}

	iCRC ^= 0xFFFFFFFF;

	const char *pBuffer = (const char *) pVoidBuffer;
	for( unsigned i = 0; i < iSize; ++i )
		iCRC = (iCRC >> 8) ^ tab[(iCRC ^ pBuffer[i]) & 0xFF];

	iCRC ^= 0xFFFFFFFF;
}

unsigned int GetHashForString ( const RString &s )
{
	unsigned crc = 0;
	CRC32( crc, s.data(), s.size() );
	return crc;
}


/* Return true if "dir" is empty or does not exist. */
bool DirectoryIsEmpty( const RString &sDir )
{
	if( sDir.empty() )
		return true;
	if( !DoesFileExist(sDir) )
		return true;

	vector<RString> asFileNames;
	GetDirListing( sDir, asFileNames );
	return asFileNames.empty();
}

bool CompareRStringsAsc( const RString &sStr1, const RString &sStr2 )
{
	return sStr1.CompareNoCase( sStr2 ) < 0;
}

bool CompareRStringsDesc( const RString &sStr1, const RString &sStr2 )
{
	return sStr1.CompareNoCase( sStr2 ) > 0;
}

void SortRStringArray( vector<RString> &arrayRStrings, const bool bSortAscending )
{
	sort( arrayRStrings.begin(), arrayRStrings.end(),
			bSortAscending?CompareRStringsAsc:CompareRStringsDesc );
}

float calc_mean( const float *pStart, const float *pEnd )
{
	return accumulate( pStart, pEnd, 0.f ) / distance( pStart, pEnd );
}

float calc_stddev( const float *pStart, const float *pEnd, bool bSample )
{
	/* Calculate the mean. */
	float fMean = calc_mean( pStart, pEnd );

	/* Calculate stddev. */
	float fDev = 0.0f;
	for( const float *i=pStart; i != pEnd; ++i )
		fDev += (*i - fMean) * (*i - fMean);
	fDev /= distance( pStart, pEnd ) - (bSample ? 1 : 0);
	fDev = sqrtf( fDev );

	return fDev;
}

void TrimLeft( RString &sStr, const char *s )
{
	int n = 0;
	while( n < int(sStr.size()) && strchr(s, sStr[n]) )
		n++;

	sStr.erase( sStr.begin(), sStr.begin()+n );
}

void TrimRight( RString &sStr, const char *s )
{
	int n = sStr.size();
	while( n > 0 && strchr(s, sStr[n-1]) )
		n--;

	/* Delete from n to the end.  If n == sStr.size(), nothing is deleted;
	 * if n == 0, the whole string is erased. */
	sStr.erase( sStr.begin()+n, sStr.end() );
}

void StripCrnl( RString &s )
{
	while( s.size() && (s[s.size()-1] == '\r' || s[s.size()-1] == '\n') )
		s.erase( s.size()-1 );
}

bool BeginsWith( const RString &sTestThis, const RString &sBeginning )
{
	ASSERT( !sBeginning.empty() );
	return sTestThis.compare( 0, sBeginning.length(), sBeginning ) == 0;
}

bool EndsWith( const RString &sTestThis, const RString &sEnding )
{
	ASSERT( !sEnding.empty() );
	if( sTestThis.size() < sEnding.size() )
		return false;
	return sTestThis.compare( sTestThis.length()-sEnding.length(), sEnding.length(), sEnding ) == 0;
}

RString URLEncode( const RString &sStr )
{
	RString sOutput;
	for( unsigned k = 0; k < sStr.size(); k++ )
	{
		char t = sStr[k];
		if( t >= '!' && t <= 'z' )
			sOutput += t;
		else
			sOutput += "%" + ssprintf( "%02X", t );
	}
	return sOutput;
}

void StripCvs( vector<RString> &vs )
{
	for( unsigned i=0; i<vs.size(); i++ )
	{
		if( vs[i].Right(3).CompareNoCase("CVS") == 0 )
			vs.erase( vs.begin()+i );
	}
}

/* path is a .redir pathname.  Read it and return the real one. */
RString DerefRedir( const RString &_path )
{
	RString sPath = _path;

	for( int i=0; i<100; i++ )
	{
		if( GetExtension(sPath) != "redir" )
			return sPath;

		RString sNewFileName;
		GetFileContents( sPath, sNewFileName, true );

		/* Empty is invalid. */
		if( sNewFileName == "" )
			return RString();

		FixSlashesInPlace( sNewFileName );

		RString sPath2 = Dirname(sPath) + sNewFileName;

		CollapsePath( sPath2 );

		sPath2 += "*";

		vector<RString> matches;
		GetDirListing( sPath2, matches, false, true );

		if( matches.empty() )
			RageException::Throw( "The redirect \"%s\" references a file \"%s\" which doesn't exist.", sPath.c_str(), sPath2.c_str() );
		else if( matches.size() > 1 )
			RageException::Throw( "The redirect \"%s\" references a file \"%s\" with multiple matches.", sPath.c_str(), sPath2.c_str() );

		sPath = matches[0];
	}

	RageException::Throw( "Circular redirect \"%s\".", sPath.c_str() );
}

bool GetFileContents( const RString &sPath, RString &sOut, bool bOneLine )
{
	/* Don't warn if the file doesn't exist, but do warn if it exists and fails to open. */
	if( !IsAFile(sPath) )
		return false;
	
	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->Warn( "GetFileContents(%s): %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}
	
	RString sData;
	int iGot;
	if( bOneLine )
		iGot = file.GetLine( sData );
	else
		iGot = file.Read( sData, file.GetFileSize() );

	if( iGot == -1 )
	{
		LOG->Warn( "GetFileContents(%s): %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}

	if( bOneLine )
		StripCrnl( sData );
	
	sOut = sData;
	return true;
}

#include "pcre/pcre.h"
void Regex::Compile()
{
	const char *error;
	int offset;
	m_pReg = pcre_compile( m_sPattern.c_str(), PCRE_CASELESS, &error, &offset, NULL );

	if( m_pReg == NULL )
		RageException::Throw( "Invalid regex: \"%s\" (%s).", m_sPattern.c_str(), error );

	int iRet = pcre_fullinfo( (pcre *) m_pReg, NULL, PCRE_INFO_CAPTURECOUNT, &m_iBackrefs );
	ASSERT( iRet >= 0 );

	++m_iBackrefs;
	ASSERT( m_iBackrefs < 128 );
}

void Regex::Set( const RString &sStr )
{
	Release();
	m_sPattern = sStr;
	Compile();
}

void Regex::Release()
{
	pcre_free( m_pReg );
	m_pReg = NULL;
	m_sPattern = RString();
}

Regex::Regex( const RString &sStr )
{
	m_pReg = NULL;
	Set( sStr );
}

Regex::Regex( const Regex &rhs )
{
	m_pReg = NULL;
	Set( rhs.m_sPattern );
}

Regex &Regex::operator=( const Regex &rhs )
{
	if( this != &rhs )
		Set( rhs.m_sPattern );
	return *this;
}

Regex::~Regex()
{
	Release();
}

bool Regex::Compare( const RString &sStr )
{
	int iMat[128*3];
	int iRet = pcre_exec( (pcre *) m_pReg, NULL, sStr.data(), sStr.size(), 0, 0, iMat, 128*3 );

	if( iRet < -1 )
		RageException::Throw( "Unexpected return from pcre_exec('%s'): %i.", m_sPattern.c_str(), iRet );

	return iRet >= 0;
}

bool Regex::Compare( const RString &sStr, vector<RString> &asMatches )
{
	asMatches.clear();

	int iMat[128*3];
	int iRet = pcre_exec( (pcre *) m_pReg, NULL, sStr.data(), sStr.size(), 0, 0, iMat, 128*3 );

	if( iRet < -1 )
		RageException::Throw( "Unexpected return from pcre_exec('%s'): %i.", m_sPattern.c_str(), iRet );

	if( iRet == -1 )
		return false;

	for( unsigned i = 1; i < m_iBackrefs; ++i )
	{
		const int iStart = iMat[i*2], end = iMat[i*2+1];
		if( iStart == -1 )
			asMatches.push_back( RString() ); /* no match */
		else
			asMatches.push_back( sStr.substr(iStart, end - iStart) );
	}

	return true;
}

// Arguments and behavior are the same are similar to
// http://us3.php.net/manual/en/function.preg-replace.php
bool Regex::Replace( const RString &sReplacement, const RString &sSubject, RString &sOut )
{
	vector<RString> asMatches;
	if( !Compare(sSubject, asMatches) )
		return false;

	sOut = sReplacement;

	// TODO: optimize me by iterating only once over the string
	for( unsigned i=0; i<asMatches.size(); i++ )
	{
		RString sFrom = ssprintf( "\\${%d}", i );
		RString sTo = asMatches[i];
		sOut.Replace(sFrom, sTo);
	}

	return true;
}




/* Given a UTF-8 byte, return the length of the codepoint (if a start code)
 * or 0 if it's a continuation byte. */
int utf8_get_char_len( char p )
{
	if( !(p & 0x80) ) return 1; /* 0xxxxxxx - 1 */
	if( !(p & 0x40) ) return 1; /* 10xxxxxx - continuation */
	if( !(p & 0x20) ) return 2; /* 110xxxxx */
	if( !(p & 0x10) ) return 3; /* 1110xxxx */
	if( !(p & 0x08) ) return 4; /* 11110xxx */
	if( !(p & 0x04) ) return 5; /* 111110xx */
	if( !(p & 0x02) ) return 6; /* 1111110x */
	return 1; /* 1111111x */
}

static inline bool is_utf8_continuation_byte( char c )
{
	return (c & 0xC0) == 0x80;
}

/* Decode one codepoint at start; advance start and place the result in ch.  If
 * the encoded string is invalid, false is returned. */
bool utf8_to_wchar_ec( const RString &s, unsigned &start, wchar_t &ch )
{
	if( start >= s.size() )
		return false;

	if( is_utf8_continuation_byte( s[start] ) || /* misplaced continuation byte */
	    (s[start] & 0xFE) == 0xFE ) /* 0xFE, 0xFF */
	{
		start += 1;
		return false;
	}

	int len = utf8_get_char_len( s[start] );

	const int first_byte_mask[] = { -1, 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

	ch = wchar_t(s[start] & first_byte_mask[len]);

	for( int i = 1; i < len; ++i )
	{
		if( start+i >= s.size() )
		{
			/* We expected a continuation byte, but didn't get one.  Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}

		char byte = s[start+i];
		if( !is_utf8_continuation_byte(byte) )
		{
			/* We expected a continuation byte, but didn't get one.  Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}
		ch = (ch << 6) | byte & 0x3F;
	}

	bool bValid = true;
	{
		unsigned c1 = (unsigned) s[start] & 0xFF;
		unsigned c2 = (unsigned) s[start+1] & 0xFF;
		int c = (c1 << 8) + c2;
		if( (c & 0xFE00) == 0xC000 ||
		    (c & 0xFFE0) == 0xE080 ||
		    (c & 0xFFF0) == 0xF080 ||
		    (c & 0xFFF8) == 0xF880 ||
		    (c & 0xFFFC) == 0xFC80 )
	    {
		    bValid = false;
	    }
	}

	if( ch == 0xFFFE || ch == 0xFFFF )
		bValid = false;

	start += len;
	return bValid;
}

/* Like utf8_to_wchar_ec, but only does enough error checking to prevent crashing. */
bool utf8_to_wchar( const char *s, size_t iLength, unsigned &start, wchar_t &ch )
{
	if( start >= iLength )
		return false;

	int len = utf8_get_char_len( s[start] );

	if( start+len > iLength )
	{
		/* We don't have room for enough continuation bytes.  Return error. */
		start += len;
		ch = L'?';
		return false;
	}

	switch( len )
	{
	case 1:
		ch = (s[start+0] & 0x7F);
		break;
	case 2:
		ch = ( (s[start+0] & 0x1F) << 6 ) |
		       (s[start+1] & 0x3F);
		break;
	case 3:
		ch = ( (s[start+0] & 0x0F) << 12 ) |
		     ( (s[start+1] & 0x3F) << 6 ) |
		       (s[start+2] & 0x3F);
		break;
	case 4:
		ch = ( (s[start+0] & 0x07) << 18 ) |
		     ( (s[start+1] & 0x3F) << 12 ) |
		     ( (s[start+2] & 0x3F) << 6 ) |
		     (s[start+3] & 0x3F);
		break;
	case 5:
		ch = ( (s[start+0] & 0x03) << 24 ) |
		     ( (s[start+1] & 0x3F) << 18 ) |
		     ( (s[start+2] & 0x3F) << 12 ) |
		     ( (s[start+3] & 0x3F) << 6 ) |
		     (s[start+4] & 0x3F);
		break;

	case 6:
		ch = ( (s[start+0] & 0x01) << 30 ) |
		     ( (s[start+1] & 0x3F) << 24 ) |
		     ( (s[start+2] & 0x3F) << 18 ) |
		     ( (s[start+3] & 0x3F) << 12) |
		     ( (s[start+4] & 0x3F) << 6 ) |
		     (s[start+5] & 0x3F);
		break;

	}

	start += len;
	return true;
}


/* UTF-8 encode ch and append to out. */
void wchar_to_utf8( wchar_t ch, RString &out )
{
	if( ch < 0x80 ) { out.append( 1, (char) ch ); return; }

	int cbytes = 0;
	if( ch < 0x800 ) cbytes = 1;
	else if( ch < 0x10000 )    cbytes = 2;
	else if( ch < 0x200000 )   cbytes = 3;
	else if( ch < 0x4000000 )  cbytes = 4;
	else cbytes = 5;

	{
		int shift = cbytes*6;
		const int init_masks[] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		out.append( 1, (char) (init_masks[cbytes-1] | (ch>>shift)) );
	}

	for( int i = 0; i < cbytes; ++i )
	{
		int shift = (cbytes-i-1)*6;
		out.append( 1, (char) (0x80 | ((ch>>shift)&0x3F)) );
	}
}


wchar_t utf8_get_char( const RString &s )
{
	unsigned start = 0;
	wchar_t ret;
	if( !utf8_to_wchar_ec( s, start, ret ) )
		return INVALID_CHAR;
	return ret;
}



/* Replace invalid sequences in s. */
void utf8_sanitize( RString &s )
{
	RString ret;
	for( unsigned start = 0; start < s.size(); )
	{
		wchar_t ch;
		if( !utf8_to_wchar_ec( s, start, ch ) )
			ch = INVALID_CHAR;

		wchar_to_utf8( ch, ret );
	}

	s = ret;
}


bool utf8_is_valid( const RString &s )
{
	for( unsigned start = 0; start < s.size(); )
	{
		wchar_t ch;
		if( !utf8_to_wchar_ec( s, start, ch ) )
			return false;
	}
	return true;
}

/* Windows tends to drop garbage BOM characters at the start of UTF-8 text files.
 * Remove them. */
void utf8_remove_bom( RString &sLine )
{
	if( !sLine.compare(0, 3, "\xef\xbb\xbf") )
		sLine.erase(0, 3);
}

static int UnicodeDoUpper( char *p, size_t iLen, const unsigned char pMapping[256] )
{
	wchar_t wc = L'\0';
	unsigned iStart = 0;
	if( !utf8_to_wchar(p, iLen, iStart, wc) )
		return 1;
	
	wchar_t iUpper = wc;
	if( wc < 256 )
		iUpper = pMapping[wc];
	if( iUpper != wc )
	{
		RString sOut;
		wchar_to_utf8( iUpper, sOut );
		if( sOut.size() == iStart )
			memcpy( p, sOut.data(), sOut.size() );
		else
			WARN( ssprintf("UnicodeDoUpper: invalid character at \"%s\"", RString(p,iLen).c_str()) );
	}

	return iStart;
}

/* Fast in-place MakeUpper and MakeLower.  This only replaces characters with characters of the same UTF-8
 * length, so we never have to move the whole string.  This is optimized for strings that have no
 * non-ASCII characters. */
void MakeUpper( char *p, size_t iLen )
{
	char *pStart = p;
	char *pEnd = p + iLen;
	while( p < pEnd )
	{
		/* Fast path: */
		if( likely( !(*p & 0x80) ) )
		{
			if( unlikely(*p >= 'a' && *p <= 'z') )
				*p += 'A' - 'a';
			++p;
			continue;
		}

		int iRemaining = iLen - (p-pStart);
		p += UnicodeDoUpper( p, iRemaining, g_UpperCase );
	}
}

void MakeLower( char *p, size_t iLen )
{
	char *pStart = p;
	char *pEnd = p + iLen;
	while( p < pEnd )
	{
		/* Fast path: */
		if( likely( !(*p & 0x80) ) )
		{
			if( unlikely(*p >= 'A' && *p <= 'Z') )
				*p -= 'A' - 'a';
			++p;
			continue;
		}

		int iRemaining = iLen - (p-pStart);
		p += UnicodeDoUpper( p, iRemaining, g_LowerCase );
	}
}

void UnicodeUpperLower( wchar_t *p, size_t iLen, const unsigned char pMapping[256] )
{
	wchar_t *pEnd = p + iLen;
	while( p != pEnd )
	{
		if( *p < 256 )
			*p = pMapping[*p];
		++p;
	}
}

void MakeUpper( wchar_t *p, size_t iLen )
{
	UnicodeUpperLower( p, iLen, g_UpperCase );
}

void MakeLower( wchar_t *p, size_t iLen )
{
	UnicodeUpperLower( p, iLen, g_LowerCase );
}

float StringToFloat( const RString &sString )
{
	float ret = strtof( sString, NULL );
	
	if( !isfinite(ret) )
		ret = 0.0f;
	return ret;
}

bool StringToFloat( const RString &sString, float &fOut )
{
	char *endPtr;
	
	fOut = strtof( sString, &endPtr );
	return sString.size() && *endPtr == '\0' && isfinite( fOut );
}
	
	
const wchar_t INVALID_CHAR = 0xFFFD; /* U+FFFD REPLACEMENT CHARACTER */

wstring RStringToWstring( const RString &s )
{
	wstring ret;
	ret.reserve( s.size() );
	for( unsigned start = 0; start < s.size(); )
	{
		char c = s[start];
		if( !(c&0x80) )
		{
			/* ASCII fast path */
			ret += c;
			++start;
			continue;
		}
		
		wchar_t ch = L'\0';
		if( !utf8_to_wchar( s.data(), s.size(), start, ch ) )
			ch = INVALID_CHAR;
		ret += ch;
	}

	return ret;
}

RString WStringToRString( const wstring &sStr )
{
	RString sRet;

	for( unsigned i = 0; i < sStr.size(); ++i )
		wchar_to_utf8( sStr[i], sRet );

	return sRet;
}


RString WcharToUTF8( wchar_t c )
{
	RString ret;
	wchar_to_utf8( c, ret );
	return ret;
}

/* &a; -> a */
void ReplaceEntityText( RString &sText, const map<RString,RString> &m )
{
	RString sRet;

	size_t iOffset = 0;
	while( iOffset != sText.size() )
	{
		size_t iStart = sText.find( '&', iOffset );
		if( iStart == sText.npos )
		{
			/* Optimization: if we didn't replace anything at all, do nothing. */
			if( iOffset == 0 )
				return;

			// Append the rest of the string.
			sRet.append( sText, iOffset, sRet.npos );
			break;
		}

		// Append the text between iOffset and iStart.
		sRet.append( sText, iOffset, iStart-iOffset );
		iOffset += iStart-iOffset;

		// Optimization: stop early on "&", so "&&&&&&&&&&&" isn't n^2.
		size_t iEnd = sText.find_first_of( "&;", iStart+1 );
		if( iEnd == sText.npos || sText[iEnd] == '&' )
		{
			/* & with no matching ;, or two & in a row.  Append the & and
			 * continue. */
			sRet.append( sText, iStart, 1 );
			++iOffset;
			continue;
		}

		RString sElement = sText.substr( iStart+1, iEnd-iStart-1 );
		sElement.MakeLower();

		map<RString,RString>::const_iterator it = m.find( sElement );
		if( it == m.end() )
		{
			sRet.append( sText, iStart, iEnd-iStart+1 );
			iOffset = iEnd + 1;
			continue;
		}

		const RString &sTo = it->second;
		sRet.append( sTo );
		iOffset = iEnd + 1;
	}

	sText = sRet;
}

/* abcd -> &a; &b; &c; &d; */
void ReplaceEntityText( RString &sText, const map<char,RString> &m )
{
	RString sFind;

	FOREACHM_CONST( char, RString, m, c )
		sFind.append( 1, c->first );

	RString sRet;

	size_t iOffset = 0;
	while( iOffset != sText.size() )
	{
		size_t iStart = sText.find_first_of( sFind, iOffset );
		if( iStart == sText.npos )
		{
			/* Optimization: if we didn't replace anything at all, do nothing. */
			if( iOffset == 0 )
				return;

			// Append the rest of the string.
			sRet.append( sText, iOffset, sRet.npos );
			break;
		}

		// Append the text between iOffset and iStart.
		sRet.append( sText, iOffset, iStart-iOffset );
		iOffset += iStart-iOffset;

		char sElement = sText[iStart];

		map<char,RString>::const_iterator it = m.find( sElement );
		ASSERT( it != m.end() );

		const RString &sTo = it->second;
		sRet.append( 1, '&' );
		sRet.append( sTo );
		sRet.append( 1, ';' );
		++iOffset;
	}

	sText = sRet;
}

/* Replace &#nnnn; (decimal) and &xnnnn; (hex) with corresponding UTF-8 characters. */
void Replace_Unicode_Markers( RString &sText )
{
	unsigned iStart = 0;
	while( iStart < sText.size() )
	{
		/* Look for &#digits; */
		bool bHex = false;
		size_t iPos = sText.find( "&#", iStart );
		if( iPos == sText.npos )
		{
			bHex = true;
			iPos = sText.find( "&x", iStart );
		}

		if( iPos == sText.npos )
			break;
		iStart = iPos+1;

		unsigned p = iPos;
		p += 2;

		/* Found &# or &x.  Is it followed by digits and a semicolon? */
		if( p >= sText.size() )
			continue;

		int iNumDigits = 0;
		while( p < sText.size() &&
			(bHex && isxdigit(sText[p])) || (!bHex && isdigit(sText[p])) )
		{
		   p++;
		   iNumDigits++;
		}

		if( !iNumDigits )
			continue; /* must have at least one digit */
		if( p >= sText.size() || sText[p] != ';' )
			continue;
		p++;

		int iNum;
		if( bHex )
			sscanf( sText.c_str()+iPos, "&x%x;", &iNum );
		else
			sscanf( sText.c_str()+iPos, "&#%i;", &iNum );
		if( iNum > 0xFFFF )
			iNum = INVALID_CHAR;

		sText.replace( iPos, p-iPos, WcharToUTF8(wchar_t(iNum)) );
	}
}

/* Form a string to identify a wchar_t with ASCII. */
RString WcharDisplayText( wchar_t c )
{
	RString sChr;
	sChr = ssprintf( "U+%4.4x", c );
	if( c < 128 )
		sChr += ssprintf( " ('%c')", char(c) );
	return sChr;
}

/* Return the last named component of dir:
 * a/b/c -> c
 * a/b/c/ -> c
 */
RString Basename( const RString &sDir )
{
	size_t iEnd = sDir.find_last_not_of( "/\\" );
	if( iEnd == sDir.npos )
		return RString();

	size_t iStart = sDir.find_last_of( "/\\", iEnd );
	if( iStart == sDir.npos )
		iStart = 0;
	else
		++iStart;

	return sDir.substr( iStart, iEnd-iStart+1 );
}

/*
 * Return all but the last named component of dir:
 *
 * a/b/c -> a/b/
 * a/b/c/ -> a/b/
 * c/ -> ./
 * /foo -> /
 * / -> /
 */
RString Dirname( const RString &dir )
{
	/* Special case: "/" -> "/". */
	if( dir.size() == 1 && dir[0] == '/' )
		return "/";

	int pos = dir.size()-1;
	/* Skip trailing slashes. */
	while( pos >= 0 && dir[pos] == '/' )
		--pos;

	/* Skip the last component. */
	while( pos >= 0 && dir[pos] != '/' )
		--pos;

	if( pos < 0 )
		return "./";

	return dir.substr(0, pos+1);
}

RString Capitalize( const RString &s )	
{
	if( s.empty() )
		return RString();

	RString s2 = s;
	char *pBuf = s2.GetBuffer();
	UnicodeDoUpper( pBuf, s2.size(), g_UpperCase );
	s2.ReleaseBuffer();

	return s2;
}

unsigned char g_UpperCase[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF,
};

unsigned char g_LowerCase[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
	0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x5B,0x5C,0x5D,0x5E,0x5F,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
	0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
	0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
	0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
	0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF,
};

void FixSlashesInPlace( RString &sPath )
{
	for( unsigned i = 0; i < sPath.size(); ++i )
		if( sPath[i] == '\\' )
			sPath[i] = '/';
}

/*
 * Keep trailing slashes, since that can be used to illustrate that a path always
 * represents a directory.
 *
 * foo/bar -> foo/bar
 * foo/bar/ -> foo/bar/
 * foo///bar/// -> foo/bar/
 * foo/bar/./baz -> foo/bar/baz
 * foo/bar/../baz -> foo/baz
 * ../foo -> ../foo
 * ../../foo -> ../../foo
 * ./foo -> foo (if bRemoveLeadingDot), ./foo (if !bRemoveLeadingDot)
 * ./ -> ./
 * ./// -> ./
 */

void CollapsePath( RString &sPath, bool bRemoveLeadingDot )
{
	RString sOut;
	sOut.reserve( sPath.size() );

	size_t iPos = 0;
	size_t iNext;
	for( ; iPos < sPath.size(); iPos = iNext )
	{
		/* Find the next slash. */
		iNext = sPath.find( '/', iPos );
		if( iNext == RString::npos )
			iNext = sPath.size();
		else
			++iNext;

		/* Strip extra slashes, but don't remove slashes from the beginning of the string. */
		if( iNext - iPos == 1 && sPath[iPos] == '/' )
		{
			if( !sOut.empty() )
				continue;
		}

		/* If this is a dot, skip it. */
		if( iNext - iPos == 2 && sPath[iPos] == '.' && sPath[iPos+1] == '/' )
		{
			if( bRemoveLeadingDot || !sOut.empty() )
				continue;
		}

		/* If this is two dots, */
		if( iNext - iPos == 3 && sPath[iPos] == '.' && sPath[iPos+1] == '.' && sPath[iPos+2] == '/' )
		{
			/* If this is the first path element (nothing to delete), or all we have is a slash,
			 * leave it. */
			if( sOut.empty() || (sOut.size() == 1 && sOut[0] == '/') )
			{
				sOut.append( sPath, iPos, iNext-iPos );
				continue;
			}

			/* Search backwards for the previous path element. */
			size_t iPrev = sOut.rfind( '/', sOut.size()-2 );
			if( iPrev == RString::npos )
				iPrev = 0;
			else
				++iPrev;
			
			/* If the previous element is also .., leave it. */
			bool bLastIsTwoDots = (sOut.size() - iPrev == 3 && sOut[iPrev] == '.' && sOut[iPrev+1] == '.' );
			if( bLastIsTwoDots )
			{
				sOut.append( sPath, iPos, iNext-iPos );
				continue;
			}

			sOut.erase( iPrev );
			continue;
		}

		sOut.append( sPath, iPos, iNext-iPos );
	}
	
	sOut.swap( sPath );
}


template<> bool FromString<int>( const RString &sValue, int &out )
{
	if( sscanf( sValue.c_str(), "%d", &out ) == 1 )
		return true;

	out = 0;
	return false;
}

template<> bool FromString<unsigned>( const RString &sValue, unsigned  &out )
{
	if( sscanf( sValue.c_str(), "%u", &out ) == 1 )
		return true;

	out = 0;
	return false;
}

template<> bool FromString<float>( const RString &sValue, float &out )
{
	const char *endptr = sValue.data() + sValue.size();
	out = strtof( sValue, (char **) &endptr );
	if( endptr != sValue.data() && isfinite( out ) )
		return true;
	out = 0;
	return false;
}

template<> bool FromString<bool>( const RString &sValue, bool &out )
{
	if( sValue.size() == 0 )
		return false;

	out = (atoi(sValue) != 0);
	return true;
}

template<> RString ToString<int>( const int &value )
{
	return ssprintf( "%i", value );
}

template<> RString ToString<unsigned>( const unsigned &value )
{
	return ssprintf( "%u", value );
}

template<> RString ToString<float>( const float &value )
{
	return ssprintf( "%f", value );
}

template<> RString ToString<bool>( const bool &value )
{
	return ssprintf( "%i", value );
}


//
// Helper function for reading/writing scores
//
bool FileRead( RageFileBasic& f, RString& sOut )
{
	if( f.AtEOF() )
		return false;
	if( f.GetLine(sOut) == -1 )
		return false;
	return true;
}

bool FileRead( RageFileBasic& f, int& iOut )
{
	RString s;
	if( !FileRead(f, s) )
		return false;
	iOut = atoi(s);
	return true;
}

bool FileRead( RageFileBasic& f, unsigned& uOut )
{
	RString s;
	if( !FileRead(f, s) )
		return false;
	uOut = atoi(s);
	return true;
}

bool FileRead( RageFileBasic& f, float& fOut )
{
	RString s;
	if( !FileRead(f, s) )
		return false;
	fOut = StringToFloat( s );
	return true;
}

void FileWrite( RageFileBasic& f, const RString& sWrite )
{
	f.PutLine( sWrite );
}

void FileWrite( RageFileBasic& f, int iWrite )
{
	f.PutLine( ssprintf("%d", iWrite) );
}

void FileWrite( RageFileBasic& f, size_t uWrite )
{
	f.PutLine( ssprintf("%i", (int)uWrite) );
}

void FileWrite( RageFileBasic& f, float fWrite )
{
	f.PutLine( ssprintf("%f", fWrite) );
}

bool FileCopy( const RString &sSrcFile, const RString &sDstFile )
{
	if( !sSrcFile.CompareNoCase(sDstFile) )
	{
		LOG->Warn( "Tried to copy \"%s\" over itself", sSrcFile.c_str() );
		return false;
	}

	RageFile in;
	if( !in.Open(sSrcFile, RageFile::READ) )
		return false;

	RageFile out;
	if( !out.Open(sDstFile, RageFile::WRITE) )
		return false;

	RString sError;
	if( !FileCopy(in, out, sError) )
	{
		LOG->Warn( "FileCopy(%s,%s): %s",
				sSrcFile.c_str(), sDstFile.c_str(), sError.c_str() );
		return false;
	}

	return true;
}

bool FileCopy( RageFileBasic &in, RageFileBasic &out, RString &sError, bool *bReadError )
{
	while(1)
	{
		RString data;
		if( in.Read(data, 1024*32) == -1 )
		{
			sError = ssprintf( "read error: %s", in.GetError().c_str() );
			if( bReadError != NULL )
				*bReadError = true;
			return false;
		}
		if( data.empty() )
			break;
		int i = out.Write(data);
		if( i == -1 )
		{
			sError = ssprintf( "write error: %s", out.GetError().c_str() );
			if( bReadError != NULL )
				*bReadError = false;
			return false;
		}
	}

	if( out.Flush() == -1 )
	{
		sError = ssprintf( "write error: %s", out.GetError().c_str() );
		if( bReadError != NULL )
			*bReadError = false;
		return false;
	}

	return true;
}

LuaFunction( SecondsToMSSMsMs, SecondsToMSSMsMs( FArg(1) ) )
LuaFunction( FormatNumberAndSuffix, FormatNumberAndSuffix( IArg(1) ) )
LuaFunction( Basename, Basename( SArg(1) ) )
static RString MakeLower( RString s ) { s.MakeLower(); return s; }
LuaFunction( Lowercase, MakeLower( SArg(1) ) )
static RString MakeUpper( RString s ) { s.MakeUpper(); return s; }
LuaFunction( Uppercase, MakeUpper( SArg(1) ) )

/*
 * Copyright (c) 2001-2005 Chris Danford, Glenn Maynard
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
