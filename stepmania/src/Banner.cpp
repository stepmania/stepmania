#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "Banner.h"
#include "ThemeManager.h"




bool Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	if( pSong == NULL )
		Sprite::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );
	else if( pSong->HasBanner() )
		Sprite::Load( pSong->GetBannerPath() );
	else if( pSong->HasBackground() )
		Sprite::Load( pSong->GetBackgroundPath() );
	else
		Sprite::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER) );

	Sprite::TurnShadowOff();

	int iImageWidth		= m_pTexture->GetImageWidth();
	int iImageHeight	= m_pTexture->GetImageHeight();

	// save the original X&Y.  We're going to resore them later.
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	if( iImageWidth == iImageHeight )		// this is a SSR/DWI style banner
	{
		float fCustomImageCoords[8] = {
			0.22f,	0.98f,	// bottom left
			0.02f,	0.78f,	// top left
			0.98f,	0.22f,	// bottom right
			0.78f,	0.02f,	// top right
		};
		SetCustomImageCoords( fCustomImageCoords );
		
		SetWidth(  BANNER_WIDTH );
		SetHeight( BANNER_HEIGHT );
		SetZoom( 1 );
	}
	else	// this is a normal sized banner
	{

		// first find the correct zoom
		Sprite::ScaleToCover( CRect(0, 0,
									(int)BANNER_WIDTH,
									(int)BANNER_HEIGHT )
							 );
		float fFinalZoom = this->GetZoom();
		
		// find which dimension is larger
		bool bXDimNeedsToBeCropped = GetZoomedWidth() > BANNER_WIDTH;
		
		if( bXDimNeedsToBeCropped )	// crop X
		{
			float fPercentageToCutOff = (this->GetZoomedWidth() - BANNER_WIDTH) / this->GetZoomedWidth();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;
			
			// generate a rectangle with new texture coordinates
			FRECT fCustomImageCoords( 
				fPercentageToCutOffEachSide, 
				0, 
				1 - fPercentageToCutOffEachSide, 
				1 );
			SetCustomImageRect( fCustomImageCoords );
		}
		else		// crop Y
		{
			float fPercentageToCutOff = (this->GetZoomedHeight() - BANNER_HEIGHT) / this->GetZoomedHeight();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;
			
			// generate a rectangle with new texture coordinates
			FRECT fCustomImageCoords( 
				0, 
				fPercentageToCutOffEachSide,
				1, 
				1 - fPercentageToCutOffEachSide );
			SetCustomImageRect( fCustomImageCoords );
		}
		SetWidth(  BANNER_WIDTH );
		SetHeight( BANNER_HEIGHT );
		SetZoom( 1 );
	}

	// restore original XY
	SetXY( fOriginalX, fOriginalY );


	return true;
}
