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

