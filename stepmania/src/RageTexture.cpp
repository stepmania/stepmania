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
	if( sPath.Find("max300.png") != -1 )
		int kljds = 3;

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


	/*
	//////////////////////////////////////////////////
	// Parse m_sFilePath for the frame dimensions
	//
	// The file name must look like "name 4x15.gif".  The the space is required.
	//////////////////////////////////////////////////
	CString sDimensionsString = sPath;	// we will chop this down to the "4x15" part.  Need regular... expressions... choke :-)

	// chop off the extension
	int index_of_last_period = sDimensionsString.ReverseFind( '.' );
	if( index_of_last_period == -1 )	// this file name has no extension, but it the texture was loaded.  I doubt this code will ever execute :-)
	{
		*puFramesWide = *puFramesHigh = 1;  
		return;
	}
	sDimensionsString = sDimensionsString.Left(index_of_last_period);

	// chop off everything before and including the last space
	int index_of_last_space = sDimensionsString.ReverseFind( ' ' );
	if( index_of_last_space == -1 )	// this file name has space, so the dimensions tag cannot be read
	{
		*puFramesWide = *puFramesHigh = 1;  
		return;
	}
	sDimensionsString.Delete( 0, index_of_last_space+1 );

	// now we are left with "4x15"
	int index_of_x = sDimensionsString.Find( 'x' );
	if( index_of_x == -1 )	// this file name doesn't have an x in the last token.  So, this probably isn't a dimension tag.
	{
		*puFramesWide = *puFramesHigh = 1;  
		return;
	}
	*puFramesWide = (UINT) atoi( sDimensionsString.Left( index_of_x ) );

	// chop off the frames wide part and read in the frames high
	sDimensionsString.Delete( 0, index_of_x+1 );
	*puFramesHigh = (UINT) atoi( sDimensionsString );
	
	if( *puFramesWide == 0 )	*puFramesWide = 1;
	if( *puFramesHigh == 0 )	*puFramesHigh = 1;
	*/
}