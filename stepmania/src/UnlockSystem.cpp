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
#include "RageUtil.h"
#include "UnlockSystem.h"
#include "SongManager.h"
#include "GameState.h"
#include "IniFile.h"
#include "MsdFile.h"

UnlockSystem*	UNLOCKSYS = NULL;	// global and accessable from anywhere in our program

#define UNLOCKS_PATH "Data/Unlocks.dat"

#define MEMCARD_PATH "Data/MemCard.ini"

UnlockSystem::UnlockSystem()
{
	UNLOCKSYS = this;

	LoadFromDATFile();

	ArcadePoints = 0;
	DancePoints = 0;
	SongPoints = 0;
	ExtraClearPoints = 0;
	ExtraFailPoints = 0;
	ToastyPoints = 0;
	StagesCleared = 0;
	RouletteSeeds = "1";

	ReadValues( MEMCARD_PATH ); // in case its ever accessed, 
									// we want the values to be available
	WriteValues( MEMCARD_PATH );  // create if it does not exist
}

void UnlockSystem::RouletteUnlock( const Song *song )
{
	// if its an extra stage, don't count it
	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		return;

	UnlockEntry *p = FindSong( song );
	if( !p )
		return;  // does not exist
	if( p->m_iRouletteSeed == 0 )
		return;  // already unlocked

	ASSERT( p->m_iRouletteSeed < (int) RouletteSeeds.size() );
	RouletteSeeds[p->m_iRouletteSeed] = '1';
	WriteValues( MEMCARD_PATH );
}

bool UnlockSystem::CourseIsLocked( const Course *course )
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	UnlockEntry *p = FindCourse( course );
	if( p == NULL )
		return false;

	p->UpdateLocked();
	return p->isLocked;
}

bool UnlockSystem::SongIsLocked( const Song *song )
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	UnlockEntry *p = FindSong( song );
	if( p == NULL )
		return false;

	p->UpdateLocked();

	LOG->Trace( "current status: %slocked", p->isLocked? "":"un" );
	
	return p->isLocked;
}

bool UnlockSystem::SongIsRoulette( const Song *song )
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindSong( song );

	return p && p->m_iRouletteSeed != 0;
}

UnlockEntry *UnlockSystem::FindLockEntry( CString songname )
{
	for( unsigned i = 0; i < m_SongEntries.size(); i++ )
		if( !songname.CompareNoCase(m_SongEntries[i].m_sSongName) )
			return &m_SongEntries[i];

	return NULL;
}

UnlockEntry *UnlockSystem::FindSong( const Song *pSong )
{
	for( unsigned i = 0; i < m_SongEntries.size(); i++ )
		if( m_SongEntries[i].m_pSong == pSong )
			return &m_SongEntries[i];

	return NULL;
}

UnlockEntry *UnlockSystem::FindCourse( const Course *pCourse )
{
	for(unsigned i = 0; i < m_SongEntries.size(); i++)
		if (m_SongEntries[i].m_pCourse== pCourse )
			return &m_SongEntries[i];

	return NULL;
}


UnlockEntry::UnlockEntry()
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


void UnlockEntry::UpdateLocked()
{
	if( !isLocked )
		return;
	
	isLocked = true;
	if( m_fArcadePointsRequired && UNLOCKSYS->ArcadePoints >= m_fArcadePointsRequired )
		isLocked = false;

	if( m_fDancePointsRequired && UNLOCKSYS->DancePoints >= m_fDancePointsRequired )
		isLocked = false;

	if( m_fSongPointsRequired && UNLOCKSYS->SongPoints >= m_fSongPointsRequired )
		isLocked = false;

	if( m_fExtraStagesCleared && UNLOCKSYS->ExtraClearPoints >= m_fExtraStagesCleared )
		isLocked = false;

	if( m_fExtraStagesFailed && UNLOCKSYS->ExtraFailPoints >= m_fExtraStagesFailed )
		isLocked = false;

	if( m_fStagesCleared && UNLOCKSYS->StagesCleared >= m_fStagesCleared )
		isLocked = false;

	if( m_fToastysSeen && UNLOCKSYS->ToastyPoints >= m_fToastysSeen )
		isLocked = false;

	if ( m_iRouletteSeed )
	{
		const CString &tmp = UNLOCKSYS->RouletteSeeds;

		LOG->Trace("Seed in question: %d Roulette seeds: %s", m_iRouletteSeed, tmp.c_str() );
		if( tmp[m_iRouletteSeed] == '1' )
			isLocked = false;
	}
}

bool UnlockSystem::LoadFromDATFile()
{
	LOG->Trace( "UnlockSystem::LoadFromDATFile()" );
	
	MsdFile msd;
	if( !msd.ReadFile( UNLOCKS_PATH ) )
	{
		LOG->Warn( "Error opening file '%s' for reading: %s.", UNLOCKS_PATH, msd.GetError().c_str() );
		return false;
	}

	int MaxRouletteSlot = 0;
	unsigned i, j;

	for( i=0; i<msd.GetNumValues(); i++ )
	{
		int iNumParams = msd.GetNumParams(i);
		const MsdFile::value_t &sParams = msd.GetValue(i);
		CString sValueName = sParams[0];

		if( iNumParams < 1 )
		{
			LOG->Warn("Got \"%s\" tag with no parameters", sValueName.c_str());
			continue;
		}

		if( stricmp(sParams[0],"UNLOCK") )
		{
			LOG->Warn("Unrecognized unlock tag \"%s\", ignored.", sValueName.c_str());
			continue;
		}

		UnlockEntry current;
		current.m_sSongName = sParams[1];
		LOG->Trace("Song entry: %s", current.m_sSongName.c_str() );

		CStringArray UnlockTypes;
		split( sParams[2], ",", UnlockTypes );

		for( j=0; j<UnlockTypes.size(); ++j )
		{
			CStringArray readparam;

			split(UnlockTypes[j], "=", readparam);
			CString unlock_type = readparam[0];
			float datavalue = (float) atof(readparam[1]);

			LOG->Trace("UnlockTypes line: %s", UnlockTypes[j].c_str() );
			LOG->Trace("Unlock info: %s %f", unlock_type.c_str(), datavalue);

			if( unlock_type == "AP" )
				current.m_fArcadePointsRequired = datavalue;
			if( unlock_type == "DP" )
				current.m_fDancePointsRequired = datavalue;
			if( unlock_type == "SP" )
				current.m_fSongPointsRequired = datavalue;
			if( unlock_type == "EC" )
				current.m_fExtraStagesCleared = datavalue;
			if( unlock_type == "EF" )
				current.m_fExtraStagesFailed = datavalue;
			if( unlock_type == "CS" )
				current.m_fStagesCleared = datavalue;
			if( unlock_type == "!!" )
				current.m_fToastysSeen = datavalue;
			if( unlock_type == "RO" )
			{
				current.m_iRouletteSeed = (int)datavalue;
				MaxRouletteSlot = max( MaxRouletteSlot, (int) datavalue );
			}
		}
		current.UpdateLocked();

		m_SongEntries.push_back(current);
	}

	InitRouletteSeeds( MaxRouletteSlot ); // resize roulette seeds for more efficient use of file

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

float UnlockSystem::DancePointsUntilNextUnlock()
{
	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fDancePointsRequired > DancePoints)
			fSmallestPoints = min(fSmallestPoints, m_SongEntries[a].m_fDancePointsRequired);
	
	if( fSmallestPoints == 400000000 )
		return 0;  // no match found
	return fSmallestPoints - DancePoints;
}

float UnlockSystem::ArcadePointsUntilNextUnlock()
{
	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fArcadePointsRequired > ArcadePoints)
			fSmallestPoints = min(fSmallestPoints, m_SongEntries[a].m_fArcadePointsRequired);
	
	if( fSmallestPoints == 400000000 )
		return 0;  // no match found
	return fSmallestPoints - ArcadePoints;
}

float UnlockSystem::SongPointsUntilNextUnlock()
{
	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fSongPointsRequired > SongPoints )
			fSmallestPoints = min(fSmallestPoints, m_SongEntries[a].m_fSongPointsRequired);
	
	if( fSmallestPoints == 400000000 )
		return 0;  // no match found
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

		// display warning on invalid song entry
		if (m_SongEntries[i].m_pSong   == NULL &&
			m_SongEntries[i].m_pCourse == NULL)
		{
			LOG->Warn("UnlockSystem::UpdateSongs(): Cannot find a "
			"matching entry for %s.\nPlease check the song title.  "
			"Song titles should include the title and song title, e.g. "
			"Can't Stop Fallin' In Love -Speed Mix-.",
			m_SongEntries[i].m_sSongName.c_str() );
			m_SongEntries.erase(m_SongEntries.begin() + i);
		}
	}
}

// This is mainly to streamline the INI for unnecessary values.
void UnlockSystem::InitRouletteSeeds(int MaxRouletteSlot)
{
	MaxRouletteSlot++; // we actually need one more

	// Truncate the value if we have too many seeds:
	if ( (int)RouletteSeeds.size() > MaxRouletteSlot )
		RouletteSeeds = RouletteSeeds.Left( MaxRouletteSlot );

	// Lengthen the value if we have too few seeds:
	while ( (int)RouletteSeeds.size() < MaxRouletteSlot )
		RouletteSeeds += "0";
}

bool UnlockSystem::ReadValues( CString filename)
{
	IniFile data;

	data.SetPath( filename );

	if( !data.ReadFile() )
		return false;

	data.GetValue ( "Unlock", "ArcadePointsAccumulated",	ArcadePoints );
	data.GetValue ( "Unlock", "DancePointsAccumulated",		DancePoints );
	data.GetValue ( "Unlock", "SongPointsAccumulated",		SongPoints );
	data.GetValue ( "Unlock", "ExtraStagesCleared",			ExtraClearPoints );
	data.GetValue ( "Unlock", "ExtraStagesFailed",			ExtraFailPoints );
	data.GetValue ( "Unlock", "TotalStagesCleared",			StagesCleared );
	data.GetValue ( "Unlock", "TotalToastysSeen",			ToastyPoints );
	data.GetValue ( "Unlock", "RouletteSeeds",				RouletteSeeds );

	return true;
}


bool UnlockSystem::WriteValues( CString filename)
{
	IniFile data;

	data.SetPath( filename );

	data.SetValue( "Unlock", "ArcadePointsAccumulated",		ArcadePoints );
	data.SetValue( "Unlock", "DancePointsAccumulated",		DancePoints );
	data.SetValue( "Unlock", "SongPointsAccumulated",		SongPoints );
	data.SetValue( "Unlock", "ExtraStagesCleared",			ExtraClearPoints );
	data.SetValue( "Unlock", "ExtraStagesFailed",			ExtraFailPoints );
	data.SetValue( "Unlock", "TotalStagesCleared",			StagesCleared );
	data.SetValue( "Unlock", "TotalToastysSeen",			ToastyPoints );
	data.SetValue( "Unlock", "RouletteSeeds",				RouletteSeeds );

	data.WriteFile();

	return true;
}

float UnlockSystem::UnlockAddAP(float credit)
{
	ReadValues( MEMCARD_PATH );
	ArcadePoints += credit;
	WriteValues( MEMCARD_PATH );

	return ArcadePoints;
}

float UnlockSystem::UnlockAddAP(Grade grade)
{
	ReadValues( MEMCARD_PATH );
	switch( grade )
	{
	case GRADE_FAILED:
		;	// no points
		break;
	case GRADE_TIER_1:
	case GRADE_TIER_2:
		ArcadePoints += 9;
		break;
	case GRADE_NO_DATA:
		ASSERT(0);
		break;
	default:
		ArcadePoints += 1;
		break;
	}
	WriteValues( MEMCARD_PATH );

	return ArcadePoints;
}

float UnlockSystem::UnlockAddDP(float credit)
{
	ReadValues( MEMCARD_PATH );

	// we don't want to ever take away dance points
	if( credit > 0 )
		DancePoints += credit;
	WriteValues( MEMCARD_PATH );

	return DancePoints;
}

float UnlockSystem::UnlockAddSP(float credit)
{
	ReadValues( MEMCARD_PATH );
	SongPoints += credit;
	WriteValues( MEMCARD_PATH );

	return SongPoints;
}

float UnlockSystem::UnlockAddSP( Grade grade )
{
	ReadValues( MEMCARD_PATH );

	// TODO: move these to PREFS
	switch( grade )
	{
	case GRADE_TIER_1:/*AAAA*/	SongPoints += 20;	break;
	case GRADE_TIER_2:/*AAA*/	SongPoints += 10;	break;
	case GRADE_TIER_3:/*AA*/	SongPoints += 5;	break;
	case GRADE_TIER_4:/*A*/		SongPoints += 4;	break;
	case GRADE_TIER_5:/*B*/		SongPoints += 3;	break;
	case GRADE_TIER_6:/*C*/		SongPoints += 2;	break;
	case GRADE_TIER_7:/*D*/		SongPoints += 1;	break;
	}

	WriteValues( MEMCARD_PATH );

	return SongPoints;
}

float UnlockSystem::UnlockClearExtraStage()
{
	ReadValues( MEMCARD_PATH );
	ExtraClearPoints++;
	WriteValues( MEMCARD_PATH );

	return ExtraClearPoints;
}

float UnlockSystem::UnlockFailExtraStage()
{
	ReadValues( MEMCARD_PATH );
	ExtraFailPoints++;
	WriteValues( MEMCARD_PATH );

	return ExtraFailPoints;
}

float UnlockSystem::UnlockClearStage()
{
	ReadValues( MEMCARD_PATH );
	StagesCleared++;
	WriteValues( MEMCARD_PATH );

	return StagesCleared;
}

float UnlockSystem::UnlockToasty()
{
	ReadValues( MEMCARD_PATH );
	ToastyPoints++;
	WriteValues( MEMCARD_PATH );

	return ToastyPoints;
}

int UnlockSystem::GetNumUnlocks() const
{
	return m_SongEntries.size();
}
