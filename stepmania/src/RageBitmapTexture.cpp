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
#include "RageTypes.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_endian.h"
#include "SDL_rotozoom.h"
#include "SDL_utils.h"
#include "SDL_dither.h"

#include "RageTimer.h"

static void GetResolutionFromFileName( CString sPath, int &Width, int &Height )
{
	/* Match:
	 *  Foo (res 512x128).png
	 * Also allow, eg:
	 *  Foo (dither, res 512x128).png
	 *
	 * Be careful that this doesn't get mixed up with frame dimensions. */
	Regex re("\\([^\\)]*res ([0-9]+)x([0-9]+).*\\)");

	vector<CString> matches;
	if(!re.Compare(sPath, matches))
		return;

	Width = atoi(matches[0].c_str());
	Height = atoi(matches[1].c_str());
}

//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( RageTextureID name ) :
	RageTexture( name )
{
//	LOG->Trace( "RageBitmapTexture::RageBitmapTexture()" );
	Create();
}

RageBitmapTexture::~RageBitmapTexture()
{
	Destroy();
}

void RageBitmapTexture::Reload()
{
	Destroy();
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
	RageTextureID actualID = GetID();

	/* Create (and return) a surface ready to be loaded to OpenGL */
	/* Load the image into an SDL surface. */
	SDL_Surface *img = IMG_Load(GetFilePath());

	/* XXX: Wait, we don't want to throw for all images; in particular, we
	 * want to tolerate corrupt/unknown background images. */
	if(img == NULL)
		RageException::Throw( "RageBitmapTexture: Couldn't load %s: %s", GetFilePath().c_str(), SDL_GetError() );

	if(actualID.bHotPinkColorKey)
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
			actualID.iAlphaBits = 0;
		else if(traits & TRAIT_BOOL_TRANSPARENCY) 
			actualID.iAlphaBits = 1;
		if(traits & TRAIT_WHITE_ONLY) 
			actualID.iTransparencyOnly = 8;
	}

	// look in the file name for a format hints
	CString HintString = GetFilePath();
	HintString.MakeLower();

	if( HintString.Find("4alphaonly") != -1 )		actualID.iTransparencyOnly = 4;
	else if( HintString.Find("8alphaonly") != -1 )	actualID.iTransparencyOnly = 8;
	if( HintString.Find("dither") != -1 )			actualID.bDither = true;

	if( actualID.iTransparencyOnly )
		actualID.iColorDepth = 32;	/* Treat the image as 32-bit, so we don't lose any alpha precision. */

	/* Cap the max texture size to the hardware max. */
	actualID.iMaxSize = min( actualID.iMaxSize, DISPLAY->GetMaxTextureSize() );

	/* Save information about the source. */
	m_iSourceWidth = img->w;
	m_iSourceHeight = img->h;

	/* image size cannot exceed max size */
	m_iImageWidth = min( m_iSourceWidth, actualID.iMaxSize );
	m_iImageHeight = min( m_iSourceHeight, actualID.iMaxSize );

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(m_iImageWidth);
	m_iTextureHeight = power_of_two(m_iImageHeight);

	ASSERT( m_iTextureWidth <= actualID.iMaxSize );
	ASSERT( m_iTextureHeight <= actualID.iMaxSize );

	if(actualID.bStretch)
	{
		/* The hints asked for the image to be stretched to the texture size,
		 * probably for tiling. */
		m_iImageWidth = m_iTextureWidth;
		m_iImageHeight = m_iTextureHeight;
	}

	if( img->w != m_iImageWidth || img->h != m_iImageHeight ) 
	{
		/* resize currently only does RGBA8888 */
		ConvertSDLSurface(img, img->w, img->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		zoomSurface(img, m_iImageWidth, m_iImageHeight );
	}

	// Format of the image that we will pass to OpenGL and that we want OpenGL to use
	PixelFormat pixfmt;

	/* Figure out which texture format to use. */
	// if the source is palleted, load palleted no matter what the prefs
	if(img->format->BitsPerPixel == 8 && DISPLAY->SupportsTextureFormat(FMT_PAL))
	{
		pixfmt = FMT_PAL;
	}
	else
	{
		// not paletted
		switch( actualID.iColorDepth )
		{
		case 16:
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
				src_alpha_bits = min( actualID.iAlphaBits, src_alpha_bits );

				switch( src_alpha_bits ) {
				case 0:
				case 1:
					pixfmt = FMT_RGB5A1;
					break;
				default:	
					pixfmt = FMT_RGBA4;
					break;
				}
			}
			break;
		case 32:
			pixfmt = FMT_RGBA8;
			break;
		default:
			RageException::Throw( "Invalid color depth: %d bits", actualID.iColorDepth );
		}

		/* Override the internalformat with an alpha format if it was requested. 
		 * Don't use iTransparencyOnly with paletted images; there's no point--paletted
		 * images are as small or smaller (and the load will fail). */
		/* SDL surfaces don't allow for 8 bpp surfaces that aren't paletted.  Arg! 
		 * fix this later. -Chris */
//		if(actualID.iTransparencyOnly > 0)
//		{
//			imagePixfmt = FMT_ALPHA8;
//			texturePixfmt = FMT_ALPHA8;
//		}

		/* It's either not a paletted image, or we can't handle paletted textures.
		 * Convert to the desired RGBA format, dithering if appropriate. */
		if( actualID.bDither && 
			(pixfmt==FMT_RGBA4 || pixfmt==FMT_RGB5A1) )	/* Don't dither if format is 32bpp; there's no point. */
		{
			/* Dither down to the destination format. */
			const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);
			SDL_Surface *dst = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, img->w, img->h,
				pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]);

			SM_SDL_ErrorDiffusionDither(img, dst);
			SDL_FreeSurface(img);
			img = dst;
		}
	}

	/* This needs to be done *after* the final resize, since that resize
	 * may introduce new alpha bits that need to be set.  It needs to be
	 * done *before* we set up the palette, since it might change it. */
	FixHiddenAlpha(img);

	/* Make we're using a supported format. 
	 * Every card supports either RGBA8 or RGBA4. */
	if( !DISPLAY->SupportsTextureFormat(pixfmt) )
	{
		pixfmt = FMT_RGBA8;
		if( !DISPLAY->SupportsTextureFormat(pixfmt) )
			pixfmt = FMT_RGBA4;
	}
		

	/* Convert the data to the destination format and dimensions 
	 * required by OpenGL if it's not in it already.  */
	const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);
	ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight,
		pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3]);
	
	m_uTexHandle = DISPLAY->CreateTexture( pixfmt, img );

	SDL_FreeSurface( img );

	CreateFrameRects();

	/* See if the apparent "size" is being overridden. */
	GetResolutionFromFileName(actualID.filename, m_iSourceWidth, m_iSourceHeight);


	CString props;
	props += PixelFormatToString( pixfmt ) + " ";
	if(actualID.iAlphaBits == 0) props += "opaque ";
	if(actualID.iAlphaBits == 1) props += "matte ";
	if(actualID.iTransparencyOnly) props += "mask ";
	if(actualID.bStretch) props += "stretch ";
	if(actualID.bDither) props += "dither ";
	props.erase(props.size()-1);
	LOG->Trace( "RageBitmapTexture: Loaded '%s' (%ux%u); %s, source %d,%d;  image %d,%d.", 
		actualID.filename.c_str(), GetTextureWidth(), GetTextureHeight(),
		props.c_str(), m_iSourceWidth, m_iSourceHeight,
		m_iImageWidth,	m_iImageHeight);
}

void RageBitmapTexture::Destroy()
{
	DISPLAY->DeleteTexture( m_uTexHandle );
}

