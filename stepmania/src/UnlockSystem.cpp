/*
-----------------------------------------------------------------------------
 Class: UnlockSystem

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
	Andrew Wong
-----------------------------------------------------------------------------
*/

#include "global.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "song.h"
#include "Course.h"
#include "RageException.h"
#include "RageUtil.h"
#include "UnlockSystem.h"
#include "SongManager.h"
#include "GameState.h"
#include "IniFile.h"
#include "MsdFile.h"

#include <fstream>
using namespace std;

#include "stdio.h"

UnlockSystem::UnlockSystem()
{
	ArcadePoints = 0;
	DancePoints = 0;
	SongPoints = 0;
	ExtraClearPoints = 0;
	ExtraFailPoints = 0;
	ToastyPoints = 0;
	StagesCleared = 0;
	RouletteSeeds = "1";

	ReadValues("Data/MemCard.ini"); // in case its ever accessed, 
									// we want the values to be available
	WriteValues("Data/MemCard.ini");  // create if it does not exist
}

bool UnlockSystem::RouletteUnlock( const Song *song )
{
	SongEntry *p = FindSong( song );
	if (!p) return false;                       // does not exist
	if (p->m_iRouletteSeed == 0) return false;  // already unlocked

	RouletteSeeds[p->m_iRouletteSeed] = '1';
	WriteValues("Data/MemCard.ini");
	return true;
}

bool UnlockSystem::CourseIsLocked( const Course *course )
{
	// I know, its not a song, but for purposes of title
	// comparison, its the same thing.
	SongEntry *p = FindCourse( course );

	CString tmp;

	if (p)
	{
		p->isCourse = true;  // updates flag here
		p->updateLocked();

		if (!p->isLocked) tmp = "un";
	}
	
	return (p != NULL) && (p->isLocked);

}

bool UnlockSystem::SongIsLocked( const Song *song )
{
	SongEntry *p = FindSong( song );
	if( p == NULL )
		return false;

	p->updateLocked();

	LOG->Trace( "current status: %slocked", p->isLocked? "":"un" );
	
	return p->isLocked;
}

bool UnlockSystem::SongIsRoulette( const Song *song )
{
	SongEntry *p = FindSong( song );

	return p && (p->m_iRouletteSeed != 0) ;
}

SongEntry *UnlockSystem::FindSong( CString songname )
{
	for(unsigned i = 0; i < m_SongEntries.size(); i++)
		if (!songname.CompareNoCase(m_SongEntries[i].m_sSongName))
			return &m_SongEntries[i];

	return NULL;
}

SongEntry *UnlockSystem::FindCourse( const Course *pCourse )
{
	CString CourseName = pCourse->m_sTranslitName;

	for(unsigned i = 0; i < m_SongEntries.size(); i++)
	{
		if( !CourseName.CompareNoCase(m_SongEntries[i].m_sSongName) )
			return &m_SongEntries[i];
	}

	return NULL;
}

SongEntry *UnlockSystem::FindSong( const Song *pSong )
{
	/* Manual binary searches are a bad idea; they're insanely easy to get
	 * wrong.  This breaks the matching, anyway ... */
	/*
	int left = 0;
	int right = m_SongEntries.size() - 1;
	CString songtitle = pSong->GetFullTranslitTitle();

	songtitle.MakeUpper();

	while (left != right)
	{
		int mid = (left + right)/2;
		
		if (songtitle <= m_SongEntries[mid].m_sSongName )
			right = mid;
		else
			left = mid + 1;
	}


	if (m_SongEntries[left].GetSong() == pSong)
	{
		LOG->Trace("UnlockSystem: Retrieved: %s", pSong->GetFullTranslitTitle().c_str());
		return &m_SongEntries[left];
	}
		
	LOG->Trace("UnlockSystem: Failed to find %s", pSong->GetFullTranslitTitle().c_str());
	LOG->Trace("            (landed on %s)", m_SongEntries[left].m_sSongName.c_str() );
*/

	/* This should be good enough; it doesn't iterate over all installed songs.  (This
	 * is probably called for all songs, so that was probably n^2.) */
	for(unsigned i = 0; i < m_SongEntries.size(); i++)
	{
		if( pSong->Matches("", m_SongEntries[i].m_sSongName) )
			return &m_SongEntries[i];
//		if (m_SongEntries[i].GetSong() == pSong )
//			return &m_SongEntries[i];
	}

	return NULL;
}


SongEntry::SongEntry()
{
	m_fDancePointsRequired = 0;
	m_fArcadePointsRequired = 0;
	m_fSongPointsRequired = 0;
	m_fExtraStagesCleared = 0;
	m_fExtraStagesFailed = 0;
	m_fStagesCleared = 0;
	m_fToastysSeen = 0;
	m_iRouletteSeed = 0;

	ActualSong = NULL;

	isLocked = true;
	isCourse = false;
}


static bool CompareSongEntries(const SongEntry &se1, const SongEntry &se2)
{
	return se1.m_sSongName < se2.m_sSongName;
}

bool SongEntry::updateLocked()
{
	if (!(isLocked)) return true;  // if its already true
	
	UnlockSystem *UNLOCKS = GAMESTATE->m_pUnlockingSys;
	bool locked[8];
	for(int i=0; i < 8; i++)
		locked[i] = true;

	if (m_fArcadePointsRequired != 0)
		locked[0] = ( UNLOCKS->ArcadePoints < m_fArcadePointsRequired);

	if (m_fDancePointsRequired != 0)
		locked[1] = ( UNLOCKS->DancePoints < m_fDancePointsRequired);

	if (m_fSongPointsRequired != 0)
		locked[2] = ( UNLOCKS->SongPoints < m_fSongPointsRequired);

	if (m_fExtraStagesCleared != 0)
		locked[3] = ( UNLOCKS->ExtraClearPoints < m_fExtraStagesCleared);

	if (m_fExtraStagesFailed != 0)
		locked[4] = ( UNLOCKS->ExtraFailPoints < m_fExtraStagesFailed);

	if (m_fStagesCleared != 0)
		locked[5] = ( UNLOCKS->StagesCleared < m_fStagesCleared);

	if (m_fToastysSeen != 0)
		locked[6] = ( UNLOCKS->ToastyPoints < m_fToastysSeen);

	if (m_iRouletteSeed != 0)
	{
		CString tmp = UNLOCKS->RouletteSeeds;

		LOG->Trace("Seed in question: %d Roulette seeds: %s", 
			m_iRouletteSeed, tmp.c_str() );
		locked[7] = (tmp[m_iRouletteSeed] != '1');
	}

	isLocked = true;
	for(int j=0; j < 8; j++)
		isLocked = isLocked && locked[j];

	return !isLocked;
}

bool UnlockSystem::LoadFromDATFile( CString sPath )
{
	LOG->Trace( "UnlockSystem::LoadFromDATFile(%s)", sPath.c_str() );
	
	MsdFile msd;
	if( !msd.ReadFile( sPath ) )
	{
		LOG->Warn( "Error opening file '%s' for reading: %s.", sPath.c_str(), msd.GetError().c_str() );
		return false;
	}

	int MaxRouletteSlot = 0;
	unsigned i, j;

	for( i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		CString sValueName = sParams[0];

		if(iNumParams < 1)
		{
			LOG->Warn("Got \"%s\" tag with no parameters", sValueName.c_str());
			continue;
		}

		if( stricmp(sParams[0],"UNLOCK") )
		{
			LOG->Warn("Unrecognized unlock tag \"%s\", ignored.", sValueName.c_str());
			continue;
		}

		SongEntry current;
		current.m_sSongName = sParams[1];
		LOG->Trace("Song entry: %s", current.m_sSongName.c_str() );

		CStringArray UnlockTypes;
		split(sParams[2], ",", UnlockTypes);

		for( j=0; j<UnlockTypes.size(); ++j )
		{
			CStringArray readparam;

			split(UnlockTypes[j], "=", readparam);
			CString unlock_type = readparam[0];
			float datavalue = (float) atof(readparam[1]);

			LOG->Trace("UnlockTypes line: %s", UnlockTypes[j].c_str() );
			LOG->Trace("Unlock info: %s %f", unlock_type.c_str(), datavalue);

			if (unlock_type == "AP")
				current.m_fArcadePointsRequired = datavalue;
			if (unlock_type == "DP")
				current.m_fDancePointsRequired = datavalue;
			if (unlock_type == "SP")
				current.m_fSongPointsRequired = datavalue;
			if (unlock_type == "EC")
				current.m_fExtraStagesCleared = datavalue;
			if (unlock_type == "EF")
				current.m_fExtraStagesFailed = datavalue;
			if (unlock_type == "CS")
				current.m_fStagesCleared = datavalue;
			if (unlock_type == "!!")
				current.m_fToastysSeen = datavalue;
			if (unlock_type == "RO")
			{
				current.m_iRouletteSeed = (int)datavalue;
				MaxRouletteSlot = max( MaxRouletteSlot, (int) datavalue );
			}
		}
		current.updateLocked();

		current.ActualSong = SONGMAN->FindSong( "", current.m_sSongName );

		m_SongEntries.push_back(current);
	}

	InitRouletteSeeds(MaxRouletteSlot); // resize roulette seeds
	                  // for more efficient use of file

	// sort list so we can make use of binary searching
	sort( m_SongEntries.begin(), m_SongEntries.end(), CompareSongEntries );

	for(i=0; i < m_SongEntries.size(); i++)
	{
		CString tmp = "  ";
		if (!m_SongEntries[i].isLocked) tmp = "un";

		LOG->Trace( "UnlockSystem Entry %s", m_SongEntries[i].m_sSongName.c_str() );
		if (m_SongEntries[i].ActualSong != NULL)
			LOG->Trace( "          Translit %s", m_SongEntries[i].ActualSong->GetDisplayMainTitle().c_str() );
		LOG->Trace( "                AP %f", m_SongEntries[i].m_fArcadePointsRequired );
		LOG->Trace( "                DP %f", m_SongEntries[i].m_fDancePointsRequired );
		LOG->Trace( "                SP %f", m_SongEntries[i].m_fSongPointsRequired );
		LOG->Trace( "                CS %f", m_SongEntries[i].m_fStagesCleared );
		LOG->Trace( "                RO %i", m_SongEntries[i].m_iRouletteSeed );
		LOG->Trace( "            Status %slocked", tmp.c_str() );
		
	}
	
	return true;
}

bool SongEntry::SelectableWheel()
{
	return (!isLocked);  // cached
}

bool SongEntry::SelectableRoulette()
{
	if (!isLocked) return true;

	if (m_iRouletteSeed != 0) return true;
	return false;
}

float UnlockSystem::DancePointsUntilNextUnlock()
{
	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fDancePointsRequired > DancePoints)
			fSmallestPoints = min(fSmallestPoints, m_SongEntries[a].m_fDancePointsRequired);
	
	if (fSmallestPoints == 400000000) return 0;  // no match found
	return fSmallestPoints - DancePoints;
}

float UnlockSystem::ArcadePointsUntilNextUnlock()
{
	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fArcadePointsRequired > ArcadePoints)
			fSmallestPoints = min(fSmallestPoints, m_SongEntries[a].m_fArcadePointsRequired);
	
	if (fSmallestPoints == 400000000) return 0;  // no match found
	return fSmallestPoints - ArcadePoints;
}

float UnlockSystem::SongPointsUntilNextUnlock()
{
	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fSongPointsRequired > SongPoints )
			fSmallestPoints = min(fSmallestPoints, m_SongEntries[a].m_fSongPointsRequired);
	
	if (fSmallestPoints == 400000000) return 0;  // no match found
	return fSmallestPoints - SongPoints;
}

Song *SongEntry::GetSong() const
{
	return SONGMAN->FindSong( "", m_sSongName );
}

// This is mainly to streamline the INI for unnecessary values.
void UnlockSystem::InitRouletteSeeds(int MaxRouletteSlot)
{
	CString seeds = RouletteSeeds;
	MaxRouletteSlot++; // we actually need one more

	// have exactly the needed number of slots
	if (seeds.GetLength() == MaxRouletteSlot) return;

	if (seeds.GetLength() > MaxRouletteSlot)  // truncate value
	{
		// too many seeds
		seeds = seeds.Left(MaxRouletteSlot);
		RouletteSeeds = seeds;
		return;
	}

	// if we get here, the value isn't long enough
	while (seeds.GetLength() != MaxRouletteSlot)
		seeds += "0";

	RouletteSeeds = seeds;
}

bool UnlockSystem::ReadValues( CString filename)
{
	IniFile data;

	data.SetPath(filename);

	if (!data.ReadFile())
		return false;

	data.GetValueF( "Unlock", "ArcadePointsAccumulated",	ArcadePoints );
	data.GetValueF( "Unlock", "DancePointsAccumulated",		DancePoints );
	data.GetValueF( "Unlock", "SongPointsAccumulated",		SongPoints );
	data.GetValueF( "Unlock", "ExtraStagesCleared",			ExtraClearPoints );
	data.GetValueF( "Unlock", "ExtraStagesFailed",			ExtraFailPoints );
	data.GetValueF( "Unlock", "TotalStagesCleared",			StagesCleared );
	data.GetValueF( "Unlock", "TotalToastysSeen",			ToastyPoints );
	data.GetValue ( "Unlock", "RouletteSeeds",				RouletteSeeds );

	return true;
}


bool UnlockSystem::WriteValues( CString filename)
{
	IniFile data;

	data.SetPath(filename);

	data.SetValueF( "Unlock", "ArcadePointsAccumulated",	ArcadePoints );
	data.SetValueF( "Unlock", "DancePointsAccumulated",		DancePoints );
	data.SetValueF( "Unlock", "SongPointsAccumulated",		SongPoints );
	data.SetValueF( "Unlock", "ExtraStagesCleared",			ExtraClearPoints );
	data.SetValueF( "Unlock", "ExtraStagesFailed",			ExtraFailPoints );
	data.SetValueF( "Unlock", "TotalStagesCleared",			StagesCleared );
	data.SetValueF( "Unlock", "TotalToastysSeen",			ToastyPoints );
	data.SetValue ( "Unlock", "RouletteSeeds",				RouletteSeeds );

	data.WriteFile();

	return true;
}

float UnlockSystem::UnlockAddAP(float credit)
{
	ReadValues("Data/MemCard.ini");
	ArcadePoints += credit;
	WriteValues("Data/MemCard.ini");

	return ArcadePoints;
}

float UnlockSystem::UnlockAddAP(Grade credit)
{
	ReadValues("Data/MemCard.ini");
	if (credit != GRADE_E && credit != GRADE_D)
	ArcadePoints += 1;
	if (credit == GRADE_AAA)
		ArcadePoints += 9;
	WriteValues("Data/MemCard.ini");

	return ArcadePoints;
}

float UnlockSystem::UnlockAddDP(float credit)
{
	ReadValues("Data/MemCard.ini");
	DancePoints += credit;
	WriteValues("Data/MemCard.ini");

	return DancePoints;
}

float UnlockSystem::UnlockAddSP(float credit)
{
	ReadValues("Data/MemCard.ini");
	SongPoints += credit;
	WriteValues("Data/MemCard.ini");

	return SongPoints;
}

float UnlockSystem::UnlockAddSP(Grade credit)
{
	ReadValues("Data/MemCard.ini");
	const float SongPointsVals[NUM_GRADES] = { -1 /* unused */, 0, 1, 2, 3, 4, 5, 10, 20 };

	SongPoints += SongPointsVals[credit];
	WriteValues("Data/MemCard.ini");

	return SongPoints;
}

float UnlockSystem::UnlockClearExtraStage()
{
	ReadValues("Data/MemCard.ini");
	ExtraClearPoints++;
	WriteValues("Data/MemCard.ini");

	return ExtraClearPoints;
}

float UnlockSystem::UnlockFailExtraStage()
{
	ReadValues("Data/MemCard.ini");
	ExtraFailPoints++;
	WriteValues("Data/MemCard.ini");

	return ExtraFailPoints;
}

float UnlockSystem::UnlockClearStage()
{
	ReadValues("Data/MemCard.ini");
	StagesCleared++;
	WriteValues("Data/MemCard.ini");

	return StagesCleared;
}

float UnlockSystem::UnlockToasty()
{
	ReadValues("Data/MemCard.ini");
	ToastyPoints++;
	WriteValues("Data/MemCard.ini");

	return ToastyPoints;
}
