#include "global.h"
#include "TrailUtil.h"
#include "Trail.h"
#include "Course.h"
#include "XmlFile.h"
#include "GameManager.h"
#include "Song.h"
#include <numeric>

int TrailUtil::GetNumSongs( const Trail *pTrail )
{
	return pTrail->m_vEntries.size();
}

float TrailUtil::GetTotalSeconds( const Trail *pTrail )
{
	auto getSeconds = [](float curr, TrailEntry const &e) {
		return curr + e.pSong->m_fMusicLengthSeconds;
	};
	return std::accumulate(pTrail->m_vEntries.begin(), pTrail->m_vEntries.end(), 0.f, getSeconds);
}

///////////////////////////////////////////////////////////////////////

void TrailID::FromTrail( const Trail *p )
{
	if( p == nullptr )
	{
		st = StepsType_Invalid;
		cd = Difficulty_Invalid;
	}
	else
	{
		st = p->m_StepsType;
		cd = p->m_CourseDifficulty;
	}
	m_Cache.Unset();
}

Trail *TrailID::ToTrail( const Course *p, bool bAllowNull ) const
{
	ASSERT( p != nullptr );

	Trail *pRet = nullptr;
	if( !m_Cache.Get(&pRet) )
	{
		if( st != StepsType_Invalid && cd != Difficulty_Invalid )
			pRet = p->GetTrail( st, cd );
		m_Cache.Set( pRet );
	}

	if( !bAllowNull && pRet == nullptr )
		RageException::Throw( "%i, %i, \"%s\"", st, cd, p->GetDisplayFullTitle().c_str() );

	return pRet;
}

XNode* TrailID::CreateNode() const
{
	XNode* pNode = new XNode( "Trail" );

	pNode->AppendAttr( "StepsType", GAMEMAN->GetStepsTypeInfo(st).stepTypeName );
	pNode->AppendAttr( "CourseDifficulty", DifficultyToString(cd) );

	return pNode;
}

void TrailID::LoadFromNode( const XNode* pNode )
{
	ASSERT( pNode->GetName() == "Trail" );

	std::string sTemp;

	pNode->GetAttrValue( "StepsType", sTemp );
	st = GAMEMAN->StringToStepsType( sTemp );

	pNode->GetAttrValue( "CourseDifficulty", sTemp );
	cd = StringToDifficulty( sTemp );
	m_Cache.Unset();
}

std::string TrailID::ToString() const
{
	std::string s = GAMEMAN->GetStepsTypeInfo(st).stepTypeName;
	s += " " + DifficultyToString( cd );
	return s;
}

bool TrailID::IsValid() const
{
	return st != StepsType_Invalid && cd != Difficulty_Invalid;
}

bool TrailID::operator<( const TrailID &rhs ) const
{
#define COMP(a) if(a<rhs.a) return true; if(a>rhs.a) return false;
	COMP(st);
	COMP(cd);
#undef COMP
	return false;
}


// lua start
#include "LuaBinding.h"

namespace
{
	int GetNumSongs( lua_State *L )
	{
		Trail *pTrail = Luna<Trail>::check( L, 1, true );
		int iNum = TrailUtil::GetNumSongs( pTrail );
		LuaHelpers::Push( L, iNum );
		return 1;
	}
	int GetTotalSeconds( lua_State *L )
	{
		Trail *pTrail = Luna<Trail>::check( L, 1, true );
		float fSecs = TrailUtil::GetTotalSeconds( pTrail );
		LuaHelpers::Push( L, fSecs );
		return 1;
	}

	const luaL_Reg TrailUtilTable[] =
	{
		LIST_METHOD( GetNumSongs ),
		LIST_METHOD( GetTotalSeconds ),
		{ nullptr, nullptr }
	};
}

LUA_REGISTER_NAMESPACE( TrailUtil )

/*
 * (c) 2004 Chris Danford
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
