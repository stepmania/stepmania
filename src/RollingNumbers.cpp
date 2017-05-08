#include "global.h"
#include "RollingNumbers.h"
#include "RageUtil.h"
#include "GameState.h"
#include "XmlFile.h"
#include "ActorUtil.h"
#include "LuaManager.h"
#include "ThemeManager.h"
REGISTER_ACTOR_CLASS(RollingNumbers);

RollingNumbers::RollingNumbers()
	:m_text_format("%.0f"), m_approach_seconds(1.0f), m_commify(false),
	 m_leading_glyph("0"), m_chars_wide(9),
	 m_current_number(0.0f), m_target_number(0.0f), m_score_velocity(0.0f)
{
	m_leading_text_attr.set_diffuse(Rage::Color(1, 1, 1, 1));
	m_number_text_attr.set_diffuse(Rage::Color(1, 1, 1, 1));
}

void RollingNumbers::Load(const std::string& metrics_group)
{
	// Deprecated interface, not going to bother with error handling. -Kyz
#define HAS_GET(metric, member, get) \
	if(THEME->HasMetric(metrics_group, metric)) \
	{ \
		member= THEME->get(metrics_group, metric); \
	}
	HAS_GET("Width", m_chars_wide, GetMetricI);
	HAS_GET("LeadGlyph", m_leading_glyph, GetMetric);
	HAS_GET("TextFormat", m_text_format, GetMetric);
	HAS_GET("ApproachSeconds", m_approach_seconds, GetMetricF);
	HAS_GET("Commify", m_commify, GetMetricB);
	Rage::Color lead_color;
	HAS_GET("LeadingColor", lead_color, GetMetricC);
	m_leading_text_attr.set_diffuse(lead_color);
	Rage::Color number_color;
	HAS_GET("NumberColor", number_color, GetMetricC);
	m_number_text_attr.set_diffuse(number_color);
#undef HAS_GET
	UpdateText();
}

void RollingNumbers::UpdateInternal(float fDeltaTime)
{
	if(m_current_number != m_target_number || m_bFirstUpdate)
	{
		fapproach(m_current_number, m_target_number, fabsf(m_score_velocity) * fDeltaTime);
		UpdateText();
	}

	BitmapText::UpdateInternal(fDeltaTime);
}

void RollingNumbers::SetTargetNumber(float target_number)
{
	m_target_number = target_number;
	if(m_approach_seconds > 0)
	{
		m_score_velocity= (m_target_number-m_current_number) / m_approach_seconds;
	}
	else
	{
		m_score_velocity= (m_target_number-m_current_number);
	}
}

void RollingNumbers::UpdateText()
{
	std::string s = fmt::sprintf(m_text_format, m_current_number);
	size_t number_text_width= s.size();
	size_t chars_wide= static_cast<size_t>(m_chars_wide);
	if(m_chars_wide <= 0)
	{
		chars_wide= 9;
	}
	if(s.size() < chars_wide && !m_leading_glyph.empty())
	{
		std::vector<std::string> to_join;
		for(size_t i= 0; i < chars_wide - s.size(); ++i)
		{
			to_join.push_back(m_leading_glyph);
		}
		to_join.push_back(s);
		s = Rage::join("", to_join);
	}
	if(m_commify)
	{
		s = Commify(s);
		number_text_width+= (number_text_width - 1) / 3;
	}
	int pad_width= s.size() - number_text_width;
	SetText(s);
	ClearAttributes();
	m_leading_text_attr.length= std::max(0, pad_width);
	if(m_leading_glyph.empty())
	{
		m_leading_text_attr.length= 0;
	}
	m_number_text_attr.length= s.size();
	AddAttribute(0, m_leading_text_attr);
	AddAttribute(m_leading_text_attr.length, m_number_text_attr);
}

// lua start
#include "LuaBinding.h"

struct LunaRollingNumbers : Luna<RollingNumbers>
{
	GET_SET_MEMBER(approach_seconds, FArg);
	GET_SET_MEMBER(chars_wide, IArg);
	GET_SET_MEMBER(commify, BArg);
	GET_SET_MEMBER(leading_glyph, SArg);
	GET_SET_MEMBER(text_format, SArg);
	static int set_leading_attribute(T* p, lua_State* L)
	{
		p->m_leading_text_attr.FromStack(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_number_attribute(T* p, lua_State* L)
	{
		p->m_number_text_attr.FromStack(L, 1);
		COMMON_RETURN_SELF;
	}
	static int target_number(T* p, lua_State *L)
	{
		float target= FArg(1);
		if(!std::isfinite(target))
		{
			luaL_error(L, "RollingNumbers: Invalid non-finite target number.");
		}
		p->SetTargetNumber(target);
		COMMON_RETURN_SELF;
	}

	LunaRollingNumbers()
	{
		ADD_GET_SET_METHODS(approach_seconds);
		ADD_GET_SET_METHODS(chars_wide);
		ADD_GET_SET_METHODS(commify);
		ADD_GET_SET_METHODS(leading_glyph);
		ADD_GET_SET_METHODS(text_format);
		ADD_METHOD(set_leading_attribute);
		ADD_METHOD(set_number_attribute);
		ADD_METHOD(target_number);
	}
};

LUA_REGISTER_DERIVED_CLASS(RollingNumbers, BitmapText)

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
