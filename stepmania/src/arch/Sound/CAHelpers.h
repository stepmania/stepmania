#ifndef CA_HELPERS_H
#define CA_HELPERS_H

#include <vector>
#include "FormatConverterClient.h"
#include "CAStreamBasicDescription.h"

const UInt32 kFramesPerPacket = 1;
const UInt32 kChannelsPerFrame = 2;
const UInt32 kBitsPerChannel = 16;
const UInt32 kBytesPerPacket = kChannelsPerFrame * kBitsPerChannel / 8;
const UInt32 kBytesPerFrame = kBytesPerPacket;
const UInt32 kFormatFlags = kAudioFormatFlagsNativeEndian |
                            kAudioFormatFlagIsSignedInteger;
typedef CAStreamBasicDescription Desc;


class AudioConverter : public FormatConverterClient
{
public:
    AudioConverter(CAAudioHardwareDevice *dev, RageSound_CA *driver);
    ~AudioConverter() { delete mBuffer; }
protected:
    OSStatus FormatConverterInputProc(UInt32& ioNumberDataPackets,
                                      AudioBufferList& ioData,
                                      AudioStreamPacketDescription **outDesc);
private:
    static Desc FindClosestFormat(const vector<Desc>& formats);
    
    RageSound_CA *mDriver;
    UInt8 *mBuffer;
    UInt32 mBufferSize;
};


#endif
