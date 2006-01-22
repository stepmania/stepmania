/* CourseUtil - Utility functions that deal with Course. */

#ifndef COURSEUTIL_H
#define COURSEUTIL_H

#include "GameConstantsAndTypes.h"
#include "Difficulty.h"

class Course;
class Profile;
class XNode;
class CourseEntry;
class Song;

namespace CourseUtil
{
	void SortCoursePointerArrayByDifficulty( vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByType( vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByTitle( vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByAvgDifficulty( vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByTotalDifficulty( vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByRanking( vector<Course*> &vpCoursesInOut );
	void SortCoursePointerArrayByNumPlays( vector<Course*> &vpCoursesInOut, ProfileSlot slot, bool bDescending );
	void SortCoursePointerArrayByNumPlays( vector<Course*> &vpCoursesInOut, const Profile* pProfile, bool bDescending );
	void SortByMostRecentlyPlayedForMachine( vector<Course*> &vpCoursesInOut );

	void MoveRandomToEnd( vector<Course*> &vpCoursesInOut );

	void MakeDefaultEditCourseEntry( CourseEntry &out );

	void AutogenEndlessFromGroup( const RString &sGroupName, Difficulty dc, Course &out );
	void AutogenNonstopFromGroup( const RString &sGroupName, Difficulty dc, Course &out );
	void AutogenOniFromArtist( const RString &sArtistName, RString sArtistNameTranslit, vector<Song*> aSongs, Difficulty dc, Course &out );
};

class CourseID
{
public:
	RString sPath;
	RString sFullTitle;

public:
	CourseID() { Unset(); }
	void Unset() { FromCourse(NULL); }
	void FromCourse( const Course *p );
	Course *ToCourse() const;
	bool operator<( const CourseID &other ) const
	{
		return sPath < other.sPath || sFullTitle < other.sFullTitle;
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	RString ToString() const;
	bool IsValid() const;
};

#endif

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
