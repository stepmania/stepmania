#include "global.h"
#include "RageSurface_Save_BMP.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageUtil.h"
#include "RageFile.h"

struct FatalError: public RageException  { FatalError(const CString &str): RageException(str) { } };

static void WriteBytes( RageFile &f, const void *buf, int size )
{
	int ret = f.Write( buf, size );
	if( ret == -1 )
		throw FatalError( f.GetError() );
}

static void write_le16( RageFile &f, uint16_t val )
{
	val = Swap16LE( val );
	WriteBytes( f, &val, sizeof(uint16_t) );
}

static void write_le32( RageFile &f, uint32_t val )
{
	val = Swap32LE( val );
	WriteBytes( f, &val, sizeof(uint32_t) );
}

bool RageSurface_Save_BMP( RageSurface *surface, RageFile &f )
{
	/* Convert the surface to 24bpp. */
	RageSurface *converted_surface;
	converted_surface = CreateSurface( surface->w, surface->h, 24,
		Swap24LE( 0xFF0000 ), Swap24LE( 0x00FF00 ), Swap24LE( 0x0000FF ), 0 );
	RageSurfaceUtils::CopySurface( surface, converted_surface );

try {
	int iFilePitch = converted_surface->pitch;
	iFilePitch = (iFilePitch+3) & ~3; // round up a multiple of 4

	int iDataSize = converted_surface->h * iFilePitch;
	const int iHeaderSize = 0x36;

	WriteBytes( f, "BM", 2 );
	write_le32( f, iHeaderSize+iDataSize ); // size (offset 0x2)
	write_le32( f, 0 ); // reserved (offset 0x6)
	write_le32( f, iHeaderSize ); // bitmap offset (offset 0xA)
	write_le32( f, 0x28 ); // header size (offset 0xE)
	write_le32( f, surface->w ); // width (offset 0x14)
	write_le32( f, surface->h ); // height (offset 0x18)
	write_le16( f, 1 ); // planes (offset 0x1A)
	write_le16( f, (uint16_t) converted_surface->fmt.BytesPerPixel*8 ); // bpp (offset 0x1C)
	write_le32( f, 0 ); // compression (offset 0x1E)
	write_le32( f, iDataSize ); // bitmap size (offset 0x22)
	write_le32( f, 0 ); // horiz resolution (offset 0x26)
	write_le32( f, 0 ); // vert resolution (offset 0x2A)
	write_le32( f, 0 ); // colors (offset 0x2E)
	write_le32( f, 0 ); // important colors (offset 0x32)

	for( int y = converted_surface->h-1; y >= 0; --y )
	{
		const uint8_t *pRow = converted_surface->pixels + converted_surface->pitch*y;
		WriteBytes( f, pRow, converted_surface->pitch );

		/* Pad */
		uint8_t padding[4] = { 0,0,0,0 };
		WriteBytes( f, padding, iFilePitch-converted_surface->pitch );
	}
} catch(...) {
	delete converted_surface;
	return false;
}

	delete converted_surface;

	if( f.Flush() == -1 )
		return false;

	return true;
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
