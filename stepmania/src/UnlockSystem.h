#ifndef UNLOCK_SYSTEM_H
#define UNLOCK_SYSTEM_H
/*
-----------------------------------------------------------------------------
 Class: UnlockSystem

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
-----------------------------------------------------------------------------
*/
struct SongEntry
{
	float	m_fDancePointsRequired;	// Ammount of Dance Points needed to unlock this song
	CString m_sSongName;	/* Name of the song in the DWI/SM file itself.. This allows
								for a lot easier compatibility since a lot of people's 
								song folders are named differantly, song names tend to
								be the same in the file.*/
};



class UnlockSystem
{
	public:
		UnlockSystem();
		float NumPointsUntilNextUnlock();
		bool SongIsLocked( CString sSongName );
		bool LoadFromDATFile( CString sPath );
		bool	m_bAllSongsAreUnlocked;	// Quick way to check if all songs are unlocked
		vector<SongEntry>		m_SongEntries;	// All locked songs are stored here
	
	private:
		void SortSongEntriesArray();
};
#endif
