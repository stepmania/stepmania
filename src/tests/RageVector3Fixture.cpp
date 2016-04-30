#include "RageVector3Fixture.hpp"

void RageVector3Fixture::SetUp()
{
	a = { 1, 2, 3 };
	b = { 4, 6, 8 };

	sum = { 5, 8, 11 };
	difference = { -3, -4, -5 };
	product = { 2.5, 5, 7.5 };
	quotient = { 1, 1.5, 2 };
}
