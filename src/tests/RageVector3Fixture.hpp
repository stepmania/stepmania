#ifndef RAGE_VECTOR_3_FIXTURE_HPP_
#define RAGE_VECTOR_3_FIXTURE_HPP_

#include "gtest/gtest.h"
#include "RageVector3.hpp"

class RageVector3Fixture : public ::testing::Test
{
protected:
	virtual void SetUp();

	RageVector3 a;
	RageVector3 b;
	RageVector3 sum;
	RageVector3 difference;
	RageVector3 product;
	RageVector3 quotient;
};

#endif
