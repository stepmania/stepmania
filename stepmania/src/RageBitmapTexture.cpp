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
#include "RageTextureManager.h"
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
#define NO_SDL_GLEXT
#define __glext_h_ /* try harder to stop glext.h from being forced on us by someone else */
#include "SDL_opengl.h"
#include "glext.h"

#include "RageTimer.h"



/* Definitions for various texture formats.  We'll probably want RGBA
 * in OpenGL, not ARGB ... All of these are in local (little) endian;
 * this may or may not need adjustment for OpenGL. */
const static unsigned int PixFmtMasks[][5] =
{
	/* Err.  D3D's texture formats are little-endian, so the order of
	 * colors (bytewise) is BGRA; D3DFMT_A8R8G8B8 is really BGRA (bytewise).
	 * D3DFMT_A4R4G4B4 is 0xGBAR; flip the bytes and it's sane (0xARGB).
	 */
	{	0x00FF0000,				/* B8G8R8A8 */
		0x0000FF00,
		0x000000FF,
		0xFF000000, 32 },
	{	0x0F00,					/* B4G4R4A4 */
		0x00F0,
		0x000F,
		0xF000, 16 },
	{	0x7C00,					/* B5G5R5A1 */
		0x03E0,
		0x001F,
		0x8000, 16 },
//	{	0xF800,					/* B5G6R5 */	/* this format doesn't seem to be supported in OGL 1.0 */
//		0x07E0,
//		0x001F,
//		0x0000, 16 }
};

int PixFmtMaskNo(GLenum fmt)
{
	switch(fmt) {
	case GL_RGBA8: return 0;
	case GL_RGBA4: return 1;
	case GL_RGB5_A1: return 2;
//	case D3DFMT_R5G6B5:   return 3;
	default: ASSERT(0);	  return 0;
	}
}

//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( CString sFilePath, RageTexturePrefs prefs ) :
	RageTexture( sFilePath, prefs )
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

void RageBitmapTexture::Reload( RageTexturePrefs prefs )
{
	DISPLAY->SetTexture(0);

	if(m_uGLTextureID) 
	{
		glDeleteTextures(1, &m_uGLTextureID);
		m_uGLTextureID = 0;
	}

	Create();
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
	// look in the file name for a format hints
	m_sFilePath.MakeLower();

	if( m_sFilePath.Find("no alpha") != -1 )		m_prefs.iAlphaBits = 0;
	else if( m_sFilePath.Find("1 alpha") != -1 )	m_prefs.iAlphaBits = 1;
	else if( m_sFilePath.Find("1alpha") != -1 )		m_prefs.iAlphaBits = 1;
	else if( m_sFilePath.Find("0alpha") != -1 )		m_prefs.iAlphaBits = 0;
	if( m_sFilePath.Find("dither") != -1 )			m_prefs.bDither = true; 

	/* Load the image into an SDL surface. */
	SDL_Surface *img = IMG_Load(m_sFilePath);
	/* XXX: Wait, we don't want to throw for all images; in particular, we
	 * want to tolerate corrupt/unknown background images. */
	if(img == NULL)
		throw RageException( "Couldn't load %s: %s", m_sFilePath, SDL_GetError() );

	/* Figure out which texture format to use. */
	GLenum fmtTexture;

	if( TEXTUREMAN->GetTextureColorDepth() == 16 )	
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
		src_alpha_bits = min( m_prefs.iAlphaBits, src_alpha_bits );

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
	else if( TEXTUREMAN->GetTextureColorDepth() == 32 )
		fmtTexture = GL_RGBA8;
	else
		throw RageException( "Invalid color depth: %d bits", TEXTUREMAN->GetTextureColorDepth() );

	/* Cap the max texture size to the hardware max. */
	m_prefs.iMaxSize = min( m_prefs.iMaxSize, DISPLAY->GetMaxTextureSize() );

	/* Save information about the source. */
	m_iSourceWidth = img->w;
	m_iSourceHeight = img->h;

	/* image size cannot exceed max size */
	m_iImageWidth = min( m_iSourceWidth, m_prefs.iMaxSize );
	m_iImageHeight = min( m_iSourceHeight, m_prefs.iMaxSize );

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(m_iImageWidth);
	m_iTextureHeight = power_of_two(m_iImageHeight);

	ASSERT( m_iTextureWidth <= m_prefs.iMaxSize );
	ASSERT( m_iTextureHeight <= m_prefs.iMaxSize );

	if(m_prefs.bStretch)
	{
		/* The hints asked for the image to be stretched to the texture size,
		 * probably for tiling. */
		m_iImageWidth = m_iTextureWidth;
		m_iImageHeight = m_iTextureHeight;
	}

	/* If the source is larger than the texture, we have to scale it down; that's
	 * "stretching", I guess. */
	if(m_iSourceWidth != m_iImageWidth || m_iSourceHeight > m_iImageHeight)
		m_prefs.bStretch = true;

	int target = PixFmtMaskNo(fmtTexture);

	/* Dither only when the target is 16bpp, not when it's 32bpp. */
	if( PixFmtMasks[target][4] /* XXX magic 4 */ == 32)
		m_prefs.bDither = false;

	if( m_prefs.bStretch ) 
	{
		/* resize currently only does RGBA8888 */
		int mask = 0;
		ConvertSDLSurface(img, img->w, img->h, PixFmtMasks[mask][4],
			PixFmtMasks[mask][0], PixFmtMasks[mask][1], PixFmtMasks[mask][2], PixFmtMasks[mask][3]);
		zoomSurface(img, m_iImageWidth, m_iImageHeight );
	}

	if( m_prefs.bDither )
	{
		/* Dither down to the destination format. */
		SDL_Surface *dst = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, img->w, img->h, PixFmtMasks[target][4],
			PixFmtMasks[target][0], PixFmtMasks[target][1], PixFmtMasks[target][2], PixFmtMasks[target][3]);

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
	ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight, 32,
		0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 );

	if(!m_uGLTextureID)
		glGenTextures(1, &m_uGLTextureID);

	DISPLAY->SetTexture(this);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / img->format->BytesPerPixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, fmtTexture, img->w, img->h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, img->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();

	SDL_FreeSurface(img);

	CreateFrameRects();

//	LOG->Trace( "RageBitmapTexture: Loaded '%s' (%ux%u) from disk.  bStretch = %d, source %d,%d;  image %d,%d.", 
//		m_sFilePath.GetString(), GetTextureWidth(), GetTextureHeight(),
//		bStretch, m_iSourceWidth, m_iSourceHeight,
//		m_iImageWidth,	m_iImageHeight);
}

