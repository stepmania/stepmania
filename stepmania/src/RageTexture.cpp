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
  m_uImageWidth( 0 ),
  m_uImageHeight( 0 ),
  m_uTextureWidth( 0 ),
  m_uTextureHeight( 0 ),
  m_uFramesWide( 1 ),
  m_uFramesHigh( 1 ),
  m_uFrameWidth( 0 ),
  m_uFrameHeight( 0 )
{
//	RageLog( "RageTexture::RageTexture()" );

	// save a pointer to the D3D device
	m_pd3dDevice = pScreen->GetDevice();
	assert( m_pd3dDevice != NULL );

	// save the file path
	m_sFilePath = sFilePath;
	m_pd3dTexture = NULL;
	m_iRefCount = 1;


}

RageTexture::~RageTexture()
{

}


VOID RageTexture::CreateFrameRects()
{
	GetFrameDimensionsFromFileName( m_sFilePath, m_uFramesWide, m_uFramesHigh );

	///////////////////////////////////
	// Fill in the m_FrameRects with the bounds of each frame in the animation.
	///////////////////////////////////

	// calculate the width and height of the frames
	m_uFrameWidth = GetImageWidth() / m_uFramesWide;
	m_uFrameHeight= GetImageHeight() / m_uFramesHigh;

	for( UINT j=0; j<m_uFramesHigh; j++ )
	{
		for( UINT i=0; i<m_uFramesWide; i++ )
		{
			RECT rectCurFrame;
			::SetRect( &rectCurFrame, (i+0)*m_uFrameWidth, (j+0)*m_uFrameHeight, 
								      (i+1)*m_uFrameWidth, (j+1)*m_uFrameHeight );
			m_FrameRects.Add( rectCurFrame );	// the index of this array element will be (i + j*m_uFramesWide)
			
//			RageLog( "Adding frame#%d: %d %d %d %d", (i + j*m_uFramesWide), rectCurFrame.left, rectCurFrame.top, rectCurFrame.right, rectCurFrame.bottom );
		}
	}
}

#include "string.h"
VOID RageTexture::GetFrameDimensionsFromFileName( CString sPath, UINT &uFramesWide, UINT &uFramesHigh ) const
{
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
		uFramesWide = uFramesHigh = 1;  
		return;
	}
	sDimensionsString = sDimensionsString.Left(index_of_last_period);

	// chop off everything before the last space
	int index_of_last_space = sDimensionsString.ReverseFind( ' ' );
	if( index_of_last_space == -1 )	// this file name has space, so the dimensions tag cannot be read
	{
		uFramesWide = uFramesHigh = 1;  
		return;
	}
	sDimensionsString.Delete( 0, index_of_last_space+1 );

	// now we are left with "4x15"
	int index_of_x = sDimensionsString.Find( 'x' );
	if( index_of_x == -1 )	// this file name doesn't have an x in the last token.  So, this probably isn't a dimension tag.
	{
		uFramesWide = uFramesHigh = 1;  
		return;
	}
	uFramesWide = (UINT) atoi( sDimensionsString.Left( index_of_x ) );

	// chop off the frames wide part and read in the frames high
	sDimensionsString.Delete( 0, index_of_x+1 );
	uFramesHigh = (UINT) atoi( sDimensionsString );
	
	if( uFramesWide == 0 )	uFramesWide = 1;
	if( uFramesHigh == 0 )	uFramesHigh = 1;
}