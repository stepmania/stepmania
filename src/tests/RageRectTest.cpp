#include "gtest/gtest.h"
#include "RageRect.hpp"

GTEST_TEST(RageRect, int_congruent)
{
	using namespace Rage;
	
	RectI test{2, 2, 4, 4};
	
	RectI similar{5, 5, 7, 7};
	
	EXPECT_EQ(test.GetWidth(), similar.GetWidth());
	EXPECT_EQ(test.GetHeight(), similar.GetHeight());
}

GTEST_TEST(RageRect, float_center)
{
	using namespace Rage;
	
	RectF test;
	
	RectF bigger{ -1, -1, 1, 1 };
	
	EXPECT_EQ(test.GetCenterX(), bigger.GetCenterX());
	EXPECT_EQ(test.GetCenterY(), bigger.GetCenterY());
}
