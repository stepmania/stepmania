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
#include "ActorFrame.h"

class GhostArrow : public ActorFrame
{
public:
	GhostArrow();

	void Load( CString sNoteSkin, CString sButton, CString sFile, bool bLoadJustOne );
	virtual void Init( PlayerNumber pn );

	virtual void Update( float fDeltaTime );

	void Step( TapNoteScore score );

protected:
	PlayerNumber m_PlayerNumber;
	Sprite	m_spr[NUM_TAP_NOTE_SCORES];
	CString m_sScoreCommand[NUM_TAP_NOTE_SCORES];
};


#endif
