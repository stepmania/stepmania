#include "global.h"
#include "RageSoundDriver_CA.h"
#include "CAHelpers.h"

const UInt32 kFramesPerPacket = 1;
const UInt32 kChannelsPerFrame = 2;
const UInt32 kBitsPerChannel = 16;
const UInt32 kBytesPerPacket = kChannelsPerFrame * kBitsPerChannel / 8;
const UInt32 kBytesPerFrame = kBytesPerPacket;
const UInt32 kFormatFlags = kAudioFormatFlagsNativeEndian |
                            kAudioFormatFlagIsSignedInteger;

AudioConverter::AudioConverter( RageSound_CA *driver, const Desc &procFormat )
    : mBuffer(NULL), mBufferSize(0)
{
    const Desc SMFormat(44100.0, kAudioFormatLinearPCM, kBytesPerPacket,
                        kFramesPerPacket, kBytesPerFrame, kChannelsPerFrame,
                        kBitsPerChannel, kFormatFlags);
    
    SMFormat.Print();
    procFormat.Print();

    if( this->Initialize(SMFormat, procFormat) )
        RageException::ThrowNonfatal( "Couldn't create the converter." );
    
    mDriver = driver;
}

OSStatus AudioConverter::FormatConverterInputProc(UInt32& ioNumberDataPackets,
                                                  AudioBufferList& ioData,
                                                  AudioStreamPacketDescription **outDesc)
{
    AudioBuffer& buf = ioData.mBuffers[0];
    
    // This really shouldn't happen more than once, but better be sure.
    if( mBufferSize != buf.mDataByteSize )
    {
        delete mBuffer;
        mBufferSize = buf.mDataByteSize;
        mBuffer = new UInt8[mBufferSize];
    }
    
    mDriver->FillConverter(mBuffer, mBufferSize);
    buf.mData = mBuffer;
    return noErr;
}

/*
 * (c) 2004 Steve Checkoway
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
