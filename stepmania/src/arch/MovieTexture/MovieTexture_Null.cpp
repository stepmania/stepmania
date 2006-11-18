#include "global.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "MovieTexture_Null.h"
#include "RageSurface.h"

REGISTER_MOVIE_TEXTURE_CLASS( Null );
MovieTexture_Null::MovieTexture_Null(RageTextureID ID) : RageMovieTexture(ID)
{
	LOG->Trace("MovieTexture_Null::MovieTexture_Null(ID)");
	texHandle = 0;

	RageTextureID actualID = GetID();

	actualID.iAlphaBits = 0;
	int size = 64;
	m_iSourceWidth = size;
	m_iSourceHeight = size;
	m_iImageWidth = size;
	m_iImageHeight = size;
	m_iTextureWidth = power_of_two(size);
	m_iTextureHeight = m_iTextureWidth;
	m_iFramesWide = 1;
	m_iFramesHigh = 1;

	CreateFrameRects();

	PixelFormat pixfmt = PixelFormat_RGBA4;
	if( !DISPLAY->SupportsTextureFormat(pixfmt) )
		pixfmt = PixelFormat_RGBA8;
	ASSERT( DISPLAY->SupportsTextureFormat(pixfmt) );

	const RageDisplay::PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc( pixfmt );
	RageSurface *img = CreateSurface( size, size, pfd->bpp,
		pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3] );
	memset( img->pixels, 0, img->pitch*img->h );

	texHandle = DISPLAY->CreateTexture( pixfmt, img, false );

	delete img;
}

MovieTexture_Null::~MovieTexture_Null()
{
	DISPLAY->DeleteTexture( texHandle );
}

/*
 * (c) 2003 Steve Checkoway
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
