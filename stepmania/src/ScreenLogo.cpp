#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenLogo

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenLogo.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "GameDef.h"
#include "RageLog.h"
#include "SongManager.h"

#define LOGO_ON_COMMAND			THEME->GetMetric("ScreenLogo","LogoOnCommand")


ScreenLogo::ScreenLogo() : ScreenAttract("ScreenLogo")
{
	m_sprLogo.Load( THEME->GetPathToG(ssprintf("ScreenLogo %s",GAMESTATE->GetCurrentGameDef()->m_szName)) );
	m_sprLogo.Command( LOGO_ON_COMMAND );
	this->AddChild( &m_sprLogo );

	this->MoveToTail( &m_In );	// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );	// put it in the back so it covers up the stuff we just added
}
