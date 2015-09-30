#include "RageVector3Fixture.hpp"

TEST_F(RageVector3Fixture, addition)
{
	Rage::Vector3 c = a + b;

	EXPECT_EQ(sum.x, c.x);
	EXPECT_EQ(sum.y, c.y);
	EXPECT_EQ(sum.z, c.z);
	EXPECT_EQ(sum, c);
}

TEST_F(RageVector3Fixture, subtraction)
{
	Rage::Vector3 c = a - b;

	EXPECT_EQ(difference.x, c.x);
	EXPECT_EQ(difference.y, c.y);
	EXPECT_EQ(difference.z, c.z);
	EXPECT_EQ(difference, c);
}

TEST_F(RageVector3Fixture, multiplication)
{
	Rage::Vector3 c = a * 2.5;

	EXPECT_EQ(product.x, c.x);
	EXPECT_EQ(product.y, c.y);
	EXPECT_EQ(product.z, c.z);
	EXPECT_EQ(product, c);
}

TEST_F(RageVector3Fixture, division)
{
	Rage::Vector3 c = b / 4.f;

	EXPECT_EQ(quotient.x, c.x);
	EXPECT_EQ(quotient.y, c.y);
	EXPECT_EQ(quotient.z, c.z);
	EXPECT_EQ(quotient, c);
}
