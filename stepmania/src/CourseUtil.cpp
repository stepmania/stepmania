#include "global.h"
#include "CourseUtil.h"
#include "Course.h"
#include "RageTimer.h"
#include "ProfileManager.h"
#include "SongManager.h"
#include "XmlFile.h"
#include "GameState.h"
#include "Style.h"
#include "Foreach.h"


//
// Sorting stuff
//
static bool CompareCoursePointersByName( const Course* pCourse1, const Course* pCourse2 )
{
	// HACK:  strcmp and other string comparators appear to eat whitespace.
	// For example, the string "Players Best 13-16" is sorted between 
	// "Players Best  1-4" and "Players Best  5-8".  Replace the string "  "
	// with " 0" for comparison only.

	// XXX: That doesn't happen to me, and it shouldn't (strcmp is strictly
	// a byte sort, though CompareNoCase doesn't use strcmp).  Are you sure
	// you didn't have only one space before? -glenn
	CString sName1 = pCourse1->GetFullDisplayTitle();
	CString sName2 = pCourse2->GetFullDisplayTitle();
	sName1.Replace( "  " , " 0" );
	sName2.Replace( "  " , " 0" );
	return sName1.CompareNoCase( sName2 ) == -1;
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
	return ( pCourse1->IsFixed() && !pCourse2->IsFixed() );
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

static map<const Course*, CString> course_sort_val;

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
		const Trail* pTrail = vpCoursesInOut[i]->GetTrail( GAMESTATE->GetCurrentStyle()->m_StepsType );
		course_sort_val[vpCoursesInOut[i]] = ssprintf("%09.3f", pTrail != NULL? (float) pTrail->GetMeter(): 0.0f);
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
		CString val = iNumTimesPlayed ? pProfile->GetCourseLastPlayedDateTime(*c).GetString() : "9999999999999";
		course_sort_val[*c] = val;
	}

	stable_sort( vpCoursesInOut.begin(), vpCoursesInOut.end(), CompareCoursePointersBySortValueAscending );
	course_sort_val.clear();
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
			sFullTitle = p->GetFullTranslitTitle();
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
}

Course *CourseID::ToCourse() const
{
	if( !sPath.empty() )
	{
		Course *pCourse = SONGMAN->GetCourseFromPath( sPath );
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
	XNode* pNode = new XNode;
	pNode->m_sName = "Course";

	if( !sPath.empty() )
		pNode->AppendAttr( "Path", sPath );
	if( !sFullTitle.empty() )
		pNode->AppendAttr( "FullTitle", sFullTitle );

	return pNode;
}

void CourseID::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->m_sName == "Course" );
	pNode->GetAttrValue("Path", sPath);
	pNode->GetAttrValue("FullTitle", sFullTitle);
}

CString CourseID::ToString() const
{
	if( !sPath.empty() )
		return sPath;
	if( !sFullTitle.empty() )
		return sFullTitle;
	return "";
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
