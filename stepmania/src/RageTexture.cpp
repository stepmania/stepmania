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
RageTexture::RageTexture( LPRageScreen pScreen, CString sFilePath ) :
  m_uSourceWidth( 0 ),
  m_uSourceHeight( 0 ),
  m_uTextureWidth( 0 ),
  m_uTextureHeight( 0 ),
  m_uFramesWide( 1 ),
  m_uFramesHigh( 1 ),
  m_uSourceFrameWidth( 0 ),
  m_uSourceFrameHeight( 0 )
{
//	RageLog( "RageTexture::RageTexture()" );

	// save a pointer to the D3D device
	m_pd3dDevice = pScreen->GetDevice();
	assert( m_pd3dDevice != NULL );

	// save the file path
	m_sFilePath = sFilePath;
//	m_pd3dTexture = NULL;
	m_iRefCount = 1;


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
			FRECT frect( (i+0)/(float)m_uFramesWide,	// these will all be between 0.0 and 1.0
						 (j+0)/(float)m_uFramesHigh, 
						 (i+1)/(float)m_uFramesWide, 
						 (j+1)/(float)m_uFramesHigh );
			m_TextureCoordRects.Add( frect );	// the index of this array element will be (i + j*m_uFramesWide)
			
			//RageLog( "Adding frect%d %f %f %f %f", (i + j*m_uFramesWide), frect.left, frect.top, frect.right, frect.bottom );
		}
	}
}

#include "string.h"
void RageTexture::GetFrameDimensionsFromFileName( CString sPath, UINT* puFramesWide, UINT* puFramesHigh ) const
{
	*puFramesWide = *puFramesHigh = 1;	// initialize in case we don't find dimensions in the file name

	sPath.MakeLower();

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt);

	CStringArray sFileNameBits;
	split( sFName, " ", sFileNameBits, false );

	if( sFileNameBits.GetSize() < 2 )
		return;		// there can't be dimensions in the file name if there's no space

	CString sDimensionsPart = sFileNameBits[ sFileNameBits.GetSize()-1 ];	// looks like "10x5"

	CStringArray sDimensionsBits;
	split( sDimensionsPart, "x", sDimensionsBits, false );

	if( sDimensionsBits.GetSize() != 2 )
	{
		return;
	}
	else
	{
		if( !IsAnInt(sDimensionsBits[0]) || !IsAnInt(sDimensionsBits[1]) )
			return;

		*puFramesWide = atoi(sDimensionsBits[0]);
		*puFramesHigh = atoi(sDimensionsBits[1]);
		return;
	}

}