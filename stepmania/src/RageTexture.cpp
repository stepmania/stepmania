#include "global.h"
/*
-----------------------------------------------------------------------------
 File: RageTexture.h

 Desc: Abstract class for a texture with metadata.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageTexture.h"
#include "RageUtil.h"
#include "RageTextureManager.h"
#include <string.h>

void RageTextureID::Init()
{
	/* Maximum size of the texture, per dimension. */
	iMaxSize = 2048;

	/* Number of mipmaps. (unimplemented) */
	iMipMaps = 4;

	/* Maximum number of bits for alpha.  In 16-bit modes, lowering
	 * this gives more bits for color values. (0, 1 or 4) */
	iAlphaBits = 4;

	/* If true and color precision is being lost, dither. (slow) */
	bDither = false;
	/* If true, resize the image to fill the internal texture. (slow) */
	bStretch = false;

	/* Preferred color depth of the image.  (This is overridden for
	 * paletted images and transparencies.) */
   	iColorDepth = -1; /* default */

	/* If true, enable HOT PINK color keying. (deprecated but needed for
	 * banners) */
	bHotPinkColorKey = false;

	/* This indicates that the image is a transparency.  This allows for
	 * high-resolution, memory-compact transparent layers.  The diffuse
	 * color will be used with no texture color.  (0, 4, 8) */
	iTransparencyOnly = 0;
}

bool RageTextureID::operator<(const RageTextureID &rhs) const
{
#define COMP(a) if(a<rhs.a) return true; if(a>rhs.a) return false;
	COMP(filename);
	COMP(iMaxSize);
	COMP(iMipMaps);
	COMP(iAlphaBits);
	COMP(iColorDepth);
	COMP(iTransparencyOnly);
	COMP(bDither);
	COMP(bStretch);
	COMP(bHotPinkColorKey);
#undef COMP
	return false;
}

bool RageTextureID::operator==(const RageTextureID &rhs) const
{
#define EQUAL(a) (a==rhs.a)
	return 
		EQUAL(filename) &&
		EQUAL(iMaxSize) &&
		EQUAL(iMipMaps) &&
		EQUAL(iAlphaBits) &&
		EQUAL(iColorDepth) &&
		EQUAL(iTransparencyOnly) &&
		EQUAL(bDither) &&
		EQUAL(bStretch) &&
		EQUAL(bHotPinkColorKey);
}


RageTexture::RageTexture( RageTextureID name ):
	m_ID(name)
{
//	LOG->Trace( "RageTexture::RageTexture()" );

	m_iRefCount = 1;
	m_Policy = TEX_DEFAULT;
	m_bWasUsed = false;

//	SetActualID();
	m_iSourceWidth = m_iSourceHeight = 0;
	m_iTextureWidth = m_iTextureHeight = 0;
	m_iImageWidth = m_iImageHeight = 0;
	m_iFramesWide = m_iFramesHigh = 0;
}


/* Set the initial ActualID; this is what the actual texture will start
 * from. */
//void RageTexture::SetActualID()
//{
//	m_ActualID = m_ID;
//
//	/* Texture color depth preference can be overridden. */
//	if(m_ID.iColorDepth == -1)
//		m_ActualID.iColorDepth = TEXTUREMAN->GetTextureColorDepth();
//
//	/* The max texture size can never be higher than the preference,
//	 * since it might be set to something to fix driver problems. */
//	m_ActualID.iMaxSize = min(m_ActualID.iMaxSize, TEXTUREMAN->GetMaxTextureResolution());
//}

RageTexture::~RageTexture()
{

}


void RageTexture::CreateFrameRects()
{
	GetFrameDimensionsFromFileName( GetID().filename, &m_iFramesWide, &m_iFramesHigh );

	///////////////////////////////////
	// Fill in the m_FrameRects with the bounds of each frame in the animation.
	///////////////////////////////////
	m_TextureCoordRects.clear();

	for( int j=0; j<m_iFramesHigh; j++ )		// traverse along Y
	{
		for( int i=0; i<m_iFramesWide; i++ )	// traverse along X (important that this is the inner loop)
		{
			RectF frect( (i+0)/(float)m_iFramesWide*m_iImageWidth /(float)m_iTextureWidth,	// these will all be between 0.0 and 1.0
						 (j+0)/(float)m_iFramesHigh*m_iImageHeight/(float)m_iTextureHeight, 
						 (i+1)/(float)m_iFramesWide*m_iImageWidth /(float)m_iTextureWidth, 
						 (j+1)/(float)m_iFramesHigh*m_iImageHeight/(float)m_iTextureHeight );
			m_TextureCoordRects.push_back( frect );	// the index of this array element will be (i + j*m_iFramesWide)
			
			//LOG->Trace( "Adding frect%d %f %f %f %f", (i + j*m_iFramesWide), frect.left, frect.top, frect.right, frect.bottom );
		}
	}
}

void RageTexture::GetFrameDimensionsFromFileName( CString sPath, int* piFramesWide, int* piFramesHigh )
{
	*piFramesWide = *piFramesHigh = 1;	// set default values in case we don't find the dimension in the file name

	sPath.MakeLower();

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt);

	CStringArray arrayBits;
	split( sFName, " ", arrayBits, false );

	/* XXX: allow dims to be in parens */
	for( unsigned i=0; i<arrayBits.size(); i++ )
	{
		CString &sBit = arrayBits[ i ];	
		
		// Test to see if it looks like "%ux%u" (e.g. 16x8)

		CStringArray arrayDimensionsBits;
		split( sBit, "x", arrayDimensionsBits, false );

		if( arrayDimensionsBits.size() != 2 )
			continue;
		else if( !IsAnInt(arrayDimensionsBits[0]) || !IsAnInt(arrayDimensionsBits[1]) )
			continue;

		*piFramesWide = atoi(arrayDimensionsBits[0]);
		*piFramesHigh = atoi(arrayDimensionsBits[1]);
		return;
	}

}

int RageTexture::GetFrameCountFromFileName( CString sPath )
{
	int wide, high;
	GetFrameDimensionsFromFileName(sPath, &wide, &high );
	return wide*high;
}

const RectF *RageTexture::GetTextureCoordRect( int frameNo ) const
{
	return &m_TextureCoordRects[frameNo];
}

