#ifndef COURSEUTIL_H
#define COURSEUTIL_H
/*
-----------------------------------------------------------------------------
 Class: CourseUtil

 Desc: A queue of songs and notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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
	CString sName;

public:
	CourseID() { Unset(); }
	void Unset() { FromCourse(NULL); }
	void FromCourse( const Course *p );
	Course *ToCourse() const;
	bool operator<( const CourseID &other ) const
	{
		return sPath < other.sPath || sName < other.sName;
	}

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	bool IsValid() const;
};

#endif
