#include "global.h"
#include "ActorScroller.h"
#include "ActorCollision.h"
#include "RageUtil.h"
#include "RageDisplay.h"
#include "IniFile.h"
#include "arch/Dialog/Dialog.h"
#include "RageLog.h"
#include "ActorUtil.h"
#include <sstream>

/* Tricky: We need ActorFrames created in XML to auto delete their children.
 * We don't want classes that derive from ActorFrame to auto delete their 
 * children.  The name "ActorFrame" is widely used in XML, so we'll have
 * that string instead create an ActorFrameAutoDeleteChildren object.
 */
//REGISTER_ACTOR_CLASS( ActorScroller )
REGISTER_ACTOR_CLASS_WITH_NAME( ActorScrollerAutoDeleteChildren, ActorScroller )
Actor *ActorScroller::Copy() const { return new ActorScroller(*this); }


ActorScroller::ActorScroller()
{
	m_bLoaded = false;
	m_fCurrentItem = 0;
	m_fDestinationItem = 0;
	m_fSecondsPerItem = 1;
	m_fNumItemsToDraw = 7;
	m_fSecondsPauseBetweenItems = 0;
	m_fNumItemsToDraw = 7;
	m_bLoop = false;
	m_bFastCatchup = false;
	m_fPauseCountdownSeconds = 0;
	m_fQuantizePixels = 0;

	m_bUseMask = false;
	m_fMaskWidth = 1;
	m_fMaskHeight = 1;
	m_quadMask.SetBlendMode( BLEND_NO_EFFECT );	// don't change color values
	m_quadMask.SetUseZBuffer( true );	// we want to write to the Zbuffer
	m_quadMask.SetHidden( true );
}

void ActorScroller::Load2(
	float fNumItemsToDraw, 
	float fItemWidth, 
	float fItemHeight, 
	bool bLoop, 
	float fSecondsPerItem, 
	float fSecondsPauseBetweenItems )
{
	CLAMP( fNumItemsToDraw, 1, 10000 );
	CLAMP( fItemWidth, 1, 10000 );
	CLAMP( fItemHeight, 1, 10000 );
	CLAMP( fSecondsPerItem, 0.01f, 10000 );
	CLAMP( fSecondsPauseBetweenItems, 0, 10000 );

	m_fNumItemsToDraw = fNumItemsToDraw;
	m_fMaskWidth = fItemWidth;
	m_fMaskHeight = fItemHeight;

	m_exprTransformFunction.SetFromExpression( 
		ssprintf("function(self,offset,itemIndex,numItems) return self:y(%f*offset) end",fItemHeight),
		1
		);

	m_bLoop = bLoop; 
	m_fSecondsPerItem = fSecondsPerItem; 
	m_fSecondsPauseBetweenItems = fSecondsPauseBetweenItems;
	m_fPauseCountdownSeconds = 0;
	m_fQuantizePixels = 0;

	ScrollThroughAllItems();

	m_bUseMask = true;
	RectF rectBarSize(
		-m_fMaskWidth/2,
		-m_fMaskHeight/2,
		m_fMaskWidth/2,
		m_fMaskHeight/2 );
	m_quadMask.StretchTo( rectBarSize );
	m_quadMask.SetZ( 1 );

	m_quadMask.SetHidden( false );

	m_bLoaded = true;
}

void ActorScroller::Load3(
	float fSecondsPerItem, 
	float fNumItemsToDraw, 
	bool bFastCatchup,
	const CString &sTransformFunction,
	int iSubdivisions,
	bool bUseMask,
	bool bLoop
	)
{
	m_fSecondsPerItem = fSecondsPerItem;
	m_fNumItemsToDraw = fNumItemsToDraw;
	m_bFastCatchup = bFastCatchup;
	m_exprTransformFunction.SetFromExpression( sTransformFunction, iSubdivisions );
	m_fQuantizePixels = 0;
	m_bUseMask = bUseMask;
	m_bLoop = bLoop;
	m_bLoaded = true;
}

void ActorScroller::ScrollThroughAllItems()
{
	m_fCurrentItem = m_bLoop ? +m_fNumItemsToDraw/2 : -(m_fNumItemsToDraw/2)-1;
	m_fDestinationItem = (float)(m_SubActors.size()+m_fNumItemsToDraw/2+1);
}

float ActorScroller::GetSecondsForCompleteScrollThrough() const
{
	float fTotalItems = m_fNumItemsToDraw + m_SubActors.size();
	return fTotalItems * (m_fSecondsPerItem + m_fSecondsPauseBetweenItems );
}

void ActorScroller::LoadFromNode( const CString &sDir, const XNode *pNode )
{
	ActorFrame::LoadFromNode( sDir, pNode );

	bool bUseScroller = false;
	pNode->GetAttrValue( "UseScroller", bUseScroller );
	if( !bUseScroller )
		return;

#define GET_VALUE( szName, valueOut ) \
	if( !pNode->GetAttrValue( szName, valueOut ) ) { \
		CString sError = ssprintf("Animation in '%s' is missing the value Scroller::%s", sDir.c_str(), szName); \
		LOG->Warn( sError ); \
		Dialog::OK( sError ); \
	}

	float fSecondsPerItem = 1;
	float fNumItemsToDraw = 0;
	float fItemPaddingStart = 0;
	float fItemPaddingEnd = 0;
	CString sTransformFunction;
	int iSubdivisions = 0;

	GET_VALUE( "SecondsPerItem", fSecondsPerItem );
	GET_VALUE( "NumItemsToDraw", fNumItemsToDraw );
	GET_VALUE( "ItemPaddingStart", fItemPaddingStart );
	GET_VALUE( "ItemPaddingEnd", fItemPaddingEnd );
	GET_VALUE( "TransformFunction", sTransformFunction );
	pNode->GetAttrValue( "Subdivisions", iSubdivisions );
#undef GET_VALUE

	Load3( 
		fSecondsPerItem,
		fNumItemsToDraw,
		false,
		sTransformFunction,
		iSubdivisions,
		false,
		false );
	SetCurrentAndDestinationItem( -fItemPaddingStart );
	SetDestinationItem( m_SubActors.size()-1+fItemPaddingEnd );

	pNode->GetAttrValue( "UseMask", m_bUseMask );
	pNode->GetAttrValue( "QuantizePixels", m_fQuantizePixels );
}

void ActorScroller::UpdateInternal( float fDeltaTime )
{
	ActorFrame::UpdateInternal( fDeltaTime );

	/* If we have no children, the code below will busy loop. */
	if( !m_SubActors.size() )
		return;

	// handle pause
	if( fDeltaTime > m_fPauseCountdownSeconds )
	{
		fDeltaTime -= m_fPauseCountdownSeconds;
		m_fPauseCountdownSeconds = 0;
	}
	else
	{
		m_fPauseCountdownSeconds -= fDeltaTime;
		fDeltaTime = 0;
		return;
	}


	if( m_fCurrentItem == m_fDestinationItem )
		return;	// done scrolling


	float fOldItemAtTop = m_fCurrentItem;
	if( m_fSecondsPerItem > 0 )
	{
		float fApproachSpeed = fDeltaTime/m_fSecondsPerItem;
		if( m_bFastCatchup )
		{
			float fDistanceToMove = fabsf(m_fCurrentItem - m_fDestinationItem);
			if( fDistanceToMove > 1 )
				fApproachSpeed *= fDistanceToMove*fDistanceToMove;
		}

		fapproach( m_fCurrentItem, m_fDestinationItem, fApproachSpeed );
	}

	// if items changed, then pause
	if( (int)fOldItemAtTop != (int)m_fCurrentItem )
		m_fPauseCountdownSeconds = m_fSecondsPauseBetweenItems;

	if( m_bLoop )
		m_fCurrentItem = fmodf( m_fCurrentItem, m_fNumItemsToDraw+1 );
}

void ActorScroller::DrawPrimitives()
{
	PositionItemsAndDrawPrimitives( true, true );
}

void ActorScroller::PositionItems()
{
	PositionItemsAndDrawPrimitives( true, false );
}

void ActorScroller::PositionItemsAndDrawPrimitives( bool bPosition, bool bDrawPrimitives )
{
	// Optimization:  If we weren't loaded, then fall back to the ActorFrame logic
	if( !m_bLoaded )
	{
		ActorFrame::DrawPrimitives();
		return;
	}

	if( m_SubActors.empty() )
		return;

	float fFirstItemToDraw = 0;
	float fLastItemToDraw = 0;

	if( m_bUseMask )
	{
		// write to z buffer so that top and bottom are clipped
		float fPositionFullyOnScreenTop = -(m_fNumItemsToDraw-1)/2.f;
		float fPositionFullyOnScreenBottom = (m_fNumItemsToDraw-1)/2.f;
		float fPositionFullyOffScreenTop = fPositionFullyOnScreenTop - 1;
		float fPositionFullyOffScreenBottom = fPositionFullyOnScreenBottom + 1;

		if( bPosition )			m_exprTransformFunction.PositionItem( &m_quadMask, fPositionFullyOffScreenTop, -1, m_SubActors.size() );
		if( bDrawPrimitives )	m_quadMask.Draw();

		if( bPosition )			m_exprTransformFunction.PositionItem( &m_quadMask, fPositionFullyOffScreenBottom, m_SubActors.size(), m_SubActors.size() );
		if( bDrawPrimitives )	m_quadMask.Draw();

		fFirstItemToDraw = fPositionFullyOffScreenTop + m_fCurrentItem;
		fLastItemToDraw = fPositionFullyOffScreenBottom + m_fCurrentItem;
	}
	else
	{
		float fPositionOnEdgeOfScreenTop = -(m_fNumItemsToDraw)/2.f;
		float fPositionOnEdgeOfScreenBottom = (m_fNumItemsToDraw)/2.f;
		
		fFirstItemToDraw = fPositionOnEdgeOfScreenTop + m_fCurrentItem;
		fLastItemToDraw = fPositionOnEdgeOfScreenBottom + m_fCurrentItem;
	}

	bool bDelayedDraw = m_bDrawByZPosition && !m_bLoop;
	vector<Actor*> subs;

	for( int iItem=(int)truncf(ceilf(fFirstItemToDraw)); iItem<=fLastItemToDraw; iItem++ )
	{
		float fPosition = iItem - m_fCurrentItem;
		int iIndex = iItem;
		if( m_bLoop )
			wrap( iIndex, m_SubActors.size() );
		else if( iIndex < 0 || iIndex >= (int)m_SubActors.size() )
			continue;

		if( bPosition )		
			m_exprTransformFunction.PositionItem( m_SubActors[iIndex], fPosition, iIndex, m_SubActors.size() );
		if( bDrawPrimitives )
		{
			if( bDelayedDraw )
				subs.push_back( m_SubActors[iIndex] );
			else
				m_SubActors[iIndex]->Draw();
		}
	}

	if( bDelayedDraw )
	{
		ActorUtil::SortByZPosition( subs );
		FOREACH( Actor*, subs, a )
			(*a)->Draw();
	}
}

// lua start
#include "LuaBinding.h"

class LunaActorScroller: public Luna<ActorScroller>
{
public:
	LunaActorScroller() { LUA->Register( Register ); }

	static int SetCurrentAndDestinationItem( T* p, lua_State *L )	{ p->SetCurrentAndDestinationItem( FArg(1) ); return 0; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( SetCurrentAndDestinationItem );
		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( ActorScroller, ActorFrame )
// lua end

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
