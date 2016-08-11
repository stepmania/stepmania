#include "RageVector4Fixture.hpp"

void RageVector4Fixture::SetUp()
{
	a = { 1, 2, 3, 4 };
	b = { 4, 6, 8, 16 };

	sum = { 5, 8, 11, 20 };
	difference = { -3, -4, -5, -12 };
	product = { 2.5, 5, 7.5, 10 };
	quotient = { 1, 1.5, 2, 4 };
}
