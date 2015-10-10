#include "RageString.hpp"
#include <cstdlib>

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
