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
#include "ThemeManager.h"

enum BANNER_PREFS_TYPES
{
	BANNERPREFS_DDRFLAT=0,
	BANNERPREFS_DDRROT,
	BANNERPREFS_EZ2,
	BANNERPREFS_PUMP,
	BANNERPREFS_PARA
};

#define BANNER_WIDTH			THEME->GetMetricF("ScreenSelectMusic","BannerWidth")
#define BANNER_HEIGHT			THEME->GetMetricF("ScreenSelectMusic","BannerHeight")
#define EZ2_BANNER_WIDTH 92
#define EZ2_BANNER_HEIGHT 92
#define EZ2_BANNER_ZOOM 2.0

#define SPRITE_TYPE_SPRITE 0
#define SPRITE_TYPE_CROPPEDSPRITE 1
#define DDRROT_ROTATION 315

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
	m_iBannerPrefs = BANNERPREFS_EZ2;
	m_iSpriteType = SPRITE_TYPE_SPRITE;
	m_iSelection = 0;
	m_fSelectionLag = 0;
	m_iSpacing = DEFAULT_SPACING;
	m_iNumVisible = DEFAULT_VISIBLE_ELEMENTS;
}

void ScrollingList::UseSpriteType(int NewSpriteType)
{
	m_iSpriteType = NewSpriteType;
}

ScrollingList::~ScrollingList()
{
	Unload();
}

void ScrollingList::Unload()
{
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		for( unsigned i=0; i<m_apSprites.size(); i++ )
			delete m_apSprites[i];
		m_apSprites.clear();
	}
	else
	{
		for( unsigned i=0; i<m_apCSprites.size(); i++ )
			delete m_apCSprites[i];
		m_apCSprites.clear();
	}
}

/************************************
Allows us to create a graphic element
in the scrolling list
*************************************/
void ScrollingList::Load( const CStringArray& asGraphicPaths )
{
	RageTexturePrefs prefs;

	Unload();
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		for( unsigned i=0; i<asGraphicPaths.size(); i++ )
		{
			Sprite* pNewSprite = new Sprite;
			pNewSprite->Load( asGraphicPaths[i] );
			m_apSprites.push_back( pNewSprite );
		}
	}
	else
	{
		for( unsigned i=0; i<asGraphicPaths.size(); i++ )
		{
			CroppedSprite* pNewCSprite = new CroppedSprite;
			if(m_iBannerPrefs == BANNERPREFS_DDRFLAT)
			{
				pNewCSprite->SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
			}
			else if(m_iBannerPrefs == BANNERPREFS_DDRROT)
			{
				pNewCSprite->SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
				pNewCSprite->SetRotation( DDRROT_ROTATION );
			}
			else if(m_iBannerPrefs == BANNERPREFS_EZ2)
			{
				pNewCSprite->SetCroppedSize( EZ2_BANNER_WIDTH*2, EZ2_BANNER_HEIGHT*2 );
			}

			pNewCSprite->Load( asGraphicPaths[i], prefs );
			m_apCSprites.push_back( pNewCSprite );
		}
	}
}


/**************************************
ShiftLeft

Make the entire list shuffle left
**************************************/
void ScrollingList::Left()
{
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		ASSERT( !m_apSprites.empty() );	// nothing loaded!

		m_iSelection = (m_iSelection + m_apSprites.size() - 1) % m_apSprites.size();	// decrement with wrapping
		m_fSelectionLag -= 1;
	}
	else
	{
		ASSERT( !m_apCSprites.empty() );	// nothing loaded!

		m_iSelection = (m_iSelection + m_apCSprites.size() - 1) % m_apCSprites.size();	// decrement with wrapping
		m_fSelectionLag -= 1;
	}
}

/**************************************
ShiftRight

Make the entire list shuffle right
**************************************/
void ScrollingList::Right()
{
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		ASSERT( !m_apSprites.empty() );	// nothing loaded!

		m_iSelection = (m_iSelection + 1) % m_apSprites.size();	// increment with wrapping
		m_fSelectionLag += 1;
	}
	else
	{
		ASSERT( !m_apCSprites.empty() );	// nothing loaded!

		m_iSelection = (m_iSelection + 1) % m_apCSprites.size();	// increment with wrapping
		m_fSelectionLag += 1;
	}
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
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		if( m_apSprites.empty() )
			return;
	}
	else
	{
		if( m_apCSprites.empty() )
			return;
	}

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

	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		for( unsigned i=0; i<m_apSprites.size(); i++ )
			m_apSprites[i]->Update( fDeltaTime );
	}
	else
	{
		for( unsigned i=0; i<m_apCSprites.size(); i++ )
			m_apCSprites[i]->Update( fDeltaTime );
	}
}

/********************************
DrawPrimitives

Draws the elements onto the screen
*********************************/
void ScrollingList::DrawPrimitives()
{
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		ASSERT( !m_apSprites.empty() );
	}
	else
	{
		ASSERT( !m_apCSprites.empty() );
	}

	for( int i=(m_iNumVisible)/2; i>= 0; i-- )	// draw outside to inside
	{
		int iIndexToDraw1 = m_iSelection - i;
		int iIndexToDraw2 = m_iSelection + i;
		
		if(m_iSpriteType == SPRITE_TYPE_SPRITE)
		{
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
		else
		{
			// wrap IndexToDraw*
			iIndexToDraw1 = (iIndexToDraw1 + m_apCSprites.size()*300) % m_apCSprites.size();	// make sure this is positive
			iIndexToDraw2 = iIndexToDraw2 % m_apCSprites.size();

			ASSERT( iIndexToDraw1 >= 0 );

			m_apCSprites[iIndexToDraw1]->SetX( (-i+m_fSelectionLag) * m_iSpacing );
			m_apCSprites[iIndexToDraw2]->SetX( (+i+m_fSelectionLag) * m_iSpacing );

			if( i==0 )	// so we don't draw 0 twice
			{
				m_apCSprites[iIndexToDraw1]->SetDiffuse( COLOR_SELECTED );
				m_apCSprites[iIndexToDraw1]->Draw();
			}
			else
			{
				m_apCSprites[iIndexToDraw1]->SetDiffuse( COLOR_NOT_SELECTED );
				m_apCSprites[iIndexToDraw2]->SetDiffuse( COLOR_NOT_SELECTED );
				m_apCSprites[iIndexToDraw1]->Draw();
				m_apCSprites[iIndexToDraw2]->Draw();
			}
		}
	}
}
