#include "RageVector4Fixture.hpp"

TEST_F(RageVector4Fixture, addition)
{
	Rage::Vector4 c = a + b;

	EXPECT_EQ(sum.x, c.x);
	EXPECT_EQ(sum.y, c.y);
	EXPECT_EQ(sum.z, c.z);
	EXPECT_EQ(sum.w, c.w);
	EXPECT_EQ(sum, c);
}

TEST_F(RageVector4Fixture, subtraction)
{
	Rage::Vector4 c = a - b;

	EXPECT_EQ(difference.x, c.x);
	EXPECT_EQ(difference.y, c.y);
	EXPECT_EQ(difference.z, c.z);
	EXPECT_EQ(difference.w, c.w);
	EXPECT_EQ(difference, c);
}

TEST_F(RageVector4Fixture, multiplication)
{
	Rage::Vector4 c = a * 2.5;

	EXPECT_EQ(product.x, c.x);
	EXPECT_EQ(product.y, c.y);
	EXPECT_EQ(product.z, c.z);
	EXPECT_EQ(product.w, c.w);
	EXPECT_EQ(product, c);
}

TEST_F(RageVector4Fixture, division)
{
	Rage::Vector4 c = b / 4.f;

	EXPECT_EQ(quotient.x, c.x);
	EXPECT_EQ(quotient.y, c.y);
	EXPECT_EQ(quotient.z, c.z);
	EXPECT_EQ(quotient.w, c.w);
	EXPECT_EQ(quotient, c);
}
