#ifndef AttackDisplay_H
#define AttackDisplay_H
/*
-----------------------------------------------------------------------------
 Class: AttackDisplay

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"	// for TapNoteScore


class AttackDisplay : public ActorFrame
{
public:
	AttackDisplay();

	void Init( PlayerNumber pn ) { m_PlayerNumber = pn; }

	virtual void Update( float fDelta );

protected:
	PlayerNumber m_PlayerNumber;

	BitmapText	m_textAttack;
};

#endif
