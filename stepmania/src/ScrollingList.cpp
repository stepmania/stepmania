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

const int DEFAULT_VISIBLE_ELEMENTS = 3;

const float SPACING3ELEMENTS = 310.0f;
const float SPACING4ELEMENTS = 225.0f;
const float SPACING5ELEMENTS = 150.0f;


ScrollingListDisplay::ScrollingListDisplay()
{

}

/************************************
Allows us to create a graphic element
in the scrolling list
*************************************/

void ScrollingListDisplay::Load( CString graphiclocation )
{
	m_gLocation = graphiclocation;
	m_sprListElement.Load( THEME->GetPathTo("Graphics",m_gLocation) );
	this->AddSubActor( &m_sprListElement );
}

/***********************************
RedefineGraphic

Allows us to change a graphic
element in the scrolling list
************************************/

void ScrollingListDisplay::RedefineGraphic( CString graphiclocation )
{
	m_gLocation = graphiclocation;
	m_sprListElement.Load( THEME->GetPathTo("Graphics",m_gLocation) );
}

/*************************************
GetGraphicLocation

Returns the graphic filename from a
scrollinglist element
**************************************/

CString ScrollingListDisplay::GetGraphicLocation()
{
	return m_gLocation;
}

/***************************************
ScrollingList

Initializes Variables for the ScrollingList
****************************************/

ScrollingList::ScrollingList()
{
	m_iNumContents = 0;
	m_iNumVisElements = DEFAULT_VISIBLE_ELEMENTS;
	m_iCurrentPos = 0;
}

/**************************************
CreateNewElement

Adds a new graphic element to the end of
the scrolling element
***************************************/

void ScrollingList::CreateNewElement( CString graphiclocation )
{
	float CurrentSpacing;
	if (m_iNumVisElements <= 3)
	{		
		CurrentSpacing = SPACING3ELEMENTS;
	}
	else if (m_iNumVisElements == 4)
	{
		CurrentSpacing = SPACING4ELEMENTS;
	}
	else
	{
		CurrentSpacing = SPACING5ELEMENTS;
	}
	
	m_ScrollingListDisplays[m_iNumContents].Load( graphiclocation );
	this->AddSubActor( &m_ScrollingListDisplays[m_iNumContents] );

	m_ScrollingListDisplays[m_iNumContents].SetX( 0 - (CurrentSpacing * m_iNumContents) );

	if (m_iNumContents != SCRLIST_MAX_TOTAL_CONTENTS) // make sure that we cannot 'over create' menus
		m_iNumContents = m_iNumContents + 1;
}


/**************************************
ShiftLeft

Make the entire list shuffle left
**************************************/

void ScrollingList::ShiftLeft()
{
	if ( m_iCurrentPos == 0 ) // if we're at the start of the list wrap to the end
	{
		m_iCurrentPos = m_iNumContents - 1;
	}
	else // just decrease our position
	{
		m_iCurrentPos--;
	}

	float CurrentSpacing;
	if (m_iNumVisElements <= 3)
	{		
		CurrentSpacing = SPACING3ELEMENTS;
	}
	else if (m_iNumVisElements == 4)
	{
		CurrentSpacing = SPACING4ELEMENTS;
	}
	else
	{
		CurrentSpacing = SPACING5ELEMENTS;
	}


	SetCurrentPosition( m_iCurrentPos );

	for (int i=10; i > 0; i-- ) // 21 elements (10 going in both directions plus a 0)
	{
		if (m_iCurrentPos - i >= 0) // set -ve -tweening
		{
			m_ScrollingListDisplays[m_iCurrentPos - i].SetX( 0 - ((i-1) * CurrentSpacing) - (CurrentSpacing * 2) );
			m_ScrollingListDisplays[m_iCurrentPos - i].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[m_iCurrentPos - i].SetTweenX( 0 - (CurrentSpacing * i) );
		}
		else
		{
			m_ScrollingListDisplays[m_iNumContents - i].SetX( 0 - ((i-1) * CurrentSpacing)  - (CurrentSpacing * 2));
			m_ScrollingListDisplays[m_iNumContents - i].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[m_iNumContents - i].SetTweenX( 0 - (CurrentSpacing * i) );
		}

		if (m_iCurrentPos + i <= m_iNumContents - 1) // set +ve tweening
		{
			m_ScrollingListDisplays[m_iCurrentPos + i].SetX( 0 + (CurrentSpacing * i) + CurrentSpacing  - (CurrentSpacing * 2));
			m_ScrollingListDisplays[m_iCurrentPos + i].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[m_iCurrentPos + i].SetTweenX( 0 + (CurrentSpacing * i) );
		}
		else
		{
			m_ScrollingListDisplays[i-1].SetX( 0 + (CurrentSpacing * i) + CurrentSpacing  - (CurrentSpacing * 2));
			m_ScrollingListDisplays[i-1].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[i-1].SetTweenX( 0 + (CurrentSpacing * i) );
		}
	}

	m_ScrollingListDisplays[m_iCurrentPos].SetX( 0 + CurrentSpacing   - (CurrentSpacing * 2));
	m_ScrollingListDisplays[m_iCurrentPos].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
	m_ScrollingListDisplays[m_iCurrentPos].SetTweenX( 0 );
}

/**************************************
ShiftRight

Make the entire list shuffle right
**************************************/

void ScrollingList::ShiftRight()
{
	if ( m_iCurrentPos == m_iNumContents - 1 ) // if we're at the end of the list wrap to the start
	{
		m_iCurrentPos = 0;
	}
	else // just decrease our position
	{
		m_iCurrentPos++;
	}

	float CurrentSpacing;
	if (m_iNumVisElements <= 3)
	{		
		CurrentSpacing = SPACING3ELEMENTS;
	}
	else if (m_iNumVisElements == 4)
	{
		CurrentSpacing = SPACING4ELEMENTS;
	}
	else
	{
		CurrentSpacing = SPACING5ELEMENTS;
	}


	SetCurrentPosition( m_iCurrentPos );

	for (int i=10; i > 0; i-- ) // 21 elements (10 going in both directions plus a 0)
	{
		if (m_iCurrentPos - i >= 0) // set -ve -tweening
		{
			m_ScrollingListDisplays[m_iCurrentPos - i].SetX( 0 - ((i-1) * CurrentSpacing) );
			m_ScrollingListDisplays[m_iCurrentPos - i].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[m_iCurrentPos - i].SetTweenX( 0 - (CurrentSpacing * i) );
		}
		else
		{
			m_ScrollingListDisplays[m_iNumContents - i].SetX( 0 - ((i-1) * CurrentSpacing) );
			m_ScrollingListDisplays[m_iNumContents - i].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[m_iNumContents - i].SetTweenX( 0 - (CurrentSpacing * i) );
		}

		if (m_iCurrentPos + i <= m_iNumContents - 1) // set +ve tweening
		{
			m_ScrollingListDisplays[m_iCurrentPos + i].SetX( 0 + (CurrentSpacing * i) + CurrentSpacing );
			m_ScrollingListDisplays[m_iCurrentPos + i].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[m_iCurrentPos + i].SetTweenX( 0 + (CurrentSpacing * i) );
		}
		else
		{
			m_ScrollingListDisplays[i-1].SetX( 0 + (CurrentSpacing * i) + CurrentSpacing );
			m_ScrollingListDisplays[i-1].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
			m_ScrollingListDisplays[i-1].SetTweenX( 0 + (CurrentSpacing * i) );
		}
	}

	m_ScrollingListDisplays[m_iCurrentPos].SetX( 0 + CurrentSpacing );
	m_ScrollingListDisplays[m_iCurrentPos].BeginTweening( 0.3f, TWEEN_BIAS_BEGIN );
	m_ScrollingListDisplays[m_iCurrentPos].SetTweenX( 0 );
}

/***********************************
SetCurrentPostion

From the current postion in the array, add graphic elements
in either direction to make the list seem infinite.
***********************************/

void ScrollingList::SetCurrentPosition( int CurrentPos )
{
	m_iCurrentPos = CurrentPos;
	// Setup Spacing
	float CurrentSpacing;
	if (m_iNumVisElements <= 3)
	{		
		CurrentSpacing = SPACING3ELEMENTS;
	}
	else if (m_iNumVisElements == 4)
	{
		CurrentSpacing = SPACING4ELEMENTS;
	}
	else
	{
		CurrentSpacing = SPACING5ELEMENTS;
	}

	// ORDER SPECIFICALLY!
	// Central Element at front, then the next two behind, then the outer two behind them e.t.c.
	// 21012 << Central element is 0, 1's are either side, 2's either side of those 3's come off-screen (regardless of spacing)
	// we need 3's incase the user suddenly scrolls!!

		for( int i=3; i > 0; i--)
		{
			// Find the -ve Element
			if ((CurrentPos - i) >= 0) // Bounds Checking: If we aren't under the first element
			{
				m_ScrollingListDisplays[CurrentPos-i].SetX( 0 - (i * CurrentSpacing ));
			}
			else // if we are under the final limit (by i)
			{
				m_ScrollingListDisplays[m_iNumContents+(CurrentPos - i)].SetX( 0 - (i * CurrentSpacing ));
			}

			// Find the +ve Element
			if ((CurrentPos + i) <= m_iNumContents-1) // Bounds Checking: If we aren't over the final element
			{
				m_ScrollingListDisplays[CurrentPos+i].SetX( 0 + (i * CurrentSpacing ));
			}
			else // if we are over the final limit (by i)
			{
				m_ScrollingListDisplays[i-1].SetX( 0 + (i * CurrentSpacing ));
			}
		}

	// Set The MIDDLE element
	m_ScrollingListDisplays[CurrentPos].SetX( 0 );
}

/******************************
SetNumberVisibleElements

Allows us to set whether 3,4 or 5
elements are visible on screen at once
*******************************/

void ScrollingList::SetNumberVisibleElements( int VisibleElements )
{
	m_iNumVisElements = VisibleElements;
}

/*******************************
Update

Updates the actorframe
********************************/

void ScrollingList::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	for( int i=0; i<m_iNumContents; i++ )
		m_ScrollingListDisplays[i].Update( fDeltaTime );
}

/********************************
DrawPrimitives

Draws the elements onto the screen
*********************************/

void ScrollingList::DrawPrimitives()
{
	const D3DXCOLOR OPT_NOT_SELECTED = D3DXCOLOR(0.4f,0.4f,0.4f,1);
	const D3DXCOLOR OPT_SELECTED = D3DXCOLOR(1.0f,1.0f,1.0f,1);

	for (int i=m_iNumContents; i>=0; i--)
	{
		if (i != m_iCurrentPos)
		{
			m_ScrollingListDisplays[i].SetDiffuseColor( OPT_NOT_SELECTED );
		}
		else
		{
			m_ScrollingListDisplays[i].SetDiffuseColor( OPT_SELECTED );
		}
	}


	// Start Drawing in a Specific Order For the elements around the current element
	if (m_iCurrentPos == 0) // start of list?
	{
		m_ScrollingListDisplays[m_iNumContents - 2].Draw();
		m_ScrollingListDisplays[2].Draw();

		m_ScrollingListDisplays[m_iNumContents - 1].Draw();
		m_ScrollingListDisplays[1].Draw();
	}
	else if (m_iCurrentPos == m_iNumContents - 1) // end of list
	{
		m_ScrollingListDisplays[1].Draw();
		m_ScrollingListDisplays[m_iNumContents - 3].Draw();

		m_ScrollingListDisplays[0].Draw();
		m_ScrollingListDisplays[m_iNumContents - 2].Draw();
	}
	else if (m_iCurrentPos == 1) // near start
	{
		m_ScrollingListDisplays[m_iNumContents - 2].Draw();
		m_ScrollingListDisplays[3].Draw();

		m_ScrollingListDisplays[2].Draw();
		m_ScrollingListDisplays[0].Draw();		
	}
	else if (m_iCurrentPos == m_iNumContents - 2) // near end
	{
		m_ScrollingListDisplays[0].Draw();
		m_ScrollingListDisplays[m_iNumContents - 4].Draw();

		m_ScrollingListDisplays[m_iNumContents - 1].Draw();
		m_ScrollingListDisplays[m_iNumContents - 3].Draw();		
	}
	else // we're somewhere in the middle
	{
		m_ScrollingListDisplays[m_iCurrentPos + 2].Draw();
		m_ScrollingListDisplays[m_iCurrentPos - 2].Draw();
		m_ScrollingListDisplays[m_iCurrentPos - 1].Draw();
		m_ScrollingListDisplays[m_iCurrentPos + 1].Draw();
	}

	m_ScrollingListDisplays[m_iCurrentPos].Draw();

}
