#ifndef MemoryCardManager_H
#define MemoryCardManager_H
/*
-----------------------------------------------------------------------------
 Class: MemoryCardManager

 Desc: Holds user-chosen preferences that are saved between sessions.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "GameConstantsAndTypes.h"	// for MemoryCardState
#include "PlayerNumber.h"
#include "RageSound.h"
#include "arch/MemoryCard/MemoryCardDriver.h"


const CString MEM_CARD_MOUNT_POINT[NUM_PLAYERS] =
{
	/* @ is importast; see RageFileManager LoadedDriver::GetPath */
	"@mc1/",
	"@mc2/",
};

class MemoryCardManager
{
public:
	MemoryCardManager();
	~MemoryCardManager();

	void Update( float fDelta );

	MemoryCardState GetCardState( PlayerNumber pn );
	
//	CString GetOsMountDir( PlayerNumber pn );	// only valid when state = ready

	void LockCards( bool bLock );	// prevent removing or changing of memory cards
	
	void FlushAllDisks();	// force all files to be written to mounted memory cards

	bool PathIsMemCard( CString sDir ) const;
	
	CString GetName( PlayerNumber pn ) const;

protected:
	void AssignUnassignedCards();	// do our best to assign a Device to each player

	MemoryCardDriver *m_pDriver;

	vector<UsbStorageDevice> m_vStorageDevices;	// all currently connected

	bool	m_bCardsLocked;
	bool	m_bTooLate[NUM_PLAYERS];	// card was inserted after lock
	bool	m_bWriteError[NUM_PLAYERS];	// couldn't write to the card
	UsbStorageDevice m_Device[NUM_PLAYERS];	// device in the memory card slot, NULL if none

	RageSound m_soundReady;
	RageSound m_soundError;
	RageSound m_soundTooLate;
	RageSound m_soundDisconnect;
};

extern MemoryCardManager*	MEMCARDMAN;	// global and accessable from anywhere in our program

#endif
