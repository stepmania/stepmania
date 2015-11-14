#ifndef COURSEUTIL_H
#define COURSEUTIL_H

#include "GameConstantsAndTypes.h"
#include "Difficulty.h"
#include "RageUtil_CachedObject.h"

class Course;
class Profile;
class XNode;
class CourseEntry;
class Song;

bool CompareCoursePointersBySortValueAscending( const Course *pSong1, const Course *pSong2 );
bool CompareCoursePointersBySortValueDescending( const Course *pSong1, const Course *pSong2 );
bool CompareCoursePointersByTitle( const Course *pCourse1, const Course *pCourse2 );

/** @brief Utility functions that deal with Courses. */
namespace CourseUtil
{
	void SortCoursePointerArrayByDifficulty( std::vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByType( std::vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByTitle( std::vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByAvgDifficulty( std::vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByTotalDifficulty( std::vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByRanking( std::vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByNumPlays( std::vector<Course*> &vpCoursesInOut, ProfileSlot slot, bool bDescending );
	void SortCoursePointerArrayByNumPlays( std::vector<Course*> &vpCoursesInOut, const Profile* pProfile, bool bDescending );
	void SortByMostRecentlyPlayedForMachine( std::vector<Course*> &vpCoursesInOut );
	// sm-ssc sort additions:
	//void SortCoursePointerArrayBySectionName( std::vector<Course*> &vpCoursesInOut, SortOrder so );

	void MoveRandomToEnd( std::vector<Course*> &vpCoursesInOut );

	void MakeDefaultEditCourseEntry( CourseEntry &out );

	void AutogenEndlessFromGroup( const std::string &sGroupName, Difficulty dc, Course &out );
	void AutogenNonstopFromGroup( const std::string &sGroupName, Difficulty dc, Course &out );
	void AutogenOniFromArtist( const std::string &sArtistName, std::string sArtistNameTranslit, std::vector<Song*> aSongs, Difficulty dc, Course &out );

	bool ValidateEditCourseName( const std::string &sAnswer, std::string &sErrorOut );

	void WarnOnInvalidMods( std::string sMods );

	// sm-ssc additions:
	//std::string GetSectionNameFromCourseAndSort( const Course *pCourse, SortOrder so );
};

/** @brief Utility functions that deal with Edit Courses. */
namespace EditCourseUtil
{
	void UpdateAndSetTrail();
	void PrepareForPlay();
	void LoadDefaults( Course &out );
	bool RemoveAndDeleteFile( Course *pCourse );
	bool ValidateEditCourseName( const std::string &sAnswer, std::string &sErrorOut );
	void GetAllEditCourses( std::vector<Course*> &vpCoursesOut );
	bool Save( Course *pCourse );
	bool RenameAndSave( Course *pCourse, std::string sName );

	bool ValidateEditCourseNametName( const std::string &sAnswer, std::string &sErrorOut );

	extern int MAX_NAME_LENGTH;
	extern int MAX_PER_PROFILE;
	extern int MIN_WORKOUT_MINUTES;
	extern int MAX_WORKOUT_MINUTES;

	extern bool s_bNewCourseNeedsName;	// if true, we are working with a Course that has never been named
};


class CourseID
{
public:
	CourseID(): sPath(""), sFullTitle(""), m_Cache() { Unset(); }
	void Unset() { FromCourse(nullptr); }
	void FromCourse( const Course *p );
	Course *ToCourse() const;
	const std::string &GetPath() const { return sPath; }
	bool operator<( const CourseID &other ) const
	{
		if (sPath != other.sPath)
			return sPath < other.sPath;
		return sFullTitle < other.sFullTitle;
	}
	bool operator==(const CourseID& rhs) const
	{
		return sPath == rhs.sPath && sFullTitle == rhs.sFullTitle;
	}
	bool operator!=(const CourseID& rhs) const
	{
		return !operator==(rhs);
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	std::string ToString() const;
	bool IsValid() const;

private:
	std::string sPath;
	std::string sFullTitle;
	mutable CachedObjectPointer<Course> m_Cache;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
