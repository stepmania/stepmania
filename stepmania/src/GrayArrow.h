#ifndef GRAY_ARROW_H
#define GRAY_ARROW_H
/*
-----------------------------------------------------------------------------
 Class: GrayArrow

 Desc: A gray arrow that "receives" ColorNotes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "PlayerNumber.h"

class GrayArrow : public Sprite
{
public:
	GrayArrow();
	bool Load( CString NoteSkin, PlayerNumber pn, int iColNo );

	virtual void  Update( float fDeltaTime );
	void Step();
};

#endif 
