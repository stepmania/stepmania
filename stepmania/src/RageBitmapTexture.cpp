#include "global.h"
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
#include "RageDisplayInternal.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_endian.h"
#include "SDL_rotozoom.h"
#include "SDL_utils.h"
#include "SDL_dither.h"

#include "RageTimer.h"

enum pixfmts {
	FMT_RGBA8,
	FMT_RGBA4,
	FMT_RGB5A1,
	FMT_PAL,
	NUM_PIX_FORMATS
};

/* Return true for pixfmts that need to be converted to FMT_RGBA8 for GLDirect
 * (those with packed pixel data types). */
static bool IsPackedPixelFormat(GLenum pixfmt)
{
    return pixfmt == FMT_RGBA4 || pixfmt == FMT_RGB5A1;
}

/* Definitions for various texture formats.  We'll probably want RGBA
 * in OpenGL, not ARGB ... All of these are in local (little) endian;
 * this may or may not need adjustment for OpenGL. */
struct PixFmt_t {
	int bpp;
	GLenum internalfmt; /* target format */
	GLenum format; /* target format */
	GLenum type; /* data format */
	unsigned int masks[4];
} PixFmtMasks[NUM_PIX_FORMATS] = {
	/* XXX: GL_UNSIGNED_SHORT_4_4_4_4 is affected by endianness; GL_UNSIGNED_BYTE
	 * is not, but all SDL masks are affected by endianness, so GL_UNSIGNED_BYTE
	 * is reversed.  This isn't endian-safe. */
	{
		/* B8G8R8A8 */
		32,
		GL_RGBA8,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		{ 0x000000FF,
		  0x0000FF00,
		  0x00FF0000,
		  0xFF000000 }
	}, {
		/* B4G4R4A4 */
		16,
		GL_RGBA4,
		GL_RGBA,
		GL_UNSIGNED_SHORT_4_4_4_4,
		{ 0xF000,
		  0x0F00,
		  0x00F0,
		  0x000F },
	}, {
		/* B5G5R5A1 */
		16,
		GL_RGB5_A1,
		GL_RGBA,
		GL_UNSIGNED_SHORT_5_5_5_1,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0001 },
	}, {
		/* Paletted */
		8,
		GL_COLOR_INDEX8_EXT,
		GL_COLOR_INDEX,
		GL_UNSIGNED_BYTE,
		{ 0,0,0,0 } /* N/A */
	}
};

int PixFmtMaskNo(GLenum fmt)
{
	switch(fmt) {
	case GL_RGBA8: return FMT_RGBA8;
	case GL_RGBA4: return FMT_RGBA4;
	case GL_RGB5_A1: return FMT_RGB5A1;
	default: ASSERT(0);	  return FMT_RGBA8;
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
		glDeleteTextures(1,reinterpret_cast<GLuint*>(&m_uGLTextureID));
}

void RageBitmapTexture::Reload()
{
	RageTexture::Reload();
	DISPLAY->SetTexture(0);

	if(m_uGLTextureID) 
	{
		glDeleteTextures(1, reinterpret_cast<GLuint*>(&m_uGLTextureID));
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
	/* Load the image into an SDL surface. */
	SDL_Surface *img = IMG_Load(GetFilePath());

	/* XXX: Wait, we don't want to throw for all images; in particular, we
	 * want to tolerate corrupt/unknown background images. */
	if(img == NULL)
		RageException::Throw( "RageBitmapTexture: Couldn't load %s: %s", GetFilePath().GetString(), SDL_GetError() );

	if(m_ActualID.bHotPinkColorKey)
	{
		/* Annoying: SDL will do a nearest-match on paletted images if
		 * they don't have the color we ask for, so images without HOT PINK
		 * will get some other random color transparent.  We have to make
		 * sure the value returned for paletted images is exactly #FF00FF. */
		int color = SDL_MapRGB(img->format, 0xFF, 0, 0xFF);
		if( img->format->BitsPerPixel == 8 ) {
			if(img->format->palette->colors[color].r != 0xFF ||
			   img->format->palette->colors[color].g != 0x00 ||
			   img->format->palette->colors[color].b != 0xFF )
			   color = -1;
		}

		if( color != -1 )
			SDL_SetColorKey( img, SDL_SRCCOLORKEY, color);
	}

	{
		/* This should eventually obsolete 8alphaonly, 0alpha and 1alpha,
		 * and remove the need to special case background loads.  Do this
		 * after setting the color key for paletted images; it'll also return
		 * TRAIT_NO_TRANSPARENCY if the color key is never used. */
		int traits = FindSurfaceTraits(img);
		if(traits & TRAIT_NO_TRANSPARENCY) 
			m_ActualID.iAlphaBits = 0;
		else if(traits & TRAIT_BOOL_TRANSPARENCY) 
			m_ActualID.iAlphaBits = 1;
		if(traits & TRAIT_WHITE_ONLY) 
			m_ActualID.iTransparencyOnly = 8;
	}

	// look in the file name for a format hints
	CString HintString = GetFilePath();
	HintString.MakeLower();

	if( HintString.Find("4alphaonly") != -1 )		m_ActualID.iTransparencyOnly = 4;
	else if( HintString.Find("8alphaonly") != -1 )	m_ActualID.iTransparencyOnly = 8;
	if( HintString.Find("dither") != -1 )			m_ActualID.bDither = true;

	if( m_ActualID.iTransparencyOnly )
	{
		/* Treat the image as 32-bit, so we don't lose any alpha precision. */
		m_ActualID.iColorDepth = 32;
	}

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

	if( m_ActualID.bStretch ) 
	{
		/* resize currently only does RGBA8888 */
		int mask = 0;
		ConvertSDLSurface(img, img->w, img->h, PixFmtMasks[mask].bpp,
			PixFmtMasks[mask].masks[0], PixFmtMasks[mask].masks[1],
			PixFmtMasks[mask].masks[2], PixFmtMasks[mask].masks[3]);
		zoomSurface(img, m_iImageWidth, m_iImageHeight );
	}

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
	/* This will be set to the pixfmt we should use if we use an RGBA texture. */
	int desired_rgba_pixfmt;

	
	SDL_Surface *img = CreateImg(desired_rgba_pixfmt);

	if(!m_uGLTextureID)
		glGenTextures(1, reinterpret_cast<GLuint*>(&m_uGLTextureID));
	ASSERT(m_uGLTextureID);
	
	DISPLAY->SetTexture(this);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/* This is an experiment: some people are seeing skips when the danger
	 * background comes in.  One hypothesis: we're loading the danger background,
	 * keeping it in memory without displaying it for several games, it gets
	 * swapped out of texture memory, and then we skip swapping it in.
	 *
	 * It's rather small, so I'm not entirely convinced this is what's happening.
	 * Let's try boosting the texture priority for the danger background and
	 * see if it goes away.  If it does, I'll make texture pris a texture hint
	 * (or find another way to do this; perhaps we should do a dummy render of
	 * a texture when it's reused from our texture cache if it hasn't been used
	 * in a long time). */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0.5f);
	if(GetFilePath().Find("danger") != -1)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0f);

	int pixfmt = desired_rgba_pixfmt;

retry:
	if(img->format->BitsPerPixel == 8 && DISPLAY->GetSpecs().EXT_paletted_texture)
		pixfmt = FMT_PAL;

	if(pixfmt != FMT_PAL)
	{
		/* It's either not a paletted image, or we can't handle paletted textures.
		 * Convert to the desired RGBA format, dithering if appropriate. */

		/* Never dither when the target is 32bpp; there's no point. */
		if( PixFmtMasks[pixfmt].bpp == 32)
			m_ActualID.bDither = false;

		if( m_ActualID.bDither )
		{
			/* Dither down to the destination format. */
			SDL_Surface *dst = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, img->w, img->h, PixFmtMasks[pixfmt].bpp,
				PixFmtMasks[pixfmt].masks[0], PixFmtMasks[pixfmt].masks[1],
				PixFmtMasks[pixfmt].masks[2], PixFmtMasks[pixfmt].masks[3]);

			SM_SDL_OrderedDither(img, dst);
			//SM_SDL_ErrorDiffusionDither(img, dst);
			SDL_FreeSurface(img);
			img = dst;
		}
	}

	/* Convert the data to the destination format if it's not in it already.  */
	ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight, PixFmtMasks[pixfmt].bpp,
		PixFmtMasks[pixfmt].masks[0], PixFmtMasks[pixfmt].masks[1],
		PixFmtMasks[pixfmt].masks[2], PixFmtMasks[pixfmt].masks[3]);

	/* This needs to be done *after* the final resize, since that resize
	 * may introduce new alpha bits that need to be set.  It needs to be
	 * done *before* we set up the palette, since it might change it. */
	FixHiddenAlpha(img);

	if(pixfmt == FMT_PAL)
	{
		/* The image is currently paletted.  Let's try to set up a paletted texture. */
		GLubyte palette[256*4];
		memset(palette, 0, sizeof(palette));
		int p = 0;
		/* Copy the palette to the simple, unpacked data OGL expects. If
		 * we're color keyed, change it over as we go. */
		for(int i = 0; i < img->format->palette->ncolors; ++i)
		{
			palette[p++] = img->format->palette->colors[i].r;
			palette[p++] = img->format->palette->colors[i].g;
			palette[p++] = img->format->palette->colors[i].b;

			if(img->flags & SDL_SRCCOLORKEY && i == int(img->format->colorkey))
				palette[p++] = 0;
			else
				palette[p++] = 0xFF; /* opaque */
		}

		/* Set the palette. */
		GLExt::glColorTableEXT(GL_TEXTURE_2D, GL_RGBA8, 256, GL_RGBA, GL_UNSIGNED_BYTE, palette);

		GLint RealFormat = 0;
		GLExt::glGetColorTableParameterivEXT(GL_TEXTURE_2D, GL_COLOR_TABLE_FORMAT, &RealFormat);
		if(RealFormat != GL_RGBA8)
		{
			/* This is a case I don't expect to happen; if it does, log,
			 * turn off PT's permanently and continue as an RGBA texture. */
			LOG->Info("Expected an RGBA8 palette, got %i instead; disabling paletted textures", RealFormat);
			DISPLAY->DisablePalettedTexture();
			pixfmt = desired_rgba_pixfmt;
			goto retry;
		}
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / img->format->BytesPerPixel);

	GLenum internalfmt = PixFmtMasks[pixfmt].internalfmt;

	if(DISPLAY->GetSpecs().bPackedPixelsCauseProblems &&
	   IsPackedPixelFormat(pixfmt))
	{
	    /* GLDirect crashes if we give it packed pixel format.  We need
	     * to convert to a non-packed format (FMT_RGBA8).  However, we
	     * can still get 16-bit textures by passing it the appropriate
	     * internalformat; it'll simply truncate the extra bits.  This
	     * means that it'll still use the dithering we gave it; we just
	     * have to do a bit of redundant conversion work. */
	    ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight, PixFmtMasks[FMT_RGBA8].bpp,
		    PixFmtMasks[FMT_RGBA8].masks[0], PixFmtMasks[FMT_RGBA8].masks[1],
		    PixFmtMasks[FMT_RGBA8].masks[2], PixFmtMasks[FMT_RGBA8].masks[3]);

		pixfmt = FMT_RGBA8;
		/* (don't change internalfmt) */
	}

	if(internalfmt != GL_COLOR_INDEX8_EXT)
	{
		/* Override the internalformat with an alpha format if it was requested. 
		 * Don't use iTransparencyOnly with paletted images; there's no point--paletted
		 * images are as small or smaller (and the load will fail). */
		if(m_ActualID.iTransparencyOnly == 4)
			internalfmt = GL_ALPHA4;
		else if(m_ActualID.iTransparencyOnly == 8)
			internalfmt = GL_ALPHA8;
		else ASSERT(m_ActualID.iTransparencyOnly == 0);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalfmt, 
			m_iTextureWidth, m_iTextureHeight, 0,
			PixFmtMasks[pixfmt].format, PixFmtMasks[pixfmt].type, img->pixels);

	/* If we're paletted, and didn't get the 8-bit palette we asked for ...*/
	if(img->format->BitsPerPixel == 8)
	{
		GLint size = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GLenum(GL_TEXTURE_INDEX_SIZE_EXT), &size);
		if(size != 8)
		{
			/* I don't know any reason this should actually fail (paletted textures
			 * but no support for 8-bit palettes?), so let's just disable paletted
			 * textures the first time this happens. */
			LOG->Info("Expected an 8-bit palette, got a %i-bit one instead; disabling paletted textures", size);
			DISPLAY->DisablePalettedTexture();
			pixfmt = desired_rgba_pixfmt;
			goto retry;
		}
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glFlush();

	SDL_FreeSurface(img);

	CreateFrameRects();
	CString props = " ";
	switch( internalfmt )
	{
	case GL_RGBA4:				props += "GL_RGBA4 ";				break;
	case GL_RGBA8:				props += "GL_RGBA8 ";				break;
	case GL_RGB5_A1:			props += "GL_RGB5_A1 ";				break;
	case GL_ALPHA8:				props += "GL_ALPHA8 ";				break;
	case GL_COLOR_INDEX8_EXT:	props += "GL_COLOR_INDEX8_EXT ";	break;
	default:	props += ssprintf("unknown-format-%d ",internalfmt); break;
	}

	if(m_ActualID.iAlphaBits == 0) props += "opaque ";
	if(m_ActualID.iAlphaBits == 1) props += "matte ";
	if(m_ActualID.iTransparencyOnly) props += "mask ";
	if(m_ActualID.bStretch) props += "stretch ";
	if(m_ActualID.bDither) props += "dither ";
	if(IsPackedPixelFormat(pixfmt)) props += "paletted ";
	props.erase(props.size()-1);
	LOG->Trace( "RageBitmapTexture: Loaded '%s' (%ux%u); %s, source %d,%d;  image %d,%d.", 
		m_ActualID.filename.GetString(), GetTextureWidth(), GetTextureHeight(),
		props.GetString(), m_iSourceWidth, m_iSourceHeight,
		m_iImageWidth,	m_iImageHeight);
}

