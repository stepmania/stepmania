/*  

  SDL_rotozoom.c - rotozoomer for 32bit or 8bit surfaces

  LGPL (c) A. Schiffler

  This file is indented with a tab size of 8, unlike the rest of the SM
  source which has a tab size of 4.  I'm leaving it that way so I can
  send patches upstream.  (Since 8 is the de-facto standard tab size,
  it'd be nice to fix the rest of the source to use it, but I don't want
  to do a full-source commit ...) -glenn
*/

#include "SDL_rotozoom.h"

#include <math.h>

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

float frac(float x) { return x-int(x); }
int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst)
{
    /* Ratio from source to dest. */
    float sx = float(src->w) / dst->w;
    float sy = float(src->h) / dst->h;

    float *sax = new float[dst->w];
    float *say = new float[dst->h];

    /* Precalculate row increments */
    int x, y;
    /* sax[x] is the exact (floating-point) x coordinate in the source
     * that the destination pixel at x should from.  For example, if we're
     * going 512->256, then dst[0] should come from the pixels from 0..1 and
     * 1..2, so sax[0] is 1. sx is the total number of pixels, so sx/2 is the
     * distance from the start of the sample to its center. */
    for (x = 0; x < dst->w; x++)
	sax[x] = sx*x + sx/2;

    for (y = 0; y < dst->h; y++)
	say[y] = sy*y + sy/2;

    tColorRGBA *sp = (tColorRGBA *) src->pixels;

    /* Scan destination */
    for (y = 0; y < dst->h; y++) {
	/* Set destination pointer. */

	/* sx/2 is the distance from the start of the sample to the center;
	 * sx/4 is the distance from the center of the sample to the center of
	 * either pixel. */
        float xstep = sx/4;
        float ystep = sy/4;

	int ytop = int(say[y]-ystep);
	int ybottom = int(say[y]+ystep);

	tColorRGBA *dp = (tColorRGBA *) ((Uint8 *) dst->pixels + dst->pitch*y);
	tColorRGBA *csp = (tColorRGBA *) ((Uint8 *) sp + ytop * src->pitch);
	tColorRGBA *ncsp = (tColorRGBA *) ((Uint8 *) sp + ybottom * src->pitch);

	for (x = 0; x < dst->w; x++) {
	    /* x coordinates of left and right pixels to sample */
	    int xleft = int(sax[x]-xstep);
	    int xright = int(sax[x]+xstep);

	    /* Grab pointers to the sampled pixels: */
	    tColorRGBA *c00 = csp + xleft;
	    tColorRGBA *c01 = csp + xright;
	    tColorRGBA *c10 = ncsp + xleft;
	    tColorRGBA *c11 = ncsp + xright;

	    /* Distance between the pixels that were sampled: */
	    int xdist = xright - xleft;
	    int ydist = ybottom - ytop;

	    float ex0, ex1, ey0, ey1;
	    if(xdist == 0) {
		/* If the sampled pixels happen to be the same, the distance
		 * will be 0.  Avoid division by zero. */
		ex0 = 1.f; ex1 = 0.f;
	    } else {
		/* fleft is the left pixel sampled; +.5 is the center: */
    		float fleft = xleft + .5f;
		/* sax[x] is somewhere between the centers of both sampled
		 * pixels; find the percentage: */
		float p = (sax[x] - fleft) / xdist;
		ex0 = 1-p;
		ex1 = 1-ex0;
	    }

	    if(ydist == 0) {
		ey0 = 1.f; ey1 = 0.f;
	    } else {
    		float ftop = ytop + .5f;
		float p = (say[y] - ftop) / ydist;
		ey0 = 1-p;
		ey1 = 1-ey0;
	    }

	    for(int c = 0; c < 4; ++c) {
		float xxx0, xxx1;
		xxx0 = c01->c[c] * ex1;
		xxx0 += c00->c[c] * ex0;
		xxx1 = c11->c[c] * ex1;
		xxx1 += c10->c[c] * ex0;

		float res = (xxx0 * ey0) + (xxx1 * ey1);
		dp->c[c] = Uint8(res);
	    }

	    /* Advance destination pointer. */
	    dp++;
	}
    }

    /* Remove temp arrays */
    delete [] sax;
    delete [] say;

    return (0);
}

#define VALUE_LIMIT	0.001


/* Local rotozoom-size function with trig result return */


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
