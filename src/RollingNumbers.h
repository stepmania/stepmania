#ifndef RollingNumbers_H
#define RollingNumbers_H

#include "BitmapText.h"
#include "ThemeMetric.h"

/** @brief Animates from one number to another by scrolling its digits. */
class RollingNumbers : public BitmapText
{
public:
	RollingNumbers();

	virtual RollingNumbers *Copy() const;

	void Load(const std::string& metrics_group);
	virtual void UpdateInternal(float fDeltaTime);

	/** 
	 * @brief Set the new target number to be reached.
	 * @param fTargetNumber the new target number. */
	void SetTargetNumber( float fTargetNumber );

	void UpdateText();

	// Commands
	virtual void PushSelf( lua_State *L );

	std::string m_text_format;
	float m_approach_seconds;
	bool m_commify;
	std::string m_leading_glyph;
	int m_chars_wide;
	BitmapText::Attribute m_leading_text_attr;
	BitmapText::Attribute m_number_text_attr;
private:

	/** @brief The currently showing number. */
	float	m_current_number;
	/** @brief The number we are trying to approach. */
	float	m_target_number;
	/** @brief The speed we are trying to reach the target number. */
	float	m_score_velocity;
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
