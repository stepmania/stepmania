/*
-----------------------------------------------------------------------------
 Class: GrayArrow

 Desc: A gray arrow that "receives" ColorArrows.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/


#ifndef _GrayArrow_H_
#define _GrayArrow_H_


#include "Sprite.h"
#include "NoteMetadata.h"

class GrayArrow : public Sprite
{
public:
	GrayArrow();

	virtual void  SetBeat( const float fSongBeat );
	void Step();
};

#endif 
