/*
-----------------------------------------------------------------------------
 File: Grade.h

 Desc: A graphic displayed in the Grade during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Grade_H_
#define _Grade_H_




enum Grade { GRADE_NO_DATA=0, GRADE_E, GRADE_D, GRADE_C, GRADE_B, GRADE_A, GRADE_AA, GRADE_AAA };

CString GradeToString( Grade g );
Grade StringToGrade( CString s );



#endif
