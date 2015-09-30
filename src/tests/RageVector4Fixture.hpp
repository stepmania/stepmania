#ifndef RAGE_VECTOR_4_FIXTURE_HPP_
#define RAGE_VECTOR_4_FIXTURE_HPP_

#include "gtest/gtest.h"
#include "RageVector4.hpp"

class RageVector4Fixture : public ::testing::Test
{
protected:
	virtual void SetUp();

	Rage::Vector4 a;
	Rage::Vector4 b;
	Rage::Vector4 sum;
	Rage::Vector4 difference;
	Rage::Vector4 product;
	Rage::Vector4 quotient;
};

#endif
