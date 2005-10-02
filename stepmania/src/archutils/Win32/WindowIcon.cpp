#include "global.h"
#include "archutils/Win32/WindowIcon.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageSurface_Load.h"

#include <wingdi.h>

HICON IconFromSurface( const RageSurface *pSrcImg )
{
	RageSurface *pImg;

	{
		/* Round the width up to a multiple of 8, convert to 32-bit BGR, and reduce
		 * to one-bit alpha. */
		int iWidth = pSrcImg->w;
		iWidth = (iWidth+7) & ~7;

		pImg = CreateSurface( iWidth, pSrcImg->h, 32,
			0x00FF0000,
			0x0000FF00,
			0x000000FF,
			0xFF000000 );
		RageSurfaceUtils::Blit( pSrcImg, pImg );
	}

	RageSurfaceUtils::FlipVertically( pImg );

	int iSize = sizeof(BITMAPINFOHEADER);
	int iSizeImage = 0;
	iSizeImage += pImg->h * pImg->pitch; /* image */
	iSizeImage += (pImg->h * pImg->w) / 8; /* mask */

	BITMAPINFOHEADER *pBitmap = (BITMAPINFOHEADER *) malloc( iSize + iSizeImage );
	memset( pBitmap, 0, iSize + iSizeImage );

	pBitmap->biSize  = sizeof(BITMAPINFOHEADER);
	pBitmap->biWidth = pImg->w;
	pBitmap->biHeight = pImg->h * 2;
	pBitmap->biPlanes = 1;
	pBitmap->biBitCount = 32;
	pBitmap->biCompression = BI_RGB;
	pBitmap->biSizeImage = pImg->h * pImg->pitch;

	uint8_t *pImage = ((uint8_t *) pBitmap) + iSize;
	uint8_t *pMask = pImage + pImg->h * pImg->pitch;

	memcpy( pImage, pImg->pixels, pImg->h * pImg->pitch );

	int iMaskPitch = pImg->w/8;
	for( int y = 0; y < pImg->h; ++y )
	{
		int bit = 0x80;
		uint32_t *pRow = (uint32_t *) (pImage + y*pImg->pitch);
		uint8_t *pMaskRow = pMask + y*iMaskPitch;
		for( int x = 0; x < pImg->w; ++x )
		{
			if( !(pRow[x] & pImg->fmt.Mask[3]) )
			{
				/* Transparent; set this mask bit. */
				*pMaskRow |= bit;
				pRow[x] = 0;
			}

			bit >>= 1;
			if( bit == 0 )
			{
				bit = 0x80;
				++pMaskRow;
			}
		}
	}

	HICON icon = CreateIconFromResourceEx( (BYTE *) pBitmap, iSize + iSizeImage, TRUE, 0x00030000, pImg->w, pImg->h, LR_DEFAULTCOLOR );

	delete pImg;
	pImg = NULL;
	free( pBitmap );

	if( icon == NULL )
	{
		LOG->Trace( "%s", werr_ssprintf( GetLastError(), "CreateIconFromResourceEx" ).c_str() );
		return NULL;
	}

	return icon;
}

HICON IconFromFile( const CString &sIconFile )
{
	CString sError;
	RageSurface *pImg = RageSurfaceUtils::LoadFile( sIconFile, sError );
	if( pImg == NULL )
	{
		LOG->Warn( "Couldn't open icon \"%s\": %s", sIconFile.c_str(), sError.c_str() );
		return NULL;
	}

	HICON icon = IconFromSurface( pImg );
	delete pImg;
	return icon;
}

/*
 * (c) 2004 Glenn Maynard
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
