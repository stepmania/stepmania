#include "gtest/gtest.h"
#include "RageSpriteVertex.hpp"

GTEST_TEST(RageSpriteVertex, sane_defaults)
{
	using namespace Rage;
	
	SpriteVertex target;
	
	Vector3 norm3;
	Vector2 norm2;
	VColor normC;
	
	EXPECT_EQ(norm3, target.n);
	EXPECT_EQ(norm3, target.p);
	EXPECT_EQ(normC, target.c);
	EXPECT_EQ(norm2, target.t);
}
