#include "global.h"
#include "MemoryCardDriverThreaded.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFileManager.h"


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
	m_mutexPause("Pause"),
	m_mutexStorageDevices("StorageDevices")
{
	m_bShutdownNextUpdate = false;
	m_bStorageDevicesChanged = false;
	m_MountThreadState = detect_and_mount;
}

void MemoryCardDriverThreaded::StartThread()
{
	m_threadMemoryCardMount.SetName("MemoryCardMount");
	m_threadMemoryCardMount.Create( MountThread_Start, this );
}

void MemoryCardDriverThreaded::StopThread()
{
	m_bShutdownNextUpdate = true;
	LOG->Trace( "Shutting down Mount thread..." );
	m_threadMemoryCardMount.Wait();
	LOG->Trace( "Mount thread shut down." );
}

MemoryCardDriverThreaded::~MemoryCardDriverThreaded()
{
	/* Must call StopThread from the derived destructor.  Otherwise, we can
	 * call MountThreadDoOneUpdate from the thread after the derived dtor,
	 * which means that function is now virtual, and we'll crash with "pure
	 * virtual method called". */
	ASSERT( !m_threadMemoryCardMount.IsCreated() );
}

void MemoryCardDriverThreaded::SetMountThreadState( MountThreadState mts )
{
	CHECKPOINT;

	MountThreadState old = m_MountThreadState;

	if( old != paused && mts == paused )
		m_mutexPause.Lock();

	if( old == paused && mts != paused )
		m_mutexPause.Unlock();

	m_MountThreadState = mts;
}

int MemoryCardDriverThreaded::MountThread_Start( void *p )
{
	((MemoryCardDriverThreaded*)p)->MountThreadMain();
	return 0;
}

void MemoryCardDriverThreaded::MountThreadMain()
{
	CHECKPOINT;

	while( !m_bShutdownNextUpdate )
	{      
		LockMut( m_mutexPause );	// wait until we're unpaused
		this->MountThreadDoOneUpdate();
	}
}

bool MemoryCardDriverThreaded::StorageDevicesChanged()
{
	CHECKPOINT;

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
	CHECKPOINT;

	LockMut( m_mutexStorageDevices );
	vDevicesOut.clear();
	for( unsigned i=0; i<m_vStorageDevices.size(); i++ )
		vDevicesOut.push_back( m_vStorageDevices[i] );
}

bool MemoryCardDriverThreaded::MountAndTestWrite( UsbStorageDevice* pDevice, CString sMountPoint )
{
	LockMut( m_mutexStorageDevices );
	vector<UsbStorageDevice>::const_iterator iter = find( m_vStorageDevices.begin(), m_vStorageDevices.end(), *pDevice );
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


void MemoryCardDriverThreaded::UnmountMountPoint( const CString &sMountPoint )
{
	vector<RageFileManager::DriverLocation> Mounts;
	FILEMAN->GetLoadedDrivers( Mounts );
	for( unsigned i = 0; i < Mounts.size(); ++i )
	{
		if( Mounts[i].Type.CompareNoCase( "dir" ) )
				continue; // wrong type
		if( Mounts[i].MountPoint.CompareNoCase( sMountPoint ) )
				continue; // wrong mount point
		FILEMAN->Unmount( Mounts[i].Type, Mounts[i].Root, Mounts[i].MountPoint );
	}
}

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
