/* LightsDriver_Linux_stac: Control lights for the stac by icedragon.io via direct hidraw writes. */

#ifndef LightsDriver_Linux_stac_H
#define LightsDriver_Linux_stac_H

/*
 * -------------------------- NOTE --------------------------
 *
 * This driver needs user read/write access to the icedragon.io stac.
 * This can be achieved by using a udev rule like this:
 *
 * (player 1 then player 2)
 * SUBSYSTEMS=="usb", ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="eb5b", OWNER="dance", GROUP="dance", MODE="0660"
 * SUBSYSTEMS=="usb", ATTRS{idVendor}=="04d8", ATTRS{idProduct}=="eb5a", OWNER="dance", GROUP="dance", MODE="0660"
 *
 * Refer to your distrobution's documentation on how to properly apply a udev rule.
 *
 * -------------------------- NOTE --------------------------
 */

#include "arch/Lights/LightsDriver.h"

#include <cstdint>

//static information about the device(s) in question.
#define STAC_VID "04d8"
#define STAC_PID_P1 "ea4b"
#define STAC_PID_P2 "ea4a"
#define STAC_NUMOF_LIGHTS 5

//the first byte of the buffer is a static report id.
#define STAC_HIDREPORT_SIZE (STAC_NUMOF_LIGHTS + 1)
#define STAC_REPORT_ID 0x01

//all indicies contain their respective 573 pinouts
enum StacLightIndex
{
    STAC_LIGHTINDEX_BTN1 = 0,
    STAC_LIGHTINDEX_BTN2 = 1,
    STAC_LIGHTINDEX_BTN3 = 2,
    STAC_LIGHTINDEX_BTN4 = 3,
    STAC_LIGHTINDEX_BTN5 = 4,
    STAC_LIGHTINDEX_MAX
};

class StacDevice
{
public:
    const char *deviceVID;
    const char *devicePID;

    const char *devicePath;
    int fd = -1;

    std::uint8_t playerNumber = 0;
    bool newState = false;

    std::uint8_t outputBuffer[STAC_HIDREPORT_SIZE];

    StacDevice(std::uint8_t pn);

    void FindDevice();
    void Connect();
    void Close();

    bool IsConnected()
    {
        //a zero/positive file descriptor is a valid one.
        //and if we have opened it, then we are connected.
        //if there is an error, then we close it.
        return (fd >= 0);
    }

    void SetInBuffer(int index, bool lightState);

    void PushBufferToDevice();
};

class LightsDriver_Linux_stac : public LightsDriver
{
private:
    StacDevice *stacPlayer1 = new StacDevice(1);
    StacDevice *stacPlayer2 = new StacDevice(2);

    bool haveSeenP1 = false;
    bool haveSeenP2 = false;

    void HandleState(const LightsState *ls, StacDevice *dev, GameController ctrlNum);

public:
    LightsDriver_Linux_stac();
    virtual ~LightsDriver_Linux_stac();

    virtual void Set(const LightsState *ls);
};

#endif

/*
 * (c) 2021 StepMania team
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
