#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: SmallGradeDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "SmallGradeDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"


const float SCROLL_TIME = 5.0f;
const int NUM_GRADE_FRAMES = 7;
const float GRADE_FRAME_HEIGHT = 1/(float)NUM_GRADE_FRAMES;
const float GRADES_TO_SCROLL = NUM_GRADE_FRAMES*4;


SmallGradeDisplay::SmallGradeDisplay()
{
	Load( THEME->GetPathTo("Graphics","select music small grades 2x8") );
	StopAnimating();

	SetGrade( PLAYER_1, GRADE_NO_DATA );
}

void SmallGradeDisplay::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void SmallGradeDisplay::DrawPrimitives()
{
	Sprite::DrawPrimitives();
}

void SmallGradeDisplay::SetGrade( PlayerNumber pn, Grade g )
{
	m_Grade = g;

	SetDiffuse( D3DXCOLOR(1,1,1,1) );

	int iNumCols = 2;
	switch( g )
	{
	case GRADE_AAA:		SetState( 0*iNumCols+pn );	break;
	case GRADE_AA:		SetState( 1*iNumCols+pn );	break;
	case GRADE_A:		SetState( 2*iNumCols+pn );	break;
	case GRADE_B:		SetState( 3*iNumCols+pn );	break;
	case GRADE_C:		SetState( 4*iNumCols+pn );	break;
	case GRADE_D:		SetState( 5*iNumCols+pn );	break;
	case GRADE_E:		SetState( 6*iNumCols+pn );	break;
	case GRADE_NO_DATA:	SetState( 7*iNumCols+pn );	break;
	default:			ASSERT(0);
	}
}
