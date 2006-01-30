/* LuaExpressionTransform - . */

#ifndef LuaExpressionTransform_H
#define LuaExpressionTransform_H

#include "Actor.h"
class LuaExpression;
#include <map>

class LuaExpressionTransform
{
public:
	LuaExpressionTransform();

	void SetFromExpression( const RString &sExpression );
	void SetNumSubdivisions( int iNumSubdivisions ) { ASSERT( iNumSubdivisions > 0 ); m_iNumSubdivisions = iNumSubdivisions; }

	void PositionItem( Actor *pActor, float fPositionOffsetFromCenter, int iItemIndex, int iNumItems );
	const Actor::TweenState &GetPosition( float fPositionOffsetFromCenter, int iItemIndex, int iNumItems ) const;
	void ClearCache() { m_mapPositionToTweenStateCache.clear(); }

protected:

	LuaExpression *m_pexprTransformFunction;	// params: self,offset,itemIndex,numItems
	int m_iNumSubdivisions;	// 1 == one evaluation per position
	struct PositionOffsetAndItemIndex
	{
		float fPositionOffsetFromCenter;
		int iItemIndex;

		bool operator<( const PositionOffsetAndItemIndex &other ) const
		{
			if( fPositionOffsetFromCenter != other.fPositionOffsetFromCenter )
				return fPositionOffsetFromCenter < other.fPositionOffsetFromCenter;
			return iItemIndex < other.iItemIndex;
		}
	};
	mutable map<PositionOffsetAndItemIndex,Actor::TweenState> m_mapPositionToTweenStateCache;
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
