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
		bNeedsWriteTest = true;
		bWriteTestSucceeded = false;
		sName = "";
	};
	int iBus;
	int iPort;
	int iLevel;
	CString sSerial;
	int iScsiIndex;
	CString sScsiDevice;
	CString	sOsMountDir;	// WITHOUT trailing slash
  bool bNeedsWriteTest;
  bool bWriteTestSucceeded;  // only valid if bNeedsWriteTest == false
  CString sName;  // Name in the profile on the memory card.

	bool IsBlank() const { return sOsMountDir.empty(); }
	void SetOsMountDir( const CString &s );

  bool operator==(const UsbStorageDevice& other) const;
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
	enum MountThreadState 
	{
		detect_and_mount,
		detect_and_dont_mount,
		paused
	};
	virtual void SetMountThreadState( MountThreadState mts ) = 0;
};

MemoryCardDriver *MakeMemoryCardDriver();

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

