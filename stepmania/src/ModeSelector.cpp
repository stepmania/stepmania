#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ModeSelector

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ModeSelector.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ModeChoice.h"



bool ModeSelector::IsSelectable( const ModeChoice& choice )
{
	return choice.numSidesJoinedToPlay == GAMESTATE->GetNumSidesJoined();
}


#include "ModeSelectorMaxType1.h"
#include "ModeSelectorMaxType2.h"

ModeSelector* ModeSelector::Create( CString sClassName )
{
	if( sClassName.CompareNoCase("ModeSelectorMaxType1") == 0 )		return new ModeSelectorMaxType1;
	if( sClassName.CompareNoCase("ModeSelectorMaxType2") == 0 )		return new ModeSelectorMaxType2;

	RageException::Throw( "Invalid ModeSelector class name '%s'.", sClassName.GetString() );
}
