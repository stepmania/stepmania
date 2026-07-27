#include "global.h"
#include "RollingNumbers.h"
#include "RageUtil.h"
#include "GameState.h"
#include "XmlFile.h"
#include "ActorUtil.h"
#include "LuaManager.h"
#include "ThemeManager.h"

#include <cmath>

REGISTER_ACTOR_CLASS( RollingNumbers );

RollingNumbers::RollingNumbers()
{
	m_fCurrentNumber = 0;
	m_fTargetNumber = 0;
	m_fScoreVelocity = 0;
	m_metrics_loaded= false;
}

void RollingNumbers::Load( const RString &sMetricsGroup )
{
	m_metrics_loaded= true;
	TEXT_FORMAT.Load(sMetricsGroup, "TextFormat");
	APPROACH_SECONDS.Load(sMetricsGroup, "ApproachSeconds");
	COMMIFY.Load(sMetricsGroup, "Commify");
	LEADING_ZERO_MULTIPLY_COLOR.Load(sMetricsGroup, "LeadingZeroMultiplyColor");

	UpdateText();
}

void RollingNumbers::DrawPart(RageColor const* diffuse, RageColor const& stroke,
	float crop_left, float crop_right)
{
	for(int i= 0; i < NUM_DIFFUSE_COLORS; ++i)
	{
		m_pTempState->diffuse[i]= diffuse[i];
	}
	SetCurrStrokeColor(stroke);
	m_pTempState->crop.left= crop_left;
	m_pTempState->crop.right= crop_right;
	BitmapText::DrawPrimitives();
}

void RollingNumbers::DrawPrimitives()
{
	if(!m_metrics_loaded)
	{
		return;
	}
	RageColor diffuse_orig[NUM_DIFFUSE_COLORS];
	RageColor diffuse_temp[NUM_DIFFUSE_COLORS];
	RageColor stroke_orig= GetCurrStrokeColor();
	RageColor stroke_temp= stroke_orig * LEADING_ZERO_MULTIPLY_COLOR;
	for(int i= 0; i < NUM_DIFFUSE_COLORS; ++i)
	{
		diffuse_orig[i]= m_pTempState->diffuse[i];
		diffuse_temp[i]= m_pTempState->diffuse[i] * LEADING_ZERO_MULTIPLY_COLOR;
	}
	float original_crop_left= m_pTempState->crop.left;
	float original_crop_right= m_pTempState->crop.right;

	RString s = this->GetText();
	int i;
	// find the first non-zero non-comma character, or the last character
	for( i=0; i<(int)(s.length()-1); i++ )
	{
		if( s[i] != '0'  &&  s[i] != ',' )
			break;
	}

	// Rewind to the first number, even if it's a zero.  If the string is "0000", we want the last zero to show in the regular color.
	for( ; i>=0; i-- )
	{
		if( s[i] >= '0'  &&  s[i] <= '9' )
			break;
	}
	float f = i / (float)s.length();

	// draw leading part
	DrawPart(diffuse_temp, stroke_temp,
		std::max((float) 0, original_crop_left), std::max(1-f, original_crop_right));
	// draw regular color part
	DrawPart(diffuse_orig, stroke_orig,
		std::max(f, original_crop_left), std::max(0.0f, original_crop_right));

	m_pTempState->crop.left= original_crop_left;
	m_pTempState->crop.right= original_crop_right;
}

void RollingNumbers::Update( float fDeltaTime )
{
	if(m_fCurrentNumber != m_fTargetNumber)
	{
		fapproach( m_fCurrentNumber, m_fTargetNumber, std::abs(m_fScoreVelocity) * fDeltaTime );
		UpdateText();
	}

	BitmapText::Update( fDeltaTime );
}

void RollingNumbers::SetTargetNumber( float fTargetNumber )
{
	if(!m_metrics_loaded)
	{
		LuaHelpers::ReportScriptError("You must use Load to load the metrics for a RollingNumbers actor before doing anything else.");
		return;
	}
	if( fTargetNumber == m_fTargetNumber ) // no change
		return;
	m_fTargetNumber = fTargetNumber;
	float approach_secs= APPROACH_SECONDS.GetValue();
	if(approach_secs > 0)
	{
		m_fScoreVelocity= (m_fTargetNumber-m_fCurrentNumber) / approach_secs;
	}
	else
	{
		m_fScoreVelocity= (m_fTargetNumber-m_fCurrentNumber);
	}
}

void RollingNumbers::UpdateText()
{
	if(!m_metrics_loaded)
	{
		return;
	}
	RString s = ssprintf( TEXT_FORMAT.GetValue(), m_fCurrentNumber );
	if(COMMIFY)
	{
		s = Commify( s );
	}
	SetText( s );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the RollingNumbers. */
class LunaRollingNumbers: public Luna<RollingNumbers>
{
public:
	static int Load( T* p, lua_State *L )			{ p->Load(SArg(1)); COMMON_RETURN_SELF; }
	static int targetnumber( T* p, lua_State *L )	{ p->SetTargetNumber( FArg(1) ); COMMON_RETURN_SELF; }

	LunaRollingNumbers()
	{
		ADD_METHOD( Load );
		ADD_METHOD( targetnumber );
	}
};

LUA_REGISTER_DERIVED_CLASS( RollingNumbers, BitmapText )

// lua end

/*
 * (c) 2001-2004 Chris Danford
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
