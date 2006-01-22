#include "global.h"

#include "RageTexture.h"
#include "RageUtil.h"
#include "RageTextureManager.h"
#include <cstring>


RageTexture::RageTexture( RageTextureID name ):
	m_ID(name)
{
	m_iRefCount = 1;
	m_bWasUsed = false;

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
	GetFrameDimensionsFromFileName( GetID().filename, &m_iFramesWide, &m_iFramesHigh );

	//
	// Fill in the m_FrameRects with the bounds of each frame in the animation.
	//
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

void RageTexture::GetFrameDimensionsFromFileName( RString sPath, int* piFramesWide, int* piFramesHigh )
{
	static Regex match( " ([0-9]+)x([0-9]+)([\\. ]|$)" );
        vector<RString> asMatch;
	if( !match.Compare(sPath, asMatch) )
	{
		*piFramesWide = *piFramesHigh = 1;
		return;
	}
	*piFramesWide = atoi(asMatch[0]);
	*piFramesHigh = atoi(asMatch[1]);
}

const RectF *RageTexture::GetTextureCoordRect( int iFrameNo ) const
{
	return &m_TextureCoordRects[iFrameNo];
}

/*
 * Copyright (c) 2001-2004 Chris Danford
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
