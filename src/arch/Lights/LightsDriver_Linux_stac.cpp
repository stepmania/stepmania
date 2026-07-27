#include "global.h"
#include "LightsDriver_Linux_stac.h"
#include "GameState.h"
#include "Game.h"
#include "RageLog.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#include <libudev.h>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>

REGISTER_LIGHTS_DRIVER_CLASS2(stac, Linux_stac);

StacDevice::StacDevice(std::uint8_t pn)
{
    memset(outputBuffer, 0x00, sizeof(outputBuffer));

    deviceVID = STAC_VID;
    playerNumber = pn;

    switch (playerNumber)
    {
    case 1:
        devicePID = STAC_PID_P1;
        break;

    case 2:
        devicePID = STAC_PID_P2;
        break;

    default:
        LOG->Warn("STAC: Invalid player number instantiated, no PID set.");
        break;
    }
}

//function adapted from example in:
//https://web.archive.org/web/20190312223426/http://www.signal11.us/oss/udev/
//allows us to find the /dev/hidraw device that matches the Snek usb VID and PID.
void StacDevice::FindDevice()
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    const char *devVID;
    const char *devPID;

    bool found = false;

    /* Create the udev object */
    udev = udev_new();
    if (!udev)
    {
        LOG->Warn("Can't create udev");
        return;
    }

    /* Create a list of the devices in the 'hidraw' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    /* For each item enumerated, print out its information.
	   udev_list_entry_foreach is a macro which expands to
	   a loop. The loop will be executed for each member in
	   devices, setting dev_list_entry to a list entry
	   which contains the device's path in /sys. */
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;

        /* Get the filename of the /sys entry for the device
		   and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        devicePath = udev_device_get_devnode(dev);

        /* The device pointed to by dev contains information about
		   the hidraw device. In order to get information about the
		   USB device, get the parent device with the
		   subsystem/devtype pair of "usb"/"usb_device". This will
		   be several levels up the tree, but the function will find
		   it.*/
        dev = udev_device_get_parent_with_subsystem_devtype(
            dev,
            "usb",
            "usb_device");

        if (!dev)
        {
            LOG->Warn("Unable to find parent usb device.");
            return;
        }

        devVID = udev_device_get_sysattr_value(dev, "idVendor");
        devPID = udev_device_get_sysattr_value(dev, "idProduct");

        udev_device_unref(dev);

        if (strcmp(devVID, deviceVID) == 0 &&
            strcmp(devPID, devicePID) == 0)
        {
            found = true;
            break;
        }
    }

    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    udev_unref(udev);

    if (!found)
    {
        devicePath = nullptr;
    }
}

void StacDevice::Connect()
{
    devicePath = nullptr;
    fd = -1;

    FindDevice();

    if (devicePath == nullptr)
    {
        LOG->Warn("stac pn%d not found. Please ensure device is connected to system.", playerNumber);
    }
    else
    {
        fd = open(devicePath, O_RDWR | O_NONBLOCK);

        if (!IsConnected())
        {
            LOG->Warn("Could not open stac pn%d at %s. Check device permissions or udev rules. errno: %d: %s", playerNumber, devicePath, errno, strerror(errno));
        }
        else
        {
            LOG->Info("Successfully opened stac pn%d at: %s", playerNumber, devicePath);

            //ensure the device, when connected, gets a fresh state.
            newState = true;
        }
    }
}

void StacDevice::Close()
{
    if (fd >= 0)
    {
        close(fd);

        LOG->Info("Closed stac pn%d at: %s", playerNumber, devicePath);

        fd = -1;
    }
}

void StacDevice::SetInBuffer(int index, bool lightState)
{
    //the first byte is the report ID, so we offset it here to adjust.
    std::uint8_t index_offset = index + 1;

    //each index in the array represents a single light,
    //the light will turn on for any value that isn't 0x00
    std::uint8_t val = lightState ? 0xFF : 0x00;

    //ensure the index is valid and the light value has changed.
    if (index_offset < STAC_HIDREPORT_SIZE && outputBuffer[index_offset] != val)
    {
        outputBuffer[index_offset] = val;

        //signal the loop to push the new buffer to the device.
        newState = true;
    }
}

void StacDevice::PushBufferToDevice()
{
    //first check to make sure we have *new* data to send to the device.
    if (newState)
    {
        //Always ensure there is a report ID before writing,
        //otherwise linux will complain and throw errors.
        outputBuffer[0] = STAC_REPORT_ID;

        int res = write(fd, outputBuffer, STAC_HIDREPORT_SIZE);

        if (res < 0)
        {
            LOG->Warn("stac pn%d Write Error: %d: %s", playerNumber, errno, strerror(errno));

            //close on error write to signal a reconnect.
            Close();
        }

        newState = false;
    }
}

LightsDriver_Linux_stac::LightsDriver_Linux_stac()
{
    //attempt to connect on game launch
    stacPlayer1->Connect();
    stacPlayer2->Connect();

    //we keep track of if the device has *ever* been connected to signal
    //if we want to try to reconnect.
    //because if there is no P2 stac connected, then we don't want to go
    //searching for something that was *never* there.
    haveSeenP1 = stacPlayer1->IsConnected();
    haveSeenP2 = stacPlayer2->IsConnected();
}

LightsDriver_Linux_stac::~LightsDriver_Linux_stac()
{
    stacPlayer1->Close();
    stacPlayer2->Close();

    delete stacPlayer1;
    delete stacPlayer2;
}

void LightsDriver_Linux_stac::HandleState(const LightsState *ls, StacDevice *dev, GameController ctrlNum)
{
    //check to see which game we are running as it can change during gameplay.
    const InputScheme *pInput = &GAMESTATE->GetCurrentGame()->m_InputScheme;
    RString sInputName = pInput->m_szName;

    if (sInputName.EqualsNoCase("dance"))
    {
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN1, ls->m_bGameButtonLights[ctrlNum][DANCE_BUTTON_UP]);
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN2, ls->m_bGameButtonLights[ctrlNum][DANCE_BUTTON_DOWN]);
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN3, ls->m_bGameButtonLights[ctrlNum][DANCE_BUTTON_LEFT]);
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN4, ls->m_bGameButtonLights[ctrlNum][DANCE_BUTTON_RIGHT]);
    }
    else if (sInputName.EqualsNoCase("pump"))
    {
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN1, ls->m_bGameButtonLights[ctrlNum][PUMP_BUTTON_UPLEFT]);
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN2, ls->m_bGameButtonLights[ctrlNum][PUMP_BUTTON_UPRIGHT]);
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN3, ls->m_bGameButtonLights[ctrlNum][PUMP_BUTTON_CENTER]);
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN4, ls->m_bGameButtonLights[ctrlNum][PUMP_BUTTON_DOWNLEFT]);
        dev->SetInBuffer(STAC_LIGHTINDEX_BTN5, ls->m_bGameButtonLights[ctrlNum][PUMP_BUTTON_DOWNRIGHT]);
    }

    dev->PushBufferToDevice();
}

void LightsDriver_Linux_stac::Set(const LightsState *ls)
{
    if (stacPlayer1->IsConnected())
    {
        HandleState(ls, stacPlayer1, GameController_1);
    }
    else if (haveSeenP1)
    {
        //only attempt to connect again if it was connected on launch.
        stacPlayer1->Connect();
    }

    if (stacPlayer2->IsConnected())
    {
        HandleState(ls, stacPlayer2, GameController_2);
    }
    else if (haveSeenP2)
    {
        //only attempt to connect again if it was connected on launch.
        stacPlayer2->Connect();
    }
}
