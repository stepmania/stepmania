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
    //Movie movie;
    //Track track;
    long stopPos;
    //long lastPos;
    bool stopping;
    //long totalMediaSamples;
    //SoundDescriptionHandle sndDescHdl;
    //short dataRefIndices[4];

    soundInfo() { snd=NULL; stopPos=0; stopping=false; }
    //static soundInfo **MakeSoundInfoHdl();
    //static void DisposeSoundInfoHdl(soundInfo **sHdl);
    //void Reset();
    void DisposeSampleReferences(Media media, short index);
    //void GetData();
} *soundInfoPtr, **soundInfoHdl;

/*soundInfoHdl soundInfo::MakeSoundInfoHdl()
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

    bzero(s->dataRefIndices, 8);
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
}*/

/* Make sure that the Handle to this data is locked before calling this method. */
/*void soundInfo::Reset()
{
    Media media = GetTrackMedia(track);

    if (media)
    {
        this->DisposeSampleReferences(media, 0);
        this->DisposeSampleReferences(media, 1);
        this->DisposeSampleReferences(media, 2);
        DisposeTrackMedia(media);
    }
    //dataRefIndices[3] = 0;
    stopPos = 0;
    lastPos = 0;
    //snd = NULL;
    GoToBeginningOfMovie(movie);
}*/

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

/* Make sure that the Handle to this data is locked before calling this method. */
/*inline void soundInfo::GetData()
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
    OSErr err = MemError();
    TEST_ERR(err, PtrToHand);
    Handle dataRef;
    PtrToHand(&sndHdl, &dataRef, sizeof(Handle));
    err = MemError();
    TEST_ERR(err, PtrToHand);
    Media media = GetTrackMedia(track);
    bool insert;

    if (media)
    {
        short index;
        err = AddMediaDataRef(media, &index, dataRef, HandleDataHandlerSubType);
        TEST_ERR(err, AddMediaDataRef);
        ASSERT(index);
        dataRefIndices[dataRefIndices[3] % 3] = index;
        insert = false;
    }
    else
    {
        media = NewTrackMedia(track, SoundMediaType, freq, dataRef, HandleDataHandlerSubType);
        OSErr err = GetMoviesError();
        TEST_ERR(err, NewTrackMedia);
        dataRefIndices[0] = 1;
        insert = true;
    }
    CHECKPOINT;
    BeginMediaEdits(media);
    AddMediaSample(media, sndHdl, 0, got, 1, SampleDescriptionHandle(sndDescHdl), samples, 0, NULL);
    EndMediaEdits(media);
    CHECKPOINT;
    if (insert)
        InsertMediaIntoTrack(track, -1, lastPos - samples, got / samplesize, 0x00010000);
    CHECKPOINT;

    if (++dataRefIndices[3] > 3)
    {
        SetMovieActiveSegment(movie, (dataRefIndices[3] - 2) * samples, twicesamples);
        DisposeSampleReferences(media, dataRefIndices[3] % 3);
    }
    CHECKPOINT;
}


static void CallBack(QTCallBack cb, long refCon)
{
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
}*/

//static vector<soundInfoHdl> playingSounds;
//static queue<soundInfoHdl> freeSounds;
static vector<soundInfoPtr> sounds;
static long lastPos;
static Movie movie;
static Track track;

static void CallBack(QTCallBack cb, long refCon)
{
#pragma unused(refCon)
    while (!SOUNDMAN)
        SDL_Delay(10);
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
    bool insert;

    if (media)
    {
        short index;
        err = AddMediaDataRef(media, &index, dataRef, HandleDataHandlerSubType);
        TEST_ERR(err, AddMediaDataRef);
        ASSERT(index);
        dataRefIndices[dataRefIndices[3] % 3] = index;
        insert = false;
    }
    else
    {
        media = NewTrackMedia(track, SoundMediaType, freq, dataRef, HandleDataHandlerSubType);
        OSErr err = GetMoviesError();
        TEST_ERR(err, NewTrackMedia);
        dataRefIndices[0] = 1;
        insert = true;
    }
    CHECKPOINT;
    BeginMediaEdits(media);
    AddMediaSample(media, sndHdl, 0, samples, 1, SampleDescriptionHandle(sndDescHdl), samples, 0, NULL);
    EndMediaEdits(media);
    CHECKPOINT;
    if (insert)
        InsertMediaIntoTrack(track, -1, lastPos - samples, samples, 0x00010000);
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
    /*playingSounds.reserve(initialSounds);
    for (unsigned i=0; i<initialSounds; ++i)
    {
        soundInfoHdl sHdl = soundInfo::MakeSoundInfoHdl();
        freeSounds.push(sHdl);
    }*/
    lastPos = 0;
    bzero(dataRefIndices, 8);

    movie = NewMovie(newMovieActive);
    OSErr err = GetMoviesError();
    TEST_ERR(err, NewMovie);
    SetMovieTimeScale(movie, freq);
    err = GetMoviesError();
    TEST_ERR(err, SetMovieTimeScale);

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
    /*while (!playingSounds.empty())
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
    }*/
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
    /*soundInfoHdl sHdl;
    if (freeSounds.empty())
        sHdl = soundInfo::MakeSoundInfoHdl();
    else
    {
        sHdl = freeSounds.front();
        freeSounds.pop();
    }
    (*sHdl)->snd = snd;
    playingSounds.push_back(sHdl);
    CallBack(NULL, long(sHdl));*/
    soundInfoPtr s = new soundInfo;
    s->snd = snd;
    sounds.push_back(s);
}

void RageSound_QT::StopMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);
    CHECKPOINT;
    LOG->Trace("StopMixing");

    /*for (vector<soundInfoHdl>::iterator iter=playingSounds.begin(); iter<playingSounds.end(); ++iter)
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
    }*/
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

    /*vector<soundInfoHdl> snds = playingSounds;
    for (vector<soundInfoHdl>::iterator iter=snds.begin(); iter<snds.end(); ++iter)
    {
        soundInfoHdl sHdl = *iter;

        HLock(Handle(sHdl));
        OSErr err = MemError();
        TEST_ERR(err, HLock);
        if (IsMovieDone((*sHdl)->movie))
            (*sHdl)->snd->StopPlaying();
        else
            HUnlock(Handle(sHdl));
        CHECKPOINT;
    }*/
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

    /*for (vector<soundInfoHdl>::iterator iter=playingSounds.begin(); iter<playingSounds.end(); ++iter)
    {
        soundInfoHdl sHdl = *iter;

        if ((*sHdl)->snd == snd)
            return GetMovieTime((*sHdl)->movie, NULL);
    }
    RageException::Throw("Can't get the position of a sound that isn't playing");*/
    return GetMovieTime(movie, NULL);
}

float RageSound_QT::GetPlayLatency() const
{
    return float(halfsamples / 2) / freq;
    //return 0;
}
