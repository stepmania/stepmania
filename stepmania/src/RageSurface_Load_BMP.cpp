#include "global.h"
#include "RageSurface_Load_BMP.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageSurface.h"
using namespace FileReading;

/* Tested with http://entropymine.com/jason/bmpsuite/. */

enum
{
	COMP_BI_RGB = 0,
	COMP_BI_RLE4, /* unsupported */
	COMP_BI_RLE8, /* unsupported */
	COMP_BI_BITFIELDS
};


/* When returning error, the first error encountered takes priority. */
#define FATAL_ERROR(s) \
{ \
	if( sError.size() == 0 ) sError = (s); \
	return RageSurfaceUtils::OPEN_FATAL_ERROR; \
}

static RageSurfaceUtils::OpenResult LoadBMP( RageFile &f, RageSurface *&img, RString &sError )
{
	char magic[2];
	ReadBytes( f, magic, 2, sError );
	if( magic[0] != 'B' || magic[1] != 'M' )
	{
		sError = "not a BMP";
		return RageSurfaceUtils::OPEN_UNKNOWN_FILE_FORMAT;
	}

	img = NULL;

	read_u32_le( f, sError ); /* file size */
	read_u32_le( f, sError ); /* unused */
	uint32_t iDataOffset = read_u32_le( f, sError );
	uint32_t iHeaderSize = read_u32_le( f, sError );


	uint32_t iWidth, iHeight, iPlanes, iBPP, iCompression = COMP_BI_RGB, iColors = 0;
	if( iHeaderSize == 12 )
	{
		/* OS/2 format */
		iWidth = read_u16_le( f, sError );
		iHeight = read_u16_le( f, sError );
		iPlanes = read_u16_le( f, sError );
		iBPP = read_u16_le( f, sError );
	}
	else if( iHeaderSize == 40 )
	{
		iWidth = read_u32_le( f, sError );
		iHeight = read_u32_le( f, sError );
		iPlanes = read_u16_le( f, sError );
		iBPP = read_u16_le( f, sError );
		iCompression = read_u32_le( f, sError );
		read_u32_le( f, sError ); /* bitmap size */
		read_u32_le( f, sError ); /* horiz resolution */
		read_u32_le( f, sError ); /* vert resolution */
		iColors = read_u32_le( f, sError );
		read_u32_le( f, sError ); /* "important" colors */
	}
	else
		FATAL_ERROR( ssprintf( "expected header size of 40, got %u", iHeaderSize ) );

	if( iBPP <= 8 && iColors == 0 )
		iColors = 1 << iBPP;
	if( iPlanes != 1 )
		FATAL_ERROR( ssprintf( "expected one plane, got %u", iPlanes ) );
	if( iBPP != 1 && iBPP != 4 && iBPP != 8 && iBPP != 16 && iBPP != 24 && iBPP != 32 )
		FATAL_ERROR( ssprintf( "unsupported bpp %u", iBPP ) );
	if( iCompression != COMP_BI_RGB && iCompression != COMP_BI_BITFIELDS )
		FATAL_ERROR( ssprintf( "unsupported compression %u", iCompression ) );

	if( iCompression == COMP_BI_BITFIELDS && iBPP <= 8 )
		FATAL_ERROR( ssprintf( "BI_BITFIELDS unexpected with bpp %u", iBPP ) );

	int iFileBPP = iBPP;
	iBPP = max( iBPP, 8u );

	int Rmask = 0, Gmask = 0, Bmask = 0, Amask = 0;
	switch( iBPP )
	{
	case 16:
		Rmask = Swap16LE( 0x7C00 );
		Gmask = Swap16LE( 0x03E0 );
		Bmask = Swap16LE( 0x001F );
		break;
	case 24:
		Rmask = Swap24LE( 0xFF0000 );
		Gmask = Swap24LE( 0x00FF00 );
		Bmask = Swap24LE( 0x0000FF );
		break;
	case 32:
		Rmask = Swap32LE( 0x00FF0000 );
		Gmask = Swap32LE( 0x0000FF00 );
		Bmask = Swap32LE( 0x000000FF );
		break;
	}

	if( iCompression == COMP_BI_BITFIELDS )
	{
		Rmask = read_u32_le( f, sError );
		Gmask = read_u32_le( f, sError );
		Bmask = read_u32_le( f, sError );
	}

	/* Stop on error before we use any of the values we just read. */
	if( sError.size() != 0 )
		return RageSurfaceUtils::OPEN_FATAL_ERROR;

	img = CreateSurface( iWidth, iHeight, iBPP, Rmask, Gmask, Bmask, Amask );

	if( iBPP == 8 )
	{
		RageSurfaceColor Palette[256];
		ZERO( Palette );

		if( iColors > 256 )
			FATAL_ERROR( ssprintf( "unexpected colors %i", iColors ) );

		for( unsigned i = 0; i < iColors; ++i )
		{
			Palette[i].b = read_8( f, sError );
			Palette[i].g = read_8( f, sError );
			Palette[i].r = read_8( f, sError );
			Palette[i].a = 0xFF;
			/* Windows BMP palettes are padded to 32bpp.  */
			if( iHeaderSize == 40 )
				read_8( f, sError );
		}

		memcpy( img->fmt.palette->colors, Palette, sizeof(Palette) );
	}

	/* Stop on error before we seek, so we don't return the wrong error message. */
	if( sError.size() != 0 )
		return RageSurfaceUtils::OPEN_FATAL_ERROR;

	int iFilePitch = iFileBPP * iWidth; // in bits
	iFilePitch = (iFilePitch+7) / 8; // in bytes: round up
	iFilePitch = (iFilePitch+3) & ~3; // round up a multiple of 4

	{
		int ret = f.Seek( iDataOffset );
		if( ret == -1 )
			FATAL_ERROR( f.GetError() );
		if( ret != (int) iDataOffset )
			FATAL_ERROR( "Unexpected end of file" );
	}

	for( int y = (int) iHeight-1; y >= 0; --y )
	{
		uint8_t *pRow = img->pixels + img->pitch*y;
		RString buf;

		f.Read( buf, iFilePitch );

		/* Expand 1- and 4-bits to 8-bits. */
		if( iFileBPP == 1 )
		{
			for( unsigned x = 0; x < iWidth; ++x )
			{
				int iByteNo = x >> 3;
				int iBitNo = 7-(x&7);
				int iBit = 1 << iBitNo;
				pRow[x] = !!(buf[iByteNo] & iBit);
			}
		}
		else if( iFileBPP == 4 )
		{
			for( unsigned x = 0; x < iWidth; ++x )
			{
				if( (x & 1) == 0 )
					pRow[x] = buf[x/2] & 0x0F;
				else
					pRow[x] = (buf[x/2] >> 4) & 0x0F;
			}
		}
		else
			memcpy( pRow, buf.data(), img->pitch );
	}

	return sError.size() != 0? RageSurfaceUtils::OPEN_FATAL_ERROR: RageSurfaceUtils::OPEN_OK;
}

RageSurfaceUtils::OpenResult RageSurface_Load_BMP( const RString &sPath, RageSurface *&img, bool bHeaderOnly, RString &error )
{
	RageFile f;

	if( !f.Open( sPath ) )
	{
		error = f.GetError();
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	}

	RageSurfaceUtils::OpenResult ret;
	img = NULL;
	ret = LoadBMP( f, img, error );

	if( ret != RageSurfaceUtils::OPEN_OK && img != NULL )
	{
		delete img;
		img = NULL;
	}

	return ret;
}

/*
 * Copyright (c) 2004 Glenn Maynard
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
