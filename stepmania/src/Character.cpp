#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Character

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Character.h"
#include "IniFile.h"
#include "RageUtil.h"


void Character::Load( CString sCharIniPath )
{
	IniFile ini;
	ini.SetPath( sCharIniPath );
	ini.ReadFile();

	// save file name
	CString sThrowAway;
	splitrelpath( sCharIniPath, sThrowAway, m_sFileName, sThrowAway );
	
	// load attacks
	for( int i=0; i<NUM_ATTACK_LEVELS; i++ )
		for( int j=0; j<NUM_ATTACKS_PER_LEVEL; j++ )
			ini.GetValue( "Character", ssprintf("Level%dAttack%d",i+1,j+1), m_sAttacks[i][j] );
}
