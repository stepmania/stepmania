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
			CString sSongDir = sParams[1];
			Difficulty dc = StringToDifficulty( sParams[2] );
			CString sModifiers = sParams[3];

			if( sSongDir.GetLength() == 0 ) {
			    /* Err. */
			    LOG->Trace( "Course file '%s' has an empty #SONG.  Ignored.", sPath.GetString(), sSongDir.GetString() );
			    continue;
			}
			
			m_entries.push_back( course_entry(sSongDir, dc, sModifiers) );
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

	for( unsigned s=0; s<apSongsInGroup.size(); s++ )
	{
		Song* pSong = apSongsInGroup[s];
		m_entries.push_back( course_entry(pSong->GetSongDir(), dc, "") );
	}
}


bool Course::HasDifficult() const
{
	for( unsigned i=0; i<m_entries.size(); i++ )
		if( m_entries[i].difficulty >= DIFFICULTY_HARD )
			return false;
	return true;
}


void Course::GetCourseInfo( 
	vector<Song*>& vSongsOut, 
	vector<Notes*>& vNotesOut, 
	vector<CString>& vsModifiersOut, 
	NotesType nt, 
	bool bDifficult ) const
{
	if( bDifficult )
		ASSERT( HasDifficult() );


	vector<course_entry> entries = m_entries;

	if( m_bRandomize )
		random_shuffle( entries.begin(), entries.end() );

	for( unsigned i=0; i<entries.size(); i++ )
	{
		CString sSongDir = entries[i].songDir;

		Song* pSong;
		if( sSongDir.Left(strlen("PlayersBest")) == "PlayersBest" )
		{
			int iNumber = atoi( sSongDir.Right(sSongDir.length()-strlen("PlayersBest")) );
			int index = iNumber - 1;
			pSong = SONGMAN->GetPlayersBest( index );
		}
		else if( sSongDir.Left(strlen("PlayersWorst")) == "PlayersWorst" )
		{
			int iNumber = atoi( sSongDir.Right(sSongDir.length()-strlen("PlayersWorst")) );
			int index = iNumber - 1;
			pSong = SONGMAN->GetPlayersWorst( index );
		}
		else if( sSongDir.Right(1) == "*" )
		{
			CStringArray asSongDirs;
			GetDirListing( sSongDir, asSongDirs, true, true );
			if( asSongDirs.empty() )
				pSong = NULL;
			else
				pSong = FindSong( asSongDirs[rand()%asSongDirs.size()] );
		}
		else
			pSong = FindSong( sSongDir );

		if( pSong == NULL )
			continue;	// skip


		Difficulty dc = entries[i].difficulty;
		if( bDifficult  &&  dc != NUM_DIFFICULTIES-1 )
			dc = (Difficulty)(dc+1);
		Notes* pNotes = pSong->GetNotes( nt, dc );

		if( pNotes == NULL )
			continue;	// skip


		CString sModifiers = entries[i].modifiers;

		vSongsOut.push_back( pSong ); 
		vNotesOut.push_back( pNotes ); 
		vsModifiersOut.push_back( sModifiers );
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

	GetCourseInfo(
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
	CString sSongDir = m_entries[stage].songDir;
	return sSongDir.Right(1) == "*";
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
	GetCourseInfo(
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
	m_MemCardScores[MEMORY_CARD_MACHINE][nt].iNumTimesPlayed++;


	vector<RankingToInsert> vHS;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		iRankingIndexOut[p] = -1;
		bNewRecordOut = false;

		if( !bPlayerEnabled[p] )
			continue;	// skip
        

		// Update memory card
		m_MemCardScores[p][nt].iNumTimesPlayed++;

		if( iDancePoints[p] > m_MemCardScores[p][nt].iDancePoints )
		{
			m_MemCardScores[p][nt].iDancePoints = iDancePoints[p];
			m_MemCardScores[p][nt].fSurviveTime = fSurviveTime[p];
			bNewRecordOut[p] = true;
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
				for( int j=i+1; j<NUM_RANKING_LINES; j++ )
					rankingScores[j] = rankingScores[j-1];
				// insert
				rankingScores[i].fSurviveTime = newHS.fSurviveTime;
				rankingScores[i].iDancePoints = newHS.iDancePoints;
				rankingScores[i].sName = DEFAULT_RANKING_NAME;
				iRankingIndexOut[newHS.pn] = i;
			}
		}
	}
}


//
// Sorting stuff
//
static int CompareCoursePointersByDifficulty(const Course* pCourse1, const Course* pCourse2)
{
	return pCourse1->GetEstimatedNumStages() < pCourse2->GetEstimatedNumStages();
}

void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses )
{
	sort( apCourses.begin(), apCourses.end(), CompareCoursePointersByDifficulty );
}
