#ifndef WIN32_USB_H
#define WIN32_USB_H

class USBDevice
{
public:
	USBDevice();
	~USBDevice();
	int GetPadEvent();
	bool Open(int VID, int PID, int num);
	bool IsOpen();

private:
	HANDLE h;
	OVERLAPPED ov;
	long buf;
	bool pending;
};

#endif
/*
-----------------------------------------------------------------------------
 Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
	Glenn Maynard
-----------------------------------------------------------------------------
*/
