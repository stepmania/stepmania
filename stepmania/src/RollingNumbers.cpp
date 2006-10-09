#include "global.h"
#include "RollingNumbers.h"
#include "RageUtil.h"
#include "XmlFile.h"
#include "ActorUtil.h"
#include "LuaManager.h"
#include "ThemeManager.h"
REGISTER_ACTOR_CLASS( RollingNumbers )

RollingNumbers::RollingNumbers()
{
	m_sFormat = "%9.0f";
	m_fApproachSeconds = 0.2f;

	m_fCurrentNumber = 0;
	m_fTargetNumber = 0;
	m_fScoreVelocity = 0;
}

void RollingNumbers::LoadFromNode( const RString& sDir, const XNode* pNode )
{
	BitmapText::LoadFromNode( sDir, pNode );

	pNode->GetAttrValue( "Format", m_sFormat );
	ThemeManager::EvaluateString( m_sFormat );
	pNode->GetAttrValue( "ApproachSeconds", m_fApproachSeconds );
	
	float fTargetNumber;
	if( pNode->GetAttrValue( "TargetNumber", fTargetNumber ) )
		SetTargetNumber( fTargetNumber );

	UpdateText();
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
	if( fTargetNumber == m_fTargetNumber )	// no change
		return;
	m_fTargetNumber = fTargetNumber;
	m_fScoreVelocity = (m_fTargetNumber-m_fCurrentNumber) / m_fApproachSeconds;
}

void RollingNumbers::UpdateText()
{
	SetText( ssprintf(m_sFormat, m_fCurrentNumber) );
}

// lua start
#include "LuaBinding.h"

class LunaRollingNumbers: public Luna<RollingNumbers>
{
public:
	static int targetnumber( T* p, lua_State *L )	{ p->SetTargetNumber( FArg(1) ); return 0; }

	LunaRollingNumbers()
	{
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
