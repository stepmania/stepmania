#pragma once
/*
-----------------------------------------------------------------------------
 File: GradeDisplay.h

 Desc: A graphic displayed in the GradeDisplay during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "ThemeManager.h"
#include "Grade.h"



class GradeDisplay : public Sprite
{
public:
	GradeDisplay();
	
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetGrade( Grade g );
	void SpinAndSettleOn( Grade g );

protected:
	
	Grade m_Grade;

	// for scrolling
	FRECT m_frectStartTexCoords;
	FRECT m_frectDestTexCoords;
	float m_fTimeLeftInScroll;
};
