#include "RageString.hpp"
#include <cstdlib>
#include <sstream>

using namespace Rage;

std::string Rage::head(std::string const &source, int32_t const length)
{
	if (std::abs(length) >= source.size())
	{
		return source;
	}
	if (length < 0)
	{
		return source.substr(0, source.size() + length);
	}
	return source.substr(0, length);
}

std::string Rage::tail(std::string const &source, int32_t const length)
{
	if (std::abs(length) >= source.size())
	{
		return source;
	}
	if (length < 0)
	{
		return source.substr(-length);
	}
	return source.substr(source.size() - length);
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
