/*
-----------------------------------------------------------------------------
 Class: Grade

 Desc: This a mark the player receives after clearing a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#ifndef GRADE_H
#define GRADE_H

#include "RageUtil.h"

#define NUM_GRADE_TIERS 20
enum Grade 
{ 
	GRADE_TIER_1,	// = AAAA
	GRADE_TIER_2,	// = AAA
	GRADE_TIER_3,	// = AA
	GRADE_TIER_4,	// = A
	GRADE_TIER_5,	// = B
	GRADE_TIER_6,	// = C
	GRADE_TIER_7,	// = D
	GRADE_TIER_8,
	GRADE_TIER_9,
	GRADE_TIER_10,
	GRADE_TIER_11,
	GRADE_TIER_12,
	GRADE_TIER_13,
	GRADE_TIER_14,
	GRADE_TIER_15,
	GRADE_TIER_16,
	GRADE_TIER_17,
	GRADE_TIER_18,
	GRADE_TIER_19,
	GRADE_TIER_20,
	GRADE_FAILED,	// = E
	NUM_GRADES, 
	GRADE_NO_DATA,	// ~GRADE_INVALID
};

CString GradeToString( Grade g );
CString GradeToOldString( Grade g );	// "AAA", "B", etc for backward compatibility
CString GradeToThemedString( Grade g );
Grade StringToGrade( const CString &s );
#define FOREACH_Grade( g ) FOREACH_ENUM( Grade, NUM_GRADES, g )

#endif
