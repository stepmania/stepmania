#ifndef MEMORY_CARD_ENUMERATOR_NULL_H
#define MEMORY_CARD_ENUMERATOR_NULL_H 1

#include "MemoryCardDriver.h"

class MemoryCardDriver_Null : public MemoryCardDriver
{
public:
	MemoryCardDriver_Null() {};
	virtual bool StorageDevicesChanged() { return false; };
	virtual void GetStorageDevices( vector<UsbStorageDevice>& vStorageDevicesOut ) {};
	virtual bool MountAndTestWrite( UsbStorageDevice* pDevice, CString sMountPoint ) { return false; };
	virtual void Unmount( UsbStorageDevice* pDevice, CString sMountPoint ) {};
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
