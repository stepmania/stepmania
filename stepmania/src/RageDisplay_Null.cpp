#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: RageDisplay

 Desc: See header.

 Copyright (c) 2004 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "SDL_utils.h"
#include "RageFile.h"

#include "RageDisplay.h"
#include "RageDisplay_Null.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageTimer.h"
#include "RageTexture.h"
#include "RageTextureManager.h"
#include "RageMath.h"
#include "RageTypes.h"
#include "StepMania.h"
#include "RageUtil.h"

#include <math.h>

static RageDisplay::PixelFormatDesc PIXEL_FORMAT_DESC[RageDisplay::NUM_PIX_FORMATS] = {
	{
		/* R8G8B8A8 */
		32,
		{ 0xFF000000,
		  0x00FF0000,
		  0x0000FF00,
		  0x000000FF }
	}, {
		/* R4G4B4A4 */
		16,
		{ 0xF000,
		  0x0F00,
		  0x00F0,
		  0x000F },
	}, {
		/* R5G5B5A1 */
		16,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0001 },
	}, {
		/* R5G5B5 */
		16,
		{ 0xF800,
		  0x07C0,
		  0x003E,
		  0x0000 },
	}, {
		/* R8G8B8 */
		24,
		{ 0xFF0000,
		  0x00FF00,
		  0x0000FF,
		  0x000000 }
	}, {
		/* Paletted */
		8,
		{ 0,0,0,0 } /* N/A */
	}, {
		/* B8G8R8A8 */
		24,
		{ 0x0000FF,
		  0x00FF00,
		  0xFF0000,
		  0x000000 }
	}, {
		/* A1B5G5R5 */
		16,
		{ 0x7C00,
		  0x03E0,
		  0x001F,
		  0x8000 },
	}
};


RageDisplay_Null::RageDisplay_Null( VideoModeParams p )
{
	LOG->MapLog("renderer", "Current renderer: null");
	SetVideoMode( p );
}

SDL_Surface* RageDisplay_Null::CreateScreenshot()
{
	const PixelFormatDesc &desc = PIXEL_FORMAT_DESC[FMT_RGB8];
	SDL_Surface *image = SDL_CreateRGBSurfaceSane(
		SDL_SWSURFACE, 640, 480,
		desc.bpp, desc.masks[0], desc.masks[1], desc.masks[2], desc.masks[3] );

	memset( image->pixels, 0, 480*image->pitch );

	return image;
}

const RageDisplay::PixelFormatDesc *RageDisplay_Null::GetPixelFormatDesc(PixelFormat pf) const
{
	ASSERT( pf < NUM_PIX_FORMATS );
	return &PIXEL_FORMAT_DESC[pf];
}


RageMatrix RageDisplay_Null::GetOrthoMatrix( float l, float r, float b, float t, float zn, float zf )
{
	RageMatrix m(
		2/(r-l),      0,            0,           0,
		0,            2/(t-b),      0,           0,
		0,            0,            -2/(zf-zn),   0,
		-(r+l)/(r-l), -(t+b)/(t-b), -(zf+zn)/(zf-zn),  1 );
	return m;
}


void RageDisplay_Null::EndFrame()
{
	ProcessStatsOnFlip();
}
	
