#include "global.h"
#include "MemoryCardDriverThreaded.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFileManager.h"


int MemoryCardDriverThreaded::MountThread_Start( void *p )
{
	((MemoryCardDriverThreaded*)p)->MountThreadMain();
	return 0;
}

template<class T>
bool VectorsAreEqual( const T &a, const T &b )
{
	if( a.size() != b.size() )
		return false;
	
	for( unsigned i=0; i<a.size(); i++ )
    {
		if( a[i] != b[i] )
			return false;
    }
	
	return true;
}

MemoryCardDriverThreaded::MemoryCardDriverThreaded() :
m_mutexStorageDevices("StorageDevices")
{
	m_bShutdown = false;
}

void MemoryCardDriverThreaded::StartThread()
{
	m_threadMemoryCardMount.SetName("MemoryCardMount");
	m_threadMemoryCardMount.Create( MountThread_Start, this );
}

MemoryCardDriverThreaded::~MemoryCardDriverThreaded()
{
	m_bShutdown = true;
	LOG->Trace( "Shutting down Mount thread..." );
	m_threadMemoryCardMount.Wait();
	LOG->Trace( "Mount thread shut down." );
}

bool MemoryCardDriverThreaded::StorageDevicesChanged()
{
	LockMut( m_mutexStorageDevices );
	if( m_bStorageDevicesChanged )
    {
		m_bStorageDevicesChanged = false;
		return true;
    }
	else
    {
		return false;
    }
}

void MemoryCardDriverThreaded::GetStorageDevices( vector<UsbStorageDevice>& vDevicesOut )
{
	LockMut( m_mutexStorageDevices );
	vDevicesOut.clear();
	for( unsigned i=0; i<m_vStorageDevices.size(); i++ )
		vDevicesOut.push_back( m_vStorageDevices[i] );
}

bool MemoryCardDriverThreaded::MountAndTestWrite( UsbStorageDevice* pDevice, CString sMountPoint )
{
	LockMut( m_mutexStorageDevices );
	vector<UsbStorageDeviceEx>::const_iterator iter = find( m_vStorageDevices.begin(), m_vStorageDevices.end(), *pDevice );
	if( iter == m_vStorageDevices.end() )
	{
		LOG->Warn( "Trying to mount a memory card that is no longer connected. bus %d port %d device %d path %s", pDevice->iBus, pDevice->iPort, pDevice->iLevel, pDevice->sOsMountDir.c_str() );
		return false;
	}

	if( !iter->bWriteTestSucceeded )
		return false;

	this->Mount( pDevice, sMountPoint );
	return true;
}

/*
* Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
*	Chris Danford
*/
