#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: StageDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "StageDisplay.h"
#include "GameState.h"
#include "ThemeManager.h"


StageDisplay::StageDisplay()
{
	LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	TurnShadowOff();
	Refresh();
}

void StageDisplay::Refresh()
{
	SetText( GAMESTATE->GetStageText() );
	SetDiffuse( GAMESTATE->GetStageColor() );
}
