#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: DifficultyIcon

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

/* Obsolete: use DifficultyClass. */
#include "DifficultyIcon.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "Steps.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "arch/ArchHooks/ArchHooks.h"


bool DifficultyIcon::Load( CString sPath )
{
	Sprite::Load( sPath );
	int iStates = GetNumStates();
	if( iStates != NUM_DIFFICULTIES  &&  iStates != NUM_DIFFICULTIES*2 )
	{
		CString sError = ssprintf(
			"The difficulty icon graphic '%s' must have %d or %d frames.  It has %d states.", 
			sPath.c_str(), 
			NUM_DIFFICULTIES,
			NUM_DIFFICULTIES*2,
			iStates );
		HOOKS->MessageBoxOK( sError );
	}
	StopAnimating();
	return true;
}

void DifficultyIcon::SetFromNotes( PlayerNumber pn, Steps* pNotes )
{
	if( pNotes == NULL )
	{
		SetDiffuse( RageColor(1,1,1,0) );
		return;
	}
	else
	{
		SetDiffuse( RageColor(1,1,1,1) );

		int iStateNo = pNotes->GetDifficulty();

		switch( GetNumStates() )
		{
		case NUM_DIFFICULTIES:		SetState( iStateNo );		break;
		case NUM_DIFFICULTIES*2:	SetState( iStateNo*2+pn );	break;
		default:					SetState( 0 );				break;
		}
	}
}
