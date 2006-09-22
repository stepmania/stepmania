/* ActorScroller - ActorFrame that moves its children. */

#ifndef ActorScroller_H
#define ActorScroller_H

#include "ActorFrame.h"
#include "Quad.h"
class XNode;
#include "LuaExpressionTransform.h"

class ActorScroller : public ActorFrame
{
public:
	ActorScroller();

	void SetTransformFromReference( const LuaReference &ref );
	void SetTransformFromExpression( const RString &sTransformFunction );
	void SetTransformFromHeight( float fItemHeight );

	void Load2();

	void EnableMask( float fWidth, float fHeight );
	void DisableMask();

	virtual void UpdateInternal( float fDelta );
	virtual void DrawPrimitives();	// handles drawing and doesn't call ActorFrame::DrawPrimitives

	void PositionItems();

	void LoadFromNode( const RString &sDir, const XNode *pNode );
	virtual Actor *Copy() const;
	
	void SetLoop( bool bLoop )					{ m_bLoop = bLoop; }
	void SetNumItemsToDraw( float fNumItemsToDraw )			{ m_fNumItemsToDraw = fNumItemsToDraw; }
	void SetDestinationItem( float fItemIndex )			{ m_fDestinationItem = fItemIndex; }
	void SetCurrentAndDestinationItem( float fItemIndex )		{ m_fCurrentItem = m_fDestinationItem = fItemIndex; }
	float GetCurrentItem() const					{ return m_fCurrentItem; }
	float GetDestinationItem() const				{ return m_fDestinationItem; }
	void ScrollThroughAllItems();
	void ScrollWithPadding( float fItemPaddingStart, float fItemPaddingEnd );
	void SetPauseCountdownSeconds( float fSecs )			{ m_fPauseCountdownSeconds = fSecs; }
	void SetFastCatchup( bool bOn )					{ m_bFastCatchup = bOn; }
	void SetSecondsPerItem( float fSeconds )			{ m_fSecondsPerItem = fSeconds; }
	void SetSecondsPauseBetweenItems( float fSeconds )		{ m_fSecondsPauseBetweenItems = fSeconds; }
	void SetNumSubdivisions( int iNumSubdivisions )			{ m_exprTransformFunction.SetNumSubdivisions( iNumSubdivisions ); }
	float GetSecondsForCompleteScrollThrough() const;
	float GetSecondsToDestination() const;

	//
	// Commands
	//
	void PushSelf( lua_State *L );

protected:
	void PositionItemsAndDrawPrimitives( bool bDrawPrimitives );
	virtual void ShiftSubActors( int iDist );

	int		m_iNumItems;
	float	m_fCurrentItem; // Item at center of list, usually between 0 and m_SubActors.size(), approaches destination
	float	m_fDestinationItem;
	float	m_fSecondsPerItem;		// <= 0 means don't scroll
	float	m_fSecondsPauseBetweenItems;
	float	m_fNumItemsToDraw;
	int		m_iFirstSubActorIndex;
	bool	m_bLoop; 
	bool	m_bFastCatchup; 
	bool	m_bFunctionDependsOnPositionOffset;
	bool	m_bFunctionDependsOnItemIndex;
	float	m_fPauseCountdownSeconds;
	float	m_fQuantizePixels;

	Quad	m_quadMask;

	LuaExpressionTransform m_exprTransformFunction;	// params: self,offset,itemIndex,numItems
};

class ActorScrollerAutoDeleteChildren : public ActorScroller
{
public:
	ActorScrollerAutoDeleteChildren() { DeleteChildrenWhenDone(true); }
	virtual bool AutoLoadChildren() const { return true; }
	virtual Actor *Copy() const;
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
