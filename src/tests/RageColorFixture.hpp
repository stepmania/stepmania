#ifndef RAGE_COLOR_FIXTURE_HPP_
#define RAGE_COLOR_FIXTURE_HPP_

#include "gtest/gtest.h"
#include "RageColor.hpp"

class RageColorFixture: public ::testing::Test
{
protected:
	virtual void SetUp();
	
	Rage::Color red;
	Rage::Color green;
	Rage::Color blue;
};

#endif

