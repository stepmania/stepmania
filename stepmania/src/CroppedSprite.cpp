#include "global.h"
/*
-----------------------------------------------------------------------------
 File: CroppedSprite.h

 Desc: The song's CroppedSprite displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "CroppedSprite.h"
#include "PrefsManager.h"
#include "RageBitmapTexture.h"

#include <math.h>

CroppedSprite::CroppedSprite()
{
	/* By default, crop to the size of the image. */
	m_fCropWidth = m_fCropHeight = -1;
}

bool CroppedSprite::Load( RageTextureID ID )
{
	Sprite::Load( ID );
	CropToSize( m_fCropWidth, m_fCropHeight );
	
	return true;
}

void CroppedSprite::SetWH(float fWidth, float fHeight)
{
	Sprite::SetWidth(fWidth);
	Sprite::SetHeight(fHeight);
}

void CroppedSprite::SetCroppedSize( float fWidth, float fHeight )
{
	m_fCropWidth = fWidth;
	m_fCropHeight = fHeight;
}

void CroppedSprite::CropToSize( float fWidth, float fHeight )
{
	m_fCropWidth = fWidth;
	m_fCropHeight = fHeight;


	int iSourceWidth	= m_pTexture->GetSourceWidth();
	int iSourceHeight	= m_pTexture->GetSourceHeight();

	// save the original X&Y.  We're going to resore them later.
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	// TODO: Add this back in
//	if( IsDiagonalBanner(iSourceWidth, iSourceHeight) )		// this is a SSR/DWI CroppedSprite
//	{
//		float fCustomImageCoords[8] = {
//			0.02f,	0.78f,	// top left
//			0.22f,	0.98f,	// bottom left
//			0.98f,	0.22f,	// bottom right
//			0.78f,	0.02f,	// top right
//		};
//		Sprite::SetCustomImageCoords( fCustomImageCoords );
//
//		if( m_fCropWidth != -1 && m_fCropHeight != -1)
//			m_size = RageVector2( m_fCropWidth, m_fCropHeight );
//		else
//		{
//			/* If no crop size is set, then we're only being used to crop diagonal
//			 * banners so they look like regular ones. We don't actually care about
//			 * the size of the image, only that it has an aspect ratio of 4:1.  */
//			m_size = RageVector2(256, 64);
//		}
//		SetZoom( 1 );
//	}
//	else 
	if( m_pTexture->GetID().filename.find( "(was rotated)" ) != m_pTexture->GetID().filename.npos && 
			 m_fCropWidth != -1 && m_fCropHeight != -1 )
	{
		/* Dumb hack.  Normally, we crop all sprites except for diagonal banners,
		 * which are stretched.  Low-res versions of banners need to do the same
		 * thing as their full resolution counterpart, so the crossfade looks right.
		 * However, low-res diagonal banners are un-rotated, to save space.  BannerCache
		 * drops the above text into the "filename" (which is otherwise unused for
		 * these banners) to tell us this.
		 */
		Sprite::StopUsingCustomCoords();
		m_size = RageVector2( m_fCropWidth, m_fCropHeight );
		SetZoom( 1 );
	}
	else if( m_fCropWidth != -1 && m_fCropHeight != -1 )
	{
		// this is probably a background graphic or something not intended to be a CroppedSprite
		Sprite::StopUsingCustomCoords();

		// first find the correct zoom
		Sprite::ScaleToCover( RectI(0, 0,
									(int)m_fCropWidth,
									(int)m_fCropHeight )
							 );
		// find which dimension is larger
		bool bXDimNeedsToBeCropped = GetZoomedWidth() > m_fCropWidth+0.01;
		
		if( bXDimNeedsToBeCropped )	// crop X
		{
			float fPercentageToCutOff = (this->GetZoomedWidth() - m_fCropWidth) / this->GetZoomedWidth();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;
			
			// generate a rectangle with new texture coordinates
			RectF fCustomImageCoords( 
				fPercentageToCutOffEachSide, 
				0, 
				1 - fPercentageToCutOffEachSide, 
				1 );
			SetCustomImageCoords( fCustomImageCoords );
		}
		else		// crop Y
		{
			float fPercentageToCutOff = (this->GetZoomedHeight() - m_fCropHeight) / this->GetZoomedHeight();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;
			
			// generate a rectangle with new texture coordinates
			RectF fCustomImageCoords( 
				0, 
				fPercentageToCutOffEachSide,
				1, 
				1 - fPercentageToCutOffEachSide );
			SetCustomImageCoords( fCustomImageCoords );
		}
		m_size = RageVector2( m_fCropWidth, m_fCropHeight );
		SetZoom( 1 );
	}

	// restore original XY
	SetXY( fOriginalX, fOriginalY );
}

bool CroppedSprite::IsDiagonalBanner( int iWidth, int iHeight )
{
	/* A diagonal banner is a square.  Give a couple pixels of leeway. */
	return iWidth >= 100 && abs(iWidth - iHeight) < 2;
}

