/* Utility functions for RageSurfaces. */

#ifndef RAGE_SURFACE_UTILS_H
#define RAGE_SURFACE_UTILS_H

#include <cstdint>

struct RageSurfaceColor;
struct RageSurfacePalette;
struct RageSurfaceFormat;
struct RageSurface;

/** @brief Utility functions for the RageSurfaces. */
namespace RageSurfaceUtils
{
	std::uint32_t decodepixel( const std::uint8_t *p, int bpp );
	void encodepixel( std::uint8_t *p, int bpp, std::uint32_t pixel );

	void GetRawRGBAV( std::uint32_t pixel, const RageSurfaceFormat &fmt, std::uint8_t *v );
	void GetRawRGBAV( const std::uint8_t *p, const RageSurfaceFormat &fmt, std::uint8_t *v );
	void GetRGBAV( std::uint32_t pixel, const RageSurface *src, std::uint8_t *v );
	void GetRGBAV( const std::uint8_t *p, const RageSurface *src, std::uint8_t *v );

	std::uint32_t SetRawRGBAV( const RageSurfaceFormat *fmt, const std::uint8_t *v );
	void SetRawRGBAV( std::uint8_t *p, const RageSurface *src, const std::uint8_t *v );
	std::uint32_t SetRGBAV( const RageSurfaceFormat *fmt, const std::uint8_t *v );
	void SetRGBAV( std::uint8_t *p, const RageSurface *src, const std::uint8_t *v );

	/* Get the number of bits representing each color channel in fmt. */
	void GetBitsPerChannel( const RageSurfaceFormat *fmt, std::uint32_t bits[4] );

	void CopySurface( const RageSurface *src, RageSurface *dest );
	bool ConvertSurface( const RageSurface *src, RageSurface *&dst,
		int width, int height, int bpp, std::uint32_t R, std::uint32_t G, std::uint32_t B, std::uint32_t A );
	void ConvertSurface( RageSurface *&image,
		int width, int height, int bpp, std::uint32_t R, std::uint32_t G, std::uint32_t B, std::uint32_t A );

	void FixHiddenAlpha( RageSurface *img );

	int FindSurfaceTraits( const RageSurface *img );

	/* The surface contains no transparent pixels and/or never uses its color
	 * key, so it doesn't need any alpha bits at all. */
	enum { TRAIT_NO_TRANSPARENCY = 0x0001 }; /* 0alpha */

	/* The surface contains only transparent values of 0 or 1; no translucency.
	 * It only needs one bit of alpha. */
	enum { TRAIT_BOOL_TRANSPARENCY = 0x0002 }; /* 1alpha */

	void BlitTransform( const RageSurface *src, RageSurface *dst,
					const float fCoords[8] /* TL, BR, BL, TR */ );

	void Blit( const RageSurface *src, RageSurface *dst, int width = -1, int height = -1 );
	void CorrectBorderPixels( RageSurface *img, int width, int height );

	bool SaveSurface( const RageSurface *img, RString file );
	RageSurface *LoadSurface( RString file );

	/* Quickly palettize to an gray/alpha texture. */
	RageSurface *PalettizeToGrayscale( const RageSurface *src_surf, unsigned int GrayBits, unsigned int AlphaBits );

	RageSurface *MakeDummySurface( int height, int width );

	void ApplyHotPinkColorKey( RageSurface *&img );
	void FlipVertically( RageSurface *img );
};

#endif

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
