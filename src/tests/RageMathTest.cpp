#include "gtest/gtest.h"
#include "RageMath.hpp"
#include "RageVector2.hpp"

using Rage::clamp;
using Rage::lerp;
using Rage::scale;

GTEST_TEST(clamp, within_range)
{
	int actual = clamp(4, 2, 6);
	
	EXPECT_EQ(4, actual);
}

GTEST_TEST(clamp, below_range)
{
	int actual = clamp(1, 3, 5);
	
	EXPECT_EQ(3, actual);
}

GTEST_TEST(clamp, above_range)
{
	int actual = clamp(100, 20, 60);
	
	EXPECT_EQ(60, actual);
}

GTEST_TEST(lerp, interlopant_0)
{
	int actual = lerp(0, 0, 100);
	
	EXPECT_EQ(0, actual);
}

GTEST_TEST(lerp, interlopant_1)
{
	int actual = lerp(1, 0, 100);
	
	EXPECT_EQ(100, actual);
}

GTEST_TEST(lerp, interlopant_point5)
{
	int actual = lerp(0.5, 0, 100);
	
	EXPECT_EQ(50, actual);
}

GTEST_TEST(lerp, vectors_too)
{
	Rage::Vector2 a { 0, 0 };
	Rage::Vector2 b { 10, 20 };
	
	Rage::Vector2 actual = lerp(0.6f, a, b);
	EXPECT_EQ( (Rage::Vector2{6, 12}), actual);
	
}

GTEST_TEST(scale, basic_interpolation)
{
	auto actual = scale(0.5f, 0.f, 1.f, 0.f, 1.f);
	
	EXPECT_EQ(0.5f, actual);
}

GTEST_TEST(scale, int_bad_idea)
{
	int actual = scale(1, 0, 1, 50, 150);
	
	EXPECT_EQ(150, actual);
}
