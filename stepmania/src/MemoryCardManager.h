#ifndef MemoryCardManager_H
#define MemoryCardManager_H

#include "GameConstantsAndTypes.h"	// for MemoryCardState
#include "PlayerNumber.h"
#include "RageSound.h"
#include "arch/MemoryCard/MemoryCardDriver.h"


extern const CString MEM_CARD_MOUNT_POINT[NUM_PLAYERS];

class MemoryCardManager
{
public:
	MemoryCardManager();
	~MemoryCardManager();

	void Update( float fDelta );

	MemoryCardState GetCardState( PlayerNumber pn );
	
	void LockCards();	// prevent removing or changing of memory cards
	void UnlockCards();
	void TryMountAllCards();
	void MountUsedCard( PlayerNumber pn );
	void MountCard( PlayerNumber pn );
	void UnmountCard( PlayerNumber pn );
	
	void FlushAndReset();	// force all files to be flushed to mounted memory cards

	bool PathIsMemCard( CString sDir ) const;

	bool IsNameAvailable( PlayerNumber pn ) const;
	CString GetName( PlayerNumber pn ) const;

protected:
	void UpdateUnassignedCards();	// do our best to assign a Device to each player
	void PauseMountingThread();	// call this before mouting, reading, or writing to memory card
	void UnPauseMountingThread();	// call this when done mouting, reading, or writing to memory card

	MemoryCardDriver *m_pDriver;

	vector<UsbStorageDevice> m_vStorageDevices;	// all currently connected

	bool	m_bCardsLocked;
	bool	m_bTooLate[NUM_PLAYERS];	// card was inserted after lock
	bool	m_bMounted[NUM_PLAYERS];	// card is currently mounted
	UsbStorageDevice m_Device[NUM_PLAYERS];	// device in the memory card slot, NULL if none

	RageSound m_soundReady;
	RageSound m_soundError;
	RageSound m_soundTooLate;
	RageSound m_soundDisconnect;
};

extern MemoryCardManager*	MEMCARDMAN;	// global and accessable from anywhere in our program

#endif

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
