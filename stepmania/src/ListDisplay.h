#ifndef ListDisplay_H
#define ListDisplay_H
/*
-----------------------------------------------------------------------------
 Class: ListDisplay

 Desc: Holds course name and banner.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Quad.h"

class ListDisplay : public ActorFrame
{
public:
	ListDisplay();

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Load(
		const vector<Actor*> &m_vpItems, 
		int iNumItemsToShow, 
		float fItemWidth, 
		float fItemHeight, 
		bool bLoop, 
		float fSecondsPerItem, 
		float fSecondsPauseBetweenItems,
		bool bSlide );

	float GetSecondsForCompleteScrollThrough();

protected:
	int		m_iNumItemsToShow;
	float	m_fItemWidth;
	float	m_fItemHeight;
	bool	m_bLoop; 
	float	m_fSecondsPerItem;
	float	m_fSecondsPauseBetweenItems;
	bool	m_bSlide;

	float	m_fItemAtTopOfList;
	float	m_fSecondsPauseCountdown;
	Quad	m_quadMask;
};

#endif
