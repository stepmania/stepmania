#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: LightsDriver_SystemMessage

 Desc: See header.

 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LightsDriver_SystemMessage.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "GameDef.h"


LightsDriver_SystemMessage::LightsDriver_SystemMessage()
{
}

LightsDriver_SystemMessage::~LightsDriver_SystemMessage()
{
}

void LightsDriver_SystemMessage::Set( const LightsState *ls )
{
	CString s;
	
	s += "Cabinet: ";
	FOREACH_CabinetLight( cl )
	{
		s += ls->m_bCabinetLights[cl] ? '1' : '0';
	}
	s += "\n";

	int iNumGameButtonsToShow = GAMESTATE->GetCurrentGameDef()->GetNumGameplayButtons();
	
	FOREACH_GameController( gc )
	{
		s += ssprintf("Controller%d: ",gc+1);
		for( int gb=0; gb<iNumGameButtonsToShow; gb++ )
		{
			s += ls->m_bGameButtonLights[gc][gb] ? '1' : '0';
		}
		s += "\n";
	}

	SCREENMAN->SystemMessageNoAnimate( s );
}
