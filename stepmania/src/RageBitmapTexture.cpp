#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageBitmapTexture

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "RageBitmapTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"

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
	{	0xF800,					/* B5G6R5 */
		0x07E0,
		0x001F,
		0x0000, 16 }
};

int PixFmtMaskNo(D3DFORMAT fmt)
{
	switch(fmt) {
	case D3DFMT_A8R8G8B8: return 0;
	case D3DFMT_A4R4G4B4: return 1;
	case D3DFMT_A1R5G5B5: return 2;
	case D3DFMT_R5G6B5:   return 3;
	default: ASSERT(0);	  return 0;
	}
}

//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( 
	RageDisplay* pScreen, 
	const CString &sFilePath, 
	int dwMaxSize, 
	int dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch
	) :
	RageTexture( pScreen, sFilePath, dwMaxSize, dwTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch )
{
//	LOG->Trace( "RageBitmapTexture::RageBitmapTexture()" );

	m_pd3dTexture = NULL;

	//if( !LoadFromCacheFile() )
		Create( dwMaxSize, dwTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );
	
	//SaveToCache();

	CreateFrameRects();
}

RageBitmapTexture::~RageBitmapTexture()
{
	SAFE_RELEASE(m_pd3dTexture);
}

void RageBitmapTexture::Reload( 	
	int dwMaxSize, 
	int dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch
	)
{
	SAFE_RELEASE(m_pd3dTexture);
	Create( dwMaxSize, dwTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );
	// leave m_iRefCount alone!
	CreateFrameRects();
}

//-----------------------------------------------------------------------------
// GetTexture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE8 RageBitmapTexture::GetD3DTexture()
{
	return m_pd3dTexture; 
}

#if 1

static int power_of_two(int input)
{
    int value = 1;

	while ( value < input ) value <<= 1;

	return value;
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

void RageBitmapTexture::Create( 
	int dwMaxSize,
	int dwTextureColorDepth,
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch
	)
{
	HRESULT hr;

	// look in the file name for a format hints
	m_sFilePath.MakeLower();

	if( m_sFilePath.Find("no alpha") != -1 )		iAlphaBits = 0;
	else if( m_sFilePath.Find("1 alpha") != -1 )	iAlphaBits = 1;
	else if( m_sFilePath.Find("1alpha") != -1 )		iAlphaBits = 1;
	else if( m_sFilePath.Find("0alpha") != -1 )		iAlphaBits = 0;
	if( m_sFilePath.Find("dither") != -1 )			bDither = true; 

	/* Load the image into an SDL surface. */
	SDL_Surface *img = IMG_Load(m_sFilePath);

	/* Figure out which texture format to use. */
	D3DFORMAT fmtTexture;

	if( dwTextureColorDepth == 16 )	{
		/* Bits of alpha in the source: */
		int src_alpha_bits = 8 - img->format->Aloss;

		/* No real alpha in paletted input. */
		if( img->format->BytesPerPixel == 1 )
			src_alpha_bits = 0;

		/* Colorkeyed input effectively has at least one bit of alpha: */
		if( img->flags & SDL_SRCCOLORKEY )
			src_alpha_bits = max( 1, src_alpha_bits );

		/* Don't use more than we were hinted to. */
		src_alpha_bits = min( iAlphaBits, src_alpha_bits );

		/* XXX Scan the image, and see if it actually uses its alpha channel/color key
		* (if any).  Reduce to 1 or 0 bits of alpha if possible. */

		switch( src_alpha_bits ) {
		case 0:		fmtTexture = D3DFMT_R5G6B5;		break;
		case 1:		fmtTexture = D3DFMT_A1R5G5B5;	break;
		default:	fmtTexture = D3DFMT_A4R4G4B4;	break;
		}
	} else if( dwTextureColorDepth == 32 )
		fmtTexture = D3DFMT_A8R8G8B8;
	else
		throw RageException( "Invalid color depth: %d bits", dwTextureColorDepth );

	/* Cap the max texture size to the hardware max.  Is this needed--won't the
	 * texture creation do this for us? */
	dwMaxSize = min( dwMaxSize, int(DISPLAY->GetDeviceCaps().MaxTextureWidth) );

	/* Save information about the source.  Unless something else changes this
	 * later, the image inside the texture is the same size as the source. */
	m_iImageWidth = m_iSourceWidth = img->w;
	m_iImageHeight = m_iSourceHeight = img->h;

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(img->w);
	m_iTextureHeight = power_of_two(img->h);

	/* Cap the size. */
	m_iTextureWidth = min(m_iTextureWidth, int(dwMaxSize));
	m_iTextureHeight = min(m_iTextureHeight, int(dwMaxSize));

	if( FAILED( hr = m_pd3dDevice->CreateTexture(
		m_iTextureWidth,		// width 
		m_iTextureHeight,		// height 
		1,						// mip map levels  XXX: sending 4 breaks 2x2 pngs
		0,							// usage (is a render target?)
		fmtTexture,					// our preferred texture format
		D3DPOOL_MANAGED,			// which memory pool
		&m_pd3dTexture) ))
	{
		throw RageException( hr, "CreateTexture() failed for file '%s'.", m_sFilePath.GetString() );
	}

	{
		D3DSURFACE_DESC ddsd;
		if ( FAILED( hr = m_pd3dTexture->GetLevelDesc( 0, &ddsd ) ) ) 
			throw RageException( hr, "Could not get level Description of D3D texture!" );

		/* We might have received a texture of a different size or format than 
		 * we asked for. */
		m_iTextureWidth		= ddsd.Width;
		m_iTextureHeight	= ddsd.Height;
		fmtTexture			= ddsd.Format;
	}

	/* If the source is larger than the texture, we have to scale it down; that's
	 * "stretching", I guess. */
	if(m_iSourceWidth > m_iTextureWidth || m_iSourceHeight > m_iTextureHeight)
		bStretch = true;

	int target = PixFmtMaskNo(fmtTexture);

	/* Dither only when the target is 16bpp, not when it's 32bpp. */
	if( PixFmtMasks[target][4] /* XXX magic 4 */ == 4)
		bDither = false;

	if( bStretch ) {
		/* zoomSurface takes a ratio.  I'm not sure if it's always exact; we don't
		 * want to accidentally create a 513x513 texture, for example (it won't just
		 * cause lines; it'll be copied to the texture completely wrong). */
		
		/* resize currently only does RGBA8888 */
		{
			int mask = 0;
			ConvertSDLSurface(img, img->w, img->h, PixFmtMasks[mask][4],
				PixFmtMasks[mask][0], PixFmtMasks[mask][1], PixFmtMasks[mask][2], PixFmtMasks[mask][3]);
		}

		while (m_iImageWidth != m_iTextureWidth || m_iImageHeight != m_iTextureHeight) {
			float xscale = float(m_iTextureWidth)/m_iImageWidth;
			float yscale = float(m_iTextureHeight)/m_iImageHeight;

			/* Our filter is a simple linear filter, so it can't scale to 
			 * less than .5 very well.  If we need to go lower than .5, do
			 * it iteratively. */
			xscale = max(xscale, .5f);
			yscale = max(yscale, .5f);

			SDL_Surface *dst = zoomSurface(img, xscale, yscale);

			SDL_FreeSurface(img);
			img = dst;

			/* The new image size is the full texture size. */
			m_iImageWidth	= int(m_iImageWidth * xscale + .0001);
			m_iImageHeight	= int(m_iImageHeight * yscale + .0001);
		}
	}

	if( bDither )
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
	ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight, PixFmtMasks[target][4],
			PixFmtMasks[target][0], PixFmtMasks[target][1], PixFmtMasks[target][2], PixFmtMasks[target][3]);

	/* Copy the data to the target texture. */
	{
		D3DLOCKED_RECT d3dlr;
		if( FAILED( hr=m_pd3dTexture->LockRect(0, &d3dlr, 0, 0) ) )
			throw RageException( hr, "LockRect failed for file '%s'.", m_sFilePath.GetString() );

		memcpy( (byte *)(d3dlr.pBits), img->pixels, img->h*img->pitch );
		ASSERT( !FAILED( m_pd3dTexture->UnlockRect(0) ) ) ;
	}

	SDL_FreeSurface(img);
	LOG->Trace( "RageBitmapTexture: Loaded '%s' (%ux%u) from disk.  bStretch = %d, source %d,%d;  image %d,%d.", 
		m_sFilePath.GetString(), GetTextureWidth(), GetTextureHeight(),
		bStretch, m_iSourceWidth, m_iSourceHeight,
		m_iImageWidth,	m_iImageHeight);
}

#else

#include "DXUtil.h"
#include "dxerr8.h"

void RageBitmapTexture::Create( 
	int dwMaxSize, 
	int dwTextureColorDepth, 
	int iMipMaps,
	int iAlphaBits,
	bool bDither,
	bool bStretch
	)
{
	HRESULT hr;

	// look in the file name for a format hints
	m_sFilePath.MakeLower();

	if( -1 != m_sFilePath.Find("no alpha") )
		iAlphaBits = 0;
	else if( -1 != m_sFilePath.Find("1 alpha") )
		iAlphaBits = 1;
	else if( -1 != m_sFilePath.Find("1alpha") )
		iAlphaBits = 1;
	else if( -1 != m_sFilePath.Find("0alpha") )
		iAlphaBits = 0;
	if( -1 != m_sFilePath.Find("dither") )
		bDither = true; 

	
	/////////////////////
	// Get info about the bitmap
	/////////////////////
	D3DXIMAGE_INFO ddii;
	if( FAILED( hr = D3DXGetImageInfoFromFile(m_sFilePath,&ddii) ) )
	{
        throw RageException( hr, "D3DXGetImageInfoFromFile() failed for file '%s'.", m_sFilePath.GetString() );
	}


	///////////////////////
	// Figure out which texture format to use
	///////////////////////
	D3DFORMAT fmtTexture;
	if( dwTextureColorDepth == 32 )
		fmtTexture = D3DFMT_A8R8G8B8;
	else if( ddii.Format == D3DFMT_P8 )
		fmtTexture = D3DFMT_A1R5G5B5;
	else // dwTextureColorDepth == 16
	{
		switch( iAlphaBits )
		{
		case 0:	fmtTexture = D3DFMT_R5G6B5;		break;
		case 1:	fmtTexture = D3DFMT_A1R5G5B5;	break;
		case 4:	fmtTexture = D3DFMT_A4R4G4B4;	break;
		default:	ASSERT(0);	fmtTexture = D3DFMT_A4R4G4B4;	break;
		}
	}


	// find out what the min texture size is
	dwMaxSize = min( dwMaxSize, int(DISPLAY->GetDeviceCaps().MaxTextureWidth) );

	bStretch |= int(ddii.Width) > dwMaxSize || int(ddii.Height) > dwMaxSize;
	
	// HACK:  On a Voodoo3 and Win98, D3DXCreateTextureFromFileEx fail randomly on rare occasions.
	// So, we'll try the call 2x in a row in case the first one fails.
	for( int i=0; i<2; i++ )
	{
		if( FAILED( hr = D3DXCreateTextureFromFileEx( 
			m_pd3dDevice,				// device
			m_sFilePath,				// soure file
			D3DX_DEFAULT,				// width 
			D3DX_DEFAULT,				// height 
			iMipMaps,					// mip map levels
			0,							// usage (is a render target?)
			fmtTexture,					// our preferred texture format
			D3DPOOL_MANAGED,			// which memory pool
			(bStretch ? D3DX_FILTER_LINEAR : D3DX_FILTER_NONE) | (bDither ? D3DX_FILTER_DITHER : 0),		// filter
			D3DX_FILTER_BOX | (bDither ? D3DX_FILTER_DITHER : 0),				// mip filter
			D3DCOLOR_ARGB(255,255,0,255), // pink color key
			&ddii,						// struct to fill with source image info
			NULL,						// no palette
			&m_pd3dTexture ) ) )
		{
			if( i==0 )
			{
				LOG->Trace( "WARNING! D3DXCreateTextureFromFileEx failed.  Sleep and try one more time..." );
				::Sleep( 10 );
				continue;
			}
			throw RageException( hr, "D3DXCreateTextureFromFileEx() failed for file '%s'.", m_sFilePath.GetString() );
		}
		else
			break;
	}

	/////////////////////
	// Save information about the texture
	/////////////////////
	m_iSourceWidth = ddii.Width;
	m_iSourceHeight= ddii.Height;

	D3DSURFACE_DESC ddsd;
	if ( FAILED( hr = m_pd3dTexture->GetLevelDesc( 0, &ddsd ) ) ) 
		throw RageException( hr, "Could not get level Description of D3DX texture!" );

	// save information about the texture
	m_iTextureWidth		= ddsd.Width;
	m_iTextureHeight	= ddsd.Height;


	if( bStretch )
	{
		m_iImageWidth	= m_iTextureWidth;
		m_iImageHeight	= m_iTextureHeight;
	}
	else
	{
		m_iImageWidth	= m_iSourceWidth;
		m_iImageHeight	= m_iSourceHeight;
	}

	LOG->Trace( "RageBitmapTexture: Loaded '%s' (%ux%u) from disk.  bStretch = %d, source %d,%d;  image %d,%d.", 
		m_sFilePath.GetString(), 
		GetTextureWidth(), 
		GetTextureHeight(),
		bStretch,
		m_iSourceWidth,
		m_iSourceHeight,
		m_iImageWidth,
		m_iImageHeight
		);
}

#endif