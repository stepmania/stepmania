/*
 *  RageSoundDriver_QT.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Mon Jun 23 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "global.h"
#include "RageSoundManager.h"
#include "RageSoundDriver_QT1.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageLog.h"
#include <QuickTime/QuickTime.h>
#include <stdlib.h>
/* Ugh */
using namespace QT;

const unsigned channels = 2;
const unsigned samplesize = channels*16;
const unsigned samples = 2048;
const unsigned freq = 44100;
const unsigned buffersize = samples*samplesize/8;

static volatile Uint32 fill_me = 0;
static UInt8  *buffer[2];
static CmpSoundHeader header;

RageSound_QT1::RageSound_QT1() {
    SndCallBackUPP callback;
    callback = NewSndCallBackUPP(GetData);
    memset(&header, 0, sizeof(header));
    header.numChannels = channels;
    header.sampleSize = samplesize / channels;
    header.sampleRate = rate44khz; /* really 44.1kHz */
    header.numFrames = samples;
    header.encode = cmpSH;

    buffer[0] = new Uint8[buffersize];
    buffer[1] = new Uint8[buffersize];
    memset(buffer[0], 0, buffersize);
    memset(buffer[1], 0, buffersize);

    channel = new SndChannel;
    channel->userInfo = reinterpret_cast<long>(this);
    channel->qLength = 2;

    latency = static_cast<float>(samples) / freq; /* double buffer */
    LOG->Trace("The output latency is %f", latency);

    OSErr err = SndNewChannel(&channel, sampledSynth, initStereo, callback);

    if (err != noErr) {
        delete channel;
        channel = NULL;
        RageException::ThrowNonfatal("Unable to create audio channel");
    }

    SndCommand cmd;
    cmd.cmd = clockComponentCmd;
    cmd.param1 = true;
    cmd.param2 = 0;
    err |= SndDoImmediate(channel, &cmd);

    cmd.cmd = getClockComponentCmd;
    cmd.param1 = 0;
    cmd.param2 = reinterpret_cast<long>(&clock);
    err |= SndDoImmediate(channel, &cmd);
    last_pos = 0;

    cmd.cmd = callBackCmd;
    cmd.param2 = 0;
    err |= SndDoCommand(channel, &cmd, false);

    if (err != noErr)
        RageException::ThrowNonfatal("Unable to create audio channel");
}

RageSound_QT1::~RageSound_QT1() {
    if (channel)
        SndDisposeChannel(channel, true);
    SAFE_DELETE(channel);
    SAFE_DELETE_ARRAY(buffer[0]);
    SAFE_DELETE_ARRAY(buffer[1]);
}

void RageSound_QT1::GetData(SndChannel *chan, SndCommand *cmd_passed) {
    while (!SOUNDMAN)
        SDL_Delay(10);
    LockMutex L(SOUNDMAN->lock);
    static SoundMixBuffer mix;
    RageSound_QT1 *P = reinterpret_cast<RageSound_QT1 *>(chan->userInfo);

    fill_me = cmd_passed->param2;
    UInt32 play_me = !fill_me;

    if (!P->last_pos){
        TimeRecord tr;
        ClockGetTime(P->clock, &tr);
        UInt64 temp = tr.value.hi;
        temp <<= 32;
        temp |= tr.value.lo;
        double d = static_cast<double>(temp)/tr.scale*freq;
        temp = static_cast<UInt64>(d);
        P->last_pos = static_cast<UInt32>(temp & 0x00000000FFFFFFFFLL);
        P->last_pos += samples * 3 / 2;
    } else
        P->last_pos += samples;

    bool moreThanOneSound;
    /* Swap buffers */
    header.samplePtr = reinterpret_cast<Ptr>(buffer[play_me]);
    SndCommand cmd;
    cmd.cmd = bufferCmd;
    cmd.param1 = 0;
    cmd.param2 = reinterpret_cast<long>(&header);
    OSErr err = SndDoCommand(chan, &cmd, 0);
    if (err != noErr)
        goto bail;

    /* Clear the fill buffer */
    memset(buffer[fill_me], 0, buffersize);
    moreThanOneSound = P->sounds.size() > 1;

    for (unsigned i=0; i<P->sounds.size(); ++i) {
        if (P->sounds[i]->stopping)
            continue;

        unsigned got = P->sounds[i]->snd->GetPCM(reinterpret_cast<char *>(buffer[fill_me]), buffersize, P->last_pos);
        if (moreThanOneSound)
            mix.write(reinterpret_cast<SInt16 *>(buffer[fill_me]), got / 2);
        if (got < buffersize) {
            P->sounds[i]->stopping = true;
            P->sounds[i]->flush_pos = P->last_pos + (got * 8 / samplesize);
        }
    }

    if (moreThanOneSound)
        mix.read(reinterpret_cast<SInt16 *>(buffer[fill_me]));

    cmd.cmd = callBackCmd;
    cmd.param2 = play_me;
    err = SndDoCommand(chan, &cmd, 0);
    if (err != noErr)
        goto bail;
    return;

bail:
    RageException::Throw("SndDoCommand failed with error %d", err);

}

void RageSound_QT1::StartMixing(RageSound *snd) {
    sound *s = new sound;
    s->snd = snd;

    LockMutex L(SOUNDMAN->lock);
    sounds.push_back(s);
    LOG->Trace("There are %D sounds playing", sounds.size());
}

void RageSound_QT1::StopMixing(RageSound *snd) {
    LockMutex L(SOUNDMAN->lock);

    /* Find the sound. */
    unsigned i;
    for(i = 0; i < sounds.size(); ++i)
        if(sounds[i]->snd == snd) break;
    if(i == sounds.size())
    {
        LOG->Trace("not stopping a sound because it's not playing");
        return;
    }

    delete sounds[i];
    sounds.erase(sounds.begin()+i, sounds.begin()+i+1);
    LOG->Trace("There are %D sounds playing", sounds.size());
}

void RageSound_QT1::Update(float delta) {
#pragma unused (delta)
    LockMutex L(SOUNDMAN->lock);

    vector<sound *>snds = sounds;
    for (unsigned i = 0; i < snds.size(); ++i) {
        if (!sounds[i]->stopping)
            continue;
        if (GetPosition(snds[i]->snd) < sounds[i]->flush_pos)
            continue;
        snds[i]->snd->StopPlaying();
    }
}

int RageSound_QT1::GetPosition(const RageSound *snd) const {
#pragma unused (snd)
    TimeRecord tr;
    ClockGetTime(clock, &tr);
    UInt64 temp = tr.value.hi;
    temp <<= 32;
    temp |= tr.value.lo;
    double d = static_cast<double>(temp)/tr.scale*freq;
    temp = static_cast<UInt64>(d);
    return static_cast<UInt32>(temp & 0x00000000FFFFFFFFLL);
}

float RageSound_QT1::GetPlayLatency() const {
    return latency;
}
