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
#include "RageLog.h"
#include "RageUtil.h"
#include "RageLog.h"
#include <QuickTime/QuickTime.h>
#include <stdlib.h>
/* Ugh */
using namespace QT;

const unsigned channels = 2;
const unsigned samplesize = channels*16;
const unsigned samples = 512;
const unsigned buffersize = samples*samplesize/8;

/* Oh boy! dealing with memory at inturupt time. What fun! */
#pragma options align=power
static volatile Uint32 fill_me = 0;
static UInt8  *buffer[2];
static CmpSoundHeader header;

RageSound_QT::RageSound_QT() {
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

  soundOutput = OpenDefaultComponent(kSoundOutputDeviceType, NULL);
  ASSERT(soundOutput != NULL);

  TimeRecord	record;
  SoundComponentGetInfo(soundOutput, NULL, siOutputLatency, &record);
  latency = record.value.lo / record.scale;

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

RageSound_QT::~RageSound_QT() {
  if (channel)
    SndDisposeChannel(channel, true);
  SAFE_DELETE_ARRAY(buffer[0]);
  SAFE_DELETE_ARRAY(buffer[1]);
  if (soundOutput)
    CloseComponent(soundOutput);
}

void RageSound_QT::GetData(SndChannel *chan, SndCommand *cmd_passed) {
  while (!SOUNDMAN)
    SDL_Delay(10);
  LockMutex L(SOUNDMAN->lock);
  static SoundMixBuffer mix;
  RageSound_QT *P = reinterpret_cast<RageSound_QT *>(chan->userInfo);


  fill_me = cmd_passed->param2;
  UInt32 play_me = !fill_me;

  /* Swap buffers */
  header.samplePtr = reinterpret_cast<Ptr>(buffer[play_me]);
  SndCommand cmd;
  cmd.cmd = bufferCmd;
  cmd.param1 = 0;
  cmd.param2 = reinterpret_cast<long>(&header);
  SndDoCommand(chan, &cmd, 0);


  /* Clear the fill buffer */
  memset(buffer[fill_me], 0, buffersize);

  if (!P->last_pos) {
    TimeRecord tr;
    ClockGetTime(P->clock, &tr);
    P->last_pos = tr.value.lo;
  }
    
  for (unsigned i=0; i<P->sounds.size(); ++i) {
    if (P->sounds[i]->stopping)
      continue;

    unsigned got = P->sounds[i]->snd->GetPCM(reinterpret_cast<char *>(buffer[fill_me]), buffersize, P->last_pos);
    mix.write(reinterpret_cast<SInt16 *>(buffer[fill_me]), got / 2);
    if (got < buffersize) {
      P->sounds[i]->stopping = true;
      P->sounds[i]->flush_pos = P->last_pos + (got / samplesize / 8);
    }
  }

  mix.read(reinterpret_cast<SInt16 *>(buffer[fill_me]));
  P->last_pos += buffersize;

  cmd.cmd = callBackCmd;
  cmd.param2 = play_me;
  SndDoCommand(chan, &cmd, 0);
  
}

void RageSound_QT::StartMixing(RageSound *snd) {
  sound *s = new sound;
  s->snd = snd;

  LockMutex L(SOUNDMAN->lock);
  sounds.push_back(s);
}

void RageSound_QT::StopMixing(RageSound *snd) {
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
}

void RageSound_QT::Update(float delta) {
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

int RageSound_QT::GetPosition(const RageSound *snd) const {
#pragma unused (snd)
  TimeRecord tr;
  ClockGetTime(clock, &tr);
  return tr.value.lo;
}

float RageSound_QT::GetPlayLatency() const {
  return latency;
}
  

  