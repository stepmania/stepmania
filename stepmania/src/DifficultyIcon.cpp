#include "stdafx.h"
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
	if( GetNumStates() != 6  &&  GetNumStates() != 12 )
		throw RageException( "The difficulty icon graphic '%s' must have 6 or 12 states.", sPath.GetString() );
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

		int iStateNo = pNotes->GetNotesDisplayType();

		switch( GetNumStates() )
		{
		case 6:		SetState( iStateNo );		break;
		case 12:	SetState( iStateNo*2+pn );	break;
		default:	ASSERT(0);
		}
	}
}
