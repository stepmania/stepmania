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
		m_sDisplayName = "";
		m_sLastUsedHighScoreName = "";
	}

	bool LoadFromIni( CString sIniPath );
	bool WriteToIni( CString sIniPath );
	CString m_sDisplayName;
	CString m_sLastUsedHighScoreName;
};

class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

	void CreateProfile( CString sDisplayName );

	bool DoesProfileExist( CString sProfile );

	void GetProfiles( vector<CString> &asProfilesOut );
	void GetProfileDisplayNames( vector<CString> &asNamesOut );

	void TryLoadProfile( PlayerNumber pn );
	void UnloadProfile( PlayerNumber pn );

	CString GetDisplayName( PlayerNumber pn );

	bool IsUsingProfile( PlayerNumber pn ) { return !m_sProfileDir[pn].empty(); }

	bool IsMemoryCardInserted( PlayerNumber pn ) { return false; }
	bool IsMemoryCardValid( PlayerNumber pn ) { return false; }
	bool IsUsingMemoryCard( PlayerNumber pn ) { return m_bUsingMemoryCard[pn]; }

private:
	CString m_sProfileDir[NUM_PLAYERS];
	bool m_bUsingMemoryCard[NUM_PLAYERS];

	Profile	m_Profile[NUM_PLAYERS];
};


extern ProfileManager*	PROFILEMAN;	// global and accessable from anywhere in our program

#endif
