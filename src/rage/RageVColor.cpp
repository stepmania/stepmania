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
inline uint8_t ftoc(float a);

inline uint8_t ftoc(float a)
{
	//This value is 2^52 * 1.5.
	const double INT_MANTISSA = 6755399441055744.0;

	/* Be sure to truncate (not round) positive values. The highest value that
	 * should be converted to 1 is roughly(1 / 256 - 0.00001); if we don't
	 * truncate, values up to (1/256 + 0.5) will be converted to 1, which is
	 * wrong. */
	double base = double(a * 256.f - 0.5f);

	/* INT_MANTISSA is chosen such that, when added to a sufficiently small
	 * double, the mantissa bits of that double can be reinterpreted as that
	 * number rounded to an integer. This is done to improve performance. */
	base += INT_MANTISSA;
	int ret = reinterpret_cast<int&>(base);
	
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
