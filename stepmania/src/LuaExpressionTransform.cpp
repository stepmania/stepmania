#include "global.h"
#include "LuaExpressionTransform.h"
#include "LuaReference.h"
#include "LuaManager.h"

LuaExpressionTransform::LuaExpressionTransform()
{
	m_pexprTransformFunction = new LuaExpression;
	m_iNumSubdivisions = 1;
}

void LuaExpressionTransform::SetFromExpression( const CString &sExpression, int iNumSubdivisions )
{
	m_pexprTransformFunction->SetFromExpression( sExpression );
	m_iNumSubdivisions = iNumSubdivisions;
}

const Actor::TweenState &LuaExpressionTransform::GetPosition( float fPositionOffsetFromCenter, int iItemIndex, int iNumItems ) const
{
	PositionOffsetAndItemIndex key = { fPositionOffsetFromCenter, iItemIndex };

	map<PositionOffsetAndItemIndex,Actor::TweenState>::const_iterator iter = m_mapPositionToTweenStateCache.find( key );
	if( iter != m_mapPositionToTweenStateCache.end() )
		return iter->second;

	Actor a;

	Lua *L = LUA->Get();
	m_pexprTransformFunction->PushSelf( L );
	ASSERT( !lua_isnil(L, -1) );
	a.PushSelf( L );
	LuaHelpers::Push( fPositionOffsetFromCenter, L );
	LuaHelpers::Push( iItemIndex, L );
	LuaHelpers::Push( iNumItems, L );
	lua_call( L, 4, 0 ); // 4 args, 0 results
	LUA->Release(L);

	return m_mapPositionToTweenStateCache[key] = a.DestTweenState();
}

void LuaExpressionTransform::PositionItem( Actor *pActor, float fPositionOffsetFromCenter, int iItemIndex, int iNumItems )
{
	float fInterval = 1.0f / m_iNumSubdivisions;
	float fFloor = QuantizeDown( fPositionOffsetFromCenter, fInterval );
	float fCeil = QuantizeUp( fPositionOffsetFromCenter, fInterval );

	if( fFloor == fCeil )
	{
		pActor->DestTweenState() = GetPosition( fCeil, iItemIndex, iNumItems );
	}
	else
	{
		const Actor::TweenState &tsFloor = GetPosition( fFloor, iItemIndex, iNumItems );
		const Actor::TweenState &tsCeil = GetPosition( fCeil, iItemIndex, iNumItems );

		float fPercentTowardCeil = SCALE( fPositionOffsetFromCenter, fFloor, fCeil, 0.0f, 1.0f );
		Actor::TweenState::MakeWeightedAverage( pActor->DestTweenState(), tsFloor, tsCeil, fPercentTowardCeil );
	}
}


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
