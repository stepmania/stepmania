#include "RageVector2Fixture.hpp"

TEST_F(RageVector2Fixture, addition)
{
	RageVector2 c = a1 + b1;

	EXPECT_EQ(sum1.x, c.x);
	EXPECT_EQ(sum1.y, c.y);
	EXPECT_EQ(sum1, c);
}

TEST_F(RageVector2Fixture, subtraction)
{
	RageVector2 c = a1 - b1;

	EXPECT_EQ(difference1.x, c.x);
	EXPECT_EQ(difference1.y, c.y);
	EXPECT_EQ(difference1, c);
}

TEST_F(RageVector2Fixture, multiplication)
{
	RageVector2 c = a1 * 2.5f;

	EXPECT_EQ(product1.x, c.x);
	EXPECT_EQ(product1.y, c.y);
	EXPECT_EQ(product1, c);
}

TEST_F(RageVector2Fixture, division)
{
	RageVector2 c = b1 / 2;

	EXPECT_EQ(quotient1.x, c.x);
	EXPECT_EQ(quotient1.y, c.y);
	EXPECT_EQ(quotient1, c);
}
