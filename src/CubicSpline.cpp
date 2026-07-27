#include "global.h"
#include "CubicSpline.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <cstddef>
#include <list>
#include <vector>


// Spline solving optimization:
// The tridiagonal part of the system of equations for a spline of size n is
//   the same for all splines of size n.  It's not affected by the positions
//   of the points.
// So spline solving can be split into two parts.  Part 1 solves the
//   tridiagonal and stores the result.  Part 2 takes the solved tridiagonal
//   and applies it to the positions to find the coefficients.
// Part 1 only needs to be done when the number of points changes.  So this
//   could cut solve time for the same number of points substantially.
// Further optimization is to cache the part 1 results for the last 16 spline
//   sizes solved, to reduce the cost of using lots of splines with a small
//   number of sizes.

struct SplineSolutionCache
{
	struct Entry
	{
		std::vector<float> diagonals;
		std::vector<float> multiples;
	};
	void solve_diagonals_straight(std::vector<float>& diagonals, std::vector<float>& multiples);
	void solve_diagonals_looped(std::vector<float>& diagonals, std::vector<float>& multiples);
private:
	void prep_inner(std::size_t last, std::vector<float>& out);
	bool find_in_cache(std::list<Entry>& cache, std::vector<float>& outd, std::vector<float>& outm);
	void add_to_cache(std::list<Entry>& cache, std::vector<float>& outd, std::vector<float>& outm);
	std::list<Entry> straight_diagonals;
	std::list<Entry> looped_diagonals;
};

const std::size_t solution_cache_limit= 16;

bool SplineSolutionCache::find_in_cache(std::list<Entry>& cache, std::vector<float>& outd, std::vector<float>& outm)
{
	std::size_t out_size= outd.size();
	for(std::list<Entry>::iterator entry= cache.begin();
			entry != cache.end(); ++entry)
	{
		if(out_size == entry->diagonals.size())
		{
			for(std::size_t i= 0; i < out_size; ++i)
			{
				outd[i]= entry->diagonals[i];
			}
			outm.resize(entry->multiples.size());
			for(std::size_t i= 0; i < entry->multiples.size(); ++i)
			{
				outm[i]= entry->multiples[i];
			}
			return true;
		}
	}
	return false;
}

void SplineSolutionCache::add_to_cache(std::list<Entry>& cache, std::vector<float>& outd, std::vector<float>& outm)
{
	if(cache.size() >= solution_cache_limit)
	{
		cache.pop_back();
	}
	cache.push_front(Entry());
	cache.front().diagonals= outd;
	cache.front().multiples= outm;
}

void SplineSolutionCache::prep_inner(std::size_t last, std::vector<float>& out)
{
	for(std::size_t i= 1; i < last; ++i)
	{
		out[i]= 4.0f;
	}
}

void SplineSolutionCache::solve_diagonals_straight(std::vector<float>& diagonals, std::vector<float>& multiples)
{
	if(find_in_cache(straight_diagonals, diagonals, multiples))
	{
		return;
	}

	// Solution steps:
	// Two stages:  First, work downwards, zeroing the 1s below each diagonal.
	// | 2 1 0 0 | -> | 2 1 0 0 | -> | 2 1 0 0 | -> | 2 1 0 0 |
	// | 1 4 1 0 | -> | 0 a 1 0 | -> | 0 d 1 0 | -> | 0 a 1 0 |
	// | 0 1 4 1 | -> | 0 1 4 1 | -> | 0 0 b 1 | -> | 0 0 b 1 |
	// | 0 0 1 2 | -> | 0 0 1 2 | -> | 0 0 1 2 | -> | 0 0 0 c |
	// Second stage:  Work upwards, zeroing the 1s above each diagonal.
	// V
	// | 2 1 0 0 | -> | 2 1 0 0 | -> | 2 0 0 0 |
	// | 0 a 1 0 | -> | 0 a 0 0 | -> | 0 a 0 0 |
	// | 0 0 b 0 | -> | 0 0 b 0 | -> | 0 0 b 0 |
	// | 0 0 0 c | -> | 0 0 0 c | -> | 0 0 0 c |

	std::size_t last= diagonals.size();
	diagonals[0]= 2.0f;
	prep_inner(last-1, diagonals);
	diagonals[last-1]= 2.0f;

	// Stage one.
	// Operation:  Add row[0] * -.5 to row[1] to zero [r1][c0].
	diagonals[1]-= .5f;
	multiples.push_back(.5f);
	for(std::size_t i= 1; i < last-1; ++i)
	{
		// Operation:  Add row[i] / -[ri][ci] to row[i+1] to zero [ri+1][ci].
		const float diag_recip= 1.0f / diagonals[i];
		diagonals[i+1]-= diag_recip;
		multiples.push_back(diag_recip);
	}
	// Stage two.
	for(std::size_t i= last-1; i > 0; --i)
	{
		// Operation:  Add row [i] / -[ri][ci] to row[i-1] to zero [ri-1][ci].
		multiples.push_back(1.0f / diagonals[i]);
	}
  // Solving finished.
	add_to_cache(straight_diagonals, diagonals, multiples);
}

void SplineSolutionCache::solve_diagonals_looped(std::vector<float>& diagonals, std::vector<float>& multiples)
{
	if(find_in_cache(looped_diagonals, diagonals, multiples))
	{
		return;
	}

	// The steps to solve the system of equations look like this:
	// Stage one:  Zero the 1s below the diagonals.
	// | 4 1 0 0 1 | -> | 4 1 0 0 1 | -> | 4 1 0 0 1 | -> | 4 1 0 0 1 |
	// | 1 4 1 0 0 | -> | 0 a 1 0 u | -> | 0 a 1 0 u | -> | 0 a 1 0 u |
	// | 0 1 4 1 0 | -> | 0 1 4 1 0 | -> | 0 0 b 1 v | -> | 0 0 b 1 v |
	// | 0 0 1 4 1 | -> | 0 0 1 4 1 | -> | 0 0 1 4 1 | -> | 0 0 0 c w |
	// | 1 0 0 1 4 | -> | 1 0 0 1 4 | -> | 1 0 0 1 4 | -> | 1 0 0 1 4 |
	// V
	// | 4 1 0 0 1 |
	// | 0 a 1 0 u |
	// | 0 0 b 1 v |
	// | 0 0 0 c w |
	// | 1 0 0 0 d |
	// The top of the right column is left unzeroed because it will be changed
	// by stage two, nullifying the effect of zeroing it.
	// V Stage two:  Zero the 1s above the diagonals, starting with the second
	//   to last row to avoid carrying effects across the left column.
	// | 4 1 0 0 1 | -> | 4 1 0 0 1 | -> | 4 0 0 0 z | -> | 4 0 0 0 z |
	// | 0 a 1 0 u | -> | 0 a 0 0 y | -> | 0 a 0 0 y | -> | 0 a 0 0 y |
	// | 0 0 b 0 x | -> | 0 0 b 0 x | -> | 0 0 b 0 x | -> | 0 0 b 0 x |
	// | 0 0 0 c w | -> | 0 0 0 c w | -> | 0 0 0 c w | -> | 0 0 0 c w |
	// | 1 0 0 0 d | -> | 1 0 0 0 d | -> | 1 0 0 0 d | -> | 0 0 0 0 f |
	// V Stage three:  Zero the right column.
	// | 4 0 0 0 0 | -> | 4 0 0 0 0 | -> | 4 0 0 0 0 | -> | 4 0 0 0 0 |
	// | 0 a 0 0 y | -> | 0 a 0 0 0 | -> | 0 a 0 0 0 | -> | 0 a 0 0 0 |
	// | 0 0 b 0 x | -> | 0 0 b 0 x | -> | 0 0 b 0 0 | -> | 0 0 b 0 0 |
	// | 0 0 0 c w | -> | 0 0 0 c w | -> | 0 0 0 c w | -> | 0 0 0 c 0 |
	// | 0 0 0 0 f | -> | 0 0 0 0 f | -> | 0 0 0 0 f | -> | 0 0 0 0 f |

	std::size_t last= diagonals.size();
	diagonals[0]= 4.0f;
	prep_inner(last, diagonals);
	// right_column is sized to not store the diagonal .
	std::vector<float> right_column(diagonals.size()-1, 0.0f);
	right_column[0]= 1.0f;
	right_column[last-2]= 1.0f;

	// Stage one.
	for(std::size_t i= 0; i < last-2; ++i)
	{
		// Operation:  Add row[i] / -[ri][ci] to row[i+1] to zero [ri+1][ci].
		const float diag_recip= 1.0f / diagonals[i];
		diagonals[i+1]-= diag_recip;
		right_column[i+1]-= right_column[i] * diag_recip;
		multiples.push_back(diag_recip);
	}
	// Last step of stage one needs special handling for right_column.
	// Operation: Add row[l-2] / [rl-2][cl-2] to row[l-1] to zero [rl-1][cl-2].
	{
		const float diag_recip= 1.0f / diagonals[last-2];
		diagonals[last-1]-= right_column[last-2] * diag_recip;
		multiples.push_back(diag_recip);
	}
	// Stage two.
	for(std::size_t i= last-2; i > 0; --i)
	{
		// Operation: Add row[i] / -[ri][ci] to row[i-1] to zero [ri-1][ci].
		const float diag_recip= 1.0f / diagonals[i];
		right_column[i-1]-= right_column[i] * diag_recip;
		multiples.push_back(diag_recip);
	}
	// Last step of stage two.
	{
		// Operation: Add row[0] / [r0][c0] to row[l-1] to zero [rl-1][c0].
		const float diag_recip= 1.0f / diagonals[0];
		right_column[0]-= right_column[1] * diag_recip;
		multiples.push_back(diag_recip);
	}
	// Stage three.
	const std::size_t end= last-1;
	for(std::size_t i= 0; i < end; ++i)
	{
		// Operation: Add row[e] * (right_column[i] / [re][ce]) to row[i] to
		// zero right_column[i].
		multiples.push_back(right_column[i] / diagonals[end]);
	}

  // Solving finished.
	add_to_cache(looped_diagonals, diagonals, multiples);
}

SplineSolutionCache solution_cache;

// loop_space_difference exists to handle numbers that exist in a finite
// looped space, instead of the flat infinite space.
// To put it more concretely, loop_space_difference exists to allow a spline
// to control rotation with wrapping behavior at 0.0 and 2pi, instead of
// suddenly jerking from 2pi to 0.0. -Kyz
float loop_space_difference(float a, float b, float spatial_extent);
float loop_space_difference(float a, float b, float spatial_extent)
{
	const float norm_diff= a - b;
	if(spatial_extent == 0.0f) { return norm_diff; }
	const float plus_diff= a - (b + spatial_extent);
	const float minus_diff= a - (b - spatial_extent);
	const float abs_norm_diff= std::abs(norm_diff);
	const float abs_plus_diff= std::abs(plus_diff);
	const float abs_minus_diff= std::abs(minus_diff);
	if(abs_norm_diff < abs_plus_diff)
	{
		if(abs_norm_diff < abs_minus_diff)
		{
			return norm_diff;
		}
		if(abs_plus_diff < abs_minus_diff)
		{
			return plus_diff;
		}
		return minus_diff;
	}
	if(abs_plus_diff < abs_minus_diff)
	{
		return plus_diff;
	}
	return minus_diff;
}

void CubicSpline::solve_looped()
{
	if(check_minimum_size()) { return; }
	std::size_t last= m_points.size();
	std::vector<float> results(m_points.size());
	std::vector<float> diagonals(m_points.size());
	std::vector<float> multiples;
	solution_cache.solve_diagonals_looped(diagonals, multiples);
	results[0]= 3 * loop_space_difference(
		m_points[1].a, m_points[last-1].a, m_spatial_extent);
	prep_inner(last, results);
	results[last-1]= 3 * loop_space_difference(
		m_points[0].a, m_points[last-2].a, m_spatial_extent);

	// Steps explained in detail in SplineSolutionCache.
	// Only the operations on the results column are performed here.
	// Stage one.
	// SplineSolutionCache's Stage one loop ends at last-2 because it has to
	// handle right_column.  This does not handle right_column, so the loop
	// goes to last-1.
	for(std::size_t i= 0; i < last-1; ++i)
	{
		// Operation: Add row[i] * -multiples[i] to row[i+1].
		results[i+1]-= results[i] * multiples[i];
	}
	std::size_t next_mult= last-1;
	// Stage two.
	for(std::size_t i= last-2; i > 0; --i)
	{
		// Operation: Add row[i] * -multiples[nm] to row[i-1].
		results[i-1]-= results[i] * multiples[next_mult];
		++next_mult;
	}
	// Last step of stage two.
	// Operation: Add row[0] * -multiples[nm] to row[l-1].
	results[last-1]-= results[0] * multiples[next_mult];
	++next_mult;
	// Stage three.
	const std::size_t end= last-1;
	for(std::size_t i= 0; i < end; ++i)
	{
		// Operation: Add row[e] * -multiples[nm] to row[i].
		results[i]-= results[end] * multiples[next_mult];
		++next_mult;
	}
	// Solving finished.
	set_results(last, diagonals, results);
}

void CubicSpline::solve_straight()
{
	if(check_minimum_size()) { return; }
	std::size_t last= m_points.size();
	std::vector<float> results(m_points.size());
	std::vector<float> diagonals(m_points.size());
	std::vector<float> multiples;
	solution_cache.solve_diagonals_straight(diagonals, multiples);
	results[0]= 3 * (m_points[1].a - m_points[0].a);
	prep_inner(last, results);
	results[last-1]= 3 * loop_space_difference(
		m_points[last-1].a, m_points[last-2].a, m_spatial_extent);

	// Steps explained in detail in SplineSolutionCache.
	// Only the operations on the results column are performed here.
	// Stage one.
	for(std::size_t i= 0; i < last-1; ++i)
	{
		// Operation: Add row[i] * -multiples[i] to row[i+1].
		results[i+1]-= results[i] * multiples[i];
	}
	std::size_t next_mult= last-1;
	// Stage two.
	for(std::size_t i= last-1; i > 0; --i)
	{
		// Operation: Add row[i] * -multiples[nm] to row [i-1].
		results[i-1]-= results[i] * multiples[next_mult];
		++next_mult;
	}
	// Solving finished.
	set_results(last, diagonals, results);
}

void CubicSpline::solve_polygonal()
{
	if(check_minimum_size()) { return; }
	std::size_t last= m_points.size() - 1;
	for(std::size_t i= 0; i < last; ++i)
	{
		m_points[i].b= loop_space_difference(
			m_points[i+1].a, m_points[i].a, m_spatial_extent);
	}
	m_points[last].b= loop_space_difference(
		m_points[0].a, m_points[last].a, m_spatial_extent);
}

bool CubicSpline::check_minimum_size()
{
	std::size_t last= m_points.size();
	if(last < 2)
	{
		m_points[0].b= m_points[0].c= m_points[0].d= 0.0f;
		return true;
	}
	if(last == 2)
	{
		m_points[0].b= loop_space_difference(
			m_points[1].a, m_points[0].a, m_spatial_extent);
		m_points[0].c= m_points[0].d= 0.0f;
		// These will be used in the looping case.
		m_points[1].b= loop_space_difference(
			m_points[0].a, m_points[1].a, m_spatial_extent);
		m_points[1].c= m_points[1].d= 0.0f;
		return true;
	}
	float a= m_points[0].a;
	bool all_points_identical= true;
	for(std::size_t i= 0; i < m_points.size(); ++i)
	{
		m_points[i].b= m_points[i].c= m_points[i].d= 0.0f;
		if(m_points[i].a != a) { all_points_identical= false; }
	}
	return all_points_identical;
}

void CubicSpline::prep_inner(std::size_t last, std::vector<float>& results)
{
	for(std::size_t i= 1; i < last - 1; ++i)
	{
		results[i]= 3 * loop_space_difference(
			m_points[i+1].a, m_points[i-1].a, m_spatial_extent);
	}
}

void CubicSpline::set_results(std::size_t last, std::vector<float>& diagonals, std::vector<float>& results)
{
	// No more operations left, everything not a diagonal should be zero now.
	for(std::size_t i= 0; i < last; ++i)
	{
		results[i]/= diagonals[i];
	}
	// Now we can go through and set the b, c, d values of each point.
	// b, c, d values of the last point are not set because they are unused.
	for(std::size_t i= 0; i < last; ++i)
	{
		std::size_t next= (i+1) % last;
		float diff= loop_space_difference(
			m_points[next].a, m_points[i].a, m_spatial_extent);
		m_points[i].b= results[i];
		m_points[i].c= (3 * diff) - (2 * results[i]) - results[next];
		m_points[i].d= (2 * -diff) + results[i] + results[next];
#define UNNAN(n) if(n != n) { n = 0.0f; }
		UNNAN(m_points[i].b);
		UNNAN(m_points[i].c);
		UNNAN(m_points[i].d);
#undef UNNAN
	}
	// Solving is now complete.
}

void CubicSpline::p_and_tfrac_from_t(float t, bool loop, std::size_t& p, float& tfrac) const
{
	if(loop)
	{
		float max_t= static_cast<float>(m_points.size());
		t= std::fmod(t, max_t);
		if(t < 0.0f) { t+= max_t; }
		p= static_cast<std::size_t>(t);
		tfrac= t - static_cast<float>(p);
	}
	else
	{
		int flort= static_cast<int>(t);
		if(flort < 0)
		{
			p= 0;
			tfrac= 0;
		}
		else if(static_cast<std::size_t>(flort) >= m_points.size() - 1)
		{
			p= m_points.size() - 1;
			tfrac= 0;
		}
		else
		{
			p= static_cast<std::size_t>(flort);
			tfrac= t - static_cast<float>(p);
		}
	}
}

#define RETURN_IF_EMPTY if(m_points.empty()) { return 0.0f; }
#define DECLARE_P_AND_TFRAC \
std::size_t p= 0; float tfrac= 0.0f; \
p_and_tfrac_from_t(t, loop, p, tfrac);

float CubicSpline::evaluate(float t, bool loop) const
{
	RETURN_IF_EMPTY;
	DECLARE_P_AND_TFRAC;
	float tsq= tfrac * tfrac;
	float tcub= tsq * tfrac;
	return m_points[p].a + (m_points[p].b * tfrac) +
		(m_points[p].c * tsq) + (m_points[p].d * tcub);
}

float CubicSpline::evaluate_derivative(float t, bool loop) const
{
	RETURN_IF_EMPTY;
	DECLARE_P_AND_TFRAC;
	float tsq= tfrac * tfrac;
	return m_points[p].b + (2.0f * m_points[p].c * tfrac) +
		(3.0f * m_points[p].d * tsq);
}

float CubicSpline::evaluate_second_derivative(float t, bool loop) const
{
	RETURN_IF_EMPTY;
	DECLARE_P_AND_TFRAC;
	return (2.0f * m_points[p].c) + (6.0f * m_points[p].d * tfrac);
}

float CubicSpline::evaluate_third_derivative(float t, bool loop) const
{
	RETURN_IF_EMPTY;
	DECLARE_P_AND_TFRAC;
	return 6.0f * m_points[p].d;
}

#undef RETURN_IF_EMPTY
#undef DECLARE_P_AND_TFRAC

void CubicSpline::set_point(std::size_t i, float v)
{
	ASSERT_M(i < m_points.size(), "CubicSpline::set_point requires the index to be less than the number of points.");
	m_points[i].a= v;
}

void CubicSpline::set_coefficients(std::size_t i, float b, float c, float d)
{
	ASSERT_M(i < m_points.size(), "CubicSpline: point index must be less than the number of points.");
	m_points[i].b= b;
	m_points[i].c= c;
	m_points[i].d= d;
}

void CubicSpline::get_coefficients(std::size_t i, float& b, float& c, float& d) const
{
	ASSERT_M(i < m_points.size(), "CubicSpline: point index must be less than the number of points.");
	b= m_points[i].b;
	c= m_points[i].c;
	d= m_points[i].d;
}

void CubicSpline::set_point_and_coefficients(std::size_t i, float a, float b,
	float c, float d)
{
	set_coefficients(i, b, c, d);
	m_points[i].a= a;
}

void CubicSpline::get_point_and_coefficients(std::size_t i, float& a, float& b,
	float& c, float& d) const
{
	get_coefficients(i, b, c, d);
	a= m_points[i].a;
}

void CubicSpline::resize(std::size_t s)
{
	m_points.resize(s);
}

std::size_t CubicSpline::size() const
{
	return m_points.size();
}

bool CubicSpline::empty() const
{
	return m_points.empty();
}

void CubicSplineN::weighted_average(CubicSplineN& out,
	const CubicSplineN& from, CubicSplineN const& to, float between)
{
	ASSERT_M(out.dimension() == from.dimension() &&
		to.dimension() == from.dimension(),
		"Cannot tween splines of different dimensions.");
#define BOOLS_FROM_CLOSEST(closest) \
	out.set_loop(closest.get_loop()); \
	out.set_polygonal(closest.get_polygonal());
	if(between >= 0.5f)
	{
		BOOLS_FROM_CLOSEST(to);
	}
	else
	{
		BOOLS_FROM_CLOSEST(from);
	}
#undef BOOLS_FROM_CLOSEST
	// Behavior for splines of different sizes:  Use a size between the two.
	// Points that exist in both will be averaged.
	// Points that only exist in one will come only from that one.
	const std::size_t from_size= from.size();
	const std::size_t to_size= to.size();
	std::size_t out_size= to_size;
	std::size_t limit= to_size;
	if(from_size < to_size)
	{
		out_size= from_size + static_cast<std::size_t>(
			static_cast<float>(to_size - from_size) * between);
	}
	else if(to_size < from_size)
	{
		limit= from_size;
		out_size= to_size + static_cast<std::size_t>(
			static_cast<float>(from_size - to_size) * between);
	}
	CLAMP(out_size, 0, limit);
	out.resize(out_size);

	for(std::size_t spli= 0; spli < out.m_splines.size(); ++spli)
	{
		for(std::size_t p= 0; p < out_size; ++p)
		{
			float fc[4]= {0.0f, 0.0f, 0.0f, 0.0f};
			float tc[4]= {0.0f, 0.0f, 0.0f, 0.0f};
			if(p < from_size)
			{
				from.m_splines[spli].get_point_and_coefficients(p, fc[0], fc[1],
					fc[2], fc[3]);
			}
			if(p < to_size)
			{
				to.m_splines[spli].get_point_and_coefficients(p, tc[0], tc[1],
					tc[2], tc[3]);
			}
			else
			{
				for(int i= 0; i < 4; ++i)
				{
					tc[i]= fc[i];
				}
			}
			if(p >= from_size)
			{
				for(int i= 0; i < 4; ++i)
				{
					fc[i]= tc[i];
				}
			}
			float oc[4]= {0.0f, 0.0f, 0.0f, 0.0f};
			for(int i= 0; i < 4; ++i)
			{
				oc[i]= lerp(between, fc[i], tc[i]);
			}
			out.m_splines[spli].set_point_and_coefficients(p, oc[0], oc[1], oc[2],
				oc[3]);
		}
	}
	// The spline is not solved after averaging because my testing showed that
	// it is unnecessary.
	// My testing method was this:
	// Spline A is generated by lerping all points and coefficients.
	// Spline B is generated by lerping all points then solving.
	// The coefficients for Spline A and Spline B are identical to 5 to 9
	// significant digits.  Thus, solving is unnecessary.
	// Additionally, solving would require a mechanism to disable solving for
	// the people that wish to set their own coefficients instead of solving.
	// -Kyz
}

void CubicSplineN::solve()
{
	if(!m_dirty) { return; }
#define SOLVE_LOOP(solvent) \
	for(spline_cont_t::iterator spline= m_splines.begin(); \
		spline != m_splines.end(); ++spline) \
	{ \
		spline->solvent(); \
	}
	if(m_polygonal)
	{
		SOLVE_LOOP(solve_polygonal);
	}
	else
	{
		if(m_loop)
		{
			SOLVE_LOOP(solve_looped);
		}
		else
		{
			SOLVE_LOOP(solve_straight);
		}
	}
#undef SOLVE_LOOP
	m_dirty= false;
}

#define CSN_EVAL_SOMETHING(something) \
void CubicSplineN::something(float t, std::vector<float>& v) const \
{ \
	for(spline_cont_t::const_iterator spline= m_splines.begin(); \
			spline != m_splines.end(); ++spline) \
	{ \
		v.push_back(spline->something(t, m_loop)); \
	} \
}

CSN_EVAL_SOMETHING(evaluate);
CSN_EVAL_SOMETHING(evaluate_derivative);
CSN_EVAL_SOMETHING(evaluate_second_derivative);
CSN_EVAL_SOMETHING(evaluate_third_derivative);

#undef CSN_EVAL_SOMETHING

#define CSN_EVAL_RV_SOMETHING(something) \
void CubicSplineN::something(float t, RageVector3& v) const \
{ \
	ASSERT(m_splines.size() == 3); \
	v.x= m_splines[0].something(t, m_loop); \
	v.y= m_splines[1].something(t, m_loop); \
	v.z= m_splines[2].something(t, m_loop); \
}

CSN_EVAL_RV_SOMETHING(evaluate);
CSN_EVAL_RV_SOMETHING(evaluate_derivative);

#undef CSN_EVAL_RV_SOMETHING

void CubicSplineN::set_point(std::size_t i, const std::vector<float>& v)
{
	ASSERT_M(v.size() == m_splines.size(), "CubicSplineN::set_point requires the passed point to be the same dimension as the spline.");
	for(std::size_t n= 0; n < m_splines.size(); ++n)
	{
		m_splines[n].set_point(i, v[n]);
	}
	m_dirty= true;
}

void CubicSplineN::set_coefficients(std::size_t i, const std::vector<float>& b,
	const std::vector<float>& c, const std::vector<float>& d)
{
	ASSERT_M(b.size() == c.size() && c.size() == d.size() &&
		d.size() == m_splines.size(), "CubicSplineN: coefficient vectors must be "
		"the same dimension as the spline.");
	for(std::size_t n= 0; n < m_splines.size(); ++n)
	{
		m_splines[n].set_coefficients(i, b[n], c[n], d[n]);
	}
	m_dirty= true;
}

void CubicSplineN::get_coefficients(std::size_t i, std::vector<float>& b,
	std::vector<float>& c, std::vector<float>& d)
{
	ASSERT_M(b.size() == c.size() && c.size() == d.size() &&
		d.size() == m_splines.size(), "CubicSplineN: coefficient vectors must be "
		"the same dimension as the spline.");
	for(std::size_t n= 0; n < m_splines.size(); ++n)
	{
		m_splines[n].get_coefficients(i, b[n], c[n], d[n]);
	}
}

void CubicSplineN::set_spatial_extent(std::size_t i, float extent)
{
	ASSERT_M(i < m_splines.size(), "CubicSplineN: index of spline to set extent"
		" of is out of range.");
	m_splines[i].m_spatial_extent= extent;
	m_dirty= true;
}

float CubicSplineN::get_spatial_extent(std::size_t i)
{
	ASSERT_M(i < m_splines.size(), "CubicSplineN: index of spline to get extent"
		" of is out of range.");
	return m_splines[i].m_spatial_extent;
}

void CubicSplineN::resize(std::size_t s)
{
	for(spline_cont_t::iterator spline= m_splines.begin();
			spline != m_splines.end(); ++spline)
	{
		spline->resize(s);
	}
	m_dirty= true;
}

std::size_t CubicSplineN::size() const
{
	if(!m_splines.empty())
	{
		return m_splines[0].size();
	}
	return 0;
}

bool CubicSplineN::empty() const
{
	return m_splines.empty() || m_splines[0].empty();
}

void CubicSplineN::redimension(std::size_t d)
{
	m_splines.resize(d);
	m_dirty= true;
}

std::size_t CubicSplineN::dimension() const
{
	return m_splines.size();
}

// m_dirty is set before the member so that the set_dirty that is created
// can actually be used to set the dirty flag. -Kyz
#define SET_GET_MEM(member, name) \
void CubicSplineN::set_##name(bool b) \
{ \
	m_dirty= true; \
	member= b; \
} \
bool CubicSplineN::get_##name() const \
{ \
	return member; \
}

SET_GET_MEM(m_loop, loop);
SET_GET_MEM(m_polygonal, polygonal);
SET_GET_MEM(m_dirty, dirty);

#undef SET_GET_MEM

#include "LuaBinding.h"

struct LunaCubicSplineN : Luna<CubicSplineN>
{
	static std::size_t dimension_index(T* p, lua_State* L, int s)
	{
		std::size_t i= static_cast<std::size_t>(IArg(s)-1);
		if(i >= p->dimension())
		{
			luaL_error(L, "Spline dimension index out of range.");
		}
		return i;
	}
	static std::size_t point_index(T* p, lua_State* L, int s)
	{
		std::size_t i= static_cast<std::size_t>(IArg(s)-1);
		if(i >= p->size())
		{
			luaL_error(L, "Spline point index out of range.");
		}
		return i;
	}
	static int solve(T* p, lua_State* L)
	{
		p->solve();
		COMMON_RETURN_SELF;
	}
#define LCSN_EVAL_SOMETHING(something) \
	static int something(T* p, lua_State* L) \
	{ \
		std::vector<float> pos; \
		p->something(FArg(1), pos); \
		lua_createtable(L, pos.size(), 0); \
		for(std::size_t i= 0; i < pos.size(); ++i) \
		{ \
			lua_pushnumber(L, pos[i]); \
			lua_rawseti(L, -2, i+1); \
		} \
		return 1; \
	}
	LCSN_EVAL_SOMETHING(evaluate);
	LCSN_EVAL_SOMETHING(evaluate_derivative);
	LCSN_EVAL_SOMETHING(evaluate_second_derivative);
	LCSN_EVAL_SOMETHING(evaluate_third_derivative);
#undef LCSN_EVAL_SOMETHING

	static void get_element_table_from_stack(T* p, lua_State* L, int s,
		std::size_t limit, std::vector<float>& ret)
	{
		std::size_t elements= lua_objlen(L, s);
		// Too many elements is not an error because allowing it allows the user
		// to reuse the same position data set after changing the dimension size.
		// The same is true for too few elements.
		for(std::size_t e= 0; e < elements; ++e)
		{
			lua_rawgeti(L, s, e+1);
			ret.push_back(FArg(-1));
		}
		while(ret.size() < limit)
		{
			ret.push_back(0.0f);
		}
		ret.resize(limit);
	}
	static void set_point_from_stack(T* p, lua_State* L, std::size_t i, int s)
	{
		if(!lua_istable(L, s))
		{
			luaL_error(L, "Spline point must be a table.");
		}
		std::vector<float> pos;
		get_element_table_from_stack(p, L, s, p->dimension(), pos);
		p->set_point(i, pos);
	}
	static int set_point(T* p, lua_State* L)
	{
		std::size_t i= point_index(p, L, 1);
		set_point_from_stack(p, L, i, 2);
		COMMON_RETURN_SELF;
	}
	static void set_coefficients_from_stack(T* p, lua_State* L, std::size_t i, int s)
	{
		if(!lua_istable(L, s) || !lua_istable(L, s+1) || !lua_istable(L, s+2))
		{
			luaL_error(L, "Spline coefficient args must be three tables.");
		}
		std::size_t limit= p->dimension();
		std::vector<float> b; get_element_table_from_stack(p, L, s, limit, b);
		std::vector<float> c; get_element_table_from_stack(p, L, s+1, limit, c);
		std::vector<float> d; get_element_table_from_stack(p, L, s+2, limit, d);
		p->set_coefficients(i, b, c, d);
	}
	static int set_coefficients(T* p, lua_State* L)
	{
		std::size_t i= point_index(p, L, 1);
		set_coefficients_from_stack(p, L, i, 2);
		COMMON_RETURN_SELF;
	}
	static int get_coefficients(T* p, lua_State* L)
	{
		std::size_t i= point_index(p, L, 1);
		std::size_t limit= p->dimension();
		std::vector<std::vector<float> > coeff(3);
		coeff[0].resize(limit);
		coeff[1].resize(limit);
		coeff[2].resize(limit);
		p->get_coefficients(i, coeff[0], coeff[1], coeff[2]);
		lua_createtable(L, 3, 0);
		for(std::size_t co= 0; co < coeff.size(); ++co)
		{
			lua_createtable(L, limit, 0);
			for(std::size_t v= 0; v < limit; ++v)
			{
				lua_pushnumber(L, coeff[co][v]);
				lua_rawseti(L, -2, v+1);
			}
			lua_rawseti(L, -2, co+1);
		}
		return 1;
	}
	static int set_spatial_extent(T* p, lua_State* L)
	{
		std::size_t i= dimension_index(p, L, 1);
		p->set_spatial_extent(i, FArg(2));
		COMMON_RETURN_SELF;
	}
	static int get_spatial_extent(T* p, lua_State* L)
	{
		std::size_t i= dimension_index(p, L, 1);
		lua_pushnumber(L, p->get_spatial_extent(i));
		return 1;
	}
	static int get_max_t(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_max_t());
		return 1;
	}
	static int set_size(T* p, lua_State* L)
	{
		int siz= IArg(1);
		if(siz < 0)
		{
			luaL_error(L, "A spline cannot have less than 0 points.");
		}
		p->resize(static_cast<std::size_t>(siz));
		COMMON_RETURN_SELF;
	}
	static int get_size(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->size());
		return 1;
	}
	static int set_dimension(T* p, lua_State* L)
	{
		if(p->m_owned_by_actor)
		{
			luaL_error(L, "This spline cannot be redimensioned because it is "
				"owned by an actor that relies on it having fixed dimensions.");
		}
		int dim= IArg(1);
		if(dim < 0)
		{
			luaL_error(L, "A spline cannot have less than 0 dimensions.");
		}
		p->redimension(static_cast<std::size_t>(dim));
		COMMON_RETURN_SELF;
	}
	static int get_dimension(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->dimension());
		return 1;
	}
	static int empty(T* p, lua_State* L)
	{
		lua_pushboolean(L, p->empty());
		return 1;
	}
#define SET_GET_LUA(name) \
	static int set_##name(T* p, lua_State* L) \
	{ \
		p->set_##name(lua_toboolean(L, 1)); \
		COMMON_RETURN_SELF; \
	} \
	static int get_##name(T* p, lua_State* L) \
	{ \
		lua_pushboolean(L, p->get_##name()); \
		return 1; \
	}
	SET_GET_LUA(loop);
	SET_GET_LUA(polygonal);
	SET_GET_LUA(dirty);
#undef SET_GET_LUA
	static int destroy(T* p, lua_State* L)
	{
		if(p->m_owned_by_actor)
		{
			luaL_error(L, "This spline cannot be destroyed because it is "
				"owned by an actor that relies on it existing.");
		}
		SAFE_DELETE(p);
		return 0;
	}
	LunaCubicSplineN()
	{
		ADD_METHOD(solve);
		ADD_METHOD(evaluate);
		ADD_METHOD(evaluate_derivative);
		ADD_METHOD(evaluate_second_derivative);
		ADD_METHOD(evaluate_third_derivative);
		ADD_METHOD(set_point);
		ADD_METHOD(set_coefficients);
		ADD_METHOD(get_coefficients);
		ADD_METHOD(set_spatial_extent);
		ADD_METHOD(get_spatial_extent);
		ADD_METHOD(get_max_t);
		ADD_METHOD(set_size);
		ADD_METHOD(get_size);
		ADD_METHOD(set_dimension);
		ADD_METHOD(get_dimension);
		ADD_METHOD(empty);
		ADD_METHOD(set_loop);
		ADD_METHOD(get_loop);
		ADD_METHOD(set_polygonal);
		ADD_METHOD(get_polygonal);
		ADD_METHOD(set_dirty);
		ADD_METHOD(get_dirty);
		ADD_METHOD(destroy);
	}
};
LUA_REGISTER_CLASS(CubicSplineN);

int LuaFunc_create_spline(lua_State* L);
int LuaFunc_create_spline(lua_State* L)
{
	CubicSplineN* spline= new CubicSplineN;
	spline->PushSelf(L);
	return 1;
}
LUAFUNC_REGISTER_COMMON(create_spline);

// Side note:  Actually written between 2014/12/26 and 2014/12/28
/*
 * Copyright (c) 2014-2015 Eric Reese
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
