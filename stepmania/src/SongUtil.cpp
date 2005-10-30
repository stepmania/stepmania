#include "global.h"
#include "SongUtil.h"
#include "song.h"
#include "Steps.h"
#include "GameState.h"
#include "Style.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "XmlFile.h"
#include "Foreach.h"
#include "UnlockManager.h"
#include "ThemeMetric.h"

static ThemeMetric<CString> SORT_NOT_AVAILABLE( "Sort", "NotAvailable" );
static ThemeMetric<CString> SORT_OTHER        ( "Sort", "Other" );

/////////////////////////////////////
// Sorting
/////////////////////////////////////

/* Just calculating GetNumTimesPlayed within the sort is pretty slow, so let's precompute
 * it.  (This could be generalized with a template.) */
static map<const Song*, CString> g_mapSongSortVal;

static bool CompareSongPointersBySortValueAscending( const Song *pSong1, const Song *pSong2 )
{
	return g_mapSongSortVal[pSong1] < g_mapSongSortVal[pSong2];
}

static bool CompareSongPointersBySortValueDescending( const Song *pSong1, const Song *pSong2 )
{
	return g_mapSongSortVal[pSong1] > g_mapSongSortVal[pSong2];
}


CString SongUtil::MakeSortString( CString s )
{
	s.MakeUpper();

	// Make sure that non-alphanumeric strings are placed at the very end.
	if( s.size() > 0 )
	{
		if( s[0] == '.' )	// ".59"
			s.erase(s.begin());
		if( (s[0] < 'A' || s[0] > 'Z') && (s[0] < '0' || s[0] > '9') )
			s = char(126) + s;
	}

	return s;
}

static bool CompareSongPointersByTitle( const Song *pSong1, const Song *pSong2 )
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

	int ret = strcmp( s1, s2 );
	if(ret < 0) return true;
	if(ret > 0) return false;

	/* The titles are the same.  Ensure we get a consistent ordering
	 * by comparing the unique SongFilePaths. */
	return pSong1->GetSongFilePath().CompareNoCase(pSong2->GetSongFilePath()) < 0;
}

void SongUtil::SortSongPointerArrayByTitle( vector<Song*> &vpSongsInOut )
{
	sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByTitle );
}

static bool CompareSongPointersByBPM( const Song *pSong1, const Song *pSong2 )
{
	DisplayBpms bpms1, bpms2;
	pSong1->GetDisplayBpms( bpms1 );
	pSong2->GetDisplayBpms( bpms2 );

	if( bpms1.GetMax() < bpms2.GetMax() )
		return true;
	if( bpms1.GetMax() > bpms2.GetMax() )
		return false;
	
	return CompareCStringsAsc( pSong1->GetSongFilePath(), pSong2->GetSongFilePath() );
}

void SongUtil::SortSongPointerArrayByBPM( vector<Song*> &vpSongsInOut )
{
	sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByBPM );
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

static bool CompDescending( const pair<Song *, CString> &a, const pair<Song *, CString> &b )
{
	return a.second > b.second;
}
static bool CompAscending( const pair<Song *, CString> &a, const pair<Song *, CString> &b )
{
	return a.second < b.second;
}

void SongUtil::SortSongPointerArrayByGrades( vector<Song*> &vpSongsInOut, bool bDescending )
{
	/* Optimize by pre-writing a string to compare, since doing GetNumNotesWithGrade
	 * inside the sort is too slow. */
	typedef pair< Song *, CString > val;
	vector<val> vals;
	vals.reserve( vpSongsInOut.size() );

	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
	{
		Song *pSong = vpSongsInOut[i];

		int iCounts[NUM_Grade];
		const Profile *pProfile = PROFILEMAN->GetMachineProfile();
		const Style *pStyle = GAMESTATE->GetCurrentStyle();
		StepsType st = pStyle->m_StepsType;
		pProfile->GetGrades( pSong, st, iCounts );

		CString foo;
		foo.reserve(256);
		for( int g=Grade_Tier01; g<=Grade_NoData; ++g )
			AppendOctal( iCounts[g], 3, foo );
		vals.push_back( val(pSong, foo) );
	}

	sort( vals.begin(), vals.end(), bDescending ? CompDescending : CompAscending );

	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
		vpSongsInOut[i] = vals[i].first;
}


void SongUtil::SortSongPointerArrayByArtist( vector<Song*> &vpSongsInOut )
{
	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
		g_mapSongSortVal[vpSongsInOut[i]] = MakeSortString( vpSongsInOut[i]->GetTranslitArtist() );
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
}

/* This is for internal use, not display; sorting by Unicode codepoints isn't very
 * interesting for display. */
void SongUtil::SortSongPointerArrayByDisplayArtist( vector<Song*> &vpSongsInOut )
{
	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
		g_mapSongSortVal[vpSongsInOut[i]] = MakeSortString( vpSongsInOut[i]->GetDisplayArtist() );
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
}

static int CompareSongPointersByGenre(const Song *pSong1, const Song *pSong2)
{
	return pSong1->m_sGenre < pSong2->m_sGenre;
}

void SongUtil::SortSongPointerArrayByGenre( vector<Song*> &vpSongsInOut )
{
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByGenre );
}

int SongUtil::CompareSongPointersByGroup(const Song *pSong1, const Song *pSong2)
{
	return pSong1->m_sGroupName < pSong2->m_sGroupName;
}

static int CompareSongPointersByGroupAndTitle( const Song *pSong1, const Song *pSong2 )
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

void SongUtil::SortSongPointerArrayByGroupAndTitle( vector<Song*> &vpSongsInOut )
{
	sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByGroupAndTitle );
}

void SongUtil::SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, ProfileSlot slot, bool bDescending )
{
	if( !PROFILEMAN->IsPersistentProfile(slot) )
		return;	// nothing to do since we don't have data
	Profile* pProfile = PROFILEMAN->GetProfile(slot);
	SortSongPointerArrayByNumPlays( vpSongsInOut, pProfile, bDescending );
}

void SongUtil::SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, const Profile* pProfile, bool bDescending )
{
	ASSERT( pProfile );
	for(unsigned i = 0; i < vpSongsInOut.size(); ++i)
		g_mapSongSortVal[vpSongsInOut[i]] = ssprintf("%9i", pProfile->GetSongNumTimesPlayed(vpSongsInOut[i]));
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), bDescending ? CompareSongPointersBySortValueDescending : CompareSongPointersBySortValueAscending );
	g_mapSongSortVal.clear();
}

CString SongUtil::GetSectionNameFromSongAndSort( const Song* pSong, SortOrder so )
{
	if( pSong == NULL )
		return CString();

	switch( so )
	{
	case SORT_PREFERRED:
		return CString();
	case SORT_GROUP:
		// guaranteed not empty	
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
				return CString();
			else if( s[0] >= '0' && s[0] <= '9' )
				return "0-9";
			else if( s[0] < 'A' || s[0] > 'Z')
				return SORT_OTHER.GetValue();
			else
				return s.Left(1);
		}
	case SORT_GENRE:
		if( !pSong->m_sGenre.empty() )
			return pSong->m_sGenre;
		return SORT_NOT_AVAILABLE.GetValue();
	case SORT_BPM:
		{
			const int iBPMGroupSize = 20;
			DisplayBpms bpms;
			pSong->GetDisplayBpms( bpms );
			int iMaxBPM = (int)bpms.GetMax();
			iMaxBPM += iBPMGroupSize - (iMaxBPM%iBPMGroupSize) - 1;
			return ssprintf("%03d-%03d",iMaxBPM-(iBPMGroupSize-1), iMaxBPM);
		}
	case SORT_POPULARITY:
		return CString();
	case SORT_TOP_GRADES:
		{
			int iCounts[NUM_Grade];
			PROFILEMAN->GetMachineProfile()->GetGrades( pSong, GAMESTATE->GetCurrentStyle()->m_StepsType, iCounts );

			for( int i=Grade_Tier01; i<NUM_Grade; ++i )
			{
				Grade g = (Grade)i;
				if( iCounts[i] > 0 )
					return ssprintf( "%4s x %d", GradeToThemedString(g).c_str(), iCounts[i] );
			}
			return GradeToThemedString( Grade_NoData );
		}
	case SORT_EASY_METER:
		{
			Steps* pSteps = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_EASY);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_MEDIUM_METER:
		{
			Steps* pSteps = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_MEDIUM);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_HARD_METER:
		{
			Steps* pSteps = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_HARD);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_CHALLENGE_METER:
		{
			Steps* pSteps = pSong->GetStepsByDifficulty(GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_CHALLENGE);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_MODE_MENU:
		return CString();
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
	default:
		ASSERT(0);
		return CString();
	}
}

void SongUtil::SortSongPointerArrayBySectionName( vector<Song*> &vpSongsInOut, SortOrder so )
{
	CString sOther = SORT_OTHER.GetValue();
	for(unsigned i = 0; i < vpSongsInOut.size(); ++i)
	{
		CString val = GetSectionNameFromSongAndSort( vpSongsInOut[i], so );

		/* Make sure 0-9 comes first and OTHER comes last. */
		if( val == "0-9" )			val = "0";
		else if( val == sOther )    val = "2";
		else						val = "1" + MakeSortString(val);

		g_mapSongSortVal[vpSongsInOut[i]] = val;
	}

	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
	g_mapSongSortVal.clear();
}

void SongUtil::SortSongPointerArrayByMeter( vector<Song*> &vpSongsInOut, Difficulty dc )
{
	g_mapSongSortVal.clear();
	for(unsigned i = 0; i < vpSongsInOut.size(); ++i)
	{
		/* Ignore locked steps. */
		const Steps* pSteps = vpSongsInOut[i]->GetClosestNotes( GAMESTATE->GetCurrentStyle()->m_StepsType, dc, true );
		CString &s = g_mapSongSortVal[vpSongsInOut[i]];
		s = ssprintf("%03d", pSteps ? pSteps->GetMeter() : 0);

		/* Hack: always put tutorial songs first. */
		s += ssprintf( "%c", vpSongsInOut[i]->IsTutorial()? '0':'1' );

		/* 
		 * pSteps may not be exactly the difficulty we want; for example, we may
		 * be sorting by Hard difficulty and a song may have no Hard steps.
		 *
		 * In this case, we can end up with unintuitive ties; for example, pSteps
		 * may be Medium with a meter of 5, which will sort it among the 5-meter
		 * Hard songs.  Break the tie, by adding the difficulty to the sort as
		 * well.  That way, we'll always put Medium 5s before Hard 5s.  If all
		 * songs are using the preferred difficulty (dc), this will be a no-op.
		 */
		s += ssprintf( "%c", (pSteps? pSteps->GetDifficulty():0) + '0' );

		if( PREFSMAN->m_bSubSortByNumSteps )
			s += ssprintf("%06.0f",pSteps ? pSteps->GetRadarValues()[RADAR_NUM_TAPS_AND_HOLDS] : 0);
	}
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
}

void SongUtil::SortByMostRecentlyPlayedForMachine( vector<Song*> &vpSongsInOut )
{
	Profile *pProfile = PROFILEMAN->GetMachineProfile();

	FOREACH_CONST( Song*, vpSongsInOut, s )
	{
		int iNumTimesPlayed = pProfile->GetSongNumTimesPlayed( *s );
		CString val = iNumTimesPlayed ? pProfile->GetSongLastPlayedDateTime(*s).GetString() : "0";
		g_mapSongSortVal[*s] = val;
	}

	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueDescending );
	g_mapSongSortVal.clear();
}

//////////////////////////////////
// SongID
//////////////////////////////////

void SongID::FromSong( const Song *p )
{
	if( p )
		sDir = p->GetSongDir();
	else
		sDir = "";
	
	// HACK for backwards compatibility:
	// Strip off leading "/".  2005/05/21 file layer changes added a leading slash.
	if( sDir.Left(1) == "/" )
		sDir.erase( sDir.begin() );
}

Song *SongID::ToSong() const
{
	// HACK for backwards compatibility:
	// Re-add the leading "/".  2005/05/21 file layer changes added a leading slash.
	CString sDir2 = sDir;
	if( sDir2.Left(1) != "/" )
		sDir2 = "/" + sDir2;

	return SONGMAN->GetSongFromDir( sDir2 );
}

XNode* SongID::CreateNode() const
{
	XNode* pNode = new XNode;
	pNode->m_sName = "Song";

	pNode->AppendAttr( "Dir", sDir );

	return pNode;
}

void SongID::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->m_sName == "Song" );
	pNode->GetAttrValue("Dir", sDir);
}

CString SongID::ToString() const
{
	return sDir;
}

bool SongID::IsValid() const
{
	return !sDir.empty();
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
