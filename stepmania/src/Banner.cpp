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



bool Banner::LoadFromSong( Song &song )
{
	Sprite::LoadFromTexture( song.GetBannerPath() );
	//Sprite::TurnShadowOff();

	float fSourceWidth		= (float)m_pTexture->GetSourceWidth();
	float fSourceHeight		= (float)m_pTexture->GetSourceHeight();

	// first find the correct zoom
	Sprite::ScaleToCover( CRect(0, 0,
			                    BANNER_WIDTH,
								BANNER_HEIGHT )
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


	return true;
}
