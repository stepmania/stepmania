#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Course.h"
#include "PrefsManager.h"
#include "song.h"
#include "GameManager.h"
#include "SongManager.h"
#include "RageException.h"
#include "RageLog.h"
#include "MsdFile.h"
#include "PlayerOptions.h"
#include "SongOptions.h"
#include "RageUtil.h"
#include "TitleSubstitution.h"
#include "Notes.h"
#include "GameState.h"


Course::Course()
{
	m_bIsAutogen = false;
	m_bRepeat = false;
	m_bRandomize = false;
	m_bDifficult = false;
	m_iLives = -1;

	//
	// Init high scores
	//
	for( unsigned i=0; i<NUM_NOTES_TYPES; i++ )
		for( int j=0; j<NUM_RANKING_LINES; j++ )
		{
			m_RankingScores[i][j].iDancePoints = 573;
			m_RankingScores[i][j].fSurviveTime = 57.3f;
			m_RankingScores[i][j].sName = DEFAULT_RANKING_NAME;
		}

	for( unsigned m=0; m<NUM_MEMORY_CARDS; m++ )
		for( unsigned i=0; i<NUM_NOTES_TYPES; i++ )
		{
			m_MemCardScores[m][i].iNumTimesPlayed = 0;
			m_MemCardScores[m][i].iDancePoints = 0;
			m_MemCardScores[m][i].fSurviveTime = 0;
		}
}

/*
 * GetSongDir() contains a path to the song, possibly a full path, eg:
 * Songs\Group\SongName                   or 
 * My Other Song Folder\Group\SongName    or
 * c:\Corny J-pop\Group\SongName
 *
 * Most course group names are "Group\SongName", so we want to
 * match against the last two elements. Let's also support
 * "SongName" alone, since the group is only important when it's
 * potentially ambiguous.
 *
 * Let's *not* support "Songs\Group\SongName" in course files.
 * That's probably a common error, but that would result in
 * course files floating around that only work for people who put
 * songs in "Songs"; we don't want that.
 */

Song *Course::FindSong(CString sGroup, CString sSong) const
{
	const vector<Song*> &apSongs = SONGMAN->GetAllSongs();

	// foreach song
	for( unsigned i = 0; i < apSongs.size(); i++ )
	{
		Song* pSong = apSongs[i];

		if( sGroup.size() && sGroup.CompareNoCase(pSong->m_sGroupName) != 0)
			continue; /* wrong group */

		CString sDir = pSong->GetSongDir();
		sDir.Replace("\\","/");
		CStringArray bits;
		split( sDir, "/", bits );
		ASSERT(bits.size() >= 2); /* should always have at least two parts */
		CString sLastBit = bits[bits.size()-1];

		// match on song dir or title (ala DWI)
		if( sSong.CompareNoCase(sLastBit)==0 )
			return pSong;
		if( sSong.CompareNoCase(pSong->GetFullTranslitTitle())==0 )
			return pSong;
	}

	LOG->Trace( "Course file '%s' contains a song '%s%s%s' that is not present",
			m_sPath.c_str(), sGroup.c_str(), sGroup.size()? "/":"", sSong.c_str());

	return NULL;	
}

void Course::LoadFromCRSFile( CString sPath )
{
	m_sPath = sPath;	// save path

	MsdFile msd;
	if( !msd.ReadFile(sPath) )
		RageException::Throw( "Error opening CRS file '%s'.", sPath.c_str() );

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );
	sFName = sDir + sFName;

	CStringArray arrayPossibleBanners;
	GetDirListing( sFName + ".png", arrayPossibleBanners, false, true );
	GetDirListing( sFName + ".jpg", arrayPossibleBanners, false, true );
	GetDirListing( sFName + ".bmp", arrayPossibleBanners, false, true );
	GetDirListing( sFName + ".gif", arrayPossibleBanners, false, true );
	if( !arrayPossibleBanners.empty() )
		m_sBannerPath = arrayPossibleBanners[0];

	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		CString sValueName = msd.GetParam(i, 0);
		const MsdFile::value_t &sParams = msd.GetValue(i);

		// handle the data
		if( 0 == stricmp(sValueName, "COURSE") )
			m_sName = sParams[1];

		else if( 0 == stricmp(sValueName, "REPEAT") )
		{
			CString str = sParams[1];
			str.MakeLower();
			if( str.Find("yes") != -1 )
				m_bRepeat = true;
		}

		else if( 0 == stricmp(sValueName, "LIVES") )
			m_iLives = atoi( sParams[1] );

		else if( 0 == stricmp(sValueName, "EXTRA") )
			m_iExtra = atoi( sParams[1] );

		else if( 0 == stricmp(sValueName, "SONG") )
		{
			Entry new_entry;

			// infer entry::Type from the first param
			if( sParams[1].Left(strlen("BEST")) == "BEST" )
			{
				new_entry.type = Entry::best;
				new_entry.players_index = atoi( sParams[1].Right(sParams[1].size()-strlen("BEST")) ) - 1;
				CLAMP( new_entry.players_index, 0, 500 );
			}
			else if( sParams[1].Left(strlen("WORST")) == "WORST" )
			{
				new_entry.type = Entry::worst;
				new_entry.players_index = atoi( sParams[1].Right(sParams[1].size()-strlen("WORST")) ) - 1;
				CLAMP( new_entry.players_index, 0, 500 );
			}
			else if( sParams[1] == "*" )
			{
				new_entry.type = Entry::random;
			}
			else if( sParams[1].Right(1) == "*" )
			{
				new_entry.type = Entry::random_within_group;
				CString sSong = sParams[1];
				sSong.Replace( "\\", "/" );
				CStringArray bits;
				split( sSong, "/", bits );
				if( bits.size() == 2 )
					new_entry.group_name = bits[0];
				else
					LOG->Warn( "Course file '%s' contains a random_within_group entry '%s' that is invalid. "
								"Song should be in the format '<group>/*'.",
								m_sPath.c_str(), sSong.c_str());
				if( !SONGMAN->DoesGroupExist(new_entry.group_name) )
				{
					LOG->Warn( "Course file '%s' random_within_group entry '%s' specifies a group that doesn't exist. "
								"This entry will be ignored.",
								m_sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}
			else
			{
				new_entry.type = Entry::fixed;

				CString sSong = sParams[1];
				sSong.Replace( "\\", "/" );
				CStringArray bits;
				split( sSong, "/", bits );
				if( bits.size() == 1 )
					new_entry.pSong = FindSong( "", bits[0] );
				else if( bits.size() == 2 )
					new_entry.pSong = FindSong( bits[0], bits[1] );
				else
				{
					LOG->Warn( "Course file '%s' contains a fixed song entry '%s' that is invalid. "
								"Song should be in the format '<group>/<song>'.",
								m_sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
				if( !new_entry.pSong )
				{
					LOG->Warn( "Course file '%s' contains a fixed song entry '%s' that does not exist. "
								"This entry will be ignored.",
								m_sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}

			new_entry.difficulty = StringToDifficulty( sParams[2] );
			if( new_entry.difficulty == DIFFICULTY_INVALID )
			{
				int retval = sscanf( sParams[2], "%d..%d", &new_entry.low_meter, &new_entry.high_meter );
				if( retval == 1 )
					new_entry.high_meter = new_entry.low_meter;
				else if( retval != 2 )
				{
					LOG->Warn("Course file '%s' contains an invalid difficulty setting: \"%s\", 3..6 used instead",
						m_sPath.c_str(), sParams[2].c_str());
					new_entry.low_meter = 3;
					new_entry.high_meter = 6;
				}
			}

			new_entry.modifiers = sParams[3];

			m_entries.push_back( new_entry );
		}

		else
			LOG->Trace( "Unexpected value named '%s'", sValueName.c_str() );
	}
	static TitleSubst tsub("courses");

	CString ignore;
	tsub.Subst(m_sName, ignore, ignore,
				ignore, ignore, ignore);
}


void Course::Save()
{
	ASSERT( !m_bIsAutogen );

	FILE* fp = fopen( m_sPath, "w" );
	if( fp == NULL )
	{
		LOG->Warn( "Could not write course file '%s'.", m_sPath.c_str() );
		return;
	}

	fprintf( fp, "#COURSE:%s;\n", m_sName.c_str() );
	fprintf( fp, "#REPEAT:%s;\n", m_bRepeat ? "YES" : "NO" );
	fprintf( fp, "#LIVES:%i;\n", m_iLives );
	fprintf( fp, "#EXTRA:%i;\n", m_iExtra );

	for( unsigned i=0; i<m_entries.size(); i++ )
	{
		const Entry& entry = m_entries[i];

		switch( entry.type )
		{
		case Entry::fixed:
			fprintf( fp, "#SONG:%s", entry.pSong->GetSongDir().c_str() );
			break;
		case Entry::random:
			fprintf( fp, "#SONG:*" );
			break;
		case Entry::random_within_group:
			fprintf( fp, "#SONG:%s/*", entry.group_name.c_str() );
			break;
		case Entry::best:
			fprintf( fp, "#SONG:BEST%d", entry.players_index+1 );
			break;
		case Entry::worst:
			fprintf( fp, "#SONG:WORST%d", entry.players_index+1 );
			break;
		default:
			ASSERT(0);
		}

		if( entry.difficulty != DIFFICULTY_INVALID )
			fprintf( fp, ":%s", DifficultyToString(entry.difficulty).c_str() );
	
		if( entry.low_meter != -1  &&  entry.high_meter != -1 )
			fprintf( fp, ":%d..%d", entry.low_meter, entry.high_meter );

		fprintf( fp, ";\n" );
	}

	fclose( fp );
}


void Course::AutogenEndlessFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup )
{
	m_bIsAutogen = true;
	m_bRepeat = true;
	m_bRandomize = true;
	m_iLives = -1;

	CStringArray asPossibleBannerPaths;
	GetDirListing( "Songs/" + sGroupName + "/banner.png", asPossibleBannerPaths, false, true );
	GetDirListing( "Songs/" + sGroupName + "/banner.jpg", asPossibleBannerPaths, false, true );
	GetDirListing( "Songs/" + sGroupName + "/banner.gif", asPossibleBannerPaths, false, true );
	GetDirListing( "Songs/" + sGroupName + "/banner.bmp", asPossibleBannerPaths, false, true );
	if( !asPossibleBannerPaths.empty() )
		m_sBannerPath = asPossibleBannerPaths[0];

	m_sName = SONGMAN->ShortenGroupName( sGroupName );	

	// We want multiple songs, so we can try to prevent repeats during
	// gameplay. (We might still get a repeat at the repeat boundary,
	// but that'd be rare.) -glenn
	Entry e;
	e.type = Entry::random_within_group;
	e.group_name = sGroupName;
	e.difficulty = DIFFICULTY_MEDIUM;

	vector<Song*> vSongs;
	SONGMAN->GetSongs( vSongs, e.group_name );
	for( unsigned i = 0; i < vSongs.size(); ++i)
		m_entries.push_back( e );
}

void Course::AutogenNonstopFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup )
{
	AutogenEndlessFromGroup( sGroupName, apSongsInGroup );

	m_bRepeat = false;

	m_sName += " Random";	

	// resize to 4
	while( m_entries.size() < 4 )
		m_entries.push_back( m_entries[0] );
	while( m_entries.size() > 4 )
		m_entries.pop_back();
}


bool Course::HasDifficult() const
{
	for( unsigned i=0; i<m_entries.size(); i++ )
		if( m_entries[i].difficulty >= DIFFICULTY_HARD )
			return false;
	return true;
}

bool Course::IsPlayableIn( NotesType nt ) const
{
	/* XXX: Is this good enough?  This needs to be guaranteed: if we say
	 * a course is playable and it's not, we'll crash in ScreenGameplay. */

	vector<Song*> vSongs;
	vector<Notes*> vNotes;
	vector<CString> vsModifiers;
	GetStageInfo( vSongs, vNotes, vsModifiers, nt, false);
	
	return vNotes.size() > 0;
}


void Course::GetStageInfo( 
	vector<Song*>& vSongsOut, 
	vector<Notes*>& vNotesOut, 
	vector<CString>& vsModifiersOut, 
	NotesType nt, 
	bool bDifficult ) const
{
	if( bDifficult )
		ASSERT( HasDifficult() );

	vector<Entry> entries = m_entries;

	if( m_bRandomize )
		random_shuffle( entries.begin(), entries.end() );

	vector<Song*> vSongsByMostPlayed;

	vector<Song*> AllSongsShuffled = SONGMAN->GetAllSongs();
	random_shuffle( AllSongsShuffled.begin(), AllSongsShuffled.end() );
	int CurSong = 0; /* Current offset into AllSongsShuffled */

	for( unsigned i=0; i<entries.size(); i++ )
	{
		const Entry& e = entries[i];

		Song* pSong = NULL;	// fill this in
		Notes* pNotes = NULL;	// fill this in


		switch( e.type )
		{
		case Entry::fixed:
			pSong = e.pSong;
			if( pSong )
			{
				if( e.difficulty == DIFFICULTY_INVALID )
					pNotes = pSong->GetNotes( nt, e.low_meter, e.high_meter, PREFSMAN->m_bAutogenMissingTypes );
				else
					pNotes = pSong->GetNotes( nt, e.difficulty, PREFSMAN->m_bAutogenMissingTypes );
			}
			break;
		case Entry::random:
		case Entry::random_within_group:
			{
				// find a song with the notes we want
				for( unsigned j=0; j<AllSongsShuffled.size(); j++ )
				{
					/* See if the first song matches what we want. */
					pSong = AllSongsShuffled[CurSong];
					CurSong = (CurSong+1) % AllSongsShuffled.size();

					if(e.type == Entry::random_within_group &&
					   pSong->m_sGroupName.CompareNoCase(e.group_name))
					   continue; /* wrong group */

					if( e.difficulty == DIFFICULTY_INVALID )
						pNotes = pSong->GetNotes( nt, e.low_meter, e.high_meter, PREFSMAN->m_bAutogenMissingTypes );
					else
						pNotes = pSong->GetNotes( nt, e.difficulty, PREFSMAN->m_bAutogenMissingTypes );

					if( pNotes )	// found a match
						break;		// stop searching

					pSong = NULL;
					pNotes = NULL;
				}
			}
			break;
		case Entry::best:
		case Entry::worst:
			{
				if(vSongsByMostPlayed.size() == 0)
				{
					/* Probably the first time getting here; fill it in just once. */
					/* XXX: This is still expensive enough to cause a noticable
					 * frame drop when scrolling over best/worst entries. */
					vSongsByMostPlayed = SONGMAN->GetAllSongs();
					SortSongPointerArrayByMostPlayed( vSongsByMostPlayed );
					
					// filter out songs that don't have both medium and hard steps and long ver sons
					for( int j=vSongsByMostPlayed.size()-1; j>=0; j-- )
					{
						Song* pSong = vSongsByMostPlayed[j];
						if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fLongVerSongSeconds  ||
							pSong->m_fMusicLengthSeconds > PREFSMAN->m_fMarathonVerSongSeconds  || 
							!pSong->GetNotes(nt, DIFFICULTY_MEDIUM, PREFSMAN->m_bAutogenMissingTypes)  ||
							!pSong->GetNotes(nt, DIFFICULTY_HARD, PREFSMAN->m_bAutogenMissingTypes) )
							vSongsByMostPlayed.erase( vSongsByMostPlayed.begin()+j );
					}
				}

				if( e.players_index >= (int)vSongsByMostPlayed.size() )
					break;

				switch( e.type )
				{
				case Entry::best:
					pSong = vSongsByMostPlayed[e.players_index];
					break;
				case Entry::worst:
					pSong = vSongsByMostPlayed[vSongsByMostPlayed.size()-1-e.players_index];
					break;
				default:
					ASSERT(0);
				}

				if( e.difficulty == DIFFICULTY_INVALID )
					pNotes = pSong->GetNotes( nt, e.low_meter, e.high_meter, PREFSMAN->m_bAutogenMissingTypes );
				else
					pNotes = pSong->GetNotes( nt, e.difficulty, PREFSMAN->m_bAutogenMissingTypes );

				if( pNotes == NULL )
					pNotes = pSong->GetClosestNotes( nt, DIFFICULTY_MEDIUM );
			}
			break;
		default:
			ASSERT(0);
		}

		if( !pSong || !pNotes )
			continue;	// this song entry isn't playable.  Skip.

		if(GAMESTATE->m_bDifficultCourses)
		{
			/* See if we can find a NoteData that's one notch more difficult than
			 * the one we found above. */
			Difficulty dc = pNotes->GetDifficulty();
			if(dc < DIFFICULTY_CHALLENGE)
			{
				dc  = Difficulty(dc + 1);
				Notes* pNewNotes = pSong->GetNotes( nt, dc, PREFSMAN->m_bAutogenMissingTypes );
				if(pNewNotes)
					pNotes = pNewNotes;
			}
		}

		vSongsOut.push_back( pSong ); 
		vNotesOut.push_back( pNotes ); 
		vsModifiersOut.push_back( e.modifiers );
	}


}

bool Course::GetFirstStageInfo(
	Song*& pSongOut, 
	Notes*& pNotesOut, 
	CString& sModifiersOut, 
	NotesType nt ) const
{
	vector<Song*> vSongs; 
	vector<Notes*> vNotes; 
	vector<CString> vsModifiers;

	GetStageInfo(
		vSongs, 
		vNotes, 
		vsModifiers, 
		nt, 
		false );
	if( vSongs.empty() )
		return false;
	
	pSongOut = vSongs[0]; 
	pNotesOut = vNotes[0]; 
	sModifiersOut = vsModifiers[0];
	return true;
}

RageColor Course::GetColor() const
{
	// This could be made smarter
	if( m_entries.size() >= 7 )
		return RageColor(1,0,0,1);	// red
	else if( m_entries.size() >= 4 )
		return RageColor(1,0.5f,0,1);	// orange
	else
		return RageColor(0,1,0,1);	// green
}

bool Course::IsMysterySong( int stage ) const
{
	switch( m_entries[stage].type )
	{
	case Entry::fixed:					return false;
	case Entry::random:					return true;
	case Entry::random_within_group:	return true;
	case Entry::best:			return false;
	case Entry::worst:			return false;
	default:		ASSERT(0);			return true;
	}
}

Difficulty Course::GetDifficulty( int stage ) const
{
	Difficulty dc = m_entries[stage].difficulty;

	if(GAMESTATE->m_bDifficultCourses && dc < DIFFICULTY_CHALLENGE)
		dc  = Difficulty(dc + 1);

	return dc;
}

void Course::GetMeterRange( int stage, int& iMeterLowOut, int& iMeterHighOut ) const
{
	iMeterLowOut = m_entries[stage].low_meter;
	iMeterHighOut = m_entries[stage].high_meter;
}

bool Course::ContainsAnyMysterySongs() const
{
	for( unsigned i=0; i<m_entries.size(); i++ )
		if( IsMysterySong(i) )
			return true;
	return false;
}

bool Course::GetTotalSeconds( float& fSecondsOut ) const
{
	if( ContainsAnyMysterySongs() )
		return false;
	
	vector<Song*> vSongsOut;
	vector<Notes*> vNotesOut;
	vector<CString> vsModifiersOut;
	GetStageInfo(
		vSongsOut, 
		vNotesOut, 
		vsModifiersOut, 
		NOTES_TYPE_DANCE_SINGLE, 	// doesn't matter
		false );	// doesn't matter

	fSecondsOut = 0;
	for( unsigned i=0; i<vSongsOut.size(); i++ )
		fSecondsOut += vSongsOut[i]->m_fMusicLengthSeconds;
	return true;
}


struct RankingToInsert
{
	PlayerNumber pn;
	int iDancePoints;
	float fSurviveTime;

	static int CompareDescending( const RankingToInsert &hs1, const RankingToInsert &hs2 )
	{
		if( hs1.iDancePoints > hs2.iDancePoints )		return -1;
		else if( hs1.iDancePoints == hs2.iDancePoints )	return 0;
		else											return +1;
	}
	static void SortDescending( vector<RankingToInsert>& vHSout )
	{ 
		sort( vHSout.begin(), vHSout.end(), CompareDescending ); 
	}
};

void Course::AddScores( NotesType nt, bool bPlayerEnabled[NUM_PLAYERS], int iDancePoints[NUM_PLAYERS], float fSurviveTime[NUM_PLAYERS], int iRankingIndexOut[NUM_PLAYERS], bool bNewRecordOut[NUM_PLAYERS] )
{
	vector<RankingToInsert> vHS;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		iRankingIndexOut[p] = -1;
		bNewRecordOut[p] = false;

		if( !bPlayerEnabled[p] )
			continue;	// skip
        

		// Update memory card
		m_MemCardScores[p][nt].iNumTimesPlayed++;
		m_MemCardScores[MEMORY_CARD_MACHINE][nt].iNumTimesPlayed++;

		if( iDancePoints[p] > m_MemCardScores[p][nt].iDancePoints )
		{
			m_MemCardScores[p][nt].iDancePoints = iDancePoints[p];
			m_MemCardScores[p][nt].fSurviveTime = fSurviveTime[p];
			bNewRecordOut[p] = true;
		}

		if( iDancePoints[p] > m_MemCardScores[MEMORY_CARD_MACHINE][nt].iDancePoints )
		{
			m_MemCardScores[MEMORY_CARD_MACHINE][nt].iDancePoints = iDancePoints[p];
			m_MemCardScores[MEMORY_CARD_MACHINE][nt].fSurviveTime = fSurviveTime[p];
		}


		// Update Ranking
		RankingToInsert hs;
		hs.iDancePoints = iDancePoints[p];
		hs.fSurviveTime = fSurviveTime[p];
		hs.pn = (PlayerNumber)p;
		vHS.push_back( hs );
	}

	// Sort descending before inserting.
	// This guarantees that a high score will not switch poitions on us when we later insert scores for the other player
	RankingToInsert::SortDescending( vHS );

	for( unsigned i=0; i<vHS.size(); i++ )
	{
		RankingToInsert& newHS = vHS[i];
		RankingScore* rankingScores = m_RankingScores[nt];
		for( int i=0; i<NUM_RANKING_LINES; i++ )
		{
			if( newHS.iDancePoints > rankingScores[i].iDancePoints )
			{
				// We found the insert point.  Shift down.
				for( int j=NUM_RANKING_LINES-1; j>i; j-- )
					rankingScores[j] = rankingScores[j-1];
				// insert
				rankingScores[i].fSurviveTime = newHS.fSurviveTime;
				rankingScores[i].iDancePoints = newHS.iDancePoints;
				rankingScores[i].sName = DEFAULT_RANKING_NAME;
				iRankingIndexOut[newHS.pn] = i;
				break;
			}
		}
	}
}


//
// Sorting stuff
//
static bool CompareCoursePointersByName(const Course* pCourse1, const Course* pCourse2)
{
	// HACK:  strcmp and other string comparators appear to eat whitespace.
	// For example, the string "Players Best 13-16" is sorted between 
	// "Players Best  1-4" and "Players Best  5-8".  Replace the string "  "
	// with " 0" for comparison only.

	// XXX: That doesn't happen to me, and it shouldn't (strcmp is strictly
	// a byte sort, though CompareNoCase doesn't use strcmp).  Are you sure
	// you didn't have only one space before? -glenn
	CString sName1 = pCourse1->m_sName;
	CString sName2 = pCourse2->m_sName;
	sName1.Replace( "  " , " 0" );
	sName2.Replace( "  " , " 0" );
	return sName1.CompareNoCase( sName2 ) == -1;
}

static bool CompareCoursePointersByAutogen(const Course* pCourse1, const Course* pCourse2)
{
	int b1 = pCourse1->m_bIsAutogen;
	int b2 = pCourse2->m_bIsAutogen;
	if( b1 < b2 )
		return true;
	else if( b1 > b2 )
		return false;
	else
		return CompareCoursePointersByName(pCourse1,pCourse2);
}

static bool CompareCoursePointersByDifficulty(const Course* pCourse1, const Course* pCourse2)
{
	int iNum1 = pCourse1->GetEstimatedNumStages();
	int iNum2 = pCourse2->GetEstimatedNumStages();
	if( iNum1 < iNum2 )
		return true;
	else if( iNum1 > iNum2 )
		return false;
	else // iNum1 == iNum2
		return CompareCoursePointersByAutogen( pCourse1, pCourse2 );
}

void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses )
{
	sort( apCourses.begin(), apCourses.end(), CompareCoursePointersByDifficulty );
}

bool Course::HasBanner() const
{
	return m_sBannerPath != ""  &&  IsAFile(m_sBannerPath);
}
