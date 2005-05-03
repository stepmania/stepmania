/* ActorScroller - ActorFrame that moves its children. */

#ifndef ActorScroller_H
#define ActorScroller_H

#include "ActorFrame.h"
#include "Quad.h"
struct XNode;

template<class T>
class LunaActorScroller : public LunaActorFrame<T>
{
public:
	LunaActorScroller() { LUA->Register( Register ); }

	static int SetCurrentAndDestinationItem( T* p, lua_State *L )	{ p->SetCurrentAndDestinationItem( FArg(1) ); return 0; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( SetCurrentAndDestinationItem )
		LunaActor<T>::Register( L );
	}
};

class ActorScroller : public ActorFrame
{
public:
	ActorScroller();

	void Load( 
		float fScrollSecondsPerItem, 
		float fNumItemsToDraw, 
		const RageVector3	&vRotationDegrees,
		const RageVector3	&vTranslateTerm0,
		const RageVector3	&vTranslateTerm1,
		const RageVector3	&vTranslateTerm2,
		bool bUseMask = false );

	void Load2(
		float fNumItemsToDraw, 
		float fItemWidth, 
		float fItemHeight, 
		bool bLoop, 
		float fSecondsPerItem, 
		float fSecondsPauseBetweenItems );

	void Load3(
		float fSecondsPerItem, 
		float fNumItemsToDraw, 
		bool bFastCatchup,
		const CString &sExprTransform );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();	// DOES draw

	void LoadFromNode( const CString &sDir, const XNode *pNode );
	
	void SetDestinationItem( float fItemIndex )				{ m_fDestinationItem = fItemIndex; }
	void SetCurrentAndDestinationItem( float fItemIndex )	{ m_fCurrentItem = m_fDestinationItem = fItemIndex; }
	float GetCurrentItem()									{ return m_fCurrentItem; }
	float GetDestinationItem()								{ return m_fDestinationItem; }
	void SetPauseCountdownSeconds( float fSecs )			{ m_fPauseCountdownSeconds = fSecs; }

	float	GetSecondsForCompleteScrollThrough();

	//
	// Commands
	//
	void PushSelf( lua_State *L );

protected:
	void PositionItem( Actor *pActor, float fPositionOffsetFromCenter, int iItemIndex, int iNumItems );

	bool	m_bLoaded;
	float	m_fCurrentItem; // Item at top of list, usually between 0 and m_SubActors.size(), approaches destination
	float	m_fDestinationItem;
	float	m_fSecondsPerItem;		// <= 0 means don't scroll
	float	m_fSecondsPauseBetweenItems;
	float	m_fNumItemsToDraw;
	bool	m_bLoop; 
	bool	m_bFastCatchup; 
	float	m_fPauseCountdownSeconds;
	float	m_fQuantizePixels;

	bool	m_bUseMask;
	float	m_fMaskWidth;
	float	m_fMaskHeight;
	Quad	m_quadMask;

	LuaExpression m_exprTransform;	// params: self,offset,itemIndex,numItems
};

class ActorScrollerAutoDeleteChildren : public ActorScroller
{
public:
	ActorScrollerAutoDeleteChildren() { DeleteChildrenWhenDone(true); }
	void LoadFromNode( const CString& sDir, const XNode* pNode )
	{
		// Load children before ActorScroller::LoadFromNode or
		// else it won't SetDestinationItem correctly.
		LoadChildrenFromNode( sDir, pNode );

		ActorScroller::LoadFromNode( sDir, pNode );
	}
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
