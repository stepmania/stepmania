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
/* DifficultyClass was the old name for enum Difficulty, right? -Chris */
/* Maybe, but that wouldn't make sense.  I might have meant DifficultyMeter, whose
 * functionality is a superset of DifficultyIcon; this class can probably be phased
 * out. -glenn */

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
		SetHidden(true);
	else
		SetFromDifficulty( pn, pNotes->GetDifficulty() );
}

void DifficultyIcon::SetFromDifficulty( PlayerNumber pn, Difficulty dc )
{
	switch( GetNumStates() )
	{
	case NUM_DIFFICULTIES:		SetState( dc );			break;
	case NUM_DIFFICULTIES*2:	SetState( dc*2+pn );	break;
	default:		ASSERT(0);
	}	
}

void DifficultyIcon::SetFromCourseDifficulty( PlayerNumber pn, CourseDifficulty cd  )
{
	switch( cd )
	{
	case COURSE_DIFFICULTY_REGULAR:		SetFromDifficulty(pn,DIFFICULTY_MEDIUM);	break;
	case COURSE_DIFFICULTY_DIFFICULT:	SetFromDifficulty(pn,DIFFICULTY_HARD);		break;
	default:		ASSERT(0);
	}
}
