#include "global.h"
#include "InputHandler_Win32_Pump.h"

#include "RageLog.h"
#include "RageUtil.h"
#include "RageInputDevice.h"
#include "InputFilter.h"

//-----------------------------------------------------------------------------
// In-line Links
//-----------------------------------------------------------------------------
#pragma comment(lib, "ddk/setupapi.lib") 
#pragma comment(lib, "ddk/hid.lib") 

struct InputHandler_Win32_Pump::dev_t {
	dev_t();
	~dev_t();
	HANDLE h;
	OVERLAPPED ov;
	long buf;
	bool pending;
	int GetPadEvent();
};


namespace USB {
	char *GetUSBDevicePath (int num);
	HANDLE OpenUSB (int VID, int PID, int num);
};

extern "C" {
#include "ddk/setupapi.h"
/* Quiet header warning: */
#pragma warning( push )
#pragma warning (disable : 4201)
#include "ddk/hidsdi.h"
#pragma warning( pop )
}

char *USB::GetUSBDevicePath (int num)
{
    GUID guid;
    HidD_GetHidGuid(&guid);

    HDEVINFO DeviceInfo = SetupDiGetClassDevs (&guid,
                 NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    SP_DEVICE_INTERFACE_DATA DeviceInterface;
    DeviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    char *ret = NULL;
    PSP_INTERFACE_DEVICE_DETAIL_DATA DeviceDetail = NULL;

    if (!SetupDiEnumDeviceInterfaces (DeviceInfo,
               NULL, &guid, num, &DeviceInterface))
		goto err;

    unsigned long size;
    SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface, NULL, 0, &size, 0);

    DeviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) malloc(size);
    DeviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    if (SetupDiGetDeviceInterfaceDetail (DeviceInfo, &DeviceInterface,
		DeviceDetail, size, &size, NULL)) 
    {
        ret = strdup(DeviceDetail->DevicePath);
    }

err:
    SetupDiDestroyDeviceInfoList (DeviceInfo);
    free (DeviceDetail);
    return ret;
}

HANDLE USB::OpenUSB (int VID, int PID, int num)
{
    DWORD index = 0;

    char *path;
	HANDLE h = INVALID_HANDLE_VALUE;

    while ((path = GetUSBDevicePath (index++)) != NULL)
    {
		if(h != INVALID_HANDLE_VALUE)
			CloseHandle (h);

		h = CreateFile (path, GENERIC_READ,
			   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

		free(path);

		if(h == INVALID_HANDLE_VALUE)
			continue;

		HIDD_ATTRIBUTES attr;
		if (!HidD_GetAttributes (h, &attr))
			continue;

        if ((VID != -1 && attr.VendorID != VID) &&
            (PID != -1 && attr.ProductID != PID))
			continue; /* This isn't it. */

		/* The VID and PID match. */
		if(num-- == 0)
            return h;
    }
	if(h != INVALID_HANDLE_VALUE)
		CloseHandle (h);

    return INVALID_HANDLE_VALUE;
}


InputHandler_Win32_Pump::dev_t::dev_t()
{
	ZeroMemory( &ov, sizeof(ov) );
	pending=false;
	h = INVALID_HANDLE_VALUE;
}

InputHandler_Win32_Pump::dev_t::~dev_t()
{
	if(h != INVALID_HANDLE_VALUE)
		CloseHandle(h);
}

InputHandler_Win32_Pump::InputHandler_Win32_Pump()
{
	const int pump_usb_vid = 0x0d2f, pump_usb_pid = 0x0001;

	dev = new dev_t[NUM_PUMPS];
	
	for(int i = 0; i < NUM_PUMPS; ++i)
	{
		dev[i].h = USB::OpenUSB (pump_usb_vid, pump_usb_pid, i);
		if(dev[i].h != INVALID_HANDLE_VALUE)
			LOG->Info("Found Pump pad %i", i);
	}
}

InputHandler_Win32_Pump::~InputHandler_Win32_Pump()
{
	delete[] dev;
}

int InputHandler_Win32_Pump::dev_t::GetPadEvent()
{
    int ret;

    if(!pending)
    {
		/* Request feedback from the device. */
	    unsigned long r;
    	ret = ReadFile(h, &buf, sizeof(buf), &r, &ov);
    	pending=true;
    }

	/* See if we have a response for our request (which we may
	 * have made on a previous call): */
    if(WaitForSingleObjectEx(h, 0, TRUE) == WAIT_TIMEOUT)
		return -1;
    
	/* We do; get the result.  It'll go into the original &buf
	 * we supplied on the original call; that's why buf is a
	 * member instead of a local. */
    unsigned long cnt;
    ret = GetOverlappedResult(h, &ov, &cnt, FALSE);
    pending=false;

    if(ret == 0 && (GetLastError() == ERROR_IO_PENDING || GetLastError() == ERROR_IO_INCOMPLETE))
		return -1;

    if(ret == 0) {
		// this prints too much info in Win98
//		LOG->Warn(werr_ssprintf(GetLastError(), "Error reading Pump pad"));
	    return -1;
    }

    return buf;
}

void InputHandler_Win32_Pump::Update(float fDeltaTime)
{
	static const int bits[] = {
	/* P1 */	(1<<9), (1<<12), (1<<13), (1<<11), (1<<10),
	/* ESC */	(1<<16),
	/* P1 */	(1<<17), (1<<20), (1<<21), (1<<19), (1<<18),
	};

	for(int i = 0; i < NUM_PUMPS; ++i)
	{
		int ret = dev[i].GetPadEvent();

		if(ret == -1) 
			continue; /* no event */

		InputDevice id = InputDevice(DEVICE_PUMP1 + i);
	
		for (int butno = 0 ; butno < NUM_PUMP_PAD_BUTTONS ; butno++)
			INPUTFILTER->ButtonPressed(DeviceInput(id, butno), !(ret & bits[butno]));
	}
}

