#include "global.h"
#include "RageBitmapTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTextureManager.h"
#include "RageException.h"
#include "RageDisplay.h"
#include "RageTypes.h"
#include "arch/Dialog/Dialog.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageSurfaceUtils_Zoom.h"
#include "RageSurfaceUtils_Dither.h"
#include "RageSurface_Load.h"

static void GetResolutionFromFileName( CString sPath, int &Width, int &Height )
{
	/* Match:
	 *  Foo (res 512x128).png
	 * Also allow, eg:
	 *  Foo (dither, res 512x128).png
	 *
	 * Be careful that this doesn't get mixed up with frame dimensions. */
	static Regex re("\\([^\\)]*res ([0-9]+)x([0-9]+).*\\)");

	vector<CString> matches;
	if(!re.Compare(sPath, matches))
		return;

	Width = atoi(matches[0].c_str());
	Height = atoi(matches[1].c_str());
}

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

	ASSERT( actualID.filename != "" );

	/* Create (and return) a surface ready to be loaded to OpenGL */
	/* Load the image into a RageSurface. */
	CString error;
	RageSurface *img = RageSurfaceUtils::LoadFile( actualID.filename, error );

	/* Tolerate corrupt/unknown images. */
	if( img == NULL )
	{
		CString sWarning = ssprintf( "RageBitmapTexture: Couldn't load %s: %s", actualID.filename.c_str(), error.c_str() );
		Dialog::OK( sWarning );
		img = RageSurfaceUtils::MakeDummySurface( 64, 64 );
		ASSERT( img != NULL );
	}

	if( actualID.bHotPinkColorKey )
		RageSurfaceUtils::ApplyHotPinkColorKey( img );

	{
		/* Do this after setting the color key for paletted images; it'll also return
		 * TRAIT_NO_TRANSPARENCY if the color key is never used. */
		int traits = RageSurfaceUtils::FindSurfaceTraits(img);
		if( traits & RageSurfaceUtils::TRAIT_NO_TRANSPARENCY )
			actualID.iAlphaBits = 0;
		else if( traits & RageSurfaceUtils::TRAIT_BOOL_TRANSPARENCY )
			actualID.iAlphaBits = 1;
	}

	// look in the file name for a format hints
	CString HintString = GetID().filename + actualID.AdditionalTextureHints;
	HintString.MakeLower();

	if( HintString.Find("32bpp") != -1 )			actualID.iColorDepth = 32;
	else if( HintString.Find("16bpp") != -1 )		actualID.iColorDepth = 16;
	if( HintString.Find("dither") != -1 )			actualID.bDither = true;
	if( HintString.Find("stretch") != -1 )			actualID.bStretch = true;
	if( HintString.Find("mipmaps") != -1 )			actualID.bMipMaps = true;
	if( HintString.Find("nomipmaps") != -1 )		actualID.bMipMaps = false;	// check for "nomipmaps" after "mipmaps"

	/* If the image is marked grayscale, then use all bits not used for alpha
	 * for the intensity.  This way, if an image has no alpha, you get an 8-bit
	 * grayscale; if it only has boolean transparency, you get a 7-bit grayscale. */
	if( HintString.Find("grayscale") != -1 )		actualID.iGrayscaleBits = 8-actualID.iAlphaBits;

	/* This indicates that the only component in the texture is alpha; assume all
	 * color is white. */
	if( HintString.Find("alphamap") != -1 )			actualID.iGrayscaleBits = 0;

	/* No iGrayscaleBits for images that are already paletted.  We don't support
	 * that; and that hint is intended for use on images that are already grayscale,
	 * it's not intended to change a color image into a grayscale image. */
	if( actualID.iGrayscaleBits != -1 && img->format->BitsPerPixel == 8 )
		actualID.iGrayscaleBits = -1;

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

	/* If we're under 8x8, increase it, to avoid filtering problems on odd hardware. */
	if(m_iTextureWidth < 8 || m_iTextureHeight < 8)
	{
		actualID.bStretch = true;
		m_iTextureWidth = max(8, m_iTextureWidth);
		m_iTextureHeight = max(8, m_iTextureHeight);
	}

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
		RageSurfaceUtils::Zoom( img, m_iImageWidth, m_iImageHeight );

	// Format of the image that we will pass to OpenGL and that we want OpenGL to use
	RageDisplay::PixelFormat pixfmt;

	if( actualID.iGrayscaleBits != -1 && DISPLAY->SupportsTextureFormat(RageDisplay::FMT_PAL) )
	{
		RageSurface *dst = RageSurfaceUtils::PalettizeToGrayscale( img, actualID.iGrayscaleBits, actualID.iAlphaBits );

		delete img;
		img = dst;
	}

	/* Figure out which texture format to use. */
	// if the source is palleted, load palleted no matter what the prefs
	if(img->format->BitsPerPixel == 8 && DISPLAY->SupportsTextureFormat(RageDisplay::FMT_PAL))
	{
		pixfmt = RageDisplay::FMT_PAL;
	}
	else
	{
		// not paletted
		switch( actualID.iColorDepth )
		{
		case 16:
			{
				/* Bits of alpha in the source: */
				int src_alpha_bits = 8 - img->format->Loss[3];

				/* Don't use more than we were hinted to. */
				src_alpha_bits = min( actualID.iAlphaBits, src_alpha_bits );

				switch( src_alpha_bits ) {
				case 0:
				case 1:
					pixfmt = RageDisplay::FMT_RGB5A1;
					break;
				default:	
					pixfmt = RageDisplay::FMT_RGBA4;
					break;
				}
			}
			break;
		case 32:
			pixfmt = RageDisplay::FMT_RGBA8;
			break;
		default:
			RageException::Throw( "Invalid color depth: %d bits", actualID.iColorDepth );
		}
	}

	/* Make we're using a supported format. Every card supports either RGBA8 or RGBA4. */
	if( !DISPLAY->SupportsTextureFormat(pixfmt) )
	{
		pixfmt = RageDisplay::FMT_RGBA8;
		if( !DISPLAY->SupportsTextureFormat(pixfmt) )
			pixfmt = RageDisplay::FMT_RGBA4;
	}

	/* Dither if appropriate. XXX: This is a special case: don't bother dithering to
	 * RGBA8888.  We actually want to dither only if the destination has greater color
	 * depth on at least one color channel than the source.  For example, it doesn't
	 * make sense to do this when pixfmt is RGBA5551 if the image is only RGBA555. */
	if( actualID.bDither && 
		(pixfmt==RageDisplay::FMT_RGBA4 || pixfmt==RageDisplay::FMT_RGB5A1) )
	{
		/* Dither down to the destination format. */
		const RageDisplay::PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);
		RageSurface *dst = CreateSurface( img->w, img->h, pfd->bpp,
			pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3] );

		RageSurfaceUtils::ErrorDiffusionDither( img, dst );
		delete img;
		img = dst;
	}

	/* This needs to be done *after* the final resize, since that resize
	 * may introduce new alpha bits that need to be set.  It needs to be
	 * done *before* we set up the palette, since it might change it. */
	RageSurfaceUtils::FixHiddenAlpha(img);

	/* Convert the data to the destination format and dimensions 
	 * required by OpenGL if it's not in it already.  */
	/* We no longer need to do this; pixfmt and the format of img no longer have
	 * to match.  This means that if we have a paletted image, but the hardware
	 * doesn't support it, we can leave the data in the smaller paletted format
	 * and let OpenGL dereference it, as long as nothing else (such as dithering)
	 * ends up depalettizing it first.  We only have to scale it up to the texture
	 * size, which we won't have to do either if the image size is already a power
	 * of two. */
	// const RageDisplay::PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);
	RageSurfaceUtils::ConvertSurface( img, m_iTextureWidth, m_iTextureHeight,
		img->fmt.BitsPerPixel, img->fmt.Mask[0], img->fmt.Mask[1], img->fmt.Mask[2], img->fmt.Mask[3] );
	//	pfd->bpp, pfd->masks[0], pfd->masks[1], pfd->masks[2], pfd->masks[3] );
	
	m_uTexHandle = DISPLAY->CreateTexture( pixfmt, img, actualID.bMipMaps );

	CreateFrameRects();


	//
	// Enforce frames in the image have even dimensions.  Otherwise, 
	// pixel/texel alignment will be off.
	//
	bool bRunCheck = true;
	
	 // Don't check if the artist intentionally blanked the image by making it very tiny.
	if( this->GetSourceWidth()<=2 || this->GetSourceHeight()<=2 )
		 bRunCheck = false;
	
	// HACK: Don't check song graphics.  Many of them are weird dimensions.
	if( !TEXTUREMAN->GetOddDimensionWarning() )
		 bRunCheck = false;

	if( bRunCheck  )
	{
		float fFrameWidth = this->GetSourceWidth() / (float)this->GetFramesWide();
		float fFrameHeight = this->GetSourceHeight() / (float)this->GetFramesHigh();
		float fBetterFrameWidth = roundf((fFrameWidth+0.99f)/2)*2;
		float fBetterFrameHeight = roundf((fFrameHeight+0.99f)/2)*2;
		float fBetterSourceWidth = this->GetFramesWide() * fBetterFrameWidth;
		float fBetterSourceHeight = this->GetFramesHigh() * fBetterFrameHeight;
		if( fFrameWidth!=fBetterFrameWidth || fFrameHeight!=fBetterFrameHeight )
		{
			CString sWarning = ssprintf(
				"The graphic '%s' has frame dimensions that aren't even numbers.\n\n"
				"The entire image is %dx%d and frame size is %.1fx%.1f.\n\n"
				"Image quality will be much improved if you resize the graphic to %.0fx%.0f, which is a frame size of %.0fx%.0f.", 
				actualID.filename.c_str(), 
				this->GetSourceWidth(), this->GetSourceHeight(), 
				fFrameWidth, fFrameHeight,
				fBetterSourceWidth, fBetterSourceHeight,
				fBetterFrameWidth, fBetterFrameHeight );
			LOG->Warn( sWarning );
			Dialog::OK( sWarning, "FRAME_DIMENSIONS_WARNING" );
		}
	}



	delete img;

	/* See if the apparent "size" is being overridden. */
	GetResolutionFromFileName(actualID.filename, m_iSourceWidth, m_iSourceHeight);


	CString props;
	props += RageDisplay::PixelFormatToString( pixfmt ) + " ";
	if(actualID.iAlphaBits == 0) props += "opaque ";
	if(actualID.iAlphaBits == 1) props += "matte ";
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

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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
