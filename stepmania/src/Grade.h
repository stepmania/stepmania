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

enum Grade { GRADE_NO_DATA=0,
			GRADE_E, GRADE_D, GRADE_C, GRADE_B, GRADE_A,
			GRADE_AA,GRADE_AAA,GRADE_AAAA, NUM_GRADES };

CString GradeToString( Grade g );
Grade StringToGrade( const CString &s );

#endif
