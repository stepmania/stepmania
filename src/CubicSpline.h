#ifndef CUBIC_SPLINE_H
#define CUBIC_SPLINE_H

#include <vector>
using std::vector;
struct lua_State;

struct CubicSpline
{
	void solve_looped();
	void solve_straight();
	float evaluate(float t, bool loop) const;
	void set_point(size_t i, float v);
	void resize(size_t s);
	size_t size() const;
	bool empty() const;
private:
	bool check_minimum_size();
	void prep_inner(size_t last, vector<float>& results);
	void set_results(size_t last, vector<float>& diagonals, vector<float>& results);

	struct SplinePoint
	{
		float a, b, c, d;
	};
	vector<SplinePoint> m_points;
};

struct CubicSplineN
{
	CubicSplineN()
		:m_owned_by_actor(false), m_dirty(true)
	{}
	void solve();
	void evaluate(float t, vector<float>& v) const;
	void set_point(size_t i, vector<float> const& v);
	void resize(size_t s);
	size_t size() const;
	void redimension(size_t d);
	size_t dimension() const;
	bool empty() const;
	typedef vector<CubicSpline> spline_cont_t;
	bool loop;
	bool m_owned_by_actor;

	void PushSelf(lua_State* L);
private:
	bool m_dirty;
	spline_cont_t m_splines;
};

#endif

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
