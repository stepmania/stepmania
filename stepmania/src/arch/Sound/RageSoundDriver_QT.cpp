/*
 *  RageSoundDriver_QT.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Mon Jun 23 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "global.h"
#include "RageSoundDriver_QT.h"
#include "RageSoundManager.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageThreads.h"
#include <QuickTime/QuickTime.h>
#include <queue>
#include <vector>
#include <pthread.h>
#include <sys/types.h>

const unsigned samplesize = 4;
const unsigned samples = 4096;
const unsigned freq = 44100;
const unsigned buffersize = samples * samplesize;

typedef struct soundInfo{
    RageSound *snd;
    Movie movie[2];
    long fillme;
    long stopPos;
    long lastPos;
    long finishedBuffers;
} *soundInfoPtr, **soundInfoHdl;

static void CallBack(QTCallBack cb, long refCon);
static inline void GetData(soundInfoPtr s);

static vector<soundInfoHdl> sounds;
static MovieImportComponent miComponent;

RageSound_QT::RageSound_QT() {
#if 1
    RageException::ThrowNonfatal("Class not finished!");
#endif
    if (EnterMovies() != noErr)
        RageException::ThrowNonfatal("Could not start movies. Falling back.");
    miComponent = OpenDefaultComponent(MovieImportType, kQTFileTypeWave);
}

RageSound_QT::~RageSound_QT() {
    LockMutex L(SOUNDMAN->lock);
    while (!sounds.empty()) {
        soundInfoHdl sHdl = sounds.back();
        soundInfoPtr s = *sHdl;
        if (s->movie[0]) {
            StopMovie(s->movie[0]);
            DisposeMovie(s->movie[0]);
        }
        if (s->movie[1]) {
            StopMovie(s->movie[1]);
            DisposeMovie(s->movie[1]);
        }
        DisposeHandle(Handle(sHdl));
        sounds.pop_back();
    }
    if (miComponent)
        CloseComponent(miComponent);
}

void RageSound_QT::StartMixing(RageSound *snd) {
    LockMutex L(SOUNDMAN->lock);
    soundInfoHdl sHdl = (soundInfoHdl)NewHandle(sizeof(soundInfo));
    HLockHi(Handle(sHdl)); /* Move to the top of the heap */
    soundInfoPtr s = *sHdl;

    s->fillme = 0;
    s->movie[0] = NULL;
    s->movie[1] = NULL;
    s->stopPos = 0;
    s->lastPos = 0;
    s->snd = snd;
    s->finishedBuffers = -1;
    sounds.push_back(sHdl);
    CallBack(NULL, long(s));
}

void RageSound_QT::StopMixing(RageSound *snd) {
    LockMutex L(SOUNDMAN->lock);
    unsigned size = sounds.size();

    for (unsigned i=0; i<size; ++i) {
        soundInfoHdl sHdl = sounds[i];
        soundInfoPtr s = *sHdl;
        if (s->snd != snd)
            continue;
        if (s->movie[0]) {
            StopMovie(s->movie[0]);
            DisposeMovie(s->movie[0]);
        }
        if (s->movie[1]) {
            StopMovie(s->movie[1]);
            DisposeMovie(s->movie[1]);
        }
        DisposeHandle(Handle(sHdl));
    }
}

void RageSound_QT::Update(float delta) {
#pragma unused (delta)
    LockMutex L(SOUNDMAN->lock);
    unsigned size = sounds.size();
    vector<soundInfoHdl> snds = sounds;

    for (unsigned i=0; i<size; ++i) {
        soundInfoPtr s = *(snds[i]);
        if ((s->movie[0] && !IsMovieDone(s->movie[0])) ||
            (s->movie[1] && !IsMovieDone(s->movie[1])))
            continue;
        s->snd->StopPlaying();
    }
}

int RageSound_QT::GetPosition(const RageSound *snd) const {
    LockMutex L(SOUNDMAN->lock);
    soundInfoPtr s;
    unsigned size = sounds.size();
   /* Find the info */
    for (unsigned i=0; i<size; ++i) {
        s = *(sounds[i]);
        if (s->snd != snd)
            continue;
        if (s->finishedBuffers == -1)
            return 0; /* It hasn't started yet. */
        return s->finishedBuffers * samples + GetMovieTime(s->movie[!s->fillme], NULL);
    }
    RageException::Throw("Can't get the position of a sound that isn't playing");
    return 0; /* unrechable statement */
}


void CallBack(QTCallBack cb, long refCon) {
    LockMutex L(SOUNDMAN->lock);
    soundInfoPtr s = soundInfoPtr(refCon);

    s->finishedBuffers++;
    if (!cb)
        GetData(s);
    long playme = !s->fillme;
    SetMovieVolume(s->movie[playme], kFullVolume);
    GoToBeginningOfMovie(s->movie[playme]);
    StartMovie(s->movie[playme]);
    if (s->stopPos == 0) {
        /* Not stopping */
        GetData(s);
        QTCallBack callBack = NewCallBack(GetMovieTimeBase(s->movie[playme]), callBackAtExtremes);
        CallMeWhen(callBack, CallBack, long(s), 0, 0, 0);
    }
    DisposeCallBack(cb);
}

inline void GetData(soundInfoPtr s) {
    static long flags = newMovieActive;
    Handle hdl, dataRef = NULL;
    Track targetTrack = NULL;
    TimeValue addedDuration = 0;
    long outFlags = 0;
    OSErr err;
    ComponentResult result;    
    Movie &movie = s->movie[s->fillme];
    char buffer[buffersize];
    int got = s->snd->GetPCM(buffer, buffersize, s->lastPos);

    if (got < buffersize) {
        s->stopPos = got / samplesize + s->lastPos;
        if (got == 0)
            return;
    }
    s->lastPos += samples;

    /* I really don' understand this part. Oh well. */
    hdl = NewHandle(got);
    HLock(hdl);
    BlockMove(buffer, *hdl, got);
    err = PtrToHand(&hdl, &dataRef, sizeof(Handle)); /* um... */
    ASSERT(err == noErr);
    HUnlock(hdl);
    /* end unknown section. */

    if (movie)
        DisposeMovie(movie);
    
    movie = NewMovie(flags);
    SetMovieTimeScale(movie, freq);
    if (GetMoviesError() != noErr)
        RageException::Throw("Couldn't create new movie");
    result = MovieImportDataRef(miComponent, dataRef, HandleDataHandlerSubType,
                                movie, NULL, &targetTrack, NULL, &addedDuration,
                                movieImportCreateTrack, &outFlags);
    s->fillme = !s->fillme;
}

