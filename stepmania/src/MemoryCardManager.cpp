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
#include "PrefsManager.h"
#include "RageLog.h"

MemoryCardManager*	MEMCARDMAN = NULL;	// global and accessable from anywhere in our program


MemoryCardManager::MemoryCardManager()
{
	m_pDriver = MakeMemoryCardDriver();
	m_bCardsLocked = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_bTooLate[NUM_PLAYERS] = false;
		m_bWriteError[NUM_PLAYERS] = false;
	}
	m_pDriver->GetStorageDevices( m_vStorageDevices );
	ReassignCards();

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
		vector<UsbStorageDevice> vOld = m_vStorageDevices;	// copy
		m_pDriver->GetStorageDevices( m_vStorageDevices );
		vector<UsbStorageDevice> &vNew = m_vStorageDevices;
		vector<UsbStorageDevice> vConnects;	// fill these in below
		vector<UsbStorageDevice> vDisconnects;	// fill these in below

		unsigned i;

		// check for disconnects
		for( i=0; i<vOld.size(); i++ )
		{
			const UsbStorageDevice old = vOld[i];
			if( find(vNew.begin(),vNew.end(),old) == vNew.end() )	// didn't find
			{
				LOG->Trace( ssprintf("Disconnected bus %d port %d device %d path %s", old.iBus, old.iPortOnHub, old.iDeviceOnBus, old.sOsMountDir.c_str()) );
				vDisconnects.push_back( old );
			}
		}

		// check for connects
		for( i=0; i<vNew.size(); i++ )
		{
			const UsbStorageDevice newd = vNew[i];
			if( find(vOld.begin(),vOld.end(),newd) == vOld.end() )	// didn't find
			{
				LOG->Trace( ssprintf("Connected bus %d port %d device %d path %s", newd.iBus, newd.iPortOnHub, newd.iDeviceOnBus, newd.sOsMountDir.c_str()) );
				vConnects.push_back( newd );
			}
		}

		if( m_bCardsLocked )	// Cards are locked and play has begun.
		{
			// Don't re-assign devices
			
			// play disconnect sound if locked card was removed
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !m_Device[p].IsBlank() )	// card is in use
				{
					if( find(vOld.begin(),vOld.end(),m_Device[p]) != vOld.end() )	// match
					{
						m_Device[p].MakeBlank();
						m_soundDisconnect.Play();
					}
				}
			}
		}
		else	// cards aren't locked.  
		{
			// play sound for connects or disconnects
			if( !vConnects.empty() )
				m_soundConnect.Play();

			if( !vDisconnects.empty() )
				m_soundDisconnect.Play();

			ReassignCards();
		}
	}
}

MemoryCardManager::CardState MemoryCardManager::GetCardState( PlayerNumber pn )
{
	if( m_Device[pn].IsBlank() )
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
	if( m_Device[pn].IsBlank() )
		return "";
	else
		return m_Device[pn].sOsMountDir;
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
			if( !m_Device[p].IsBlank() )
				m_bWriteError[p] = m_pDriver->MountAndTestWrite( &m_Device[p] );
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

		ReassignCards();
	}
}

void MemoryCardManager::ReassignCards()
{
	// Do re-assigning: choose a storage device for each player
	vector<UsbStorageDevice> vDevices = m_vStorageDevices;	// copy
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_Device[p].MakeBlank();

		int i;

		// search for card dir match
		if( !PREFSMAN->m_sMemoryCardDir[p].empty() )
		{
			for( i=0; i<vDevices.size(); i++ )
			{
				UsbStorageDevice &usd = vDevices[i];
				if( usd.sOsMountDir.CompareNoCase(PREFSMAN->m_sMemoryCardDir[p]) == 0 )	// match
					goto match;
			}
		}

		// search for USB bus match
		for( i=0; i<vDevices.size(); i++ )
		{
			UsbStorageDevice &usd = vDevices[i];
			if( PREFSMAN->m_iMemoryCardUsbBus[p] != -1 && 
				PREFSMAN->m_iMemoryCardUsbBus[p] != usd.iBus )
				continue;	// not a match

			if( PREFSMAN->m_iMemoryCardUsbPort[p] != -1 && 
				PREFSMAN->m_iMemoryCardUsbPort[p] != usd.iPortOnHub )
				continue;	// not a match

			goto match;
		}
		
		LOG->Trace( "Player %d memory card: none", p+1 );
		continue;	// no matches
match:
		m_Device[p] = vDevices[i];	// save a copy
		LOG->Trace( "Player %d memory card: '%s'", p+1, m_Device[p].sOsMountDir.c_str() );
		vDevices.erase( vDevices.begin()+i );	// remove the device so we don't match it for another player
	}
}