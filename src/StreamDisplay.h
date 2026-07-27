/* StreamDisplay -  */
#ifndef StreamDisplay_H
#define StreamDisplay_H

#include "ActorFrame.h"
#include "Sprite.h"
#include "Quad.h"
#include "LuaExpressionTransform.h"
#include "ThemeMetric.h"

#include <vector>


enum StreamType
{
	StreamType_Normal,
	StreamType_Passing,
	StreamType_Hot,
	NUM_StreamType,
};

class StreamDisplay : public ActorFrame
{
public:
	StreamDisplay();

	virtual void Update( float fDeltaSecs );

	void Load( const RString &sMetricsGroup );

	void SetPercent( float fPercent );
	void SetPassingAlpha( float fPassingAlpha ) { m_fPassingAlpha = fPassingAlpha; }
	void SetHotAlpha( float fHotAlpha ) { m_fHotAlpha = fHotAlpha; }

	float GetPercent() { return m_fPercent; }

private:
	std::vector<Sprite*>	m_vpSprPill[NUM_StreamType];

	LuaExpressionTransform		m_transformPill;	// params: self,offsetFromCenter,itemIndex,numItems
	ThemeMetric<float> VELOCITY_MULTIPLIER;
	ThemeMetric<float> VELOCITY_MIN;
	ThemeMetric<float> VELOCITY_MAX;
	ThemeMetric<float> SPRING_MULTIPLIER;
	ThemeMetric<float> VISCOSITY_MULTIPLIER;

	float m_fPercent;	// percent filled
	float m_fTrailingPercent;	// this approaches m_fPercent, use this value to draw
	float m_fVelocity;	// velocity of m_fTrailingPercent

	float m_fPassingAlpha;
	float m_fHotAlpha;

	bool m_bAlwaysBounce;
};

#endif

/*
 * (c) 2003-2004 Chris Danford
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
