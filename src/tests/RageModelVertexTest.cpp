#include "gtest/gtest.h"
#include "RageModelVertex.hpp"

GTEST_TEST(RageModelVertex, sane_defaults)
{
	using namespace Rage;
	
	ModelVertex target;
	
	Vector3 norm3;
	Vector2 norm2;
	
	EXPECT_EQ(norm3, target.n);
	EXPECT_EQ(norm3, target.p);
	EXPECT_EQ(norm2, target.t);
  EXPECT_EQ(0, target.bone);
  EXPECT_EQ((Vector2{1, 1}), target.TextureMatrixScale);
}
