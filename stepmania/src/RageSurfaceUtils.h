/* Utility functions for RageSurfaces. */

#ifndef RAGE_SURFACE_UTILS_H
#define RAGE_SURFACE_UTILS_H

struct RageSurfaceColor;
struct RageSurfacePalette;
struct RageSurfaceFormat;
struct RageSurface;

namespace RageSurfaceUtils
{
	uint32_t decodepixel( const uint8_t *p, int bpp );
	void encodepixel( uint8_t *p, int bpp, uint32_t pixel );

	void GetRawRGBAV( uint32_t pixel, const RageSurfaceFormat &fmt, uint8_t *v );
	void GetRawRGBAV( const uint8_t *p, const RageSurfaceFormat &fmt, uint8_t *v );
	void GetRGBAV( uint32_t pixel, const RageSurface *src, uint8_t *v );
	void GetRGBAV( const uint8_t *p, const RageSurface *src, uint8_t *v );

	uint32_t SetRawRGBAV( const RageSurfaceFormat *fmt, const uint8_t *v );
	void SetRawRGBAV( uint8_t *p, const RageSurface *src, const uint8_t *v );
	uint32_t SetRGBAV( const RageSurfaceFormat *fmt, const uint8_t *v );
	void SetRGBAV( uint8_t *p, const RageSurface *src, const uint8_t *v );
	
	/* Get the number of bits representing each color channel in fmt. */
	void GetBitsPerChannel( const RageSurfaceFormat *fmt, uint32_t bits[4] );

	void CopySurface( const RageSurface *src, RageSurface *dest );
	bool ConvertSurface( RageSurface *src, RageSurface *&dst,
		int width, int height, int bpp, uint32_t R, uint32_t G, uint32_t B, uint32_t A );
	void ConvertSurface( RageSurface *&image,
		int width, int height, int bpp, uint32_t R, uint32_t G, uint32_t B, uint32_t A );

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

	void Blit( const RageSurface *src, RageSurface *dst, int width, int height );

	bool SaveSurface( const RageSurface *img, CString file );
	RageSurface *LoadSurface( CString file );

	/* Quickly palettize to an gray/alpha texture. */
	RageSurface *PalettizeToGrayscale( const RageSurface *src_surf, int GrayBits, int AlphaBits );

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
