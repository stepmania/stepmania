#ifndef RAGE_SOUND_DRIVER_CA_H
#define RAGE_SOUND_DRIVER_CA_H

#include "RageSoundDriver_Generic_Software.h"

struct AudioTimeStamp;
struct AudioBufferList;
typedef unsigned long UInt32;
typedef UInt32 AudioDeviceID;
typedef long OSStatus;
class CAAudioHardwareDevice;
class RageSoundBase;

class RageSound_CA : public RageSound_Generic_Software
{
private:
    int64_t mDecodePos;
    float mLatency;
    CAAudioHardwareDevice *mOutputDevice;
    
    static OSStatus GetData(AudioDeviceID inDevice,
                            const AudioTimeStamp *inNow,
                            const AudioBufferList *inInputData,
                            const AudioTimeStamp *inInputTime,
                            AudioBufferList *outOutputData,
                            const AudioTimeStamp *inOutputTime,
                            void *inClientData);

public:
    void FillConverter(void *data, UInt32 dataByteSize);
    RageSound_CA();
    ~RageSound_CA();
    float GetPlayLatency() const { return mLatency; }
    int64_t GetPosition(const RageSoundBase *sound) const;
    void SetupDecodingThread();
};

#define RAGE_SOUND_CA

#endif
