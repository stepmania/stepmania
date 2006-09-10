#include "global.h"
#include "RageSurfaceUtils_Zoom.h"
#include "RageSurface.h"
#include "RageSurfaceUtils.h"
#include "RageUtil.h"

#include <vector>
using namespace std;

/*
 * Coordinate 0x0 represents the exact top-left corner of a bitmap.  .5x.5
 * represents the center of the top-left pixel; 1x1 is the center of the top
 * square of pixels.  
 *
 * (Look at a grid: map coordinates to the lines, not the squares between the
 * lines.)
 */

static void InitVectors( vector<int> &s0, vector<int> &s1, vector<float> &percent, int src, int dst )
{
	if( src >= dst )
	{
		float sx = float(src) / dst;
		for( int x = 0; x < dst; x++ )
		{
			/* sax is the exact (floating-point) x coordinate in the source
			 * that the destination pixel at x should from.  For example, if we're
			 * going 512->256, then dst[0] should come from the pixels from 0..1 and
			 * 1..2, so sax[0] is 1. sx is the total number of pixels, so sx/2 is the
			 * distance from the start of the sample to its center. */
			const float sax = sx*x + sx/2;

			/* sx/2 is the distance from the start of the sample to the center;
			 * sx/4 is the distance from the center of the sample to the center of
			 * either pixel. */
			const float xstep = sx/4;

			/* source x coordinates of left and right pixels to sample */
			s0.push_back(int(sax-xstep));
			s1.push_back(int(sax+xstep));

			if( s0[x] == s1[x] )
			{
				/* If the sampled pixels happen to be the same, the distance
				 * will be 0.  Avoid division by zero. */
				percent.push_back( 1.f );
			} else {
				const int xdist = s1[x] - s0[x];

				/* fleft is the left pixel sampled; +.5 is the center: */
				const float fleft = s0[x] + .5f;

				/* sax is somewhere between the centers of both sampled
				 * pixels; find the percentage: */
				const float p = (sax - fleft) / xdist;
				percent.push_back(1-p);
			}
		}
	}
	else
	{
		/*
		 * Fencepost: If we have source:
		 *    abcd
		 * and dest:
		 *    xyz
		 * then we want x to be sampled entirely from a, and z entirely from d;
		 * the inner pixels are interpolated.  (This behavior mimics Photoshop's
		 * resize.)
		 */
		float sx = float(src-1) / (dst-1);
		for( int x = 0; x < dst; x++ )
		{
			const float sax = sx*x;

			/* source x coordinates of left and right pixels to sample */
			s0.push_back( clamp(int(sax), 0, src-1));
			s1.push_back( clamp(int(sax+1), 0, src-1) );

			const float p = 1 - (sax - floorf(sax));
			percent.push_back( p );
		}
	}
}

static void ZoomSurface( const RageSurface * src, RageSurface * dst )
{
	/* For each destination coordinate, two source rows, two source columns
	 * and the percentage of the first row and first column: */
	vector<int> esx0, esx1, esy0, esy1;
	vector<float> ex0, ey0;

	InitVectors( esx0, esx1, ex0, src->w, dst->w );
	InitVectors( esy0, esy1, ey0, src->h, dst->h );

	/* This is where all of the real work is done. */
	const uint8_t *sp = (uint8_t *) src->pixels;
	for( int y = 0; y < dst->h; y++ )
	{
		uint32_t *dp = (uint32_t *) (dst->pixels + dst->pitch*y);
		/* current source pointer and next source pointer (first and second 
		 * rows sampled for this row): */
		const uint8_t *csp = sp + esy0[y] * src->pitch;
		const uint8_t *ncsp = sp + esy1[y] * src->pitch;

		for( int x = 0; x < dst->w; x++ )
		{
			/* Grab pointers to the sampled pixels: */
			const uint8_t *c00 = csp + esx0[x]*4;
			const uint8_t *c01 = csp + esx1[x]*4;
			const uint8_t *c10 = ncsp + esx0[x]*4;
			const uint8_t *c11 = ncsp + esx1[x]*4;

			uint8_t color[4];
			for( int c = 0; c < 4; ++c )
			{
				float x0, x1;
				x0 = c00[c] * ex0[x];
				x0 += c01[c] * (1-ex0[x]);
				x1 = c10[c] * ex0[x];
				x1 += c11[c] * (1-ex0[x]);

				const float res = (x0 * ey0[y]) + (x1 * (1-ey0[y])) + 0.5f;
				color[c] = uint8_t(res);
			}
			*dp = *(uint32_t *) color;

			/* Advance destination pointer. */
			++dp;
		}
	}
}


void RageSurfaceUtils::Zoom( RageSurface *&src, int dstwidth, int dstheight )
{
	ASSERT_M( dstwidth > 0, ssprintf("%i",dstwidth) );
	ASSERT_M( dstheight > 0, ssprintf("%i",dstheight) );
	if( src == NULL )
		return;

	if( src->w == dstwidth && src->h == dstheight )
		return;

	/* resize currently only does RGBA8888 */
	if( src->fmt.BytesPerPixel != 4 )
	{
		RageSurfaceUtils::ConvertSurface( src, src->w, src->h, 32,
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	}

	while( src->w != dstwidth || src->h != dstheight )
	{
		float xscale = float(dstwidth)/src->w;
		float yscale = float(dstheight)/src->h;

		/* Our filter is a simple linear filter, so it can't scale to less than
		 * 1:2 or more than 2:1 very well.  If we need to go beyond that, do it
		 * iteratively. */
		xscale = clamp( xscale, .5f, 2.0f );
		yscale = clamp( yscale, .5f, 2.0f );

		int target_width = lrintf( src->w*xscale );
		int target_height = lrintf( src->h*yscale );

		RageSurface *dst =
			CreateSurface(target_width, target_height, 32,
					src->format->Rmask, src->format->Gmask,
					src->format->Bmask, src->format->Amask);

		ZoomSurface( src, dst );

		delete src;

		src = dst;
	}
}

/*  
 * (c) A. Schiffler, Glenn Maynard
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
 * 
 * This is based on code from SDL_rotozoom, under the above license with
 * permission from Andreas Schiffler.
 */

