#include "global.h"
/*  
 * SDL_rotozoom.c - rotozoomer for 32bit or 8bit surfaces
 *
 * LGPL (c) A. Schiffler, Glenn Maynard
 */

#include "SDL_rotozoom.h"

#include <math.h>
#include <vector>
using namespace std;

struct tColorRGBA {
    Uint8 c[4];
};

/* 
 
 32bit Zoomer with optional anti-aliasing by bilinear interpolation.

 Zooms 32bit RGBA/ABGR 'src' surface to 'dst' surface.
 
*/
/* Coordinate 0x0 represents the exact top-left corner of a bitmap.  .5x.5
 * represents the center of the top-left pixel; 1x1 is the center of the top
 * square of pixels.  
 *
 * (Look at a grid: map coordinates to the lines, not the squares between the
 * lines.)
 */

static void ZoomSurface( SDL_Surface * src, SDL_Surface * dst )
{
    /* Ratio from source to dest. */
    float sx = float(src->w) / dst->w;
    float sy = float(src->h) / dst->h;

    /* For each destination coordinate, two source rows, two source columns
     * and the percentage of the first row and first column: */
    vector<int> esx0, esx1, esy0, esy1;
    vector<float> ex0, ey0;

    int x, y;

    /* sax[x] is the exact (floating-point) x coordinate in the source
     * that the destination pixel at x should from.  For example, if we're
     * going 512->256, then dst[0] should come from the pixels from 0..1 and
     * 1..2, so sax[0] is 1. sx is the total number of pixels, so sx/2 is the
     * distance from the start of the sample to its center. */
    for( x = 0; x < dst->w; x++ )
	{
		const float sax = sx*x + sx/2;

		/* sx/2 is the distance from the start of the sample to the center;
		 * sx/4 is the distance from the center of the sample to the center of
		 * either pixel. */
		const float xstep = sx/4;

		/* source x coordinates of left and right pixels to sample */
		esx0.push_back(int(sax-xstep));
		esx1.push_back(int(sax+xstep));

		if( esx1[x] == esx0[x] )
		{
			/* If the sampled pixels happen to be the same, the distance
			 * will be 0.  Avoid division by zero. */
			ex0.push_back( 1.f );
		} else {
			const int xdist = esx1[x] - esx0[x];

			/* fleft is the left pixel sampled; +.5 is the center: */
    		const float fleft = esx0[x] + .5f;
		    
			/* sax is somewhere between the centers of both sampled
			 * pixels; find the percentage: */
			const float p = (sax - fleft) / xdist;
			ex0.push_back(1-p);
		}
    }

    for( y = 0; y < dst->h; y++ )
	{
		float say = sy*y + sy/2;

		float ystep = sy/4;

		esy0.push_back( int(say-ystep) );
		esy1.push_back( int(say+ystep) );

		if( esy0[y] == esy1[y] )
		{
    		ey0.push_back( 1.f );
		} else {
			const int ydist = esy1[y] - esy0[y];
      		const float ftop = esy0[y] + .5f;
			const float p = (say - ftop) / ydist;
			ey0.push_back( 1-p );
		}
    }

    const tColorRGBA *sp = (tColorRGBA *) src->pixels;

    /* Scan destination */
    for( y = 0; y < dst->h; y++ )
	{
		tColorRGBA *dp = (tColorRGBA *) ((Uint8 *) dst->pixels + dst->pitch*y);
		/* current source pointer and next source pointer (first and second 
		 * rows sampled for this row): */
		const tColorRGBA *csp = (tColorRGBA *) ((Uint8 *) sp + esy0[y] * src->pitch);
		const tColorRGBA *ncsp = (tColorRGBA *) ((Uint8 *) sp + esy1[y] * src->pitch);

		for( x = 0; x < dst->w; x++ )
		{
			/* Grab pointers to the sampled pixels: */
			const tColorRGBA *c00 = csp + esx0[x];
			const tColorRGBA *c01 = csp + esx1[x];
			const tColorRGBA *c10 = ncsp + esx0[x];
			const tColorRGBA *c11 = ncsp + esx1[x];

			for( int c = 0; c < 4; ++c )
			{
				float x0, x1;
				x0 = c00->c[c] * ex0[x];
				x0 += c01->c[c] * (1-ex0[x]);
				x1 = c10->c[c] * ex0[x];
				x1 += c11->c[c] * (1-ex0[x]);

				const float res = (x0 * ey0[y]) + (x1 * (1-ey0[y]));
				dp->c[c] = Uint8(res);
			}

			/* Advance destination pointer. */
			dp++;
		}
    }
}


static SDL_Surface *zoomSurface_ll( SDL_Surface *src, int dstwidth, int dstheight )
{
    if( src == NULL )
		return NULL;

    SDL_Surface *dst =
		SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
			    src->format->Rmask, src->format->Gmask,
			    src->format->Bmask, src->format->Amask);

    ZoomSurface( src, dst );

    return dst;
}


void zoomSurface( SDL_Surface *&src, int dstwidth, int dstheight )
{
    if( src == NULL )
		return;

	while( src->w != dstwidth || src->h != dstheight )
	{
		float xscale = float(dstwidth)/src->w;
		float yscale = float(dstheight)/src->h;

		/* Our filter is a simple linear filter, so it can't scale to 
		 * less than .5 very well.  If we need to go lower than .5, do
		 * it iteratively. */
		xscale = max( xscale, .5f );
		yscale = max( yscale, .5f );

		int target_width = int(src->w*xscale + .5);
		int target_height = int(src->h*yscale + .5);

		SDL_Surface	*dst = zoomSurface_ll(src, target_width, target_height);

		SDL_FreeSurface(src);

		src = dst;
	}
}
