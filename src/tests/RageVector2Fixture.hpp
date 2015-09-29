#ifndef RAGE_VECTOR_2_FIXTURE_HPP_
#define RAGE_VECTOR_2_FIXTURE_HPP_

#include "gtest/gtest.h"
#include "RageVector2.hpp"

class RageVector2Fixture : public ::testing::Test
{
protected:
	virtual void SetUp();

	RageVector2 a1;
	RageVector2 b1;
	RageVector2 sum1;
	RageVector2 difference1;
	RageVector2 product1;
	RageVector2 quotient1;
};

#endif
