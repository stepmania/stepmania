#ifndef Character_H
#define Character_H
/*
-----------------------------------------------------------------------------
 Class: Character

 Desc: An persona that defines attacks for use in battle

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"


class Character
{
public:
//	Character();

	void Load( CString sCharIniPath );

	CString m_sFileName;
	CString	m_sAttacks[NUM_ATTACK_LEVELS][NUM_ATTACKS_PER_LEVEL];
};


#endif
