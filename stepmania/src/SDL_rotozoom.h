/*

 SDL_rotozoom - rotozoomer

 LGPL (c) A. Schiffler

*/

#ifndef SDL_ROTOZOOM_H
#define SDL_ROTOZOOM_H 1

#include "SDL.h"

/* ---- Defines */

#define SMOOTHING_OFF		0
#define SMOOTHING_ON		1

/* 
 
 zoomSurface()

 Zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
 'zoomx' and 'zoomy' are scaling factors for width and height. If 'smooth' is 1
 then the destination 32bit surface is anti-aliased. If the surface is not 8bit
 or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.

*/

SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth);

/* Returns the size of the target surface for a zoomSurface() call */
void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight);

#endif
