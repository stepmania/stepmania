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

// lua start
LUA_REGISTER_CLASS( ActorScroller )
// lua end

/* Tricky: We need ActorFrames created in XML to auto delete their children.
 * We don't want classes that derive from ActorFrame to auto delete their 
 * children.  The name "ActorFrame" is widely used in XML, so we'll have
 * that string instead create an ActorFrameAutoDeleteChildren object.
 */
//REGISTER_ACTOR_CLASS( ActorScroller )
REGISTER_ACTOR_CLASS_WITH_NAME( ActorScrollerAutoDeleteChildren, ActorScroller )


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

void ActorScroller::Load( 
	float fSecondsPerItem, 
	float fNumItemsToDraw, 
	const RageVector3	&vRotationDegrees,
	const RageVector3	&vTranslateTerm0,
	const RageVector3	&vTranslateTerm1,
	const RageVector3	&vTranslateTerm2,
	bool bUseMask
	)
{
	m_fSecondsPerItem = fSecondsPerItem;
	m_fNumItemsToDraw = fNumItemsToDraw;
	
	
	// Note: Rotation is applied before translation.
	// rot = m_vRotationDegrees*itemOffset^1
	// trans = m_vTranslateTerm0*itemOffset^0 + 
	//		   m_vTranslateTerm1*itemOffset^1 +
	//		   m_vTranslateTerm2*itemOffset^2
	ostringstream s;
	s << 
		"function(self,offset,itemIndex,numItems) " <<
		"self:x(" << vTranslateTerm0.x << " + " << vTranslateTerm1.x << "*offset + " << vTranslateTerm2.x << "*offset*offset); " <<
		"self:y(" << vTranslateTerm0.y << " + " << vTranslateTerm1.y << "*offset + " << vTranslateTerm2.y << "*offset*offset); " <<
		"self:z(" << vTranslateTerm0.z << " + " << vTranslateTerm1.z << "*offset + " << vTranslateTerm2.z << "*offset*offset); " <<
		"self:rotationx(" << vRotationDegrees.x << "*offset); " <<
		"self:rotationy(" << vRotationDegrees.y << "*offset); " <<
		"self:rotationz(" << vRotationDegrees.z << "*offset); " <<
		"end";
	m_exprTransform.SetFromExpression( s.str() );

	m_fQuantizePixels = 0;
	m_bUseMask = bUseMask;

	m_bLoaded = true;
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

	m_exprTransform.SetFromExpression( 
		ssprintf("function(self,offset,itemIndex,numItems) return self:y(%f*offset) end",fItemHeight) 
		);

	m_bLoop = bLoop; 
	m_fSecondsPerItem = fSecondsPerItem; 
	m_fSecondsPauseBetweenItems = fSecondsPauseBetweenItems;
	m_fCurrentItem = m_bLoop ? 0 : (float)-m_fNumItemsToDraw;
	m_fDestinationItem = (float)(m_SubActors.size()+1);
	m_fPauseCountdownSeconds = 0;
	m_fQuantizePixels = 0;

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
	const CString &sExprTransform
	)
{
	m_fSecondsPerItem = fSecondsPerItem;
	m_fNumItemsToDraw = fNumItemsToDraw;
	m_bFastCatchup = bFastCatchup;
	m_exprTransform.SetFromExpression( sExprTransform );
	m_fQuantizePixels = 0;
	m_bLoaded = true;
}

float ActorScroller::GetSecondsForCompleteScrollThrough()
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
	RageVector3	vRotationDegrees = RageVector3(0,0,0);
	RageVector3	vTranslateTerm0 = RageVector3(0,0,0);
	RageVector3	vTranslateTerm1 = RageVector3(0,0,0);
	RageVector3	vTranslateTerm2 = RageVector3(0,0,0);
	float fItemPaddingStart = 0;
	float fItemPaddingEnd = 0;

	GET_VALUE( "SecondsPerItem", fSecondsPerItem );
	GET_VALUE( "NumItemsToDraw", fNumItemsToDraw );
	GET_VALUE( "RotationDegreesX", vRotationDegrees.x );
	GET_VALUE( "RotationDegreesY", vRotationDegrees.y );
	GET_VALUE( "RotationDegreesZ", vRotationDegrees.z );
	GET_VALUE( "TranslateTerm0X", vTranslateTerm0.x );
	GET_VALUE( "TranslateTerm0Y", vTranslateTerm0.y );
	GET_VALUE( "TranslateTerm0Z", vTranslateTerm0.z );
	GET_VALUE( "TranslateTerm1X", vTranslateTerm1.x );
	GET_VALUE( "TranslateTerm1Y", vTranslateTerm1.y );
	GET_VALUE( "TranslateTerm1Z", vTranslateTerm1.z );
	GET_VALUE( "TranslateTerm2X", vTranslateTerm2.x );
	GET_VALUE( "TranslateTerm2Y", vTranslateTerm2.y );
	GET_VALUE( "TranslateTerm2Z", vTranslateTerm2.z );
	GET_VALUE( "ItemPaddingStart", fItemPaddingStart );
	GET_VALUE( "ItemPaddingEnd", fItemPaddingEnd );
#undef GET_VALUE

	Load( 
		fSecondsPerItem,
		fNumItemsToDraw,
		vRotationDegrees,
		vTranslateTerm0,
		vTranslateTerm1,
		vTranslateTerm2 );
	SetCurrentAndDestinationItem( -fItemPaddingStart );
	SetDestinationItem( m_SubActors.size()-1+fItemPaddingEnd );

	pNode->GetAttrValue( "QuantizePixels", m_fQuantizePixels );
}

void ActorScroller::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_fHibernateSecondsLeft > 0 )
		return;	// early abort

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


void ActorScroller::PositionItem( Actor *pActor, float fPositionOffsetFromCenter, int iItemIndex, int iNumItems )
{
	m_exprTransform.PushSelf( LUA->L );
	ASSERT( !lua_isnil(LUA->L, -1) );
	pActor->PushSelf( LUA->L );
	LuaHelpers::Push( fPositionOffsetFromCenter, LUA->L );
	LuaHelpers::Push( iItemIndex, LUA->L );
	LuaHelpers::Push( iNumItems, LUA->L );
	lua_call( LUA->L, 4, 0 ); // 4 args, 0 results
}

void ActorScroller::DrawPrimitives()
{
	// Optimization:  If we weren't loaded, then fall back to the ActorFrame logic
	if( !m_bLoaded )
	{
		ActorFrame::DrawPrimitives();
		return;
	}

	if( m_SubActors.empty() )
		return;

	// write to z buffer so that top and bottom are clipped
	float fPositionFullyOnScreenTop = -(m_fNumItemsToDraw-1)/2.f;
	float fPositionFullyOnScreenBottom = (m_fNumItemsToDraw-1)/2.f;
	float fPositionFullyOffScreenTop = fPositionFullyOnScreenTop - 1;
	float fPositionFullyOffScreenBottom = fPositionFullyOnScreenBottom + 1;
	float fPositionOnEdgeOfScreenTop = -(m_fNumItemsToDraw)/2.f;
	float fPositionOnEdgeOfScreenBottom = (m_fNumItemsToDraw)/2.f;
	
	float fFirstItemToDraw = 0;
	float fLastItemToDraw = 0;

	if( m_bUseMask )
	{
		PositionItem( &m_quadMask, fPositionFullyOffScreenTop, -1, m_SubActors.size() );
		m_quadMask.Draw();

		PositionItem( &m_quadMask, fPositionFullyOffScreenBottom, m_SubActors.size(), m_SubActors.size() );
		m_quadMask.Draw();

		fFirstItemToDraw = fPositionFullyOffScreenTop + m_fCurrentItem;
		fLastItemToDraw = fPositionFullyOffScreenBottom + m_fCurrentItem;
	}
	else
	{
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
			wrap( iIndex, m_SubActors.size()-1 );
		else if( iIndex < 0 || iIndex >= (int)m_SubActors.size() )
			continue;

		PositionItem( m_SubActors[iIndex], fPosition, iIndex, m_SubActors.size() );
		if( bDelayedDraw )
			subs.push_back( m_SubActors[iIndex] );
		else
			m_SubActors[iIndex]->Draw();
	}

	if( bDelayedDraw )
	{
		ActorUtil::SortByZPosition( subs );
		FOREACH( Actor*, subs, a )
			(*a)->Draw();
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
