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



BOOL Banner::LoadFromSong( Song &song )
{
	BOOL bResult = Sprite::LoadFromTexture( song.GetBannerPath() );
	if( bResult )
	{
		RECT r;
		int iImageWidth = this->GetZoomedWidth();
		int iImageHeight = this->GetZoomedHeight();

		// first find the correct zoom
		::SetRect( &r, 0, 0, BANNER_WIDTH, BANNER_HEIGHT );
		Sprite::ScaleToCover( &r );
		FLOAT fFinalZoom = this->GetZoom();

		// find which dimension is larger
		BOOL bYDimIsLarger = this->GetZoomedHeight() > BANNER_HEIGHT;
		
		// now crop it
		if( !bYDimIsLarger )	// crop X
		{
			int iScreenPixelsToCrop = (this->GetZoomedWidth() - BANNER_WIDTH) / 2;
			int iImagePixelsToCrop  = roundf( iScreenPixelsToCrop / fFinalZoom );
			RECT rectNewSrc;
			::SetRect( &rectNewSrc, iImagePixelsToCrop, 
									0, 
									iImageWidth - iImagePixelsToCrop, 
									iImageHeight );
			Sprite::SetCustomSrcRect( rectNewSrc );
		}
		else		// crop Y
		{
			int iScreenPixelsToCrop = (this->GetZoomedHeight() - BANNER_HEIGHT) / 2;
			int iImagePixelsToCrop  = roundf( iScreenPixelsToCrop / fFinalZoom );
			RECT rectNewSrc;
			::SetRect( &rectNewSrc, 0, 
									iImagePixelsToCrop, 
									iImageWidth, 
									iImageHeight - iImagePixelsToCrop );
			Sprite::SetCustomSrcRect( rectNewSrc );
		}
	}

	return TRUE;
}
