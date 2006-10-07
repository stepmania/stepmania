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
#include "Foreach.h"
#include "GameState.h"
#include "LocalizedString.h"


//
// Sorting stuff
//
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

void CourseUtil::SortCoursePointerArrayByDifficulty( vector<Course*> &vpCoursesInOut )
{
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByDifficulty );
}

void CourseUtil::SortCoursePointerArrayByRanking( vector<Course*> &vpCoursesInOut )
{
	for( unsigned i=0; i<vpCoursesInOut.size(); i++ )
		vpCoursesInOut[i]->UpdateCourseStats( GAMESTATE->GetCurrentStyle()->m_StepsType );
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByRanking );
}

void CourseUtil::SortCoursePointerArrayByTotalDifficulty( vector<Course*> &vpCoursesInOut )
{
	for( unsigned i=0; i<vpCoursesInOut.size(); i++ )
		vpCoursesInOut[i]->UpdateCourseStats( GAMESTATE->GetCurrentStyle()->m_StepsType );
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByTotalDifficulty );
}

static bool CompareCoursePointersByType( const Course* pCourse1, const Course* pCourse2 )
{
	return pCourse1->GetPlayMode() < pCourse2->GetPlayMode();
}

void CourseUtil::SortCoursePointerArrayByType( vector<Course*> &vpCoursesInOut )
{
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByType );
}

void CourseUtil::MoveRandomToEnd( vector<Course*> &vpCoursesInOut )
{
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareRandom );
}

static map<const Course*, RString> course_sort_val;

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

void CourseUtil::SortCoursePointerArrayByTitle( vector<Course*> &vpCoursesInOut )
{
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByTitle );
}

void CourseUtil::SortCoursePointerArrayByAvgDifficulty( vector<Course*> &vpCoursesInOut )
{
	RageTimer foo;
	course_sort_val.clear();
	for( unsigned i = 0; i < vpCoursesInOut.size(); ++i )
	{
		int iMeter = vpCoursesInOut[i]->GetMeter( GAMESTATE->GetCurrentStyle()->m_StepsType, COURSE_DIFFICULTY_REGULAR );
		course_sort_val[vpCoursesInOut[i]] = ssprintf( "%06i", iMeter );
	}
	sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersByTitle );
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersBySortValueAscending );

	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), MovePlayersBestToEnd );
}
 

void CourseUtil::SortCoursePointerArrayByNumPlays( vector<Course*> &vpCoursesInOut, ProfileSlot slot, bool bDescending )
{
	if( !PROFILEMAN->IsPersistentProfile(slot) )
		return;	// nothing to do since we don't have data
	Profile* pProfile = PROFILEMAN->GetProfile(slot);
	SortCoursePointerArrayByNumPlays( vpCoursesInOut, pProfile, bDescending );
}

void CourseUtil::SortCoursePointerArrayByNumPlays( vector<Course*> &vpCoursesInOut, const Profile* pProfile, bool bDescending )
{
	ASSERT( pProfile );
	for(unsigned i = 0; i < vpCoursesInOut.size(); ++i)
		course_sort_val[vpCoursesInOut[i]] = ssprintf( "%09i", pProfile->GetCourseNumTimesPlayed(vpCoursesInOut[i]) );
	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), bDescending ? CompareCoursePointersBySortValueDescending : CompareCoursePointersBySortValueAscending );
	course_sort_val.clear();
}

void CourseUtil::SortByMostRecentlyPlayedForMachine( vector<Course*> &vpCoursesInOut )
{
	Profile *pProfile = PROFILEMAN->GetMachineProfile();

	FOREACH_CONST( Course*, vpCoursesInOut, c )
	{
		int iNumTimesPlayed = pProfile->GetCourseNumTimesPlayed( *c );
		RString val = iNumTimesPlayed ? pProfile->GetCourseLastPlayedDateTime(*c).GetString() : "9999999999999";
		course_sort_val[*c] = val;
	}

	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersBySortValueAscending );
	course_sort_val.clear();
}

void CourseUtil::MakeDefaultEditCourseEntry( CourseEntry& out )
{
	out.pSong = GAMESTATE->GetDefaultSong();
	out.stepsCriteria.m_difficulty = DIFFICULTY_MEDIUM;
}

//////////////////////////////////
// Autogen
//////////////////////////////////

void CourseUtil::AutogenEndlessFromGroup( const RString &sGroupName, CourseDifficulty cd, Course &out )
{
	out.m_bIsAutogen = true;
	out.m_bRepeat = true;
	out.m_bShuffle = true;
	out.m_iLives = -1;
	FOREACH_Difficulty(dc)
		out.m_iCustomMeter[dc] = -1;

	if( sGroupName == "" )
	{
		out.m_sMainTitle = "All Songs";
		// m_sBannerPath = ""; // XXX
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
	e.songCriteria.m_sGroupName = sGroupName;
	e.stepsCriteria.m_difficulty = (Difficulty)cd;
	e.bSecret = true;

	vector<Song*> vSongs;
	SONGMAN->GetSongs( vSongs, e.songCriteria.m_sGroupName );
	for( unsigned i = 0; i < vSongs.size(); ++i)
		out.m_vEntries.push_back( e );
}

void CourseUtil::AutogenNonstopFromGroup( const RString &sGroupName, CourseDifficulty cd, Course &out )
{
	AutogenEndlessFromGroup( sGroupName, cd, out );

	out.m_bRepeat = false;

	out.m_sMainTitle += " Random";	

	// resize to 4
	while( out.m_vEntries.size() < 4 )
		out.m_vEntries.push_back( out.m_vEntries[0] );
	while( out.m_vEntries.size() > 4 )
		out.m_vEntries.pop_back();
}

void CourseUtil::AutogenOniFromArtist( const RString &sArtistName, RString sArtistNameTranslit, vector<Song*> aSongs, Difficulty dc, Course &out )
{
	out.m_bIsAutogen = true;
	out.m_bRepeat = false;
	out.m_bShuffle = true;
	out.m_bSortByMeter = true;

	out.m_iLives = 4;
	FOREACH_Difficulty(cd)
		out.m_iCustomMeter[cd] = -1;

	ASSERT( sArtistName != "" );
	ASSERT( aSongs.size() > 0 );

	/* "Artist Oni" is a little repetitive; "by Artist" stands out less, and lowercasing
	 * "by" puts more emphasis on the artist's name.  It also sorts them together. */
	out.m_sMainTitle = "by " + sArtistName;
	if( sArtistNameTranslit != sArtistName )
		out.m_sMainTitleTranslit = "by " + sArtistNameTranslit;


	// m_sBannerPath = ""; // XXX

	/* Shuffle the list to determine which songs we'll use.  Shuffle it deterministically,
	 * so we always get the same set of songs unless the song set changes. */
	{
		RandomGen rng( GetHashForString( sArtistName ) + aSongs.size() );
		random_shuffle( aSongs.begin(), aSongs.end(), rng );
	}

	/* Only use up to four songs. */
	if( aSongs.size() > 4 )
		aSongs.erase( aSongs.begin()+4, aSongs.end() );

	CourseEntry e;
	e.stepsCriteria.m_difficulty = dc;

	for( unsigned i = 0; i < aSongs.size(); ++i )
	{
		e.pSong = aSongs[i];
		out.m_vEntries.push_back( e );
	}
}

static LocalizedString EDIT_NAME_CONFLICTS	( "ScreenOptionsManageCourses", "The name you chose conflicts with another edit. Please use a different name." );
bool CourseUtil::ValidateEditCourseName( const RString &sAnswer, RString &sErrorOut )
{
	if( sAnswer.empty() )
		return false;

	// Course name must be unique
	vector<Course*> v;
	SONGMAN->GetAllCourses( v, false );
	FOREACH_CONST( Course*, v, c )
	{
		if( GAMESTATE->m_pCurCourse.Get() == *c )
			continue;	// don't compare name against ourself

		if( (*c)->GetDisplayFullTitle() == sAnswer )
		{
			sErrorOut = EDIT_NAME_CONFLICTS;
			return false;
		}
	}

	return true;
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
	// HACK for backwards compatibility:
	// Re-add the leading "/".  2005/05/21 file layer changes added a leading slash.
	RString sPath2 = sPath;
	if( sPath2.Left(1) != "/" )
		sPath2 = "/" + sPath2;

	if( !sPath2.empty() )
	{
		Course *pCourse = SONGMAN->GetCourseFromPath( sPath2 );
		if( pCourse ) 
			return pCourse;
	}

	if( !sFullTitle.empty() )
	{
		Course *pCourse = SONGMAN->GetCourseFromName( sFullTitle );
		if( pCourse ) 
			return pCourse;
	}

	return NULL;
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
	pNode->GetAttrValue("Path", sPath);
	pNode->GetAttrValue("FullTitle", sFullTitle);
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
