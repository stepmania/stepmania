#ifndef TrialUtil_H
#define TrialUtil_H
/*
-----------------------------------------------------------------------------
 Class: TrialUtil

 Desc: An queue of Songs and Steps that are generated from a Course.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"

class Trail;
class Course;
struct XNode;

class TrailID
{
	StepsType st;
	CourseDifficulty cd;

public:
	TrailID() { Unset(); }
	void Unset() { FromTrail(NULL); }
	void FromTrail( const Trail *p );
	Trail *ToTrail( const Course *p, bool bAllowNull ) const;
	bool operator<( const TrailID &rhs ) const;
	bool MatchesStepsType( StepsType s ) const { return st == s; }

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	CString ToString() const;
	bool IsValid() const;
	static void FlushCache();
};

#endif
