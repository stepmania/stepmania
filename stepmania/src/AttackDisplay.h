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
#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"	// for TapNoteScore


class AttackDisplay : public ActorFrame
{
public:
	AttackDisplay();

	void Init( PlayerNumber pn );
	void SetAttack( const CString &mod );

	virtual void Update( float fDelta );

protected:
	PlayerNumber m_PlayerNumber;

	Sprite		m_sprAttack;
};

#endif
