#ifndef MEMORY_CARD_DRIVER_WINDOWS_H
#define MEMORY_CARD_DRIVER_WINDOWS_H 1

#include "MemoryCardDriver.h"
#include <windows.h>

class MemoryCardDriver_Windows : public MemoryCardDriver
{
public:
	MemoryCardDriver_Windows();
	virtual ~MemoryCardDriver_Windows();
	virtual bool StorageDevicesChanged();
	virtual void GetStorageDevices( vector<UsbStorageDevice>& vStorageDevicesOut );
	virtual bool MountAndTestWrite( UsbStorageDevice* pDevice );
	virtual void Unmount( UsbStorageDevice* pDevice );
protected:
	DWORD m_dwLastLogicalDrives;
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
