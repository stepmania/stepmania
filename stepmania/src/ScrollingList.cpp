#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScrollingList

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ScrollingList.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"

const int DEFAULT_VISIBLE_ELEMENTS = 9;
const int DEFAULT_SPACING = 300;

const RageColor COLOR_SELECTED = RageColor(1.0f,1.0f,1.0f,1);
const RageColor COLOR_NOT_SELECTED = RageColor(0.4f,0.4f,0.4f,1);

/***************************************
ScrollingList

Initializes Variables for the ScrollingList
****************************************/
ScrollingList::ScrollingList()
{
	m_iSelection = 0;
	m_fSelectionLag = 0;
	m_iSpacing = DEFAULT_SPACING;
	m_iNumVisible = DEFAULT_VISIBLE_ELEMENTS;
}

ScrollingList::~ScrollingList()
{
	Unload();
}

void ScrollingList::Unload()
{
	for( unsigned i=0; i<m_apSprites.size(); i++ )
		delete m_apSprites[i];
	m_apSprites.clear();
}

/************************************
Allows us to create a graphic element
in the scrolling list
*************************************/
void ScrollingList::Load( const CStringArray& asGraphicPaths )
{
	Unload();
	for( unsigned i=0; i<asGraphicPaths.size(); i++ )
	{
		Sprite* pNewSprite = new Sprite;
		pNewSprite->Load( asGraphicPaths[i] );
		m_apSprites.push_back( pNewSprite );
	}
}


/**************************************
ShiftLeft

Make the entire list shuffle left
**************************************/
void ScrollingList::Left()
{
	ASSERT( !m_apSprites.empty() );	// nothing loaded!

	m_iSelection = (m_iSelection + m_apSprites.size() - 1) % m_apSprites.size();	// decrement with wrapping
	m_fSelectionLag -= 1;
}

/**************************************
ShiftRight

Make the entire list shuffle right
**************************************/
void ScrollingList::Right()
{
	ASSERT( !m_apSprites.empty() );	// nothing loaded!

	m_iSelection = (m_iSelection + 1) % m_apSprites.size();	// increment with wrapping
	m_fSelectionLag += 1;
}

/***********************************
SetCurrentPostion

From the current postion in the array, add graphic elements
in either direction to make the list seem infinite.
***********************************/
void ScrollingList::SetSelection( int iIndex )
{
	m_iSelection = iIndex;
}

int ScrollingList::GetSelection()
{
	return m_iSelection;
}

void ScrollingList::SetSpacing( int iSpacingInPixels )
{
	m_iSpacing = iSpacingInPixels;
}

/******************************
SetNumberVisibleElements

Allows us to set whether 3,4 or 5
elements are visible on screen at once
*******************************/
void ScrollingList::SetNumberVisible( int iNumVisibleElements )
{
	m_iNumVisible = iNumVisibleElements;
}

/*******************************
Update

Updates the actorframe
********************************/
void ScrollingList::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_apSprites.empty() )
		return;

	// update m_fLaggingSelection
	if( m_fSelectionLag != 0 )
	{
		const float fSign = m_fSelectionLag<0 ? -1.0f : +1.0f; 
		const float fVelocity = -fSign + -m_fSelectionLag*10;
		m_fSelectionLag += fVelocity * fDeltaTime;

		// check to see if m_fLaggingSelection passed its destination
		const float fNewSign = m_fSelectionLag<0 ? -1.0f : +1.0f; 
		if( (fSign<0) ^ (fNewSign<0) )	// they have different signs
			m_fSelectionLag = 0;		// snap
	}

	for( unsigned i=0; i<m_apSprites.size(); i++ )
		m_apSprites[i]->Update( fDeltaTime );
}

/********************************
DrawPrimitives

Draws the elements onto the screen
*********************************/
void ScrollingList::DrawPrimitives()
{
	ASSERT( !m_apSprites.empty() );

	for( int i=(m_iNumVisible)/2; i>= 0; i-- )	// draw outside to inside
	{
		int iIndexToDraw1 = m_iSelection - i;
		int iIndexToDraw2 = m_iSelection + i;
		
		// wrap IndexToDraw*
		iIndexToDraw1 = (iIndexToDraw1 + m_apSprites.size()*300) % m_apSprites.size();	// make sure this is positive
		iIndexToDraw2 = iIndexToDraw2 % m_apSprites.size();

		ASSERT( iIndexToDraw1 >= 0 );

		m_apSprites[iIndexToDraw1]->SetX( (-i+m_fSelectionLag) * m_iSpacing );
		m_apSprites[iIndexToDraw2]->SetX( (+i+m_fSelectionLag) * m_iSpacing );

		if( i==0 )	// so we don't draw 0 twice
		{
			m_apSprites[iIndexToDraw1]->SetDiffuse( COLOR_SELECTED );
			m_apSprites[iIndexToDraw1]->Draw();
		}
		else
		{
			m_apSprites[iIndexToDraw1]->SetDiffuse( COLOR_NOT_SELECTED );
			m_apSprites[iIndexToDraw2]->SetDiffuse( COLOR_NOT_SELECTED );
			m_apSprites[iIndexToDraw1]->Draw();
			m_apSprites[iIndexToDraw2]->Draw();
		}
	}
}
