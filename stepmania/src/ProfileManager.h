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
	}

	bool LoadFromIni( CString sIniPath );
	bool SaveToIni( CString sIniPath );
	CString m_sName;
	CString m_sLastUsedHighScoreName;
};

class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

	bool CreateMachineProfile( CString sName );
	bool RenameMachineProfile( CString sProfileID, CString sNewName );
	bool DeleteMachineProfile( CString sProfileID );

	void GetMachineProfileIDs( vector<CString> &asProfileIDsOut );
	void GetMachineProfileNames( vector<CString> &asNamesOut );

	bool LoadDefaultProfileFromMachine( PlayerNumber pn );
	bool LoadProfileFromMemoryCard( PlayerNumber pn );
	bool SaveProfile( PlayerNumber pn );
	void UnloadProfile( PlayerNumber pn );

	bool IsUsingProfile( PlayerNumber pn ) { return !m_sProfileDir[pn].empty(); }
	Profile* GetProfile( PlayerNumber pn );


	bool IsMemoryCardInserted( PlayerNumber pn );
	bool IsUsingMemoryCard( PlayerNumber pn ) { return m_bUsingMemoryCard[pn]; }

private:
	// Directory that contains the profile.  Either on local machine or
	// on a memory card.
	CString m_sProfileDir[NUM_PLAYERS];

	bool m_bUsingMemoryCard[NUM_PLAYERS];
	
	// actual loaded profile data
	Profile	m_Profile[NUM_PLAYERS];	
};


extern ProfileManager*	PROFILEMAN;	// global and accessable from anywhere in our program

#endif
