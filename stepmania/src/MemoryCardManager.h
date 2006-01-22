#ifndef MemoryCardManager_H
#define MemoryCardManager_H

#include "GameConstantsAndTypes.h"	// for MemoryCardState
#include "PlayerNumber.h"
#include "RageSound.h"
#include "arch/MemoryCard/MemoryCardDriver.h"
#include "Preference.h"


extern const RString MEM_CARD_MOUNT_POINT[NUM_PLAYERS];

class MemoryCardManager
{
public:
	MemoryCardManager();
	~MemoryCardManager();

	void Update( float fDelta );

	MemoryCardState GetCardState( PlayerNumber pn ) const { return m_State[pn]; }
	RString GetCardError( PlayerNumber pn ) const { return m_sError[pn]; }
	
	void LockCards();	// prevent removing or changing of memory cards
	void UnlockCards();
	bool MountCard( PlayerNumber pn, int iTimeout = 10 );
	bool MountCard( PlayerNumber pn, const UsbStorageDevice &d );
	void UnmountCard( PlayerNumber pn );

	bool IsMounted( PlayerNumber pn ) const { return m_bMounted[pn]; }

	/* When paused, no changes in memory card state will be noticed until unpaused. */
	void PauseMountingThread( int iTimeout = 20 );
	void UnPauseMountingThread();
	
	bool GetCardsLocked() const { return m_bCardsLocked; }

	bool PathIsMemCard( RString sDir ) const;

	bool IsNameAvailable( PlayerNumber pn ) const;
	RString GetName( PlayerNumber pn ) const;

	const vector<UsbStorageDevice> &GetStorageDevices() { return m_vStorageDevices; }

	static Preference1D<RString>	m_sMemoryCardOsMountPoint;
	static Preference1D<int>		m_iMemoryCardUsbBus;
	static Preference1D<int>		m_iMemoryCardUsbPort;
	static Preference1D<int>		m_iMemoryCardUsbLevel;

	static Preference<RString>	m_sEditorMemoryCardOsMountPoint;	

protected:
	void CheckStateChanges();

	vector<UsbStorageDevice> m_vStorageDevices;	// all currently connected

	bool	m_bCardsLocked;
	bool	m_bMounted[NUM_PLAYERS];	// card is currently mounted

	UsbStorageDevice m_Device[NUM_PLAYERS];	// device in the memory card slot, blank if none
	UsbStorageDevice m_FinalDevice[NUM_PLAYERS];	// device in the memory card slot when we finalized, blank if none

	MemoryCardState m_State[NUM_PLAYERS];
	RString m_sError[NUM_PLAYERS]; // if MemoryCardState_Error

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
