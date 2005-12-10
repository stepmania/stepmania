#ifndef MemoryCardDriverThreaded_Windows_H
#define MemoryCardDriverThreaded_Windows_H

#include "MemoryCardDriver.h"
#include <windows.h>

class MemoryCardDriverThreaded_Windows: public MemoryCardDriver
{
public:
	MemoryCardDriverThreaded_Windows();
	virtual ~MemoryCardDriverThreaded_Windows();

	virtual bool Mount( UsbStorageDevice* pDevice );
	virtual void Unmount( UsbStorageDevice* pDevice );
	virtual void Flush( UsbStorageDevice* pDevice );
    virtual void Reset();

private:
	void GetUSBStorageDevices( vector<UsbStorageDevice>& vDevicesOut );
	bool USBStorageDevicesChanged();
	bool TestWrite( UsbStorageDevice* pDevice );

	DWORD m_dwLastLogicalDrives;
};

#ifdef ARCH_MEMORY_CARD_DRIVER
#error "More than one MemoryCardDriver included!"
#endif
#define ARCH_MEMORY_CARD_DRIVER MemoryCardDriverThreaded_Windows

#endif

/*
 * (c) 2003-2004 Chris Danford
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
