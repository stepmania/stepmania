#include "global.h"
#include "RageSurface_Load_BMP.h"
#include "RageFile.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageSurface.h"

/* Tested with http://entropymine.com/jason/bmpsuite/. */

enum
{
	COMP_BI_RGB = 0,
	COMP_BI_RLE4, /* unsupported */
	COMP_BI_RLE8, /* unsupported */
	COMP_BI_BITFIELDS
};

struct NotBMP: public RageException  { NotBMP(): RageException("not a BMP") { } };
struct FatalError: public RageException  { FatalError(const CString &str): RageException(str) { } };
struct UnexpectedEOF: public RageException  { UnexpectedEOF(): RageException("Unexpected EOF") { } };


static void ReadBytes( RageFile &f, void *buf, int size )
{
	int ret = f.Read( buf, size );
	if( ret == -1 )
		throw FatalError( f.GetError() );

	if( ret < size )
		throw UnexpectedEOF();
}

static uint8_t read_8( RageFile &f )
{
	uint8_t val;
	ReadBytes( f, &val, sizeof(uint8_t) );
    return val;
}

static uint16_t read_le16( RageFile &f )
{
	uint16_t val;
	ReadBytes( f, &val, sizeof(uint16_t) );
    return Swap16LE( val );
}
	
static uint32_t read_le32( RageFile &f )
{
	uint32_t val;
	ReadBytes( f, &val, sizeof(uint32_t) );
    return Swap32LE( val );
}

static RageSurface *LoadBMP( RageFile &f, RageSurface *&ret )
{
	char magic[2];
	ReadBytes( f, magic, 2 );
	if( magic[0] != 'B' || magic[1] != 'M' )
		throw NotBMP();

	read_le32( f ); /* file size */
	read_le32( f ); /* unused */
	uint32_t iDataOffset = read_le32( f );
	uint32_t iHeaderSize = read_le32( f );

	uint32_t iWidth, iHeight, iPlanes, iBPP, iCompression = COMP_BI_RGB, iColors = 0;
	if( iHeaderSize == 12 )
	{
		/* OS/2 format */
		iWidth = read_le16( f );
		iHeight = read_le16( f );
		iPlanes = read_le16( f );
		iBPP = read_le16( f );
	}
	else if( iHeaderSize == 40 )
	{
		iWidth = read_le32( f );
		iHeight = read_le32( f );
		iPlanes = read_le16( f );
		iBPP = read_le16( f );
		iCompression = read_le32( f );
		read_le32( f ); /* bitmap size */
		read_le32( f ); /* horiz resolution */
		read_le32( f ); /* vert resolution */
		iColors = read_le32( f );
		read_le32( f ); /* "important" colors */
	}
	else
		throw FatalError( ssprintf( "expected header size of 40, got %u", iHeaderSize ) );

	if( iBPP <= 8 && iColors == 0 )
		iColors = 1 << iBPP;
	if( iPlanes != 1 )
		throw FatalError( ssprintf( "expected one plane, got %u", iPlanes ) );
	if( iBPP != 1 && iBPP != 4 && iBPP != 8 && iBPP != 16 && iBPP != 24 && iBPP != 32 )
		throw FatalError( ssprintf( "unsupported bpp %u", iBPP ) );
	if( iCompression != COMP_BI_RGB && iCompression != COMP_BI_BITFIELDS )
		throw FatalError( ssprintf( "unsupported compression %u", iCompression ) );

	if( iCompression == COMP_BI_BITFIELDS && iBPP <= 8 )
		throw FatalError( ssprintf( "BI_BITFIELDS unexpected with bpp %u", iBPP ) );

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
		Rmask = read_le32( f );
		Gmask = read_le32( f );
		Bmask = read_le32( f );
	}

	RageSurface *img = CreateSurface( iWidth, iHeight, iBPP, Rmask, Gmask, Bmask, Amask );

try { /* file reading may throw */
	if( iBPP == 8 )
	{
		RageSurfaceColor Palette[256];
		ZERO( Palette );

		if( iColors > 256 )
			throw FatalError( ssprintf( "unexpected colors %i", iColors ) );

		for( unsigned i = 0; i < iColors; ++i )
		{
			Palette[i].b = read_8( f );
			Palette[i].g = read_8( f );
			Palette[i].r = read_8( f );
			Palette[i].a = 0xFF;
			/* Windows BMP palettes are padded to 32bpp.  */
			if( iHeaderSize == 40 )
				read_8( f );
		}

		memcpy( img->fmt.palette->colors, Palette, sizeof(Palette) );
	}

	int iFilePitch = iFileBPP * iWidth; // in bits
	iFilePitch = (iFilePitch+7) / 8; // in bytes: round up
	iFilePitch = (iFilePitch+3) & ~3; // round up a multiple of 4

	{
		int ret = f.Seek( iDataOffset );
		if( ret == -1 )
			throw FatalError( f.GetError() );
		if( ret != (int) iDataOffset )
			throw UnexpectedEOF();
	}

	for( int y = (int) iHeight-1; y >= 0; --y )
	{
		uint8_t *pRow = img->pixels + img->pitch*y;
		CString buf;

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
} catch(...) {
	delete img;
	throw;
}

	return img;
}

RageSurfaceUtils::OpenResult RageSurface_Load_BMP( const CString &sPath, RageSurface *&ret, bool bHeaderOnly, CString &error )
{
	RageFile f;

	if( !f.Open( sPath ) )
	{
		error = f.GetError();
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	}

	try {
		ret = LoadBMP( f, ret );
	} catch( const NotBMP & ) {
		error = "not a BMP";
		return RageSurfaceUtils::OPEN_UNKNOWN_FILE_FORMAT;
	} catch( const FatalError &err ) {
		error = err.what();
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	} catch( const UnexpectedEOF & ) {
		error = "unexpected end of file";
		return RageSurfaceUtils::OPEN_FATAL_ERROR;
	}

	return RageSurfaceUtils::OPEN_OK;
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
