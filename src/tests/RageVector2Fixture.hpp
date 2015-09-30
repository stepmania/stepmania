#ifndef RAGE_VECTOR_2_FIXTURE_HPP_
#define RAGE_VECTOR_2_FIXTURE_HPP_

#include "gtest/gtest.h"
#include "RageVector2.hpp"

class RageVector2Fixture : public ::testing::Test
{
protected:
	virtual void SetUp();

	Rage::Vector2 a1;
	Rage::Vector2 b1;
	Rage::Vector2 sum1;
	Rage::Vector2 difference1;
	Rage::Vector2 product1;
	Rage::Vector2 quotient1;
};

#endif
