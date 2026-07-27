#include "global.h"
#include "RageSurface.h"
#include "RageUtil.h"

#include <climits>
#include <cmath>
#include <cstdint>


std::int32_t RageSurfacePalette::FindColor( const RageSurfaceColor &color ) const
{
	for( int i = 0; i < ncolors; ++i )
		if( colors[i] == color )
			return i;
	return -1;
}

/* XXX: untested */
std::int32_t RageSurfacePalette::FindClosestColor( const RageSurfaceColor &color ) const
{
	int iBest = -1;
	int iBestDist = INT_MAX;
	for( int i = 0; i < ncolors; ++i )
	{
		if( colors[i] == color )
			return i;

		int iDist = std::abs( colors[i].r - color.r ) +
			std::abs( colors[i].g - color.g ) +
			std::abs( colors[i].b - color.b ) +
			std::abs( colors[i].a - color.a );
		if( iDist < iBestDist )
		{
			iBestDist = iDist ;
			iBest = i;
		}
	}

	return iBest;
}

RageSurfaceFormat::RageSurfaceFormat():
	Mask(), Shift(), Loss(),
	Rmask(Mask[0]), Gmask(Mask[1]), Bmask(Mask[2]), Amask(Mask[3]),
	Rshift(Shift[0]), Gshift(Shift[1]), Bshift(Shift[2]), Ashift(Shift[3])
{
}

RageSurfaceFormat::RageSurfaceFormat( const RageSurfaceFormat &cpy ):
	Mask(), Shift(), Loss(),
	Rmask(Mask[0]), Gmask(Mask[1]), Bmask(Mask[2]), Amask(Mask[3]),
	Rshift(Shift[0]), Gshift(Shift[1]), Bshift(Shift[2]), Ashift(Shift[3])
{
	BytesPerPixel = cpy.BytesPerPixel;
	BitsPerPixel = cpy.BitsPerPixel;
	Mask = cpy.Mask;
	Shift = cpy.Shift;
	Loss = cpy.Loss;
	if( palette ) {
		palette = std::make_unique<RageSurfacePalette>(*palette);
	}
}

void RageSurfaceFormat::GetRGB( std::uint32_t val, std::uint8_t *r, std::uint8_t *g, std::uint8_t *b ) const
{
	if( BytesPerPixel == 1 )
	{
		ASSERT( palette != nullptr );
		*r = palette->colors[val].r;
		*g = palette->colors[val].g;
		*b = palette->colors[val].b;
	} else {
		*r = std::int8_t( (val & Mask[0]) >> Shift[0] << Loss[0] );
		*g = std::int8_t( (val & Mask[1]) >> Shift[1] << Loss[1] );
		*b = std::int8_t( (val & Mask[2]) >> Shift[2] << Loss[2] );
	}
}

bool RageSurfaceFormat::MapRGBA( std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a, std::uint32_t &val ) const
{
	if( BytesPerPixel == 1 )
	{
		RageSurfaceColor c( r, g, b, a );
		std::int32_t n = palette->FindColor( c );
		if( n == -1 )
			return false;
		val = (std::uint32_t) n;
	} else {
		val  =
			(r >> Loss[0] << Shift[0]) |
			(g >> Loss[1] << Shift[1]) |
			(b >> Loss[2] << Shift[2]) |
			(a >> Loss[3] << Shift[3]);
	}
	return true;
}

bool RageSurfaceFormat::operator== ( const RageSurfaceFormat &rhs ) const
{
	if( !Equivalent(rhs) )
		return false;

	if( BytesPerPixel == 1 )
		if( memcmp( palette.get(), rhs.palette.get(), sizeof(RageSurfaceFormat) ) )
			return false;

	return true;
}

bool RageSurfaceFormat::Equivalent( const RageSurfaceFormat &rhs ) const
{
#define COMP(a) if( a != rhs.a ) return false;
	COMP( BytesPerPixel );
	COMP( Rmask );
	COMP( Gmask );
	COMP( Bmask );
	COMP( Amask );

	return true;
}

RageSurface::RageSurface()
{
	format = &fmt;
	pixels = nullptr;
	pixels_owned = true;
}

RageSurface::RageSurface( const RageSurface &cpy )
{
	format = &fmt;

	w = cpy.w;
	h = cpy.h;
	pitch = cpy.pitch;
	flags = cpy.flags;
	pixels_owned = true;
	if( cpy.pixels )
	{
		pixels = new std::uint8_t[ pitch*h ];
		memcpy( pixels, cpy.pixels, pitch*h );
	}
	else
		pixels = nullptr;
}

RageSurface::~RageSurface()
{
	if( pixels_owned )
		delete [] pixels;
}

static int GetShiftFromMask( std::uint32_t mask )
{
	if( !mask )
		return 0;

	int iShift = 0;
	while( (mask & 1) == 0 )
	{
		mask >>= 1;
		++iShift;
	}
	return iShift;
}

static int GetBitsFromMask( std::uint32_t mask )
{
	if( !mask )
		return 0;

	mask >>= GetShiftFromMask(mask);

	int iBits = 0;
	while( (mask & 1) == 1 )
	{
		mask >>= 1;
		++iBits;
	}
	return iBits;
}


void SetupFormat( RageSurfaceFormat &fmt,
						 int width, int height, int BitsPerPixel, std::uint32_t Rmask, std::uint32_t Gmask, std::uint32_t Bmask, std::uint32_t Amask )
{
	fmt.BitsPerPixel = BitsPerPixel;
	fmt.BytesPerPixel = BitsPerPixel/8;
	if( fmt.BytesPerPixel == 1 )
	{
		ZERO( fmt.Mask );
		ZERO( fmt.Shift );

		// Loss for paletted textures is zero; the actual palette entries are 8-bit.
		ZERO( fmt.Loss );

		fmt.palette = std::make_unique<RageSurfacePalette>();
		fmt.palette->ncolors = 256;
	}
	else
	{
		fmt.Mask[0] = Rmask;
		fmt.Mask[1] = Gmask;
		fmt.Mask[2] = Bmask;
		fmt.Mask[3] = Amask;

		fmt.Shift[0] = GetShiftFromMask( Rmask );
		fmt.Shift[1] = GetShiftFromMask( Gmask );
		fmt.Shift[2] = GetShiftFromMask( Bmask );
		fmt.Shift[3] = GetShiftFromMask( Amask );

		fmt.Loss[0] = (std::uint8_t) (8-GetBitsFromMask( Rmask ));
		fmt.Loss[1] = (std::uint8_t) (8-GetBitsFromMask( Gmask ));
		fmt.Loss[2] = (std::uint8_t) (8-GetBitsFromMask( Bmask ));
		fmt.Loss[3] = (std::uint8_t) (8-GetBitsFromMask( Amask ));
	}
}

RageSurface *CreateSurface( int width, int height, int BitsPerPixel, std::uint32_t Rmask, std::uint32_t Gmask, std::uint32_t Bmask, std::uint32_t Amask )
{
	RageSurface *pImg = new RageSurface;

	SetupFormat( pImg->fmt, width, height, BitsPerPixel, Rmask, Gmask, Bmask, Amask );

	pImg->w = width;
	pImg->h = height;
	pImg->flags = 0;
	pImg->pitch = width*BitsPerPixel/8;
	pImg->pixels = new std::uint8_t[ pImg->pitch*height ];

	/*
	if( BitsPerPixel == 8 )
	{
		pImg->fmt.palette = new RageSurfacePalette;
	}
	*/

	return pImg;
}

RageSurface *CreateSurfaceFrom( int width, int height, int BitsPerPixel, std::uint32_t Rmask, std::uint32_t Gmask, std::uint32_t Bmask, std::uint32_t Amask, std::uint8_t *pPixels, std::uint32_t pitch )
{
	RageSurface *pImg = new RageSurface;

	SetupFormat( pImg->fmt, width, height, BitsPerPixel, Rmask, Gmask, Bmask, Amask );

	pImg->w = width;
	pImg->h = height;
	pImg->flags = 0;
	pImg->pitch = pitch;
	pImg->pixels = pPixels;
	pImg->pixels_owned = false;

	return pImg;
}

/*
 * (c) 2001-2004 Glenn Maynard, Chris Danford
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

