#include "global.h"
#include "MemoryCardDriverThreaded_OSX.h"
#include "archutils/Darwin/DarwinMCHelpers.h"
#include "Foreach.h"
#include "RageUtil.h"
#include "RageLog.h"

#include <unistd.h>
#include <Carbon/Carbon.h>

void MemoryCardDriverThreaded_OSX::Flush( UsbStorageDevice *pDevice )
{
	if( pDevice->iRefNum == -1 )
		return;
	
	ParamBlockRec pb;
	
	memset( &pb, 0, sizeof(pb) );
	pb.volumeParam.ioNamePtr = NULL;
	pb.volumeParam.ioVRefNum = pDevice->iRefNum;
	
	if( PBFlushVolSync(&pb) != noErr )
		LOG->Warn( "Failed to flush the memory card." );
}

void MemoryCardDriverThreaded_OSX::GetUSBStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	LOG->Trace( "GetUSBStorageDevices." );
	vector<CString> vDevicePaths;
	
	DarwinMCHelpers::GetRemovableDevicePaths( vDevicePaths );
	FOREACH( CString, vDevicePaths, i )
	{
		vDevicesOut.push_back( UsbStorageDevice() );
		
		const CString& path = *i;
		UsbStorageDevice& usbd = vDevicesOut.back();
		
		LOG->Trace( "Found memory card at path: %s.", path.c_str() );
		usbd.SetOsMountDir( path );
				
		// Find volume size.
		XVolumeParam param;
		Str255 name; // A pascal string.
		const CString& base = Basename(path);
		
		memset( &param, 0, sizeof(param) );
		name[0] = min( base.length(), size_t(255) );
		strncpy( (char *)&name[1], base, name[0] );
		param.ioNamePtr = name;
		param.ioVolIndex = -1; // Use ioNamePtr to find the volume.
		
		if( PBXGetVolInfoSync(&param) == noErr )
		{
			usbd.iVolumeSizeMB = param.ioVTotalBytes >> 20;
			usbd.iRefNum = param.ioVRefNum;
		}
		else
		{
			usbd.SetError( "Failed to get volume info." );
		}
	}		
}

bool MemoryCardDriverThreaded_OSX::TestWrite( UsbStorageDevice *pDevice )
{
	if( access(pDevice->sOsMountDir, W_OK) )
	{
		pDevice->SetError( "Write test failed." );
		return false;
	}
	return true;
}

/*
 * (c) 2005 Steve Checkoway
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
