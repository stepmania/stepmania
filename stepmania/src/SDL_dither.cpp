#include "global.h"

#include "RageUtil.h"
#include "SDL_dither.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"

#define DitherMatDim 4


/*
 Added error-diffusion algorithm. (SM_SDL_ErrorDiffusionDither)
 Error distributed per-row, left to right.
 http://www.gamasutra.com/features/19990521/pixel_conversion_03.htm
 */

/* Fractions, 0/16 to 15/16:  */
static const int DitherMat[DitherMatDim][DitherMatDim] =
{
	{  0,  8,  2, 10 },
	{ 12,  4, 14,  6 },
	{  3, 11,  1,  9 },
	{ 15,  7, 13,  5 }
};

static int DitherMatCalc[DitherMatDim][DitherMatDim];

/* conv is the ratio from the input to the output. */
static uint8_t DitherPixel(int x, int y, int intensity,  int conv)
{
	/* The intensity matrix wraps.  This assumes the matrix dims are a power of 2. */
	x &= DitherMatDim-1;
	y &= DitherMatDim-1;

	/* Ordered dithering is scaling the old intensity range to the new, with
	 * the matrix values biasing to rounding down or up. Matrix values are in
	 * the range [0..1).  For example, converting the 8-bit value 100 to 4
	 * bits directly gives 6.25.  A matrix value of 0 means the pixel is not
	 * biased at all, which would cause it to be truncated to 6.  A value
	 * of 5/16 means that the value is biased to 6.5625, which is also truncated
	 * to 6.  A value of 15/16 biases to 7.1875, which causes it to be rounded
	 * up to 7.  So, a proportion  of pixels gets rounded up based on how close
	 * the number is to the next value. */

	/* Convert the number to the destination range. */
	int out_intensity = intensity * conv;
	
	/* Add bias. */
	out_intensity += DitherMatCalc[y][x];

	/* Truncate, and add e to make sure a value of 14.999998 -> 15. */
	return uint8_t((out_intensity + 1) >> 16);
}

void RageSurfaceUtils::OrderedDither( const RageSurface *src, RageSurface *dst )
{
	static bool DitherMatCalc_initted = false;
	if( !DitherMatCalc_initted )
	{
		for( int i = 0; i < DitherMatDim; ++i )
		{
			for( int j = 0; j < DitherMatDim; ++j )
			{
				/* Each value is 0..15.  They represent 0/16 through 15/16.
				 * Set DitherMatCalc to that value * 65536, so we can do it
				 * with integer calcs. */
				DitherMatCalc[i][j] = DitherMat[i][j] * 65536 / 16;
			}
		}
			
		DitherMatCalc_initted = true;
	}

	/* We can't dither to paletted surfaces. */
	ASSERT( dst->format->BytesPerPixel > 1 );

	uint32_t src_cbits[4], dst_cbits[4];
	RageSurfaceUtils::GetBitsPerChannel( src->format, src_cbits );
	RageSurfaceUtils::GetBitsPerChannel( dst->format, dst_cbits );

	/* Calculate the ratio from the old bit depth to the new for each color channel. */
	int conv[4];
	for( int i = 0; i < 4; ++i )
	{
		int MaxInputIntensity = (1 << src_cbits[i])-1;
		int MaxOutputIntensity = (1 << dst_cbits[i])-1;
		/* If the source is missing the channel, avoid div/0. */
		if( MaxInputIntensity == 0 )
			conv[i] = 0;
		else
			conv[i] = MaxOutputIntensity * 65536 / MaxInputIntensity;
	}

	/* Max alpha value; used when there's no alpha source.  */
	const uint8_t alpha_max = uint8_t((1 << dst_cbits[3]) - 1);

	/* For each row: */
	for( int row = 0; row < src->h; ++row )
	{
		const uint8_t *srcp = src->pixels + row * src->pitch;
		uint8_t *dstp = dst->pixels + row * dst->pitch;

		/* For each pixel: */
		for( int col = 0; col < src->w; ++col )
		{
			uint8_t colors[4];
			RageSurfaceUtils::GetRawRGBAV( srcp, src->fmt, colors );

			/* Note that we don't dither the alpha channel. */
			for( int c = 0; c < 3; ++c )
			{
				/* If the destination has less bits, dither: */
				colors[c] = DitherPixel( col, row, colors[c], conv[c] );
			}

			/* If the source has no alpha, the conversion formula will end up
			 * with 0; that's fine for color channels, but for alpha we need to
			 * be opaque. */
			if( src_cbits[3] == 0 )
			{
				colors[3] = alpha_max;
			} else {
				/* Same as DitherPixel, except it doesn't actually dither; dithering
				 * looks bad on the alpha channel. */
				int out_intensity = colors[3] * conv[3];
	
				/* Round: */
				colors[3] = uint8_t((out_intensity + 32767) >> 16);
			}

			/* Raw value -> int -> pixel */
			RageSurfaceUtils::SetRawRGBAV(dstp, dst, colors);

			srcp += src->format->BytesPerPixel;
			dstp += dst->format->BytesPerPixel;
		}
	}
}


static uint8_t EDDitherPixel( int x, int y, int intensity, int conv, int32_t &accumError )
{
	/* Convert the number to the destination range. */
	int out_intensity = intensity * conv;

	/* Add e to make sure a value of 14.999998 -> 15. */
	++out_intensity;
	
	/* Add bias. */
	out_intensity += accumError;

	/* out_intensity is now what we actually want this component to be.
	 * To store it, we have to clamp it (prevent overflow) and shift it
	 * from fixed-point to [0,255].  The error introduced in that calculation
	 * becomes the new accumError. */
	int clamped_intensity = clamp( out_intensity, 0, 0xFFFFFF );
	clamped_intensity &= 0xFF0000;

	/* Truncate. */
	uint8_t ret = uint8_t(clamped_intensity >> 16);

	accumError = out_intensity - clamped_intensity;

	// Reduce funky streaks in low-bit channels by clamping error.
	CLAMP( accumError, -128 * 65536, +128 * 65536 );

	return ret;
}

/* This is very similar to SM_SDL_OrderedDither, except instead of using a matrix
 * containing rounding values, we truncate and then add the resulting error for
 * each pixel to the next pixel on the same line.  (Maybe we could do both?) */
void RageSurfaceUtils::ErrorDiffusionDither( const RageSurface *src, RageSurface *dst )
{
	/* We can't dither to paletted surfaces. */
	ASSERT( dst->format->BytesPerPixel > 1 );

	uint32_t src_cbits[4], dst_cbits[4];
	RageSurfaceUtils::GetBitsPerChannel( src->format, src_cbits );
	RageSurfaceUtils::GetBitsPerChannel( dst->format, dst_cbits );

	/* Calculate the ratio from the old bit depth to the new for each color channel. */
	int conv[4];
	for( int i = 0; i < 4; ++i )
	{
		int MaxInputIntensity = (1 << src_cbits[i])-1;
		int MaxOutputIntensity = (1 << dst_cbits[i])-1;
		/* If the source is missing the channel, avoid div/0. */
		if( MaxInputIntensity == 0 )
			conv[i] = 0;
		else
			conv[i] = MaxOutputIntensity * 65536 / MaxInputIntensity;
	}

	/* Max alpha value; used when there's no alpha source.  */
	const uint8_t alpha_max = uint8_t((1 << dst_cbits[3]) - 1);

	/* For each row: */
	for(int row = 0; row < src->h; ++row) 
	{
		int32_t accumError[4] = { 0, 0, 0, 0 }; // accum error values are reset every row

		const uint8_t *srcp = src->pixels + row * src->pitch;
		uint8_t *dstp = dst->pixels + row * dst->pitch;

		/* For each pixel in row: */
		for( int col = 0; col < src->w; ++col )
		{
			uint8_t colors[4];
			RageSurfaceUtils::GetRawRGBAV( srcp, src->fmt, colors );

			for( int c = 0; c < 3; ++c )
			{
				colors[c] = EDDitherPixel( col, row, colors[c], conv[c], accumError[c] );
			}

			/* If the source has no alpha, the conversion formula will end up
			 * with 0; that's fine for color channels, but for alpha we need to
			 * be opaque. */
			if( src_cbits[3] == 0 )
			{
				colors[3] = alpha_max;
			} else {
				/* Same as DitherPixel, except it doesn't actually dither; dithering
				 * looks bad on the alpha channel. */
				int out_intensity = colors[3] * conv[3];
	
				/* Round: */
				colors[3] = uint8_t((out_intensity + 32767) >> 16);
			}

			RageSurfaceUtils::SetRawRGBAV( dstp, dst, colors );

			srcp += src->format->BytesPerPixel;
			dstp += dst->format->BytesPerPixel;
		}
	}
}

/*
 * (c) 2002-2004 Glenn Maynard, Chris Danford
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
