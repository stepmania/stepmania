#ifndef SMALLGRADEDISPLAY_H
#define SMALLGRADEDISPLAY_H
/*
-----------------------------------------------------------------------------
 Class: SmallGradeDisplay

 Desc: The grade shows on ScreenEvaluation

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "Grade.h"
#include "GameConstantsAndTypes.h"



class SmallGradeDisplay : public Sprite
{
public:
	SmallGradeDisplay();
	
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetGrade( PlayerNumber pn, Grade g );

protected:
	
	Grade m_Grade;
};

#endif
