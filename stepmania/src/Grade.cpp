#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Grade

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Grade.h"
#include "RageUtil.h"

CString GradeToString( Grade g )
{
	// string is meant to be human readable
	switch( g )
	{
	case GRADE_NO_DATA:	return "NoData";
	case GRADE_FAILED:	return "Failed";
	default:
		return ssprintf("%02d",g);
	}
};

Grade StringToGrade( const CString &sGrade )
{
	CString s = sGrade;
	s.MakeUpper();

	// letters for backward compatibility
	if	   ( s == "AAAA" )		return GRADE_TIER_1;
	else if( s == "AAA" )		return GRADE_TIER_2;
	else if( s == "AA" )		return GRADE_TIER_3;
	else if( s == "A" )			return GRADE_TIER_4;
	else if( s == "B" )			return GRADE_TIER_5;
	else if( s == "C" )			return GRADE_TIER_6;
	else if( s == "D" )			return GRADE_TIER_7;
	else if( s == "E" )			return GRADE_FAILED;
	else if( s == "FAILED" )	return GRADE_FAILED;
	else if( s == "NODATA" )	return GRADE_NO_DATA;

#define MY_IS_DIGIT( c ) (c>='0'&&c<='9')

	// If the string is a two digit number...
	if( s.length()==2 && MY_IS_DIGIT(s[0]) && MY_IS_DIGIT(s[1]) )
	{
		return (Grade)atoi(s);
	}

	ASSERT(0);
	return GRADE_NO_DATA;
};

