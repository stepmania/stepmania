/* CourseUtil - Utility functions that deal with Course. */

#ifndef COURSEUTIL_H
#define COURSEUTIL_H

#include "GameConstantsAndTypes.h"

class Course;
class Profile;
struct XNode;

namespace CourseUtil
{
	void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses );
	void SortCoursePointerArrayByType( vector<Course*> &apCourses );
	void SortCoursePointerArrayByAvgDifficulty( vector<Course*> &apCourses );
	void SortCoursePointerArrayByTotalDifficulty( vector<Course*> &apCourses );
	void SortCoursePointerArrayByRanking( vector<Course*> &apCourses );
	void SortCoursePointerArrayByNumPlays( vector<Course*> &arrayCoursePointers, ProfileSlot slot, bool bDescending );
	void SortCoursePointerArrayByNumPlays( vector<Course*> &arrayCoursePointers, const Profile* pProfile, bool bDescending );

	void MoveRandomToEnd( vector<Course*> &apCourses );
};

class CourseID
{
	CString sPath;
	CString sFullTitle;

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
	CString ToString() const;
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
