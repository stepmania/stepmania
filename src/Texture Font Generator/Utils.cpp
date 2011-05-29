#include "stdafx.h"
#include "Utils.h"

Surface::Surface( const Surface &cpy )
{
	iWidth = cpy.iWidth;
	iHeight = cpy.iHeight;
	iPitch = cpy.iPitch;
	if( cpy.pRGBA )
	{
		pRGBA = new unsigned char[iHeight * iPitch];
	}
	else
	{
		pRGBA = NULL;
	}
}

void BitmapToSurface( HBITMAP hBitmap, Surface *pSurf )
{
	HDC hDC = CreateCompatibleDC( NULL );
	HGDIOBJ hOldBitmap = SelectObject( hDC, hBitmap );

	BITMAPINFO bi;
	memset( &bi, 0, sizeof(bi) );
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	GetDIBits( hDC, hBitmap, 0, 0, NULL, &bi, DIB_RGB_COLORS );

	pSurf->iWidth = bi.bmiHeader.biWidth;
	pSurf->iHeight = bi.bmiHeader.biHeight;
	pSurf->iPitch = bi.bmiHeader.biWidth * 4;
	pSurf->pRGBA = (unsigned char *) new unsigned char[pSurf->iWidth * pSurf->iHeight * 4];

	bi.bmiHeader.biHeight = -bi.bmiHeader.biHeight;
	bi.bmiHeader.biPlanes = 1; 
	bi.bmiHeader.biBitCount = 32; 
	bi.bmiHeader.biCompression = BI_RGB; 
	bi.bmiHeader.biSizeImage = pSurf->iHeight * pSurf->iWidth * 4;
//LONG   biXPelsPerMeter; 
//LONG   biYPelsPerMeter; 
// DWORD  biClrUsed; 
// DWORD  biClrImportant; 

	if( !GetDIBits(hDC, hBitmap, 0, pSurf->iHeight, pSurf->pRGBA, &bi, DIB_RGB_COLORS) )
	{
		int q = 1;
	}

	SelectObject( hDC, hOldBitmap );
	DeleteDC( hDC );
}

void GrayScaleToAlpha( Surface *pSurf )//void *pImg, int iWidth, int iHeight, int iPitch )
{
	char *p = (char *) pSurf->pRGBA;
	for( int iY = 0; iY < pSurf->iHeight; ++iY )
	{
		unsigned *pos = (unsigned *) p;
		for( int iX = 0; iX < pSurf->iWidth; ++iX )
		{
			unsigned pixel = pos[iX];
			pixel <<= 24;
			pixel |= 0x00FFFFFF;
			pos[iX] = pixel;
		}
		p += pSurf->iPitch;
	}
}

void GetBounds( const Surface *pSurf, RECT *out )
{
	out->left = out->right = out->top = out->bottom = 0;

	int FirstCol = 9999, LastCol = 0, FirstRow = 9999, LastRow = 0;
	const char *p = (const char *) pSurf->pRGBA;
	for( int row = 0; row < pSurf->iHeight; ++row )
	{
		const unsigned *pos = (const unsigned *) p;
		for( int col = 0; col < pSurf->iWidth; ++col )
		{
			unsigned pixel = pos[col];
			if( (pixel & 0xFFFFFF) == 0 )
				continue; /* black */

			FirstCol = min(FirstCol, col);
			LastCol = max(LastCol, col);
			FirstRow = min(FirstRow, row);
			LastRow = max(LastRow, row);
		}
		p += pSurf->iPitch;
	}

	if( FirstCol != 9999 && FirstRow != 9999 )
	{
		out->left = FirstCol;
		out->top = FirstRow;
		out->right = LastCol+1;
		out->bottom = LastRow+1;
	}
}


#pragma include_alias( "zlib/zlib.h", "../zlib/zlib.h" )
#include "png.h"
#if defined(_MSC_VER)
#  pragma comment(lib, "libpng.lib")
#pragma warning(disable: 4611) /* interaction between '_setjmp' and C++ object destruction is non-portable */
#endif

static void File_png_write( png_struct *pPng, png_byte *pData, png_size_t iSize )
{
	FILE *f = (FILE *) png_get_io_ptr(pPng);
	size_t iGot = fwrite( pData, (int) iSize, 1, f );
	if( iGot == 0 )
		png_error( pPng, strerror(errno) );
}

static void File_png_flush( png_struct *pPng )
{
	FILE *f = (FILE *) png_get_io_ptr(pPng);
	int iGot = fflush(f);
	if( iGot == -1 )
		png_error( pPng, strerror(errno) );
}

struct error_info
{
	char *szErr;
};

static void PNG_Error( png_struct *pPng, const char *szError )
{
	error_info *pInfo = (error_info *) png_get_error_ptr(pPng);
	strncpy( pInfo->szErr, szError, 1024 );
	pInfo->szErr[1023] = 0;
	longjmp( png_jmpbuf(pPng), 1 );
}

static void PNG_Warning( png_struct *png, const char *warning )
{
}

/* Since libpng forces us to use longjmp, this function shouldn't create any C++
 * objects, and needs to watch out for memleaks. */
bool SavePNG( FILE *f, char szErrorbuf[1024], const Surface *pSurf )
{
/*	RageSurfaceUtils::ConvertSurface( pImgIn, pImg, pImgIn->w, pImgIn->h, 32,
			Swap32BE( 0xFF000000 ),
			Swap32BE( 0x00FF0000 ),
			Swap32BE( 0x0000FF00 ),
			Swap32BE( 0x000000FF ) );
*/
	error_info error;
	error.szErr = szErrorbuf;

	png_struct *pPng = png_create_write_struct( PNG_LIBPNG_VER_STRING, &error, PNG_Error, PNG_Warning );
	if( pPng == NULL )
	{
		sprintf( szErrorbuf, "creating png_create_write_struct failed");
		return false;
	}

	png_info *pInfo = png_create_info_struct(pPng);
	if( pInfo == NULL )
	{
		png_destroy_read_struct( &pPng, NULL, NULL );
		sprintf( szErrorbuf, "creating png_create_info_struct failed");
		return false;
	}

	if( setjmp(png_jmpbuf(pPng)) )
	{
		png_destroy_read_struct( &pPng, &pInfo, NULL );
		return false;
	}

	png_set_write_fn( pPng, f, File_png_write, File_png_flush );
	png_set_compression_level( pPng, 1 );

	png_set_IHDR( pPng, pInfo, pSurf->iWidth, pSurf->iHeight, 8, PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

	png_write_info( pPng, pInfo );
	png_set_filler( pPng, 0, PNG_FILLER_AFTER );

	png_byte *pixels = (png_byte *) pSurf->pRGBA;
	for( int y = 0; y < pSurf->iHeight; y++ )
		png_write_row( pPng, pixels + pSurf->iPitch*y );

	png_write_end( pPng, pInfo );
	png_destroy_write_struct( &pPng, &pInfo );

	return true;
}

/*
 * Copyright (c) 2003-2007 Glenn Maynard
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
