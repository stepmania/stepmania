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

#include <stdlib.h>
#include <math.h>

struct tColorRGBA {
    Uint8 r,g,b,a;
};

#define MAX(a,b)    (((a) > (b)) ? (a) : (b))

/* 
 
 32bit Zoomer with optional anti-aliasing by bilinear interpolation.

 Zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.
 
*/

int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int smooth)
{
    int x, y, sx, sy, *sax, *say;

    /* Variable setup */
    // if (smooth) {
	/* For interpolation: assume source dimension is one pixel */
	/* smaller to avoid overflow on right and bottom edge. */
	/* XXX: no, this misaligns stuff; we need to clamp properly -glenn */
	// sx = (int) (65536.0 * (float) (src->w - 1) / (float) dst->w);
	// sy = (int) (65536.0 * (float) (src->h - 1) / (float) dst->h);
    // } else {
	sx = 65536 * src->w / dst->w;
	sy = 65536 * src->h / dst->h;
    //}

    /* Allocate memory for row increments */
    if ((sax = (int *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL)
	return (-1);

    if ((say = (int *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
	free(sax);
	return (-1);
    }

    /* Precalculate row increments */
    int csx = 0;
    int *csax = sax;
    for (x = 0; x <= dst->w; x++) {
	*csax = csx;
	csx &= 0xffff;
	csax++;
	csx += sx;
    }
    int csy = 0;
    int *csay = say;
    for (y = 0; y <= dst->h; y++) {
	*csay = csy;
	csy &= 0xffff;
	csay++;
	csy += sy;
    }

    /* Pointer setup */
    tColorRGBA *sp, *csp;
    sp = csp = (tColorRGBA *) src->pixels;

    /* Switch between interpolating and non-interpolating code */
    csay = say;
    if (smooth) {
	/* Interpolating Zoom */

	/* Scan destination */
	for (y = 0; y < dst->h; y++) {
	    /* Set up color source pointers */
	    tColorRGBA *c00 = csp;
	    tColorRGBA *c01 = csp+1;
	    tColorRGBA *c10 = (tColorRGBA *) ((Uint8 *) csp + src->pitch);
	    tColorRGBA *c11 = c10+1;
	    csax = sax;
	    /* Set destination pointer. */
	    tColorRGBA *dp = (tColorRGBA *) ((Uint8 *) dst->pixels + dst->pitch*y);

	    for (x = 0; x < dst->w; x++) {

		/*
		 * Interpolate colors 
		 */
		int ex = *csax & 0xffff;
		int ey = *csay & 0xffff;
		int t1, t2;
		t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
		t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
		dp->r = (Uint8)((((t2 - t1) * ey) >> 16) + t1);
		t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
		t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
		dp->g = (Uint8)((((t2 - t1) * ey) >> 16) + t1);
		t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
		t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
		dp->b = (Uint8)((((t2 - t1) * ey) >> 16) + t1);
		t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
		t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
		dp->a = (Uint8)((((t2 - t1) * ey) >> 16) + t1);

		/* Advance source pointers. */
		csax++;
		int sstep = (*csax >> 16);
		c00 += sstep;
		c01 += sstep;
		c10 += sstep;
		c11 += sstep;
		/* Advance destination pointer. */
		dp++;
	    }
	    /* Advance source pointer. */
	    csay++;
	    csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
	}

    } else {
	/* Non-Interpolating Zoom */
	for (y = 0; y < dst->h; y++) {
	    sp = csp;
	    csax = sax;
	    tColorRGBA *dp = (tColorRGBA *) ((Uint8 *) dst->pixels + dst->pitch*y);
	    for (x = 0; x < dst->w; x++) {
		/* Draw */
		*dp = *sp;
		/* Advance source pointers */
		csax++;
		sp += (*csax >> 16);
		/* Advance destination pointer */
		dp++;
	    }
	    /* Advance source pointer */
	    csay++;
	    csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
	}

    }

    /*
     * Remove temp arrays 
     */
    free(sax);
    free(say);

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

SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth)
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
    zoomSurfaceRGBA(src, rz_dst, smooth);
    /* Turn on source-alpha support */
    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255);
    /* Unlock source surface */
    SDL_UnlockSurface(src);

    /* Return destination surface  */
    return (rz_dst);
}
