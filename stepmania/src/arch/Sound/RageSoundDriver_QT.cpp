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
#include <queue>
#include <vector>
/* Ugh */
using namespace QT;

const unsigned channels = 2;
const unsigned samplesize = channels*16;
const unsigned samples = 4096;
const unsigned freq = 44100;
const unsigned buffersize = samples*samplesize/8;
const unsigned SndCommand_qLength = 2;
const unsigned initialQTSoundChannels = 1;

#pragma options align=power
struct channelInfo {
	RageSound *snd;
	bool stopping;
	int flush_pos;
	int last_pos;
	int got;
	Uint64 start_time;
	volatile Uint32 fill_me;
	SndChannelPtr channel;
	CmpSoundHeader header;
	Uint8 *buffer[2];
	channelInfo() { snd=NULL; channel=NULL; }
	void reset() { stopping=false; flush_pos=0; last_pos=0; start_time=0; fill_me=0; }
};
static queue<channelInfo *>free_channels;
static vector<channelInfo *>playing_channels;
static ComponentInstance soundClock;


RageSound_QT::RageSound_QT() {
#if 1
	RageException::ThrowNonfatal("Class not finished!");
#endif

	if (!CanPlayMultiChannels())
        RageException::ThrowNonfatal("Multiple Sound Manager channels not supported. Fallback");

	channelInfo *info;
	SndCallBackUPP callback = NewSndCallBackUPP(GetData);
	OSErr err;

	soundOutput = OpenDefaultComponent(kSoundOutputDeviceType, NULL);
	ASSERT(soundOutput != NULL);

	TimeRecord	record;
	SoundComponentGetInfo(soundOutput, NULL, siOutputLatency, &record);
	latency = record.value.lo / record.scale;

	playing_channels.reserve(initialQTSoundChannels);

	for (unsigned i=0; i<initialQTSoundChannels; ++i) {
		info = new channelInfo;

		info->buffer[0] = new Uint8[buffersize];
		info->buffer[1] = new Uint8[buffersize];
		info->channel = new SndChannel;
		info->channel->userInfo = reinterpret_cast<long>(info);
		info->channel->qLength = SndCommand_qLength;
		info->header.numChannels = channels;
		info->header.sampleSize = samplesize / channels;
		info->header.sampleRate = rate44khz;
		info->header.encode = cmpSH;

		err = SndNewChannel(&info->channel, sampledSynth, initStereo, callback);
		if (err != noErr) {
			SAFE_DELETE(info->channel);
		    RageException::ThrowNonfatal("Unable to create audio channel. THIS IS A BUG!");
		}
		free_channels.push(info);
	}

	SndCommand cmd;
	cmd.cmd = clockComponentCmd;
	cmd.param1 = true;
	cmd.param2 = 0;
	err = SndDoImmediate(info->channel, &cmd);

	cmd.cmd = getClockComponentCmd;
	cmd.param1 = 0;
	cmd.param2 = reinterpret_cast<long>(&soundClock);
	err |= SndDoImmediate(info->channel, &cmd);

	if (err != noErr)
		RageException::ThrowNonfatal("Unable to create audio channel. THIS IS A BUG!");
}

RageSound_QT::~RageSound_QT() {
	unsigned size = playing_channels.size();

	for (unsigned i=0; i<size; ++i) {
		channelInfo *info = playing_channels.back();
		
		if (info->channel)
			SndDisposeChannel(info->channel, true);
		SAFE_DELETE_ARRAY(info->buffer[0]);
		SAFE_DELETE_ARRAY(info->buffer[1]);
		playing_channels.pop_back();
	}

	while (!free_channels.empty()) {
		channelInfo *info = free_channels.front();

		if (info->channel)
			SndDisposeChannel(info->channel, true);
		SAFE_DELETE_ARRAY(info->buffer[0]);
		SAFE_DELETE_ARRAY(info->buffer[1]);
		free_channels.pop();
	}

	if (soundOutput)
		CloseComponent(soundOutput);
	if (soundClock)
		CloseComponent(soundClock);
}

void RageSound_QT::GetData(SndChannelPtr chan, SndCommand *cmd_passed) {
	LockMutex L(SOUNDMAN->lock);

	channelInfo *info = reinterpret_cast<channelInfo *>(chan->userInfo);
	ASSERT(info);
	ASSERT(chan == info->channel);
	SndCommand cmd;
	Uint32 play_me;
	OSErr err;
	int got;

	if (cmd_passed) {
		info->fill_me = cmd_passed->param2;
		play_me = !info->fill_me;
		info->header.samplePtr = reinterpret_cast<Ptr>(info->buffer[play_me]);
		info->header.numFrames = info->got;
		cmd.cmd = bufferCmd;
		cmd.param1 = 0;
		cmd.param2 = reinterpret_cast<long>(&info->header);
		LOG->Trace("bufferCmd");
		err = SndDoCommand(chan, &cmd, false);
		if (err != noErr)
			goto bail;
	}

	if (info->stopping)
		return;

	memset(info->buffer[info->fill_me], 0, buffersize);
	if (!info->last_pos) {
		TimeRecord tr;
		ClockGetTime(soundClock, &tr);
		info->start_time = tr.value.hi;
		info->start_time <<= 32;
		info->start_time |= tr.value.lo;
	}

	got = info->snd->GetPCM(reinterpret_cast<char *>(info->buffer[info->fill_me]), buffersize, info->last_pos);
	if (static_cast<unsigned>(got) < buffersize) {
		info->stopping = true;
		info->flush_pos = info->last_pos + (got * 8 / samplesize);
	}
	info->got = got * 8 / samplesize; /* got / (samplesize / 8) */
	info->last_pos += samples;

	if (cmd_passed) {
		cmd.cmd = callBackCmd;
		cmd.param2 = play_me;
		LOG->Trace("callBackCmd");
		err = SndDoCommand(chan, &cmd, false);
		if (err != noErr)
			goto bail;
	}
	return;

bail:
    RageException::Throw("SndDoCommand failed with error %d", err);
}

void RageSound_QT::StartMixing(RageSound *snd) {
	LockMutex L(SOUNDMAN->lock);
	SndCommand cmd;
	channelInfo *info;

	if (free_channels.size()) {
		info = free_channels.front();
		free_channels.pop();
	} else { /* No free audio channels, create another */
		LOG->Warn("Out of audio channels, creating channel number %D", playing_channels.size()+1);
		info = new channelInfo;

		memset(&info->header, 0, sizeof(&info->header));
		info->header.numChannels = channels;
		info->header.sampleSize = samplesize / channels;
		info->header.sampleRate = rate44khz; /* really 44.1kHz */
		info->header.numFrames = samples;
		info->header.encode = cmpSH;

		info->buffer[0] = new Uint8[buffersize];
		info->buffer[1] = new Uint8[buffersize];
		memset(info->buffer[0], 0, buffersize);
		memset(info->buffer[1], 0, buffersize);

		info->channel = new SndChannel;
		info->channel->userInfo = reinterpret_cast<long>(info);
		info->channel->qLength = SndCommand_qLength;
		OSErr err = SndNewChannel(&(info->channel), sampledSynth, initStereo, NewSndCallBackUPP(GetData));

		if (err != noErr) {
			SAFE_DELETE(info->channel);
            RageException::Throw("Unable to create audio channel. THIS IS A BUG.");
		}
	}
	
	info->reset();
	info->snd = snd;
	/* Fill the first buffer with data */
	playing_channels.push_back(info);
	GetData(info->channel, NULL);
	cmd.cmd = callBackCmd;
	cmd.param1 = 0;
	cmd.param2 = !info->fill_me;
	OSErr err = SndDoCommand(info->channel, &cmd, 0); /* command queue is empty */
	if (err != noErr)
        RageException::Throw("Couldn't start the callback");
}

void RageSound_QT::StopMixing(RageSound *snd) {
	LockMutex L(SOUNDMAN->lock);
	LOG->Trace("StopMixing(). Stopping sound 0x%X", reinterpret_cast<long>(snd));
	/* Find the sound. */
	int size = playing_channels.size();
	int i;
	channelInfo *info;

	for (i=0; i<=size; ++i)
		if ((info=playing_channels[i])->snd == snd)
			break;
	if (i==size) {
		LOG->Trace("Not stopping a sound because it's not playing.");
		return;
	}

	playing_channels.erase(playing_channels.begin()+i);
	SndCommand cmd;
	cmd.cmd = flushCmd;
	cmd.param1 = 0;
	cmd.param2 = 0;
	LOG->Trace("Flushing channel with song %X", reinterpret_cast<long>(snd));
	SndDoImmediate(info->channel, &cmd);
	cmd.cmd = quietCmd;
	SndDoImmediate(info->channel, &cmd);
	free_channels.push(info);
}

void RageSound_QT::Update(float delta) {
#pragma unused (delta)
	LockMutex L(SOUNDMAN->lock);

	vector<channelInfo *> playing = playing_channels;
	int size = playing.size();
	for (int i=0; i<size; ++i) {
		channelInfo *info = playing[i];
		if (info->stopping != 0)
			continue;
		if (GetPosition(info->snd) < info->flush_pos)
			continue;
		info->snd->StopPlaying();
	}
}

int RageSound_QT::GetPosition(const RageSound *snd) const {
	LockMutex L(SOUNDMAN->lock);
	TimeRecord tr;
	ClockGetTime(soundClock, &tr);
	UInt64 time = tr.value.hi;
	time <<= 32;
	time |= tr.value.lo;

	/* Find the info */
	int size = playing_channels.size();
	int i;
	channelInfo *info;
	for (i=0; i<size; ++i)
		if ((info = playing_channels[i])->snd == snd)
			break;
	if (i == size)
        RageException::Throw("Can't get the position of a sound that isn't playing");

	LOG->Trace("GetPosition() sound 0x%X", reinterpret_cast<long>(snd));

	time -= info->start_time; /* Now time contains the total time the sound was played. */
	time = time * freq / tr.scale; /* Now time contains the number of samples played. */
	return static_cast<int>(time % 0x00000000FFFFFFFFLL);
}

/* Pascal code from SoundManager docs.

FUNCTION MyCanPlayMultiChannels: Boolean;
VAR
myResponse:    LongInt;
myResult:      Boolean;
myErr:         OSErr;
myVersion:     NumVersion;
BEGIN
myResult := FALSE;
myVersion := SndSoundManagerVersion;
myErr := Gestalt(gestaltSoundAttr, myResponse);
IF myVersion.majorRev >= 3 THEN
IF (myErr = noErr) AND (BTst(myResponse, gestaltMultiChannels)) THEN
myResult := TRUE
ELSE
BEGIN
myErr := Gestalt(gestaltHardwareAttr, myResponse);
IF (myErr = noErr) AND (BTst(myResponse, gestaltHasASC)) THEN
myResult := TRUE
END;
MyCanPlayMultiChannels := myResult;
END;
*/

bool RageSound_QT::CanPlayMultiChannels(){
	register long response;
	register OSErr err;
	NumVersion version;
	register bool result;

	result = false;
	version = SndSoundManagerVersion();
	err = Gestalt(gestaltSoundAttr, &response);
	if (version.majorRev >= 3){
		if((err == noErr) && (response & gestaltMultiChannels == gestaltMultiChannels))
			result = true;
	} else {
		err = Gestalt(gestaltHardwareAttr, &response);
		if ((err == noErr) && (response & gestaltHasASC == gestaltHasASC))
			result = true;
	}
	return result;
}
