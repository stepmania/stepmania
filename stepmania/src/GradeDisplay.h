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
#include "GameConstantsAndTypes.h"


class GradeDisplay : public Sprite
{
public:
	GradeDisplay();
	
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void SetGrade( PlayerNumber pn, Grade g );
	void SpinAndSettleOn( Grade g );
	void SettleImmediately();

protected:
	
	Grade m_Grade;

	// for scrolling
	bool  m_bDoScrolling;
	RectF m_frectStartTexCoords;
	RectF m_frectDestTexCoords;
	float m_fTimeLeftInScroll;
};
