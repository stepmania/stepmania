#ifndef MEMORY_CARD_DRIVER_THREADED_MACOSX_H
#define MEMORY_CARD_DRIVER_THREADED_MACOSX_H

#include "MemoryCardDriver.h"
#include "RageThreads.h"

#include <vector>


class MemoryCardDriverThreaded_MacOSX : public MemoryCardDriver
{
public:
	MemoryCardDriverThreaded_MacOSX();
	~MemoryCardDriverThreaded_MacOSX();
	bool Mount( UsbStorageDevice *pDevice ) { return true; }
	void Unmount( UsbStorageDevice *pDevice );

protected:
	bool USBStorageDevicesChanged();
	void GetUSBStorageDevices( std::vector<UsbStorageDevice>& vStorageDevicesOut );
	bool TestWrite( UsbStorageDevice *pDevice );

private:
	MemoryCardDriverThreaded_MacOSX( const MemoryCardDriverThreaded_MacOSX &m );
	MemoryCardDriverThreaded_MacOSX &operator=( const MemoryCardDriverThreaded_MacOSX &m );
	bool m_bChanged;
	RageMutex m_ChangedLock;
	class Helper;
	friend class Helper;
	Helper *m_pHelper;
};

#ifdef ARCH_MEMORY_CARD_DRIVER
#error "More than one MemoryCardDriver selected."
#endif
#define ARCH_MEMORY_CARD_DRIVER MemoryCardDriverThreaded_MacOSX


#endif

/*
 * (c) 2005-2006, 2008 Steve Checkoway
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
