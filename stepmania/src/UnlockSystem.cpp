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
#include "MsdFile.h"
#include "ProfileManager.h"

UnlockSystem*	UNLOCKMAN = NULL;	// global and accessable from anywhere in our program

#define UNLOCKS_PATH "Data/Unlocks.dat"

static const char *g_UnlockNames[NUM_UNLOCK_TYPES] =
{
	"ArcadePointsAccumulated",
	"DancePointsAccumulated",
	"SongPointsAccumulated",
	"ExtraStagesCleared",
	"ExtraStagesFailed",
	"TotalToastysSeen",
	"TotalStagesCleared"
};

UnlockSystem::UnlockSystem()
{
	UNLOCKMAN = this;

	Load();
}

void UnlockSystem::UnlockSong( const Song *song )
{
	const UnlockEntry *p = FindSong( song );
	if( !p )
		return;  // does not exist
	if( p->m_iCode == -1 )
		return;

	UnlockCode( p->m_iCode );
}

bool UnlockSystem::CourseIsLocked( const Course *course ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindCourse( course );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

bool UnlockSystem::SongIsLocked( const Song *song ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindSong( song );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

/* Return true if the song is available in roulette (overriding #SELECTABLE). */
bool UnlockSystem::SongIsRoulette( const Song *song ) const
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	const UnlockEntry *p = FindSong( song );
	if( !p )
		return false;

	/* If the song is locked by a code, and it's a roulette code, honor IsLocked. */
	if( p->m_iCode != -1 && 
		m_RouletteCodes.find( p->m_iCode ) != m_RouletteCodes.end() )
		return p->IsLocked();
	return true;
}

const UnlockEntry *UnlockSystem::FindLockEntry( CString songname ) const
{
	for( unsigned i = 0; i < m_SongEntries.size(); i++ )
		if( !songname.CompareNoCase(m_SongEntries[i].m_sSongName) )
			return &m_SongEntries[i];

	return NULL;
}

const UnlockEntry *UnlockSystem::FindSong( const Song *pSong ) const
{
	for( unsigned i = 0; i < m_SongEntries.size(); i++ )
		if( m_SongEntries[i].m_pSong == pSong )
			return &m_SongEntries[i];

	return NULL;
}

const UnlockEntry *UnlockSystem::FindCourse( const Course *pCourse ) const
{
	for(unsigned i = 0; i < m_SongEntries.size(); i++)
		if (m_SongEntries[i].m_pCourse== pCourse )
			return &m_SongEntries[i];

	return NULL;
}


UnlockEntry::UnlockEntry()
{
	memset( m_fRequired, 0, sizeof(m_fRequired) );
	m_iCode = -1;

	m_pSong = NULL;
	m_pCourse = NULL;
}

static float GetArcadePoints( const Profile *pProfile )
{
	float fAP =	0;

	FOREACH_Grade(g)
	{
		switch(g)
		{
		case GRADE_TIER_1:
		case GRADE_TIER_2:	fAP += 9 * pProfile->m_iNumSongsPassedByGrade[g]; break;
		default:			fAP += 1 * pProfile->m_iNumSongsPassedByGrade[g]; break;

		case GRADE_FAILED:
		case GRADE_NO_DATA:
			;	// no points
			break;
		}
	}

	FOREACH_PlayMode(pm)
	{
		switch(pm)
		{
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			fAP += pProfile->m_iNumSongsPlayedByPlayMode[pm];
			break;
		}

	}

	return fAP;
}

static float GetSongPoints( const Profile *pProfile )
{
	float fSP =	0;

	FOREACH_Grade(g)
	{
		switch( g )
		{
		case GRADE_TIER_1:/*AAAA*/	fSP += 20 * pProfile->m_iNumSongsPassedByGrade[g];	break;
		case GRADE_TIER_2:/*AAA*/	fSP += 10* pProfile->m_iNumSongsPassedByGrade[g];	break;
		case GRADE_TIER_3:/*AA*/	fSP += 5* pProfile->m_iNumSongsPassedByGrade[g];	break;
		case GRADE_TIER_4:/*A*/		fSP += 4* pProfile->m_iNumSongsPassedByGrade[g];	break;
		case GRADE_TIER_5:/*B*/		fSP += 3* pProfile->m_iNumSongsPassedByGrade[g];	break;
		case GRADE_TIER_6:/*C*/		fSP += 2* pProfile->m_iNumSongsPassedByGrade[g];	break;
		case GRADE_TIER_7:/*D*/		fSP += 1* pProfile->m_iNumSongsPassedByGrade[g];	break;
		case GRADE_FAILED:
		case GRADE_NO_DATA:
			;	// no points
			break;
		}
	}

	FOREACH_PlayMode(pm)
	{
		switch(pm)
		{
		case PLAY_MODE_NONSTOP:
		case PLAY_MODE_ONI:
		case PLAY_MODE_ENDLESS:
			fSP += pProfile->m_iNumSongsPlayedByPlayMode[pm];
			break;
		}

	}

	return fSP;
}

void UnlockSystem::GetPoints( const Profile *pProfile, float fScores[NUM_UNLOCK_TYPES] ) const
{
	fScores[UNLOCK_ARCADE_POINTS] = GetArcadePoints( pProfile );
	fScores[UNLOCK_SONG_POINTS] = GetSongPoints( pProfile );
	fScores[UNLOCK_DANCE_POINTS] = (float) pProfile->m_iTotalDancePoints;
	fScores[UNLOCK_CLEARED] = (float) pProfile->GetTotalNumSongsPassed();
}

bool UnlockEntry::IsLocked() const
{
	float fScores[NUM_UNLOCK_TYPES];
	UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	for( int i = 0; i < NUM_UNLOCK_TYPES; ++i )
		if( m_fRequired[i] && fScores[i] >= m_fRequired[i] )
			return false;

	if( m_iCode != -1 && PROFILEMAN->GetMachineProfile()->m_UnlockedSongs.find(m_iCode) != PROFILEMAN->GetMachineProfile()->m_UnlockedSongs.end() )
		return false;

	return true;
}

static const char *g_ShortUnlockNames[NUM_UNLOCK_TYPES] =
{
	"AP",
	"DP",
	"SP",
	"EC",
	"EF",
	"!!",
	"CS"
};

static UnlockType StringToUnlockType( const CString &s )
{
	for( int i = 0; i < NUM_UNLOCK_TYPES; ++i )
		if( g_ShortUnlockNames[i] == s )
			return (UnlockType) i;
	return UNLOCK_INVALID;
}

bool UnlockSystem::Load()
{
	LOG->Trace( "UnlockSystem::Load()" );
	
	if( !IsAFile(UNLOCKS_PATH) )
		return false;

	MsdFile msd;
	if( !msd.ReadFile( UNLOCKS_PATH ) )
	{
		LOG->Warn( "Error opening file '%s' for reading: %s.", UNLOCKS_PATH, msd.GetError().c_str() );
		return false;
	}

	unsigned i;

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

		if( !stricmp(sParams[0],"ROULETTE") )
		{
			for( unsigned j = 1; j < sParams.params.size(); ++j )
				m_RouletteCodes.insert( atoi(sParams[j]) );
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

		for( unsigned j=0; j<UnlockTypes.size(); ++j )
		{
			CStringArray readparam;

			split( UnlockTypes[j], "=", readparam );
			const CString &unlock_type = readparam[0];

			LOG->Trace("UnlockTypes line: %s", UnlockTypes[j].c_str() );

			const float val = (float) atof( readparam[1] );
			LOG->Trace("Unlock info: %s %f", unlock_type.c_str(), val);

			const UnlockType ut = StringToUnlockType( unlock_type );
			if( ut != UNLOCK_INVALID )
				current.m_fRequired[ut] = val;
			if( unlock_type == "CODE" )
				current.m_iCode = (int) val;
			if( unlock_type == "RO" )
			{
				current.m_iCode = (int)val;
				m_RouletteCodes.insert( (int)val );
			}
		}

		m_SongEntries.push_back(current);
	}

	UpdateSongs();

	for(i=0; i < m_SongEntries.size(); i++)
	{
		LOG->Trace( "UnlockSystem Entry %s", m_SongEntries[i].m_sSongName.c_str() );
		if (m_SongEntries[i].m_pSong != NULL)
			LOG->Trace( "          Translit %s", m_SongEntries[i].m_pSong->GetTranslitMainTitle().c_str() );
		for( int j = 0; j < NUM_UNLOCK_TYPES; ++j )
			if( m_SongEntries[i].m_fRequired[j] )
				LOG->Trace( "                %s %f", g_UnlockNames[j], m_SongEntries[i].m_fRequired[j] );

		LOG->Trace( "              CODE %i", m_SongEntries[i].m_iCode );
		LOG->Trace( "            Status %s", m_SongEntries[i].IsLocked()? "locked":"unlocked" );
		if( m_SongEntries[i].m_pSong )
			LOG->Trace( "                   Found matching song entry" );
		if( m_SongEntries[i].m_pCourse )
			LOG->Trace( "                   Found matching course entry" );
	}
	
	return true;
}

float UnlockSystem::PointsUntilNextUnlock( UnlockType t ) const
{
	float fScores[NUM_UNLOCK_TYPES];
	UNLOCKMAN->GetPoints( PROFILEMAN->GetMachineProfile(), fScores );

	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fRequired[t] > fScores[t] )
			fSmallestPoints = min( fSmallestPoints, m_SongEntries[a].m_fRequired[t] );
	
	if( fSmallestPoints == 400000000 )
		return 0;  // no match found
	return fSmallestPoints - fScores[t];
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
			LOG->Warn("Unlock: Cannot find a matching entry for \"%s\"", m_SongEntries[i].m_sSongName.c_str() );
			m_SongEntries.erase(m_SongEntries.begin() + i);
			--i;
		}
	}
}



void UnlockSystem::UnlockCode( int num )
{
	FOREACH_PlayerNumber( pn )
		if( PROFILEMAN->IsUsingProfile(pn) )
			PROFILEMAN->GetProfile(pn)->m_UnlockedSongs.insert( num );

	PROFILEMAN->GetMachineProfile()->m_UnlockedSongs.insert( num );
}

int UnlockSystem::GetNumUnlocks() const
{
	return m_SongEntries.size();
}
