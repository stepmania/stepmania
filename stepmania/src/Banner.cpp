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
#include "RageBitmapTexture.h"



bool Banner::LoadFromSong( Song* pSong )		// NULL means no song
{
	Sprite::TurnShadowOff();

	if( pSong == NULL )					Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER), TEXTURE_HINT_NOMIPMAPS );
	else if( pSong->HasBanner() )		Banner::Load( pSong->GetBannerPath(), TEXTURE_HINT_NOMIPMAPS );
	else if( pSong->HasBackground() )	Banner::Load( pSong->GetBackgroundPath(), TEXTURE_HINT_NOMIPMAPS );
	else								Banner::Load( THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER), TEXTURE_HINT_NOMIPMAPS );

	return true;
}

bool Banner::Load( const CString &sFilePath, DWORD dwHints, bool bForceReload )
{
	Sprite::Load( sFilePath, dwHints, bForceReload );
	CropToRightSize();

	return true;
}

void Banner::CropToRightSize()
{
	int iSourceWidth	= m_pTexture->GetSourceWidth();
	int iSourceHeight	= m_pTexture->GetSourceHeight();

	// save the original X&Y.  We're going to resore them later.
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	if( iSourceWidth == iSourceHeight )		// this is a SSR/DWI banner
	{
		float fCustomImageCoords[8] = {
			0.22f,	0.98f,	// bottom left
			0.02f,	0.78f,	// top left
			0.98f,	0.22f,	// bottom right
			0.78f,	0.02f,	// top right
		};
		Sprite::SetCustomImageCoords( fCustomImageCoords );
		
		SetWidth(  BANNER_WIDTH );
		SetHeight( BANNER_HEIGHT );
		SetZoom( 1 );
	}
	else if( iSourceWidth * 2 > iSourceHeight )
	{
		Sprite::StopUsingCustomCoords();
		Sprite::StretchTo( CRect(0, 0,
									(int)BANNER_WIDTH,
									(int)BANNER_HEIGHT )
							 );		
	}
	else	// this is probably a background graphic or something not intended to be a banner
	{
		Sprite::StopUsingCustomCoords();

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
}
