#include "RageString.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>
#include "RageUnicode.hpp"

void make_upper( char *p, size_t len );
void make_lower( char *p, size_t len );

std::string Rage::head(std::string const &source, int32_t const length)
{
	auto sourceSize = static_cast<int>(source.size());
	if (std::abs(length) >= sourceSize)
	{
		return source;
	}
	if (length < 0)
	{
		return source.substr(0, sourceSize + length);
	}
	return source.substr(0, length);
}

std::string Rage::tail(std::string const &source, int32_t const length)
{
	auto sourceSize = static_cast<int>(source.size());
	if (std::abs(length) >= sourceSize)
	{
		return source;
	}
	if (length < 0)
	{
		return source.substr(-length);
	}
	return source.substr(sourceSize - length);
}

bool Rage::starts_with(std::string const &source, std::string const &target)
{
	return Rage::head(source, target.size()) == target;
}

bool Rage::ends_with(std::string const &source, std::string const &target)
{
	return Rage::tail(source, target.size()) == target;
}

std::string Rage::hexify(wchar_t const src, unsigned int dstlen)
{
	static char const alphabet[] = "0123456789abcdef";

	std::stringstream builder;
	
	unsigned int i = 0;
	wchar_t const *ptr = &src;
	
	while (*ptr && ( 2 * i ) + 1 < dstlen)
	{
		builder << alphabet[*ptr / 16];
		builder << alphabet[*ptr % 16];
		++ptr;
		++i;
	}
	
	std::string setup{builder.str()};
	while (setup.size() < dstlen)
	{
		setup = '0' + setup;
	}
	return setup;
}

void Rage::replace(std::string &target, char from, char to)
{
	std::replace(target.begin(), target.end(), from, to);
}

void Rage::replace(std::string &target, std::string const &from, std::string const &to)
{
	std::string newString;
	newString.reserve(target.length());  // avoids a few memory allocations

	std::string::size_type lastPos = 0;
	std::string::size_type findPos;

	while (std::string::npos != (findPos = target.find(from, lastPos)))
	{
		newString.append(target, lastPos, findPos - lastPos);
		newString += to;
		lastPos = findPos + from.length();
	}

	// Care for the rest after last occurrence
	newString += target.substr(lastPos);

	target.swap(newString);
}

std::string Rage::join(std::string const &delimiter, std::vector<std::string> const &source)
{
	if (source.empty())
	{
		return "";
	}
	return Rage::join(delimiter, source.begin(), source.end());
}

std::string Rage::join(std::string const &delimiter, std::vector<std::string>::const_iterator start, std::vector<std::string>::const_iterator finish )
{
	if (start == finish)
	{
		return "";
	}
	std::stringstream builder;
	
	auto append = [&builder, &delimiter](std::string const &target) {
		builder << target;
		builder << delimiter;
	};
	auto inclusive = finish - 1;
	std::for_each(start, inclusive, append);
	
	builder << *inclusive;
	return builder.str();
}

template <class S>
static int DelimitorLength( S const &delimitor )
{
	return delimitor.size();
}

static int DelimitorLength( char delimitor )
{
	return 1;
}

static int DelimitorLength( wchar_t delimitor )
{
	return 1;
}

template <class S, class C>
std::vector<S> do_split( S const &source, C const delimitor, Rage::EmptyEntries const shouldIgnore )
{
	std::vector<S> result;
	/* Short-circuit if the source is empty; we want to return an empty vector if
	 * the string is empty, even if bIgnoreEmpty is true. */
	if( source.empty() )
	{
		return result;
	}
	size_t startpos = 0;

	do {
		size_t pos { source.find( delimitor, startpos ) };
		if( pos == source.npos )
		{
			pos = source.size();
		}
		if( pos != startpos || shouldIgnore == Rage::EmptyEntries::include )
		{
			/* Optimization: if we're copying the whole string, avoid substr; this
			 * allows this copy to be refcounted, which is much faster. */
			if( startpos == 0 && pos - startpos == source.size() )
			{
				result.push_back(source);
			}
			else
			{
				S const target {source.substr(startpos, pos-startpos) };
				result.push_back(target);
			}
		}

		startpos = pos + DelimitorLength(delimitor);
	} while ( startpos <= source.size() );

	return result;
}

std::vector<std::string> Rage::split(std::string const &source, std::string const &delimiter)
{
	return Rage::split(source, delimiter, Rage::EmptyEntries::skip);
}
std::vector<std::string> Rage::split(std::string const &source, std::string const &delimiter, Rage::EmptyEntries shouldIgnoreEmptyEntries)
{
	if (delimiter.size() == 1)
	{
		// TODO: Look into an optimized character string tokenizer. Perhaps std::getline?
		return do_split(source, delimiter[0], shouldIgnoreEmptyEntries);
	}
	return do_split(source, delimiter, shouldIgnoreEmptyEntries);
}

std::vector<std::wstring> Rage::split(std::wstring const &source, std::wstring const &delimiter)
{
	return Rage::split(source, delimiter, Rage::EmptyEntries::include);
}
std::vector<std::wstring> Rage::split(std::wstring const &source, std::wstring const &delimiter, Rage::EmptyEntries shouldIgnoreEmptyEntries)
{
	if (delimiter.size() == 1)
	{
		return do_split(source, delimiter[0], shouldIgnoreEmptyEntries);
	}
	return do_split(source, delimiter, shouldIgnoreEmptyEntries);
}

template <class S>
void do_split( S const &source, S const &delimitor, int &start, int &size, int len, Rage::EmptyEntries const shouldIgnore )
{
	using std::min;
	if( size != -1 )
	{
		// Start points to the beginning of the last delimiter. Move it up.
		start += size + delimitor.size();
		start = min( start, len );
	}

	size = 0;

	if( shouldIgnore == Rage::EmptyEntries::skip )
	{
		while( start + delimitor.size() < source.size() &&
			  !source.compare( start, delimitor.size(), delimitor ) )
		{
			++start;
		}
	}

	/* Where's the string function to find within a substring?
	 * C++ strings apparently are missing that ... */
	size_t pos;
	if( delimitor.size() == 1 )
	{
		pos = source.find( delimitor[0], start );
	}
	else
	{
		pos = source.find( delimitor, start );
	}
	if( pos == source.npos || static_cast<int>(pos) > len )
	{
		pos = len;
	}
	size = pos - start;
}

void Rage::split_in_place( std::string const &source, std::string const &delimitor, int &start, int &size )
{
	do_split(source, delimitor, start, size, source.size(), Rage::EmptyEntries::skip);
}

void Rage::split_in_place( std::string const &source, std::string const &delimitor, int &start, int &size, Rage::EmptyEntries shouldIgnore )
{
	do_split(source, delimitor, start, size, source.size(), shouldIgnore);
}

void Rage::split_in_place( std::wstring const &source, std::wstring const &delimitor, int &start, int &size )
{
	do_split(source, delimitor, start, size, source.size(), Rage::EmptyEntries::skip);
}

void Rage::split_in_place( std::wstring const &source, std::wstring const &delimitor, int &start, int &size, Rage::EmptyEntries shouldIgnore )
{
	do_split(source, delimitor, start, size, source.size(), shouldIgnore);
}

void Rage::split_in_place( std::string const &source, std::string const &delimitor, int &start, int &size, int len )
{
	do_split(source, delimitor, start, size, len, Rage::EmptyEntries::skip);
}

void Rage::split_in_place( std::string const &source, std::string const &delimitor, int &start, int &size, int len, Rage::EmptyEntries shouldIgnore )
{
	do_split(source, delimitor, start, size, len, shouldIgnore);
}

void Rage::split_in_place( std::wstring const &source, std::wstring const &delimitor, int &start, int &size, int len )
{
	do_split(source, delimitor, start, size, len, Rage::EmptyEntries::skip);
}

void Rage::split_in_place( std::wstring const &source, std::wstring const &delimitor, int &start, int &size, int len, Rage::EmptyEntries shouldIgnore )
{
	do_split(source, delimitor, start, size, len, shouldIgnore);
}

std::string Rage::trim_left(std::string const &source)
{
	return Rage::trim_left(source, "\r\n\t ");
}

std::string Rage::trim_left(std::string const &source, std::string const &delimiters)
{
	auto n = 0;
	auto end = static_cast<int>(source.size());
	auto const *d_str = delimiters.c_str();
	while (n < end && std::strchr(d_str, source[n]))
	{
		++n;
	}
	return source.substr(n);
}

std::string Rage::trim_right(std::string const &source)
{
	return Rage::trim_right(source, "\r\n\t ");
}

std::string Rage::trim_right(std::string const &source, std::string const &delimiters)
{
	int n = source.size();
	auto const *d_str = delimiters.c_str();
	while( n > 0 && std::strchr(d_str, source[n - 1]) )
	{
		n--;
	}
	
	return source.substr(0, n);
}

std::string Rage::trim(std::string const &source)
{
	return Rage::trim(source, "\r\n\t ");
}

std::string Rage::trim(std::string const &source, std::string const &delimiters)
{
	std::string::size_type start = 0;
	std::string::size_type lastPos = source.size();
	auto const *d_str = delimiters.c_str();
	while ( start < lastPos && std::strchr(d_str, source[start]))
	{
		++start;
	}
	while (start < lastPos && std::strchr(d_str, source[lastPos - 1]))
	{
		--lastPos;
	}
	return source.substr(start, lastPos - start);
}

std::string Rage::base_name(std::string const &dir)
{
	size_t iEnd = dir.find_last_not_of( "/\\" );
	if( iEnd == dir.npos )
	{
		return "";
	}
	size_t iStart = dir.find_last_of( "/\\", iEnd );
	if( iStart == dir.npos )
	{
		iStart = 0;
	}
	else
	{
		++iStart;
	}
	return dir.substr( iStart, iEnd-iStart+1 );
}

std::string Rage::dir_name(std::string const &dir)
{
	// Special case: "/" -> "/".
	if( dir.size() == 1 && dir[0] == '/' )
	{
		return "/";
	}
	int pos = dir.size()-1;
	// Skip trailing slashes.
	while( pos >= 0 && dir[pos] == '/' )
	{
		--pos;
	}
	// Skip the last component.
	while( pos >= 0 && dir[pos] != '/' )
	{
		--pos;
	}
	if( pos < 0 )
	{
		return "./";
	}
	return dir.substr(0, pos+1);
}

/* Branch optimizations: */
#if defined(__GNUC__) || defined(__clang__)
#define likely(x) (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

int unicode_do_casing( char *p, size_t iLen, const unsigned char pMapping[256] )
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
		{
			std::memcpy( p, sOut.data(), sOut.size() );
		}
		else
		{
			// TODO: Find another
			// WARN( fmt::sprintf("UnicodeDoUpper: invalid character at \"%s\"", RString(p,iLen).c_str()) );
		}
	}
	
	return iStart;
}

void make_upper( char *p, size_t len )
{
	char *start = p;
	char *end = p + len;
	while( p < end )
	{
		// Fast path:
		if( likely( !(*p & 0x80) ) )
		{
			if( unlikely(*p >= 'a' && *p <= 'z') )
				*p += 'A' - 'a';
			++p;
			continue;
		}
		
		int iRemaining = len - (p - start);
		p += unicode_do_casing( p, iRemaining, Rage::upperCase );
	}
}

void make_lower( char *p, size_t len )
{
	char *start = p;
	char *end = p + len;
	while( p < end )
	{
		// Fast path:
		if( likely( !(*p & 0x80) ) )
		{
			if( unlikely(*p >= 'A' && *p <= 'Z') )
				*p -= 'A' - 'a';
			++p;
			continue;
		}
		
		int iRemaining = len - (p - start);
		p += unicode_do_casing( p, iRemaining, Rage::lowerCase );
	}
}

std::string Rage::make_upper(std::string const &source)
{
	std::vector<char> buffer{source.begin(), source.end()};
	
	// Ensure a null terminating character is in place just in case.
	buffer.push_back(0);
	
	::make_upper(&buffer[0], source.size());
	
	return std::string{buffer.begin(), buffer.end() - 1};
}

std::string Rage::make_lower(std::string const &source)
{
	std::vector<char> buffer{source.begin(), source.end()};
	
	// Ensure a null terminating character is in place just in case.
	buffer.push_back(0);
	
	::make_lower(&buffer[0], source.size());
	
	return std::string{buffer.begin(), buffer.end() - 1};
}
