#pragma once
/*
-----------------------------------------------------------------------------
 File: GradeDisplay.h

 Desc: A graphic displayed in the GradeDisplay during Dancing.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
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

	void SetGrade( Grade g );
	void SpinAndSettleOn( Grade g );

protected:
	
	// for scrolling
	void ScrollToVirtualFrame( int iFrameNo );	// a virtual frame is a tiled state number
	FRECT m_frectStartTexCoords;
	FRECT m_frectDestTexCoords;
	float m_fTimeLeftInScroll;
};
