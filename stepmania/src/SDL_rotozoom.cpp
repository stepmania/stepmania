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

 Zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.
 
*/
/* Coordinate 0x0 represents the exact top-left corner of a bitmap.  .5x.5
 * represents the center of the top-left pixel; 1x1 is the center of the top
 * square of pixels.  
 *
 * (Look at a grid: map coordinates to the lines, not the squares between the
 * lines.)
 */

void zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst)
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
    for (x = 0; x < dst->w; x++) {
		float sax = sx*x + sx/2;

		/* sx/2 is the distance from the start of the sample to the center;
		* sx/4 is the distance from the center of the sample to the center of
		* either pixel. */
		float xstep = sx/4;

		/* source x coordinates of left and right pixels to sample */
		esx0.push_back(int(sax-xstep));
		esx1.push_back(int(sax+xstep));

		if(esx1[x] == esx0[x]) {
			/* If the sampled pixels happen to be the same, the distance
			* will be 0.  Avoid division by zero. */
			ex0.push_back(1.f);
		} else {
			int xdist = esx1[x] - esx0[x];

			/* fleft is the left pixel sampled; +.5 is the center: */
    		float fleft = esx0[x] + .5f;
		    
			/* sax is somewhere between the centers of both sampled
			* pixels; find the percentage: */
			float p = (sax - fleft) / xdist;
			ex0.push_back(1-p);
		}
    }

    for (y = 0; y < dst->h; y++) {
		float say = sy*y + sy/2;

		float ystep = sy/4;

		esy0.push_back(int(say-ystep));
		esy1.push_back(int(say+ystep));

		if(esy0[y] == esy1[y]) {
    		ey0.push_back(1.f);
		} else {
			int ydist = esy1[y] - esy0[y];
      		float ftop = esy0[y] + .5f;
			float p = (say - ftop) / ydist;
			ey0.push_back(1-p);
		}
    }

    tColorRGBA *sp = (tColorRGBA *) src->pixels;

    /* Scan destination */
    for (y = 0; y < dst->h; y++) {
		tColorRGBA *dp = (tColorRGBA *) ((Uint8 *) dst->pixels + dst->pitch*y);
		/* current source pointer and next source pointer (first and second 
		* rows sampled for this row): */
		tColorRGBA *csp = (tColorRGBA *) ((Uint8 *) sp + esy0[y] * src->pitch);
		tColorRGBA *ncsp = (tColorRGBA *) ((Uint8 *) sp + esy1[y] * src->pitch);

		for (x = 0; x < dst->w; x++) {
			/* Grab pointers to the sampled pixels: */
			tColorRGBA *c00 = csp + esx0[x];
			tColorRGBA *c01 = csp + esx1[x];
			tColorRGBA *c10 = ncsp + esx0[x];
			tColorRGBA *c11 = ncsp + esx1[x];

			for(int c = 0; c < 4; ++c) {
				float x0, x1;
				x0 = c00->c[c] * ex0[x];
				x0 += c01->c[c] * (1-ex0[x]);
				x1 = c10->c[c] * ex0[x];
				x1 += c11->c[c] * (1-ex0[x]);

				float res = (x0 * ey0[y]) + (x1 * (1-ey0[y]));
				dp->c[c] = Uint8(res);
			}

			/* Advance destination pointer. */
			dp++;
		}
    }
}

#define VALUE_LIMIT	0.001

/* 
 
 zoomSurface()

 Zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
 'zoomx' and 'zoomy' are scaling factors for width and height. If 'smooth' is 1
 then the destination 32bit surface is anti-aliased. If the surface is not 8bit
 or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

*/

void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight)
{
    /*
     * Sanity check zoom factors 
     */
    if (zoomx < VALUE_LIMIT) zoomx = VALUE_LIMIT;
    if (zoomy < VALUE_LIMIT) zoomy = VALUE_LIMIT;

    /* Calculate target size */
    *dstwidth = (int) ((double) width * zoomx);
    *dstheight = (int) ((double) height * zoomy);
    if (*dstwidth < 1) *dstwidth = 1;
    if (*dstheight < 1) *dstheight = 1;
}

SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy)
{
    SDL_Surface *rz_dst;
    int dstwidth, dstheight;

    /*
     * Sanity check 
     */
    if (src == NULL)
	return (NULL);

    /* Get size if target */
    zoomSurfaceSize(src->w, src->h, zoomx, zoomy, &dstwidth, &dstheight);

    /* Alloc space to completely contain the zoomed surface */
    /* Target surface is 32bit with source RGBA/ABGR ordering */
    rz_dst =
    SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32,
			    src->format->Rmask, src->format->Gmask,
			    src->format->Bmask, src->format->Amask);

    /*
     * Lock source surface 
     */
    SDL_LockSurface(src);

    /* Call the 32bit transformation routine to do the zooming (using alpha) */
    zoomSurfaceRGBA(src, rz_dst);
    /* Turn on source-alpha support */
    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
    /* Unlock source surface */
    SDL_UnlockSurface(src);

    /* Return destination surface  */
    return (rz_dst);
}
