#include "global.h"
#include "ScrollingList.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "Course.h"
#include "SongManager.h"
#include "ThemeManager.h"

enum BANNER_PREFS_TYPES
{
	BANNERPREFS_FLAT=0,
	BANNERPREFS_ROT,
	BANNERPREFS_EZ2,
	BANNERPREFS_PUMP,
	BANNERPREFS_PARA
};

#define BANNER_WIDTH			THEME->GetMetricF("ScreenSelectMusic","BannerWidth")
#define BANNER_HEIGHT			THEME->GetMetricF("ScreenSelectMusic","BannerHeight")
#define EZ2_BANNER_WIDTH THEME->GetMetricF("ScreenSelectMusic","BannerWidth")
#define EZ2_BANNER_HEIGHT THEME->GetMetricF("ScreenSelectMusic","BannerHeight")
#define EZ2_BANNER_ZOOM 2.0

#define ZOOM_OFFSET THEME->GetMetricF("ScreenEz2SelectMusic","BannerZoomOffset")
#define FADE_OFFSET THEME->GetMetricF("ScreenEz2SelectMusic","BannerFadeOffset")
#define BANNER_ROTATION THEME->GetMetricF("ScreenEz2SelectMusic","BannerRotation")


#define SPRITE_TYPE_SPRITE 0
#define SPRITE_TYPE_CROPPEDSPRITE 1
#define ROT_ROTATION 315

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
	m_iBouncingState = 0;
	m_iBounceSize = 0;
	m_fNextTween = 0;
	m_iBannerPrefs = BANNERPREFS_EZ2;
	m_iSpriteType = SPRITE_TYPE_SPRITE;
	m_iSelection = 0;
	m_fSelectionLag = 0;
	m_iSpacing = DEFAULT_SPACING;
	m_iNumVisible = DEFAULT_VISIBLE_ELEMENTS;
	m_iBounceDir=0;
	m_iBounceWait=0;
	m_sprBannerMask.SetName( "Banner" );	// use the same metrics and animation as Banner
	m_sprBannerMask.Load( THEME->GetPathG("ScreenSelectMusic","banner mask") );
	m_sprBannerMask.SetClearZBuffer( true );
	m_sprBannerMask.SetBlendMode( BLEND_NO_EFFECT );	// don't draw to color buffer
	m_sprBannerMask.SetZWrite( true );	// do draw to the zbuffer
	m_sprBannerMask.SetWidth(EZ2_BANNER_WIDTH);
	m_sprBannerMask.SetHeight(EZ2_BANNER_HEIGHT);

	m_RippleCSprite.SetXY(0,0);
	m_RippleSprite.SetXY(0,0);
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

void ScrollingList::StartBouncing()
{
	m_iBouncingState = 1;
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		m_RippleSprite.UnloadTexture();
		m_RippleSprite.Load( m_apSprites[m_iSelection]->GetTexturePath() );
		m_RippleSprite.SetXY( m_apSprites[m_iSelection]->GetX(), m_apSprites[m_iSelection]->GetY() );
		m_RippleSprite.SetZoom( 1.1f );
		m_RippleSprite.SetDiffuse( RageColor(1,1,1,0.5f));
	}
	else
	{
		m_RippleCSprite.UnloadTexture();
		m_RippleCSprite.Load( m_apCSprites[m_iSelection]->GetTexturePath() );


		// ScaleToClipped should detect rotated banner files and correct
		// accordingly.  If there's a case I didn't think about, feel free
		// to change it back.  -Chris
//		if(m_RippleCSprite.GetUnzoomedWidth() == m_RippleCSprite.GetUnzoomedHeight()) // rotated graphics need cropping
//		{
//			m_RippleCSprite.ScaleToClipped( 100, 100 );
//		}
//		else // flat, unrotated graphics need widths changing
//		{
//			m_RippleCSprite.ScaleToClipped( -1, -1 ); // default image size.
//			m_RippleCSprite.SetWH(EZ2_BANNER_WIDTH+10, EZ2_BANNER_HEIGHT+10);
//		}
		m_RippleCSprite.ScaleToClipped( EZ2_BANNER_WIDTH+10, EZ2_BANNER_HEIGHT+10 );

		m_RippleCSprite.SetXY( m_apCSprites[m_iSelection]->GetX(), m_apCSprites[m_iSelection]->GetY() );
		m_RippleCSprite.SetZoom( 2.0f );
		m_RippleCSprite.SetDiffuse( RageColor(1,1,1,0.5f));
	}
}

void ScrollingList::StopBouncing()
{
	m_iBouncingState = 0;
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
		m_apSprites[m_iSelection]->SetZoom( 1.0f );
	else
		m_apCSprites[m_iSelection]->SetZoom( 1.0f );
}

/************************************
Allows us to create a graphic element
in the scrolling list
*************************************/
void ScrollingList::Load( const vector<RString>& asGraphicPaths )
{
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
			Sprite* pNewCSprite = new Sprite;
			pNewCSprite->Load( asGraphicPaths[i] );

			m_apCSprites.push_back( pNewCSprite );
			for(int i=m_apCSprites.size()-1; i>=0; i--)
				Replace(asGraphicPaths[i],i);
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
		if(m_iBouncingState)	// bouncing
		{
			if(m_fNextTween <= 0) // we're ready to update stuff
			{
				m_fNextTween = 0.1f; // reset the tween count
				if(m_apSprites[m_iSelection]->GetZoom() >= 1.2f && m_iBounceDir == 1) // if we're over biggest boundary
				{
					m_iBounceDir = 2; // next phase will be a wait
					m_apSprites[m_iSelection]->SetZoom( m_apSprites[m_iSelection]->GetZoom() - 0.25f); // make it smaller
					m_RippleSprite.SetZoom( m_RippleSprite.GetZoom() - 0.30f); // make the ripple smaller
				}
				else if(m_apSprites[m_iSelection]->GetZoom() <= 1.0f && m_iBounceDir == 0) // if we're over smallest boundary
				{
					m_iBounceDir = 1; // next phase will be making graphic bigger
					m_apSprites[m_iSelection]->SetZoom( m_apSprites[m_iSelection]->GetZoom() + 0.25f); // make it bigger
					m_RippleSprite.SetZoom( m_RippleSprite.GetZoom() + 0.30f); // make ripple bigger
					m_RippleSprite.SetDiffuse( RageColor(1,1,1,0.5f)); // make ripple appear semi transparent
				}
				else if(m_iBounceDir == 0 && m_apSprites[m_iSelection]->GetZoom() != 1.0f) // travelling smaller
				{
					m_apSprites[m_iSelection]->SetZoom( m_apSprites[m_iSelection]->GetZoom() - 0.25f); // make smaller
					m_RippleSprite.SetZoom( m_RippleSprite.GetZoom() - 0.30f); // make smaller
				}
				else if(m_iBounceDir == 1 && m_apSprites[m_iSelection]->GetZoom() != 1.2f) // travelling bigger
				{
					m_apSprites[m_iSelection]->SetZoom( m_apSprites[m_iSelection]->GetZoom() + 0.25f); // make bigger
					m_RippleSprite.SetZoom( m_RippleSprite.GetZoom() + 0.30f ); // make bigger
				}
				else if(m_iBounceDir == 2) // we're waiting before doing bounce processes again
				{
					if(m_iBounceWait == 0) // if we're at 0 from last time....
						m_iBounceWait = 3; // start wait at 3
					else
						m_iBounceWait--; // otherwise decrease by 1
					
					if(m_iBounceWait == 2) // if we're one moment after start of wait
						m_RippleSprite.SetDiffuse( RageColor(1,1,1,0.0f)); // hide the ripple

					if(m_iBounceWait == 0) // if we just turned to 0 
						m_iBounceDir = 0; // go to the 'make smaller' stage. as we SHOULD already be pretty small, we should start increasing in size a couple phases on.
				}
			}
			else
			{
				m_fNextTween -= fDeltaTime; // update the tween time.
			}

		}

		for( unsigned i=0; i<m_apSprites.size(); i++ )
			m_apSprites[i]->Update( fDeltaTime );
	}
	else
	{
		if(m_iBouncingState)	// bouncing
		{
			if(m_fNextTween <= 0) // we're ready to update stuff
			{
				m_fNextTween = 0.1f; // reset the tween count
								
				if(m_iBounceSize <= 0 && m_iBounceDir == 0 ) // going smaller
				{
					m_iBounceDir = 2;
				}
				else if(m_iBounceDir == 1 && m_iBounceSize >= 0.2f) // getting big
				{
					m_iBounceDir = 0;
				}
				else if(m_iBounceDir == 0)
				{
					m_iBounceSize-=0.2f;
				}
				else if(m_iBounceDir == 1)
				{
					m_iBounceSize+=0.2f;
				}
				else if(m_iBounceDir == 2) // we're waiting before doing bounce processes again
				{
					if(m_iBounceWait == 0) // if we're at 0 from last time....
						m_iBounceWait = 3; // start wait at 3
					else
						m_iBounceWait--; // otherwise decrease by 1
					
					if(m_iBounceWait == 2) // if we're one moment after start of wait
						m_RippleSprite.SetDiffuse( RageColor(1,1,1,0.0f)); // hide the ripple

					if(m_iBounceWait == 0) // if we just turned to 0 
						m_iBounceDir = 1;
				}
			}
			else
			{
				m_fNextTween -= fDeltaTime; // update the tween time.
			}

		}

		for( unsigned i=0; i<m_apCSprites.size(); i++ )
			m_apCSprites[i]->Update( fDeltaTime );
	}
}

void ScrollingList::Replace(RString sGraphicPath, int ElementNumber)
{
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		Sprite* pNewSprite = new Sprite;
		pNewSprite->Load( sGraphicPath );
		m_apSprites[ElementNumber] = pNewSprite;
	}
	else
	{
		Sprite* pNewCSprite = new Sprite;
		pNewCSprite->Load( sGraphicPath );
		if(m_iBannerPrefs == BANNERPREFS_FLAT)
		{
			// ScaleToClipped should detect rotated banner files and correct
			// accordingly.  If there's a case I didn't think about, feel free
			// to change it back.  -Chris
//			if(pNewCSprite->GetUnzoomedWidth() == pNewCSprite->GetUnzoomedHeight()) // rotated graphics need cropping
//			{
//				pNewCSprite->SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
//			}
//			else // flat, unrotated graphics need widths changing
//			{
//				pNewCSprite->SetWH(BANNER_WIDTH, BANNER_HEIGHT );
//			}
		
			pNewCSprite->ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
		}
		else if(m_iBannerPrefs == BANNERPREFS_ROT)
		{
			pNewCSprite->ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
			pNewCSprite->SetRotationZ( ROT_ROTATION );
		}
		else if(m_iBannerPrefs == BANNERPREFS_EZ2)
		{
			// ScaleToClipped should detect rotated banner files and correct
			// accordingly.  If there's a case I didn't think about, feel free
			// to change it back.  -Chris
//			if(pNewCSprite->GetUnzoomedWidth() == pNewCSprite->GetUnzoomedHeight()) // rotated graphics need cropping
//			{
//				pNewCSprite->ScaleToClipped( EZ2_BANNER_WIDTH, EZ2_BANNER_HEIGHT );
//			}
//			else // flat, unrotated graphics need widths changing
//			{
//				pNewCSprite->SetWH(EZ2_BANNER_WIDTH, EZ2_BANNER_HEIGHT);
//			}
			pNewCSprite->ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
		}
			
		m_apCSprites[ElementNumber] = pNewCSprite;
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
		m_RippleSprite.SetXY( m_apSprites[m_iSelection]->GetX(), m_apSprites[m_iSelection]->GetY() ); // keep the ripple sprites with the current selection
	}
	else
	{
		ASSERT( !m_apCSprites.empty() );
		m_RippleCSprite.SetXY( m_apCSprites[m_iSelection]->GetX(), m_apCSprites[m_iSelection]->GetY() ); // keep the ripple sprites with the current selection
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
			m_apCSprites[iIndexToDraw1]->SetRotationZ(BANNER_ROTATION);
			m_apCSprites[iIndexToDraw2]->SetRotationZ(BANNER_ROTATION);
			m_sprBannerMask.SetRotationZ(BANNER_ROTATION);
			if( i==0 )	// so we don't draw 0 twice
			{
				if(!m_iBouncingState)
					m_apCSprites[iIndexToDraw1]->SetZoom( 1.0f - (ZOOM_OFFSET * i) );
				m_apCSprites[iIndexToDraw1]->SetDiffuse( COLOR_SELECTED + RageColor(0,0,0,(1.0f - (FADE_OFFSET * i))) );
				m_apCSprites[iIndexToDraw1]->SetZTestMode( ZTEST_WRITE_ON_PASS );	// do have to pass the z test
				m_sprBannerMask.SetXY(m_apCSprites[iIndexToDraw1]->GetX(), m_apCSprites[iIndexToDraw1]->GetY());
				m_sprBannerMask.SetZoom( m_apCSprites[iIndexToDraw1]->GetZoom());
				m_sprBannerMask.Draw();
				m_apCSprites[iIndexToDraw1]->Draw();
				if(m_iBouncingState)	// bouncing
				{
					if(m_iBounceDir == 1)
					{
						m_apCSprites[iIndexToDraw1]->SetZoom(m_apCSprites[iIndexToDraw1]->GetZoom() + 0.05f );
						m_sprBannerMask.SetZoom( m_apCSprites[iIndexToDraw1]->GetZoom() );
						m_sprBannerMask.Draw();
						m_apCSprites[iIndexToDraw1]->Draw();
						m_apCSprites[iIndexToDraw1]->SetZoom(m_apCSprites[iIndexToDraw1]->GetZoom() - 0.05f );
						m_sprBannerMask.SetZoom( m_apCSprites[iIndexToDraw1]->GetZoom() );
					}
					m_apCSprites[iIndexToDraw1]->SetZoom(m_apCSprites[iIndexToDraw1]->GetZoom() + m_iBounceSize );
					RageColor currentcolor = m_apCSprites[iIndexToDraw1]->GetDiffuse();
					m_sprBannerMask.SetZoom( m_sprBannerMask.GetZoom() + m_iBounceSize );
					m_apCSprites[iIndexToDraw1]->SetDiffuse( RageColor( 1, 1, 1, 0.4f ) );
					m_sprBannerMask.Draw();
					m_apCSprites[iIndexToDraw1]->Draw(); // again for the bounce effect
					m_apCSprites[iIndexToDraw1]->SetZoom(m_apCSprites[iIndexToDraw1]->GetZoom() - m_iBounceSize );
					m_sprBannerMask.SetZoom( m_sprBannerMask.GetZoom() - m_iBounceSize );
					m_apCSprites[iIndexToDraw1]->SetDiffuse( currentcolor );
				}
			}
			else
			{
				m_apCSprites[iIndexToDraw1]->SetZoom( 1.0f - (ZOOM_OFFSET * i) );
				m_apCSprites[iIndexToDraw2]->SetZoom( 1.0f - (ZOOM_OFFSET * i) );
				m_apCSprites[iIndexToDraw1]->SetDiffuse( COLOR_NOT_SELECTED + RageColor(0,0,0,(- (FADE_OFFSET * i))) );
				m_apCSprites[iIndexToDraw2]->SetDiffuse( COLOR_NOT_SELECTED + RageColor(0,0,0,(- (FADE_OFFSET * i))) );
				m_apCSprites[iIndexToDraw1]->SetZTestMode( ZTEST_WRITE_ON_PASS );	// do have to pass the z test
				m_sprBannerMask.SetXY(m_apCSprites[iIndexToDraw1]->GetX(), m_apCSprites[iIndexToDraw1]->GetY());
				m_sprBannerMask.SetZoom( m_apCSprites[iIndexToDraw1]->GetZoom());
				m_sprBannerMask.Draw();
				m_apCSprites[iIndexToDraw1]->Draw();
				m_apCSprites[iIndexToDraw2]->SetZTestMode( ZTEST_WRITE_ON_PASS );	// do have to pass the z test
				m_sprBannerMask.SetXY(m_apCSprites[iIndexToDraw2]->GetX(), m_apCSprites[iIndexToDraw1]->GetY());
				m_sprBannerMask.SetZoom( m_apCSprites[iIndexToDraw2]->GetZoom());
				m_sprBannerMask.Draw();
				m_apCSprites[iIndexToDraw2]->Draw();
			}
		}
	}
	if(m_iSpriteType == SPRITE_TYPE_SPRITE)
	{
		if(m_iBouncingState)
		{
			m_RippleSprite.Draw();
		}
	}
}

/*
 * (c) 2001-2003 "Frieza"
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
