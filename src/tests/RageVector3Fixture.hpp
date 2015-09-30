#ifndef RAGE_VECTOR_3_FIXTURE_HPP_
#define RAGE_VECTOR_3_FIXTURE_HPP_

#include "gtest/gtest.h"
#include "RageVector3.hpp"

class RageVector3Fixture : public ::testing::Test
{
protected:
	virtual void SetUp();

	Rage::Vector3 a;
	Rage::Vector3 b;
	Rage::Vector3 sum;
	Rage::Vector3 difference;
	Rage::Vector3 product;
	Rage::Vector3 quotient;
};

#endif
