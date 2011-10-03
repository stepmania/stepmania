#ifndef StepsDisplay_H
#define StepsDisplay_H

#include "BitmapText.h"
#include "PlayerNumber.h"
#include "AutoActor.h"
#include "GameConstantsAndTypes.h"
#include "ActorUtil.h"
#include "Difficulty.h"
#include "ActorFrame.h"
#include "ThemeMetric.h"
#include "Sprite.h"

class Steps;
class Trail;
class PlayerState;
/**
 * @brief A graphical representation of a Steps or a Trail.
 *
 * It has a difficulty number, meter, text, and an edit description. */
class StepsDisplay : public ActorFrame
{
public:
	StepsDisplay();

	void Load( const RString &sMetricsGroup, const PlayerState *pPlayerState );

	virtual StepsDisplay *Copy() const;

	void SetFromGameState( PlayerNumber pn );
	void SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType st, int iMeter, Difficulty dc, CourseType ct );
	void SetFromSteps( const Steps* pSteps );
	void SetFromTrail( const Trail* pTrail );
	void Unset();

	// Lua
	void PushSelf( lua_State *L );

private:
	struct SetParams
	{
		const Steps *pSteps;
		const Trail *pTrail;
		int iMeter;
		StepsType st; // pass because there may be a StepType icon
		Difficulty dc;
		CourseType ct;
	};
	void SetInternal( const SetParams &params );

	RString		m_sMetricsGroup;

	AutoActor	m_sprFrame;
	BitmapText	m_textTicks; // 111100000
	BitmapText	m_textMeter; // 3, 9
	/**
	 * @brief The description of the chart.
	 *
	 * This is meant to be separate from the author of the chart. */
	BitmapText	m_textDescription;
	/** @brief The author of the chart. */
	BitmapText	m_textAuthor;
	AutoActor	m_sprAutogen; // visible if Steps and is autogen'd
	AutoActor	m_sprStepsType;

	ThemeMetric<int>	m_iNumTicks;
	ThemeMetric<int>	m_iMaxTicks;
	ThemeMetric<bool>	m_bShowTicks;
	ThemeMetric<bool>	m_bShowMeter;
	ThemeMetric<bool>	m_bShowDescription;
	ThemeMetric<bool>	m_bShowCredit;
	ThemeMetric<bool>	m_bShowAutogen;
	ThemeMetric<bool>	m_bShowStepsType;
	ThemeMetric<RString>	m_sZeroMeterString;
	ThemeMetric<RString>	m_sMeterFormatString;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
