#include "global.h"
#include "RageSurface.h"

bool RageSurfaceColor::operator== ( const RageSurfaceColor &rhs ) const
{
	return rhs.r == r && rhs.g == g && rhs.b == b && rhs.a == a;
}

int32_t RageSurfacePalette::FindColor( const RageSurfaceColor &color ) const
{
	for( int i = 0; i < ncolors; ++i )
		if( colors[i] == color )
			return i;
	return -1;
}

/* XXX: untested */
int32_t RageSurfacePalette::FindClosestColor( const RageSurfaceColor &color ) const
{
	int iBest = -1;
	int iBestDist = 99999;
	for( int i = 0; i < ncolors; ++i )
	{
		if( colors[i] == color )
			return i;

		int iDist = abs( colors[i].r - color.r ) +
			abs( colors[i].g - color.g ) +
			abs( colors[i].b - color.b ) +
			abs( colors[i].a - color.a );
		if( iDist < iBestDist )
		{
			iBestDist = iDist ;
			iBest = i;
		}
	}

	return iBest;
}

void RageSurfaceFormat::GetRGB( uint32_t val, uint8_t *r, uint8_t *g, uint8_t *b ) const
{
	if( BytesPerPixel == 1 )
	{
		ASSERT( palette );
		*r = palette->colors[val].r;
		*g = palette->colors[val].g;
		*b = palette->colors[val].b;
	} else {
		*r = int8_t( (val & Rmask) >> Rshift << Rloss );
		*g = int8_t( (val & Gmask) >> Gshift << Gloss );
		*b = int8_t( (val & Bmask) >> Bshift << Bloss );
//		*a = int8_t( (val & Amask) >> Ashift << Aloss );
	}
}

bool RageSurfaceFormat::MapRGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint32_t &val ) const
{
	if( BytesPerPixel == 1 )
	{
		RageSurfaceColor c( r, g, b, a );
		int32_t n = palette->FindColor( c );
		if( n == -1 )
			return false;
		val = (uint32_t) n;
	} else {
		val  = (r >> Rloss << Rshift) |
			(g >> Gloss << Gshift) |
			(b >> Bloss << Bshift) |
			(a >> Aloss << Ashift);
	}
	return true;
}

bool RageSurfaceFormat::operator== ( const RageSurfaceFormat &rhs ) const
{
#define COMP(a) if( a != rhs.a ) return false;
	COMP( BytesPerPixel );
	COMP( Rmask );
	COMP( Gmask );
	COMP( Bmask );
	COMP( Amask );
	if( BytesPerPixel == 1 )
		COMP( palette );

	return true;
}

RageSurface::RageSurface()
{
	format = &fmt;
	pixels = NULL;
}

RageSurface::~RageSurface()
{
	delete [] pixels;
}

static int GetShiftFromMask( uint32_t mask )
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

static int GetBitsFromMask( uint32_t mask )
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
						 int width, int height, int BitsPerPixel, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask )
{
	fmt.BitsPerPixel = BitsPerPixel;
	fmt.BytesPerPixel = BitsPerPixel/8;
	if( fmt.BytesPerPixel == 1 )
	{
		fmt.Rmask = fmt.Gmask = fmt.Bmask = fmt.Amask = 0;
		fmt.Rshift = fmt.Gshift = fmt.Bshift = fmt.Ashift = 0;

		/* Loss for paletted textures is zero, since the actual palette entries are 8-bit. */
		fmt.Rloss = fmt.Gloss = fmt.Bloss = fmt.Aloss = 0;

		fmt.palette = new RageSurfacePalette;
		fmt.palette->ncolors = 256;
	}
	else
	{
		fmt.Rmask = Rmask;
		fmt.Gmask = Gmask;
		fmt.Bmask = Bmask;
		fmt.Amask = Amask;

		fmt.Rshift = GetShiftFromMask( Rmask );
		fmt.Gshift = GetShiftFromMask( Gmask );
		fmt.Bshift = GetShiftFromMask( Bmask );
		fmt.Ashift = GetShiftFromMask( Amask );

		fmt.Rloss = (uint8_t) (8-GetBitsFromMask( Rmask ));
		fmt.Gloss = (uint8_t) (8-GetBitsFromMask( Gmask ));
		fmt.Bloss = (uint8_t) (8-GetBitsFromMask( Bmask ));
		fmt.Aloss = (uint8_t) (8-GetBitsFromMask( Amask ));
	}
}

RageSurface *CreateSurface( int width, int height, int BitsPerPixel, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask )
{
	RageSurface *pImg = new RageSurface;

	SetupFormat( pImg->fmt, width, height, BitsPerPixel, Rmask, Gmask, Bmask, Amask );

	pImg->w = width;
	pImg->h = height;
	pImg->flags = 0;
	pImg->pitch = width*BitsPerPixel/8;
	pImg->pixels = new uint8_t[ pImg->pitch*height ];

//	if( BitsPerPixel == 8 )
//	{
//		pImg->fmt.palette = new RageSurfacePalette;
//	}

	return pImg;
}

RageSurface *CreateSurfaceFrom( int width, int height, int BitsPerPixel, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask, uint8_t *pPixels, uint32_t pitch )
{
	RageSurface *pImg = new RageSurface;

	SetupFormat( pImg->fmt, width, height, BitsPerPixel, Rmask, Gmask, Bmask, Amask );

	pImg->w = width;
	pImg->h = height;
	pImg->flags = 0;
	pImg->pitch = pitch;
	pImg->pixels = pPixels;

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

