#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: DifficultyIcon

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "DifficultyIcon.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "Notes.h"
#include "GameState.h"


bool DifficultyIcon::Load( CString sPath )
{
	Sprite::Load( sPath );
	if( GetNumStates() != 5  &&  GetNumStates() != 10 )
		RageException::Throw( "The difficulty icon graphic '%s' must have 5 or 10 frames.", sPath.c_str() );
	StopAnimating();
	return true;
}

void DifficultyIcon::SetFromNotes( PlayerNumber pn, Notes* pNotes )
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
		case 5:		SetState( iStateNo );		break;
		case 10:	SetState( iStateNo*2+pn );	break;
		default:	ASSERT(0);
		}
	}
}
