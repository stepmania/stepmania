#ifndef PlayerAI_H
#define PlayerAI_H
/*
-----------------------------------------------------------------------------
 Class: PlayerAI

 Desc: .

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"


class PlayerAI
{
public:

	static TapNoteScore PlayerAI::GetTapNoteScore( PlayerController pc, int iSumOfAttackLevels );

};

#endif
