#ifndef MEMORY_CARD_DRIVER_LINUX_H
#define MEMORY_CARD_DRIVER_LINUX_H 1

#include "MemoryCardDriver.h"

class MemoryCardDriver_Linux : public MemoryCardDriver
{
public:
	MemoryCardDriver_Linux();
	virtual ~MemoryCardDriver_Linux();
	virtual bool StorageDevicesChanged();
	virtual void GetStorageDevices( vector<UsbStorageDevice>& vStorageDevicesOut );
	virtual bool MountAndTestWrite( UsbStorageDevice* pDevice );
	virtual void Flush( UsbStorageDevice* pDevice );
protected:
	int m_fds;
	time_t m_lastModTime;
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
