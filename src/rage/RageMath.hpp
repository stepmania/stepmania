#ifndef RAGE_MATH_HPP_
#define RAGE_MATH_HPP_

namespace Rage
{

/** @brief Bring a value within range. */
template<typename T>
T clamp( T const & val, T const & low, T const & high )
{
	return std::max(low, std::min(val, high));
}

/** @brief Interpolate within the ranges and interlopant. */
template<typename T, typename U>
inline U lerp( T x, U l, U h )
{
	return static_cast<U>( (h - l) * x + l );
}

/** @brief Scale the target number so that the two targets match.
 *
 * This does not modify x, so it MUST assign the result to something!
 * Do the multiply before the divide so that integer scales have more precision.
 *
 * One such example: scale(x, 0, 1, L, H); interpolate between L and H.
 */
template<typename T>
T scale(T x, T l1, T h1, T l2, T h2)
{
	if (l1 == 0 && h1 == 1)
	{
		return lerp(x, l2, h2);
	}
	return (x - l1) * (h2 - l2) / (h1 - l1) + l2;
}

}

#endif
