#ifndef MemoryCardDriverThreaded_H
#define MemoryCardDriverThreaded_H 1
/*
-----------------------------------------------------------------------------
 Class: MemoryCardDriverThreaded

 Desc: Adapter .

 Copyright (c) 2001-2004 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MemoryCardDriver.h"
#include "RageThreads.h"

struct UsbStorageDeviceEx : public UsbStorageDevice
{
	UsbStorageDeviceEx() { MakeBlank(); }

	void MakeBlank()
	{
		bWriteTestSucceeded = false;
	}
	
	bool bWriteTestSucceeded;
};

class MemoryCardDriverThreaded : public MemoryCardDriver
{
public:
	MemoryCardDriverThreaded();
	~MemoryCardDriverThreaded();
	void StartThread();	// call this in the derived constructor to start the mounting thread

	virtual bool StorageDevicesChanged();
	virtual void GetStorageDevices( vector<UsbStorageDevice>& vStorageDevicesOut );
	virtual bool MountAndTestWrite( UsbStorageDevice* pDevice, CString sMountPoint );
	static int MountThread_Start( void *p );
protected:
	virtual void MountThreadMain() = 0;
	virtual void Mount( UsbStorageDevice* pDevice, CString sMountPoint ) = 0;

	RageThread m_threadMemoryCardMount;
	bool m_bShutdown;

	vector<UsbStorageDeviceEx> m_vStorageDevices;
	bool m_bStorageDevicesChanged;
	RageMutex m_mutexStorageDevices;	// protects the above two
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
