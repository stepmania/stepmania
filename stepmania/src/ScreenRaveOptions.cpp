#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenRaveOptions

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
-----------------------------------------------------------------------------
*/

#include "ScreenRaveOptions.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "ThemeManager.h"

#define PREV_SCREEN			THEME->GetMetric ("ScreenRaveOptions","PrevScreen")
#define NEXT_SCREEN			THEME->GetMetric ("ScreenRaveOptions","NextScreen")

ScreenRaveOptions::ScreenRaveOptions( CString sClassName ): ScreenOptionsMaster( sClassName )
{
	LOG->Trace( "ScreenRaveOptions::ScreenRaveOptions()" );
}

void ScreenRaveOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( PREV_SCREEN );
}

void ScreenRaveOptions::GoToNextState()
{
	SCREENMAN->SetNewScreen( NEXT_SCREEN );
}
