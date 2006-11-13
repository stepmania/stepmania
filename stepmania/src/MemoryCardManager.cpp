#include "global.h"
#include "MemoryCardManager.h"
#include "arch/MemoryCard/MemoryCardDriver.h"	// for UsbStorageDevice
#include "ScreenManager.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "RageFileManager.h"
#include "RageFileDriver.h"
#include "RageFileDriverTimeout.h"
#include "MessageManager.h"
#include "Foreach.h"
#include "RageUtil_WorkerThread.h"
#include "arch/arch.h"
#include "arch/MemoryCard/MemoryCardDriver_Null.h"
#include "LuaManager.h"

MemoryCardManager*	MEMCARDMAN = NULL;	// global and accessable from anywhere in our program

static void MemoryCardOsMountPointInit( size_t /*PlayerNumber*/ i, RString &sNameOut, RString &defaultValueOut )
{
	sNameOut = ssprintf( "MemoryCardOsMountPointP%d", int(i+1) );
	defaultValueOut = "";
}

static void MemoryCardUsbBusInit( size_t /*PlayerNumber*/ i, RString &sNameOut, int &defaultValueOut )
{
	sNameOut = ssprintf( "MemoryCardUsbBusP%d", int(i+1) );
	defaultValueOut = -1;
}

static void MemoryCardUsbPortInit( size_t /*PlayerNumber*/ i, RString &sNameOut, int &defaultValueOut )
{
	sNameOut = ssprintf( "MemoryCardUsbPortP%d",int(i+1) );
	defaultValueOut = -1;
}

static void MemoryCardUsbLevelInit( size_t /*PlayerNumber*/ i, RString &sNameOut, int &defaultValueOut )
{
	sNameOut = ssprintf( "MemoryCardUsbLevelP%d", int(i+1) );
	defaultValueOut = -1;
}

static Preference<bool>		g_bMemoryCards( "MemoryCards", true );
static Preference<bool>		g_bMemoryCardProfiles( "MemoryCardProfiles", true );

// if set, always use the device that mounts to this point
Preference1D<RString>		MemoryCardManager::m_sMemoryCardOsMountPoint( MemoryCardOsMountPointInit,	NUM_PLAYERS );

// Look for this level, bus, port when assigning cards.  -1 = match any
Preference1D<int>		MemoryCardManager::m_iMemoryCardUsbBus( MemoryCardUsbBusInit,			NUM_PLAYERS );
Preference1D<int>		MemoryCardManager::m_iMemoryCardUsbPort( MemoryCardUsbPortInit,			NUM_PLAYERS );
Preference1D<int>		MemoryCardManager::m_iMemoryCardUsbLevel( MemoryCardUsbLevelInit,		NUM_PLAYERS );

Preference<RString>		MemoryCardManager::m_sEditorMemoryCardOsMountPoint( "EditorMemoryCardOsMountPoint",	"" );

const RString MEM_CARD_MOUNT_POINT[NUM_PLAYERS] =
{
	/* @ is importast; see RageFileManager LoadedDriver::GetPath */
	"/@mc1/",
	"/@mc2/",
};

static const RString MEM_CARD_MOUNT_POINT_INTERNAL[NUM_PLAYERS] =
{
	/* @ is importast; see RageFileManager LoadedDriver::GetPath */
	"/@mc1int/",
	"/@mc2int/",
};

/* Only access the memory card driver in a timeout-safe thread. */
class ThreadedMemoryCardWorker: public RageWorkerThread
{
public:
	ThreadedMemoryCardWorker();
	~ThreadedMemoryCardWorker();

	enum MountThreadState 
	{
		detect_and_mount,
		detect_and_dont_mount,
		paused
	};
	void SetMountThreadState( MountThreadState mts );

	/* These functions may time out. */
	bool Mount( const UsbStorageDevice *pDevice );
	bool Unmount( const UsbStorageDevice *pDevice );

	/* This function will not time out. */
	bool StorageDevicesChanged( vector<UsbStorageDevice> &aOut );

protected:
	void HandleRequest( int iRequest );
	void RequestTimedOut();
	void DoHeartbeat();

private:
	MemoryCardDriver *m_pDriver;
	MountThreadState m_MountThreadState;

	/* We make a copy of the device info we're working with, since the pointer
	 * we're given will become invalid if the operation times out and DoRequest
	 * returns. */
	UsbStorageDevice m_RequestDevice;

	bool m_bResult;

	RageMutex UsbStorageDevicesMutex;
	bool m_bUsbStorageDevicesChanged;
	vector<UsbStorageDevice> m_aUsbStorageDevices;

	vector<UsbStorageDevice> m_aMountedDevices;

	enum
	{
		REQ_MOUNT,
		REQ_UNMOUNT,
	};
};

bool ThreadedMemoryCardWorker::StorageDevicesChanged( vector<UsbStorageDevice> &aOut )
{
	UsbStorageDevicesMutex.Lock();
	if( !m_bUsbStorageDevicesChanged )
	{
		UsbStorageDevicesMutex.Unlock();
		return false;
	}

	aOut = m_aUsbStorageDevices;
	m_aUsbStorageDevices.clear();
	m_bUsbStorageDevicesChanged = false;

	UsbStorageDevicesMutex.Unlock();
	return true;
}


ThreadedMemoryCardWorker::ThreadedMemoryCardWorker():
	RageWorkerThread("MemoryCardWorker"),
	UsbStorageDevicesMutex("UsbStorageDevicesMutex")
{
	if( g_bMemoryCards )
		m_pDriver = MakeMemoryCardDriver();
	else
		m_pDriver = new MemoryCardDriver_Null;

	m_MountThreadState = detect_and_mount;
	SetHeartbeat( 0.1f );

	StartThread();
}

ThreadedMemoryCardWorker::~ThreadedMemoryCardWorker()
{
	StopThread();

	delete m_pDriver;
}



void ThreadedMemoryCardWorker::SetMountThreadState( MountThreadState mts )
{
	/* If "pause", stop calling updates in the heartbeat.  In principle, we should
	 * also not return from this function until the current heartbeat, if running,
	 * finishes.  However, since we can't guarantee that it'll exit within the timeout,
	 * there's no point: we have to return when we time out, and in that case the
	 * heartbeat will still be running.  I don't know if the reasons for pausing
	 * really need us to wait, so don't. */
	m_MountThreadState = mts;
}

void ThreadedMemoryCardWorker::HandleRequest( int iRequest )
{
	switch( iRequest )
	{
	case REQ_MOUNT:
		m_bResult = m_pDriver->Mount( &m_RequestDevice );
		m_aMountedDevices.push_back( m_RequestDevice );
		break;

	case REQ_UNMOUNT:
	{
		m_pDriver->Unmount( &m_RequestDevice );
		vector<UsbStorageDevice>::iterator it = 
			find( m_aMountedDevices.begin(), m_aMountedDevices.end(), m_RequestDevice );
		if( it == m_aMountedDevices.end() )
			LOG->Warn( "Unmounted a device that wasn't mounted" );
		else
			m_aMountedDevices.erase( it );
		break;
	}
	}
}

void ThreadedMemoryCardWorker::RequestTimedOut()
{
	/* We timed out, so the current operation will abort.  The unmount request
	 * may be skipped, if it's attempted during the timeout, so unmount all
	 * mounted devices. */
	for( unsigned i = 0; i < m_aMountedDevices.size(); ++i )
		m_pDriver->Unmount( &m_aMountedDevices[i] );

	m_aMountedDevices.clear();
}

void ThreadedMemoryCardWorker::DoHeartbeat()
{
	if( m_MountThreadState == paused )
		return;

	/* If true, detect and mount.  If false, only detect. */
	bool bMount = (m_MountThreadState == detect_and_mount);

	vector<UsbStorageDevice> aStorageDevices;
//	LOG->Trace("update");
	if( !m_pDriver->DoOneUpdate( bMount, aStorageDevices ) )
		return;

	UsbStorageDevicesMutex.Lock();
	m_aUsbStorageDevices = aStorageDevices;
	m_bUsbStorageDevicesChanged = true;
	UsbStorageDevicesMutex.Unlock();
}

bool ThreadedMemoryCardWorker::Mount( const UsbStorageDevice *pDevice )
{
	ASSERT( TimeoutEnabled() );

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
		return false;

	m_RequestDevice = *pDevice;
	if( !DoRequest(REQ_MOUNT) )
		return false;

	return m_bResult;
}

bool ThreadedMemoryCardWorker::Unmount( const UsbStorageDevice *pDevice )
{
	ASSERT( TimeoutEnabled() );

	/* If we're currently in a timed-out state, fail. */
	if( IsTimedOut() )
		return false;

	m_RequestDevice = *pDevice;
	if( !DoRequest(REQ_UNMOUNT) )
		return false;

	return true;
}

static ThreadedMemoryCardWorker *g_pWorker = NULL;

MemoryCardManager::MemoryCardManager()
{
	ASSERT( g_pWorker == NULL );

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "MEMCARDMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}

	g_pWorker = new ThreadedMemoryCardWorker;

	m_bCardsLocked = false;
	FOREACH_PlayerNumber( p )
	{
		m_bMounted[p] = false;
		m_State[p] = MemoryCardState_NoCard;
	}
	
	/* These can play at any time.  Preload them, so we don't cause a skip in gameplay. */
	m_soundReady.Load( THEME->GetPathS("MemoryCardManager","ready"), true );
	m_soundError.Load( THEME->GetPathS("MemoryCardManager","error"), true );
	m_soundTooLate.Load( THEME->GetPathS("MemoryCardManager","too late"), true );
	m_soundDisconnect.Load( THEME->GetPathS("MemoryCardManager","disconnect"), true );

	/* Mount the filesystems that we'll use with Mount().  Use a bogus root for the internal
	 * mount for now, since we don't know where we'll mount to yet; nothing reads from it
	 * until it's mounted, anyway. */
	FOREACH_PlayerNumber( pn )
	{
		FILEMAN->Mount( "dir", "/", MEM_CARD_MOUNT_POINT_INTERNAL[pn] );
		FILEMAN->Mount( "timeout", MEM_CARD_MOUNT_POINT_INTERNAL[pn], MEM_CARD_MOUNT_POINT[pn] );
	}
}

MemoryCardManager::~MemoryCardManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "MESSAGEMAN" );

	ASSERT( g_pWorker != NULL );
	SAFE_DELETE(g_pWorker);

	FOREACH_PlayerNumber( pn )
	{
		FILEMAN->Unmount( "", "", MEM_CARD_MOUNT_POINT[pn] );
		FILEMAN->Unmount( "", "", MEM_CARD_MOUNT_POINT_INTERNAL[pn] );
	}
}

void MemoryCardManager::Update( float fDelta )
{
	vector<UsbStorageDevice> vOld;
	
	vOld = m_vStorageDevices;	// copy
	if( !g_pWorker->StorageDevicesChanged( m_vStorageDevices ) )
		return;

	UpdateAssignments();

	MESSAGEMAN->Broadcast( Message_StorageDevicesChanged );
}

/* Assign cards from m_vStorageDevices to m_Device. */
void MemoryCardManager::UpdateAssignments()
{
	if( !g_bMemoryCardProfiles.Get() )
		return;

	// make a list of unassigned
	vector<UsbStorageDevice> vUnassignedDevices = m_vStorageDevices;        // copy
	
	// remove cards that are already assigned
	FOREACH_PlayerNumber( p )
	{
		UsbStorageDevice &assigned_device = m_Device[p];
		if( assigned_device.IsBlank() )     // no card assigned to this player
			continue;
		
		FOREACH( UsbStorageDevice, vUnassignedDevices, d )
		{
			if( *d == assigned_device )
			{
				vUnassignedDevices.erase( d );
				break;
			}
		}
	}
	
	// Try to assign each device to a player.  If a player already has a device
	// assigned, and the device still exists, keep him on the same card.
	FOREACH_PlayerNumber( p )
	{
		UsbStorageDevice &assigned_device = m_Device[p];
		if( !assigned_device.IsBlank() )
		{
			/* The player has a card assigned.  If it's been removed, clear it. */
			vector<UsbStorageDevice>::iterator it = find( m_vStorageDevices.begin(), m_vStorageDevices.end(), assigned_device );
			if( it != m_vStorageDevices.end() )
			{
				/* The player has a card, and it's still plugged in.  Update any changed
				 * state, such as m_State. */
				LOG->Trace( "Player %d already has a card: '%s'", p+1, assigned_device.sOsMountDir.c_str() );
				assigned_device = *it;
				continue;
			}

			/* The assigned card has been removed; clear it and re-search. */
			LOG->Trace( "Player %i: disconnected bus %d port %d device %d path %s",
				p+1, assigned_device.iBus, assigned_device.iPort, assigned_device.iLevel, assigned_device.sOsMountDir.c_str() );
			assigned_device.MakeBlank();
		}

		LOG->Trace( "Looking for a card for Player %d", p+1 );
				
		FOREACH( UsbStorageDevice, vUnassignedDevices, d )
		{
			// search for card dir match
			if( !m_sMemoryCardOsMountPoint[p].Get().empty() &&
				d->sOsMountDir.CompareNoCase(m_sMemoryCardOsMountPoint[p].Get()) )
				continue;      // not a match
			
			// search for USB bus match
			if( m_iMemoryCardUsbBus[p] != -1 &&
				m_iMemoryCardUsbBus[p] != d->iBus )
				continue;       // not a match
			
			if( m_iMemoryCardUsbPort[p] != -1 &&
				m_iMemoryCardUsbPort[p] != d->iPort )
				continue;       // not a match
			
			if( m_iMemoryCardUsbLevel[p] != -1 &&
				m_iMemoryCardUsbLevel[p] != d->iLevel )
				continue;       // not a match
			
			LOG->Trace( "Player %i: matched %s", p+1, d->sDevice.c_str() );

			assigned_device = *d;    // save a copy
			vUnassignedDevices.erase( d );       // remove the device so we don't match it for another player
			break;
		}
	}

	CheckStateChanges();
}

void MemoryCardManager::CheckStateChanges()
{
	/* Deal with assignment changes. */
	FOREACH_PlayerNumber( p )
	{
		const UsbStorageDevice &new_device = m_Device[p];		    

		MemoryCardState state = MemoryCardState_Invalid;
		RString sError;

		if( m_bCardsLocked )
		{
			if( m_FinalDevice[p].m_State == UsbStorageDevice::STATE_NONE )
			{
				/* We didn't have a card when we finalized, so we won't accept anything.
				 * If anything is inserted (even if it's still checking), say TOO LATE. */
				if( new_device.m_State == UsbStorageDevice::STATE_NONE )
					state = MemoryCardState_NoCard;
				else
					state = MemoryCardState_TooLate;
			}
			else
			{
				/* We had a card inserted when we finalized. */
				if( new_device.m_State == UsbStorageDevice::STATE_NONE )
					state = MemoryCardState_Removed;
				if( new_device.m_State == UsbStorageDevice::STATE_READY )
				{
					if( m_FinalDevice[p].sSerial != new_device.sSerial )
					{
						/* A different card is inserted than we had when we finalized. */
						state = MemoryCardState_Error;
						sError = "Changed";
					}
				}

				/* Otherwise, the card is checking or has an error.  Use the regular logic. */
			}
		}

		if( state == MemoryCardState_Invalid )
		{
			switch( new_device.m_State )
			{
			case UsbStorageDevice::STATE_NONE:
				state = MemoryCardState_NoCard;
				break;

			case UsbStorageDevice::STATE_CHECKING:
				state = MemoryCardState_Checking;
				break;

			case UsbStorageDevice::STATE_ERROR:
				state = MemoryCardState_Error;
				sError = new_device.m_sError;
				break;

			case UsbStorageDevice::STATE_READY:
				state = MemoryCardState_Ready;
				break;
			}
		}

		MemoryCardState LastState = m_State[p];
		if( m_State[p] != state )
		{
			// play sound
			switch( state )
			{
			case MemoryCardState_NoCard:
			case MemoryCardState_Removed:
				if( LastState == MemoryCardState_Ready )
				{
					m_soundDisconnect.Play();
					MESSAGEMAN->Broadcast( (MessageID)(Message_CardRemovedP1+p) );
				}
				break;
			case MemoryCardState_Ready:
				m_soundReady.Play();
				break;
			case MemoryCardState_TooLate:
				m_soundTooLate.Play();
				break;
			case MemoryCardState_Error:
				m_soundError.Play();
				break;
			}

			m_State[p] = state;
			m_sError[p] = sError;
		}
	}

	SCREENMAN->RefreshCreditsMessages();
}

void MemoryCardManager::LockCards()
{
	if( m_bCardsLocked )
		return;

	g_pWorker->SetTimeout( 5 );

	/* If either player's card is in STATE_CHECKING, we need to give it a chance
	 * to finish up before returning. */
	bool bLogged = false;
	while( !g_pWorker->IsTimedOut() )
	{
		/* Check for changes. */
		Update(0);

		bool bEitherPlayerIsChecking = false;
		FOREACH_PlayerNumber( p )
			if( m_Device[p].m_State == UsbStorageDevice::STATE_CHECKING )
				bEitherPlayerIsChecking = true;
		if( !bEitherPlayerIsChecking )
			break;

		/* Only if we need to, wait for something to happen.  If we time out waiting for
		 * a heartbeat, give up. */
		if( !bLogged )
		{
			bLogged = true;
			LOG->Trace( "One or more cards are in STATE_CHECKING; waiting for them ..." );
		}

		if( !g_pWorker->WaitForOneHeartbeat() )
		{
			LOG->Trace( "STATE_CHECKING wait timed out" );
			break;
		}
	}

	g_pWorker->SetTimeout( -1 );

	/* Set the final state. */
	CheckStateChanges();

	FOREACH_PlayerNumber( p )
	{
		/* If the card in this player's slot is ready, then use it.  If there is
		 * no card ready when we finalize, clear m_FinalDevice. */
		if( m_Device[p].m_State == UsbStorageDevice::STATE_READY )
			m_FinalDevice[p] = m_Device[p];
		else
			m_FinalDevice[p] = UsbStorageDevice();
	}

	/* Set this last, since it changes the behavior of CheckStateChanges. */
	m_bCardsLocked = true;
}

void MemoryCardManager::UnlockCards()
{
	m_bCardsLocked = false;
	
	g_pWorker->SetMountThreadState( ThreadedMemoryCardWorker::detect_and_mount );

	/* If a memory card was inserted too late last game, allow it now. */
	CheckStateChanges();
}

/* Called just before reading or writing to the memory card.  Should block. */
bool MemoryCardManager::MountCard( PlayerNumber pn, int iTimeout )
{
	LOG->Trace( "MemoryCardManager::MountCard(%i)", pn );
	if( GetCardState(pn) != MemoryCardState_Ready )
		return false;
	ASSERT( !m_Device[pn].IsBlank() );

	/* Pause the mounting thread when we mount the first drive. */
	bool bStartingMemoryCardAccess = true;
	FOREACH_PlayerNumber( p )
		if( m_bMounted[p] )
			bStartingMemoryCardAccess = false; /* already did */
	if( bStartingMemoryCardAccess )
	{
		/* We're starting to do stuff to the memory cards. */
		this->PauseMountingThread( iTimeout );
	}

	if( !g_pWorker->Mount( &m_Device[pn] ) )
	{
		LOG->Trace( "MemoryCardManager::MountCard: mount failed" );
		if( bStartingMemoryCardAccess )
			this->UnPauseMountingThread();

		return false;
	}

	m_bMounted[pn] = true;

	RageFileDriver *pDriver = FILEMAN->GetFileDriver( MEM_CARD_MOUNT_POINT_INTERNAL[pn] );
	if( pDriver == NULL )
	{
		LOG->Warn( "FILEMAN->GetFileDriver(%s) failed", MEM_CARD_MOUNT_POINT_INTERNAL[pn].c_str() );
		return true;
	}

	/* We don't want to unmount the timeout FS.  Instead, just move the target. */
	pDriver->Remount( m_Device[pn].sOsMountDir );

	/* Flush mountpoints pointing to what we've mounted. */
	FILEMAN->FlushDirCache( MEM_CARD_MOUNT_POINT[pn] );
	FILEMAN->FlushDirCache( MEM_CARD_MOUNT_POINT_INTERNAL[pn] );

	FILEMAN->ReleaseFileDriver( pDriver );

	return true;
}

/* Temporarily mount a specific card.  On unmount, the device will be reverted.  This is used
 * to access cards in the editor. */
bool MemoryCardManager::MountCard( PlayerNumber pn, const UsbStorageDevice &d, int iTimeout )
{
	m_Device[pn] = d;
	CheckStateChanges();
	return MountCard( pn, iTimeout );
}

/* Called when finished accessing a memory card.  If writes have been performed,
 * will block until flushed. */
void MemoryCardManager::UnmountCard( PlayerNumber pn )
{
	LOG->Trace( "MemoryCardManager::UnmountCard(%i) (mounted: %i)", pn, m_bMounted[pn] );
	if( m_Device[pn].IsBlank() )
		return;

	if( !m_bMounted[pn] )
		return;

	/* Leave our own filesystem drivers mounted.  Unmount the kernel mount. */
	g_pWorker->Unmount( &m_Device[pn] );

	/* Flush mountpoints pointing to what we've unmounted. */
	FILEMAN->FlushDirCache( MEM_CARD_MOUNT_POINT[pn] );
	FILEMAN->FlushDirCache( MEM_CARD_MOUNT_POINT_INTERNAL[pn] );

	m_bMounted[pn] = false;

	/* Unpause the mounting thread when we unmount the last drive. */
	bool bNeedUnpause = true;
	FOREACH_PlayerNumber( p )
		if( m_bMounted[p] )
			bNeedUnpause = false;
	if( bNeedUnpause )
		this->UnPauseMountingThread();

	/* If memory card profiles are disabled, then this was assigned by passing to a
	 * UsbStorageDevice to MountCard.  Remove the temporary assignment. */
	if( !g_bMemoryCardProfiles.Get() )
	{
		m_Device[pn] = UsbStorageDevice();
		CheckStateChanges();
	}
}

bool MemoryCardManager::PathIsMemCard( RString sDir ) const
{
	FOREACH_PlayerNumber( p )
		if( !sDir.Left(MEM_CARD_MOUNT_POINT[p].size()).CompareNoCase( MEM_CARD_MOUNT_POINT[p] ) )
			return true;
	return false;
}

bool MemoryCardManager::IsNameAvailable( PlayerNumber pn ) const
{
	return m_Device[pn].bIsNameAvailable;
}

RString MemoryCardManager::GetName( PlayerNumber pn ) const
{
	return m_Device[pn].sName;
}

void MemoryCardManager::PauseMountingThread( int iTimeout )
{
	LOG->Trace( "MemoryCardManager::PauseMountingThread" );

	g_pWorker->SetMountThreadState( ThreadedMemoryCardWorker::paused );

	/* Start the timeout period. */
	g_pWorker->SetTimeout( (float) iTimeout );
	RageFileDriverTimeout::SetTimeout( (float) iTimeout );
}

void MemoryCardManager::UnPauseMountingThread()
{
	LOG->Trace( "MemoryCardManager::UnPauseMountingThread" );

	g_pWorker->SetMountThreadState( ThreadedMemoryCardWorker::detect_and_mount );

	/* End the timeout period. */
	g_pWorker->SetTimeout( -1 );
	RageFileDriverTimeout::SetTimeout( -1 );
}

bool IsAnyPlayerUsingMemoryCard()
{
	FOREACH_HumanPlayer( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) == MemoryCardState_Ready )
			return true;
	}
	return false;
}


// lua start
#include "LuaBinding.h"

class LunaMemoryCardManager: public Luna<MemoryCardManager>
{
public:
	static int IsAnyPlayerUsingMemoryCard( T* p, lua_State *L )	{ lua_pushboolean(L, ::IsAnyPlayerUsingMemoryCard() ); return 1; }

	LunaMemoryCardManager()
	{
		ADD_METHOD( IsAnyPlayerUsingMemoryCard );
	}
};

LUA_REGISTER_CLASS( MemoryCardManager )
// lua end


/*
 * (c) 2003-2005 Chris Danford, Glenn Maynard
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
