#pragma once
/*
-----------------------------------------------------------------------------
 Class: GradeDisplay

 Desc: The grade shows on ScreenEvaluation

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "PrefsManager.h"
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
	bool  m_bDoScrolling;
	FRECT m_frectStartTexCoords;
	FRECT m_frectDestTexCoords;
	float m_fTimeLeftInScroll;
};
