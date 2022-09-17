#include "global.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageSoundReader_FileReader.h"
#include "LocalizedString.h"
#include "LuaBinding.h"
#include "LuaManager.h"
#include <float.h>

#include <json/json.h>

#include <numeric>
#include <ctime>
#include <sstream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

const RString CUSTOM_SONG_PATH= "/@mem/";

bool HexToBinary(const RString&, RString&);
void utf8_sanitize(RString &);
void UnicodeUpperLower(wchar_t *, size_t, const unsigned char *);

RandomGen g_RandomNumberGenerator;

MersenneTwister::MersenneTwister( int iSeed ) : m_iNext(0)
{
	Reset( iSeed );
}

void MersenneTwister::Reset( int iSeed )
{
	if( iSeed == 0 )
		iSeed = time(nullptr);

	m_Values[0] = iSeed;
	m_iNext = 0;
	for( int i = 1; i < 624; ++i )
		m_Values[i] = ((69069 * m_Values[i-1]) + 1) & 0xFFFFFFFF;

	GenerateValues();
}

void MersenneTwister::GenerateValues()
{
	static const unsigned mask[] = { 0, 0x9908B0DF };

	for( int i = 0; i < 227; ++i )
	{
		int iVal = (m_Values[i] & 0x80000000) | (m_Values[i+1] & 0x7FFFFFFF);
		int iNext = (i + 397);

		m_Values[i] = m_Values[iNext];
		m_Values[i] ^= (iVal >> 1);
		m_Values[i] ^= mask[iVal&1];
	}

	for( int i = 227; i < 623; ++i )
	{
		int iVal = (m_Values[i] & 0x80000000) | (m_Values[i+1] & 0x7FFFFFFF);
		int iNext = (i + 397) - 624;

		m_Values[i] = m_Values[iNext];
		m_Values[i] ^= (iVal >> 1);
		m_Values[i] ^= mask[iVal&1];
	}

	int iVal = (m_Values[623] & 0x80000000) + (m_Values[0] & 0x7FFFFFFF);
	int iNext = (623 + 397) - 624;
	m_Values[623] = m_Values[iNext] ^ (iVal>>1);
	m_Values[623] ^= mask[iVal&1];
}

int MersenneTwister::Temper( int iVal )
{
	iVal ^= (iVal >> 11);
	iVal ^= (iVal << 7) & 0x9D2C5680;
	iVal ^= (iVal << 15) & 0xEFC60000;
	iVal ^= (iVal >> 18);
	return iVal;
}

int MersenneTwister::operator()()
{
	if( m_iNext == 624 )
	{
		m_iNext = 0;
		GenerateValues();
	}

	return Temper( m_Values[m_iNext++] );
}

/* Extend MersenneTwister into Lua space. This is intended to replace
 * math.randomseed and math.random, so we conform to their behavior. */

namespace
{
	MersenneTwister g_LuaPRNG;

	/* To map from [0..2^31-1] to [0..1), we divide by 2^31. */
	const double DIVISOR = pow( double(2), double(31) );

	static int Seed( lua_State *L )
	{
		g_LuaPRNG.Reset( IArg(1) );
		return 0;
	}

	static int Random( lua_State *L )
	{
		switch( lua_gettop(L) )
		{
			/* [0..1) */
			case 0:
			{
				double r = double(g_LuaPRNG()) / DIVISOR;
				lua_pushnumber( L, r );
				return 1;
			}

			/* [1..u] */
			case 1:
			{
				int upper = IArg(1);
				luaL_argcheck( L, 1 <= upper, 1, "interval is empty" );
				lua_pushnumber( L, g_LuaPRNG(upper) + 1 );
				return 1;
			}
			/* [l..u] */
			case 2:
			{
				int lower = IArg(1);
				int upper = IArg(2);
				luaL_argcheck( L, lower < upper, 2, "interval is empty" );
				lua_pushnumber( L, (int(g_LuaPRNG()) % (upper-lower+1)) + lower );
				return 1;
			}

			/* wrong amount of arguments */
			default:
			{
				return luaL_error( L, "wrong number of arguments" );
			}
		}
	}

	const luaL_Reg MersenneTwisterTable[] =
	{
		LIST_METHOD( Seed ),
		LIST_METHOD( Random ),
		{ nullptr, nullptr }
	};
}

LUA_REGISTER_NAMESPACE( MersenneTwister );

void fapproach( float& val, float other_val, float to_move )
{
	ASSERT_M( to_move >= 0, ssprintf("to_move: %f < 0", to_move) );
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
	x += y;			/* x is [0,y*2] */
	x = fmodf(x, y);	/* x is [0,y] */
	return x;
}

int power_of_two( int input )
{
	int exp = 31, i = input;
	if (i >> 16)
		i >>= 16;
	else exp -= 16;
	if (i >> 8)
		i >>= 8;
	else exp -= 8;
	if (i >> 4)
		i >>= 4;
	else exp -= 4;
	if (i >> 2)
		i >>= 2;
	else exp -= 2;
	if (i >> 1 == 0)
		exp -= 1;
	int value = 1 << exp;
	return (input == value) ? value : (value << 1);
}

bool IsAnInt( const RString &s )
{
	if( !s.size() )
		return false;

	for( size_t i=0; i < s.size(); ++i )
		if( s[i] < '0' || s[i] > '9' )
			return false;

	return true;
}

bool IsHexVal( const RString &s )
{
	if( !s.size() )
		return false;

	for( size_t i=0; i < s.size(); ++i )
		if( !(s[i] >= '0' && s[i] <= '9') &&
			!(toupper(s[i]) >= 'A' && toupper(s[i]) <= 'F'))
			return false;

	return true;
}

RString BinaryToHex( const void *pData_, size_t iNumBytes )
{
	const unsigned char *pData = (const unsigned char *) pData_;
	RString s;
	for( size_t i=0; i<iNumBytes; i++ )
	{
		unsigned val = pData[i];
		s += ssprintf( "%02x", val );
	}
	return s;
}

RString BinaryToHex( const RString &sString )
{
	return BinaryToHex( sString.data(), sString.size() );
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

bool HexToBinary( const RString &s, RString &sOut )
{
	sOut.resize(s.size() / 2);
	return HexToBinary(s, (unsigned char *) sOut.data());
}

float HHMMSSToSeconds( const RString &sHHMMSS )
{
	vector<RString> arrayBits;
	split( sHHMMSS, ":", arrayBits, false );

	while( arrayBits.size() < 3 )
		arrayBits.insert(arrayBits.begin(), "0" );	// pad missing bits

	float fSeconds = 0;
	fSeconds += StringToInt( arrayBits[0] ) * 60 * 60;
	fSeconds += StringToInt( arrayBits[1] ) * 60;
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

RString SecondsToMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	RString sReturn = ssprintf( "%01d:%02d", iMinsDisplay, iSecsDisplay);
	return sReturn;
}

RString SecondsToMMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	RString sReturn = ssprintf( "%02d:%02d", iMinsDisplay, iSecsDisplay);
	return sReturn;
}

RString PrettyPercent( float fNumerator, float fDenominator)
{
	return ssprintf("%0.2f%%",fNumerator/fDenominator*100);
}

RString Commify( int iNum )
{
	RString sNum = ssprintf("%d",iNum);
	return Commify( sNum );
}

RString Commify(const RString& num, const RString& sep, const RString& dot)
{
	size_t num_start= 0;
	size_t num_end= num.size();
	size_t dot_pos= num.find(dot);
	size_t dash_pos= num.find('-');
	if(dot_pos != string::npos)
	{
		num_end= dot_pos;
	}
	if(dash_pos != string::npos)
	{
		num_start= dash_pos + 1;
	}
	size_t num_size= num_end - num_start;
	size_t commies= (num_size / 3) - (!(num_size % 3));
	if(commies < 1)
	{
		return num;
	}
	size_t commified_len= num.size() + (commies * sep.size());
	RString ret;
	ret.resize(commified_len);
	size_t dest= 0;
	size_t next_comma= (num_size % 3) + (3 * (!(num_size % 3))) + num_start;
	for(size_t c= 0; c < num.size(); ++c)
	{
		if(c == next_comma && c < num_end)
		{
			for(size_t s= 0; s < sep.size(); ++s)
			{
				ret[dest]= sep[s];
				++dest;
			}
			next_comma+= 3;
		}
		ret[dest]= num[c];
		++dest;
	}
	return ret;
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
	const time_t t = time(nullptr);
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

#if defined(WIN32)
	char *pBuf = nullptr;
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
		char ignore;
		bExactSizeSupported = ( snprintf( &ignore, 0, "Hello World" ) == 11 );
		bInitialized = true;
	}

	if( bExactSizeSupported )
	{
		va_list tmp;
		va_copy( tmp, argList );
		char ignore;
		int iNeeded = vsnprintf( &ignore, 0, szFormat, tmp );
		va_end(tmp);

		char *buf = new char[iNeeded + 1];
		std::fill(buf, buf + iNeeded + 1, '\0');
		vsnprintf( buf, iNeeded+1, szFormat, argList );
		RString ret(buf);
		delete [] buf;
		return ret;
	}

	int iChars = FMT_BLOCK_SIZE;
	int iTry = 1;
	for (;;)
	{
		// Grow more than linearly (e.g. 512, 1536, 3072, etc)
		char *buf = new char[iChars];
		std::fill(buf, buf + iChars, '\0');
		int used = vsnprintf( buf, iChars - 1, szFormat, argList );
		if ( used == -1 )
		{
			iChars += ( ++iTry * FMT_BLOCK_SIZE );
		}
		else
		{
			/* OK */
			sStr.assign(buf, used);
		}

		delete [] buf;
		if (used != -1)
		{
			break;
		}
	}
#endif
	return sStr;
}

/* Windows uses %I64i to format a 64-bit int, instead of %lli. Convert "a b %lli %-3llu c d"
 * to "a b %I64 %-3I64u c d". This assumes a well-formed format string; invalid format strings
 * should not crash, but the results are undefined. */
#if defined(WIN32)
RString ConvertI64FormatString( const RString &sStr )
{
	RString sRet;
	sRet.reserve( sStr.size() + 16 );

	size_t iOffset = 0;
	while( iOffset < sStr.size() )
	{
		size_t iPercent = sStr.find( '%', iOffset );
		if( iPercent != sStr.npos )
		{
			sRet.append( sStr, iOffset, iPercent - iOffset );
			iOffset = iPercent;
		}

		size_t iEnd = sStr.find_first_of( "diouxXeEfFgGaAcsCSpnm%", iOffset + 1 );
		if( iEnd != sStr.npos && iEnd - iPercent >= 3 && iPercent > 2 && sStr[iEnd-2] == 'l' && sStr[iEnd-1] == 'l' )
		{
			sRet.append( sStr, iPercent, iEnd - iPercent - 2 ); // %
			sRet.append( "I64" ); // %I64
			sRet.append( sStr, iEnd, 1 ); // %I64i
			iOffset = iEnd + 1;
		}
		else
		{
			if( iEnd == sStr.npos )
				iEnd = sStr.size() - 1;
			sRet.append( sStr, iOffset, iEnd - iOffset + 1 );
			iOffset = iEnd + 1;
		}
	}
	return sRet;
}
#else
RString ConvertI64FormatString( const RString &sStr ) { return sStr; }
#endif

/* ISO-639-1 codes: http://www.loc.gov/standards/iso639-2/php/code_list.php
 * We don't use 3-letter codes, so we don't bother supporting them. */
static const LanguageInfo g_langs[] =
{
	{"aa", "Afar"},
	{"ab", "Abkhazian"},
	{"af", "Afrikaans"},
	{"am", "Amharic"},
	{"ar", "Arabic"},
	{"as", "Assamese"},
	{"ay", "Aymara"},
	{"az", "Azerbaijani"},
	{"ba", "Bashkir"},
	{"be", "Byelorussian"},
	{"bg", "Bulgarian"},
	{"bh", "Bihari"},
	{"bi", "Bislama"},
	{"bn", "Bengali"},
	{"bo", "Tibetan"},
	{"br", "Breton"},
	{"ca", "Catalan"},
	{"co", "Corsican"},
	{"cs", "Czech"},
	{"cy", "Welsh"},
	{"da", "Danish"},
	{"de", "German"},
	{"dz", "Bhutani"},
	{"el", "Greek"},
	{"en", "English"},
	{"eo", "Esperanto"},
	{"es", "Spanish"},
	{"et", "Estonian"},
	{"eu", "Basque"},
	{"fa", "Persian"},
	{"fi", "Finnish"},
	{"fj", "Fiji"},
	{"fo", "Faeroese"},
	{"fr", "French"},
	{"fy", "Frisian"},
	{"ga", "Irish"},
	{"gd", "Gaelic"},
	{"gl", "Galician"},
	{"gn", "Guarani"},
	{"gu", "Gujarati"},
	{"ha", "Hausa"},
	{"he", "Hebrew"},
	{"hi", "Hindi"},
	{"hr", "Croatian"},
	{"hu", "Hungarian"},
	{"hy", "Armenian"},
	{"ia", "Interlingua"},
	{"id", "Indonesian"},
	{"ie", "Interlingue"},
	{"ik", "Inupiak"},
	{"in", "Indonesian"}, // compatibility
	{"is", "Icelandic"},
	{"it", "Italian"},
	{"iw", "Hebrew"}, // compatibility
	{"ja", "Japanese"},
	{"ji", "Yiddish"}, // compatibility
	{"jw", "Javanese"},
	{"ka", "Georgian"},
	{"kk", "Kazakh"},
	{"kl", "Greenlandic"},
	{"km", "Cambodian"},
	{"kn", "Kannada"},
	{"ko", "Korean"},
	{"ks", "Kashmiri"},
	{"ku", "Kurdish"},
	{"ky", "Kirghiz"},
	{"la", "Latin"},
	{"ln", "Lingala"},
	{"lo", "Laothian"},
	{"lt", "Lithuanian"},
	{"lv", "Latvian"},
	{"mg", "Malagasy"},
	{"mi", "Maori"},
	{"mk", "Macedonian"},
	{"ml", "Malayalam"},
	{"mn", "Mongolian"},
	{"mo", "Moldavian"},
	{"mr", "Marathi"},
	{"ms", "Malay"},
	{"mt", "Maltese"},
	{"my", "Burmese"},
	{"na", "Nauru"},
	{"ne", "Nepali"},
	{"nl", "Dutch"},
	{"no", "Norwegian"},
	{"oc", "Occitan"},
	{"om", "Oromo"},
	{"or", "Oriya"},
	{"pa", "Punjabi"},
	{"pl", "Polish"},
	{"ps", "Pashto"},
	{"pt", "Portuguese"},
	{"qu", "Quechua"},
	{"rm", "Rhaeto-Romance"},
	{"rn", "Kirundi"},
	{"ro", "Romanian"},
	{"ru", "Russian"},
	{"rw", "Kinyarwanda"},
	{"sa", "Sanskrit"},
	{"sd", "Sindhi"},
	{"sg", "Sangro"},
	{"sh", "Serbo-Croatian"},
	{"si", "Singhalese"},
	{"sk", "Slovak"},
	{"sl", "Slovenian"},
	{"sm", "Samoan"},
	{"sn", "Shona"},
	{"so", "Somali"},
	{"sq", "Albanian"},
	{"sr", "Serbian"},
	{"ss", "Siswati"},
	{"st", "Sesotho"},
	{"su", "Sudanese"},
	{"sv", "Swedish"},
	{"sw", "Swahili"},
	{"ta", "Tamil"},
	{"te", "Tegulu"},
	{"tg", "Tajik"},
	{"th", "Thai"},
	{"ti", "Tigrinya"},
	{"tk", "Turkmen"},
	{"tl", "Tagalog"},
	{"tn", "Setswana"},
	{"to", "Tonga"},
	{"tr", "Turkish"},
	{"ts", "Tsonga"},
	{"tt", "Tatar"},
	{"tw", "Twi"},
	{"uk", "Ukrainian"},
	{"ur", "Urdu"},
	{"uz", "Uzbek"},
	{"vi", "Vietnamese"},
	{"vo", "Volapuk"},
	{"wo", "Wolof"},
	{"xh", "Xhosa"},
	{"yi", "Yiddish"},
	{"yo", "Yoruba"},
	{"zh-Hans", "Chinese (Simplified)"},
	{"zh-Hant", "Chinese (Traditional)"},
	{"zu", "Zulu"},
};

void GetLanguageInfos( vector<const LanguageInfo*> &vAddTo )
{
	for( unsigned i=0; i<ARRAYLEN(g_langs); ++i )
		vAddTo.push_back( &g_langs[i] );
}

const LanguageInfo *GetLanguageInfo( const RString &sIsoCode )
{
	for( unsigned i=0; i<ARRAYLEN(g_langs); ++i )
	{
		if( sIsoCode.EqualsNoCase(g_langs[i].szIsoCode) )
			return &g_langs[i];
	}

	return nullptr;
}

RString join( const RString &sDeliminator, const vector<RString> &sSource)
{
	if( sSource.empty() )
		return RString();

	RString sTmp;
	size_t final_size= 0;
	size_t delim_size= sDeliminator.size();
	for(size_t n= 0; n < sSource.size()-1; ++n)
	{
		final_size+= sSource[n].size() + delim_size;
	}
	final_size+= sSource.back().size();
	sTmp.reserve(final_size);

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
	size_t final_size= 0;
	size_t delim_size= sDelimitor.size();
	for(vector<RString>::const_iterator curr= begin; curr != end; ++curr)
	{
		final_size+= curr->size();
		if(curr != end)
		{
			final_size+= delim_size;
		}
	}
	sRet.reserve(final_size);

	while( begin != end )
	{
		sRet += *begin;
		++begin;
		if( begin != end )
			sRet += sDelimitor;
	}

	return sRet;
}

RString SmEscape( const RString &sUnescaped )
{
	return SmEscape( sUnescaped.c_str(), sUnescaped.size() );
}

RString SmEscape( const char *cUnescaped, int len )
{
	RString answer = "";
	for( int i = 0; i < len; ++i )
	{
		// Other characters we could theoretically escape:
		// NotesWriterSM.cpp used to claim ',' should be escaped, but there was no explanation why
		// '#' is both a control character and a valid part of a parameter.  The only way for there to be
		//   any confusion is in a misformatted .sm file, though, so it is unnecessary to escape it.
		if( cUnescaped[i] == '/' && i + 1 < len && cUnescaped[i + 1] == '/' )
		{
			answer += "\\/\\/";
			++i; // increment here so we skip both //s
			continue;
		}
		if( cUnescaped[i] == '\\' || cUnescaped[i] == ':' || cUnescaped[i] == ';' )
		    answer += "\\";
		answer += cUnescaped[i];
	}
	return answer;
}

RString DwiEscape( const RString &sUnescaped )
{
	return DwiEscape( sUnescaped.c_str(), sUnescaped.size() );
}

RString DwiEscape( const char *cUnescaped, int len )
{
	RString answer = "";
	for( int i = 0; i < len; ++i )
	{
		switch( cUnescaped[i] )
		{
		// TODO: Which of these characters actually affect DWI?
		case '\\':
		case ':':
		case ';': answer += '|'; break;
		case '[': answer += '('; break;
		case ']': answer += ')'; break;
		default: answer += cUnescaped[i];
		}
	}
	return answer;
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
for(;;)
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
		// Start points to the beginning of the last delimiter. Move it up.
		begin += size+Delimitor.size();
		begin = min( begin, len );
	}

	size = 0;

	if( bIgnoreEmpty )
	{
		// Skip delims.
		while( begin + Delimitor.size() < Source.size() &&
			!Source.compare( begin, Delimitor.size(), Delimitor ) )
			++begin;
	}

	/* Where's the string function to find within a substring?
	 * C++ strings apparently are missing that ... */
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

RString custom_songify_path(RString const& path)
{
	vector<RString> parts;
	split(path, "/", parts, false);
	if(parts.size() < 2)
	{
		return CUSTOM_SONG_PATH + path;
	}
	return CUSTOM_SONG_PATH + parts[parts.size()-2] + "/" + parts[parts.size()-1];
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

		/* We could replace with closest matches in ASCII: convert the character
		 * to UTF-8 NFD (decomposed) (maybe NFKD?), and see if the first
		 * character is ASCII. This is useless for non-Western languages,
		 * since we'll replace the whole filename. */
		wsName[i] = '_';
	}

	sName = WStringToRString( wsName );
}

bool FindFirstFilenameContaining(const vector<RString>& filenames,
	RString& out, const vector<RString>& starts_with,
	const vector<RString>& contains, const vector<RString>& ends_with)
{
	for(size_t i= 0; i < filenames.size(); ++i)
	{
		RString lower= GetFileNameWithoutExtension(filenames[i]);
		lower.MakeLower();
		for(size_t s= 0; s < starts_with.size(); ++s)
		{
			if(!lower.compare(0, starts_with[s].size(), starts_with[s]))
			{
				out= filenames[i];
				return true;
			}
		}
		size_t lower_size= lower.size();
		for(size_t s= 0; s < ends_with.size(); ++s)
		{
			if(lower_size >= ends_with[s].size())
			{
				size_t end_pos= lower_size - ends_with[s].size();
				if(!lower.compare(end_pos, string::npos, ends_with[s]))
				{
					out= filenames[i];
					return true;
				}
			}
		}
		for(size_t s= 0; s < contains.size(); ++s)
		{
			if(lower.find(contains[s]) != string::npos)
			{
				out= filenames[i];
				return true;
			}
		}
	}
	return false;
}

int g_argc = 0;
char **g_argv = nullptr;

void SetCommandlineArguments( int argc, char **argv )
{
	g_argc = argc;
	g_argv = argv;
}

void GetCommandLineArguments( int &argc, char **&argv )
{
	argc = g_argc;
	argv = g_argv;
}

/* Search for the commandline argument given; eg. "test" searches for the
 * option "--test".  All commandline arguments are getopt_long style: --foo;
 * short arguments (-x) are not supported.  (These are not intended for
 * common, general use, so having short options isn't currently needed.)
 * If argument is non-nullptr, accept an argument. */
bool GetCommandlineArgument( const RString &option, RString *argument, int iIndex )
{
	const RString optstr = "--" + option;

	for( int arg = 1; arg < g_argc; ++arg )
	{
		const RString CurArgument = g_argv[arg];

		const size_t i = CurArgument.find( "=" );
		RString CurOption = CurArgument.substr(0,i);
		if( CurOption.CompareNoCase(optstr) )
			continue; // no match

		// Found it.
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
	char buf[PATH_MAX];
	bool ret = getcwd(buf, PATH_MAX) != nullptr;
	ASSERT(ret);
	return buf;
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

unsigned int GetHashForString( const RString &s )
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

bool CalcLeastSquares( const vector< pair<float, float> > &vCoordinates,
                       float &fSlope, float &fIntercept, float &fError )
{
	if( vCoordinates.empty() )
		return false;
	float fSumXX = 0.0f, fSumXY = 0.0f, fSumX = 0.0f, fSumY = 0.0f;
	for( unsigned i = 0; i < vCoordinates.size(); ++i )
	{
		fSumXX += vCoordinates[i].first * vCoordinates[i].first;
		fSumXY += vCoordinates[i].first * vCoordinates[i].second;
		fSumX += vCoordinates[i].first;
		fSumY += vCoordinates[i].second;
	}
	const float fDenominator = vCoordinates.size() * fSumXX - fSumX * fSumX;
	fSlope = (vCoordinates.size() * fSumXY - fSumX * fSumY) / fDenominator;
	fIntercept = (fSumXX * fSumY - fSumX * fSumXY) / fDenominator;

	fError = 0.0f;
	for( unsigned i = 0; i < vCoordinates.size(); ++i )
	{
		const float fOneError = fIntercept + fSlope * vCoordinates[i].first - vCoordinates[i].second;
		fError += fOneError * fOneError;
	}
	fError /= vCoordinates.size();
	fError = sqrtf( fError );
	return true;
}

void FilterHighErrorPoints( vector< pair<float, float> > &vCoordinates,
                            float fSlope, float fIntercept, float fCutoff )
{
	unsigned int iOut = 0;
	for( unsigned int iIn = 0; iIn < vCoordinates.size(); ++iIn )
	{
		const float fError = fIntercept + fSlope * vCoordinates[iIn].first - vCoordinates[iIn].second;
		if( fabsf(fError) < fCutoff )
		{
			vCoordinates[iOut] = vCoordinates[iIn];
			++iOut;
		}
	}
	vCoordinates.resize( iOut );
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

	/* Delete from n to the end. If n == sStr.size(), nothing is deleted;
	 * if n == 0, the whole string is erased. */
	sStr.erase( sStr.begin()+n, sStr.end() );
}

void Trim( RString &sStr, const char *s )
{
	RString::size_type b = 0, e = sStr.size();
	while( b < e && strchr(s, sStr[b]) )
		++b;
	while( b < e && strchr(s, sStr[e-1]) )
		--e;
	sStr.assign( sStr.substr(b, e-b) );
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

// remove various version control-related files
static bool CVSOrSVN( const RString& s )
{
	return s.Right(3).EqualsNoCase("CVS") ||
			s.Right(4) == ".svn" ||
			s.Right(3).EqualsNoCase(".hg");
}

void StripCvsAndSvn( vector<RString> &vs )
{
	RemoveIf( vs, CVSOrSVN );
}

static bool MacResourceFork( const RString& s )
{
	return s.Left(2).EqualsNoCase("._");
}

void StripMacResourceForks( vector<RString> &vs )
{
	RemoveIf( vs, MacResourceFork );
}

// path is a .redir pathname. Read it and return the real one.
RString DerefRedir( const RString &_path )
{
	RString sPath = _path;

	for( int i=0; i<100; i++ )
	{
		if( GetExtension(sPath) != "redir" )
			return sPath;

		RString sNewFileName;
		GetFileContents( sPath, sNewFileName, true );

		// Empty is invalid.
		if( sNewFileName == "" )
			return RString();

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
	// Don't warn if the file doesn't exist, but do warn if it exists and fails to open.
	if( !IsAFile(sPath) )
		return false;

	RageFile file;
	if( !file.Open(sPath) )
	{
		LOG->Warn( "GetFileContents(%s): %s", sPath.c_str(), file.GetError().c_str() );
		return false;
	}

	// todo: figure out how to make this UTF-8 safe. -aj
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

bool GetFileContents( const RString &sFile, vector<RString> &asOut )
{
	RageFile file;
	if( !file.Open(sFile) )
	{
		LOG->Warn( "GetFileContents(%s): %s", sFile.c_str(), file.GetError().c_str() );
		return false;
	}

	RString sLine;
	while( file.GetLine(sLine) )
		asOut.push_back( sLine );
	return true;
}

#ifndef USE_SYSTEM_PCRE
#include "../extern/pcre/pcre.h"
#else
#include <pcre.h>
#endif
void Regex::Compile()
{
	const char *error;
	int offset;
	m_pReg = pcre_compile( m_sPattern.c_str(), PCRE_CASELESS, &error, &offset, nullptr );

	if( m_pReg == nullptr )
		RageException::Throw( "Invalid regex: \"%s\" (%s).", m_sPattern.c_str(), error );

	int iRet = pcre_fullinfo( (pcre *) m_pReg, nullptr, PCRE_INFO_CAPTURECOUNT, &m_iBackrefs );
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
	m_pReg = nullptr;
	m_sPattern = RString();
}

Regex::Regex( const RString &sStr ): m_pReg(nullptr), m_iBackrefs(0), m_sPattern(RString())
{
	Set( sStr );
}

Regex::Regex( const Regex &rhs ): m_pReg(nullptr), m_iBackrefs(0), m_sPattern(RString())
{
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
	int iRet = pcre_exec( (pcre *) m_pReg, nullptr, sStr.data(), sStr.size(), 0, 0, iMat, 128*3 );

	if( iRet < -1 )
		RageException::Throw( "Unexpected return from pcre_exec('%s'): %i.", m_sPattern.c_str(), iRet );

	return iRet >= 0;
}

bool Regex::Compare( const RString &sStr, vector<RString> &asMatches )
{
	asMatches.clear();

	int iMat[128*3];
	int iRet = pcre_exec( (pcre *) m_pReg, nullptr, sStr.data(), sStr.size(), 0, 0, iMat, 128*3 );

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

/* Decode one codepoint at start; advance start and place the result in ch.
 * If the encoded string is invalid, false is returned. */
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
			/* We expected a continuation byte, but didn't get one. Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}

		char byte = s[start+i];
		if( !is_utf8_continuation_byte(byte) )
		{
			/* We expected a continuation byte, but didn't get one. Return error, and point
			 * start at the unexpected byte; it's probably a new sequence. */
			start += i;
			return false;
		}
		ch = (ch << 6) | (byte & 0x3F);
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
		// We don't have room for enough continuation bytes. Return error.
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


// UTF-8 encode ch and append to out.
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

// Replace invalid sequences in s.
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
	// Note: this has problems with certain accented characters. -aj
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

/* Fast in-place MakeUpper and MakeLower. This only replaces characters with characters of the same UTF-8
 * length, so we never have to move the whole string. This is optimized for strings that have no
 * non-ASCII characters. */
void MakeUpper( char *p, size_t iLen )
{
	char *pStart = p;
	char *pEnd = p + iLen;
	while( p < pEnd )
	{
		// Fast path:
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
		// Fast path:
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
	float fOut = std::strtof(sString, nullptr);
	if (!isfinite(fOut))
	{
		fOut = 0.0f;
	}
	return fOut;
}

bool StringToFloat( const RString &sString, float &fOut )
{
	char *endPtr = nullptr;

	fOut = std::strtof(sString, &endPtr);
	return sString.size() && *endPtr == '\0' && isfinite(fOut);
}

RString FloatToString( const float &num )
{
	stringstream ss;
	ss << num;
	return ss.str();
}

int StringToInt( const std::string& str, std::size_t* pos, int base, int exceptVal )
{
  try
  {
    return std::stoi(str, pos, base);
  }
  catch (const std::invalid_argument & e) {
    LOG->Warn( "stoi(%s): %s", str.c_str(), e.what() );
  }
  catch (const std::out_of_range & e) {
    LOG->Warn( "stoi(%s): %s", str.c_str(), e.what() );
  }
  return exceptVal;
}

long StringToLong( const std::string& str, std::size_t* pos, int base, long exceptVal )
{
  try
  {
    return std::stol(str, pos, base);
  }
  catch (const std::invalid_argument & e) {
    LOG->Warn( "stol(%s): %s", str.c_str(), e.what() );
  }
  catch (const std::out_of_range & e) {
    LOG->Warn( "stol(%s): %s", str.c_str(), e.what() );
  }
  return exceptVal;
}

long long StringToLLong( const std::string& str, std::size_t* pos, int base, long long exceptVal )
{
  try
  {
    return std::stoll(str, pos, base);
  }
  catch (const std::invalid_argument & e) {
    LOG->Warn( "stoll(%s): %s", str.c_str(), e.what() );
  }
  catch (const std::out_of_range & e) {
    LOG->Warn( "stoll(%s): %s", str.c_str(), e.what() );
  }
  return exceptVal;
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
			// ASCII fast path
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

// &a; -> a
void ReplaceEntityText( RString &sText, const map<RString,RString> &m )
{
	RString sRet;

	size_t iOffset = 0;
	while( iOffset != sText.size() )
	{
		size_t iStart = sText.find( '&', iOffset );
		if( iStart == sText.npos )
		{
			// Optimization: if we didn't replace anything at all, do nothing.
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
			// & with no matching ;, or two & in a row. Append the & and continue.
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

// abcd -> &a; &b; &c; &d;
void ReplaceEntityText( RString &sText, const map<char,RString> &m )
{
	RString sFind;

	for (std::pair<char, RString> const &c : m)
		sFind.append( 1, c.first );

	RString sRet;

	size_t iOffset = 0;
	while( iOffset != sText.size() )
	{
		size_t iStart = sText.find_first_of( sFind, iOffset );
		if( iStart == sText.npos )
		{
			// Optimization: if we didn't replace anything at all, do nothing.
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

// Replace &#nnnn; (decimal) and &xnnnn; (hex) with corresponding UTF-8 characters.
void Replace_Unicode_Markers( RString &sText )
{
	unsigned iStart = 0;
	while( iStart < sText.size() )
	{
		// Look for &#digits;
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

		// Found &# or &x. Is it followed by digits and a semicolon?
		if( p >= sText.size() )
			continue;

		int iNumDigits = 0;
		while( p < sText.size() && bHex? isxdigit(sText[p]):isdigit(sText[p]) )
		{
			p++;
			iNumDigits++;
		}

		if( !iNumDigits )
			continue; // must have at least one digit
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

// Form a string to identify a wchar_t with ASCII.
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

/* Return all but the last named component of dir:
 *
 * a/b/c -> a/b/
 * a/b/c/ -> a/b/
 * c/ -> ./
 * /foo -> /
 * / -> /
 */
RString Dirname( const RString &dir )
{
	// Special case: "/" -> "/".
	if( dir.size() == 1 && dir[0] == '/' )
		return "/";

	int pos = dir.size()-1;
	// Skip trailing slashes.
	while( pos >= 0 && dir[pos] == '/' )
		--pos;

	// Skip the last component.
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

	char *buf = const_cast<char *>(s.c_str());

	UnicodeDoUpper( buf, s.size(), g_UpperCase );

	return buf;
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

/* Keep trailing slashes, since that can be used to illustrate that a path always
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
		// Find the next slash.
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

		// If this is a dot, skip it.
		if( iNext - iPos == 2 && sPath[iPos] == '.' && sPath[iPos+1] == '/' )
		{
			if( bRemoveLeadingDot || !sOut.empty() )
				continue;
		}

		// If this is two dots,
		if( iNext - iPos == 3 && sPath[iPos] == '.' && sPath[iPos+1] == '.' && sPath[iPos+2] == '/' )
		{
			/* If this is the first path element (nothing to delete),
			 * or all we have is a slash, leave it. */
			if( sOut.empty() || (sOut.size() == 1 && sOut[0] == '/') )
			{
				sOut.append( sPath, iPos, iNext-iPos );
				continue;
			}

			// Search backwards for the previous path element.
			size_t iPrev = sOut.rfind( '/', sOut.size()-2 );
			if( iPrev == RString::npos )
				iPrev = 0;
			else
				++iPrev;

			// If the previous element is also .., leave it.
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

namespace StringConversion
{
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

		out = StringToInt(sValue) != 0;
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
	for(;;)
	{
		RString data;
		if( in.Read(data, 1024*32) == -1 )
		{
			sError = ssprintf( "read error: %s", in.GetError().c_str() );
			if( bReadError != nullptr )
			{
				*bReadError = true;
			}
			return false;
		}
		if( data.empty() )
		{
			break;
		}
		int i = out.Write(data);
		if( i == -1 )
		{
			sError = ssprintf( "write error: %s", out.GetError().c_str() );
			if( bReadError != nullptr )
			{
				*bReadError = false;
			}
			return false;
		}
	}

	if( out.Flush() == -1 )
	{
		sError = ssprintf( "write error: %s", out.GetError().c_str() );
		if( bReadError != nullptr )
		{
			*bReadError = false;
		}
		return false;
	}

	return true;
}

LuaFunction( SecondsToMSSMsMs, SecondsToMSSMsMs( FArg(1) ) )
LuaFunction( SecondsToHHMMSS, SecondsToHHMMSS( FArg(1) ) )
LuaFunction( SecondsToMMSSMsMs, SecondsToMMSSMsMs( FArg(1) ) )
LuaFunction( SecondsToMMSSMsMsMs, SecondsToMMSSMsMsMs( FArg(1) ) )
LuaFunction( SecondsToMSS, SecondsToMSS( FArg(1) ) )
LuaFunction( SecondsToMMSS, SecondsToMMSS( FArg(1) ) )
LuaFunction( FormatNumberAndSuffix, FormatNumberAndSuffix( IArg(1) ) )
LuaFunction( Basename, Basename( SArg(1) ) )
static RString MakeLower( RString s ) { s.MakeLower(); return s; }
LuaFunction( Lowercase, MakeLower( SArg(1) ) )
static RString MakeUpper( RString s ) { s.MakeUpper(); return s; }
LuaFunction( Uppercase, MakeUpper( SArg(1) ) )
LuaFunction( mbstrlen, (int)RStringToWstring(SArg(1)).length() )
LuaFunction( URLEncode, URLEncode( SArg(1) ) );
LuaFunction( PrettyPercent, PrettyPercent( FArg(1), FArg(2) ) );
//LuaFunction( IsHexVal, IsHexVal( SArg(1) ) );
LuaFunction( lerp, lerp(FArg(1), FArg(2), FArg(3)) );

int LuaFunc_BinaryToHex(lua_State* L);
int LuaFunc_BinaryToHex(lua_State* L)
{
	size_t l;
	const char *s = luaL_checklstring(L, 1, &l);

	RString hex = BinaryToHex(s, l);

	LuaHelpers::Push(L, hex);
	return 1;
}
LUAFUNC_REGISTER_COMMON(BinaryToHex);

int LuaFunc_commify(lua_State* L);
int LuaFunc_commify(lua_State* L)
{
	RString num= SArg(1);
	RString sep= ",";
	RString dot= ".";
	if(!lua_isnoneornil(L, 2))
	{
		sep= lua_tostring(L, 2);
	}
	if(!lua_isnoneornil(L, 3))
	{
		dot= lua_tostring(L, 3);
	}
	RString ret= Commify(num, sep, dot);
	LuaHelpers::Push(L, ret);
	return 1;
}
LUAFUNC_REGISTER_COMMON(commify);

int LuaFunc_JsonEncode(lua_State* L);
int LuaFunc_JsonEncode(lua_State* L)
{
	int argc = lua_gettop(L);
	bool minified = false;

	if (argc < 1 || argc > 2)
	{
		luaL_error(L, "JsonEncode must be called with one or two arguments");
	}

	if (argc == 2)
	{
		minified = lua_toboolean(L, 2);
	}

	std::function<Json::Value(int)> convert = [&L, &convert](int index) -> Json::Value
	{
		if (lua_isboolean(L, index))
		{
			return Json::Value(static_cast<bool>(lua_toboolean(L, index)));
		}
		else if (lua_isnil(L, index))
		{
			return Json::Value(Json::nullValue);
		}
		else if (lua_isnumber(L, index))
		{
			double val = lua_tonumber(L, index);

			if (val == static_cast<Json::UInt>(val))
			{
				return Json::Value(static_cast<Json::UInt>(val));
			}
			else if (val == static_cast<Json::Int>(val))
			{
				return Json::Value(static_cast<Json::Int>(val));
			}
			return Json::Value(val);
		}
		else if (lua_isstring(L, index))
		{
			size_t len;
			const char *s = lua_tolstring(L, index, &len);

			return Json::Value(std::string(s, len));
		}
		else if (lua_istable(L, index))
		{
			// if the index is relative to the top of the stack,
			// then calculate the absolute index, so we have a
			// stable reference
			if (index < 0)
			{
				index = lua_gettop(L) + index + 1;
			}

			size_t len = lua_objlen(L, index);

			if (len > 0)
			{
				// array
				Json::Value array(Json::arrayValue);
				array.resize(len);

				for (int i = 0; i < len; i++)
				{
					lua_rawgeti(L, index, i + 1);
					array[i] = convert(-1);
					lua_pop(L, 1);
				}

				return array;
			}
			else
			{
				// object
				Json::Value obj(Json::objectValue);

				lua_pushnil(L);
				while (lua_next(L, index) != 0)
				{
					if (!lua_isstring(L, -2))
					{
						luaL_error(L, "object keys must be strings");
					}

					size_t keylen;
					const char *key = lua_tolstring(L, -2, &keylen);
					obj[std::string(key, keylen)] = convert(-1);
					lua_pop(L, 1);
				}

				if (obj.size() < 1)
				{
					return Json::Value(Json::arrayValue);
				}
				return obj;
			}

		}

		int tp = lua_type(L, index);
		luaL_error(L, "%s is not JSON serializable", lua_typename(L, tp));
		return Json::Value(Json::nullValue);	/* not reached */
	};

	Json::Value root = convert(1);

	std::string data;
	if(!minified)
	{
		Json::StyledWriter writer;
		data = writer.write(root);
	}
	else
	{
		Json::FastWriter writer;
		data = writer.write(root);
	}

	lua_pushlstring(L, data.c_str(), data.length());
	return 1;
}
LUAFUNC_REGISTER_COMMON(JsonEncode);

int LuaFunc_JsonDecode(lua_State* L);
int LuaFunc_JsonDecode(lua_State* L)
{
	int argc = lua_gettop(L);

	if (argc < 1)
	{
		luaL_error(L, "JsonDecode requires an argument");
	}

	size_t datalen;
	const char *data = lua_tolstring(L, 1, &datalen);

	Json::Reader reader;
	Json::Value root;

	bool ok = reader.parse(std::string(data, datalen), root, true);
	if (!ok)
	{
		std::string error = reader.getFormattedErrorMessages();
		luaL_error(L, "failed to parse JSON: %s", error.c_str());
	}

	std::function<void(const Json::Value&)> convert = [&L, &convert](const Json::Value& val)
	{
		if (val.isNull())
		{
			lua_pushnil(L);
		}
		else if (val.isInt() || val.isUInt() || val.isDouble())
		{
			lua_pushnumber(L, val.asDouble());
		}
		else if (val.isString())
		{
			std::string s = val.asString();
			lua_pushlstring(L, s.c_str(), s.length());
		}
		else if (val.isBool())
		{
			lua_pushboolean(L, val.asBool());
		}
		else if (val.isArray())
		{
			lua_createtable(L, val.size(), 0);
			for (int i = 0; i < val.size(); i++)
			{
				convert(val[i]);
				lua_rawseti(L, -2, i + 1);
			}
		}
		else if (val.isObject())
		{
			lua_createtable(L, 0, val.size());
			for (const std::string& member : val.getMemberNames())
			{
				lua_pushlstring(L, member.c_str(), member.length());
				convert(val[member]);
				lua_rawset(L, -3);
			}
		}
		else
		{
			luaL_error(L, "failed to parse JSON: invalid type");
		}
	};

	convert(root);
	return 1;
}
LUAFUNC_REGISTER_COMMON(JsonDecode);

void luafunc_approach_internal(lua_State* L, int valind, int goalind, int speedind, const float mult, int process_index);
void luafunc_approach_internal(lua_State* L, int valind, int goalind, int speedind, const float mult, int process_index)
{
#define TONUMBER_NICE(dest, num_name, index) \
	if(!lua_isnumber(L, index)) \
	{ \
		luaL_error(L, "approach: " #num_name " for approach %d is not a number.", process_index); \
	} \
	dest= lua_tonumber(L, index);
	float val= 0;
	float goal= 0;
	float speed= 0;
	TONUMBER_NICE(val, current, valind);
	TONUMBER_NICE(goal, goal, goalind);
	TONUMBER_NICE(speed, speed, speedind);
#undef TONUMBER_NICE
	if(speed < 0)
	{
		luaL_error(L, "approach: speed %d is negative.", process_index);
	}
	fapproach(val, goal, speed*mult);
	lua_pushnumber(L, val);
}

int LuaFunc_approach(lua_State* L);
int LuaFunc_approach(lua_State* L)
{
	// Args:  current, goal, speed
	// Returns:  new_current
	luafunc_approach_internal(L, 1, 2, 3, 1.0f, 1);
	return 1;
}
LUAFUNC_REGISTER_COMMON(approach);

int LuaFunc_multiapproach(lua_State* L);
int LuaFunc_multiapproach(lua_State* L)
{
	// Args:  {currents}, {goals}, {speeds}, speed_multiplier
	// speed_multiplier is optional, and is intended to be the delta time for
	// the frame, so that this can be used every frame and have the current
	// approach the goal at a framerate independent speed.
	// Returns:  {currents}
	// Modifies the values in {currents} in place.
	if(lua_gettop(L) < 3)
	{
		luaL_error(L, "multiapproach:  A table of current values, a table of goal values, and a table of speeds must be passed.");
	}
	size_t currents_len= lua_objlen(L, 1);
	size_t goals_len= lua_objlen(L, 2);
	size_t speeds_len= lua_objlen(L, 3);
	float mult= 1.0f;
	if(lua_isnumber(L, 4))
	{
		mult= lua_tonumber(L, 4);
	}
	if(currents_len != goals_len || currents_len != speeds_len)
	{
		luaL_error(L, "multiapproach:  There must be the same number of current values, goal values, and speeds.");
	}
	if(!lua_istable(L, 1) || !lua_istable(L, 2) || !lua_istable(L, 3))
	{
		luaL_error(L, "multiapproach:  current, goal, and speed must all be tables.");
	}
	for(size_t i= 1; i <= currents_len; ++i)
	{
		lua_rawgeti(L, 1, i);
		lua_rawgeti(L, 2, i);
		lua_rawgeti(L, 3, i);
		luafunc_approach_internal(L, -3, -2, -1, mult, i);
		lua_rawseti(L, 1, i);
		lua_pop(L, 3);
	}
	lua_pushvalue(L, 1);
	return 1;
}
LUAFUNC_REGISTER_COMMON(multiapproach);

int LuaFunc_get_music_file_length(lua_State* L);
int LuaFunc_get_music_file_length(lua_State* L)
{
	// Args:  file_path
	// Returns:  The length of the music in seconds.
	RString path= SArg(1);
	RString error;
	RageSoundReader* sample= RageSoundReader_FileReader::OpenFile(path, error);
	if(sample == nullptr)
	{
		luaL_error(L, "The music file '%s' does not exist.", path.c_str());
	}
	lua_pushnumber(L, sample->GetLength() / 1000.0f);
	return 1;
}
LUAFUNC_REGISTER_COMMON(get_music_file_length);

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
