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
#include "ScreenManager.h"
#include "ProfileManager.h"

MemoryCardManager*	MEMCARDMAN = NULL;	// global and accessable from anywhere in our program


MemoryCardManager::MemoryCardManager()
{
	m_pDriver = MakeMemoryCardDriver();
	m_bCardsLocked = false;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_bTooLate[p] = false;
		m_bWriteError[p] = false;
	}
	m_pDriver->GetStorageDevices( m_vStorageDevices );

	m_soundReady.Load( THEME->GetPathToS("MemoryCardManager ready") );
	m_soundError.Load( THEME->GetPathToS("MemoryCardManager error") );
	m_soundTooLate.Load( THEME->GetPathToS("MemoryCardManager too late") );
	m_soundDisconnect.Load( THEME->GetPathToS("MemoryCardManager disconnect") );

	AssignUnassignedCards();
}

MemoryCardManager::~MemoryCardManager()
{
	delete m_pDriver;
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


		// check for disconnects
		{
			for( unsigned i=0; i<vOld.size(); i++ )
			{
				const UsbStorageDevice old = vOld[i];
				if( find(vNew.begin(),vNew.end(),old) == vNew.end() )	// didn't find
				{
					LOG->Trace( ssprintf("Disconnected bus %d port %d device %d path %s", old.iBus, old.iPortOnHub, old.iDeviceOnBus, old.sOsMountDir.c_str()) );
					vDisconnects.push_back( old );
				}
			}
		}

		// check for connects
		{
			for( unsigned i=0; i<vNew.size(); i++ )
			{
				const UsbStorageDevice newd = vNew[i];
				if( find(vOld.begin(),vOld.end(),newd) == vOld.end() )	// didn't find
				{
					LOG->Trace( ssprintf("Connected bus %d port %d device %d path %s", newd.iBus, newd.iPortOnHub, newd.iDeviceOnBus, newd.sOsMountDir.c_str()) );
					vConnects.push_back( newd );
				}
			}
		}

		// unassign cards that were disconnected
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( m_Device[p].IsBlank() )	// not assigned a card
					continue;
				
				if( find(vDisconnects.begin(),vDisconnects.end(),m_Device[p]) != vDisconnects.end() )	// assigned card was disconnected
				{
					m_Device[p].MakeBlank();
					m_soundDisconnect.Play();

					if( PROFILEMAN->ProfileWasLoadedFromMemoryCard((PlayerNumber)p) )
						PROFILEMAN->UnloadProfile( (PlayerNumber)p );
				}
			}
		}

		AssignUnassignedCards();

		if( !m_bCardsLocked )
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
				PROFILEMAN->LoadFirstAvailableProfile( (PlayerNumber)p );
		}
		SCREENMAN->RefreshCreditsMessages();
	}
}

MemoryCardState MemoryCardManager::GetCardState( PlayerNumber pn )
{
	if( m_Device[pn].IsBlank() )
		return MEMORY_CARD_STATE_NO_CARD;
	else if( m_bTooLate[pn] )
		return MEMORY_CARD_STATE_TOO_LATE;
	else if( m_bWriteError[pn] )
		return MEMORY_CARD_STATE_WRITE_ERROR;
	else
		return MEMORY_CARD_STATE_READY;
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
	m_bCardsLocked = bLock;

	if( !bLock )
	{
		// clear too late flag
		for( int p=0; p<NUM_PLAYERS; p++ )
			m_bTooLate[p] = false;

		AssignUnassignedCards();
	}
}

void MemoryCardManager::AssignUnassignedCards()
{
	LOG->Trace( "AssignUnassignedCards" );

	// make a list of unassigned
	vector<UsbStorageDevice> vUnassignedDevices = m_vStorageDevices;	// copy

	// remove cards that are already assigned
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( m_Device[p].IsBlank() )	// card not assigned to this player
				continue;

			vector<UsbStorageDevice>::iterator it = find(vUnassignedDevices.begin(),vUnassignedDevices.end(),m_Device[p]);
			ASSERT( it != vUnassignedDevices.end() )	// the assigned card better be connected!
			vUnassignedDevices.erase( it );
		}
	}

	// try to assign each device to a player
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			LOG->Trace( "Looking for a card for Player %d", p+1 );

			if( !m_Device[p].IsBlank() )	// they already have an assigned card
			{
				LOG->Trace( "Already has a card", p+1 );
				continue;	// skip
			}

			unsigned i;

			// search for card dir match
			if( !PREFSMAN->m_sMemoryCardOsMountPoint[p].empty() )
			{
				for( i=0; i<vUnassignedDevices.size(); i++ )
				{
					UsbStorageDevice &usd = vUnassignedDevices[i];
					if( usd.sOsMountDir.CompareNoCase(PREFSMAN->m_sMemoryCardOsMountPoint[p]) == 0 )	// match
					{
						LOG->Trace( "dir match:  iUsbStorageIndex: %d, iBus: %d, iDeviceOnBus: %d, iPortOnHub: %d",
							usbd.iUsbStorageIndex, usbd.iBus, usbd.iDeviceOnBus, usbd.iPortOnHub );
						goto match;
					}
				}
			}

			// search for USB bus match
			for( i=0; i<vUnassignedDevices.size(); i++ )
			{
				UsbStorageDevice &usd = vUnassignedDevices[i];

				if( PREFSMAN->m_iMemoryCardUsbBus[p] != -1 && 
					PREFSMAN->m_iMemoryCardUsbBus[p] != usd.iBus )
					continue;	// not a match

				if( PREFSMAN->m_iMemoryCardUsbPort[p] != -1 && 
					PREFSMAN->m_iMemoryCardUsbPort[p] != usd.iPortOnHub )
					continue;	// not a match

				LOG->Trace( "bus/port match:  iUsbStorageIndex: %d, iBus: %d, iDeviceOnBus: %d, iPortOnHub: %d",
					usbd.iUsbStorageIndex, usbd.iBus, usbd.iDeviceOnBus, usbd.iPortOnHub );
				goto match;
			}
			
			LOG->Trace( "Player %d memory card: none", p+1 );
			continue;	// no matches
match:
			m_Device[p] = vUnassignedDevices[i];	// save a copy
			LOG->Trace( "Player %d memory card: '%s'", p+1, m_Device[p].sOsMountDir.c_str() );
			vUnassignedDevices.erase( vUnassignedDevices.begin()+i );	// remove the device so we don't match it for another player
			m_bTooLate[p] = m_bCardsLocked;
			m_bWriteError[p] = false;
			if( !m_bCardsLocked )
				if( !m_pDriver->MountAndTestWrite(&m_Device[p]) )
					m_bWriteError[p] = true;

			// play sound
			if( m_bWriteError[p] )
				m_soundError.Play();
			else if( m_bTooLate[p] )
				m_soundTooLate.Play();
			else
				m_soundReady.Play();
		}
	}
}

void MemoryCardManager::FlushAllDisks()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !m_Device[p].IsBlank() )	// they already have an assigned card
			continue;	// skip
		if( m_bWriteError[p] || m_bTooLate[p] )
			continue;	// skip
		m_pDriver->Flush(&m_Device[p]);
	}

	m_pDriver->ResetUsbStorage();
}
