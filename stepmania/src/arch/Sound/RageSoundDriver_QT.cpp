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

const unsigned numchannels = 2;
//const unsigned samplesize = 2 * numchannels;
const unsigned samplesize = 16 * numchannels;
const unsigned samples = 4096;
const unsigned freq = 44100;
//const unsigned buffersize = samples * samplesize;
const unsigned buffersize = samples * samplesize / 8;

#if defined(DEBUG)
#define TEST_ERR(err, func) \
if (__builtin_expect(err, noErr)) \
    RageException::Throw(#func " failed with error: %d", err)
#else
#define TEST_ERR(err, func)
#endif

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

RageSound_QT::RageSound_QT() {
#if 1
    RageException::ThrowNonfatal("Class not finished!");
#endif
}

RageSound_QT::~RageSound_QT() {
    LockMutex L(SOUNDMAN->lock);
    while (!sounds.empty()) {
        soundInfoHdl sHdl = sounds.back();
        HLock(Handle(sHdl));
        OSStatus err = MemError();
        TEST_ERR(err, HLock);
        soundInfoPtr s = *sHdl;
        if (s->movie[0]) {
            StopMovie(s->movie[0]);
            DisposeMovie(s->movie[0]);
        }
        if (s->movie[1]) {
            StopMovie(s->movie[1]);
            DisposeMovie(s->movie[1]);
        }
        HUnlock(Handle(sHdl));
        err = MemError();
        TEST_ERR(err, HUnlock);
        DisposeHandle(Handle(sHdl));
        err = MemError();
        TEST_ERR(err, DisposeHandle);
        sounds.pop_back();
    }
}

void RageSound_QT::StartMixing(RageSound *snd) {
    LockMutex L(SOUNDMAN->lock);
    LOG->Trace("StartMixing()");
    soundInfoHdl sHdl = (soundInfoHdl)NewHandle(sizeof(soundInfo));
    OSStatus err = MemError();
    TEST_ERR(err, NewHandle);
    HLock(Handle(sHdl));
    err = MemError();
    TEST_ERR(err, HLock);
    soundInfoPtr s = *sHdl;

    s->fillme = 0;
    s->movie[0] = NULL;
    s->movie[1] = NULL;
    s->stopPos = 0;
    s->lastPos = 0;
    s->snd = snd;
    s->finishedBuffers = -1;
    HUnlock(Handle(sHdl));
    err = MemError();
    TEST_ERR(err, HUnlock);
    sounds.push_back(sHdl);
    CallBack(NULL, long(sHdl));
}

void RageSound_QT::StopMixing(RageSound *snd) {
    LockMutex L(SOUNDMAN->lock);
    LOG->Trace("StopMixing");
    unsigned size = sounds.size();

    for (unsigned i=0; i<size; ++i) {
        soundInfoHdl sHdl = sounds[i];
        HLock(Handle(sHdl));
        OSStatus err = MemError();
        TEST_ERR(err, HLock);
        soundInfoPtr s = *sHdl;
        if (s->snd != snd) {
            HUnlock(Handle(sHdl));
            err = MemError();
            TEST_ERR(err, HUnlock);
            continue;
        }
        if (s->movie[0]) {
            StopMovie(s->movie[0]);
            DisposeMovie(s->movie[0]);
        }
        if (s->movie[1]) {
            StopMovie(s->movie[1]);
            DisposeMovie(s->movie[1]);
        }
        sounds.erase(sounds.begin()+i);
        HUnlock(Handle(sHdl));
        err = MemError();
        TEST_ERR(err, HUnlock);
        DisposeHandle(Handle(sHdl));
        err = MemError();
        TEST_ERR(err, DisposeHandle);
        break;
    }
}

void RageSound_QT::Update(float delta) {
#pragma unused (delta)
    LockMutex L(SOUNDMAN->lock);
    unsigned size = sounds.size();
    vector<soundInfoHdl> snds = sounds;
    OSErr err;

    for (unsigned i=0; i<size; ++i) {
        soundInfoHdl sHdl = snds[i];
        HLock(Handle(sHdl));
        err = MemError();
        TEST_ERR(err, HLock);
        soundInfoPtr s = *sHdl;
        if ((s->movie[0] && !IsMovieDone(s->movie[0])) ||
            (s->movie[1] && !IsMovieDone(s->movie[1]))) {
            HUnlock(Handle(sHdl));
            continue;
        }
        /* StopPlaying() will call StopMixing which will unlock and dispose of the handle */
        s->snd->StopPlaying();
    }
}

int RageSound_QT::GetPosition(const RageSound *snd) const {
    LockMutex L(SOUNDMAN->lock);
    unsigned size = sounds.size();
   /* Find the info */
    for (unsigned i=0; i<size; ++i) {
        soundInfoHdl sHdl = sounds[i];
        HLock(Handle(sHdl));
        OSStatus err = MemError();
        TEST_ERR(err, HLock);
        soundInfoPtr s = *sHdl;
        if (s->snd != snd) {
            HUnlock(Handle(sHdl));
            continue;
        }
        if (s->finishedBuffers == -1) {
            HUnlock(Handle(sHdl));
            return 0; /* It hasn't started yet. */
        }
        int ret = s->finishedBuffers * samples + GetMovieTime(s->movie[!s->fillme], NULL);
        HUnlock(Handle(sHdl));
        LOG->Trace("GetPostion is returning: %d", ret);
        return ret;
    }
    RageException::Throw("Can't get the position of a sound that isn't playing");
    //return 0; /* unrechable statement */
}


void CallBack(QTCallBack cb, long refCon) {
    LockMutex L(SOUNDMAN->lock);
    LOG->Trace("CallBack");
    soundInfoHdl sHdl = soundInfoHdl(refCon);
    HLock(Handle(sHdl));
    OSStatus err = MemError();
    TEST_ERR(err, HLock);
    soundInfoPtr s = *sHdl;

    s->finishedBuffers++;
    if (!cb)
        GetData(s);
    long playme = !s->fillme;
    ASSERT(s->movie[playme]);
    GoToBeginningOfMovie(s->movie[playme]);
    StartMovie(s->movie[playme]);
    err = GetMoviesError();
    TEST_ERR(err, StartMovie);
    if (s->stopPos == 0) {
        /* Not stopping */
        GetData(s);
        QTCallBack callBack = NewCallBack(GetMovieTimeBase(s->movie[playme]), callBackAtExtremes);
        CallMeWhen(callBack, CallBack, long(sHdl), 0, 0, 0);
    }
    HUnlock(Handle(sHdl));
    err = MemError();
    TEST_ERR(err, HUnlock);
    DisposeCallBack(cb);
    
}

inline void GetData(soundInfoPtr s) {
    LOG->Trace("GetData");
    char buffer[buffersize];
    unsigned got = s->snd->GetPCM(buffer, buffersize, s->lastPos);

    if (got < buffersize) {
        s->stopPos = got / samplesize + s->lastPos;
        if (got == 0)
            return;
    }
    s->lastPos += samples;
    
    OSErr err = noErr;
    if (s->movie[s->fillme])
        DisposeMovie(s->movie[s->fillme]);

    s->movie[s->fillme] = NewMovie(newMovieActive);
    err = GetMoviesError();
    TEST_ERR(err, NewMovie);
    SetMovieTimeScale(s->movie[s->fillme], freq);
    err = GetMoviesError();
    TEST_ERR(err, SetMovieTimeScale);

    /* Put the sound data into a handle */
    Handle sndHdl;

    err = PtrToHand(buffer, &sndHdl, got);
    TEST_ERR(err, PtrToHand);

    
    /* Create the SoundDescription */
    SoundDescriptionHandle sndDescHdl = (SoundDescriptionHandle)NewHandleClear(sizeof(SoundDescription));
    
    err = MemError();
    TEST_ERR(err, NewHandle);
    {
        SoundDescriptionPtr sndDescPtr = *sndDescHdl;
        sndDescPtr->descSize = sizeof(SoundDescription);
        sndDescPtr->dataFormat = k16BitBigEndianFormat;
        sndDescPtr->numChannels = numchannels;
        sndDescPtr->sampleSize = samplesize / numchannels;
        sndDescPtr->sampleRate = freq << 16;
    }

    /* Add data to media and then add media to track */
    Handle dataRef;

    PtrToHand(&sndHdl, &dataRef, sizeof(Handle));
    LOG->Trace("Create new Track");
    Track track = NewMovieTrack(s->movie[s->fillme], 0, 0, kFullVolume);
    err = GetMoviesError();
    TEST_ERR(err, NewMovieTrack);
    ASSERT(track);

    LOG->Trace("Create new Media");
    Media media = NewTrackMedia(track, SoundMediaType, freq, dataRef, HandleDataHandlerSubType);
    err = GetMoviesError();
    TEST_ERR(err, NewTrackMedia);
    ASSERT(media);

    LOG->Trace("BeginMediaEdits");
    err = BeginMediaEdits(media);
    TEST_ERR(err, BeginMediaEdits);
    err = AddMediaSample(media, sndHdl, 0, got, 1, (SampleDescriptionHandle)sndDescHdl,
                         samples, 0, NULL);
    TEST_ERR(err, AddMediaSample);
    err = EndMediaEdits(media);
    TEST_ERR(err, EndMediaEdits);
    err = InsertMediaIntoTrack(track, 0, 0, GetMediaDuration(media), 0x00010000);
    TEST_ERR(err, InsertMediaIntoTrack);
    if (sndDescHdl)
        DisposeHandle(Handle(sndDescHdl));
    s->fillme = !s->fillme;
}

