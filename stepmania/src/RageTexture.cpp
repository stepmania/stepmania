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
	iMaxSize = 2048;
	iMipMaps = 4;
	iAlphaBits = 4;
	bDither = false;
	bStretch = false;
	iColorDepth = -1; /* default */
	bHotPinkColorKey = false;
}

bool RageTextureID::operator<(const RageTextureID &rhs) const
{
	if(filename < rhs.filename) return true;
	if(filename > rhs.filename) return false;
	if(iMaxSize < rhs.iMaxSize) return true;
	if(iMaxSize > rhs.iMaxSize) return false;
	if(iMipMaps < rhs.iMipMaps) return true;
	if(iMipMaps > rhs.iMipMaps) return false;
	if(iAlphaBits < rhs.iAlphaBits) return true;
	if(iAlphaBits > rhs.iAlphaBits) return false;
	if(iColorDepth < rhs.iColorDepth) return true;
	if(iColorDepth > rhs.iColorDepth) return false;
	if(bDither < rhs.bDither) return true;
	if(bDither > rhs.bDither) return false;
	if(bStretch < rhs.bStretch) return true;
	if(bStretch > rhs.bStretch) return false;
	if(bHotPinkColorKey < rhs.bHotPinkColorKey) return true;
	if(bHotPinkColorKey > rhs.bHotPinkColorKey) return false;
	return false;
}


RageTexture::RageTexture( RageTextureID name ):
	m_ID(name)
{
//	LOG->Trace( "RageTexture::RageTexture()" );

	m_iRefCount = 1;

	SetActualID();
	m_iSourceWidth = m_iSourceHeight = 0;
	m_iTextureWidth = m_iTextureHeight = 0;
	m_iImageWidth = m_iImageHeight = 0;
	m_iFramesWide = m_iFramesHigh = 0;
}


/* Set the initial ActualID; this is what the actual texture will start
 * from. */
void RageTexture::SetActualID()
{
	m_ActualID = m_ID;

	/* Texture color depth preference can be overridden. */
	if(m_ID.iColorDepth == -1)
		m_ActualID.iColorDepth = TEXTUREMAN->GetTextureColorDepth();

	/* The max texture size can never be higher than the preference,
	 * since it might be set to something to fix driver problems. */
	m_ActualID.iMaxSize = min(m_ActualID.iMaxSize, TEXTUREMAN->GetMaxTextureResolution());
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

