#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: SongUtil

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "SongUtil.h"
#include "song.h"
#include "Steps.h"
#include "GameState.h"
#include "StyleDef.h"
#include "ProfileManager.h"
#include "PrefsManager.h"
#include "SongManager.h"


/////////////////////////////////////
// Sorting
/////////////////////////////////////

/* Just calculating GetNumTimesPlayed within the sort is pretty slow, so let's precompute
 * it.  (This could be generalized with a template.) */
map<const Song*, CString> song_sort_val;

bool CompareSongPointersBySortValueAscending(const Song *pSong1, const Song *pSong2)
{
	return song_sort_val[pSong1] < song_sort_val[pSong2];
}

bool CompareSongPointersBySortValueDescending(const Song *pSong1, const Song *pSong2)
{
	return song_sort_val[pSong1] > song_sort_val[pSong2];
}


CString SongUtil::MakeSortString( CString s )
{
	s.MakeUpper();

	// Make sure that non-alphanumeric characters are placed at the very end
	if( s.size() > 0 )
	{
		if( s[0] == '.' )	// ".59"
			s.erase(s.begin());
		if( (s[0] < 'A' || s[0] > 'Z') && (s[0] < '0' || s[0] > '9') )
			s = char(126) + s;
	}

	return s;
}

bool CompareSongPointersByTitle(const Song *pSong1, const Song *pSong2)
{
	// Prefer transliterations to full titles
	CString s1 = pSong1->GetTranslitMainTitle();
	CString s2 = pSong2->GetTranslitMainTitle();
	if( s1 == s2 )
	{
		s1 = pSong1->GetTranslitSubTitle();
		s2 = pSong2->GetTranslitSubTitle();
	}

	s1 = SongUtil::MakeSortString(s1);
	s2 = SongUtil::MakeSortString(s2);

	int ret = s1.CompareNoCase( s2 );
	if(ret < 0) return true;
	if(ret > 0) return false;

	/* The titles are the same.  Ensure we get a consistent ordering
	 * by comparing the unique SongFilePaths. */
	return pSong1->GetSongFilePath().CompareNoCase(pSong2->GetSongFilePath()) < 0;
}

void SongUtil::SortSongPointerArrayByTitle( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByTitle );
}

static int GetSongSortDifficulty(const Song *pSong)
{
	vector<Steps*> aNotes;
	pSong->GetSteps( aNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );

	/* Sort by the first difficulty found in the following order: */
	const Difficulty d[] = { DIFFICULTY_EASY, DIFFICULTY_MEDIUM, DIFFICULTY_HARD,
		DIFFICULTY_CHALLENGE, DIFFICULTY_INVALID };

	for(int i = 0; d[i] != DIFFICULTY_INVALID; ++i)
	{
		for( unsigned j = 0; j < aNotes.size(); ++j)
		{
			if(aNotes[j]->GetDifficulty() != d[i])
				continue;

			return aNotes[j]->GetMeter();
		}
	}

	return 1;
}

void SongUtil::SortSongPointerArrayByDifficulty( vector<Song*> &arraySongPointers )
{
	for( unsigned i = 0; i < arraySongPointers.size(); ++i )
		song_sort_val[arraySongPointers[i]] =
			ssprintf("%9i", GetSongSortDifficulty(arraySongPointers[i]));
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
}

bool CompareSongPointersByBPM(const Song *pSong1, const Song *pSong2)
{
	float fMinBPM1, fMaxBPM1, fMinBPM2, fMaxBPM2;
	pSong1->GetDisplayBPM( fMinBPM1, fMaxBPM1 );
	pSong2->GetDisplayBPM( fMinBPM2, fMaxBPM2 );

	if( fMaxBPM1 < fMaxBPM2 )
		return true;
	if( fMaxBPM1 > fMaxBPM2 )
		return false;
	
	return CompareCStringsAsc( pSong1->GetSongFilePath(), pSong2->GetSongFilePath() );
}

void SongUtil::SortSongPointerArrayByBPM( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByBPM );
}

void AppendOctal( int n, int digits, CString &out )
{
	for( int p = digits-1; p >= 0; --p )
	{
		const int shift = p*3;
		int n2 = (n >> shift) & 0x7;
		out.insert( out.end(), (char) (n2+'0') );
	}
}

bool CompDescending( const pair<Song *, CString> &a, const pair<Song *, CString> &b )
{ return a.second > b.second; }

void SongUtil::SortSongPointerArrayByGrade( vector<Song*> &arraySongPointers )
{
	/* Optimize by pre-writing a string to compare, since doing GetNumNotesWithGrade
	 * inside the sort is too slow. */
	typedef pair< Song *, CString > val;
	vector<val> vals;
	vals.reserve( arraySongPointers.size() );

	unsigned i;
	for( i = 0; i < arraySongPointers.size(); ++i )
	{
		Song *pSong = arraySongPointers[i];

		CString foo;
		foo.reserve(256);
		for( int g=GRADE_TIER_1; g<=GRADE_NO_DATA; ++g )
		{
			int n = pSong->GetNumNotesWithGrade( (Grade)g );
			AppendOctal( n, 3, foo );
		}
		vals.push_back( val(pSong, foo) );
	}

	sort( vals.begin(), vals.end(), CompDescending );

	for( i = 0; i < arraySongPointers.size(); ++i )
		arraySongPointers[i] = vals[i].first;
}


void SongUtil::SortSongPointerArrayByArtist( vector<Song*> &arraySongPointers )
{
	for( unsigned i = 0; i < arraySongPointers.size(); ++i )
		song_sort_val[arraySongPointers[i]] = MakeSortString( arraySongPointers[i]->GetTranslitArtist() );
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
}

static int CompareSongPointersByGroup(const Song *pSong1, const Song *pSong2)
{
	return pSong1->m_sGroupName < pSong2->m_sGroupName;
}

void SongUtil::SortSongPointerArrayByGroupAndDifficulty( vector<Song*> &arraySongPointers )
{
	SortSongPointerArrayByDifficulty( arraySongPointers );
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByGroup );
}

int CompareSongPointersByGroupAndTitle(const Song *pSong1, const Song *pSong2)
{
	const CString &sGroup1 = pSong1->m_sGroupName;
	const CString &sGroup2 = pSong2->m_sGroupName;

	if( sGroup1 < sGroup2 )
		return true;
	if( sGroup1 > sGroup2 )
		return false;

	/* Same group; compare by name. */
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SongUtil::SortSongPointerArrayByGroupAndTitle( vector<Song*> &arraySongPointers )
{
	sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersByGroupAndTitle );
}

void SongUtil::SortSongPointerArrayByNumPlays( vector<Song*> &arraySongPointers, ProfileSlot slot, bool bDescending )
{
	Profile* pProfile = PROFILEMAN->GetProfile(slot);
	if( pProfile == NULL )
		return;	// nothing to do since we don't have data
	SortSongPointerArrayByNumPlays( arraySongPointers, pProfile, bDescending );
}

void SongUtil::SortSongPointerArrayByNumPlays( vector<Song*> &arraySongPointers, const Profile* pProfile, bool bDescending )
{
	ASSERT( pProfile );
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
		song_sort_val[arraySongPointers[i]] = ssprintf("%9i", pProfile->GetSongNumTimesPlayed(arraySongPointers[i]));
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), bDescending ? CompareSongPointersBySortValueDescending : CompareSongPointersBySortValueAscending );
	song_sort_val.clear();
}

CString SongUtil::GetSectionNameFromSongAndSort( const Song* pSong, SortOrder so )
{
	if( pSong == NULL )
		return "";

	switch( so )
	{
	case SORT_PREFERRED:
		return "";
	case SORT_GROUP:	
		return pSong->m_sGroupName;
	case SORT_TITLE:
	case SORT_ARTIST:	
		{
			CString s;
			switch( so )
			{
			case SORT_TITLE:	s = pSong->GetTranslitMainTitle();	break;
			case SORT_ARTIST:	s = pSong->GetTranslitArtist();		break;
			default:	ASSERT(0);
			}
			s = MakeSortString(s);	// resulting string will be uppercase
			
			if( s.empty() )
				return "";
			else if( s[0] >= '0' && s[0] <= '9' )
				return "NUM";
			else if( s[0] < 'A' || s[0] > 'Z')
				return "OTHER";
			else
				return s.Left(1);
		}
	case SORT_BPM:
		{
			const int iBPMGroupSize = 20;
			float fMinBPM, fMaxBPM;
			pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
			int iMaxBPM = (int)fMaxBPM;
			iMaxBPM += iBPMGroupSize - (iMaxBPM%iBPMGroupSize) - 1;
			return ssprintf("%03d-%03d",iMaxBPM-(iBPMGroupSize-1), iMaxBPM);
		}
	case SORT_MOST_PLAYED:
		return "";
	case SORT_GRADE:
		{
			for( int i=GRADE_TIER_1; i<NUM_GRADES; ++i )
			{
				Grade g = (Grade)i;
				int iCount = pSong->GetNumNotesWithGrade( g );
				if( iCount > 0 )
					return ssprintf( "%4s x %d", GradeToThemedString(g).c_str(), iCount );
			}
			return GradeToThemedString( GRADE_NO_DATA );
		}
	case SORT_EASY_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_EASY);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_MEDIUM_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_MEDIUM);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_HARD_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_HARD);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_CHALLENGE_METER:
		{
			Steps* pNotes = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_CHALLENGE);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_SORT_MENU:
	case SORT_MODE_MENU:
		return "";
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
	default:
		ASSERT(0);
		return "";
	}
}

void SongUtil::SortSongPointerArrayBySectionName( vector<Song*> &arraySongPointers, SortOrder so )
{
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
	{
		CString val = GetSectionNameFromSongAndSort( arraySongPointers[i], so );

		/* Make sure NUM comes first and OTHER comes last. */
		if( val == "NUM" )			val = "0";
		else if( val == "OTHER" )	val = "2";
		else						val = "1" + val;

		song_sort_val[arraySongPointers[i]] = val;
	}

	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
	song_sort_val.clear();
}

void SongUtil::SortSongPointerArrayByMeter( vector<Song*> &arraySongPointers, Difficulty dc )
{
	song_sort_val.clear();
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
	{
		Steps* pNotes = arraySongPointers[i]->GetStepsByDifficulty( GAMESTATE->GetCurrentStyleDef()->m_StepsType, dc );
		song_sort_val[arraySongPointers[i]] = ssprintf("%i", pNotes ? pNotes->GetMeter() : 0);
	}
	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortValueAscending );
}


void SongID::FromSong( const Song *p )
{
	if( p )
		sDir = p->GetSongDir();
	else
		sDir = "";
}

Song *SongID::ToSong() const
{
	return SONGMAN->GetSongFromDir( sDir );
}
