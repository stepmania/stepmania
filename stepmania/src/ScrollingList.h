/*
-----------------------------------------------------------------------------
 Class: ScrollingList.h

 Desc: Creates an array of graphics which can scroll left and right.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#pragma once
#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "RandomSample.h"
#include "RandomStream.h"


// const int SCRLIST_MAX_VISIBLE_CONTENTS = 5;
const int SCRLIST_MAX_TOTAL_CONTENTS = 20; // this is only meant for menu systems, not song lists


class ScrollingListDisplay : public ActorFrame
{
public:
	ScrollingListDisplay();

	void Load( CString graphiclocation );
	void RedefineGraphic( CString graphiclocation );
	CString GetGraphicLocation();
	
	CString m_gLocation;
	Sprite		m_sprListElement;
};


class ScrollingList : public ActorFrame
{
public:
	ScrollingList();

	void SetCurrentPosition( int CurrentPos );	
	void SetNumberVisibleElements( int VisibleElements );
	void CreateNewElement( CString graphiclocation);
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	void ShiftLeft();
	void ShiftRight();

//	void SetFromCourse( Course* pCourse );

protected:

//	Quad		m_quad;

	int						m_iCurrentPos;
	int						m_iNumVisElements;
	int						m_iNumContents;
	ScrollingListDisplay	m_ScrollingListDisplays[SCRLIST_MAX_TOTAL_CONTENTS]; // stores the list of elements (from start to finish)

//	float m_fTimeUntilScroll;
//	float m_fItemAtTopOfList;	// between 0 and m_iNumContents
};