#include "global.h"
#include "ActorScroller.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "XmlFile.h"
#include "arch/Dialog/Dialog.h"
#include "RageLog.h"
#include "ActorUtil.h"

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
	m_iNumItems = 0;
	m_fCurrentItem = 0;
	m_fDestinationItem = 0;
	m_fSecondsPerItem = 0;
	m_fSecondsPauseBetweenItems = 0;
	m_fNumItemsToDraw = 7;
	m_iFirstSubActorIndex = 0;
	m_bLoop = false;
	m_bFastCatchup = false;
	m_bUseItemNumber = false;
	m_fPauseCountdownSeconds = 0;
	m_fQuantizePixels = 0;

	m_quadMask.SetBlendMode( BLEND_NO_EFFECT );	// don't change color values
	m_quadMask.SetUseZBuffer( true );	// we want to write to the Zbuffer
	DisableMask();
}

void ActorScroller::Load2(
	float fNumItemsToDraw, 
	float fItemHeight, 
	bool bLoop )
{
	Load3( fNumItemsToDraw,
		ssprintf("function(self,offset,itemIndex,numItems) self:y(%f*offset) end",fItemHeight), bLoop );
}

void ActorScroller::Load3(
	float fNumItemsToDraw, 
	const CString &sTransformFunction,
	bool bLoop
	)
{
	m_fNumItemsToDraw = fNumItemsToDraw;
	m_exprTransformFunction.SetFromExpression( sTransformFunction );
	m_bLoop = bLoop;
	m_iNumItems = m_SubActors.size();
}

void ActorScroller::EnableMask( float fWidth, float fHeight )
{
	m_quadMask.SetHidden( false );
	m_quadMask.SetWidth( fWidth );
	m_quadMask.SetHeight( fHeight );
}

void ActorScroller::DisableMask()
{
	m_quadMask.SetHidden( true );
}

void ActorScroller::ScrollThroughAllItems()
{
	m_fCurrentItem = m_bLoop ? +m_fNumItemsToDraw/2.0f : -(m_fNumItemsToDraw/2.0f)-1;
	m_fDestinationItem = (float)(m_iNumItems+m_fNumItemsToDraw/2.0f+1);
}

void ActorScroller::ScrollWithPadding( float fItemPaddingStart, float fItemPaddingEnd )
{
	m_fCurrentItem = -fItemPaddingStart;
	m_fDestinationItem = m_iNumItems-1+fItemPaddingEnd;
}

float ActorScroller::GetSecondsForCompleteScrollThrough() const
{
	float fTotalItems = m_fNumItemsToDraw + m_iNumItems;
	return fTotalItems * (m_fSecondsPerItem + m_fSecondsPauseBetweenItems );
}

void ActorScroller::LoadFromNode( const CString &sDir, const XNode *pNode )
{
	ActorFrame::LoadFromNode( sDir, pNode );

#define GET_VALUE( szName, valueOut ) \
	if( !pNode->GetAttrValue( szName, valueOut ) ) { \
		CString sError = ssprintf("ActorScroller in '%s' is missing the value Scroller::%s", sDir.c_str(), szName); \
		LOG->Warn( sError ); \
		Dialog::OK( sError ); \
	}

	float fSecondsPerItem = 0;
	float fNumItemsToDraw = 0;
	CString sTransformFunction;
	int iSubdivisions = 1;

	GET_VALUE( "SecondsPerItem", fSecondsPerItem );
	GET_VALUE( "NumItemsToDraw", fNumItemsToDraw );
	GET_VALUE( "TransformFunction", sTransformFunction );
	pNode->GetAttrValue( "Subdivisions", iSubdivisions );
#undef GET_VALUE

	bool bUseMask = false;
	pNode->GetAttrValue( "UseMask", bUseMask );

	Load3( 
		fNumItemsToDraw,
		sTransformFunction,
		false );
	ActorScroller::SetSecondsPerItem( fSecondsPerItem );
	ActorScroller::SetNumSubdivisions( iSubdivisions );

	if( bUseMask )
		EnableMask( 10, 10 ); // XXX

	pNode->GetAttrValue( "QuantizePixels", m_fQuantizePixels );

	/* By default, don't use item numbers.  On scrollers with lots of items,
	 * especially with Subdivisions > 1, m_exprTransformFunction uses too
	 * much memory, and very few scrollers use this. */
	pNode->GetAttrValue( "UseItemNumber", m_bUseItemNumber );
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
		m_fCurrentItem = fmodf( m_fCurrentItem, (float) m_iNumItems );
}

void ActorScroller::DrawPrimitives()
{
	PositionItemsAndDrawPrimitives( true );
}

void ActorScroller::PositionItems()
{
	PositionItemsAndDrawPrimitives( false );
}

/*
 * Shift m_SubActors forward by iDist.  This will place item m_iFirstSubActorIndex
 * in m_SubActors[0].
 */
void ActorScroller::ShiftSubActors( int iDist )
{
	if( iDist != INT_MAX )
		CircularShift( m_SubActors, iDist );
}

void ActorScroller::PositionItemsAndDrawPrimitives( bool bDrawPrimitives )
{
	if( m_SubActors.empty() )
		return;

	float fNumItemsToDraw = m_fNumItemsToDraw;
	if( !m_quadMask.GetHidden() )
	{
		// write to z buffer so that top and bottom are clipped
		// Draw an extra item; this is the one that will be masked.
		fNumItemsToDraw++;
		float fPositionFullyOffScreenTop = -(fNumItemsToDraw)/2.f;
		float fPositionFullyOffScreenBottom = (fNumItemsToDraw)/2.f;

		m_exprTransformFunction.PositionItem( &m_quadMask, fPositionFullyOffScreenTop, -1, m_iNumItems );
		if( bDrawPrimitives )	m_quadMask.Draw();

		m_exprTransformFunction.PositionItem( &m_quadMask, fPositionFullyOffScreenBottom, m_iNumItems, m_iNumItems );
		if( bDrawPrimitives )	m_quadMask.Draw();
	}

	float fFirstItemToDraw = m_fCurrentItem - fNumItemsToDraw/2.f;
	float fLastItemToDraw = m_fCurrentItem + fNumItemsToDraw/2.f;
	int iFirstItemToDraw = (int) ceilf( fFirstItemToDraw );
	int iLastItemToDraw = (int) ceilf( fLastItemToDraw );
	if( !m_bLoop )
	{
		iFirstItemToDraw = clamp( iFirstItemToDraw, 0, m_iNumItems );
		iLastItemToDraw = clamp( iLastItemToDraw, 0, m_iNumItems );
	}

	bool bDelayedDraw = m_bDrawByZPosition && !m_bLoop;
	vector<Actor*> subs;

	{
		/* Shift m_SubActors so iFirstItemToDraw is at the beginning. */
		int iNewFirstIndex = iFirstItemToDraw;
		int iDist = iNewFirstIndex - m_iFirstSubActorIndex;
		m_iFirstSubActorIndex = iNewFirstIndex;
		ShiftSubActors( iDist );
	}

	int iNumToDraw = iLastItemToDraw - iFirstItemToDraw;
	for( int i = 0; i < iNumToDraw; ++i )
	{
		int iItem = i + iFirstItemToDraw;
		float fPosition = iItem - m_fCurrentItem;
		int iIndex = i; // index into m_SubActors
		if( m_bLoop )
			wrap( iIndex, m_SubActors.size() );
		else if( iIndex < 0 || iIndex >= (int)m_SubActors.size() )
			continue;

		if( !m_bUseItemNumber )
			iItem = 0;

		m_exprTransformFunction.PositionItem( m_SubActors[iIndex], fPosition, iItem, m_iNumItems );
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
	static int setsecondsperitem( T* p, lua_State *L )	{ p->SetSecondsPerItem(FArg(1)); return 0; }
	static int setnumsubdivisions( T* p, lua_State *L )	{ p->SetNumSubdivisions(IArg(1)); return 0; }
	static int scrollthroughallitems( T* p, lua_State *L )	{ p->ScrollThroughAllItems(); return 0; }
	static int scrollwithpadding( T* p, lua_State *L )	{ p->ScrollWithPadding(FArg(1),FArg(2)); return 0; }
	static int setfastcatchup( T* p, lua_State *L )	{ p->SetFastCatchup(BArg(1)); return 0; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( SetCurrentAndDestinationItem );
		ADD_METHOD( setsecondsperitem );
		ADD_METHOD( setnumsubdivisions );
		ADD_METHOD( scrollthroughallitems );
		ADD_METHOD( scrollwithpadding );
		ADD_METHOD( setfastcatchup );
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
