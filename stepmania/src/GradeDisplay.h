/*
-----------------------------------------------------------------------------
 File: GradeDisplay.h

 Desc: A graphic displayed in the GradeDisplay during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _GradeDisplay_H_
#define _GradeDisplay_H_


#include "Sprite.h"
#include "ThemeManager.h"
#include "Grade.h"



class GradeDisplay : public Sprite
{
public:
	GradeDisplay()
	{
		Load( THEME->GetPathTo(GRAPHIC_GRADES) );
		StopAnimating();
		Grade grade;
		grade.m_GradeType = Grade::GRADE_NO_DATA;
		SetGrade( grade );
	};

	void SetGrade( Grade grade )
	{
		switch( grade.m_GradeType )
		{
		case Grade::GRADE_NO_DATA:	SetState(7);	break;
		case Grade::GRADE_E:		SetState(6);	break;
		case Grade::GRADE_D:		SetState(5);	break;
		case Grade::GRADE_C:		SetState(4);	break;
		case Grade::GRADE_A:		SetState(3);	break;
		case Grade::GRADE_AA:		SetState(2);	break;
		case Grade::GRADE_AAA:		SetState(1);	break;
		default:			ASSERT( true );
		}
	};


};



#endif
