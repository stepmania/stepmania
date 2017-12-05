#include "RageColor.hpp"
#include "RageMath.hpp"
#include <sstream>
#include <cmath>
#include "fmt/format.h"

Rage::Color::Color(): r(0), g(0), b(0), a(0)
{
}

Rage::Color::Color(float red, float green, float blue, float alpha): r(red), g(green), b(blue), a(alpha)
{
}

Rage::Color & Rage::Color::operator+=(Rage::Color const &rhs)
{
	r += rhs.r;
	g += rhs.g;
	b += rhs.b;
	a += rhs.a;
	return *this;
}

Rage::Color & Rage::Color::operator-=(Rage::Color const &rhs)
{
	r -= rhs.r;
	g -= rhs.g;
	b -= rhs.b;
	a -= rhs.a;
	return *this;
}

Rage::Color & Rage::Color::operator*=(Rage::Color const &rhs)
{
	r *= rhs.r;
	g *= rhs.g;
	b *= rhs.b;
	a *= rhs.a;
	return *this;
}

Rage::Color & Rage::Color::operator*=(float rhs)
{
	r *= rhs;
	g *= rhs;
	b *= rhs;
	a *= rhs;
	return *this;
}

Rage::Color & Rage::Color::operator/=(float rhs)
{
	r /= rhs;
	g /= rhs;
	b /= rhs;
	a /= rhs;
	return *this;
}

std::string Rage::Color::NormalizeColorString(std::string color)
{
	if (color.empty())
	{
		return "";
	}
	Rage::Color c;
	if (!c.FromString(color))
	{
		return "";
	}
	return c.ToString();
}

std::string Rage::Color::ToString() const
{
	int iR = Rage::clamp( static_cast<int>(std::lrint(r * 255) ), 0, 255 );
	int iG = Rage::clamp( static_cast<int>(std::lrint(g * 255) ), 0, 255 );
	int iB = Rage::clamp( static_cast<int>(std::lrint(b * 255) ), 0, 255 );
	int iA = Rage::clamp( static_cast<int>(std::lrint(a * 255) ), 0, 255 );
	
	if( iA == 255 )
	{
		return fmt::format("#{0:02X}{1:02X}{2:02X}", iR, iG, iB);
	}
	return fmt::format("#{0:02X}{1:02X}{2:02X}{3:02X}", iR, iG, iB, iA);
}

bool Rage::Color::FromCommaString(std::string const &colorString)
{
	auto countCommas = [](char i) {
		return i == ',';
	};
	
	int numCommas = std::count_if(colorString.begin(), colorString.end(), countCommas);
	
	if (numCommas != 2 && numCommas != 3)
	{
		return false;
	}
	
	std::istringstream ss(colorString);
	std::string token;
	
	try
	{
		std::getline(ss, token, ',');
		r = std::stof(token);
		std::getline(ss, token, ',');
		g = std::stof(token);
		std::getline(ss, token, ',');
		b = std::stof(token);
		if (numCommas == 3)
		{
			std::getline(ss, token, ',');
			a = std::stof(token);
		}
		else
		{
			a = 1;
		}
		
		return
			r >= 0 && r <= 1 &&
			g >= 0 && g <= 1 &&
			b >= 0 && b <= 1 &&
			a >= 0 && a <= 1;
	}
	catch (std::invalid_argument const &)
	{
		return false;
	}
}

bool Rage::Color::FromString(std::string const &colorString)
{
	bool wasSuccessful = false;
	if (!colorString.empty())
	{
		if (colorString.front() == '#')
		{
			wasSuccessful = FromHexString(colorString.substr(1));
		}
		else
		{
			wasSuccessful = FromCommaString(colorString);
		}
	}
	
	if ( !wasSuccessful )
	{
		r = g = b = a = 1;
	}
	
	return wasSuccessful;
}

bool Rage::Color::FromHexString( std::string const &colorString )
{
	auto size = colorString.size();
	if ( size != 6 && size != 8 )
	{
		return false;
	}
	
	// validate that it is hex characters only.
	auto isHexChar = [](char i) {
		return ( i >= '0' && i <= '9' ) || ( i >= 'A' && i <= 'F') || ( i >= 'a' && i <= 'f' );
	};
	
	bool isHex = std::all_of(colorString.begin(), colorString.end(), isHexChar);
	
	if (!isHex)
	{
		return false;
	}
	
	// Borrow from http://stackoverflow.com/a/18398562 and http://stackoverflow.com/a/1582509
	unsigned long hexNum = std::stoul(colorString, 0, 16);
	
	if ( size == 6 )
	{
		a = 1;
	}
	else
	{
		a = ( hexNum & 0xFF ) / 255.f;
		hexNum >>= 8;
	}
	
	b = ( hexNum & 0xFF ) / 255.f;
	hexNum >>= 8;
	g = ( hexNum & 0xFF ) / 255.f;
	hexNum >>= 8;
	r = hexNum / 255.f;
	
	return true;
}
