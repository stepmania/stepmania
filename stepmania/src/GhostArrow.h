#ifndef GHOSTARROW_H
#define GHOSTARROW_H
/*
-----------------------------------------------------------------------------
 Class: GhostArrow

 Desc: The trail a note leaves when it is destroyed.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Sprite.h"
#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"

class GhostArrow : public Sprite
{
public:
	GhostArrow();

	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDeltaTime );

	void  Step( TapNoteScore score );

protected:
	PlayerNumber m_PlayerNumber;
	CString m_sScoreCommand[NUM_TAP_NOTE_SCORES];
};


#endif
