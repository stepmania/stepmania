#include "gtest/gtest.h"
#include "RageVColor.hpp"
#include "RageColor.hpp"

GTEST_TEST(RageVColor, red_sanity)
{
	Rage::Color red { 1, 0, 0, 1 };
	
	Rage::VColor vRed = red;
	
	Rage::VColor target { 0, 0, 255, 255 };
	
	EXPECT_EQ(vRed, target);
}