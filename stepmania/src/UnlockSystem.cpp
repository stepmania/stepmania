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

void UnlockSystem::RouletteUnlock( const Song *song )
{
	SongEntry *p = FindSong( song );
	if (!p)
		return;  // does not exist
	if (p->m_iRouletteSeed == 0)
		return;  // already unlocked

	RouletteSeeds[p->m_iRouletteSeed] = '1';
	WriteValues("Data/MemCard.ini");
}

bool UnlockSystem::CourseIsLocked( const Course *course )
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	// I know, its not a song, but for purposes of title
	// comparison, its the same thing.
	SongEntry *p = FindCourse( course );

	if (p)
		p->UpdateLocked();
	
	return p != NULL && p->isLocked;
}

bool UnlockSystem::SongIsLocked( const Song *song )
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	SongEntry *p = FindSong( song );
	if( p == NULL )
		return false;

	p->UpdateLocked();

	LOG->Trace( "current status: %slocked", p->isLocked? "":"un" );
	
	return p->isLocked;
}

bool UnlockSystem::SongIsRoulette( const Song *song )
{
	SongEntry *p = FindSong( song );

	return p && (p->m_iRouletteSeed != 0) ;
}

SongEntry *UnlockSystem::FindLockEntry( CString songname )
{
	for(unsigned i = 0; i < m_SongEntries.size(); i++)
		if (!songname.CompareNoCase(m_SongEntries[i].m_sSongName))
			return &m_SongEntries[i];

	return NULL;
}

SongEntry *UnlockSystem::FindSong( const Song *pSong )
{
	for(unsigned i = 0; i < m_SongEntries.size(); i++)
		if (m_SongEntries[i].m_pSong == pSong )
			return &m_SongEntries[i];

	return NULL;
}

SongEntry *UnlockSystem::FindCourse( const Course *pCourse )
{
	for(unsigned i = 0; i < m_SongEntries.size(); i++)
		if (m_SongEntries[i].m_pCourse== pCourse )
			return &m_SongEntries[i];

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

	m_pSong = NULL;
	m_pCourse = NULL;

	isLocked = true;
}


void SongEntry::UpdateLocked()
{
	if (!isLocked)
		return;
	
	const UnlockSystem *UNLOCKS = GAMESTATE->m_pUnlockingSys;

	isLocked = true;
	if ( m_fArcadePointsRequired && UNLOCKS->ArcadePoints >= m_fArcadePointsRequired )
		isLocked = false;

	if ( m_fDancePointsRequired && UNLOCKS->DancePoints >= m_fDancePointsRequired )
		isLocked = false;

	if ( m_fSongPointsRequired && UNLOCKS->SongPoints >= m_fSongPointsRequired )
		isLocked = false;

	if ( m_fExtraStagesCleared && UNLOCKS->ExtraClearPoints >= m_fExtraStagesCleared )
		isLocked = false;

	if ( m_fExtraStagesFailed && UNLOCKS->ExtraFailPoints >= m_fExtraStagesFailed )
		isLocked = false;

	if ( m_fStagesCleared && UNLOCKS->StagesCleared >= m_fStagesCleared )
		isLocked = false;

	if ( m_fToastysSeen && UNLOCKS->ToastyPoints >= m_fToastysSeen )
		isLocked = false;

	if ( m_iRouletteSeed )
	{
		const CString &tmp = UNLOCKS->RouletteSeeds;

		LOG->Trace("Seed in question: %d Roulette seeds: %s", m_iRouletteSeed, tmp.c_str() );
		if( tmp[m_iRouletteSeed] == '1' )
			isLocked = false;
	}
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
		current.UpdateLocked();

		m_SongEntries.push_back(current);
	}

	InitRouletteSeeds(MaxRouletteSlot); // resize roulette seeds
	                  // for more efficient use of file

	UpdateSongs();
	
	for(i=0; i < m_SongEntries.size(); i++)
	{
		CString tmp = "  ";
		if (!m_SongEntries[i].isLocked) tmp = "un";

		LOG->Trace( "UnlockSystem Entry %s", m_SongEntries[i].m_sSongName.c_str() );
		if (m_SongEntries[i].m_pSong != NULL)
			LOG->Trace( "          Translit %s", m_SongEntries[i].m_pSong->GetTranslitMainTitle().c_str() );
		LOG->Trace( "                AP %f", m_SongEntries[i].m_fArcadePointsRequired );
		LOG->Trace( "                DP %f", m_SongEntries[i].m_fDancePointsRequired );
		LOG->Trace( "                SP %f", m_SongEntries[i].m_fSongPointsRequired );
		LOG->Trace( "                CS %f", m_SongEntries[i].m_fStagesCleared );
		LOG->Trace( "                RO %i", m_SongEntries[i].m_iRouletteSeed );
		LOG->Trace( "            Status %slocked", tmp.c_str() );
		if (m_SongEntries[i].m_pSong)
			LOG->Trace( "                   Found matching song entry" );
		if (m_SongEntries[i].m_pCourse)
			LOG->Trace( "                   Found matching course entry" );

		
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

/* Update the song pointer.  Only call this when it's likely to have changed,
 * such as on load, or when a song title changes in the editor. */
void UnlockSystem::UpdateSongs()
{
	for( unsigned i = 0; i < m_SongEntries.size(); ++i )
	{
		m_SongEntries[i].m_pSong = NULL;
		m_SongEntries[i].m_pCourse = NULL;
		m_SongEntries[i].m_pSong = SONGMAN->FindSong( m_SongEntries[i].m_sSongName );
		if( m_SongEntries[i].m_pSong == NULL )
			m_SongEntries[i].m_pCourse = SONGMAN->FindCourse( m_SongEntries[i].m_sSongName );
	}
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

	// we don't want to ever take away dance points
	if (credit > 0) DancePoints += credit;
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

int UnlockSystem::GetNumUnlocks() const
{
	return m_SongEntries.size();
}