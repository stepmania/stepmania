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


Course::Course()
{
	m_bIsAutoGen = false;
	m_bRepeat = false;
	m_bRandomize = false;
	m_iLives = -1;

	//
	// Init high scores
	//
	for( unsigned i=0; i<NUM_NOTES_TYPES; i++ )
		for( unsigned j=0; j<NUM_RANKING_LINES; j++ )
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

Song *Course::FindSong(CString sSongDir) const
{
	const vector<Song*> &apSongs = SONGMAN->GetAllSongs();

	CStringArray split_SongDir;
	sSongDir.Replace("\\", "/");
	split( sSongDir, "/", split_SongDir, true );

	if( split_SongDir.size() > 2 )
	{
		LOG->Warn( "Course file \"%s\" path \"%s\" should contain "
					"at most one backslash; ignored.",
				    m_sPath.GetString(), sSongDir.GetString());
		return NULL;
	}

	// foreach song
	for( unsigned i = 0; i < apSongs.size(); i++ )
	{
		CString dir = apSongs[i]->GetSongDir();
		dir.Replace("\\", "/");
		CStringArray splitted; /* splat! */
		split( dir, "/", splitted, true );
		bool matches = true;
		
		int split_no = splitted.size()-1;
		int SongDir_no = split_SongDir.size()-1;

		while( split_no >= 0 && SongDir_no >= 0 ) {
			if( stricmp(splitted[split_no--], split_SongDir[SongDir_no--] ) )
				matches=false;
		}

		if(matches)
			return apSongs[i];
	}

	LOG->Trace( "Course \"%s\": couldn't match song \"%s\"",
			    m_sPath.GetString(), sSongDir.GetString());
	return NULL;
}

void Course::LoadFromCRSFile( CString sPath )
{
	m_sPath = sPath;	// save path

	MsdFile msd;
	if( !msd.ReadFile(sPath) )
		RageException::Throw( "Error opening CRS file '%s'.", sPath.GetString() );

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );

	CStringArray arrayPossibleBanners;
	GetDirListing( "Courses/" + sFName + ".png", arrayPossibleBanners, false, true );
	GetDirListing( "Courses/" + sFName + ".jpg", arrayPossibleBanners, false, true );
	GetDirListing( "Courses/" + sFName + ".bmp", arrayPossibleBanners, false, true );
	GetDirListing( "Courses/" + sFName + ".gif", arrayPossibleBanners, false, true );
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
			// infer entry::Type from the first param
			Entry new_entry;

			if( sParams[1].Left(strlen("PlayersBest")) == "PlayersBest" )
			{
				new_entry.type = Entry::players_best;
				new_entry.players_index = atoi( sParams[1].Right(sParams[1].size()-strlen("PlayersBest")) ) - 1;
				CLAMP( new_entry.players_index, 0, 500 );
			}
			else if( sParams[1].Left(strlen("PlayersWorst")) == "PlayersWorst" )
			{
				new_entry.type = Entry::players_worst;
				new_entry.players_index = atoi( sParams[1].Right(sParams[1].size()-strlen("PlayersWorst")) ) - 1;
				CLAMP( new_entry.players_index, 0, 500 );
			}
			else if( sParams[1] == "*" )
			{
				new_entry.type = Entry::random;
			}
			else if( sParams[1].Right(1) == "*" )
			{
				new_entry.type = Entry::random_within_group;
				CString sThrowAway;
				splitrelpath( sParams[1], new_entry.group_name, sThrowAway, sThrowAway );
				new_entry.group_name.resize( new_entry.group_name.size()-1 );		// chomp triling slash
			}
			else
			{
				new_entry.type = Entry::fixed;
				CString sThrowAway;
				splitrelpath( sParams[1], new_entry.group_name, new_entry.song_name, sThrowAway );
			}

			new_entry.difficulty = StringToDifficulty( sParams[2] );
			if( new_entry.difficulty == DIFFICULTY_INVALID )
			{
				int retval = sscanf( sParams[2], "%d..%d", &new_entry.low_meter, &new_entry.high_meter );
				if( retval != 2 )
				{
					new_entry.low_meter = 3;
					new_entry.high_meter = 6;
				}
			}

			new_entry.modifiers = sParams[3];

			m_entries.push_back( new_entry );
		}

		else
			LOG->Trace( "Unexpected value named '%s'", sValueName.GetString() );
	}
	static TitleSubst tsub;

	CString ignore;
	tsub.Subst(m_sName, ignore, ignore,
				ignore, ignore, ignore);
}


void Course::AutoGenEndlessFromGroupAndDifficulty( CString sGroupName, Difficulty dc, vector<Song*> &apSongsInGroup )
{
	m_bIsAutoGen = true;
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

	CString sShortGroupName = SONGMAN->ShortenGroupName( sGroupName );	

	m_sName = sShortGroupName + " ";
	switch( dc )
	{
	case DIFFICULTY_EASY:	m_sName += "Easy";		break;
	case DIFFICULTY_MEDIUM:	m_sName += "Medium";	break;
	case DIFFICULTY_HARD:	m_sName += "Hard";		break;
	default:
		ASSERT(0);
	}

	// only need one song because it repeats
	Entry e;
	e.type = Entry::random_within_group;
	e.group_name = sGroupName;
	e.difficulty = dc;
	m_entries.push_back( e );
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
	return true;
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

	for( unsigned i=0; i<entries.size(); i++ )
	{
		const Entry& e = entries[i];

		Song* pSong = NULL;	// fill this in
		Notes* pNotes = NULL;	// fill this in

		vector<Song*> vSongsByMostPlayed = SONGMAN->GetAllSongs();
		SortSongPointerArrayByMostPlayed( vSongsByMostPlayed );


		switch( e.type )
		{
		case Entry::fixed:
			pSong = SONGMAN->GetSongFromDir( e.group_name + "/" + e.song_name );
			break;
		case Entry::random:
		case Entry::random_within_group:
			{
				vector<Song*> vSongs;
				SONGMAN->GetSongs( vSongs, e.group_name );

				if( vSongs.size() == 0 )
					break;

				// probe to find a song with the notes we want
				for( int j=0; j<500; j++ )	// try 500 times before giving up
				{
					pSong = vSongs[rand()%vSongs.size()];
					if( e.difficulty == DIFFICULTY_INVALID )
						pNotes = pSong->GetNotes( nt, e.low_meter, e.high_meter );
					else
						pNotes = pSong->GetNotes( nt, e.difficulty );
					if( pNotes )	// found a match
						break;		// stop searching
					else
					{
						pSong = NULL;
						pNotes = NULL;
					}
				}
			}
			break;
		case Entry::players_best:
		case Entry::players_worst:
			{
				if( e.players_index >= (int)vSongsByMostPlayed.size() )
					break;

				switch( e.type )
				{
				case Entry::players_best:
					pSong = vSongsByMostPlayed[e.players_index];
					break;
				case Entry::players_worst:
					pSong = vSongsByMostPlayed[vSongsByMostPlayed.size()-1-e.players_index];
					break;
				default:
					ASSERT(0);
				}

				if( e.difficulty == DIFFICULTY_INVALID )
					pNotes = pSong->GetNotes( nt, e.low_meter, e.high_meter );
				else
					pNotes = pSong->GetNotes( nt, e.difficulty );

				if( pNotes == NULL )
					pNotes = pSong->GetClosestNotes( nt, DIFFICULTY_MEDIUM );
			}
			break;
		default:
			ASSERT(0);
		}

		if( !pSong || !pNotes )
			continue;	// this song entry isn't playable.  Skip.

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
	case Entry::players_best:			return false;
	case Entry::players_worst:			return false;
	default:		ASSERT(0);			return true;
	}
}

Difficulty Course::GetDifficulty( int stage ) const
{
	return m_entries[stage].difficulty;
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
static bool CompareCoursePointersByDifficulty(const Course* pCourse1, const Course* pCourse2)
{
	return pCourse1->GetEstimatedNumStages() < pCourse2->GetEstimatedNumStages();
}

void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses )
{
	sort( apCourses.begin(), apCourses.end(), CompareCoursePointersByDifficulty );
}
