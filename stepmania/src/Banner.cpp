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


const CString TEXTURE_FALLBACK_BANNER	=	"Textures\\Fallback Banner.png";


bool Banner::LoadFromSong( Song &song )
{
	if( song.HasBanner() )
		Sprite::LoadFromTexture( song.GetBannerPath() );
	else if( song.HasBackground() && !song.BackgroundIsAMovie() )
		Sprite::LoadFromTexture( song.GetBackgroundPath() );
	else
		Sprite::LoadFromTexture( TEXTURE_FALLBACK_BANNER );
	//Sprite::TurnShadowOff();

	int iSourceWidth	= m_pTexture->GetSourceWidth();
	int iSourceHeight	= m_pTexture->GetSourceHeight();


	if( iSourceWidth == iSourceHeight )	// this is a SSR/DWI style banner
	{
		float fCustomTexCoords[8] = {
			0.22f,	0.98f,	// bottom left
			0.02f,	0.78f,	// top left
			0.98f,	0.22f,	// bottom right
			0.78f,	0.02f,	// top right
		};
		Sprite::SetCustomTexCoords( fCustomTexCoords );
		
		SetWidth(  BANNER_WIDTH );
		SetHeight( BANNER_HEIGHT );

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
			FRECT frectNewSrc( fPercentageToCutOffEachSide, 
							   0, 
							   1 - fPercentageToCutOffEachSide, 
							   1 );
			Sprite::SetCustomSrcRect( frectNewSrc );
		}
		else		// crop Y
		{
			float fPercentageToCutOff = (this->GetZoomedHeight() - BANNER_HEIGHT) / this->GetZoomedHeight();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;
			
			// generate a rectangle with new texture coordinates
			FRECT frectNewSrc( 0, 
							   fPercentageToCutOffEachSide,
							   1, 
							   1 - fPercentageToCutOffEachSide );
			Sprite::SetCustomSrcRect( frectNewSrc );
		}
		SetWidth(  BANNER_WIDTH );
		SetHeight( BANNER_HEIGHT );
		SetZoom( 1 );
	}

	return true;
}
