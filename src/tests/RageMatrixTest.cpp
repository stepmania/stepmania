#include "RageMatrix.hpp"
#include "gtest/gtest.h"

using namespace Rage;

GTEST_TEST(RageMatrix, identity_multiplication)
{
	Matrix a = Matrix::GetIdentity();
	Matrix b
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		427, 240, 0, 1
	};
	
	Matrix c = a * b;
	
	EXPECT_EQ(b, c);
}

GTEST_TEST(RageMatrix, identity_var1)
{
	Matrix a
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		427, 240, 0, 1
	};
	Matrix b
	{
		854, 0, 0, 0,
		0, 128, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	Matrix c
	{
		854, 0, 0, 0,
		0, 128, 0, 0,
		0, 0, 1, 0,
		364658, 30720, 0, 1
	};
	Matrix d
	{
		854, 0, 0, 0,
		0, 128, 0, 0,
		0, 0, 1, 0,
		427, 240, 0, 1
	};
	
	EXPECT_EQ(c, a * b);
	EXPECT_EQ(d, b * a);
}
