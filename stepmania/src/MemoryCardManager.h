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
#include "PlayerNumber.h"
#include "RageSound.h"
#include "arch/MemoryCard/MemoryCardDriver.h"

class MemoryCardManager
{
public:
	MemoryCardManager();
	~MemoryCardManager();

	void Update( float fDelta );

	enum CardState { ready, too_late, write_error, no_card };

	CardState GetCardState( PlayerNumber pn );
	
	CString GetOsMountDir( PlayerNumber pn );	// only valid when ready

	void LockCards( bool bLock );	// prevent removing or changing of memory cards

protected:
	void ReassignCards();	// do our best to assign a Device to each player

	MemoryCardDriver *m_pDriver;

	vector<UsbStorageDevice> m_vStorageDevices;	// all currently connected

	bool	m_bCardsLocked;
	bool	m_bTooLate[NUM_PLAYERS];	// card was inserted after lock
	bool	m_bWriteError[NUM_PLAYERS];	// couldn't write to the card
	UsbStorageDevice m_Device[NUM_PLAYERS];	// device in the memory card slot, NULL if none

	RageSound m_soundConnect;
	RageSound m_soundDisconnect;
};

extern MemoryCardManager*	MEMCARDMAN;	// global and accessable from anywhere in our program

#endif
