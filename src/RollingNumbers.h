#ifndef RollingNumbers_H
#define RollingNumbers_H

#include "BitmapText.h"
#include "ThemeMetric.h"

/** @brief Animates from one number to another by scrolling its digits. */
class RollingNumbers : public BitmapText
{
public:
	RollingNumbers();

	void Load( const RString &sMetricsGroup );
	virtual RollingNumbers *Copy() const;

	void DrawPart(RageColor const* diffuse, RageColor const& stroke,
		float crop_left, float crop_right);
	virtual void DrawPrimitives();
	virtual void Update( float fDeltaTime );

	/** 
	 * @brief Set the new target number to be reached.
	 * @param fTargetNumber the new target number. */
	void SetTargetNumber( float fTargetNumber );

	void UpdateText();

	// Commands
	virtual void PushSelf( lua_State *L );

private:
	ThemeMetric<RString> TEXT_FORMAT;
	ThemeMetric<float> APPROACH_SECONDS;
	ThemeMetric<bool> COMMIFY;
	ThemeMetric<RageColor> LEADING_ZERO_MULTIPLY_COLOR;

	/** @brief The currently showing number. */
	float	m_fCurrentNumber;
	/** @brief The number we are trying to approach. */
	float	m_fTargetNumber;
	/** @brief The speed we are trying to reach the target number. */
	float	m_fScoreVelocity;
	bool m_metrics_loaded;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
