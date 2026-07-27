#ifndef MEMORY_CARD_DRIVER_H
#define MEMORY_CARD_DRIVER_H

#include <vector>


struct UsbStorageDevice
{
	UsbStorageDevice() = default;

	void MakeBlank() { *this = {}; };

	// -1 means "don't know"
	int iBus{-1};
	int iPort{-1};
	int iLevel{-1};
	RString sSerial{"none"}; // be different than a card with no serial
	RString sDevice{};
	RString	sOsMountDir{};	// WITHOUT trailing slash
	RString sSysPath{};   // Linux: /sys/block name
	enum State
	{
		/* Empty device.  This is used only by MemoryCardManager. */
		STATE_NONE,

		/* The card has been detected, but we haven't finished write tests, loading
		 * the quick profile information, etc. yet.  We can display something on
		 * screen, in order to appear responsive, show that something's happening and
		 * aid diagnostics, though. */
		STATE_CHECKING,

		/* We can't write to the device; it may be write-protected, use a filesystem
		 * that we don't understand, unformatted, etc. */
		STATE_ERROR,

		/* The device is ready and usable.  sName is filled in, if available. */
		STATE_READY,

		NUM_State,
		State_INVALID
	};
	State m_State{STATE_NONE};
	RString m_sError{};

	void SetError( const RString &sError ) { m_State = STATE_ERROR; m_sError = sError; }

	bool bIsNameAvailable{false};  // Name in the profile on the memory card.
	RString sName{};  // Name in the profile on the memory card.
	unsigned int idVendor{0};
	unsigned int idProduct{0};
	RString sVendor{};
	RString sProduct{};
	RString sVolumeLabel{};
	int iVolumeSizeMB{0};

	bool IsBlank() const { return m_State == STATE_NONE; }
	void SetOsMountDir( const RString &s );

	bool operator==(const UsbStorageDevice& other) const;
};

class MemoryCardDriver
{
public:
	static MemoryCardDriver *Create();

	MemoryCardDriver() {}
	virtual ~MemoryCardDriver() {}

	/* Make a device accessible via its pDevice->sOsMountDir.  This will be called
	 * before any access to the device, and before TestWrite. */
	virtual bool Mount( UsbStorageDevice* pDevice ) = 0;
	virtual void Unmount( UsbStorageDevice* pDevice ) = 0;

	/* Poll for memory card changes.  If anything has changed, fill in vStorageDevicesOut
	 * and return true. */
	bool DoOneUpdate( bool bMount, std::vector<UsbStorageDevice>& vStorageDevicesOut );

protected:
	/* This may be called before GetUSBStorageDevices; return false if the results of
	 * GetUSBStorageDevices have not changed.  (This is an optimization.) */
	virtual bool USBStorageDevicesChanged() { return true; }
	virtual void GetUSBStorageDevices( std::vector<UsbStorageDevice>& /* vDevicesOut */ ) { }

	/* Test the device.  On failure, call pDevice->SetError() appropriately, and return false. */
	virtual bool TestWrite( UsbStorageDevice* ) { return true; }

private:
	std::vector<UsbStorageDevice> m_vDevicesLastSeen;
	bool NeedUpdate( bool bMount );
};

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

