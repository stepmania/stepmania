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

UnlockSystem*	UNLOCKMAN = NULL;	// global and accessable from anywhere in our program

#define UNLOCKS_PATH "Data/Unlocks.dat"
#define MEMCARD_PATH "Data/MemCard.ini"

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

	LoadFromDATFile();

	memset( m_fScores, 0, sizeof(m_fScores) );

	ReadValues(); // in case its ever accessed, 
									// we want the values to be available
	WriteValues();  // create if it does not exist
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

bool UnlockSystem::CourseIsLocked( const Course *course )
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	UnlockEntry *p = FindCourse( course );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

bool UnlockSystem::SongIsLocked( const Song *song )
{
	if( !PREFSMAN->m_bUseUnlockSystem )
		return false;

	UnlockEntry *p = FindSong( song );
	if( p == NULL )
		return false;

	return p->IsLocked();
}

/* Return true if the song is available in roulette (overriding #SELECTABLE). */
bool UnlockSystem::SongIsRoulette( const Song *song )
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
	memset( m_fRequired, 0, sizeof(m_fRequired) );
	m_iCode = -1;

	m_pSong = NULL;
	m_pCourse = NULL;
}


bool UnlockEntry::IsLocked() const
{
	for( int i = 0; i < NUM_UNLOCK_TYPES; ++i )
		if( m_fRequired[i] && UNLOCKMAN->m_fScores[i] >= m_fRequired[i] )
			return false;

	if( m_iCode != -1 && UNLOCKMAN->m_UnlockedCodes.find(m_iCode) != UNLOCKMAN->m_UnlockedCodes.end() )
		return false;

	return true;
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

		for( j=0; j<UnlockTypes.size(); ++j )
		{
			CStringArray readparam;

			split(UnlockTypes[j], "=", readparam);
			CString unlock_type = readparam[0];
			float datavalue = (float) atof(readparam[1]);

			LOG->Trace("UnlockTypes line: %s", UnlockTypes[j].c_str() );
			LOG->Trace("Unlock info: %s %f", unlock_type.c_str(), datavalue);

			if( unlock_type == "AP" )
				current.m_fRequired[UNLOCK_ARCADE_POINTS] = datavalue;
			if( unlock_type == "DP" )
				current.m_fRequired[UNLOCK_DANCE_POINTS] = datavalue;
			if( unlock_type == "SP" )
				current.m_fRequired[UNLOCK_SONG_POINTS] = datavalue;
			if( unlock_type == "EC" )
				current.m_fRequired[UNLOCK_EXTRA_CLEARED] = datavalue;
			if( unlock_type == "EF" )
				current.m_fRequired[UNLOCK_EXTRA_CLEARED] = datavalue;
			if( unlock_type == "!!" )
				current.m_fRequired[UNLOCK_TOASTY] = datavalue;
			if( unlock_type == "CS" )
				current.m_fRequired[UNLOCK_CLEARED] = datavalue;
			if( unlock_type == "CODE" )
				current.m_iCode = (int) datavalue;
			if( unlock_type == "RO" )
			{
				m_UnlockedCodes.insert( (int)datavalue );
				m_RouletteCodes.insert( (int)datavalue );
			}
		}

		m_SongEntries.push_back(current);
	}

	UpdateSongs();
	
	for(i=0; i < m_SongEntries.size(); i++)
	{
		CString tmp = "  ";
		if (!m_SongEntries[i].IsLocked()) tmp = "un";

		LOG->Trace( "UnlockSystem Entry %s", m_SongEntries[i].m_sSongName.c_str() );
		if (m_SongEntries[i].m_pSong != NULL)
			LOG->Trace( "          Translit %s", m_SongEntries[i].m_pSong->GetTranslitMainTitle().c_str() );
		LOG->Trace( "                AP %f", m_SongEntries[i].m_fRequired[UNLOCK_ARCADE_POINTS] );
		LOG->Trace( "                DP %f", m_SongEntries[i].m_fRequired[UNLOCK_DANCE_POINTS] );
		LOG->Trace( "                SP %f", m_SongEntries[i].m_fRequired[UNLOCK_SONG_POINTS] );
		LOG->Trace( "                CS %f", m_SongEntries[i].m_fRequired[UNLOCK_CLEARED] );
		LOG->Trace( "              CODE %i", m_SongEntries[i].m_iCode );
		LOG->Trace( "            Status %slocked", tmp.c_str() );
		if (m_SongEntries[i].m_pSong)
			LOG->Trace( "                   Found matching song entry" );
		if (m_SongEntries[i].m_pCourse)
			LOG->Trace( "                   Found matching course entry" );
	}
	
	return true;
}

float UnlockSystem::PointsUntilNextUnlock( UnlockType t ) const
{
	float fSmallestPoints = 400000000;   // or an arbitrarily large value
	for( unsigned a=0; a<m_SongEntries.size(); a++ )
		if( m_SongEntries[a].m_fRequired[t] > m_fScores[t] )
			fSmallestPoints = min( fSmallestPoints, m_SongEntries[a].m_fRequired[t] );
	
	if( fSmallestPoints == 400000000 )
		return 0;  // no match found
	return fSmallestPoints - m_fScores[t];
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

bool UnlockSystem::ReadValues()
{
	IniFile data;

	data.SetPath( MEMCARD_PATH );

	if( !data.ReadFile() )
		return false;

	for( int i = 0; i < NUM_UNLOCK_TYPES; ++i )
		data.GetValue( "Unlock", g_UnlockNames[i],		m_fScores[i] );

	m_UnlockedCodes.clear();
	CString s;
	if( data.GetValue( "Unlock", "UnlockedCodes", s ) )
	{
		vector<CString> sCodes;
		split( s, ",", sCodes, true );
		for( unsigned c = 0; c < sCodes.size(); ++c )
		{
			const int n = atoi( sCodes[c] );
			m_UnlockedCodes.insert( n );
		}
	}

	return true;
}


bool UnlockSystem::WriteValues() const
{
	IniFile data;

	data.SetPath( MEMCARD_PATH );

	for( int i = 0; i < NUM_UNLOCK_TYPES; ++i )
		data.SetValue( "Unlock", g_UnlockNames[i],		m_fScores[i] );

	vector<CString> sCodes;
	for( set<int>::const_iterator it = m_UnlockedCodes.begin(); it != m_UnlockedCodes.end(); ++it )
		sCodes.push_back( ssprintf("%i", *it) );

	const CString list = join( ",", sCodes );
	data.SetValue( "Unlock", "UnlockedCodes", list );

	data.WriteFile();

	return true;
}

void UnlockSystem::UnlockAddAP(float credit)
{
	ReadValues();
	m_fScores[UNLOCK_ARCADE_POINTS] += credit;
	WriteValues();
}

void UnlockSystem::UnlockAddAP(Grade grade)
{
	ReadValues();
	switch( grade )
	{
	case GRADE_FAILED:
		;	// no points
		break;
	case GRADE_TIER_1:
	case GRADE_TIER_2:
		m_fScores[UNLOCK_ARCADE_POINTS] += 9;
		break;
	case GRADE_NO_DATA:
		ASSERT(0);
		break;
	default:
		m_fScores[UNLOCK_ARCADE_POINTS] += 1;
		break;
	}
	WriteValues();
}

void UnlockSystem::UnlockAddDP(float credit)
{
	ReadValues();

	// we don't want to ever take away dance points
	if( credit > 0 )
		m_fScores[UNLOCK_DANCE_POINTS] += credit;
	WriteValues();
}

void UnlockSystem::UnlockAddSP(float credit)
{
	ReadValues();
	m_fScores[UNLOCK_SONG_POINTS] += credit;
	WriteValues();
}

void UnlockSystem::UnlockAddSP( Grade grade )
{
	ReadValues();

	// TODO: move these to PREFS
	switch( grade )
	{
	case GRADE_TIER_1:/*AAAA*/	m_fScores[UNLOCK_SONG_POINTS] += 20;	break;
	case GRADE_TIER_2:/*AAA*/	m_fScores[UNLOCK_SONG_POINTS] += 10;	break;
	case GRADE_TIER_3:/*AA*/	m_fScores[UNLOCK_SONG_POINTS] += 5;	break;
	case GRADE_TIER_4:/*A*/		m_fScores[UNLOCK_SONG_POINTS] += 4;	break;
	case GRADE_TIER_5:/*B*/		m_fScores[UNLOCK_SONG_POINTS] += 3;	break;
	case GRADE_TIER_6:/*C*/		m_fScores[UNLOCK_SONG_POINTS] += 2;	break;
	case GRADE_TIER_7:/*D*/		m_fScores[UNLOCK_SONG_POINTS] += 1;	break;
	}

	WriteValues();
}

void UnlockSystem::UnlockClearExtraStage()
{
	ReadValues();
	++m_fScores[UNLOCK_EXTRA_CLEARED];
	WriteValues();
}

void UnlockSystem::UnlockFailExtraStage()
{
	ReadValues();
	++m_fScores[UNLOCK_EXTRA_FAILED];
	WriteValues();
}

void UnlockSystem::UnlockClearStage()
{
	ReadValues();
	++m_fScores[UNLOCK_CLEARED];
	WriteValues();
}

void UnlockSystem::UnlockToasty()
{
	ReadValues();
	++m_fScores[UNLOCK_TOASTY];
	WriteValues();
}

void UnlockSystem::UnlockCode( int num )
{
	ReadValues();
	m_UnlockedCodes.insert( num );
	WriteValues();
}

int UnlockSystem::GetNumUnlocks() const
{
	return m_SongEntries.size();
}
