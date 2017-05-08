#ifndef RAGE_V_COLOR_HPP_
#define RAGE_V_COLOR_HPP_

#include <cstdint>

/* Color type used only in vertex lists.  OpenGL expects colors in
 * r, g, b, a order, independent of endianness, so storing them this
 * way avoids endianness problems.  Don't try to manipulate this; only
 * manip Rage::Colors. */
/* Perhaps the math in Rage::Color could be moved to Rage::VColor.  We don't need the
 * precision of a float for our calculations anyway.   -Chris */

namespace Rage
{
	struct Color;
	struct VColor
	{
		VColor();
		VColor(uint8_t blue, uint8_t green, uint8_t red, uint8_t alpha);
		VColor(Color const &color);
		
		// Explicit assigning when using a Rage::Color
		VColor & operator=(Color const &rhs);
		
		// This order is defined by Direct3D. Do not change.
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};
	
	inline bool operator==(VColor const &lhs, VColor const &rhs)
	{
		return
			lhs.b == rhs.b &&
			lhs.g == rhs.g &&
			lhs.r == rhs.r &&
			lhs.a == rhs.a;
	}
	
	inline bool operator!=(VColor const &lhs, VColor const &rhs)
	{
		return !operator==(lhs, rhs);
	}
}

#endif
