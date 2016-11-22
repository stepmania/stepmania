#include "global.h"
#include "RageUtil.h"
#include "RageMath.hpp"
#include "RageString.hpp"
#include "RageUnicode.hpp"

#include <array>

#include "RageMath.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageSoundReader_FileReader.h"
#include "LocalizedString.h"
#include "LuaBinding.h"
#include "LuaManager.h"
#include "RageFmtWrap.h"

#include <numeric>
#include <ctime>
#include <sstream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

using std::vector;
using std::string;
using std::wstring;
using std::istringstream;
using std::stringstream;
using std::isfinite;

const std::string CUSTOM_SONG_PATH= "/@mem/";

bool HexToBinary(const std::string&, std::string&);

RandomGen g_RandomNumberGenerator;

/* Extend MersenneTwister into Lua space. This is intended to replace
 * math.randomseed and math.random, so we conform to their behavior. */

namespace
{
	RandomGen g_LuaPRNG;

	/* To map from [0..2^32-1] to [0..1), we divide by 2^32. */
	const double DIVISOR = 4294967296.0;

	static int Seed( lua_State *L )
	{
		g_LuaPRNG.seed(IArg(1));
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
				lua_pushnumber(L, r);
				return 1;
			}

			/* [1..u] */
			case 1:
			{
				int upper = IArg(1);
				luaL_argcheck(L, 1 <= upper, 1, "interval is empty");
				lua_pushnumber(L, random_up_to(g_LuaPRNG, upper) + 1);
				return 1;
			}
			/* [l..u] */
			case 2:
			{
				int lower = IArg(1);
				int upper = IArg(2);
				luaL_argcheck(L, lower < upper, 2, "interval is empty");
				lua_pushnumber(L, random_up_to(g_LuaPRNG, upper - lower + 1) + lower);
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

void seed_lua_prng()
{
	g_LuaPRNG.seed(static_cast<unsigned int>(time(nullptr)));
}

void fapproach( float& val, float other_val, float to_move )
{
	ASSERT_M( to_move >= 0, fmt::sprintf("to_move: %f < 0", to_move) );
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

bool IsAnInt( const std::string &s )
{
	if( !s.size() )
		return false;

	for( size_t i=0; i < s.size(); ++i )
		if( s[i] < '0' || s[i] > '9' )
			return false;

	return true;
}

bool IsHexVal( const std::string &s )
{
	if( !s.size() )
		return false;

	for( size_t i=0; i < s.size(); ++i )
		if( !(s[i] >= '0' && s[i] <= '9') &&
			!(toupper(s[i]) >= 'A' && toupper(s[i]) <= 'F'))
			return false;

	return true;
}

std::string BinaryToHex( const void *pData_, int iNumBytes )
{
	const unsigned char *pData = (const unsigned char *) pData_;
	std::string s;
	for( int i=0; i<iNumBytes; i++ )
	{
		unsigned val = pData[i];
		s += fmt::sprintf( "%02x", val );
	}
	return s;
}

std::string BinaryToHex( const std::string &sString )
{
	return BinaryToHex( sString.data(), sString.size() );
}

bool HexToBinary( const std::string &s, unsigned char *stringOut )
{
	if( !IsHexVal(s) )
		return false;

	for( int i=0; true; i++ )
	{
		if( (int)s.size() <= i*2 )
		{
			break;
		}
		std::string sByte = s.substr( i*2, 2 );

		uint8_t val = 0;
		if( sscanf( sByte.c_str(), "%hhx", &val ) != 1 )
		{
			return false;
		}
		stringOut[i] = val;
	}
	return true;
}

bool HexToBinary( const std::string &s, std::string &sOut )
{
	sOut.resize(s.size() / 2);
	return HexToBinary(s, (unsigned char *) sOut.data());
}

float HHMMSSToSeconds( const std::string &sHHMMSS )
{
	auto arrayBits = Rage::split(sHHMMSS, ":", Rage::EmptyEntries::include);

	while( arrayBits.size() < 3 )
	{
		arrayBits.insert(arrayBits.begin(), "0" );	// pad missing bits
	}
	float fSeconds = 0;
	fSeconds += StringToInt( arrayBits[0] ) * 60 * 60;
	fSeconds += StringToInt( arrayBits[1] ) * 60;
	fSeconds += StringToFloat( arrayBits[2] );

	return fSeconds;
}

std::string SecondsToHHMMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	std::string sReturn = fmt::sprintf( "%02d:%02d:%02d", iMinsDisplay/60, iMinsDisplay%60, iSecsDisplay );
	return sReturn;
}

std::string SecondsToMMSSMsMs( float fSecs )
{
	using std::min;
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	std::string sReturn = fmt::sprintf( "%02d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

std::string SecondsToMSSMsMs( float fSecs )
{
	using std::min;
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 100 );
	std::string sReturn = fmt::sprintf( "%01d:%02d.%02d", iMinsDisplay, iSecsDisplay, min(99,iLeftoverDisplay) );
	return sReturn;
}

std::string SecondsToMMSSMsMsMs( float fSecs )
{
	using std::min;
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	const int iLeftoverDisplay = (int) ( (fSecs - iMinsDisplay*60 - iSecsDisplay) * 1000 );
	std::string sReturn = fmt::sprintf( "%02d:%02d.%03d", iMinsDisplay, iSecsDisplay, min(999,iLeftoverDisplay) );
	return sReturn;
}

std::string SecondsToMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	std::string sReturn = fmt::sprintf( "%01d:%02d", iMinsDisplay, iSecsDisplay);
	return sReturn;
}

std::string SecondsToMMSS( float fSecs )
{
	const int iMinsDisplay = (int)fSecs/60;
	const int iSecsDisplay = (int)fSecs - iMinsDisplay*60;
	std::string sReturn = fmt::sprintf( "%02d:%02d", iMinsDisplay, iSecsDisplay);
	return sReturn;
}

std::string PrettyPercent( float fNumerator, float fDenominator)
{
	return fmt::sprintf("%0.2f%%",fNumerator/fDenominator*100);
}

std::string Commify( int iNum )
{
	std::string sNum = fmt::sprintf("%d",iNum);
	return Commify( sNum );
}

std::string Commify(const std::string& num, const std::string& sep, const std::string& dot)
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
	size_t commies= (num_size - 1) / 3;
	if(commies < 1)
	{
		return num;
	}
	size_t commified_len= num.size() + (commies * sep.size());
	std::string ret;
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
std::string FormatNumberAndSuffix( int i )
{
	std::string sSuffix;
	switch( i%10 )
	{
	case 1:		sSuffix = NUM_ST.GetValue(); break;
	case 2:		sSuffix = NUM_ND.GetValue(); break;
	case 3:		sSuffix = NUM_RD.GetValue(); break;
	default:	sSuffix = NUM_TH.GetValue(); break;
	}

	// "11th", "113th", etc.
	if( ((i%100) / 10) == 1 )
	{
		sSuffix = NUM_TH.GetValue();
	}
	return NUM_PREFIX.GetValue() + fmt::sprintf("%i", i) + sSuffix;
}

std::string unique_name(std::string const& type)
{
	// The returned name is not universally unique, it's only unique to a run
	// of the program.
	// Use only letters that can be the first character of an identifier,
	// for simplicity.  Exactly 32 letters so that each 5 bits of the counter
	// encodes one letter.
	static char const* name_chars= "abcdefghijklmnopqrstuvwxyzABCDEF";
	static int name_count= 0;
	int curr_name= name_count;
	std::string ret= type + "_"; // Minimize the chance of a name collision.
	do
	{
		int letter= curr_name & 31;
		ret= ret + name_chars[letter];
		curr_name= curr_name >> 5;
	} while(curr_name > 0);
	++name_count;
	return ret;
}

struct tm GetLocalTime()
{
	const time_t t = time(nullptr);
	struct tm tm;
	localtime_r( &t, &tm );
	return tm;
}

/* ISO-639-1 codes: http://www.loc.gov/standards/iso639-2/php/code_list.php
 * native forms: http://people.w3.org/rishida/names/languages.html
 * We don't use 3-letter codes, so we don't bother supporting them. */
static std::array<LanguageInfo, 139> const g_langs =
{
	{
		LanguageInfo{"aa", "Afar"},
		LanguageInfo{"ab", "Abkhazian"},
		LanguageInfo{"af", "Afrikaans"},
		LanguageInfo{"am", "Amharic"},
		LanguageInfo{"ar", "Arabic"},
		LanguageInfo{"as", "Assamese"},
		LanguageInfo{"ay", "Aymara"},
		LanguageInfo{"az", "Azerbaijani"},
		LanguageInfo{"ba", "Bashkir"},
		LanguageInfo{"be", "Byelorussian"},
		LanguageInfo{"bg", "Bulgarian"},
		LanguageInfo{"bh", "Bihari"},
		LanguageInfo{"bi", "Bislama"},
		LanguageInfo{"bn", "Bengali"},
		LanguageInfo{"bo", "Tibetan"},
		LanguageInfo{"br", "Breton"},
		LanguageInfo{"ca", "Catalan"},
		LanguageInfo{"co", "Corsican"},
		LanguageInfo{"cs", "Czech"},
		LanguageInfo{"cy", "Welsh"},
		LanguageInfo{"da", "Danish"},
		LanguageInfo{"de", "German"},
		LanguageInfo{"dz", "Bhutani"},
		LanguageInfo{"el", "Greek"},
		LanguageInfo{"en", "English"},
		LanguageInfo{"eo", "Esperanto"},
		LanguageInfo{"es", "Spanish"},
		LanguageInfo{"et", "Estonian"},
		LanguageInfo{"eu", "Basque"},
		LanguageInfo{"fa", "Persian"},
		LanguageInfo{"fi", "Finnish"},
		LanguageInfo{"fj", "Fiji"},
		LanguageInfo{"fo", "Faeroese"},
		LanguageInfo{"fr", "French"},
		LanguageInfo{"fy", "Frisian"},
		LanguageInfo{"ga", "Irish"},
		LanguageInfo{"gd", "Gaelic"},
		LanguageInfo{"gl", "Galician"},
		LanguageInfo{"gn", "Guarani"},
		LanguageInfo{"gu", "Gujarati"},
		LanguageInfo{"ha", "Hausa"},
		LanguageInfo{"he", "Hebrew"},
		LanguageInfo{"hi", "Hindi"},
		LanguageInfo{"hr", "Croatian"},
		LanguageInfo{"hu", "Hungarian"},
		LanguageInfo{"hy", "Armenian"},
		LanguageInfo{"ia", "Interlingua"},
		LanguageInfo{"id", "Indonesian"},
		LanguageInfo{"ie", "Interlingue"},
		LanguageInfo{"ik", "Inupiak"},
		LanguageInfo{"in", "Indonesian"}, // compatibility
		LanguageInfo{"is", "Icelandic"},
		LanguageInfo{"it", "Italian"},
		LanguageInfo{"iw", "Hebrew"}, // compatibility
		LanguageInfo{"ja", "Japanese"},
		LanguageInfo{"ji", "Yiddish"}, // compatibility
		LanguageInfo{"jw", "Javanese"},
		LanguageInfo{"ka", "Georgian"},
		LanguageInfo{"kk", "Kazakh"},
		LanguageInfo{"kl", "Greenlandic"},
		LanguageInfo{"km", "Cambodian"},
		LanguageInfo{"kn", "Kannada"},
		LanguageInfo{"ko", "Korean"},
		LanguageInfo{"ks", "Kashmiri"},
		LanguageInfo{"ku", "Kurdish"},
		LanguageInfo{"ky", "Kirghiz"},
		LanguageInfo{"la", "Latin"},
		LanguageInfo{"ln", "Lingala"},
		LanguageInfo{"lo", "Laothian"},
		LanguageInfo{"lt", "Lithuanian"},
		LanguageInfo{"lv", "Latvian"},
		LanguageInfo{"mg", "Malagasy"},
		LanguageInfo{"mi", "Maori"},
		LanguageInfo{"mk", "Macedonian"},
		LanguageInfo{"ml", "Malayalam"},
		LanguageInfo{"mn", "Mongolian"},
		LanguageInfo{"mo", "Moldavian"},
		LanguageInfo{"mr", "Marathi"},
		LanguageInfo{"ms", "Malay"},
		LanguageInfo{"mt", "Maltese"},
		LanguageInfo{"my", "Burmese"},
		LanguageInfo{"na", "Nauru"},
		LanguageInfo{"ne", "Nepali"},
		LanguageInfo{"nl", "Dutch"},
		LanguageInfo{"no", "Norwegian"},
		LanguageInfo{"oc", "Occitan"},
		LanguageInfo{"om", "Oromo"},
		LanguageInfo{"or", "Oriya"},
		LanguageInfo{"pa", "Punjabi"},
		LanguageInfo{"pl", "Polish"},
		LanguageInfo{"ps", "Pashto"},
		LanguageInfo{"pt", "Portuguese"},
		LanguageInfo{"qu", "Quechua"},
		LanguageInfo{"rm", "Rhaeto-Romance"},
		LanguageInfo{"rn", "Kirundi"},
		LanguageInfo{"ro", "Romanian"},
		LanguageInfo{"ru", "Russian"},
		LanguageInfo{"rw", "Kinyarwanda"},
		LanguageInfo{"sa", "Sanskrit"},
		LanguageInfo{"sd", "Sindhi"},
		LanguageInfo{"sg", "Sangro"},
		LanguageInfo{"sh", "Serbo-Croatian"},
		LanguageInfo{"si", "Singhalese"},
		LanguageInfo{"sk", "Slovak"},
		LanguageInfo{"sl", "Slovenian"},
		LanguageInfo{"sm", "Samoan"},
		LanguageInfo{"sn", "Shona"},
		LanguageInfo{"so", "Somali"},
		LanguageInfo{"sq", "Albanian"},
		LanguageInfo{"sr", "Serbian"},
		LanguageInfo{"ss", "Siswati"},
		LanguageInfo{"st", "Sesotho"},
		LanguageInfo{"su", "Sudanese"},
		LanguageInfo{"sv", "Swedish"},
		LanguageInfo{"sw", "Swahili"},
		LanguageInfo{"ta", "Tamil"},
		LanguageInfo{"te", "Tegulu"},
		LanguageInfo{"tg", "Tajik"},
		LanguageInfo{"th", "Thai"},
		LanguageInfo{"ti", "Tigrinya"},
		LanguageInfo{"tk", "Turkmen"},
		LanguageInfo{"tl", "Tagalog"},
		LanguageInfo{"tn", "Setswana"},
		LanguageInfo{"to", "Tonga"},
		LanguageInfo{"tr", "Turkish"},
		LanguageInfo{"ts", "Tsonga"},
		LanguageInfo{"tt", "Tatar"},
		LanguageInfo{"tw", "Twi"},
		LanguageInfo{"uk", "Ukrainian"},
		LanguageInfo{"ur", "Urdu"},
		LanguageInfo{"uz", "Uzbek"},
		LanguageInfo{"vi", "Vietnamese"},
		LanguageInfo{"vo", "Volapuk"},
		LanguageInfo{"wo", "Wolof"},
		LanguageInfo{"xh", "Xhosa"},
		LanguageInfo{"yi", "Yiddish"},
		LanguageInfo{"yo", "Yoruba"},
		LanguageInfo{"zh", "Chinese"},
		LanguageInfo{"zu", "Zulu"}
	}
};

void GetLanguageInfos( vector<const LanguageInfo*> &vAddTo )
{
	for (auto const &lang: g_langs)
	{
		vAddTo.push_back( &lang );
	}
}

const LanguageInfo *GetLanguageInfo( const std::string &sIsoCode )
{
	Rage::ci_ascii_string iso{ sIsoCode.c_str() };
	for (auto const &lang: g_langs)
	{
		if (iso == lang.isoCode)
		{
			return &lang;
		}
	}

	return nullptr;
}

std::string SmEscape( const std::string &sUnescaped )
{
	return SmEscape( sUnescaped.c_str(), sUnescaped.size() );
}

std::string SmEscape( const char *cUnescaped, int len )
{
	std::string answer = "";
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

std::string DwiEscape( const std::string &sUnescaped )
{
	return DwiEscape( sUnescaped.c_str(), sUnescaped.size() );
}

std::string DwiEscape( const char *cUnescaped, int len )
{
	std::string answer = "";
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

/*
 * foo\fum\          -> "foo\fum\", "", ""
 * c:\foo\bar.txt    -> "c:\foo\", "bar", ".txt"
 * \\foo\fum         -> "\\foo\", "fum", ""
 */
void splitpath( const std::string &sPath, std::string &sDir, std::string &sFilename, std::string &sExt )
{
	sDir = sFilename = sExt = "";

	vector<std::string> asMatches;

	/*
	 * One level of escapes for the regex, one for C. Ew.
	 * This is really:
	 * ^(.*[\\/])?(.*)$
	 */
	static Regex sep("^(.*[\\\\/])?(.*)$");
	bool bCheck = sep.Compare( sPath, asMatches );
	ASSERT( bCheck );

	sDir = asMatches[0];
	const std::string sBase = asMatches[1];

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

std::string custom_songify_path(std::string const& path)
{
	vector<std::string> parts= Rage::split(path, "/", Rage::EmptyEntries::include);
	if(parts.size() < 2)
	{
		return CUSTOM_SONG_PATH + path;
	}
	return CUSTOM_SONG_PATH + parts[parts.size()-2] + "/" + parts[parts.size()-1];
}

/* "foo.bar", "baz" -> "foo.baz"
 * "foo", "baz" -> "foo.baz"
 * "foo.bar", "" -> "foo" */
std::string SetExtension( const std::string &sPath, const std::string &sExt )
{
	std::string sDir, sFileName, sOldExt;
	splitpath( sPath, sDir, sFileName, sOldExt );
	return sDir + sFileName + (sExt.size()? ".":"") + sExt;
}

std::string GetExtension( const std::string &sPath )
{
	size_t pos = sPath.rfind( '.' );
	if( pos == sPath.npos )
		return std::string();

	size_t slash = sPath.find( '/', pos );
	if( slash != sPath.npos )
		return std::string(); /* rare: path/dir.ext/fn */

	return sPath.substr( pos+1, sPath.size()-pos+1 );
}

std::string GetFileNameWithoutExtension( const std::string &sPath )
{
	std::string sThrowAway, sFName;
	splitpath( sPath, sThrowAway, sFName, sThrowAway );
	return sFName;
}

void MakeValidFilename( std::string &sName )
{
	wstring wsName = StringToWstring( sName );
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

	sName = WStringToString( wsName );
}

bool FindFirstFilenameContaining(vector<std::string> const & filenames,
	std::string & out, vector<std::string> const & starts_with,
	vector<std::string> const & contains, vector<std::string> const & ends_with)
{
	for(size_t i= 0; i < filenames.size(); ++i)
	{
		std::string lower= Rage::make_lower(GetFileNameWithoutExtension(filenames[i]));
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
bool GetCommandlineArgument( const std::string &option, std::string *argument, int iIndex )
{
	const std::string optstr = "--" + option;
	Rage::ci_ascii_string ciOption{ optstr.c_str() };

	for( int arg = 1; arg < g_argc; ++arg )
	{
		const std::string CurArgument = g_argv[arg];

		const size_t i = CurArgument.find( "=" );
		std::string CurOption = CurArgument.substr(0,i);
		if (ciOption != CurOption)
		{
			continue; // no match
		}
		// Found it.
		if( iIndex )
		{
			--iIndex;
			continue;
		}

		if( argument )
		{
			if( i != std::string::npos )
				*argument = CurArgument.substr( i+1 );
			else
				*argument = "";
		}

		return true;
	}

	return false;
}

std::string GetCwd()
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

unsigned int GetHashForString( const std::string &s )
{
	unsigned crc = 0;
	CRC32( crc, s.data(), s.size() );
	return crc;
}

/* Return true if "dir" is empty or does not exist. */
bool DirectoryIsEmpty( const std::string &sDir )
{
	if( sDir.empty() )
	{
		return true;
	}
	if( !DoesFileExist(sDir) )
	{
		return true;
	}
	vector<std::string> asFileNames;
	GetDirListing( sDir, asFileNames );
	return asFileNames.empty();
}

bool CompareStringsAsc( const std::string &sStr1, const std::string &sStr2 )
{
	return Rage::ci_ascii_string{ sStr1.c_str() } < Rage::ci_ascii_string{ sStr2.c_str() };
}

bool CompareStringsDesc( const std::string &sStr1, const std::string &sStr2 )
{
	return Rage::ci_ascii_string{ sStr1.c_str() } > Rage::ci_ascii_string{ sStr2.c_str() };
}

void SortStringArray( vector<std::string> &arrayStrings, const bool bSortAscending )
{
	sort( arrayStrings.begin(), arrayStrings.end(),
			bSortAscending?CompareStringsAsc:CompareStringsDesc );
}

float calc_mean( const float *pStart, const float *pEnd )
{
	return std::accumulate( pStart, pEnd, 0.f ) / std::distance( pStart, pEnd );
}

float calc_stddev( const float *pStart, const float *pEnd, bool bSample )
{
	/* Calculate the mean. */
	float fMean = calc_mean( pStart, pEnd );

	/* Calculate stddev. */
	float fDev = 0.0f;
	for( const float *i=pStart; i != pEnd; ++i )
		fDev += (*i - fMean) * (*i - fMean);
	fDev /= std::distance( pStart, pEnd ) - (bSample ? 1 : 0);
  fDev = std::sqrt( fDev );

	return fDev;
}

bool CalcLeastSquares( const vector< std::pair<float, float> > &vCoordinates,
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
  fError = std::sqrt( fError );
	return true;
}

void FilterHighErrorPoints( vector< std::pair<float, float> > &vCoordinates,
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

void StripCrnl( std::string &s )
{
	while( s.size() && (s[s.size()-1] == '\r' || s[s.size()-1] == '\n') )
	{
		s.erase( s.size()-1 );
	}
}

std::string URLEncode( const std::string &sStr )
{
	std::string sOutput;
	for( unsigned k = 0; k < sStr.size(); k++ )
	{
		char t = sStr[k];
		if( t >= '!' && t <= 'z' )
			sOutput += t;
		else
			sOutput += "%" + fmt::sprintf( "%02X", t );
	}
	return sOutput;
}

// remove various version control-related files
static bool CVSOrSVN( const std::string& s )
{
	Rage::ci_ascii_string cvs{ "CVS" };
	Rage::ci_ascii_string svn{ ".svn" };
	Rage::ci_ascii_string hg{ ".hg" };

	return cvs == Rage::tail(s, 3) || svn == Rage::tail(s, 4) || hg == Rage::tail(s, 3);
}

void StripCvsAndSvn( vector<std::string> &vs )
{
	RemoveIf( vs, CVSOrSVN );
}

static bool MacResourceFork(const std::string& s)
{
	return Rage::ci_ascii_string{ "._" } == Rage::head(s, 2);
}

void StripMacResourceForks( vector<std::string> &vs )
{
	RemoveIf( vs, MacResourceFork );
}

// path is a .redir pathname. Read it and return the real one.
std::string DerefRedir( const std::string &_path )
{
	std::string sPath = _path;

	for( int i=0; i<100; i++ )
	{
		if( GetExtension(sPath) != "redir" )
			return sPath;

		std::string sNewFileName;
		GetFileContents( sPath, sNewFileName, true );

		// Empty is invalid.
		if( sNewFileName == "" )
			return std::string();

		std::string sPath2 = Rage::dir_name(sPath) + sNewFileName;

		CollapsePath( sPath2 );

		sPath2 += "*";

		vector<std::string> matches;
		GetDirListing( sPath2, matches, false, true );

		if( matches.empty() )
		{
			RageException::Throw( "The redirect \"%s\" references a file \"%s\" which doesn't exist.", sPath.c_str(), sPath2.c_str() );
		}
		else if( matches.size() > 1 )
		{
			RageException::Throw( "The redirect \"%s\" references a file \"%s\" with multiple matches.", sPath.c_str(), sPath2.c_str() );
		}
		sPath = matches[0];
	}

	RageException::Throw( "Circular redirect \"%s\".", sPath.c_str() );
}

bool GetFileContents( const std::string &sPath, std::string &sOut, bool bOneLine )
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
	std::string sData;
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

bool GetFileContents( const std::string &sFile, vector<std::string> &asOut )
{
	RageFile file;
	if( !file.Open(sFile) )
	{
		LOG->Warn( "GetFileContents(%s): %s", sFile.c_str(), file.GetError().c_str() );
		return false;
	}

	std::string sLine;
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

void Regex::Set( const std::string &sStr )
{
	Release();
	m_sPattern = sStr;
	Compile();
}

void Regex::Release()
{
	pcre_free( m_pReg );
	m_pReg = nullptr;
	m_sPattern = std::string();
}

Regex::Regex( const std::string &sStr ): m_pReg(nullptr), m_iBackrefs(0), m_sPattern(std::string())
{
	Set( sStr );
}

Regex::Regex( const Regex &rhs ): m_pReg(nullptr), m_iBackrefs(0), m_sPattern(std::string())
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

bool Regex::Compare( const std::string &sStr )
{
	int iMat[128*3];
	int iRet = pcre_exec( (pcre *) m_pReg, nullptr, sStr.data(), sStr.size(), 0, 0, iMat, 128*3 );

	if( iRet < -1 )
		RageException::Throw( "Unexpected return from pcre_exec('%s'): %i.", m_sPattern.c_str(), iRet );

	return iRet >= 0;
}

bool Regex::Compare( const std::string &sStr, vector<std::string> &asMatches )
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
			asMatches.push_back( std::string() ); /* no match */
		else
			asMatches.push_back( sStr.substr(iStart, end - iStart) );
	}

	return true;
}

// Arguments and behavior are the same are similar to
// http://us3.php.net/manual/en/function.preg-replace.php
bool Regex::Replace( const std::string &sReplacement, const std::string &sSubject, std::string &sOut )
{
	vector<std::string> asMatches;
	if( !Compare(sSubject, asMatches) )
		return false;

	sOut = sReplacement;

	// TODO: optimize me by iterating only once over the string
	for( unsigned i=0; i<asMatches.size(); i++ )
	{
		std::string sFrom = fmt::sprintf( "\\${%d}", i );
		std::string sTo = asMatches[i];
		Rage::replace(sOut, sFrom, sTo);
	}

	return true;
}

static int UnicodeDoUpper( char *p, size_t iLen, const unsigned char pMapping[256] )
{
	// Note: this has problems with certain accented characters. -aj
	wchar_t wc = L'\0';
	unsigned iStart = 0;
	if( !Rage::utf8_to_wchar(p, iLen, iStart, wc) )
		return 1;

	wchar_t iUpper = wc;
	if( wc < 256 )
		iUpper = pMapping[wc];
	if( iUpper != wc )
	{
		std::string sOut;
		Rage::wchar_to_utf8( iUpper, sOut );
		if( sOut.size() == iStart )
			memcpy( p, sOut.data(), sOut.size() );
		else
			WARN( fmt::sprintf("UnicodeDoUpper: invalid character at \"%s\"", std::string(p,iLen).c_str()) );
	}

	return iStart;
}

int StringToInt(const std::string &str)
{
	try
	{
		return std::stoi(str);
	}
	catch(...)
	{
		return 0;
	}
}

float StringToFloat(const std::string &str)
{
	try
	{
		float ret= std::stof(str);
		if(!isfinite(ret))
		{
			ret = 0.0f;
		}
		return ret;
	}
	catch(...)
	{
		return 0.0f;
	}
}

bool StringToFloat(const std::string &str, float &ret)
{
	try
	{
		ret= std::stof(str);
		return isfinite(ret);
	}
	catch(...)
	{
		ret= 0.0f;
		return false;
	}
}

std::string FloatToString(const float &num)
{
	stringstream ss;
	ss << num;
	return ss.str();
}

wstring StringToWstring( const std::string &s )
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
		if( !Rage::utf8_to_wchar( s.data(), s.size(), start, ch ) )
		{
			ch = Rage::invalid_char;
		}
		ret += ch;
	}

	return ret;
}

std::string WStringToString( const wstring &sStr )
{
	std::string sRet;

	for( unsigned i = 0; i < sStr.size(); ++i )
		Rage::wchar_to_utf8( sStr[i], sRet );

	return sRet;
}

std::string WcharToUTF8( wchar_t c )
{
	std::string ret;
	Rage::wchar_to_utf8( c, ret );
	return ret;
}

// &a; -> a
void ReplaceEntityText( std::string &sText, std::map<std::string,std::string> const &m )
{
	std::string sRet;

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

		std::string sElement = Rage::make_lower(sText.substr( iStart+1, iEnd-iStart-1 ));

		auto it = m.find( sElement );
		if( it == m.end() )
		{
			sRet.append( sText, iStart, iEnd-iStart+1 );
			iOffset = iEnd + 1;
			continue;
		}

		const std::string &sTo = it->second;
		sRet.append( sTo );
		iOffset = iEnd + 1;
	}

	sText = sRet;
}

// abcd -> &a; &b; &c; &d;
void ReplaceEntityText( std::string &sText, std::map<char,std::string> const &m )
{
	std::string sFind;

	for (auto const &c: m)
	{
		sFind.append(1, c.first);
	}

	std::string sRet;

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

		auto it = m.find( sElement );
		ASSERT( it != m.end() );

		const std::string &sTo = it->second;
		sRet.append( 1, '&' );
		sRet.append( sTo );
		sRet.append( 1, ';' );
		++iOffset;
	}

	sText = sRet;
}

// Replace &#nnnn; (decimal) and &xnnnn; (hex) with corresponding UTF-8 characters.
void Replace_Unicode_Markers( std::string &sText )
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
		{
			iNum = Rage::invalid_char;
		}
		sText.replace( iPos, p-iPos, WcharToUTF8(wchar_t(iNum)) );
	}
}

// Form a string to identify a wchar_t with ASCII.
std::string WcharDisplayText( wchar_t c )
{
	char ascii = '\0';
	if (c < 128)
	{
		ascii = static_cast<char>(c);
	}

	std::string hex = Rage::hexify(c, 4);

	if (ascii != '\0')
	{
		hex = fmt::format("U+{0} ('{1}')", hex, ascii);
	}

	return hex;
}
std::string Capitalize( const std::string &s )
{
	if( s.empty() )
		return std::string();

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

void FixSlashesInPlace( std::string &sPath )
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

void CollapsePath( std::string &sPath, bool bRemoveLeadingDot )
{
	std::string sOut;
	sOut.reserve( sPath.size() );

	size_t iPos = 0;
	size_t iNext;
	for( ; iPos < sPath.size(); iPos = iNext )
	{
		// Find the next slash.
		iNext = sPath.find( '/', iPos );
		if( iNext == std::string::npos )
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
			if( iPrev == std::string::npos )
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
	template<> bool FromString<int>( const std::string &sValue, int &out )
	{
		if( sscanf( sValue.c_str(), "%d", &out ) == 1 )
			return true;

		out = 0;
		return false;
	}

	template<> bool FromString<unsigned>( const std::string &sValue, unsigned  &out )
	{
		if( sscanf( sValue.c_str(), "%u", &out ) == 1 )
			return true;

		out = 0;
		return false;
	}

	template<> bool FromString<float>( const std::string &sValue, float &out )
	{
		const char *endptr = sValue.data() + sValue.size();
		out = strtof( sValue.c_str(), (char **) &endptr );
		if( endptr != sValue.data() && isfinite( out ) )
			return true;
		out = 0;
		return false;
	}

	template<> bool FromString<bool>( const std::string &sValue, bool &out )
	{
		if( sValue.size() == 0 )
			return false;

		out = (StringToInt(sValue) != 0);
		return true;
	}

	template<> std::string ToString<int>( const int &value )
	{
		return fmt::sprintf( "%i", value );
	}

	template<> std::string ToString<unsigned>( const unsigned &value )
	{
		return fmt::sprintf( "%u", value );
	}

	template<> std::string ToString<float>( const float &value )
	{
		return fmt::sprintf( "%f", value );
	}

	template<> std::string ToString<bool>( const bool &value )
	{
		return fmt::sprintf( "%i", value );
	}
}

bool FileCopy( std::string const &sSrcFile, std::string const &sDstFile )
{
	if (Rage::ci_ascii_string{ sSrcFile.c_str() } == sDstFile)
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

	std::string sError;
	if( !FileCopy(in, out, sError) )
	{
		LOG->Warn( "FileCopy(%s,%s): %s",
				sSrcFile.c_str(), sDstFile.c_str(), sError.c_str() );
		return false;
	}

	return true;
}

bool FileCopy( RageFileBasic &in, RageFileBasic &out, std::string &sError, bool *bReadError )
{
	for(;;)
	{
		std::string data;
		if( in.Read(data, 1024*32) == -1 )
		{
			sError = fmt::sprintf( "read error: %s", in.GetError().c_str() );
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
			sError = fmt::sprintf( "write error: %s", out.GetError().c_str() );
			if( bReadError != nullptr )
			{
				*bReadError = false;
			}
			return false;
		}
	}

	if( out.Flush() == -1 )
	{
		sError = fmt::sprintf( "write error: %s", out.GetError().c_str() );
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
LuaFunction( Basename, Rage::base_name( SArg(1) ) )
static std::string MakeLower( std::string s )
{
	return Rage::make_lower(s);
}
LuaFunction( Lowercase, MakeLower( SArg(1) ) )
static std::string MakeUpper( std::string s )
{
	return Rage::make_upper(s);
}
LuaFunction( Uppercase, MakeUpper( SArg(1) ) )
LuaFunction( mbstrlen, (int)StringToWstring(SArg(1)).length() )
LuaFunction( URLEncode, URLEncode( SArg(1) ) );
LuaFunction( PrettyPercent, PrettyPercent( FArg(1), FArg(2) ) );
//LuaFunction( IsHexVal, IsHexVal( SArg(1) ) );
static bool UndocumentedFeature( std::string s ){ sm_crash(s); return true; }
LuaFunction( UndocumentedFeature, UndocumentedFeature(SArg(1)) );
LuaFunction( lerp, Rage::lerp(FArg(1), FArg(2), FArg(3)) );

int LuaFunc_commify(lua_State* L);
int LuaFunc_commify(lua_State* L)
{
	std::string num= SArg(1);
	std::string sep= ",";
	std::string dot= ".";
	if(!lua_isnoneornil(L, 2))
	{
		sep= lua_tostring(L, 2);
	}
	if(!lua_isnoneornil(L, 3))
	{
		dot= lua_tostring(L, 3);
	}
	std::string ret= Commify(num, sep, dot);
	LuaHelpers::Push(L, ret);
	return 1;
}
LUAFUNC_REGISTER_COMMON(commify);

void luafunc_approach_internal(lua_State* L, int valind, int goalind, int speedind, const float mult);
void luafunc_approach_internal(lua_State* L, int valind, int goalind, int speedind, const float mult, int process_index)
{
#define TONUMBER_NICE(dest, num_name, index) \
if(!lua_isnumber(L, index)) \
{ \
	luaL_error(L, "approach: " #num_name " for approach %d is not a number.", process_index); \
} \
dest= static_cast<float>(lua_tonumber(L, index))

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
		mult= static_cast<float>(lua_tonumber(L, 4));
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
	std::string path= SArg(1);
	std::string error;
	RageSoundReader* sample= RageSoundReader_FileReader::OpenFile(path, error);
	if(sample == nullptr)
	{
		luaL_error(L, "The music file '%s' does not exist.", path.c_str());
	}
	else
	{
		lua_pushnumber(L, sample->GetLength() / 1000.0f);
	}
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
