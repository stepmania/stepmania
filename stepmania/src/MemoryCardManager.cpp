#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: MemoryCardManager

 Desc: See Header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "MemoryCardManager.h"
#include "arch/MemoryCard/MemoryCardDriver.h"	// for UsbStorageDevice
#include "arch/arch.h"
#include "ScreenManager.h"
#include "ThemeManager.h"

MemoryCardManager*	MEMCARDMAN = NULL;	// global and accessable from anywhere in our program

vector<UsbStorageDevice> g_StorageDevices;

MemoryCardManager::MemoryCardManager()
{
	m_pDriver = MakeMemoryCardDriver();
	m_bCardsLocked = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_bTooLate[NUM_PLAYERS] = false;
		m_bWriteError[NUM_PLAYERS] = false;
	}
	m_pDriver->GetStorageDevices( g_StorageDevices );

	m_soundConnect.Load( THEME->GetPathToS("MemoryCardManager connect") );
	m_soundDisconnect.Load( THEME->GetPathToS("MemoryCardManager disconnect") );
}

MemoryCardManager::~MemoryCardManager()
{

}

void MemoryCardManager::Update( float fDelta )
{
	if( m_pDriver->StorageDevicesChanged() )
	{
		vector<UsbStorageDevice> vOld = g_StorageDevices;	// make a copy
		m_pDriver->GetStorageDevices( g_StorageDevices );
		vector<UsbStorageDevice> &vNew = g_StorageDevices;

		unsigned i;

		// check for disconnects
		for( i=0; i<vOld.size(); i++ )
		{
			const UsbStorageDevice old = vOld[i];
			if( find(vNew.begin(),vNew.end(),old) == vNew.end() )
			{
				SCREENMAN->SystemMessage( ssprintf("Disconnected bus %d port %d device %d path %s", old.iBus, old.iPortOnHub, old.iDeviceOnBus, old.sOsMountDir.c_str()) );
				m_soundDisconnect.Play();
			}
		}

		// check for connects
		for( i=0; i<vNew.size(); i++ )
		{
			const UsbStorageDevice newd = vNew[i];
			if( find(vOld.begin(),vOld.end(),newd) == vOld.end() )
			{
				SCREENMAN->SystemMessage( ssprintf("Connected bus %d port %d device %d path %s", newd.iBus, newd.iPortOnHub, newd.iDeviceOnBus, newd.sOsMountDir.c_str()) );
				m_soundConnect.Play();
			}
		}
	}
}

MemoryCardManager::CardState MemoryCardManager::GetCardState( PlayerNumber pn )
{
	if( m_pDevice[pn] == NULL )
		return no_card;
	else if( m_bTooLate[pn] )
		return too_late;
	else if( m_bWriteError[pn] )
		return write_error;
	else
		return ready;
}

CString MemoryCardManager::GetOsMountDir( PlayerNumber pn )
{
	if( m_pDevice[pn] == NULL )
		return "";
	else
		return m_pDevice[pn]->sOsMountDir;
}

void MemoryCardManager::LockCards( bool bLock )
{
	if( bLock == m_bCardsLocked )
		return;	// redundant
	
	m_bCardsLocked = bLock;

	if( bLock )
	{
		// try mounting
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_pDevice[p] )
				m_bWriteError[p] = m_pDriver->MountAndTestWrite( m_pDevice[p] );
		}
	}
	else
	{
		// clear error flags
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_bTooLate[p] = false;
			m_bWriteError[p] = false;
		}
	}
}

