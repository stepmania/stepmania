#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageTexture.h

 Desc: Abstract class for a texture with metadata.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

 
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageTexture.h"
#include "RageUtil.h"


//-----------------------------------------------------------------------------
// RageTexture constructor
//-----------------------------------------------------------------------------
RageTexture::RageTexture( LPRageScreen pScreen, CString sFilePath )
{
//	RageLog( "RageTexture::RageTexture()" );

	// save a pointer to the D3D device
	m_pd3dDevice = pScreen->GetDevice();
	assert( m_pd3dDevice != NULL );

	// save the file path
	m_sFilePath = sFilePath;
//	m_pd3dTexture = NULL;
	m_iRefCount = 1;

	m_uSourceWidth = m_uSourceHeight = 0;
	m_uTextureWidth = m_uTextureHeight = 0;
	m_uImageWidth = m_uImageHeight = 0;
	m_uFramesWide = m_uFramesHigh = 1;
}

RageTexture::~RageTexture()
{

}


void RageTexture::CreateFrameRects()
{
	GetFrameDimensionsFromFileName( m_sFilePath, &m_uFramesWide, &m_uFramesHigh );

	///////////////////////////////////
	// Fill in the m_FrameRects with the bounds of each frame in the animation.
	///////////////////////////////////
	for( UINT j=0; j<m_uFramesHigh; j++ )		// traverse along Y
	{
		for( UINT i=0; i<m_uFramesWide; i++ )	// traverse along X (important that this is the inner loop)
		{
			FRECT frect( (i+0)/(float)m_uFramesWide*m_uImageWidth /m_uTextureWidth,	// these will all be between 0.0 and 1.0
						 (j+0)/(float)m_uFramesHigh*m_uImageHeight/m_uTextureHeight, 
						 (i+1)/(float)m_uFramesWide*m_uImageWidth /m_uTextureWidth, 
						 (j+1)/(float)m_uFramesHigh*m_uImageHeight/m_uTextureHeight );
			m_TextureCoordRects.Add( frect );	// the index of this array element will be (i + j*m_uFramesWide)
			
			//RageLog( "Adding frect%d %f %f %f %f", (i + j*m_uFramesWide), frect.left, frect.top, frect.right, frect.bottom );
		}
	}
}

#include "string.h"
void RageTexture::GetFrameDimensionsFromFileName( CString sPath, UINT* puFramesWide, UINT* puFramesHigh ) const
{
	*puFramesWide = *puFramesHigh = 1;	// set default values in case we don't find the dimension in the file name

	sPath.MakeLower();

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt);

	CStringArray arrayBits;
	split( sFName, " ", arrayBits, false );

	for( int i=0; i<arrayBits.GetSize(); i++ )
	{
		CString &sBit = arrayBits[ i ];	
		
		// Test to see if it looks like "%ux%u" (e.g. 16x8)

		CStringArray arrayDimensionsBits;
		split( sBit, "x", arrayDimensionsBits, false );

		if( arrayDimensionsBits.GetSize() != 2 )
			continue;
		else if( !IsAnInt(arrayDimensionsBits[0]) || !IsAnInt(arrayDimensionsBits[1]) )
			continue;

		*puFramesWide = atoi(arrayDimensionsBits[0]);
		*puFramesHigh = atoi(arrayDimensionsBits[1]);
		return;
	}

}