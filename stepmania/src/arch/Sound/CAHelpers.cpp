#include "global.h"
#include "RageLog.h"
#include "RageSoundDriver_CA.h"
#include "CAHelpers.h"

#include "CAAudioHardwareDevice.h"
#include "CAAudioHardwareStream.h"
#include "CAException.h"

AudioConverter::AudioConverter(CAAudioHardwareDevice *dev, RageSound_CA *driver)
    : mBuffer(NULL), mBufferSize(0)
{
    AudioStreamID sID = dev->GetStreamByIndex(kAudioDeviceSectionOutput, 0);
    CAAudioHardwareStream stream(sID);
    vector<Desc> procFormats;
    vector<Desc> physicalFormats;
    UInt32 numFormats = stream.GetNumberAvailableIOProcFormats();
    
    LOG->Trace("%u IOProc formats.", unsigned(numFormats));
    for (UInt32 i=0; i<numFormats; ++i)
    {
        Desc desc;
        
        stream.GetAvailableIOProcFormatByIndex(i, desc);
        procFormats.push_back(desc);
    }
    
    const Desc& procFormat = FindClosestFormat(procFormats);
    stream.SetCurrentIOProcFormat(procFormat);
    
    numFormats = stream.GetNumberAvailablePhysicalFormats();
    LOG->Trace("%u physical formats.", unsigned(numFormats));
    for (UInt32 i=0; i<numFormats; ++i)
    {
        Desc desc;
        
        stream.GetAvailablePhysicalFormatByIndex(i, desc);
        physicalFormats.push_back(desc);
    }
    
    const Desc& physicalFormat = FindClosestFormat(physicalFormats);
    stream.SetCurrentPhysicalFormat(physicalFormat);
    
    const Desc SMFormat(44100.0, kAudioFormatLinearPCM, kBytesPerPacket,
                        kFramesPerPacket, kBytesPerFrame, kChannelsPerFrame,
                        kBitsPerChannel, kFormatFlags);
    
    LOG->Trace("Initializing converter.");
    SMFormat.Print();
    procFormat.Print();
    if (this->Initialize(SMFormat, procFormat))
        RageException::ThrowNonfatal("Couldn't create the converter.");
    
    mDriver = driver;
}

OSStatus AudioConverter::FormatConverterInputProc(UInt32& ioNumberDataPackets,
                                                  AudioBufferList& ioData,
                                                  AudioStreamPacketDescription **outDesc)
{
    AudioBuffer& buf = ioData.mBuffers[0];
    
    // This really shouldn't happen more than once, but better be sure.
    if (mBufferSize != buf.mDataByteSize)
    {
        delete mBuffer; // deleting NULL does not crash, unlike free(NULL)
        mBufferSize = buf.mDataByteSize;
        mBuffer = new UInt8[mBufferSize];
    }
    
    mDriver->FillConverter(mBuffer, mBufferSize);
    buf.mData = mBuffer;
    return noErr;
}

Desc AudioConverter::FindClosestFormat(const vector<Desc>& formats)
{
    LOG->Trace("FindClosestFormat");
    vector<Desc> v;
    
    vector<Desc>::const_iterator i;
    for (i = formats.begin(); i != formats.end(); ++i)
    {
        const Desc& format = *i;
        
        if (!format.IsPCM() || format.mSampleRate != 44100.0)
            continue;
        
        if (format.SampleWordSize() == 2 &&
            (format.mFormatFlags & kAudioFormatFlagIsSignedInteger) ==
            kAudioFormatFlagIsSignedInteger)
        { // exact match
            LOG->Trace("Exact match.");
            return format;
        }
        v.push_back(format);
    }
    
    LOG->Trace("Failed exact match.");
    for (i = v.begin(); i != v.end(); ++i)
    {
        const Desc& format = *i;
        if (format.SampleWordSize() == 2)
        {
            LOG->Trace("Close match.");
            return format; // close
        }
    }
    LOG->Trace("Failed close match.");
    if (v.empty())
        RageException::ThrowNonfatal("Couldn't find a close format.");
    return v[0]; // something is better than nothing.
}
