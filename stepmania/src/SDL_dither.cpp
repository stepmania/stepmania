#include "global.h"

#include "RageUtil.h"
#include "SDL.h"
#include "SDL_dither.h"
#include "SDL_utils.h"

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
static Uint8 DitherPixel(int x, int y, int intensity,  int conv)
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
	 * to 6.  A value of 15/16 biases to 6.1875, which causes it to be rounded
	 * up to 6.  So, a proportion  of pixels gets rounded up based on how close
	 * the number is to the next value. */

	/* Convert the number to the destination range. */
	int out_intensity = intensity * conv;
	
	/* Add bias. */
	out_intensity += DitherMatCalc[y][x];

	/* Truncate, and add e to make sure a value of 14.999998 -> 15. */
	return Uint8((out_intensity + 1) >> 16);
}

void SM_SDL_OrderedDither(const SDL_Surface *src, SDL_Surface *dst)
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

	Uint32 src_cbits[4], dst_cbits[4];
	mySDL_GetBitsPerChannel( src->format, src_cbits );
	mySDL_GetBitsPerChannel( dst->format, dst_cbits );

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
	const Uint8 alpha_max = Uint8((1 << dst_cbits[3]) - 1);

	/* For each row: */
	for( int row = 0; row < src->h; ++row )
	{
		const Uint8 *srcp = (const Uint8 *) src->pixels + row * src->pitch;
		Uint8 *dstp = (Uint8 *) dst->pixels + row * dst->pitch;

		/* For each pixel: */
		for( int col = 0; col < src->w; ++col )
		{
			Uint8 colors[4];
			mySDL_GetRawRGBAV( srcp, src, colors );

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
	
				/* Truncate, and add e to make sure a value of 14.999998 -> 15. */
//				colors[3] = Uint8((out_intensity + 1) >> 16);

				/* I don't remember why this used to truncate.  Doing that causes
				 * dithering to an image with 1-bit alpha to be transparent on all
				 * source alpha values except for full-opaque, which is wrong. 
				 * Add .5, so we round. */
				colors[3] = Uint8((out_intensity + 32767) >> 16);
			}

			/* Raw value -> int -> pixel */
			mySDL_SetRawRGBAV(dstp, dst, colors);

			srcp += src->format->BytesPerPixel;
			dstp += dst->format->BytesPerPixel;
		}
	}
}


/* Return "random" numbers in [0,2]; this is for SM_SDL_ErrorDiffusionDither (rand() is too slow). */
/* static inline int GetFastRand()
{
	static int RandomNumbers[] =
	{
		2, 1, 0, 2, 0, 2, 2, 1, 0, 1, 1, 2, 2, 0, 1, 1, 2, 2, 1, 0, 1, 0, 1, 2, 1, 0, 2, 1, 0, 1, 1, 0,
		2, 1, 1, 2, 1, 0, 0, 1, 1, 2, 0, 2, 0, 1, 0, 1, 2, 0, 0, 1, 1, 2, 0, 2, 1, 2, 2, 2, 2, 1, 1, 1,
		1, 0, 2, 1, 2, 1, 1, 1, 2, 1, 0, 0, 1, 0, 0, 2, 0, 1, 0, 1, 2, 2, 0, 0, 0, 2, 2, 1, 2, 2, 2, 2,
		1, 2, 1, 0, 2, 0, 1, 0, 2, 1, 2, 1, 1, 0, 1, 1, 2, 0, 0, 2, 2, 2, 1, 0, 0, 1, 2, 1, 2, 0, 0, 2,
		2, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 2, 2, 2, 0, 0, 0, 1, 2, 0, 1, 1, 0, 0, 0, 2, 1, 2, 0, 0, 1,
		1, 2, 0, 1, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 0, 0, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 2,
		0, 1, 2, 0, 2, 2, 0, 2, 1, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 0, 1, 0, 1, 2, 1, 2, 1, 0, 0,
		1, 0, 0, 0, 2, 2, 2, 2, 1, 2, 1, 0, 0, 2, 1, 0, 1, 2, 0, 2, 2, 0, 0, 0, 2, 2, 0, 0, 2, 0, 1, 1
	};
	static int iNextNumber = 0;
	if( iNextNumber == ARRAYSIZE(RandomNumbers) )
		iNextNumber = 0;
	return RandomNumbers[ iNextNumber++ ];
} */

/* This is very similar to SM_SDL_OrderedDither, except instead of using a matrix
 * containing rounding values, we truncate and then add the resulting error for
 * each pixel to the next pixel on the same line.  (Maybe we could do both?) */
void SM_SDL_ErrorDiffusionDither(const SDL_Surface *src, SDL_Surface *dst)
{
	/* We can't dither to paletted surfaces. */
	ASSERT( dst->format->BytesPerPixel > 1 );

	Uint32 src_cbits[4], dst_cbits[4];
	mySDL_GetBitsPerChannel( src->format, src_cbits );
	mySDL_GetBitsPerChannel( dst->format, dst_cbits );

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
	const Uint8 alpha_max = Uint8((1 << dst_cbits[3]) - 1);

	/* For each row: */
	for(int row = 0; row < src->h; ++row) 
	{
		Sint32 accumError[4] = { 0, 0, 0, 0 }; // accum error values are reset every row

		const Uint8 *srcp = (const Uint8 *)src->pixels + row * src->pitch;
		Uint8 *dstp = (Uint8 *)dst->pixels + row * dst->pitch;

		/* For each pixel in row: */
		for( int col = 0; col < src->w; ++col )
		{
			Uint8 colors[4];
			mySDL_GetRawRGBAV( srcp, src, colors );

			for( int c = 0; c < 3; ++c )
			{
				/* Convert the number to the destination range. */
				int out_intensity = colors[c] * conv[c];

				/* Add e to make sure a value of 14.999998 -> 15. */
				++out_intensity;
	
				/* Add bias. */
				out_intensity += accumError[c];

				/* out_intensity is now what we actually want this component to be.
				 * To store it, we have to clamp it (prevent overflow) and shift it
				 * from fixed-point to [0,255].  The error introduced in that calculation
				 * becomes the new accumError. */
				int clamped_intensity = clamp( out_intensity, 0, 0xFFFFFF );
				clamped_intensity &= 0xFF0000;

				/* Truncate. */
				colors[c] = Uint8(clamped_intensity >> 16);

				accumError[c] = out_intensity - clamped_intensity;

				// Reduce funky streaks in low-bit channels by clamping error.
				CLAMP( accumError[c], -128 * 65536, +128 * 65536 );

				// Keep only a fraction of the error to make the effect more subtle.
				// This used to divide by [1,4]; shift right by [0,2] to get a similar
				// (but much faster) effect.
				/* This resulted in banding in gradients, and doesn't work quite the
				 * same way with this calculation; without it, gradients look correct.
				 * Unfortunately, I don't remember any of the problem cases we had
				 * originally; if we see problems with the dithering in the future, let's
				 * archive them somewhere for future testing. */
//				accumError[c] >>= (GetFastRand())+1;
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
	
				/* Truncate, and add e to make sure a value of 14.999998 -> 15. */
//				colors[3] = Uint8((out_intensity + 1) >> 16);

				/* I don't remember why this used to truncate.  Doing that causes
				 * dithering to an image with 1-bit alpha to be transparent on all
				 * source alpha values except for full-opaque, which is wrong. 
				 * Add .5, so we round. */
				colors[3] = Uint8((out_intensity + 32767) >> 16);
			}

			mySDL_SetRawRGBAV( dstp, dst, colors );

			srcp += src->format->BytesPerPixel;
			dstp += dst->format->BytesPerPixel;
		}
	}
}

