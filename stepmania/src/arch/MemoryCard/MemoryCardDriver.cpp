#include "global.h"
#include "MemoryCardDriver.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "Foreach.h"
#include "ProfileManager.h"

#include "Selector_MemoryCardDriver.h"

static const CString TEMP_MOUNT_POINT = "/@mctemptimeout/";

bool UsbStorageDevice::operator==(const UsbStorageDevice& other) const
{
  //  LOG->Trace( "Comparing %d %d %d %s %s to %d %d %d %s %s",
  //	      iBus, iPort, iLevel, sName.c_str(), sOsMountDir.c_str(),
  //	      other.iBus, other.iPort, other.iLevel, other.sName.c_str(), other.sOsMountDir.c_str() );
#define COMPARE(x) if( x != other.x ) return false
  COMPARE( iBus );
  COMPARE( iPort );
  COMPARE( iLevel );
  COMPARE( sOsMountDir );
  return true;
#undef COMPARE
}

void UsbStorageDevice::SetOsMountDir( const CString &s )
{
	sOsMountDir = s;
}

bool MemoryCardDriver::NeedUpdate( bool bMount )
{
	if( bMount )
	{
		/* Check if any devices need a write test. */
		for( unsigned i=0; i<m_vDevicesLastSeen.size(); i++ )
		{
			const UsbStorageDevice &d = m_vDevicesLastSeen[i];
			if( d.m_State == UsbStorageDevice::STATE_CHECKING )
				return true;
		}
	}

	return USBStorageDevicesChanged();
}

bool MemoryCardDriver::DoOneUpdate( bool bMount, vector<UsbStorageDevice>& vStorageDevicesOut )
{
	if( !NeedUpdate(bMount) )
		return false;

	vector<UsbStorageDevice> vOld = m_vDevicesLastSeen; // copy
	GetUSBStorageDevices( vStorageDevicesOut );

	// log connects
	FOREACH( UsbStorageDevice, vStorageDevicesOut, newd )
	{
		vector<UsbStorageDevice>::iterator iter = find( vOld.begin(), vOld.end(), *newd );
		if( iter == vOld.end() )    // didn't find
			LOG->Trace( "New device connected: %s", newd->sDevice.c_str() );
	}

	/* When we first see a device, regardless of bMount, just return it as CHECKING,
	 * so the main thread knows about the device.  On the next call where bMount is
	 * true, check it. */
	for( unsigned i=0; i<vStorageDevicesOut.size(); i++ )
	{
		UsbStorageDevice &d = vStorageDevicesOut[i];

		/* If this device was just connected (it wasn't here last time), set it to
		 * CHECKING and return it, to let the main thread know about the device before
		 * we start checking. */
		vector<UsbStorageDevice>::iterator iter = find( vOld.begin(), vOld.end(), d );
		if( iter == vOld.end() )    // didn't find
		{
			LOG->Trace( "New device entering CHECKING: %s", d.sDevice.c_str() );
			d.m_State = UsbStorageDevice::STATE_CHECKING;
			continue;
		}

		/* Preserve the state of the device, and any data loaded from previous checks. */
		d.m_State = iter->m_State;
		d.bIsNameAvailable = iter->bIsNameAvailable;
		d.sName = iter->sName;

		/* The device was here last time.  If CHECKING, check the device now, if
		 * we're allowed to. */
		if( d.m_State == UsbStorageDevice::STATE_CHECKING )
		{
			if( !bMount )
			{
				/* We can't check it now.  Keep STATE_CHECKING, and check it when we can. */
				d.m_State = UsbStorageDevice::STATE_CHECKING;
				continue;
			}

			if( !this->Mount(&d) )
			{
				d.SetError( "MountFailed" );
				continue;
			}

			if( TestWrite(&d) )
			{
				/* We've successfully mounted and tested the device.  Read the
				 * profile name (by mounting a temporary, private mountpoint),
				 * and then unmount it until Mount() is called. */
				d.m_State = UsbStorageDevice::STATE_READY;
			
				FILEMAN->Mount( "dir", d.sOsMountDir, TEMP_MOUNT_POINT );
				d.bIsNameAvailable = PROFILEMAN->FastLoadProfileNameFromMemoryCard( TEMP_MOUNT_POINT, d.sName );
				FILEMAN->Unmount( "dir", d.sOsMountDir, TEMP_MOUNT_POINT );
			}

			this->Unmount( &d );

			LOG->Trace( "WriteTest: %s, Name: %s", d.m_State == UsbStorageDevice::STATE_ERROR? "failed":"succeeded", d.sName.c_str() );
		}
	}

	m_vDevicesLastSeen = vStorageDevicesOut;

	return true;
}

/*
 * (c) 2002-2004 Glenn Maynard
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
