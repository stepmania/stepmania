#ifndef MEMORY_CARD_ENUMERATOR_H
#define MEMORY_CARD_ENUMERATOR_H 1


struct UsbStorageDevice
{
	UsbStorageDevice()
	{
		iBus = -1;
		iDeviceOnBus = -1;
		iPortOnHub = -1;
		iUsbStorageIndex = -1;
	};
	int iBus;
	int iDeviceOnBus;
	int iPortOnHub;
	CString sSerial;
	int iUsbStorageIndex;
	CString	sOsMountDir;	// WITHOUT trailing slash

	bool operator==(const UsbStorageDevice& other)
	{
		return 
			iBus==other.iBus &&
		  iDeviceOnBus==other.iDeviceOnBus;  // every time a device is plugged in, it gets a unique device number
	}
};

class MemoryCardDriver
{
public:
	MemoryCardDriver() {};
	virtual ~MemoryCardDriver() {};
	virtual bool StorageDevicesChanged() = 0;
	virtual void GetStorageDevices( vector<UsbStorageDevice>& vStorageDevicesOut ) = 0;
	virtual bool MountAndTestWrite( UsbStorageDevice* pDevice ) = 0;	// return false if mount or write fails
	virtual void Unmount( UsbStorageDevice* pDevice ) = 0;
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
