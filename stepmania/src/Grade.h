/*
-----------------------------------------------------------------------------
 Class: Grade

 Desc: This a mark the player receives after clearing a song.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#ifndef _Grade_H_
#define _Grade_H_




enum Grade { GRADE_NO_DATA=0, GRADE_E, GRADE_D, GRADE_C, GRADE_B, GRADE_A, GRADE_AA, GRADE_AAA };

CString GradeToString( Grade g );
Grade StringToGrade( const CString &s );



#endif
