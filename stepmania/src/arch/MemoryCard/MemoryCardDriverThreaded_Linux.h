#ifndef MemoryCardDriverThreaded_Linux_H
#define MemoryCardDriverThreaded_Linux_H 1

#include "MemoryCardDriverThreaded.h"

class MemoryCardDriverThreaded_Linux : public MemoryCardDriverThreaded
{
public:
	MemoryCardDriverThreaded_Linux();

	virtual void Unmount( UsbStorageDevice* pDevice, CString sMountPoint );
	virtual void Flush( UsbStorageDevice* pDevice );
        virtual void ResetUsbStorage();
protected:
	virtual void MountThreadMain();
	virtual void Mount( UsbStorageDevice* pDevice, CString sMountPoint );

	bool m_bReset;
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
