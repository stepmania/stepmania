#ifndef CA_HELPERS_H
#define CA_HELPERS_H

#include "FormatConverterClient.h"
#include "CAStreamBasicDescription.h"

typedef CAStreamBasicDescription Desc;


class AudioConverter : public FormatConverterClient
{
public:
    AudioConverter( RageSound_CA *driver, const Desc &procFormat );
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
