#ifndef ActorScroller_H
#define ActorScroller_H
/*
-----------------------------------------------------------------------------
 Class: ActorScroller

 Desc: An ActorFrame that moves its children.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"


class ActorScroller : public ActorFrame
{
public:
	ActorScroller();

	void Load( float fScrollSecondsPerItem, float fSpacingX, float fSpacingY );

	virtual void Update( float fDelta );

	void SetDestinationItem( int iItem ) { m_fDestinationItem = iItem; }
	void SetCurrentAndDestinationItem( int iItem ) { m_fCurrentItem = m_fDestinationItem = iItem; }

protected:
	float		m_fCurrentItem; // usually between 0 and m_SubActors.size()
	float		m_fDestinationItem;
	float		m_fScrollSecondsPerItem;
	RageVector2	m_vSpacing;
};


#endif
