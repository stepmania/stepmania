#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageBitmapTexture

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageBitmapTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageDisplay.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_endian.h"
#include "SDL_rotozoom.h"
#include "SDL_utils.h"
#include "SDL_dither.h"

#include "RageTimer.h"

/* Definitions for various texture formats.  We'll probably want RGBA
 * in OpenGL, not ARGB ... All of these are in local (little) endian;
 * this may or may not need adjustment for OpenGL. */
struct PixFmt_t {
	int bpp;
	GLenum type; /* data format */
	GLenum internalfmt; /* target format */
	unsigned int masks[4];
} PixFmtMasks[] = {
	/* XXX: GL_UNSIGNED_SHORT_4_4_4_4 is affected by endianness; GL_UNSIGNED_BYTE
	 * is not, but all SDL masks are affected by endianness, so GL_UNSIGNED_BYTE
	 * is reversed.  This isn't endian-safe. */
	{
		/* B8G8R8A8 */
		32,
		GL_UNSIGNED_BYTE,
		GL_RGBA8,
		{ 0x000000FF,
		  0x0000FF00,
		  0x00FF0000,
		  0xFF000000 }
	}, {
		/* B4G4R4A4 */
		16,
		GL_UNSIGNED_SHORT_4_4_4_4,
		GL_RGBA4,
		{ 0xF000,
		  0x0F00,
		  0x00F0,
		  0x000F },
	}, {
		/* B5G5R5A1 */
		16,
		GL_UNSIGNED_SHORT_5_5_5_1,
		GL_RGB5_A1,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0001 },
	}
};

int PixFmtMaskNo(GLenum fmt)
{
	switch(fmt) {
	case GL_RGBA8: return 0;
	case GL_RGBA4: return 1;
	case GL_RGB5_A1: return 2;
	default: ASSERT(0);	  return 0;
	}
}

//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( RageTextureID name ) :
	RageTexture( name )
{
//	LOG->Trace( "RageBitmapTexture::RageBitmapTexture()" );

	m_uGLTextureID = 0;

	Create();	// sFilePath and prefs are saved by RageTexture()
}

RageBitmapTexture::~RageBitmapTexture()
{
	if(m_uGLTextureID)
		glDeleteTextures(1, &m_uGLTextureID);
}

void RageBitmapTexture::Reload( RageTextureID ID )
{
	RageTexture::Reload(ID);
	DISPLAY->SetTexture(0);

	if(m_uGLTextureID) 
	{
		glDeleteTextures(1, &m_uGLTextureID);
		m_uGLTextureID = 0;
	}

	Create();
}

/* 1. Create (and return) a surface ready to be loaded to OpenGL, 
 * 2. Set up m_ActualID, and
 * 3. Set these texture parameters:
 *    m_iSourceWidth, m_iSourceHeight
 *    m_iTextureWidth, m_iTextureHeight
 *    m_iImageWidth, m_iImageHeight
 *    m_iFramesWide, m_iFramesHigh
 */
SDL_Surface *RageBitmapTexture::CreateImg(int &pixfmt)
{
	// look in the file name for a format hints
	CString HintString = GetFilePath();
	HintString.MakeLower();

	if( HintString.Find("no alpha") != -1 )		m_ActualID.iAlphaBits = 0;
	else if( HintString.Find("1 alpha") != -1 )	m_ActualID.iAlphaBits = 1;
	else if( HintString.Find("1alpha") != -1 )	m_ActualID.iAlphaBits = 1;
	else if( HintString.Find("0alpha") != -1 )	m_ActualID.iAlphaBits = 0;
	if( HintString.Find("dither") != -1 )		m_ActualID.bDither = true;

	/* Load the image into an SDL surface. */
	SDL_Surface *img = IMG_Load(GetFilePath());
	/* XXX: Wait, we don't want to throw for all images; in particular, we
	 * want to tolerate corrupt/unknown background images. */
	if(img == NULL)
		RageException::Throw( "Couldn't load %s: %s", GetFilePath().GetString(), SDL_GetError() );

	GLenum fmtTexture;
	/* Figure out which texture format to use. */
	if( m_ActualID.iColorDepth == 16 )
	{
		/* Bits of alpha in the source: */
		int src_alpha_bits = 8 - img->format->Aloss;

		/* No real alpha in paletted input. */
		if( img->format->BytesPerPixel == 1 )
			src_alpha_bits = 0;

		/* Colorkeyed input effectively has at least one bit of alpha: */
		if( img->flags & SDL_SRCCOLORKEY )
			src_alpha_bits = max( 1, src_alpha_bits );

		/* Don't use more than we were hinted to. */
		src_alpha_bits = min( m_ActualID.iAlphaBits, src_alpha_bits );

		/* XXX Scan the image, and see if it actually uses its alpha channel/color key
		* (if any).  Reduce to 1 or 0 bits of alpha if possible. */

		switch( src_alpha_bits ) {
		case 0:
		case 1:
			fmtTexture = GL_RGB5_A1;
			break;
		default:	
			fmtTexture = GL_RGBA4;
			break;
		}
	} 
	else if( m_ActualID.iColorDepth == 32)
		fmtTexture = GL_RGBA8;
	else
		RageException::Throw( "Invalid color depth: %d bits", m_ActualID.iColorDepth );

	/* Cap the max texture size to the hardware max. */
	m_ActualID.iMaxSize = min( m_ActualID.iMaxSize, DISPLAY->GetMaxTextureSize() );

	/* Save information about the source. */
	m_iSourceWidth = img->w;
	m_iSourceHeight = img->h;

	/* image size cannot exceed max size */
	m_iImageWidth = min( m_iSourceWidth, m_ActualID.iMaxSize );
	m_iImageHeight = min( m_iSourceHeight, m_ActualID.iMaxSize );

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(m_iImageWidth);
	m_iTextureHeight = power_of_two(m_iImageHeight);

	ASSERT( m_iTextureWidth <= m_ActualID.iMaxSize );
	ASSERT( m_iTextureHeight <= m_ActualID.iMaxSize );

	if(m_ActualID.bStretch)
	{
		/* The hints asked for the image to be stretched to the texture size,
		 * probably for tiling. */
		m_iImageWidth = m_iTextureWidth;
		m_iImageHeight = m_iTextureHeight;
	}

	/* If the source is larger than the texture, we have to scale it down; that's
	 * "stretching", I guess. */
	if(m_iSourceWidth != m_iImageWidth || m_iSourceHeight > m_iImageHeight)
		m_ActualID.bStretch = true;

	pixfmt = PixFmtMaskNo(fmtTexture);

	/* Dither only when the target is 16bpp, not when it's 32bpp. */
	if( PixFmtMasks[pixfmt].bpp == 32)
		m_ActualID.bDither = false;

	if( m_ActualID.bStretch ) 
	{
		/* resize currently only does RGBA8888 */
		int mask = 0;
		ConvertSDLSurface(img, img->w, img->h, PixFmtMasks[mask].bpp,
			PixFmtMasks[mask].masks[0], PixFmtMasks[mask].masks[1],
			PixFmtMasks[mask].masks[2], PixFmtMasks[mask].masks[3]);
		zoomSurface(img, m_iImageWidth, m_iImageHeight );
	}

	if( m_ActualID.bDither )
	{
		/* Dither down to the destination format. */
		SDL_Surface *dst = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, img->w, img->h, PixFmtMasks[pixfmt].bpp,
			PixFmtMasks[pixfmt].masks[0], PixFmtMasks[pixfmt].masks[1],
			PixFmtMasks[pixfmt].masks[2], PixFmtMasks[pixfmt].masks[3]);

		SM_SDL_OrderedDither(img, dst);
		SDL_FreeSurface(img);
		img = dst;
	}

	/* Convert the data to the destination format.  Hmm.  We could just
	 * convert the format, leaving the resolution alone (simplifying
	 * ConvertSDLSurface), and then load the texture a little more
	 * intelligently.  If we do that with OpenGL, is the rest
	 * of the texture (that we didn't fill) guaranteed to be black?
	 * We don't want anything else to be linearly filtered in on the
	 * edge of the texture ...
	 */
	/* We could check to see if we happen to simply be in a reversed
	 * pixel order, and tell OpenGL to do the switch for us. */
	ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight, PixFmtMasks[pixfmt].bpp,
			PixFmtMasks[pixfmt].masks[0], PixFmtMasks[pixfmt].masks[1],
			PixFmtMasks[pixfmt].masks[2], PixFmtMasks[pixfmt].masks[3]);

	return img;
}

/*
 * Each dwMaxSize, dwTextureColorDepth and iAlphaBits are maximums; we may
 * use less.  iAlphaBits must be 0, 1 or 4.
 *
 * XXX: change iAlphaBits == 4 to iAlphaBits == 8 to indicate "as much alpha
 * as needed", since that's what it really is; still only use 4 in 16-bit textures.
 *
 * Dither forces dithering when loading 16-bit textures.
 * Stretch forces the loaded image to fill the texture completely.
 */

void RageBitmapTexture::Create()
{
	int pixfmt;

	SDL_Surface *img = CreateImg(pixfmt);

	if(!m_uGLTextureID)
		glGenTextures(1, &m_uGLTextureID);

	DISPLAY->SetTexture(this);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / img->format->BytesPerPixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, PixFmtMasks[pixfmt].internalfmt, img->w, img->h, 0,
			GL_RGBA, PixFmtMasks[pixfmt].type, img->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();

	SDL_FreeSurface(img);

	CreateFrameRects();

//	LOG->Trace( "RageBitmapTexture: Loaded '%s' (%ux%u) from disk.  bStretch = %d, source %d,%d;  image %d,%d.", 
//		m_sFilePath.GetString(), GetTextureWidth(), GetTextureHeight(),
//		bStretch, m_iSourceWidth, m_iSourceHeight,
//		m_iImageWidth,	m_iImageHeight);
}

