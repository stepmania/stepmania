#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenGameOver

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenGameOver.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageSounds.h"
#include "ThemeManager.h"


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_PlayAnnouncer	=	ScreenMessage(SM_User + 3);


#define NEXT_SCREEN		THEME->GetMetric("ScreenGameOver","NextScreen")


ScreenGameOver::ScreenGameOver() : Screen("ScreenGameOver")
{
	GAMESTATE->Reset();

	m_Background.LoadFromAniDir( THEME->GetPathToB("ScreenGameOver background") );
	this->AddChild( &m_Background );

	m_In.Load( THEME->GetPathToB("ScreenGameOver in") );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB("ScreenGameOver out") );
	this->AddChild( &m_Out );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenGameOver music") );

	m_In.StartTransitioning( SM_PlayAnnouncer );
	this->PostScreenMessage( SM_StartFadingOut, m_Background.GetLengthSeconds() );
}


void ScreenGameOver::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PlayAnnouncer:
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("game over") );
		break;
	case SM_StartFadingOut:
		m_Out.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenGameOver::MenuStart( PlayerNumber pn )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	this->ClearMessageQueue();
	this->PostScreenMessage( SM_StartFadingOut, 0 );
}
