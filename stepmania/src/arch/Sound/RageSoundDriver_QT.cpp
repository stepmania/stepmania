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
const unsigned samples = 2048;
const long halfsamples = long(samples / 2);
const long twicesamples = long(samples * 2);
const unsigned freq = 44100;
const unsigned buffersize = samples * samplesize;
const unsigned initialSounds = 10;

#define TEST_ERR(err, func) \
CHECKPOINT; \
if (__builtin_expect(err, noErr)) \
RageException::Throw(#func " failed with error: %d at %s:%d", err, __FILE__, __LINE__)

typedef struct soundInfo
{
    RageSound *snd;
    long stopPos;
    bool stopping;

    soundInfo() { snd=NULL; stopPos=0; stopping=false; }
    void DisposeSampleReferences(Media media, short index);
} *soundInfoPtr;

static short dataRefIndices[4];
static SoundDescriptionHandle sndDescHdl;

//void soundInfo::DisposeSampleReferences(Media media, short index)
void DisposeSampleReferences(Media media, short index)
{
    LockMutex L(SOUNDMAN->lock);
    ASSERT(index <= 2 && index >= 0);

    if (!dataRefIndices[index])
        return;

    Handle dataRef;
    OSType dataRefType;
    long dataRefAttributes;
    OSErr err = GetMediaDataRef(media, dataRefIndices[index], &dataRef, &dataRefType, &dataRefAttributes);
    TEST_ERR(err, GetMediaDataRef);
    ASSERT(dataRefType == HandleDataHandlerSubType);
    ASSERT(!(dataRefAttributes & dataRefWasNotResolved));
    HLock(dataRef);
    Handle sndHdl;
    memcpy(&sndHdl, *dataRef, sizeof(Handle));
    DisposeHandle(sndHdl);
    HUnlock(dataRef);
    DisposeHandle(dataRef);
    dataRefIndices[index] = 0;
}

static vector<soundInfoPtr> sounds;
static long lastPos;
static Movie movie;
static Track track;

static void CallBack(QTCallBack cb, long refCon)
{
#pragma unused(refCon)
    LockMutex L(SOUNDMAN->lock);
    static SoundMixBuffer mix;
    char buffer[buffersize];

    bzero(buffer, buffersize);
    for (unsigned i=0; i<sounds.size(); ++i)
    {
        if (sounds[i]->stopping)
            continue;

        unsigned got = sounds[i]->snd->GetPCM(buffer, buffersize, lastPos);

        mix.write((SInt16 *)buffer, got / 2);
        if (got < buffersize)
        {
            sounds[i]->stopping = true;
            sounds[i]->stopPos = lastPos + got / samplesize;
        }
    }
    lastPos += samples;

    mix.read((SInt16 *)buffer);
    Handle sndHdl;
    PtrToHand(buffer, &sndHdl, buffersize);
    OSErr err = MemError();
    TEST_ERR(err, PtrToHand);
    Handle dataRef;
    PtrToHand(&sndHdl, &dataRef, sizeof(Handle));
    err = MemError();
    TEST_ERR(err, PtrToHand);
    Media media = GetTrackMedia(track);
    
    if (media)
    {
        short index;
        err = AddMediaDataRef(media, &index, dataRef, HandleDataHandlerSubType);
        TEST_ERR(err, AddMediaDataRef);
        ASSERT(index);
        dataRefIndices[dataRefIndices[3] % 3] = index;
    }
    else
    {
        media = NewTrackMedia(track, SoundMediaType, freq, dataRef, HandleDataHandlerSubType);
        err = GetMoviesError();
        TEST_ERR(err, NewTrackMedia);
        dataRefIndices[0] = 1;
    }
    CHECKPOINT;
    TimeValue tv;
    BeginMediaEdits(media);
    AddMediaSample(media, sndHdl, 0, samples, 1, SampleDescriptionHandle(sndDescHdl), samples, 0, &tv);
    EndMediaEdits(media);
    err = GetMoviesError();
    TEST_ERR(err, AddMediaSample);
    CHECKPOINT;
    InsertMediaIntoTrack(track, -1, tv, samples, 0x00010000);
    CHECKPOINT;

    if (++dataRefIndices[3] > 3)
    {
        SetMovieActiveSegment(movie, (dataRefIndices[3] - 2) * samples, twicesamples);
        DisposeSampleReferences(media, dataRefIndices[3] % 3);
    }
    CHECKPOINT;

    long timeToCall;
    if (!cb)
    {
        LOG->Trace("Start the movie");
        GoToBeginningOfMovie(movie);
        StartMovie(movie);
        err = GetMoviesError();
        TEST_ERR(err, StartMovie);
        cb = NewCallBack(GetMovieTimeBase(movie), callBackAtTime);
        timeToCall = halfsamples;
    }
    else
        timeToCall = lastPos - halfsamples;
    CallMeWhen(cb, CallBack, 0, triggerTimeFwd, timeToCall, freq);
}    

RageSound_QT::RageSound_QT()
{
    lastPos = 0;
    bzero(dataRefIndices, 8);

    movie = NewMovie(newMovieActive);
    OSErr err = GetMoviesError();
    TEST_ERR(err, NewMovie);
    SetMovieTimeScale(movie, freq);
    err = GetMoviesError();
    TEST_ERR(err, SetMovieTimeScale);
    SetMovieVolume(movie, kFullVolume);

    track = NewMovieTrack(movie, 0, 0, kFullVolume);
    err = GetMoviesError();
    TEST_ERR(err, NewMovieTrack);    

    sndDescHdl = (SoundDescriptionHandle)NewHandleClear(sizeof(SoundDescription));
    HLock(Handle(sndDescHdl));
    err = MemError();
    TEST_ERR(err, HLock);
    SoundDescriptionPtr sndDescPtr = *sndDescHdl;
    sndDescPtr->descSize = sizeof(SoundDescription);
    sndDescPtr->dataFormat = k16BitBigEndianFormat;
    sndDescPtr->numChannels = numchannels;
    sndDescPtr->sampleSize = samplesize * 8 / numchannels;
    sndDescPtr->sampleRate = freq << 16;
    HUnlock(Handle(sndDescHdl));
    err = MemError();
    TEST_ERR(err, HUnlock);

    /* Start the Movie */
    CallBack(NULL, 0);
}

RageSound_QT::~RageSound_QT()
{
    LockMutex L(SOUNDMAN->lock);
    while (!sounds.empty())
    {
        soundInfoPtr s = sounds.back();
        s->snd->StopPlaying();
        sounds.pop_back();
    }
}

void RageSound_QT::StartMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);
    CHECKPOINT;
    LOG->Trace("StartMixing()");
    soundInfoPtr s = new soundInfo;
    s->snd = snd;
    sounds.push_back(s);
}

void RageSound_QT::StopMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);
    CHECKPOINT;
    LOG->Trace("StopMixing");

    for (unsigned i=0; i<sounds.size(); ++i)
    {
        if (sounds[i]->snd != snd)
            continue;
        sounds.erase(sounds.begin()+i);
        return;
    }

    LOG->Warn("A sound could not be stopped because it was not being played.");
}

void RageSound_QT::Update(float delta)
{
#pragma unused (delta)
    LockMutex L(SOUNDMAN->lock);
    vector<soundInfoPtr> snds = sounds;
    for (unsigned i=0; i<snds.size(); ++i)
    {
        if (!snds[i]->stopping)
            continue;
        if (GetPosition(snds[i]->snd) < snds[i]->stopPos)
            continue;
        snds[i]->snd->StopPlaying();
    }
}

int RageSound_QT::GetPosition(const RageSound *snd) const
{
    LockMutex L(SOUNDMAN->lock);
    return GetMovieTime(movie, NULL);
}

float RageSound_QT::GetPlayLatency() const
{
    return float(halfsamples / 2) / freq;
}
