/*
-----------------------------------------------------------------------------
 File: Grade.h

 Desc: A graphic displayed in the Grade during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _Grade_H_
#define _Grade_H_


#include "Sprite.h"
#include "ThemeManager.h"


struct Grade
{
	Grade()
	{
		m_GradeType = GRADE_NO_DATA;
	};


	CString ToString() 
	{
		switch( m_GradeType )
		{
		case GRADE_AAA:		return "AAA";
		case GRADE_AA:		return "AA";
		case GRADE_A:		return "A";
		case GRADE_B:		return "B";
		case GRADE_C:		return "C";
		case GRADE_D:		return "D";
		case GRADE_E:		return "E";
		case GRADE_NO_DATA:	return "N";
		default:			ASSERT(true);	return "N";
		}
	};
	void FromString( CString sGrade )
	{
		sGrade.MakeUpper();
		if	   ( sGrade == "AAA" )	m_GradeType = GRADE_AAA;
		else if( sGrade == "AA" )	m_GradeType = GRADE_AA;
		else if( sGrade == "A" )	m_GradeType = GRADE_A;
		else if( sGrade == "B" )	m_GradeType = GRADE_B;
		else if( sGrade == "C" )	m_GradeType = GRADE_C;
		else if( sGrade == "D" )	m_GradeType = GRADE_D;
		else if( sGrade == "E" )	m_GradeType = GRADE_E;
		else if( sGrade == "N" )	m_GradeType = GRADE_NO_DATA;
		else						ASSERT(true);	// invalid grade string
	};

	enum GradeType { GRADE_NO_DATA=0, GRADE_E, GRADE_D, GRADE_C, GRADE_B, GRADE_A, GRADE_AA, GRADE_AAA };
	GradeType m_GradeType;
};

#endif
