/*
 *  RageSoundDriver_CA.cpp
 *  stepmania
 *
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Steve Checkoway
 *	Charlie Reading
 *
 */

#include "global.h"
#include "RageSoundDriver_CA.h"
#include "RageSoundManager.h"
#include "RageSound.h"
#include "RageLog.h"
#include "RageThreads.h"

#include <vector>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>

#if (FEEDER == FEEDER_THREAD)
#include "VirtualRingBuffer.h"
#endif


//
//	+-------------------------------------------+-------------------------------------------+
//	|                 Packet N                  |                Packet N+1                 |
//	| +-----------------+-----------------+     | +-----------------+-----------------+     |
//	| |     Frame M     |    Frame M+1... |     | |     Frame M     |    Frame M+1... |     |
//  | | +------+------+ | +------+------+ | ... | | +------+------+ | +------+------+ | ... | ...
//	| | | Ch-A | Ch-B | | | Ch-A | Ch-B | |     | | | Ch-A | Ch-B | | | Ch-A | Ch-B | |     |
//  | | +------+------+ | +------+------+ |     | | +------+------+ | +------+------+ |     |
//	| +-----------------+-----------------+     | +-----------------+-----------------+     |
//	+-------------------------------------------+-------------------------------------------+
//


// StepMania Sound Constants
const Float64	kSMSampleRate = 44100.0;									// Number of samples per second
const UInt32	kSMChannelsPerFrame = 2;									// Number of channels (e.g. left,right) per frame
const UInt32	kSMBitsPerChannel = 16;										// Number of bits of sound data for each channel
const UInt32	kSMBytesPerFrame = (kSMChannelsPerFrame * kSMBitsPerChannel) / 8;	// Size (in bytes) of each frame
const UInt32	kSMFramesPerPacket = 1;										// Number of frames / packet (Uncompressed=1)
const UInt32	kSMBytesPerPacket = kSMBytesPerFrame * kSMFramesPerPacket;	// Size (in bytes) of each packet

const int		kOffPacketLimit = (int) (kSMSampleRate / 10);				// Correct when off at least 0.1 second
const int		kOffPacketReset = (int) (kOffPacketLimit / 100);			// Reset when within 1% of the limit


#if (FEEDER == FEEDER_THREAD)

//
// Ring Buffer/Feeder Thread Support
//

// Number of buffers in the ring
const int		kBuffersInRing = 3;

// VRB Chunk Header
struct VRBChunkHeader
{
	int sample;			// Sample-number for chunk
	size_t offset;		// Offset into the chunk for live data (normally 0)
	size_t length;		// Length of live data within the chunk
};
typedef struct VRBChunkHeader	VRBChunkHeader;

#define FEEDER_THREAD_IMPORTANCE 6
// Additional priority to use for the feeder thread, on top of this task's ordinary priority.
// This should be the lowest possible value that gives good results (no dropouts even when the machine is under load).
// The value here (6) is good on my machine (G4/450, 1 processor, OS X 10.2.1) but don't take my word for it;
// you may want to test and adjust for other machines.
// It looks like we would use 6 to get the equivalent of what iTunes uses.


#endif


#if defined(DEBUG)
static char *__errorMessage;
static char __fourcc[5];
#define TEST_ERR(result) \
CHECKPOINT; \
memcpy(__fourcc, &result, 4); \
__fourcc[4] = '\000'; \
asprintf(&__errorMessage, "err = %d, %s", result, __fourcc); \
RAGE_ASSERT_M(result == noErr, __errorMessage); \
free(__errorMessage)

#else
#define TEST_ERR(result)
#endif

#define INTERNAL_DEBUG		0
#if (INTERNAL_DEBUG)
#define LOG_MARKER			"### "
#define DEBUG_LOG			LOG->Info
#else
#define LOG_MARKER			""
#define DEBUG_LOG
#endif


// ----------------------------------------------------------------------


#if (FEEDER == FEEDER_THREAD)

int RageSound_CA::FeederThread_start(void *p)
{
	((RageSound_CA *) p)->FeederThread();
	return 0;
}

void RageSound_CA::FeederThread()
{
	/* SOUNDMAN will be set once RageSoundManager's ctor returns and
	* assigns it; we might get here before that happens, though. */
	while (!SOUNDMAN && !shutdown)
	{
		SDL_Delay(10);
	}

	while (!shutdown)
	{
		/* Sleep */
		SDL_Delay( 10 ); // int(1000 * sleep_secs));

		// Grab data (if we have some & enough room to drop a chunk in)
		char *writePointer;
		UInt32 bytesAvailableToWrite = vrb->lengthAvailableToWriteReturningPointer((void**) &writePointer);
		if ((SOUNDMAN) && (nextSampleToWrite != 0) && (bytesAvailableToWrite >= vrbChunkSize))
		{
			if (streamsInUse == 0)
			{
				nextSampleToWrite = 0;
			}
			else
			{
				// Fill our local sound buffer
				int sampleToWrite = nextSampleToWrite;
				char buffer[buffersize];
				size_t bufferUsed = this->FillSoundBuffer(buffer, buffersize, sampleToWrite);
				//DEBUG_LOG(LOG_MARKER "RageSound_CA::FeederThread -- Buffer[fill] (sampleToWrite:%d, buffersize:%ld) = bufferUsed:%ld", sampleToWrite, buffersize, bufferUsed);
	
				// Write it into the VRB
				if (bufferUsed > 0)
				{
					VRBChunkHeader *ch = (VRBChunkHeader*) writePointer;
					ch->sample = sampleToWrite;
					ch->offset = 0;
					ch->length = 0;
					writePointer += sizeof(VRBChunkHeader);

					if (bufferUsed < buffersize)
					{
						// We're out of data for the streams, so reset nextSampleToWrite
						nextSampleToWrite = 0;
					}
					else
					{
						// Full house, so move ahead
						int sample = sampleToWrite + smBytesToSamples(bufferUsed);
						nextSampleToWrite = sample;
					}
					
					UInt32 bytesWritten = vrbChunkSize-sizeof(VRBChunkHeader);
					OSStatus oss = AudioConverterConvertBuffer(converter, bufferUsed, buffer, &bytesWritten, writePointer);
					if (bytesWritten > 0)
					{
						ch->length = bytesWritten;
						vrb->didWriteLength(bytesWritten+sizeof(VRBChunkHeader));
					}
				}
			}
		}
	}
}

#endif


// ----------------------------------------------------------------------


RageSound_CA::RageSound_CA()
: idealFormat(NULL)
, actualFormat(NULL)
, converter(NULL)
{
#if (DRIVER == DRIVER_UNFINISHED)
    RageException::ThrowNonfatal("Class not finished");
#endif

	shutdown = false;

	UInt32 thePropertySize;
    OSStatus err;

	// Create 16 streams
	for (size_t i=0; i<16; i++)
	{
		stream *s = new stream();
		stream_pool.push_back(s);
	}
	streamsInUse = 0;
	
	// Grab the output-device
    thePropertySize = sizeof(AudioDeviceID);
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &thePropertySize,
                                   &outputDevice);
    TEST_ERR(err);

	// Negotiate ideal vs. actual output-stream-format
	idealFormat = new AudioStreamBasicDescription;
    idealFormat->mSampleRate = kSMSampleRate;
    idealFormat->mFormatID = kAudioFormatLinearPCM;
    idealFormat->mFormatFlags =  kLinearPCMFormatFlagIsBigEndian
		| kLinearPCMFormatFlagIsPacked
        | kLinearPCMFormatFlagIsSignedInteger;
    idealFormat->mBytesPerPacket = kSMBytesPerPacket;
    idealFormat->mFramesPerPacket = kSMFramesPerPacket;
    idealFormat->mBytesPerFrame = kSMBytesPerFrame;
    idealFormat->mChannelsPerFrame = kSMChannelsPerFrame;
    idealFormat->mBitsPerChannel = kSMBitsPerChannel;
    idealFormat->mReserved = 0;
	actualFormat = new AudioStreamBasicDescription;
    memcpy(actualFormat, idealFormat, sizeof(AudioStreamBasicDescription));
    thePropertySize = sizeof(AudioStreamBasicDescription);
    err = AudioDeviceGetProperty(outputDevice, 0, 0, kAudioDevicePropertyStreamFormatMatch, &thePropertySize, actualFormat);
    TEST_ERR(err);

    // Create a converter to get from ideal -> actual
    err = AudioConverterNew(idealFormat, actualFormat, &converter);
    TEST_ERR(err);

    // Calculate the input buffersize by maxing @ output-device's buffer and then what the audio-converter needs to get there
    thePropertySize = sizeof(UInt32);
    err = AudioDeviceGetProperty(outputDevice, 0, 0, kAudioDevicePropertyBufferSize, &thePropertySize, &buffersize);
    TEST_ERR(err);
    err = AudioConverterGetProperty(converter, kAudioConverterPropertyCalculateInputBufferSize, &thePropertySize, &buffersize);
    TEST_ERR(err);
	samplesPerBuffer = smBytesToSamples(buffersize);
	LOG->Info(LOG_MARKER "RageSound_CA::RageSound_CA -- buffersize:%ld, samplesPerBuffer:%d", buffersize, samplesPerBuffer);

    char fourcc[5];
    memcpy(fourcc, &actualFormat->mFormatID, 4);
    fourcc[4] = '\000';

    LOG->Info("\nIdeal Format:\n"
			  "Rate:                %f\n"
              "Format ID:           %s\n"
              "Format Flags:        0x%lX\n"
              "Bytes per packet:    %lu\n"
              "Frames per packet:   %lu\n"
              "Bytes per frame:     %lu\n"
              "Channels per frame:  %lu\n"
              "Bits per channel:    %lu\n",
              idealFormat->mSampleRate,
			  fourcc,
			  idealFormat->mFormatFlags,
			  idealFormat->mBytesPerPacket,
			  idealFormat->mFramesPerPacket,
			  idealFormat->mBytesPerFrame,
			  idealFormat->mChannelsPerFrame,
			  idealFormat->mBitsPerChannel);
    LOG->Info("\nActual Format:\n"
			  "Rate:                %f\n"
              "Format ID:           %s\n"
              "Format Flags:        0x%lX\n"
              "Bytes per packet:    %lu\n"
              "Frames per packet:   %lu\n"
              "Bytes per frame:     %lu\n"
              "Channels per frame:  %lu\n"
              "Bits per channel:    %lu\n",
              actualFormat->mSampleRate,
			  fourcc,
			  actualFormat->mFormatFlags,
			  actualFormat->mBytesPerPacket,
			  actualFormat->mFramesPerPacket,
			  actualFormat->mBytesPerFrame,
			  actualFormat->mChannelsPerFrame,
			  actualFormat->mBitsPerChannel);

#if (FEEDER == FEEDER_THREAD)
	// Set up the virtual-ring-buffer (if using a feeder-thread)
	vrbChunkSize = sizeof(VRBChunkHeader) + (samplesPerBuffer * actualFormat->mBytesPerPacket);	// (for converter output)
	vrb = new VirtualRingBuffer(vrbChunkSize * kBuffersInRing);
#endif

	// Set the output-device actual stream-format & start it up
    thePropertySize = sizeof(AudioStreamBasicDescription);
    err = AudioDeviceSetProperty(outputDevice, NULL, 0, 0, kAudioDevicePropertyStreamFormat,
                                 thePropertySize, &actualFormat);
    TEST_ERR(err);
    err = AudioDeviceAddIOProc(outputDevice, GetData, this);
    TEST_ERR(err);
	err = AudioDeviceAddPropertyListener(outputDevice, 0, false, kAudioDeviceProcessorOverload,
									  OverloadListener, this);
	TEST_ERR(err);
	err = AudioDeviceAddPropertyListener(outputDevice, 1, false, kAudioDeviceProcessorOverload,
									  OverloadListener, this);
	TEST_ERR(err);
    err = AudioDeviceStart(outputDevice, GetData);
    TEST_ERR(err);

    latency = 0; //for now

	startSampleTime = 0;
	expected = 0;
#if (RESYNC == RESYNC_ACCUMULATE)
	packetsOff = 0;
	packetsOffSamples = 0;
#endif

#if (FEEDER == FEEDER_THREAD)
	// Set up the feeder-thread if we're using it
	nextSampleToWrite = 0;
	feederThread.SetName("RageSound_CA");
	feederThread.Create(FeederThread_start, this);
#endif
}

RageSound_CA::~RageSound_CA()
{
	shutdown = true;

#if (FEEDER == FEEDER_THREAD)
	/* Signal the feeder thread to quit. */
	LOG->Trace("Shutting down feeder thread ...");
	feederThread.Wait();
	LOG->Trace("Feeder thread shut down.");
#endif
	
    OSStatus err;

	// Shutdown the audio device
    err = AudioDeviceStop(outputDevice, GetData);
    TEST_ERR(err);
    err = AudioDeviceRemoveIOProc(outputDevice, GetData);
    TEST_ERR(err);

	// Nuke the converter
    err = AudioConverterDispose(converter);
    TEST_ERR(err);

	delete idealFormat;
	delete actualFormat;

	// Nuke the stream pool
	for (size_t i=0; i<stream_pool.size(); i++)
	{
		delete stream_pool[i];
	}
	
#if (FEEDER == FEEDER_THREAD)
	// Nuke the virtual-ring-buffer (if using a feeder-thread)
	delete vrb;
#endif
}

void RageSound_CA::StartMixing(RageSound *snd)
{
    LockMutex L(SOUNDMAN->lock);

	// Find an unused buffer
	size_t i;
	for (i=0; i<stream_pool.size(); i++)
	{
		if (stream_pool[i]->state == stream::INACTIVE)
			break;
	}

	// No free buffer?  Fake it. (and log it)
	if (i == stream_pool.size())
	{
		LOG->Info(LOG_MARKER "RageSound_CA::StartMixing -- exceeded free buffers, faking it");
		SOUNDMAN->AddFakeSound(snd);
		return;
	}

	// Reset the internals & stash it
	stream_pool[i]->clear();
	stream_pool[i]->snd = snd;
	streamsInUse++;
	LOG->Info(LOG_MARKER "RageSound_CA::StartMixing -- [%ld] %s", i, (const char *) snd->GetLoadedFilePath());

	/* Pre-buffer the stream. */
	//stream_pool[i]->GetData(true);
	//stream_pool[i]->pcm->Play();

	/*
	 * Normally, at this point we should still be INACTIVE, in which case,
	 * tell the mixer thread to start mixing this channel.  However, if it's
	 * been changed to STOPPING, then we actually finished the whole file
	 * in the prebuffering GetData calls above, so leave it alone and let it
	 * finish on its own.
	 */
	if (stream_pool[i]->state == stream::INACTIVE)
	{
		stream_pool[i]->state = stream::PLAYING;
	}
}

void RageSound_CA::Update(float delta)
{
#pragma unused(delta)
	//DEBUG_LOG(LOG_MARKER "RageSound_CA::Update (delta:%f)", delta);

	/*
	 * SoundStopped() might erase sounds out from under us, so make a copy
	 * of the sound list.
	 */
    vector<stream *>snds = stream_pool;

	ASSERT(SOUNDMAN);
    LockMutex L(SOUNDMAN->lock);
    for (size_t i=0; i<snds.size(); i++)
	{
		// If not stopping, we don't care
        if (snds[i]->state != stream::STOPPING)
            continue;

		// Stopping, but still flushing
		int ps = this->GetPosition(snds[i]->snd);
        if (ps < snds[i]->flush_pos)
            continue;

		// Sound has stopped and flushed all its buffers
		if (snds[i]->snd != NULL)
		{
			snds[i]->snd->StopPlaying();
		}
		streamsInUse--;
		snds[i]->snd = NULL;
		snds[i]->state = stream::INACTIVE;
    }
}

void RageSound_CA::StopMixing(RageSound *snd)
{
	ASSERT(snd != NULL);
    LockMutex L(SOUNDMAN->lock);

	size_t i;
	for (i=0; i<stream_pool.size(); i++)
	{
		if (stream_pool[i]->snd == snd)
			break;
	}

	if (i == stream_pool.size())
	{
		LOG->Trace("not stopping a sound because it's not playing");
		return;
	}

	LOG->Info(LOG_MARKER "RageSound_CA::StopMixing -- [%ld] %s", i, (const char *) snd->GetLoadedFilePath());
	
	/* STOPPING tells the mixer thread to release the stream once str->flush_bufs buffers have been flushed. */
	stream_pool[i]->state = stream_pool[i]->STOPPING;

	/* Flush two buffers worth of data. */
	stream_pool[i]->flush_pos = this->GetPosition(stream_pool[i]->snd);
	
	/*
	 * This function is called externally (by RageSound) to stop immediately.
	 * We need to prevent SoundStopped from being called; it should only be
	 * called when we stop implicitly at the end of a sound.  Set snd to NULL.
	 */
	stream_pool[i]->snd = NULL;
}

int RageSound_CA::GetPosition(const RageSound *snd) const
{
    AudioTimeStamp time;
    OSStatus err = AudioDeviceGetCurrentTime(outputDevice, &time);
    TEST_ERR(err);

    return ConvertAudioTimeStampToPosition(&time);
}

float RageSound_CA::GetPlayLatency() const
{
    OSStatus err;
	UInt32 nowLatency = 0;
    UInt32 propertySize = sizeof(nowLatency);
    err = AudioDeviceGetProperty(outputDevice, 0, 0, kAudioDevicePropertyLatency, &propertySize, &nowLatency);
    TEST_ERR(err);
	return nowLatency;
}

int RageSound_CA::GetSampleRate() const
{
    AudioTimeStamp time;
    OSStatus err = AudioDeviceGetCurrentTime(outputDevice, &time);
    TEST_ERR(err);

	//return (int) (kSampleRate / time.mRateScalar);
	return (int) kSMSampleRate;
}


size_t RageSound_CA::smSamplesToBytes(int samples)
{
	return (samples * kSMBytesPerPacket);
}

int RageSound_CA::smBytesToSamples(size_t bytes)
{
	return (bytes / kSMBytesPerPacket);
}

size_t RageSound_CA::caSamplesToBytes(int samples)
{
	return (samples * actualFormat->mBytesPerPacket);
}

int RageSound_CA::caBytesToSamples(size_t bytes)
{
	return (bytes / actualFormat->mBytesPerPacket);
}


size_t RageSound_CA::FillSoundBuffer(char* buffer, size_t maxBufferSize, int position)
{
    bzero(buffer, maxBufferSize);
    static SoundMixBuffer mix;

	size_t bufferUsed = 0;
	LockMutex L(SOUNDMAN->lock);
	for (size_t i=0; i<stream_pool.size(); i++)
	{
		if (stream_pool[i]->state != stream::PLAYING)
			continue;

		bufferUsed = stream_pool[i]->snd->GetPCM(buffer, maxBufferSize, position);
		//DEBUG_LOG(LOG_MARKER "RageSound_CA::FillSoundBuffer [%lu] GetPCM(buffer, size:%lu, sampleno:%d) = %lu", i, maxBufferSize, position, bufferUsed);
		int samplesRead = smBytesToSamples(bufferUsed);
		if (bufferUsed < maxBufferSize)
		{
			stream_pool[i]->state = stream::STOPPING;
			stream_pool[i]->flush_pos = position + samplesRead;
		}

		if (streamsInUse > 1)
		{
			mix.write((SInt16 *) buffer, bufferUsed / sizeof(SInt16));
		}
	}
	L.Unlock();

	if (streamsInUse > 1)
	{
		bufferUsed = mix.size() * sizeof(SInt16);
		mix.read((SInt16 *) buffer);
	}

	return bufferUsed;
}


OSStatus RageSound_CA::GetData(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, void* inClientData)
{
    size_t outputSize = outOutputData->mBuffers[0].mDataByteSize;
    if (!SOUNDMAN)
    {
		LOG->Info(LOG_MARKER "RageSound_CA::GetData() -- SOUNDMAN doesnt yet exist!");
        bzero(outOutputData->mBuffers[0].mData, outputSize);
        return noErr;
    }
    RageSound_CA *THIS = (RageSound_CA *) inClientData;

	OSStatus oss = noErr;

	// If uninitialized, set the startSampleTime
	if (THIS->startSampleTime == 0)
	{
		THIS->startSampleTime = inOutputTime->mSampleTime;
		LOG->Info(LOG_MARKER "RageSound_CA::GetData -- startSampleTime set to %f", THIS->startSampleTime);
		LOG->Info(LOG_MARKER "RageSound_CA::GetData -- outputSize:%ld, output-samples:%d", outputSize, THIS->caBytesToSamples(outputSize));
	}
	
	// Where are we now (well, actually a little into the future)
	int nowSample = THIS->ConvertAudioTimeStampToPosition(inOutputTime);	// expected position for stream

	// Need to resync?
	if (THIS->expected == 0)
	{
		THIS->expected = nowSample;
#if (FEEDER == FEEDER_THREAD)
		DEBUG_LOG(LOG_MARKER "RageSound_CA::GetData -- expected set to %d", THIS->expected);
#endif
	}
	int posExpected = THIS->expected;
#if (RESYNC == RESYNC_NEVER)
	
	int posPlaying = posExpected;

#else

	int posPlaying = nowSample;
#if (RESYNC == RESYNC_ACCUMULATE)
//	if (abs(posPlaying-posExpected) <= kOffPacketReset)	// If we happen to be close enough, reset how far we're off
//	{
//		THIS->packetsOff = 0;
//		THIS->packetsOffSamples = 0;
//	}
//	else
//	{
		THIS->packetsOff += (posExpected - posPlaying);	// >0=ahead of actual, <0=behind actual
		THIS->packetsOffSamples++;
//	}
#else
	THIS->packetsOff = posExpected - posPlaying;	// >0=ahead of actual, <0=behind actual
	THIS->packetsOffSamples = 1;
#endif
	if (abs(THIS->packetsOff) < kOffPacketLimit)
	{
		posPlaying = posExpected;		// Since we're not correcting, do what's expected
	}
	else
	{
		LOG->Info(LOG_MARKER "RageSound_CA::GetData -- %s (expected:%d, is:%d, dist:%d, packets-off:%d, samples:%d)", ((THIS->packetsOff<0)?"skipping ahead":"skipping back"), posExpected, posPlaying, posExpected-posPlaying, THIS->packetsOff, THIS->packetsOffSamples);
		THIS->packetsOff = 0;				// Reset number of packets off since we're resetting
		THIS->packetsOffSamples = 0;
	}

#endif

	//
	// Write into our output buffer until it's full
	//

	char *outputDataPtr = (char*) outOutputData->mBuffers[0].mData;
	size_t outputDataLeft = outputSize;

#if (FEEDER == FEEDER_DIRECT)

	// Load the sound data if we have active streams
	if (THIS->streamsInUse > 0)
	{
		// Fill the sound buffer
		char buffer[THIS->buffersize];
		size_t bufferUsed = THIS->FillSoundBuffer(buffer, THIS->buffersize, posPlaying);

		// Clean-up "expected"
		THIS->expected = posPlaying + THIS->smBytesToSamples(THIS->buffersize);

		// Convert if we got anything
		if (bufferUsed != 0)
		{
			UInt32 size = outputSize;
			oss = AudioConverterConvertBuffer(THIS->converter, bufferUsed, buffer, &size, outOutputData->mBuffers[0].mData);
			outputDataLeft -= size;
			outputDataPtr += size;
		}
	}
	else
	{
		// Since we have no active streams, reset "expected" so it's fresh when we get something
		THIS->expected = 0;
	}

	// Fill any remaining data in buffer with silence
	if (outputDataLeft > 0)
	{
		bzero(outputDataPtr, outputDataLeft);
	}

#elif (FEEDER == FEEDER_THREAD)

	//
	// 1. If we have some in the ring-buffer, use that (one chunk at a time)
	//
	int samples;
	size_t bytes;
	char *vrbReadPtr = NULL;	// so-s we can do pointer arithmetic if necessary
    UInt32 bytesAvailable = THIS->vrb->lengthAvailableToReadReturningPointer((void**) &vrbReadPtr);
	int nextSampleToPlay = posPlaying;
	while ((bytesAvailable > 0) && (outputDataLeft > 0))
	{
		// Grab the chunk and skip ahead to fresh data (since we know where the header is, we don't care
		VRBChunkHeader *ch = (VRBChunkHeader*) vrbReadPtr;
		vrbReadPtr += (sizeof(VRBChunkHeader) + ch->offset);
		int vrbSample = ch->sample + THIS->caBytesToSamples(ch->offset);
		
		if (vrbSample > nextSampleToPlay)	// sample is _after_ what we're currently loading
		{
			// Fill gap before with silence
			samples = vrbSample - nextSampleToPlay;
			bytes = min(THIS->caSamplesToBytes(samples), outputDataLeft);
			DEBUG_LOG(LOG_MARKER "RageSound_CA::GetData -- VRB(%d)[fill-gap] (nextSampleToPlay:%d, vrbSample:%d, samples:%d, bytes:%ld, outputDataLeft:%ld)", nowSample, nextSampleToPlay, vrbSample, samples, bytes, outputDataLeft);
			bzero(outputDataPtr, bytes);
			outputDataPtr += bytes;
			outputDataLeft -= bytes;
			nextSampleToPlay += samples;
		}
		else if (vrbSample < nextSampleToPlay)	// sample is _behind_ what we're currently loading
		{
			// Skip forward in the chunk
			samples = nextSampleToPlay - vrbSample;
			bytes = min(THIS->caSamplesToBytes(samples), ch->length);
			DEBUG_LOG(LOG_MARKER "RageSound_CA::GetData -- VRB(%d)[skip-forward] (nextSampleToPlay:%d, vrbSample:%d, samples:%d, bytes:%ld, outputDataLeft:%ld)", nowSample, nextSampleToPlay, vrbSample, samples, bytes, outputDataLeft);
			ch->offset += bytes;
			ch->length -= bytes;
			vrbReadPtr += bytes;
			// nextSampleToPlay is already in the right place
		}

		// Fill with what's left in the chunk
		bytes = min(ch->length, outputDataLeft);
		if (bytes > 0)
		{
			DEBUG_LOG(LOG_MARKER "RageSound_CA::GetData -- VRB(%d)[fill-from-chunk] (nextSampleToPlay:%d, bytes:%ld, outputDataLeft:%ld)", nowSample, nextSampleToPlay, bytes, outputDataLeft);
			memcpy(outputDataPtr, vrbReadPtr, bytes);
			ch->offset += bytes;
			ch->length -= bytes;
			vrbReadPtr += bytes;
			outputDataPtr += bytes;
			outputDataLeft -= bytes;
			nextSampleToPlay += THIS->caBytesToSamples(bytes);
		}

		// If our chunk is empty, free it, refetch available & go at it again
		if (ch->length == 0)
		{
			THIS->vrb->didReadLength(ch->offset+sizeof(VRBChunkHeader));
			bytesAvailable = THIS->vrb->lengthAvailableToReadReturningPointer((void**) &vrbReadPtr);
		}
	}

	// If we still have room left to fill, no more in VRB, but VRB not reading, try reading directly
	// (this will also be the initial condition)
	if ((outputDataLeft > 0) && (THIS->nextSampleToWrite == 0) && (THIS->streamsInUse > 0))
	{
		// Fill a sound-buffer
		char buffer[THIS->buffersize];
		samples = THIS->caBytesToSamples(outputDataLeft);
		bytes = min(THIS->smSamplesToBytes(samples), THIS->buffersize);
		size_t bufferUsed = THIS->FillSoundBuffer(buffer, bytes, nextSampleToPlay);
		DEBUG_LOG(LOG_MARKER "RageSound_CA::GetData -- Buffer(%d)[fill] (nextSampleToPlay:%d, samples:%d, bytes:%ld) = bufferUsed:%ld", nowSample, nextSampleToPlay, samples, bytes, bufferUsed);

		// Convert
		if (bufferUsed > 0)
		{
			UInt32 size = outputDataLeft;
			oss = AudioConverterConvertBuffer(THIS->converter, bufferUsed, buffer, &size, outputDataPtr);
			DEBUG_LOG(LOG_MARKER "RageSound_CA::GetData -- Buffer(%d)[convert] (bufferUsed:%ld, outputDataLeft:%ld) = sizeConverted:%ld", nowSample, bufferUsed, outputDataLeft, size);
	
			// Clean-up
			outputDataPtr += size;
			outputDataLeft -= size;
			nextSampleToPlay += THIS->caBytesToSamples(size);
		}
	
		// Set up next-sample-to-write so VRB can take off (we _will_ be filling the rest of the buffer)
		int nextSample = nextSampleToPlay + THIS->caBytesToSamples(outputSize - outputDataLeft);
		THIS->nextSampleToWrite = nextSample;
	}
	
	// If we still have room left to fill, but have run out of data, we've gotten behind
	if (outputDataLeft > 0)
	{
        // The ring buffer has run dry.  This happens normally when we get to the end of the file's data, and
        // also often happens after the audio has been started but before the feeder thread has run yet.
        // If neither condition is the case, then the filler thread did not keep up for some reason, and we are
        // forced to play a dropout.

		// Fill it with dead air
		bytes = outputDataLeft;
		if (bytes != outputSize)
		{
			DEBUG_LOG(LOG_MARKER "RageSound_CA::GetData -- Ran-Dry(%d)[zero] (bytes:%ld, outputDataLeft:%ld)", nowSample, bytes, outputDataLeft);
		}
		bzero(outputDataPtr, bytes);
		outputDataPtr += bytes;
		outputDataLeft -= bytes;
		nextSampleToPlay += THIS->caBytesToSamples(bytes);
    }

	// Bump what's expected
	THIS->expected = nextSampleToPlay;

#else
#error "FEEDER must be set to either FEEDER_DIRECT or FEEDER_THREAD"
#endif

	return oss;
}

OSStatus RageSound_CA::OverloadListener(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, void* inClientData)
{
	LOG->Info(LOG_MARKER "RageSound_CA::OverloadListener() -- hardware overloaded (channel=%lu)", inChannel);
	return 0;
}


int RageSound_CA::ConvertSampleTimeToPosition(const Float64 sampleTime) const
{
	return (int) (sampleTime - startSampleTime);
}

int RageSound_CA::ConvertAudioTimeStampToPosition(const AudioTimeStamp *time) const
{
	return this->ConvertSampleTimeToPosition(time->mSampleTime);
}
