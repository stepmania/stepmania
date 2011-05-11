#include "global.h"
#include "RollingNumbers.h"
#include "RageUtil.h"
#include "GameState.h"
#include "XmlFile.h"
#include "ActorUtil.h"
#include "LuaManager.h"
#include "ThemeManager.h"
REGISTER_ACTOR_CLASS( RollingNumbers );

RollingNumbers::RollingNumbers()
{
	m_fCurrentNumber = 0;
	m_fTargetNumber = 0;
	m_fScoreVelocity = 0;
}

void RollingNumbers::Load( const RString &sMetricsGroup )
{
	TEXT_FORMAT.Load(sMetricsGroup, "TextFormat");
	APPROACH_SECONDS.Load(sMetricsGroup, "ApproachSeconds");
	COMMIFY.Load(sMetricsGroup, "Commify");
	LEADING_ZERO_MULTIPLY_COLOR.Load(sMetricsGroup, "LeadingZeroMultiplyColor");

	UpdateText();
}

void RollingNumbers::DrawPrimitives()
{
	RageColor c_orig = this->GetDiffuse();
	RageColor c2_orig = this->GetStrokeColor();

	RageColor c = this->GetDiffuse();
	c *= LEADING_ZERO_MULTIPLY_COLOR;
	RageColor c2 = this->GetStrokeColor();
	c2 *= LEADING_ZERO_MULTIPLY_COLOR;

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
	SetDiffuse( c );
	SetStrokeColor( c2 );
	SetCropLeft( 0 );
	SetCropRight( 1-f );
	BitmapText::DrawPrimitives();

	// draw regular color part
	SetDiffuse( c_orig );
	SetStrokeColor( c2_orig );
	SetCropLeft( f );
	SetCropRight( 0 );
	BitmapText::DrawPrimitives();

	SetCropLeft( 0 );
	SetCropRight( 0 );
}

void RollingNumbers::Update( float fDeltaTime )
{
	if( m_fCurrentNumber != m_fTargetNumber )
	{
		fapproach( m_fCurrentNumber, m_fTargetNumber, fabsf(m_fScoreVelocity) * fDeltaTime );
		UpdateText();
	}

	BitmapText::Update( fDeltaTime );
}

void RollingNumbers::SetTargetNumber( float fTargetNumber )
{
	if( fTargetNumber == m_fTargetNumber ) // no change
		return;
	m_fTargetNumber = fTargetNumber;
	m_fScoreVelocity = (m_fTargetNumber-m_fCurrentNumber) / APPROACH_SECONDS.GetValue();
}

void RollingNumbers::UpdateText()
{
	RString s = ssprintf( TEXT_FORMAT.GetValue(), m_fCurrentNumber );
	if( COMMIFY )
		s = Commify( s );
	SetText( s );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the RollingNumbers. */ 
class LunaRollingNumbers: public Luna<RollingNumbers>
{
public:
	static int Load( T* p, lua_State *L )			{ p->Load(SArg(1)); return 0; }
	static int targetnumber( T* p, lua_State *L )	{ p->SetTargetNumber( FArg(1) ); return 0; }

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
