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
//		LOG->Trace( "current status: %slocked", tmp.c_str() );
	}
	
	return (p != NULL) && (p->isLocked);

}

bool UnlockSystem::SongIsLocked( const Song *song )
{
	SongEntry *p = FindSong( song );

	CString tmp;

	if (p)
	{
		p->updateLocked();

		if (!p->isLocked) tmp = "un";
		LOG->Trace( "current status: %slocked", tmp.c_str() );
	}
	
	return (p != NULL) && (p->isLocked);
}

bool UnlockSystem::SongIsRoulette( const Song *song )
{
	SongEntry *p = FindSong( song );
	
	CString item;

	if (p && (p->m_iRouletteSeed != 0))
		LOG->Trace("Item %s is roulettable.");
	else
		LOG->Trace("Item %s is not roulettable.");
	
	return p && (p->m_iRouletteSeed != 0) ;
}

SongEntry *UnlockSystem::FindSong( CString songname )
{
	songname.MakeUpper();
	for(int i = 0; i < m_SongEntries.size(); i++)
		if (songname == m_SongEntries[i].m_sSongName)
			return &m_SongEntries[i];

	return NULL;
}

SongEntry *UnlockSystem::FindCourse( const Course *pCourse )
{
	CString CourseName = pCourse->m_sTranslitName;

	CourseName.MakeUpper();

	for(unsigned i = 0; i < m_SongEntries.size(); i++)
	{

		if( CourseName == m_SongEntries[i].m_sSongName )
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

	isLocked = true;
	isCourse = false;
}


static bool CompareSongEntries(const SongEntry &se1, const SongEntry &se2)
{
	return se1.m_sSongName < se2.m_sSongName;
}

bool UnlockSystem::ParseRow(CString text, CString &type, float &qty,
							CString &songname)
{
	int pos = -1;
	int end = text.GetLength();  // sets a value in case | does not exist
	char unlock_type[4];
	char qty_text[20];

	while (text[pos] != '|' && pos < text[pos] != '\0')
	{
		pos++;
		if (text[pos] == '[') text[pos] = ' ';
		if (text[pos] == ']') text[pos] = ' ';;
		if (text[pos] == '|') 
		{
			end = pos;
			text[pos] = ' ';
		}
	}

	songname = text.Right(text.GetLength() - 1 - end);
	
	sscanf(text, "%s %s|", unlock_type, qty_text);

	type = unlock_type;
	qty = (float) atof(qty_text);

	LOG->Trace( "UnlockSystem Entry %s", text.c_str() );
	LOG->Trace( "             Title %s", songname.c_str() );
	LOG->Trace( "              Type %s", type.c_str() );
	LOG->Trace( "             Value %s", qty_text );


	return true;
}

bool SongEntry::updateLocked()
{
	if (!(isLocked)) return true;  // if its already true
	
	UnlockSystem *UNLOCKS = GAMESTATE->m_pUnlockingSys;

	if (m_fArcadePointsRequired != 0)
		isLocked = ( UNLOCKS->ArcadePoints < m_fArcadePointsRequired);

	if (m_fDancePointsRequired != 0)
		isLocked = ( UNLOCKS->DancePoints < m_fDancePointsRequired);

	if (m_fSongPointsRequired != 0)
		isLocked = ( UNLOCKS->SongPoints < m_fSongPointsRequired);

	if (m_fExtraStagesCleared != 0)
		isLocked = ( UNLOCKS->ExtraClearPoints < m_fExtraStagesCleared);

	if (m_fExtraStagesFailed != 0)
		isLocked = ( UNLOCKS->ExtraFailPoints < m_fExtraStagesFailed);

	if (m_fStagesCleared != 0)
		isLocked = ( UNLOCKS->StagesCleared < m_fStagesCleared);

	if (m_fToastysSeen != 0)
		isLocked = ( UNLOCKS->ToastyPoints < m_fToastysSeen);

	if (m_iRouletteSeed != 0)
	{
		CString tmp = UNLOCKS->RouletteSeeds;

		LOG->Trace("Seed in question: %d Roulette seeds: %s", 
			m_iRouletteSeed, tmp.c_str() );
		isLocked = (tmp[m_iRouletteSeed] != '1');

		if (isLocked)
			LOG->Trace("Song is currently LOCKED");
		else
			LOG->Trace("Song is currently UNLOCKED");
	}

	return !isLocked;
}

bool UnlockSystem::LoadFromDATFile( CString sPath )
{
	LOG->Trace( "UnlockSystem::LoadFromDATFile(%s)", sPath.c_str() );
	
	ifstream input;

	input.open(sPath);
	if(input.fail())
	{
		LOG->Warn( "Error opening file '%s' for reading.", sPath.c_str() );
		return false;
	}

	char line[256]; 
	CString unlock_type, song_title;
	float datavalue;
	int MaxRouletteSlot = 0;

//	m_SongEntries.clear();

	while(input.getline(line, 255))
	{
		if(line[0] == '/' && line[1] == '/')	//Check for comments
			continue;

		/* "[data1] data2".  Ignore whitespace at the beginning of the line. */
		if (!ParseRow(line, unlock_type, datavalue, song_title))
			continue;

		SongEntry current;

		song_title.MakeUpper();	//Avoid case-sensitive problems
		current.m_sSongName = song_title;
		
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
			if (datavalue > MaxRouletteSlot)
				MaxRouletteSlot = (int)datavalue;
		}
		current.updateLocked();
		m_SongEntries.push_back(current);
	}
	InitRouletteSeeds(MaxRouletteSlot); // resize roulette seeds
	                  // for more efficient use of data

	// sort list so we can make use of binary searching
	sort( m_SongEntries.begin(), m_SongEntries.end(), CompareSongEntries );

	for(unsigned i=0; i < m_SongEntries.size(); i++)
	{
		CString tmp = "  ";
		if (!m_SongEntries[i].isLocked) tmp = "un";
		LOG->Trace( "UnlockSystem entry: %s", m_SongEntries[i].m_sSongName.c_str() );
		LOG->Trace( "Initial status:%slocked", tmp.c_str() );
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

void UnlockSystem::DebugPrint()
{
	for(unsigned i=0; i < m_SongEntries.size(); i++)
	{
		CString tmp = "  ";
		if (!m_SongEntries[i].isLocked) tmp = "un";
		LOG->Trace( "UnlockSystem entry %d: %s",i, m_SongEntries[i].m_sSongName.c_str() );
		LOG->Trace( "Status:%slocked", tmp.c_str() );
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
	switch (credit)
	{
	case GRADE_AAAA:
		SongPoints += 20;
		break;
	case GRADE_AAA:
		SongPoints += 10;
		break;
	case GRADE_AA:
		SongPoints += 5;
		break;
	case GRADE_A:
		SongPoints += 4;
		break;
	case GRADE_B:
		SongPoints += 3;
		break;
	case GRADE_C:
		SongPoints += 2;
		break;
	case GRADE_D:
		SongPoints += 1;
	}
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
