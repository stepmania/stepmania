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
const unsigned samplesize = 2 * numchannels;
const unsigned samples = 40960;
const long halfsamples = long(samples / 2);
const long twicesamples = long(samples * 2);
const unsigned freq = 44100;
const unsigned buffersize = samples * samplesize;
const unsigned initialSounds = 10;

#if defined(DEBUG)
#define TEST_ERR(err, func) \
CHECKPOINT; \
if (__builtin_expect(err, noErr)) \
    RageException::Throw(#func " failed with error: %d", err)
#else
#define TEST_ERR(err, func) CHECKPOINT
#endif

typedef struct soundInfo
{
    RageSound *snd;
    Movie movie;
    Track track;
    long stopPos;
    long lastPos;
    unsigned totalMediaSamples;
    unsigned firstLiveMediaSample;
    SoundDescriptionHandle sndDescHdl;
    
    static soundInfo **MakeSoundInfoHdl();
    static void DisposeSoundInfoHdl(soundInfo **sHdl);
    void Reset();
    void DisposeSampleReferences(Media media, unsigned endIndex = 0xFFFFFFFF);
    void GetData();
} *soundInfoPtr, **soundInfoHdl;

soundInfoHdl soundInfo::MakeSoundInfoHdl()
{
    soundInfoHdl sHdl = (soundInfoHdl)NewHandle(sizeof(soundInfo));
    OSStatus err = MemError();
    TEST_ERR(err, NewHandle);
    HLock(Handle(sHdl));
    err = MemError();
    TEST_ERR(err, HLock);
    soundInfoPtr s = *sHdl;

    s->movie = NewMovie(newMovieActive);
    err = GetMoviesError();
    TEST_ERR(err, NewMovie);
    SetMovieTimeScale(s->movie, freq);
    err = GetMoviesError();
    TEST_ERR(err, SetMovieTimeScale);

    s->track = NewMovieTrack(s->movie, 0, 0, kFullVolume);
    err = GetMoviesError();
    TEST_ERR(err, NewMovieTrack);

    s->sndDescHdl = (SoundDescriptionHandle)NewHandleClear(sizeof(SoundDescription));
    err = MemError();
    TEST_ERR(err, NewHandleClear);
    HLock(Handle(s->sndDescHdl));
    err = MemError();
    TEST_ERR(err, HLock);
    SoundDescriptionPtr sndDescPtr = *s->sndDescHdl;
    sndDescPtr->descSize = sizeof(SoundDescription);
    sndDescPtr->dataFormat = k16BitBigEndianFormat;
    sndDescPtr->numChannels = numchannels;
    sndDescPtr->sampleSize = samplesize * 8 / numchannels;
    sndDescPtr->sampleRate = freq << 16;
    HUnlock(Handle(s->sndDescHdl));
    err = MemError();
    TEST_ERR(err, HUnlock);

    s->totalMediaSamples = 0;
    s->firstLiveMediaSample = 0;
    s->Reset();
    HUnlock(Handle(sHdl));
    err = MemError();
    TEST_ERR(err, HUnlock);
    return sHdl;
}

void soundInfo::DisposeSoundInfoHdl(soundInfoHdl sHdl)
{
    HLock(Handle(sHdl));
    soundInfoPtr s = soundInfoPtr(*sHdl);

    s->Reset();
    DisposeHandle(Handle(s->sndDescHdl));
    DisposeMovieTrack(s->track);
    DisposeMovie(s->movie);
    HUnlock(Handle(sHdl));
    DisposeHandle(Handle(sHdl));
}

/* Make sure that the Handle to this data is locked before calling this method. */
void soundInfo::Reset()
{
    Media media = GetTrackMedia(track);

    if (media)
    {
        this->DisposeSampleReferences(media); // All of them
        DisposeTrackMedia(media);
    }
    totalMediaSamples = 0;
    firstLiveMediaSample = 0;
    stopPos = 0;
    lastPos = 0;
    snd = NULL;
    GoToBeginningOfMovie(movie);
}

void soundInfo::DisposeSampleReferences(Media media, unsigned endIndex)
{
    LockMutex L(SOUNDMAN->lock);
    if (!firstLiveMediaSample)
        return;
    if (endIndex < firstLiveMediaSample)
    {
        LOG->Warn("Couldn't dispose of data to %d because it has already been "
                  "disposed.", endIndex);
        return;
    }
    ASSERT(totalMediaSamples);
    endIndex = min(endIndex, totalMediaSamples);

    for (; firstLiveMediaSample <= endIndex; ++firstLiveMediaSample)
    {
        Handle dataRef;
        OSType dataRefType;
        long dataRefAttributes;
        OSErr err = GetMediaDataRef(media, firstLiveMediaSample, &dataRef,
                                    &dataRefType, &dataRefAttributes);
        TEST_ERR(err, GetMediaDataRef);
        ASSERT(dataRefType == HandleDataHandlerSubType);
        ASSERT(!(dataRefAttributes & dataRefWasNotResolved));
        HLock(dataRef);
        Handle sndHdl;
        memcpy(&sndHdl, *dataRef, sizeof(Handle));
        DisposeHandle(sndHdl);
        HUnlock(dataRef);
        DisposeHandle(dataRef);
    }
}

/* Make sure that the Handle to this data is locked before calling this method. */
inline void soundInfo::GetData()
{
    CHECKPOINT;
    char buffer[buffersize];
    unsigned got = snd->GetPCM(buffer, buffersize, lastPos);

    if (got < buffersize)
    {
        stopPos = got / samplesize + lastPos;
        if (got == 0)
            return;
    }
    lastPos += samples;

    Handle sndHdl;
    PtrToHand(buffer, &sndHdl, got);
    Handle dataRef;
    PtrToHand(&sndHdl, &dataRef, sizeof(Handle));
    Media media = GetTrackMedia(track);
    OSErr err;

    if (media)
    {
        short index;
        err = AddMediaDataRef(media, &index, dataRef, HandleDataHandlerSubType);
        TEST_ERR(err, AddMediaDataRef);
        totalMediaSamples++;
        ASSERT(totalMediaSamples == unsigned(index));
    }
    else
    {
        media = NewTrackMedia(track, SoundMediaType, freq, dataRef, HandleDataHandlerSubType);
        err = GetMoviesError();
        TEST_ERR(err, NewTrackMedia);
        firstLiveMediaSample = 1;
        totalMediaSamples = 1;
    }
    CHECKPOINT;
    BeginMediaEdits(media);
    AddMediaSample(media, sndHdl, 0, got, 1, SampleDescriptionHandle(sndDescHdl), samples, 0, NULL);
    EndMediaEdits(media);
    CHECKPOINT;
    InsertMediaIntoTrack(track, -1, lastPos - samples, samples, 0x00010000);
    CHECKPOINT;

    if (totalMediaSamples > 3)
    {
        SetMovieActiveSegment(movie, (totalMediaSamples - 2) * samples, twicesamples);
        DisposeSampleReferences(media, totalMediaSamples - 3); //Remove one sample
    }
    CHECKPOINT;
}


static void CallBack(QTCallBack cb, long refCon) {
    LockMutex L(SOUNDMAN->lock);
    CHECKPOINT;
    soundInfoHdl sHdl = soundInfoHdl(refCon);
    HLock(Handle(sHdl));
    soundInfoPtr s = *sHdl;
    long timeToCall;

    s->GetData();
    if (!cb)
    {
        StartMovie(s->movie);
        cb = NewCallBack(GetMovieTimeBase(s->movie), callBackAtTime);
        timeToCall = halfsamples;
    }
    else
        timeToCall = s->lastPos - halfsamples;
    if (!s->stopPos)
        CallMeWhen(cb, CallBack, long(sHdl), triggerTimeFwd, timeToCall, freq);
    
    HUnlock(Handle(sHdl));
}

static vector<soundInfoHdl> playingSounds;
static queue<soundInfoHdl> freeSounds;

RageSound_QT::RageSound_QT()
{
#if 0
    RageException::ThrowNonfatal("Class not finished!");
#endif

    playingSounds.reserve(initialSounds);
    for (unsigned i=0; i<initialSounds; ++i)
    {
        soundInfoHdl sHdl = soundInfo::MakeSoundInfoHdl();
        freeSounds.push(sHdl);
    }
}

RageSound_QT::~RageSound_QT()
{
    LockMutex L(SOUNDMAN->lock);
    while (!playingSounds.empty())
    {
        soundInfoHdl &sHdl = playingSounds.back();
        soundInfo::DisposeSoundInfoHdl(sHdl);
        playingSounds.pop_back();
    }
    while (!freeSounds.empty())
    {
        soundInfoHdl &sHdl = freeSounds.front();
        soundInfo::DisposeSoundInfoHdl(sHdl);
        freeSounds.pop();
    }
}

void RageSound_QT::StartMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);
    CHECKPOINT;
    LOG->Trace("StartMixing()");
    soundInfoHdl sHdl;
    if (freeSounds.empty())
        sHdl = soundInfo::MakeSoundInfoHdl();
    else
    {
        sHdl = freeSounds.front();
        freeSounds.pop();
    }
    (*sHdl)->snd = snd;
    playingSounds.push_back(sHdl);
    CallBack(NULL, long(sHdl));
}

void RageSound_QT::StopMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);
    CHECKPOINT;
    LOG->Trace("StopMixing");

    for (vector<soundInfoHdl>::iterator iter=playingSounds.begin(); iter<=playingSounds.end(); ++iter)
    {
        soundInfoHdl sHdl = *iter;
        
        if ((*sHdl)->snd != snd)
            continue;
        HLock(Handle(sHdl));
        
        soundInfoPtr s = *sHdl;
        
        StopMovie(s->movie);
        s->Reset();
        playingSounds.erase(iter);
        freeSounds.push(sHdl);
        HUnlock(Handle(sHdl));
        return;
    }

    LOG->Warn("A sound could not be stopped because it was not being played.");
}

void RageSound_QT::Update(float delta)
{
#pragma unused (delta)
    LockMutex L(SOUNDMAN->lock);

    vector<soundInfoHdl> snds = playingSounds;
    for (vector<soundInfoHdl>::iterator iter=snds.begin(); iter<=snds.end(); ++iter)
    {
        soundInfoHdl sHdl = *iter;

        OSErr err = GetMoviesError();
        TEST_ERR(err, GetMoviesError);
        HLock(Handle(sHdl));
        err = MemError();
        TEST_ERR(err, MemError);
        if (IsMovieDone((*sHdl)->movie))
            (*sHdl)->snd->StopPlaying();
        else
            HUnlock(Handle(sHdl));
        CHECKPOINT;
    }
}

int RageSound_QT::GetPosition(const RageSound *snd) const
{
    LockMutex L(SOUNDMAN->lock);

    for (vector<soundInfoHdl>::iterator iter=playingSounds.begin(); iter<=playingSounds.end(); ++iter)
    {
        soundInfoHdl sHdl = *iter;

        if ((*sHdl)->snd == snd)
            return GetMovieTime((*sHdl)->movie, NULL);
    }
    RageException::Throw("Can't get the position of a sound that isn't playing");
}
