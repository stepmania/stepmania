#include "RageVColor.hpp"
#include "RageColor.hpp"
#include <cmath>

/* Convert floating-point 0..1 value to integer 0..255 value. *
 *
 * As a test case,
 *
 * int cnts[1000]; memset(cnts, 0, sizeof(cnts));
 * for( float n = 0; n <= 1.0; n += 0.0001 ) cnts[ftoc(n)]++;
 * for( int i = 0; i < 256; ++i ) printf("%i ", cnts[i]);
 *
 * should output the same value (+-1) 256 times.  If this function is
 * incorrect, the first and/or last values may be biased. */
uint8_t ftoc(float a);

uint8_t ftoc(float a)
{
	/* std::lrint is much faster than C casts.  We don't care which way negative values
	 * are rounded, since we'll clamp them to zero below.  Be sure to truncate (not
	 * round) positive values.  The highest value that should be converted to 1 is
	 * roughly (1/256 - 0.00001); if we don't truncate, values up to (1/256 + 0.5)
	 * will be converted to 1, which is wrong. */
	int ret = std::lrint(a * 256.f - 0.5f);
	
	/* Benchmarking shows that clamping here, as integers, is much faster than clamping
	 * before the conversion, as floats. */
	if (ret < 0)
	{
		return 0;
	}
	if (ret > 255)
	{
		return 255;
	}
	
	return static_cast<uint8_t>(ret);
}

Rage::VColor::VColor(): b(0), g(0), r(0), a(0)
{
}

Rage::VColor::VColor(uint8_t blue, uint8_t green, uint8_t red, uint8_t alpha): b(blue), g(green), r(red), a(alpha)
{
}

Rage::VColor::VColor(Rage::Color const &color): b(ftoc(color.b)), g(ftoc(color.g)), r(ftoc(color.r)), a(ftoc(color.a))
{
}

Rage::VColor & Rage::VColor::operator=(Rage::Color const &rhs)
{
	b = ftoc(rhs.b);
	g = ftoc(rhs.g);
	r = ftoc(rhs.r);
	a = ftoc(rhs.a);
	return *this;
}
