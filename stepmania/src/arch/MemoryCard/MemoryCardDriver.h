#ifndef MEMORY_CARD_ENUMERATOR_H
#define MEMORY_CARD_ENUMERATOR_H 1

struct UsbStorageDevice
{
	UsbStorageDevice() { MakeBlank(); }

	void MakeBlank()
	{
	  // -1 means "don't know"
		iBus = -1;
		iPort = -1;
		iLevel = -1;
		iUsbStorageIndex = -1;
		sSerial = "";
		sOsMountDir = "";
	};
	int iBus;
	int iPort;
	int iLevel;
	CString sSerial;
	int iUsbStorageIndex;
	CString	sOsMountDir;	// WITHOUT trailing slash

	bool IsBlank() { return sOsMountDir.empty(); }

	bool operator==(const UsbStorageDevice& other) const
	{
	  if( (iBus!=-1 || other.iBus!=-1) && iBus != other.iBus )
	      return false;
	  if( (iPort!=-1 || other.iPort!=-1) && iPort != other.iPort )
	      return false;
	  if( (iLevel!=-1 || other.iLevel!=-1) && iLevel != other.iLevel )
	      return false;
	  return sOsMountDir==other.sOsMountDir;  // every time a device is plugged in, it gets a unique device number
	}
  bool operator!=(const UsbStorageDevice& other) const
  {
    return !operator==(other);
  }
};

class MemoryCardDriver
{
public:
	MemoryCardDriver() {};
	virtual ~MemoryCardDriver() {};
	virtual bool StorageDevicesChanged() = 0;
	virtual void GetStorageDevices( vector<UsbStorageDevice>& vStorageDevicesOut ) = 0;
	virtual bool MountAndTestWrite( UsbStorageDevice* pDevice, CString sMountPoint ) = 0;	// return false if mount or write fails
	virtual void Unmount( UsbStorageDevice* pDevice, CString sMountPoint ) = 0;
	virtual void Flush( UsbStorageDevice* pDevice ) = 0;
	virtual void ResetUsbStorage() = 0;
};

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *	Chris Danford
 */
