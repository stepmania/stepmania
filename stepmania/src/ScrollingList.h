#ifndef SCROLLINGLIST_H
#define SCROLLINGLIST_H
/*
-----------------------------------------------------------------------------
 Class: ScrollingList

 Desc: Creates an array of graphics which can scroll left and right.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"


class ScrollingList : public ActorFrame
{
public:
	ScrollingList();
	~ScrollingList();

	void Load( const CStringArray& asGraphicPaths );
	void Unload();	// delete all items.  Called automatically on Load()

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetSelection( int iIndex );	
	int GetSelection();
	void SetNumberVisible( int iNumVisibleElements );
	void SetSpacing( int iSpacingInPixels );
	
	void Left();
	void Right();

protected:

	int						m_iSelection;
	float					m_fSelectionLag;
	int						m_iSpacing;
	int						m_iNumVisible;
	CArray<Sprite*,Sprite*>	m_apSprites;	// stores the list of elements (left to right)
};

#endif
