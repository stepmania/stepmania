#ifndef GHOSTARROWBRIGHT_H
#define GHOSTARROWBRIGHT_H
/*
-----------------------------------------------------------------------------
 Class: GhostArrowBright

 Desc: Ghost arrow used when over 100 combo.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "GameConstantsAndTypes.h"

class GhostArrowBright : public Sprite
{
public:
	GhostArrowBright();

	virtual void  Update( float fDeltaTime );

	void  Step( TapNoteScore score );

protected:
};

#endif
