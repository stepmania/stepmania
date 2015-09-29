#ifndef RAGE_VECTOR_4_FIXTURE_HPP_
#define RAGE_VECTOR_4_FIXTURE_HPP_

#include "gtest/gtest.h"
#include "RageVector4.hpp"

class RageVector4Fixture : public ::testing::Test
{
protected:
	virtual void SetUp();

	RageVector4 a;
	RageVector4 b;
	RageVector4 sum;
	RageVector4 difference;
	RageVector4 product;
	RageVector4 quotient;
};

#endif
