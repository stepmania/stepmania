/*
 *  MovieTexture_null.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Wed Jul 16 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "global.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "MovieTexture_Null.h"
#include "SDL_utils.h"

MovieTexture_Null::MovieTexture_Null(RageTextureID ID) : RageMovieTexture(ID)
{
    LOG->Trace("MovieTexture_Null::MovieTexture_Null(ID)");
    texHandle = 0;

    RageTextureID actualID = GetID();

    actualID.iAlphaBits = 0;
	int size = 64;
    m_iSourceWidth = size;
    m_iSourceHeight = size;
    m_iImageWidth = size;
    m_iImageHeight = size;
    m_iTextureWidth = power_of_two(size);
    m_iTextureHeight = m_iTextureWidth;
    m_iFramesWide = 1;
    m_iFramesHigh = 1;

    CreateFrameRects();

    RageDisplay::PixelFormat pixfmt = RageDisplay::FMT_RGBA4;

    const RageDisplay::PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc( pixfmt );
    SDL_Surface *img = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, size, size, pfd->bpp, pfd->masks[0],
                                                pfd->masks[1], pfd->masks[2], pfd->masks[3]);

    texHandle = DISPLAY->CreateTexture( pixfmt, img, false );

    SDL_FreeSurface(img);
}

MovieTexture_Null::~MovieTexture_Null()
{
    DISPLAY->DeleteTexture( texHandle );
}
