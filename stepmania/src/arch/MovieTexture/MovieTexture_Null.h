#ifndef MOVIE_TEXTURE_NULL
#define MOVIE_TEXTURE_NULL
/*
 *  MovieTexture_null.h
 *  stepmania
 *
 *  Created by Steve Checkoway on Wed Jul 16 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "MovieTexture.h"
#include "RageTexture.h"

class MovieTexture_Null : public RageMovieTexture {
private:
    bool playing;
    bool loop;
    unsigned texHandle;

public:
    MovieTexture_Null(RageTextureID ID);
    virtual ~MovieTexture_Null();
    void Invalidate() { texHandle = 0; }
    unsigned GetTexHandle() const { return texHandle; }
    void Update(float delta) { }
    void Reload() { }
    void Play() { playing = true; }
    void Pause() { playing = false; }
    void Stop() { playing = false; }
    void SetPosition(float seconds) { }
    void SetPlaybackRate(float rate) { }
    bool IsPlaying() const { return playing; }
    void SetLooping(bool looping=true) { loop = looping; }
};

#endif /* MOVIE_TEXTURE_NULL */
