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

MovieTexture_Null::MovieTexture_Null(RageTextureID ID) : RageMovieTexture(ID) {
    LOG->Trace("MovieTexture_Null::MovieTexture_Null(ID)");
    texHandle = 0;

    RageTextureID actualID = GetID();

    actualID.iAlphaBits = 0;
    long size = actualID.iMaxSize = min(actualID.iMaxSize, DISPLAY->GetMaxTextureSize());
    m_iSourceWidth = size;
    m_iSourceHeight = size;
    m_iImageWidth = size;
    m_iImageHeight = size;
    m_iTextureWidth = power_of_two(size);
    m_iTextureHeight = m_iTextureWidth;
    m_iFramesWide = 1;
    m_iFramesHigh = 1;

    CreateFrameRects();

    PixelFormat pixfmt;

    switch( TEXTUREMAN->GetMovieColorDepth() ){
        default:
            ASSERT(0);
        case 16:
            if( DISPLAY->SupportsTextureFormat(FMT_RGB5) )
                pixfmt = FMT_RGB5;
            else
                pixfmt = FMT_RGBA4;	// everything supports RGBA4
            break;
        case 32:
            if( DISPLAY->SupportsTextureFormat(FMT_RGB8) )
                pixfmt = FMT_RGB8;
            else if( DISPLAY->SupportsTextureFormat(FMT_RGBA8) )
                pixfmt = FMT_RGBA8;
            else if( DISPLAY->SupportsTextureFormat(FMT_RGB5) )
                pixfmt = FMT_RGB5;
            else
                pixfmt = FMT_RGBA4;	// everything supports RGBA4
            break;
    }

    const PixelFormatDesc *pfd = DISPLAY->GetPixelFormatDesc(pixfmt);
    SDL_Surface *img = SDL_CreateRGBSurfaceSane(SDL_SWSURFACE, size, size, pfd->bpp, pfd->masks[0],
                                                pfd->masks[1], pfd->masks[2], pfd->masks[3]);
    texHandle = DISPLAY->CreateTexture(pixfmt, img);
    //DISPLAY->UpdateTexture(texHandle, img, 0, 0, size, size);
    SDL_FreeSurface(img);
}

MovieTexture_Null::~MovieTexture_Null() {
    DISPLAY->DeleteTexture(texHandle);
}
