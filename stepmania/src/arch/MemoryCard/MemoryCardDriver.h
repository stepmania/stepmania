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
		iScsiIndex = -1;
		sScsiDevice = "";
		sSerial = "";
		sOsMountDir = "";
		sName = "";
	};
	int iBus;
	int iPort;
	int iLevel;
	CString sSerial;
	int iScsiIndex;
  CString sScsiDevice;
	CString	sOsMountDir;	// WITHOUT trailing slash
	CString	sName;	// Name in the profile on the memory card.  
					// This is passed here because it's read in the mounting thread.

	bool IsBlank() { return sOsMountDir.empty(); }

	bool operator==(const UsbStorageDevice& other) const
	{
#define COMPARE(x) if( x != other.x ) return false;
		COMPARE( iBus );
		COMPARE( iPort );
		COMPARE( iLevel );
		COMPARE( sName );
		COMPARE( sOsMountDir );
		return true;
#undef COMPARE
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
