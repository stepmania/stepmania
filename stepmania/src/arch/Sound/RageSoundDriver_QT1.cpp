/*
 *  RageSoundDriver_QT1.cpp
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
    volatile uint32_t fill_me = 0;
    uint8_t  *buffer[2];
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

    buffer[0] = new uint8_t[buffersize];
    buffer[1] = new uint8_t[buffersize];
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
	
	TimeRecord tr;
		
	ClockGetTime(clock, &tr);
	unitsPerSample = double(tr.scale) / freq;

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
    RageSound_QT1 *P = reinterpret_cast<RageSound_QT1 *>(chan->userInfo);

    fill_me = cmd_passed->param2;
    uint32_t play_me = !fill_me;

    if (!P->last_pos)
        P->last_pos = P->GetPosition(NULL);
    else
        P->last_pos += static_cast<int64_t>(samples*P->unitsPerSample);

    /* Swap buffers */
    header.samplePtr = reinterpret_cast<Ptr>(buffer[play_me]);
    SndCommand cmd;
    cmd.cmd = bufferCmd;
    cmd.param1 = 0;
    cmd.param2 = reinterpret_cast<long>(&header);
    OSErr err = SndDoCommand(chan, &cmd, false);
    if (err != noErr)
        goto bail;

    P->Mix((int16_t *)buffer[fill_me], samples, P->last_pos, P->GetPosition(NULL));
    cmd.cmd = callBackCmd;
    cmd.param2 = play_me;
    err = SndDoCommand(chan, &cmd, false);
    if (err != noErr)
        goto bail;

    return;

bail:
    RageException::Throw("SndDoCommand failed with error %d", err);

}

int64_t RageSound_QT1::GetPosition(const RageSoundBase *snd) const
{
#pragma unused (snd)
    TimeRecord tr;
    ClockGetTime(clock, &tr);
    int64_t temp = tr.value.hi;
	
    temp <<= 32;
    temp |= tr.value.lo;
    return temp;
}

/*
 * (c) 2003-2004 Steve Checkoway
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
