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
RageTexture::RageTexture( RageTextureID name )
{
//	LOG->Trace( "RageTexture::RageTexture()" );

	m_ID = m_ActualID = name;
	m_iRefCount = 1;

	m_iSourceWidth = m_iSourceHeight = 0;
	m_iTextureWidth = m_iTextureHeight = 0;
	m_iImageWidth = m_iImageHeight = 0;
	m_iFramesWide = m_iFramesHigh = 0;
}

RageTexture::~RageTexture()
{

}


void RageTexture::CreateFrameRects()
{
	GetFrameDimensionsFromFileName( GetFilePath(), &m_iFramesWide, &m_iFramesHigh );

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

void RageTexture::GetFrameDimensionsFromFileName( CString sPath, int* piFramesWide, int* piFramesHigh ) const
{
	*piFramesWide = *piFramesHigh = 1;	// set default values in case we don't find the dimension in the file name

	sPath.MakeLower();

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt);

	CStringArray arrayBits;
	split( sFName, " ", arrayBits, false );

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

const RectF *RageTexture::GetTextureCoordRect( int frameNo ) const
{
	return &m_TextureCoordRects[frameNo];
}

