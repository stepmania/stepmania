#include "global.h"
#include "RageSurfaceUtils.h"
#include "RageSurface.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageFile.h"

uint32_t RageSurfaceUtils::decodepixel( const uint8_t *p, int bpp )
{
    switch(bpp)
	{
    case 1: return *p;
    case 2: return *(uint16_t *)p;
	case 3:
        if( BYTE_ORDER == BIG_ENDIAN )
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4: return *(uint32_t *)p;
    default: return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void RageSurfaceUtils::encodepixel( uint8_t *p, int bpp, uint32_t pixel )
{
    switch(bpp)
	{
    case 1: *p = uint8_t(pixel); break;
    case 2: *(uint16_t *)p = uint16_t(pixel); break;
    case 3:
        if( BYTE_ORDER == BIG_ENDIAN )
		{
            p[0] = uint8_t((pixel >> 16) & 0xff);
            p[1] = uint8_t((pixel >> 8) & 0xff);
            p[2] = uint8_t(pixel & 0xff);
        } else {
            p[0] = uint8_t(pixel & 0xff);
            p[1] = uint8_t((pixel >> 8) & 0xff);
            p[2] = uint8_t((pixel >> 16) & 0xff);
        }
        break;
    case 4: *(uint32_t *)p = pixel; break;
    }
}

/* Get and set colors without scaling to 0..255. */
void RageSurfaceUtils::GetRawRGBAV( uint32_t pixel, const RageSurfaceFormat &fmt, uint8_t *v )
{
	if( fmt.BytesPerPixel == 1 )
	{
		v[0] = fmt.palette->colors[pixel].r;
		v[1] = fmt.palette->colors[pixel].g;
		v[2] = fmt.palette->colors[pixel].b;
		v[3] = fmt.palette->colors[pixel].a;
	} else {
		v[0] = uint8_t((pixel & fmt.Rmask) >> fmt.Rshift);
		v[1] = uint8_t((pixel & fmt.Gmask) >> fmt.Gshift);
		v[2] = uint8_t((pixel & fmt.Bmask) >> fmt.Bshift);
		v[3] = uint8_t((pixel & fmt.Amask) >> fmt.Ashift);
	}
}

void RageSurfaceUtils::GetRawRGBAV( const uint8_t *p, const RageSurfaceFormat &fmt, uint8_t *v )
{
	uint32_t pixel = decodepixel( p, fmt.BytesPerPixel );
	GetRawRGBAV( pixel, fmt, v );
}

void RageSurfaceUtils::GetRGBAV( uint32_t pixel, const RageSurface *src, uint8_t *v )
{
	GetRawRGBAV(pixel, src->fmt, v);
	const RageSurfaceFormat *fmt = src->format;
	for( int c = 0; c < 4; ++c )
		v[c] = v[c] << fmt->Loss[c];

	// Correct for surfaces that don't have an alpha channel.
	if( fmt->Loss[3] == 8 )
		v[3] = 255;
}

void RageSurfaceUtils::GetRGBAV( const uint8_t *p, const RageSurface *src, uint8_t *v )
{
	uint32_t pixel = decodepixel(p, src->format->BytesPerPixel);
	if( src->format->BytesPerPixel == 1 ) // paletted
	{
		memcpy( v, &src->format->palette->colors[pixel], sizeof(RageSurfaceColor));
	}
	else	// RGBA
		GetRGBAV(pixel, src, v);	
}


/* Inverse of GetRawRGBAV. */
uint32_t RageSurfaceUtils::SetRawRGBAV( const RageSurfaceFormat *fmt, const uint8_t *v )
{
	return 	v[0] << fmt->Rshift |
			v[1] << fmt->Gshift |
			v[2] << fmt->Bshift |
			v[3] << fmt->Ashift;
}

void RageSurfaceUtils::SetRawRGBAV( uint8_t *p, const RageSurface *src, const uint8_t *v )
{
	uint32_t pixel = SetRawRGBAV(src->format, v);
	encodepixel(p, src->format->BytesPerPixel, pixel);
}

/* Inverse of GetRGBAV. */
uint32_t RageSurfaceUtils::SetRGBAV( const RageSurfaceFormat *fmt, const uint8_t *v )
{
	return 	(v[0] >> fmt->Loss[0]) << fmt->Shift[0] |
			(v[1] >> fmt->Loss[1]) << fmt->Shift[1] |
			(v[2] >> fmt->Loss[2]) << fmt->Shift[2] |
			(v[3] >> fmt->Loss[3]) << fmt->Shift[3];
}

void RageSurfaceUtils::SetRGBAV( uint8_t *p, const RageSurface *src, const uint8_t *v )
{
	uint32_t pixel = SetRGBAV(src->format, v);
	encodepixel(p, src->format->BytesPerPixel, pixel);
}


void RageSurfaceUtils::GetBitsPerChannel( const RageSurfaceFormat *fmt, uint32_t bits[4] )
{
	/* The actual bits stored in each color is 8-loss.  */
	for( int c = 0; c < 4; ++c )
		bits[c] = 8 - fmt->Loss[c];
}

void RageSurfaceUtils::CopySurface( const RageSurface *src, RageSurface *dest )
{
	/* Copy the palette, if we have one. */
	if( src->format->BitsPerPixel == 8 && dest->format->BitsPerPixel == 8 )
	{
		ASSERT( dest->fmt.palette );
		*dest->fmt.palette = *src->fmt.palette;
	}

	Blit( src, dest, -1, -1 );
}

bool RageSurfaceUtils::ConvertSurface( RageSurface *src, RageSurface *&dst,
		int width, int height, int bpp,
		uint32_t R, uint32_t G, uint32_t B, uint32_t A )
{
    dst = CreateSurface( width, height, bpp, R, G, B, A );

	/* If the formats are the same, no conversion is needed.  Ignore the palette. */
	if( width == src->w && height == src->h && src->format->Equivalent( *dst->format ) )
	{
		delete dst;
		dst = NULL;
		return false;
	}

	CopySurface( src, dst );
	return true;
}

void RageSurfaceUtils::ConvertSurface(RageSurface *&image,
		int width, int height, int bpp,
		uint32_t R, uint32_t G, uint32_t B, uint32_t A)
{
    RageSurface *ret_image;
	if( !ConvertSurface( image, ret_image, width, height, bpp, R, G, B, A ) )
		return;

	delete image;
	image = ret_image;
}


/* Local helper for FixHiddenAlpha. */
static void FindAlphaRGB(const RageSurface *img, uint8_t &r, uint8_t &g, uint8_t &b, bool reverse)
{
	r = g = b = 0;
	
	/* If we have no alpha, there's no alpha color. */
	if( img->format->BitsPerPixel > 8 && !img->format->Amask )
		return;

	/* Eww.  Sorry.  Iterate front-to-back or in reverse. */
	for(int y = reverse? img->h-1:0;
		reverse? (y >=0):(y < img->h); reverse? (--y):(++y))
	{
		uint8_t *row = (uint8_t *)img->pixels + img->pitch*y;
		if(reverse)
			row += img->format->BytesPerPixel * (img->w-1);

		for(int x = 0; x < img->w; ++x)
		{
			uint32_t val = RageSurfaceUtils::decodepixel(row, img->format->BytesPerPixel);
			if( img->format->BitsPerPixel == 8 )
			{
				if( img->format->palette->colors[val].a )
				{
					/* This color isn't fully transparent, so grab it. */
					r = img->format->palette->colors[val].r;
					g = img->format->palette->colors[val].g;
					b = img->format->palette->colors[val].b;
					return;
				}
			}
			else
			{
				if( val & img->format->Amask )
				{
					/* This color isn't fully transparent, so grab it. */
					img->format->GetRGB( val, &r, &g, &b );
					return;
				}
			}

			if( reverse )
				row -= img->format->BytesPerPixel;
			else
				row += img->format->BytesPerPixel;
		}
	}

	/* Huh?  The image is completely transparent. */
	r = g = b = 0;
}

/* Local helper for FixHiddenAlpha. Set the underlying RGB values of all pixels in
 * img that are completely transparent. */
static void SetAlphaRGB(const RageSurface *img, uint8_t r, uint8_t g, uint8_t b)
{
	/* If it's a paletted surface, all we have to do is change the palette. */
	if( img->format->BitsPerPixel == 8 )
	{
		for( int c = 0; c < img->format->palette->ncolors; ++c )
		{
			if( img->format->palette->colors[c].a )
				continue;
			img->format->palette->colors[c].r = r;
			img->format->palette->colors[c].g = g;
			img->format->palette->colors[c].b = b;
		}
		return;
	}


	/* If it's RGBA and there's no alpha channel, we have nothing to do. */
	if( img->format->BitsPerPixel > 8 && !img->format->Amask )
		return;

	uint32_t trans;
	img->format->MapRGBA( r, g, b, 0, trans );
	for( int y = 0; y < img->h; ++y )
	{
		uint8_t *row = img->pixels + img->pitch*y;

		for( int x = 0; x < img->w; ++x )
		{
			uint32_t val = RageSurfaceUtils::decodepixel( row, img->format->BytesPerPixel );
			if( val != trans && !(val&img->format->Amask) )
			{
				RageSurfaceUtils::encodepixel( row, img->format->BytesPerPixel, trans );
			}

			row += img->format->BytesPerPixel;
		}
	}
}

/* When we scale up images (which we always do in high res), pixels
 * that are completely transparent can be blended with opaque pixels,
 * causing their RGB elements to show.  This is visible in many textures
 * as a pixel-wide border in the wrong color.  This is tricky to fix.
 * We need to set the RGB components of completely transparent pixels
 * to a reasonable color.
 *
 * Most images have a single border color.  For these, the transparent
 * color is easy: search through the image top-bottom-left-right,
 * find the first non-transparent pixel, and pull out its RGB.
 *
 * A few images don't.  We can only make a guess here.  What we'll do
 * is, after the above search, do the same in reverse (bottom-top-right-
 * left).  If the color we find is different, we'll just set the border
 * color to black.  
 */
void RageSurfaceUtils::FixHiddenAlpha( RageSurface *img )
{
	/* If there are no alpha bits, there's nothing to fix. */
	if( img->format->BitsPerPixel != 8 && img->format->Amask == 0 )
		return;

	uint8_t r, g, b;
	FindAlphaRGB(img, r, g, b, false);

	uint8_t cr, cg, cb; /* compare */
	FindAlphaRGB(img, cr, cg, cb, true);

	if( cr != r || cg != g || cb != b )
		r = g = b = 0;

	SetAlphaRGB(img, r, g, b);
}

/* Scan the surface to see what level of alpha it uses.  This can be used to
 * find the best surface format for a texture; eg. a TRAIT_BOOL_TRANSPARENCY or
 * TRAIT_NO_TRANSPARENCY surface can use RGB5A1 instead of RGBA4 for greater
 * color resolution; a TRAIT_NO_TRANSPARENCY could also use R5G6B5. */
int RageSurfaceUtils::FindSurfaceTraits( const RageSurface *img )
{
	const int NEEDS_NO_ALPHA=0, NEEDS_BOOL_ALPHA=1, NEEDS_FULL_ALPHA=2;
	int alpha_type = NEEDS_NO_ALPHA;

	uint32_t max_alpha;
	if( img->format->BitsPerPixel == 8 )
	{
		/* Short circuit if we already know we have no transparency. */
		bool bHaveNonOpaque = false;
		for( int c = 0; !bHaveNonOpaque && c < img->format->palette->ncolors; ++c )
		{
			if( img->format->palette->colors[c].a != 0xFF )
				bHaveNonOpaque = true;
		}

		if( !bHaveNonOpaque )
			return TRAIT_NO_TRANSPARENCY;

		max_alpha = 0xFF;
	}
	else
	{
		/* Short circuit if we already know we have no transparency. */
		if( img->format->Amask == 0 )
			return TRAIT_NO_TRANSPARENCY;

		max_alpha = img->format->Amask;
	}

	for(int y = 0; y < img->h; ++y)
	{
		uint8_t *row = (uint8_t *)img->pixels + img->pitch*y;

		for(int x = 0; x < img->w; ++x)
		{
			uint32_t val = decodepixel(row, img->format->BytesPerPixel);

			uint32_t alpha;
			if( img->format->BitsPerPixel == 8 )
				alpha = img->format->palette->colors[val].a;
			else
				alpha = (val & img->format->Amask);

			if( alpha == 0 )
				alpha_type = max( alpha_type, NEEDS_BOOL_ALPHA );
			else if( alpha != max_alpha )
				alpha_type = max( alpha_type, NEEDS_FULL_ALPHA );

			row += img->format->BytesPerPixel;
		}
	}

	int ret = 0;
	switch( alpha_type ) 
	{
	case NEEDS_NO_ALPHA:	ret |= TRAIT_NO_TRANSPARENCY;	break;
	case NEEDS_BOOL_ALPHA:	ret |= TRAIT_BOOL_TRANSPARENCY;	break;
	case NEEDS_FULL_ALPHA:									break;
	default:	ASSERT(0);
	}

	return ret;
}


/* Local helper for BlitTransform. */
static inline void GetRawRGBAV_XY( const RageSurface *src, uint8_t *v, int x, int y )
{
	const uint8_t *srcp = (const uint8_t *) src->pixels + (y * src->pitch);
	const uint8_t *srcpx = srcp + (x * src->fmt.BytesPerPixel);

	RageSurfaceUtils::GetRawRGBAV( srcpx, src->fmt, v );
}

static inline float scale( float x, float l1, float h1, float l2, float h2 )
{
	return ((x - l1) / (h1 - l1) * (h2 - l2) + l2);
}

/* Completely unoptimized. */
void RageSurfaceUtils::BlitTransform( const RageSurface *src, RageSurface *dst, 
					const float fCoords[8] /* TL, BR, BL, TR */ )
{
	ASSERT( src->format->BytesPerPixel == dst->format->BytesPerPixel );

	const float Coords[8] = {
		(fCoords[0] * (src->w)), (fCoords[1] * (src->h)),
		(fCoords[2] * (src->w)), (fCoords[3] * (src->h)),
		(fCoords[4] * (src->w)), (fCoords[5] * (src->h)),
		(fCoords[6] * (src->w)), (fCoords[7] * (src->h))
	};

	const int TL_X = 0, TL_Y = 1, BL_X = 2, BL_Y = 3,
			  BR_X = 4, BR_Y = 5, TR_X = 6, TR_Y = 7;

	for( int y = 0; y < dst->h; ++y )
	{
		uint8_t *dstp = (uint8_t *) dst->pixels + (y * dst->pitch); /* line */
		uint8_t *dstpx = dstp; /* pixel */

		const float start_y = scale(float(y), 0, float(dst->h), Coords[TL_Y], Coords[BL_Y]);
		const float end_y = scale(float(y), 0, float(dst->h), Coords[TR_Y], Coords[BR_Y]);

		const float start_x = scale(float(y), 0, float(dst->h), Coords[TL_X], Coords[BL_X]);
		const float end_x = scale(float(y), 0, float(dst->h), Coords[TR_X], Coords[BR_X]);

		for( int x = 0; x < dst->w; ++x )
		{
			const float src_xp = scale(float(x), 0, float(dst->w), start_x, end_x);
			const float src_yp = scale(float(x), 0, float(dst->w), start_y, end_y);

			/* If the surface is two pixels wide, src_xp is 0..2.  .5 indicates
			 * pixel[0]; 1 indicates 50% pixel[0], 50% pixel[1]; 1.5 indicates
			 * pixel[1]; 2 indicates 50% pixel[1], 50% pixel[2] (which is clamped
			 * to pixel[1]). */
			int src_x[2], src_y[2];
			src_x[0] = (int) truncf(src_xp - 0.5f);
			src_x[1] = src_x[0] + 1;

			src_y[0] = (int) truncf(src_yp - 0.5f);
			src_y[1] = src_y[0] + 1;

			/* Emulate GL_REPEAT. */
			src_x[0] = clamp(src_x[0], 0, src->w);
			src_x[1] = clamp(src_x[1], 0, src->w);
			src_y[0] = clamp(src_y[0], 0, src->h);
			src_y[1] = clamp(src_y[1], 0, src->h);

			/* Decode our four pixels. */
			uint8_t v[4][4];
			GetRawRGBAV_XY(src, v[0], src_x[0], src_y[0]);
			GetRawRGBAV_XY(src, v[1], src_x[0], src_y[1]);
			GetRawRGBAV_XY(src, v[2], src_x[1], src_y[0]);
			GetRawRGBAV_XY(src, v[3], src_x[1], src_y[1]);

			/* Distance from the pixel chosen: */
			float weight_x = src_xp - (src_x[0] + 0.5f);
			float weight_y = src_yp - (src_y[0] + 0.5f);

			/* Filter: */
			uint8_t out[4] = { 0,0,0,0 };
			for(int i = 0; i < 4; ++i)
			{
				float sum = 0;
				sum += v[0][i] * (1-weight_x) * (1-weight_y);
				sum += v[1][i] * (1-weight_x) * (weight_y);
				sum += v[2][i] * (weight_x)   * (1-weight_y);
				sum += v[3][i] * (weight_x)   * (weight_y);
				out[i] = (uint8_t) clamp( lrintf(sum), 0, 255 );
			}

			/* If the source has no alpha, set the destination to opaque. */
			if( src->format->Amask == 0 )
				out[3] = uint8_t( dst->format->Amask >> dst->format->Ashift );

			SetRawRGBAV(dstpx, dst, out);

			dstpx += dst->format->BytesPerPixel;
		}
	}
}


/*
 * Simplified:
 *
 * No source alpha.
 * Palette -> palette blits assume the palette is identical (no mapping).
 * No color key.
 * No general blitting rects.
 */

static bool blit_same_type( const RageSurface *src_surf, const RageSurface *dst_surf, int width, int height )
{
	if( src_surf->format->BytesPerPixel != dst_surf->format->BytesPerPixel ||
		src_surf->format->Rmask != dst_surf->format->Rmask ||
		src_surf->format->Gmask != dst_surf->format->Gmask ||
		src_surf->format->Bmask != dst_surf->format->Bmask ||
		src_surf->format->Amask != dst_surf->format->Amask )
		return false;

	const uint8_t *src = src_surf->pixels;
	uint8_t *dst = dst_surf->pixels;

	/* If possible, memcpy the whole thing. */
	if( src_surf->w == width && dst_surf->w == width && src_surf->pitch == dst_surf->pitch )
	{
        memcpy( dst, src, height*src_surf->pitch );
		return true;
	}

	/* The rows don't line up, so memcpy row by row. */
	while( height-- )
	{
        memcpy( dst, src, width*src_surf->format->BytesPerPixel );
        src += src_surf->pitch;
        dst += dst_surf->pitch;
	}

	return true;
}

/* Rescaling blit with no ckey.  This is used to update movies in
 * D3D, so optimization is very important. */
static bool blit_rgba_to_rgba( const RageSurface *src_surf, const RageSurface *dst_surf, int width, int height )
{
	if( src_surf->format->BytesPerPixel == 1 || dst_surf->format->BytesPerPixel == 1 )
		return false;

	const uint8_t *src = src_surf->pixels;
	uint8_t *dst = dst_surf->pixels;

	/* Bytes to skip at the end of a line. */
	const int srcskip = src_surf->pitch - width*src_surf->format->BytesPerPixel;
	const int dstskip = dst_surf->pitch - width*dst_surf->format->BytesPerPixel;

	const uint32_t *src_shifts = src_surf->format->Shift;
	const uint32_t *dst_shifts = dst_surf->format->Shift;
	const uint32_t *src_masks = src_surf->format->Mask;
	const uint32_t *dst_masks = dst_surf->format->Mask;

	uint8_t lookup[4][256];
	for( int c = 0; c < 4; ++c )
	{
		const uint32_t max_src_val = src_masks[c] >> src_shifts[c];
		const uint32_t max_dst_val = dst_masks[c] >> dst_shifts[c];
		ASSERT( max_src_val <= 0xFF );
		ASSERT( max_dst_val <= 0xFF );

		if( src_masks[c] == 0 )
		{
			/* The source is missing a channel.  Alpha defaults to opaque, other
			 * channels default to 0. */
			if( c == 3 )
				lookup[c][0] = (uint8_t) max_dst_val;
			else
				lookup[c][0] = 0;
		} else {
			/*
			 * Calculate a color conversion table.  There are a few ways we can do
			 * this (each list is the resulting table for 4->2 bit):
			 *
			 * SCALE( i, 0, max_src_val+1, 0, max_dst_val+1 );
			 * { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3 }
			 * SCALE( i, 0, max_src_val, 0, max_dst_val );
			 * { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3 }
			 * lrintf( ((float) i / max_src_val) * max_dst_val )
			 * { 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3 }
			 *
			 * We use the first for increasing resolution, since it gives the most even
			 * distribution.
			 *
			 * 2->4 bit:
			 * SCALE( i, 0, max_src_val+1, 0, max_dst_val+1 );
			 * { 0, 4, 8, 12 }
			 * SCALE( i, 0, max_src_val, 0, max_dst_val );
			 * { 0, 5, 10, 15 }
			 * lrintf( ((float) i / max_src_val) * max_dst_val )
			 * { 0, 5, 10, 15 }
			 *
			 * The latter two are equivalent and give an even distribution; we use the
			 * second, since the first doesn't scale max_src_val to max_dst_val.
			 *
			 * Having separate formulas for increasing and decreasing resolution seems
			 * strange; what's wrong here?
			 */
			if( max_src_val > max_dst_val )
				for( uint32_t i = 0; i <= max_src_val; ++i )
					lookup[c][i] = (uint8_t) SCALE( i, 0, max_src_val+1, 0, max_dst_val+1 );
			else
				for( uint32_t i = 0; i <= max_src_val; ++i )
					lookup[c][i] = (uint8_t) SCALE( i, 0, max_src_val, 0, max_dst_val );
		}
	}

	while( height-- )
	{
		int x = 0;
		while( x++ < width )
		{
			unsigned int pixel = RageSurfaceUtils::decodepixel( src, src_surf->format->BytesPerPixel );

			/* Convert pixel to the destination format. */
			unsigned int opixel = 0;
			for( int c = 0; c < 4; ++c )
			{
				int src = (pixel & src_masks[c]) >> src_shifts[c];
				opixel |= lookup[c][src] << dst_shifts[c];
			}

			/* Store it. */
			RageSurfaceUtils::encodepixel( dst, dst_surf->format->BytesPerPixel, opixel );

			src += src_surf->format->BytesPerPixel;
			dst += dst_surf->format->BytesPerPixel;
		}

		src += srcskip;
		dst += dstskip;
	}

	return true;
}

static bool blit_generic( const RageSurface *src_surf, const RageSurface *dst_surf, int width, int height )
{
	if( src_surf->format->BytesPerPixel != 1 || dst_surf->format->BytesPerPixel == 1 )
		return false;

	const uint8_t *src = src_surf->pixels;
	uint8_t *dst = dst_surf->pixels;

	/* Bytes to skip at the end of a line. */
	const int srcskip = src_surf->pitch - width*src_surf->format->BytesPerPixel;
	const int dstskip = dst_surf->pitch - width*dst_surf->format->BytesPerPixel;

	while( height-- )
	{
		int x = 0;
		while( x++ < width )
		{
			unsigned int pixel = RageSurfaceUtils::decodepixel( src, src_surf->format->BytesPerPixel );

			uint8_t colors[4];
				/* Convert pixel to the destination RGBA. */
				colors[0] = src_surf->format->palette->colors[pixel].r;
				colors[1] = src_surf->format->palette->colors[pixel].g;
				colors[2] = src_surf->format->palette->colors[pixel].b;
				colors[3] = src_surf->format->palette->colors[pixel].a;
			pixel = RageSurfaceUtils::SetRGBAV(dst_surf->format, colors);

			/* Store it. */
			RageSurfaceUtils::encodepixel( dst, dst_surf->format->BytesPerPixel, pixel );

			src += src_surf->format->BytesPerPixel;
			dst += dst_surf->format->BytesPerPixel;
		}

		src += srcskip;
		dst += dstskip;
	}

	return true;
}

/* Blit src onto dst. */
void RageSurfaceUtils::Blit( const RageSurface *src, RageSurface *dst, int width, int height )
{
	if( width == -1 )
		width = src->w;
	if( height == -1 )
		height = src->h;
	width = min( src->w, dst->w );
	height = min( src->h, dst->h );

	/* Try each blit until we find one that works; run them in order of efficiency,
	 * so we use the fastest blit possible. */
	do
	{
		/* RGBA->RGBA with the same format, or PAL->PAL.  Simple copy. */
		if( blit_same_type(src, dst, width, height) )
			break;

		/* RGBA->RGBA with different formats. */
		if( blit_rgba_to_rgba(src, dst, width, height) )
			break;

		/* PAL->RGBA. */
		if( blit_generic(src, dst, width, height) )
			break;

		/* We don't do RGBA->PAL. */
		ASSERT(0);
	} while(0);

	/*
	 * The destination surface may be larger than the source.  For example, we may be
	 * blitting a 200x200 image onto a 256x256 surface for OpenGL.  Normally, that extra
	 * space isn't actually used; we'll only render the image space.  However, bilinear
	 * filtering will cause the lines of pixels at 201x... and ...x201 to be visible. We
	 * need to make sure those pixels make sense.
	 *
	 * Previously, we just cleared the image to transparent or the color key.  This
	 * has two problems.  First, we may not have space for a color key (an image with
	 * 256 non-transparent palette colors).  Second, that's not completely correct;
	 * it'll force the outside border of the image to filter to transparent.  If the image
	 * is being tiled with another image, that may leave seams.
	 *
	 * (In some cases, filtering to transparent is preferable, particularly when displaying
	 * a sprite in perspective.  If you want that, add blank space to the image explicitly.)
	 *
	 * Copy the last column (200x... -> 201x...), then the last row (...x200 -> ...x201).
	 */

	CorrectBorderPixels( dst, width, height );
}

/*
 * If only width x height of img is actually going to be used, and there's extra
 * space on the surface, duplicate the last row and column to ensure that we don't
 * pull in unexpected data when rendering with bilinear filtering.
 *
 * We do this if there's memory available, even if that space extends outside
 * of the image (in the per-line padding or after the end).  This way, surfaces
 * can be padded to power-of-two dimensions by the image loaders, and if no other
 * adjustments are needed, they can be passed directly to the renderer without
 * doing any extra copies.
 */
void RageSurfaceUtils::CorrectBorderPixels( RageSurface *img, int width, int height )
{
	if( width*img->fmt.BytesPerPixel < img->pitch )
	{
		/* Duplicate the last column. */
		int offset = img->format->BytesPerPixel * (width-1);
		uint8_t *p = (uint8_t *) img->pixels + offset;

		for( int y = 0; y < height; ++y )
		{
			uint32_t pixel = decodepixel( p, img->format->BytesPerPixel );
			encodepixel( p+img->format->BytesPerPixel, img->format->BytesPerPixel, pixel );

			p += img->pitch;
		}
	}

	if( height < img->h )
	{
		/* Duplicate the last row. */
		uint8_t *srcp = img->pixels;
		srcp += img->pitch * (height-1);
		memcpy( srcp + img->pitch, srcp, img->pitch );
	}
}

struct SurfaceHeader
{
	int width, height, pitch;
	int Rmask, Gmask, Bmask, Amask;
	int bpp;
};

/* Save and load RageSurfaces to disk, in a very fast, nonportable way. */
bool RageSurfaceUtils::SaveSurface( const RageSurface *img, CString file )
{
	RageFile f;
	if( !f.Open( file, RageFile::WRITE ) )
		return false;

	SurfaceHeader h;
	memset( &h, 0, sizeof(h) );

	h.height = img->h;
	h.width = img->w;
	h.pitch = img->pitch;
	h.Rmask = img->format->Rmask;
	h.Gmask = img->format->Gmask;
	h.Bmask = img->format->Bmask;
	h.Amask = img->format->Amask;
	h.bpp = img->format->BitsPerPixel;

	f.Write( &h, sizeof(h) );

	if( h.bpp == 8 )
	{
		f.Write( &img->format->palette->ncolors, sizeof(img->format->palette->ncolors) );
		f.Write( img->format->palette->colors, img->format->palette->ncolors * sizeof(RageSurfaceColor) );
	}

	f.Write( img->pixels, img->h * img->pitch );
	
	return true;
}

RageSurface *RageSurfaceUtils::LoadSurface( CString file )
{
	RageFile f;
	if( !f.Open( file ) )
		return false;

	SurfaceHeader h;
	if( f.Read( &h, sizeof(h) ) != sizeof(h) )
		return NULL;
	
	RageSurfacePalette palette;
	if( h.bpp == 8 )
	{
		if( f.Read( &palette.ncolors, sizeof(palette.ncolors) ) != sizeof(palette.ncolors) )
			return NULL;
		ASSERT_M( palette.ncolors <= 256, ssprintf("%i", palette.ncolors) );
		if( f.Read( palette.colors, palette.ncolors * sizeof(RageSurfaceColor) ) != int(palette.ncolors * sizeof(RageSurfaceColor)) )
			return NULL;
	}

	/* Create the surface. */
	RageSurface *img = CreateSurface( h.width, h.height, h.bpp,
			h.Rmask, h.Gmask, h.Bmask, h.Amask );
	ASSERT( img );

	/* If the pitch has changed, this surface is either corrupt, or was
	 * created with a different version whose CreateSurface() behavior
	 * was different. */
	if( h.pitch != img->pitch )
	{
		LOG->Trace( "Error loading \"%s\": expected pitch %i, got %i (%ibpp, %i width)",
				file.c_str(), h.pitch, img->pitch, h.bpp, h.width );
		delete img;
		return NULL;
	}

	if( f.Read( img->pixels, h.height * h.pitch ) != h.height * h.pitch )
	{
		delete img;
		return NULL;
	}

	/* Set the palette. */
	if( h.bpp == 8 )
		*img->fmt.palette = palette;

	return img;
}

/*
 * This converts an image to a special 8-bit paletted format.  The palette is set up
 * so that palette indexes look like regular, packed components.
 *
 * For example, an image with 8 bits of grayscale and 0 bits of alpha has a palette
 * that looks like { 0,0,0,255 }, { 1,1,1,255 }, { 2,2,2,255 }, ... { 255,255,255,255 }.
 * This results in index components that can be treated as grayscale values.
 *
 * An image with 2 bits of grayscale and 2 bits of alpha look like
 * { 0,0,0,0  }, { 85,85,85,0  }, { 170,170,170,0  }, { 255,255,255,0  },
 * { 0,0,0,85 }, { 85,85,85,85 }, { 170,170,170,85 }, { 255,255,255,85 }, ...
 *
 * This results in index components that can be pulled apart like regular packed
 * values: the first two bits of the index are the grayscale component, and the next
 * two bits are the alpha component.
 *
 * This gives us a generic way to handle arbitrary 8-bit texture formats.
 */
RageSurface *RageSurfaceUtils::PalettizeToGrayscale( const RageSurface *src_surf, int GrayBits, int AlphaBits )
{
	AlphaBits = min( AlphaBits, 8-src_surf->format->Loss[3] );
	
	const int TotalBits = GrayBits + AlphaBits;
	ASSERT( TotalBits <= 8 );

	RageSurface *dst_surf = CreateSurface(src_surf->w, src_surf->h,
		8, 0,0,0,0 );

	/* Set up the palette. */
	const int TotalColors = 1 << TotalBits;
	const int Ivalues = 1 << GrayBits;					// number of intensity values
	const int Ishift = 0;								// intensity shift
	const int Imask = ((1 << GrayBits) - 1) << Ishift;	// intensity mask
	const int Iloss = 8-GrayBits;

	const int Avalues = 1 << AlphaBits;					// number of alpha values
	const int Ashift = GrayBits;						// alpha shift
	const int Amask = ((1 << AlphaBits) - 1) << Ashift;	// alpha mask
	const int Aloss = 8-AlphaBits;

	for( int index = 0; index < TotalColors; ++index )
	{
		const int I = (index & Imask) >> Ishift;
		const int A = (index & Amask) >> Ashift;

		int ScaledI;
		if( Ivalues == 1 )
			ScaledI = 255; // if only one intensity value, always fullbright
		else
			ScaledI = clamp( int(roundf(I * (255.0f / (Ivalues-1)))), 0, 255 );

		int ScaledA;
		if( Avalues == 1 )
			ScaledA = 255; // if only one alpha value, always opaque
		else
			ScaledA = clamp( int(roundf(A * (255.0f / (Avalues-1)))), 0, 255 );

		RageSurfaceColor c;
		c.r = uint8_t(ScaledI);
		c.g = uint8_t(ScaledI);
		c.b = uint8_t(ScaledI);
		c.a = uint8_t(ScaledA);

		dst_surf->fmt.palette->colors[index] = c;
	}

	const uint8_t *src = src_surf->pixels;
	uint8_t *dst = dst_surf->pixels;

	int height = src_surf->h;
	int width = src_surf->w;

	/* Bytes to skip at the end of a line. */
	const int srcskip = src_surf->pitch - width*src_surf->format->BytesPerPixel;
	const int dstskip = dst_surf->pitch - width*dst_surf->format->BytesPerPixel;

	while( height-- )
	{
		int x = 0;
		while( x++ < width )
		{
			unsigned int pixel = decodepixel( src, src_surf->format->BytesPerPixel );

			uint8_t colors[4];
			GetRGBAV(pixel, src_surf, colors);

			int Ival = 0;
			Ival += colors[0];
			Ival += colors[1];
			Ival += colors[2];
			Ival /= 3;

			pixel = (Ival >> Iloss) << Ishift |
					(colors[3] >> Aloss) << Ashift;

			/* Store it. */
			*dst = uint8_t(pixel);

			src += src_surf->format->BytesPerPixel;
			dst += dst_surf->format->BytesPerPixel;
		}

		src += srcskip;
		dst += dstskip;
	}

	return dst_surf;
}


RageSurface *RageSurfaceUtils::MakeDummySurface( int height, int width )
{
    RageSurface *ret_image = CreateSurface( width, height, 8, 0,0,0,0 );

	RageSurfaceColor pink( 0xFF, 0x10, 0xFF, 0xFF );
	ret_image->fmt.palette->colors[0] = pink;

	memset( ret_image->pixels, 0, ret_image->h*ret_image->pitch );

	return ret_image;
}

/* HACK:  Some banners and textures have #F800F8 as the color key. Search the edge
 * it; if we find it, use that as the color key. */
static bool ImageUsesOffHotPink( const RageSurface *img )
{
	uint32_t OffHotPink;
	if( !img->format->MapRGBA( 0xF8, 0, 0xF8, 0xFF, OffHotPink ) )
		return false;

	const uint8_t *p = img->pixels;
	for( int x = 0; x < img->w; ++x )
	{
		uint32_t val = RageSurfaceUtils::decodepixel( p, img->format->BytesPerPixel );
		if( val == OffHotPink )
			return true;
		p += img->format->BytesPerPixel;
	}

	p = img->pixels;
	p += img->pitch * (img->h-1);
	for( int i=0; i < img->w; i++ )
	{
		uint32_t val = RageSurfaceUtils::decodepixel( p, img->format->BytesPerPixel );
		if( val == OffHotPink )
			return true;
		p += img->format->BytesPerPixel;
	}
	return false;
}

/* Set #FF00FF and #F800F8 to transparent.  img may be reallocated if it has no alpha
 * bits. */
void RageSurfaceUtils::ApplyHotPinkColorKey( RageSurface *&img )
{
	if( img->format->BitsPerPixel == 8 )
	{
		uint32_t color;
		if( img->format->MapRGBA( 0xF8, 0, 0xF8, 0xFF, color ) )
			img->format->palette->colors[ color ].a = 0;
		if( img->format->MapRGBA( 0xFF, 0, 0xFF, 0xFF, color ) )
			img->format->palette->colors[ color ].a = 0;
		return;
	}

	/* RGBA. Make sure we have alpha. */
	if( img->format->Amask == 0 )
	{
		/* We don't have any alpha.  Try to enable it without copying. */
		/* XXX: need to scan the surface and make sure the new alpha bit is always 1 */
		/*
		const int used_bits = img->format->Rmask | img->format->Gmask | img->format->Bmask;
		
		for( int i = 0; img->format->Amask == 0 && i < img->format->BitsPerPixel; ++i )
		{
			if( (used_bits & (1<<i)) )
			{
				img->format->Amask = 1<<i;
				img->format->Aloss = 7;
				img->format->Ashift = (uint8_t) i;
			}
		}
		*/
		/* If we didn't have any free bits, convert to make room. */
		if( img->format->Amask == 0  )
			ConvertSurface( img, img->w, img->h,
				32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF );
	}

	uint32_t HotPink;

	bool bHaveColorKey;
	if( ImageUsesOffHotPink(img) )
		bHaveColorKey = img->format->MapRGBA( 0xF8, 0, 0xF8, 0xFF, HotPink );
	else
		bHaveColorKey = img->format->MapRGBA( 0xFF, 0, 0xFF, 0xFF, HotPink );
	if( !bHaveColorKey )
		return;

	for( int y = 0; y < img->h; ++y )
	{
		uint8_t *row = img->pixels + img->pitch*y;

		for( int x = 0; x < img->w; ++x )
		{
			uint32_t val = decodepixel( row, img->format->BytesPerPixel );
			if( val == HotPink )
				encodepixel( row, img->format->BytesPerPixel, 0 );

			row += img->format->BytesPerPixel;
		}
	}
}

void RageSurfaceUtils::FlipVertically( RageSurface *img )
{
	const int pitch = img->pitch;
	const int bytes_per_row = img->format->BytesPerPixel * img->w;
	char *row = new char[bytes_per_row];

	for( int y=0; y < img->h/2; y++ )
	{
		int y2 = img->h-1-y;
		memcpy( row, img->pixels + pitch * y, bytes_per_row );
		memcpy( img->pixels + pitch * y, img->pixels + pitch * y2, bytes_per_row );
		memcpy( img->pixels + pitch * y2, row, bytes_per_row  );
	}

	delete [] row;
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
