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
#include <cstdlib>
/* Ugh */
using namespace QT;

const unsigned channels = 2;
const unsigned samplesize = channels*16;
const unsigned samples = 2048;
const unsigned freq = 44100;
const unsigned buffersize = samples*samplesize/8;

namespace
{
    volatile Uint32 fill_me = 0;
    UInt8  *buffer[2];
    CmpSoundHeader header;
}

RageSound_QT1::RageSound_QT1()
{
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

    StartDecodeThread();

    OSErr err = SndNewChannel(&channel, sampledSynth, initStereo, callback);

    if (err != noErr)
    {
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

RageSound_QT1::~RageSound_QT1()
{
    if (channel)
        SndDisposeChannel(channel, true);
    SAFE_DELETE(channel);
    SAFE_DELETE_ARRAY(buffer[0]);
    SAFE_DELETE_ARRAY(buffer[1]);
}

void RageSound_QT1::GetData(SndChannelPtr chan, SndCommand *cmd_passed)
{
//    while (!SOUNDMAN)
//        SDL_Delay(10);
//    LockMutex L(SOUNDMAN->lock);
    RageSound_QT1 *P = reinterpret_cast<RageSound_QT1 *>(chan->userInfo);
    LOG->Trace("GetData");
    LOG->Flush();

    fill_me = cmd_passed->param2;
    UInt32 play_me = !fill_me;

    if (!P->last_pos)
    {
        P->last_pos = P->GetPosition(NULL);
        // Why the hell do I do this?
        // Maybe because there should be a hardware interrupt when the sound buffer
        // is half empty? But what does that have to do with the number of samples?
        P->last_pos += samples * 3 / 2;
    }
    else
        P->last_pos += samples;

    /* Swap buffers */
    header.samplePtr = reinterpret_cast<Ptr>(buffer[play_me]);
    SndCommand cmd;
    cmd.cmd = bufferCmd;
    cmd.param1 = 0;
    cmd.param2 = reinterpret_cast<long>(&header);
    OSErr err = SndDoCommand(chan, &cmd, false);
    if (err != noErr)
        goto bail;
    LOG->Trace("First command");
    LOG->Flush();

    /* Clear the fill buffer */
//    memset(buffer[fill_me], 0, buffersize);
//    moreThanOneSound = P->sounds.size() > 1;

    P->Mix((int16_t *)buffer[fill_me], buffersize/channels, P->last_pos, P->GetPosition(NULL));
    cmd.cmd = callBackCmd;
    cmd.param2 = play_me;
    err = SndDoCommand(chan, &cmd, false);
    if (err != noErr)
        goto bail;
    LOG->Trace("Second command");
    LOG->Flush();

    return;

bail:
    RageException::Throw("SndDoCommand failed with error %d", err);

}

int64_t RageSound_QT1::GetPosition(const RageSoundBase *snd) const
{
#pragma unused (snd)
    TimeRecord tr;
    ClockGetTime(clock, &tr);
    UInt64 temp = tr.value.hi;
    temp <<= 32;
    temp |= tr.value.lo;
    double d = static_cast<double>(temp)/tr.scale*freq;
    return static_cast<int64_t>(d);
}

float RageSound_QT1::GetPlayLatency() const
{
    return latency;
}
