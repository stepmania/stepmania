/*
-----------------------------------------------------------------------------
 Class: UnlockSystem

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
-----------------------------------------------------------------------------
*/

#include "global.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageUtil.h"
#include "UnlockSystem.h"

#include <fstream>
#include <map>
using namespace std;

#define	POINTS_ACCUMULATED_BEFORE_LAST_ROUND	// Load from file here

UnlockSystem::UnlockSystem()
{
}


bool UnlockSystem::SongIsLocked( CString sSongName )
{
	sSongName.MakeUpper();	//Avoid case-sensitive problems
	for( unsigned i=0; i<m_SongEntries.size(); i++ )
	{
		if( sSongName == m_SongEntries[i].m_sSongName )
		{
			if( PREFSMAN->m_fDancePointsAccumulated >= m_SongEntries[i].m_fDancePointsRequired ) { LOG->Trace("   *This song is UNLOCKED"); return false; }
			else { LOG->Trace("   *This song is LOCKED"); return true; };
		}
		else { continue; };
	}

	LOG->Trace( "   *This song is UNLOCKED (wasn't locked in the first place)" );
	return false;
}








static int CompareSongEntries(const SongEntry &se1, const SongEntry &se2)
{
	return se1.m_fDancePointsRequired < se2.m_fDancePointsRequired;
}
void UnlockSystem::SortSongEntriesArray()
{
	sort( m_SongEntries.begin(), m_SongEntries.end(), CompareSongEntries );
}








bool UnlockSystem::LoadFromDATFile( CString sPath )
{
	LOG->Trace( "UnlockSystem::LoadFromDATFile(%s)", sPath.c_str() );
	
	ifstream input(sPath);
	if(input.bad())
	{
		LOG->Warn( "Error opening file '%s' for reading.", sPath.c_str() );
		return false;
	}

	string line;
	m_SongEntries.clear();

	while(input.good() && getline(input, line))
	{
		if(!line.compare(0, 2, "//"))	//Check for comments
			continue;

		/* "[data1] data2".  Ignore whitespace at the beginning of the line. */
		static Regex x("^ *\\[([^]]+)\\] *(.*)$");
		
		vector<CString> matches;
		if(!x.Compare(line, matches))
			continue;
		
		CString &sValueName = matches[0];
		CString &sValueData = matches[1];
		StripCrnl(sValueData);

		// Handle our data
		if( 0==stricmp(sValueName,"DP") ) // This means the DancePoints value is coming up
		{			
			float	DP;
			CString	SongName;
			DP = (float)atof( sValueData.Left(sValueData.Find("|",1)) );
			SongName = (CString)sValueData.Right( sValueData.GetLength() - (sValueData.Find("|",1)+1) );
			SongName.MakeUpper();	// Avoid case-sensitive problems

			SongEntry	SE;
				SE.m_fDancePointsRequired = DP;
				SE.m_sSongName = SongName.c_str();
				m_SongEntries.push_back( SE );
			continue;
		}	
	}
	return true;
}

float UnlockSystem::NumPointsUntilNextUnlock()
{
	float fSmallestPoints;
	fSmallestPoints = m_SongEntries[0].m_fDancePointsRequired;
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
	{
		if( m_SongEntries[a].m_fDancePointsRequired >= fSmallestPoints )
		{
			fSmallestPoints = m_SongEntries[a].m_fDancePointsRequired;
		}
	}
	
	float fResults = (fSmallestPoints - PREFSMAN->m_fDancePointsAccumulated);
	return fResults;
}