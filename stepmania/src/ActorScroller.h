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

	void Load( 
		float fScrollSecondsPerItem, 
		int iNumItemsToDraw, 
		const RageVector3	&vRotationDegrees,
		const RageVector3	&vTranslateTerm0,
		const RageVector3	&vTranslateTerm1,
		const RageVector3	&vTranslateTerm2 );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();	// DOES draw

	void SetDestinationItem( int iItem ) { m_fDestinationItem = float(iItem); }
	void SetCurrentAndDestinationItem( int iItem ) { m_fCurrentItem = m_fDestinationItem = float(iItem); }

protected:
	bool		m_bLoaded;
	float		m_fCurrentItem; // usually between 0 and m_SubActors.size()
	float		m_fDestinationItem;
	float		m_fSecondsPerItem;
	int			m_iNumItemsToDraw;
	
	// Note: Rotation is applied before translation.

	// rot = m_vRotationDegrees*itemOffset^1
	RageVector3	m_vRotationDegrees;

	// trans = m_vTranslateTerm0*itemOffset^0 + 
	//		   m_vTranslateTerm1*itemOffset^1 +
	//		   m_vTranslateTerm2*itemOffset^2
	RageVector3	m_vTranslateTerm0;
	RageVector3	m_vTranslateTerm1;
	RageVector3	m_vTranslateTerm2;
};


#endif
