#ifndef ProfileManager_H
#define ProfileManager_H
/*
-----------------------------------------------------------------------------
 Class: ProfileManager

 Desc: Interface to profiles that exist on the machine or a memory card
	plugged into the machine.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "PlayerNumber.h"

struct Profile
{
	Profile() { Init(); }
	void Init()
	{
		m_sName = "";
		m_sLastUsedHighScoreName = "";
		m_bUsingProfileDefaultModifiers = false;
		m_sDefaultModifiers = "";
	}

	bool LoadFromIni( CString sIniPath );
	bool SaveToIni( CString sIniPath );
	CString m_sName;
	CString m_sLastUsedHighScoreName;
	bool m_bUsingProfileDefaultModifiers;
	CString m_sDefaultModifiers;
};

class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

	bool CreateMachineProfile( CString sName );
	bool RenameMachineProfile( CString sProfileID, CString sNewName );
	bool DeleteMachineProfile( CString sProfileID );

	bool CreateMemoryCardProfile( CString sName );

	void GetMachineProfileIDs( vector<CString> &asProfileIDsOut );
	void GetMachineProfileNames( vector<CString> &asNamesOut );

	bool LoadFirstAvailableProfile( PlayerNumber pn );
	bool IsMemoryCardInserted( PlayerNumber pn );
	bool SaveProfile( PlayerNumber pn );
	void UnloadProfile( PlayerNumber pn );

	bool IsUsingProfile( PlayerNumber pn ) { return !m_sProfileDir[pn].empty(); }
	Profile* GetProfile( PlayerNumber pn );


	bool IsUsingMemoryCard( PlayerNumber pn ) { return m_bUsingMemoryCard[pn]; }

private:
	bool LoadDefaultProfileFromMachine( PlayerNumber pn );
	bool LoadProfileFromMemoryCard( PlayerNumber pn );
	bool LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard );
	bool CreateProfile( CString sProfileDir, CString sName );

	// Directory that contains the profile.  Either on local machine or
	// on a memory card.
	CString m_sProfileDir[NUM_PLAYERS];

	bool m_bUsingMemoryCard[NUM_PLAYERS];
	
	// actual loaded profile data
	Profile	m_Profile[NUM_PLAYERS];	
};


extern ProfileManager*	PROFILEMAN;	// global and accessable from anywhere in our program

#endif
