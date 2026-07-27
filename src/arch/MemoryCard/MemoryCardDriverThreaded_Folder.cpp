#include "global.h"
#include "MemoryCardDriverThreaded_Folder.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "PlayerNumber.h"
#include "MemoryCardManager.h"

#include <bitset>
#include <climits>
#include <cstdlib>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>


static int g_currentSerial = 0;

MemoryCardDriverThreaded_Folder::MemoryCardDriverThreaded_Folder()
{
	m_LastDevices = 0;
}

MemoryCardDriverThreaded_Folder::~MemoryCardDriverThreaded_Folder()
{
}

bool MemoryCardDriverThreaded_Folder::FolderExists(RString path)
{
	if (path.empty()) {
		return false;
	}

	const char *pathname = path.c_str();
	struct stat info;

	int statRC = stat( pathname, &info );
	if( statRC != 0 )
	{
		if (errno == ENOENT)  { return false; } // something along the path does not exist
		if (errno == ENOTDIR) { return false; } // something in path prefix is not a dir
		return false;
	}

	if( info.st_mode & S_IFDIR ) {
		return true;
	}

	return false;
}

bool MemoryCardDriverThreaded_Folder::TestWrite( UsbStorageDevice* pDevice )
{
	//TODO

	return true;
}

int MemoryCardDriverThreaded_Folder::GetActivePlayerMask()
{
	int ret = 0;

	FOREACH_PlayerNumber( p )
	{
		const RString folder = MEMCARDMAN->m_sMemoryCardOsMountPoint[p];

		if(FolderExists(folder)) {
			ret |= 1 << p;
		}
	}

	return ret;
}

bool MemoryCardDriverThreaded_Folder::USBStorageDevicesChanged()
{
	return GetActivePlayerMask() != m_LastDevices;
}

void MemoryCardDriverThreaded_Folder::GetUSBStorageDevices( std::vector<UsbStorageDevice>& vDevicesOut )
{
	LOG->Trace( "GetUSBStorageDevices" );

	vDevicesOut.clear();
	m_LastDevices = GetActivePlayerMask();

	FOREACH_PlayerNumber( p )
	{
		if((m_LastDevices & (1 << p)) > 0){
			UsbStorageDevice usbd;
			usbd.sSerial = StringConversion::ToString(g_currentSerial++);
			usbd.sSysPath = MEMCARDMAN->m_sMemoryCardOsMountPoint[p];
			usbd.sOsMountDir = MEMCARDMAN->m_sMemoryCardOsMountPoint[p];

			vDevicesOut.push_back( usbd );
		}
	}
}

bool MemoryCardDriverThreaded_Folder::Mount( UsbStorageDevice* pDevice )
{
	return true;
}

void MemoryCardDriverThreaded_Folder::Unmount( UsbStorageDevice* pDevice )
{
	return;
}

/*
 * (c) 2018-2019 Electromuis
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
