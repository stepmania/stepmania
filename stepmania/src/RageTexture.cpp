#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageTexture.h

 Desc: Abstract class for a texture with metadata.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

 
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageTexture.h"
#include "RageUtil.h"
#include <string.h>


//-----------------------------------------------------------------------------
// RageTexture constructor
//-----------------------------------------------------------------------------
RageTexture::RageTexture(
	RageDisplay* pScreen, 
	const CString &sFilePath, 
	DWORD dwMaxSize, 
	DWORD dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch )
{
//	LOG->Trace( "RageTexture::RageTexture()" );

	// save a pointer to the D3D device
	m_pd3dDevice = pScreen->m_pd3dDevice;
	assert( m_pd3dDevice != NULL );

	// save the file path
	m_sFilePath = sFilePath;
//	m_pd3dTexture = NULL;
	m_iRefCount = 1;

	m_iSourceWidth = m_iSourceHeight = 0;
	m_iTextureWidth = m_iTextureHeight = 0;
	m_iImageWidth = m_iImageHeight = 0;
	m_iFramesWide = m_iFramesHigh = 1;
}

RageTexture::~RageTexture()
{

}


void RageTexture::CreateFrameRects()
{
	GetFrameDimensionsFromFileName( m_sFilePath, &m_iFramesWide, &m_iFramesHigh );

	///////////////////////////////////
	// Fill in the m_FrameRects with the bounds of each frame in the animation.
	///////////////////////////////////
	m_TextureCoordRects.SetSize( 1, 10 );	// most textures will have only one frame
	m_TextureCoordRects.RemoveAll();

	for( int j=0; j<m_iFramesHigh; j++ )		// traverse along Y
	{
		for( int i=0; i<m_iFramesWide; i++ )	// traverse along X (important that this is the inner loop)
		{
			FRECT frect( (i+0)/(float)m_iFramesWide*m_iImageWidth /(float)m_iTextureWidth,	// these will all be between 0.0 and 1.0
						 (j+0)/(float)m_iFramesHigh*m_iImageHeight/(float)m_iTextureHeight, 
						 (i+1)/(float)m_iFramesWide*m_iImageWidth /(float)m_iTextureWidth, 
						 (j+1)/(float)m_iFramesHigh*m_iImageHeight/(float)m_iTextureHeight );
			m_TextureCoordRects.Add( frect );	// the index of this array element will be (i + j*m_iFramesWide)
			
			//LOG->Trace( "Adding frect%d %f %f %f %f", (i + j*m_iFramesWide), frect.left, frect.top, frect.right, frect.bottom );
		}
	}
}

void RageTexture::GetFrameDimensionsFromFileName( CString sPath, int* piFramesWide, int* piFramesHigh ) const
{
	*piFramesWide = *piFramesHigh = 1;	// set default values in case we don't find the dimension in the file name

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

		*piFramesWide = atoi(arrayDimensionsBits[0]);
		*piFramesHigh = atoi(arrayDimensionsBits[1]);
		return;
	}

}

void RageTexture::TurnLoopOn()
{
}

void RageTexture::TurnLoopOff()
{
}
