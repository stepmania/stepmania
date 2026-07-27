#include "global.h"
#include "CourseUtil.h"
#include "Course.h"
#include "RageTimer.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "SongManager.h"
#include "XmlFile.h"
#include "GameState.h"
#include "Style.h"

#include "GameState.h"
#include "LocalizedString.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"
#include "RageFileManager.h"
#include "CourseWriterCRS.h"

#include <vector>


// Sorting stuff
static bool CompareCoursePointersByName( const Course* pCourse1, const Course* pCourse2 )
{
	RString sName1 = pCourse1->GetDisplayFullTitle();
	RString sName2 = pCourse2->GetDisplayFullTitle();
	return sName1.CompareNoCase( sName2 ) < 0;
}

static bool CompareCoursePointersByAutogen( const Course* pCourse1, const Course* pCourse2 )
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

static bool CompareCoursePointersByDifficulty( const Course* pCourse1, const Course* pCourse2 )
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

static bool CompareCoursePointersByTotalDifficulty( const Course* pCourse1, const Course* pCourse2 )
{
	int iNum1 = pCourse1->m_SortOrder_TotalDifficulty;
	int iNum2 = pCourse2->m_SortOrder_TotalDifficulty;

	if( iNum1 == iNum2 )
		return CompareCoursePointersByAutogen( pCourse1, pCourse2 );
	return iNum1 < iNum2;
}

static bool MovePlayersBestToEnd( const Course* pCourse1, const Course* pCourse2 )
{
	bool C1HasBest = pCourse1->CourseHasBestOrWorst();
	bool C2HasBest = pCourse2->CourseHasBestOrWorst();
	if( !C1HasBest && !C2HasBest )
		return false;
	if( C1HasBest && !C2HasBest )
		return false;
	if( !C1HasBest && C2HasBest )
		return true;

	return CompareCoursePointersByName( pCourse1, pCourse2 );
}

static bool CompareRandom( const Course* pCourse1, const Course* pCourse2 )
{
	return ( pCourse1->AllSongsAreFixed() && !pCourse2->AllSongsAreFixed() );
}

static bool CompareCoursePointersByRanking( const Course* pCourse1, const Course* pCourse2 )
{
	int iNum1 = pCourse1->m_SortOrder_Ranking;
	int iNum2 = pCourse2->m_SortOrder_Ranking;

	if( iNum1 == iNum2 )
		return CompareCoursePointersByAutogen( pCourse1, pCourse2 );
	return iNum1 < iNum2;
}

void CourseUtil::SortCoursePointerArrayByDifficulty( std::vector<Course*> &vpCoursesInOut )
{
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByDifficulty );
}

void CourseUtil::SortCoursePointerArrayByRanking( std::vector<Course*> &vpCoursesInOut )
{
	for( unsigned i=0; i<vpCoursesInOut.size(); i++ )
		vpCoursesInOut[i]->UpdateCourseStats( GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType );
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByRanking );
}

void CourseUtil::SortCoursePointerArrayByTotalDifficulty( std::vector<Course*> &vpCoursesInOut )
{
	for( unsigned i=0; i<vpCoursesInOut.size(); i++ )
		vpCoursesInOut[i]->UpdateCourseStats( GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType );
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByTotalDifficulty );
}

// this code isn't ready yet!!
#if 0
RString GetSectionNameFromCourseAndSort( const Course *pCourse, SortOrder so )
{
	if( pCourse == nullptr )
		return RString();
	// more code here
}

void SortCoursePointerArrayBySectionName( std::vector<Course*> &vpCoursesInOut, SortOrder so )
{
	RString sOther = SORT_OTHER.GetValue();
	for(unsigned i = 0; i < vpCoursesInOut.size(); ++i)
	{
		RString val = GetSectionNameFromCourseAndSort( vpCoursesInOut[i], so );

		/* Make sure 0-9 comes first and OTHER comes last. */
		if( val == "0-9" )			val = "0";
		else if( val == sOther )    val = "2";
		else						val = "1" + MakeSortString(val);

		//g_mapSongSortVal[vpSongsInOut[i]] = val;
	}
}
#endif
// ok real code begins again

static bool CompareCoursePointersByType( const Course* pCourse1, const Course* pCourse2 )
{
	return pCourse1->GetPlayMode() < pCourse2->GetPlayMode();
}

void CourseUtil::SortCoursePointerArrayByType( std::vector<Course*> &vpCoursesInOut )
{
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByType );
}

void CourseUtil::MoveRandomToEnd( std::vector<Course*> &vpCoursesInOut )
{
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareRandom );
}

static std::map<const Course*, RString> course_sort_val;

bool CompareCoursePointersBySortValueAscending( const Course *pSong1, const Course *pSong2 )
{
	return course_sort_val[pSong1] < course_sort_val[pSong2];
}

bool CompareCoursePointersBySortValueDescending( const Course *pSong1, const Course *pSong2 )
{
	return course_sort_val[pSong1] > course_sort_val[pSong2];
}

bool CompareCoursePointersByTitle( const Course *pCourse1, const Course *pCourse2 )
{
	return CompareCoursePointersByName( pCourse1, pCourse2 );
}

void CourseUtil::SortCoursePointerArrayByTitle( std::vector<Course*> &vpCoursesInOut )
{
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByTitle );
}

void CourseUtil::SortCoursePointerArrayByAvgDifficulty( std::vector<Course*> &vpCoursesInOut )
{
	RageTimer foo;
	course_sort_val.clear();
	for( unsigned i = 0; i < vpCoursesInOut.size(); ++i )
	{
		int iMeter = vpCoursesInOut[i]->GetMeter( GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType, Difficulty_Medium );
		course_sort_val[vpCoursesInOut[i]] = ssprintf( "%06i", iMeter );
	}
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByTitle );
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersBySortValueAscending );

	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), MovePlayersBestToEnd );
}

void CourseUtil::SortCoursePointerArrayByNumPlays( std::vector<Course*> &vpCoursesInOut, ProfileSlot slot, bool bDescending )
{
	if( !PROFILEMAN->IsPersistentProfile(slot) )
		return;	// nothing to do since we don't have data
	Profile* pProfile = PROFILEMAN->GetProfile(slot);
	SortCoursePointerArrayByNumPlays( vpCoursesInOut, pProfile, bDescending );
}

void CourseUtil::SortCoursePointerArrayByNumPlays( std::vector<Course*> &vpCoursesInOut, const Profile* pProfile, bool bDescending )
{
	ASSERT( pProfile != nullptr );
	for(unsigned i = 0; i < vpCoursesInOut.size(); ++i)
		course_sort_val[vpCoursesInOut[i]] = ssprintf( "%09i", pProfile->GetCourseNumTimesPlayed(vpCoursesInOut[i]) );
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), bDescending ? CompareCoursePointersBySortValueDescending : CompareCoursePointersBySortValueAscending );
	course_sort_val.clear();
}

void CourseUtil::SortByMostRecentlyPlayedForMachine( std::vector<Course*> &vpCoursesInOut )
{
	Profile *pProfile = PROFILEMAN->GetMachineProfile();

	for (Course const * c: vpCoursesInOut)
	{
		int iNumTimesPlayed = pProfile->GetCourseNumTimesPlayed( c );
		RString val = iNumTimesPlayed ? pProfile->GetCourseLastPlayedDateTime(c).GetString() : RString("9999999999999");
		course_sort_val[c] = val;
	}

	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersBySortValueAscending );
	course_sort_val.clear();
}

void CourseUtil::MakeDefaultEditCourseEntry( CourseEntry& out )
{
	out.songID.FromSong( GAMESTATE->GetDefaultSong() );
	out.stepsCriteria.m_difficulty = Difficulty_Medium;
}

//////////////////////////////////
// Autogen
//////////////////////////////////

void CourseUtil::AutogenEndlessFromGroup( const RString &sGroupName, Difficulty diff, Course &out )
{
	out.m_bIsAutogen = true;
	out.m_bRepeat = true;
	out.m_bShuffle = true;
	out.m_iLives = -1;
	FOREACH_ENUM( Difficulty,dc)
		out.m_iCustomMeter[dc] = -1;

	if( sGroupName == "" )
	{
		out.m_sMainTitle = "All Songs";
		// this sounds reasonable... -aj
		out.m_sBannerPath = THEME->GetPathG("Banner","all music");
	}
	else
	{
		out.m_sMainTitle = SONGMAN->ShortenGroupName( sGroupName );
		out.m_sBannerPath = SONGMAN->GetSongGroupBannerPath( sGroupName );
	}

	// We want multiple songs, so we can try to prevent repeats during
	// gameplay. (We might still get a repeat at the repeat boundary,
	// but that'd be rare.) -glenn
	CourseEntry e;
	e.songCriteria.m_vsGroupNames.push_back(sGroupName);
	e.stepsCriteria.m_difficulty = diff;
	e.bSecret = true;

	// Insert a copy of e for each song in the group.
	out.m_vEntries.insert( out.m_vEntries.end(), SONGMAN->GetSongs(sGroupName).size(), e );
}

void CourseUtil::AutogenNonstopFromGroup( const RString &sGroupName, Difficulty diff, Course &out )
{
	AutogenEndlessFromGroup( sGroupName, diff, out );

	out.m_bRepeat = false;

	out.m_sMainTitle += " Random";

	// resize to 4
	while( out.m_vEntries.size() < 4 )
		out.m_vEntries.push_back( out.m_vEntries[0] );
	while( out.m_vEntries.size() > 4 )
		out.m_vEntries.pop_back();
}

void CourseUtil::AutogenOniFromArtist( const RString &sArtistName, RString sArtistNameTranslit, std::vector<Song*> aSongs, Difficulty dc, Course &out )
{
	out.m_bIsAutogen = true;
	out.m_bRepeat = false;
	out.m_bShuffle = true;
	out.m_bSortByMeter = true;

	out.m_iLives = 4;
	FOREACH_ENUM( Difficulty,cd)
		out.m_iCustomMeter[cd] = -1;

	ASSERT( sArtistName != "" );
	ASSERT( aSongs.size() > 0 );

	/* "Artist Oni" is a little repetitive; "by Artist" stands out less, and
	 * lowercasing "by" puts more emphasis on the artist's name. It also sorts
	 * them together. */
	out.m_sMainTitle = "by " + sArtistName;
	if( sArtistNameTranslit != sArtistName )
		out.m_sMainTitleTranslit = "by " + sArtistNameTranslit;

	// How would we handle Artist Oni course banners, anyways? -aj
	// m_sBannerPath = ""; // XXX

	/* Shuffle the list to determine which songs we'll use. Shuffle it
	 * deterministically, so we always get the same set of songs unless the
	 * song set changes. */
	{
		RandomGen rng( GetHashForString( sArtistName ) + aSongs.size() );
		std::shuffle( aSongs.begin(), aSongs.end(), rng );
	}

	// Only use up to four songs.
	if( aSongs.size() > 4 )
		aSongs.erase( aSongs.begin()+4, aSongs.end() );

	CourseEntry e;
	e.stepsCriteria.m_difficulty = dc;

	for( unsigned i = 0; i < aSongs.size(); ++i )
	{
		e.songID.FromSong( aSongs[i] );
		out.m_vEntries.push_back( e );
	}
}

void CourseUtil::WarnOnInvalidMods( RString sMods )
{
	PlayerOptions po;
	SongOptions so;
	std::vector<RString> vs;
	split( sMods, ",", vs, true );
	for (RString const &s : vs)
	{
		bool bValid = false;
		RString sErrorDetail;
		bValid |= po.FromOneModString( s, sErrorDetail );
		bValid |= so.FromOneModString( s, sErrorDetail );
		/* ==Invalid options that used to be valid==
		 * all noteskins (solo, note, foon, &c.)
		 * protiming (done in Lua now)
		 * ==Things I've seen in real course files==
		 * 900% BRINK (damnit japan)
		 * TISPY
		 * 9000% OVERCOMING (what?)
		 * 9200% TORNADE (it's like a grenade but a tornado)
		 * 50% PROTIMING (HOW THE HELL DOES 50% PROTIMING EVEN WORK)
		 * BREAK
		 */
		if( !bValid )
		{
			RString sFullError = ssprintf("Error processing '%s' in '%s'", s.c_str(), sMods.c_str() );
			if( !sErrorDetail.empty() )
				sFullError += ": " + sErrorDetail;
			LOG->UserLog( "", "", "%s", sFullError.c_str() );
			Dialog::OK( sFullError, "INVALID_PLAYER_OPTION_WARNING" );
		}
	}
}

int EditCourseUtil::MAX_NAME_LENGTH = 16;
int EditCourseUtil::MAX_PER_PROFILE = 32;
int EditCourseUtil::MIN_WORKOUT_MINUTES = 4;
int EditCourseUtil::MAX_WORKOUT_MINUTES = 90;
bool EditCourseUtil::s_bNewCourseNeedsName = false;

bool EditCourseUtil::Save( Course *pCourse )
{
	return EditCourseUtil::RenameAndSave( pCourse, pCourse->GetDisplayFullTitle() );
}

bool EditCourseUtil::RenameAndSave( Course *pCourse, RString sNewName )
{
	ASSERT( !sNewName.empty() );

	EditCourseUtil::s_bNewCourseNeedsName = false;

	RString sNewFilePath;
	if( pCourse->IsAnEdit() )
	{
		sNewFilePath = PROFILEMAN->GetProfileDir(ProfileSlot_Machine) + EDIT_COURSES_SUBDIR + sNewName + ".crs";
	}
	else
	{
		RString sDir, sName, sExt;
		splitpath( pCourse->m_sPath, sDir, sName, sExt );
		sNewFilePath = sDir + sNewName + sExt;
	}

	// remove the old file if the name is changing
	if( !pCourse->m_sPath.empty()  &&  sNewFilePath != pCourse->m_sPath )
		FILEMAN->Remove( pCourse->m_sPath );	// not fatal if this fails

	pCourse->m_sMainTitle = sNewName;
	pCourse->m_sPath = sNewFilePath;
	return CourseWriterCRS::Write( *pCourse, pCourse->m_sPath, false );
}

bool EditCourseUtil::RemoveAndDeleteFile( Course *pCourse )
{
	if( !FILEMAN->Remove( pCourse->m_sPath ) )
		return false;
	FILEMAN->Remove( pCourse->GetCacheFilePath() );
	if( pCourse->IsAnEdit() )
	{
		PROFILEMAN->LoadMachineProfile();
	}
	else
	{
		SONGMAN->DeleteCourse( pCourse );
		delete pCourse;
	}
	return true;
}

static LocalizedString YOU_MUST_SUPPLY_NAME	( "CourseUtil", "You must supply a name for your course." );
static LocalizedString EDIT_NAME_CONFLICTS	( "CourseUtil", "The name you chose conflicts with another course. Please use a different name." );
static LocalizedString EDIT_NAME_CANNOT_CONTAIN	( "CourseUtil", "The course name cannot contain any of the following characters: %s" );
bool EditCourseUtil::ValidateEditCourseName( const RString &sAnswer, RString &sErrorOut )
{
	if( sAnswer.empty() )
	{
		sErrorOut = YOU_MUST_SUPPLY_NAME;
		return false;
	}

	static const RString sInvalidChars = "\\/:*?\"<>|";
	if( strpbrk(sAnswer, sInvalidChars) != nullptr )
	{
		sErrorOut = ssprintf( EDIT_NAME_CANNOT_CONTAIN.GetValue(), sInvalidChars.c_str() );
		return false;
	}

	// Check for name conflicts
	std::vector<Course*> vpCourses;
	EditCourseUtil::GetAllEditCourses( vpCourses );
	for (Course const *p : vpCourses)
	{
		if( GAMESTATE->m_pCurCourse == p )
			continue;	// don't comepare name against ourself

		if( p->GetDisplayFullTitle() == sAnswer )
		{
			sErrorOut = EDIT_NAME_CONFLICTS;
			return false;
		}
	}

	return true;
}

void EditCourseUtil::UpdateAndSetTrail()
{
	ASSERT( GAMESTATE->GetCurrentStyle(PLAYER_INVALID) != nullptr );
	StepsType st = GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType;
	Trail *pTrail = nullptr;
	if( GAMESTATE->m_pCurCourse )
		pTrail = GAMESTATE->m_pCurCourse->GetTrailForceRegenCache( st );
	GAMESTATE->m_pCurTrail[PLAYER_1].Set( pTrail );
}

void EditCourseUtil::PrepareForPlay()
{
	GAMESTATE->m_pCurSong.Set(nullptr);	// CurSong will be set if we back out.  Set it back to nullptr so that ScreenStage won't show the last song.
	GAMESTATE->m_PlayMode.Set( PLAY_MODE_ENDLESS );
	GAMESTATE->m_bSideIsJoined[0] = true;

	PROFILEMAN->GetProfile(ProfileSlot_Player1)->m_GoalType = GoalType_Time;
	Course *pCourse = GAMESTATE->m_pCurCourse;
	PROFILEMAN->GetProfile(ProfileSlot_Player1)->m_iGoalSeconds = static_cast<int>(pCourse->m_fGoalSeconds);
}

void EditCourseUtil::GetAllEditCourses( std::vector<Course*> &vpCoursesOut )
{
	std::vector<Course*> vpCoursesTemp;
	SONGMAN->GetAllCourses( vpCoursesTemp, false );
	for (Course *c : vpCoursesTemp)
	{
		if( c->GetLoadedFromProfileSlot() != ProfileSlot_Invalid )
			vpCoursesOut.push_back( c );
	}
}

void EditCourseUtil::LoadDefaults( Course &out )
{
	out = Course();

	out.m_fGoalSeconds = 0;

	// pick a default name
	// XXX: Make this localizable
	for( int i=0; i<10000; i++ )
	{
		out.m_sMainTitle = ssprintf("Workout %d", i+1);

		std::vector<Course*> vpCourses;
		EditCourseUtil::GetAllEditCourses( vpCourses );

		if (std::any_of(vpCourses.begin(), vpCourses.end(), [&](Course const *p) { return out.m_sMainTitle == p->m_sMainTitle; }))
			break;
	}

	std::vector<Song*> vpSongs;
	SONGMAN->GetPreferredSortSongs( vpSongs );
	for( int i=0; i<(int)vpSongs.size() && i<6; i++ )
	{
		CourseEntry ce;
		ce.songID.FromSong( vpSongs[i] );
		ce.stepsCriteria.m_difficulty = Difficulty_Easy;
		out.m_vEntries.push_back( ce );
	}
}

//////////////////////////////////
// CourseID
//////////////////////////////////

void CourseID::FromCourse( const Course *p )
{
	if( p )
	{
		if( p->m_bIsAutogen )
		{
			sPath = "";
			sFullTitle = p->GetTranslitFullTitle();
		}
		else
		{
			sPath = p->m_sPath;
			sFullTitle = "";
		}
	}
	else
	{
		sPath = "";
		sFullTitle = "";
	}

	// HACK for backwards compatibility:
	// Strip off leading "/".  2005/05/21 file layer changes added a leading slash.
	if( sPath.Left(1) == "/" )
		sPath.erase( sPath.begin() );
}

Course *CourseID::ToCourse() const
{
	Course *pCourse = nullptr;
	if(!sPath.empty())
	{
		// HACK for backwards compatibility:
		// Re-add the leading "/".  2005/05/21 file layer changes added a leading slash.
		RString slash_path = sPath;
		if(slash_path.Left(1) != "/")
		{
			slash_path = "/" + slash_path;
		}

		pCourse = SONGMAN->GetCourseFromPath(slash_path);
	}

	if( pCourse == nullptr && !sFullTitle.empty() )
	{
		pCourse = SONGMAN->GetCourseFromName( sFullTitle );
	}

	return pCourse;
}

XNode* CourseID::CreateNode() const
{
	XNode* pNode = new XNode( "Course" );;

	if( !sPath.empty() )
		pNode->AppendAttr( "Path", sPath );
	if( !sFullTitle.empty() )
		pNode->AppendAttr( "FullTitle", sFullTitle );

	return pNode;
}

void CourseID::LoadFromNode( const XNode* pNode )
{
	ASSERT( pNode->GetName() == "Course" );
	sFullTitle = RString();
	sPath = RString();
	if( !pNode->GetAttrValue("Path", sPath) )
		pNode->GetAttrValue( "FullTitle", sFullTitle );

	// HACK for backwards compatibility: /AdditionalCourses has been merged into /Courses
	if (sPath.Left(18) == "AdditionalCourses/")
		sPath.replace(0, 18, "Courses/");
}

RString CourseID::ToString() const
{
	if( !sPath.empty() )
		return sPath;
	if( !sFullTitle.empty() )
		return sFullTitle;
	return RString();
}

bool CourseID::IsValid() const
{
	return !sPath.empty() || !sFullTitle.empty();
}

/*
 * (c) 2001-2004 Chris Danford
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
