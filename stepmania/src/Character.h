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

	bool Load( CString sCharDir );	// return true if success

	CString GetTakingABreakPath();
	CString GetCardPath();
	CString GetIconPath();

	CString GetModelPath();
	CString GetRestAnimationPath();
	CString GetWarmUpAnimationPath();
	CString GetDanceAnimationPath();

	CString m_sCharDir;
	CString m_sName;

	// All the stuff below will be filled in if this character is playable in Rave mode
	bool	m_bUsableInRave;	

	CString	m_sAttacks[NUM_ATTACK_LEVELS][NUM_ATTACKS_PER_LEVEL];

	CString GetNormalHeadPath();
	CString GetHappyHeadPath();
	CString GetAngryHeadPath();
};


#endif
