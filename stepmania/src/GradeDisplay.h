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
		SetGrade( GRADE_NO_DATA );
	};

	void SetGrade( Grade g )
	{
		switch( g )
		{
		case GRADE_AAA:		SetState(0);	break;
		case GRADE_AA:		SetState(1);	break;
		case GRADE_A:		SetState(2);	break;
		case GRADE_B:		SetState(3);	break;
		case GRADE_C:		SetState(4);	break;
		case GRADE_D:		SetState(5);	break;
		case GRADE_E:		SetState(6);	break;
		case GRADE_NO_DATA:	SetState(7);	break;
		default:			ASSERT( false );
		}
	};


};



#endif
