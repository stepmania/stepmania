#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenCaution

 Desc: Screen that displays while resources are being loaded.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenCaution.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageSoundManager.h"
#include "ThemeManager.h"


#define NEXT_SCREEN				THEME->GetMetric("ScreenCaution","NextScreen")


const ScreenMessage SM_DoneOpening		= ScreenMessage(SM_User-7);
const ScreenMessage SM_StartClosing		= ScreenMessage(SM_User-8);


ScreenCaution::ScreenCaution()
{
	GAMESTATE->m_bPlayersCanJoin = true;

	if(!PREFSMAN->m_bShowDontDie)
	{
		this->SendScreenMessage( SM_GoToNextScreen, 0.f );
		return;
	}

	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations","caution background") );
	this->AddChild( &m_Background );
	
	m_In.Load( THEME->GetPathTo("BGAnimations","caution in") );
	m_In.StartTransitioning( SM_DoneOpening );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathTo("BGAnimations","caution out") );
	this->AddChild( &m_Out );

	m_Back.Load( THEME->GetPathTo("BGAnimations","menu back") );
	this->AddChild( &m_Back );

	this->SendScreenMessage( SM_StartClosing, 3 );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","caution music") );
}


void ScreenCaution::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	PlayerNumber pn = MenuI.player;
	if( MenuI.IsValid()  &&  pn!=PLAYER_INVALID  &&  !GAMESTATE->m_bSideIsJoined[pn] )
	{
		GAMESTATE->m_bSideIsJoined[pn] = true;
		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu start") );
		SCREENMAN->RefreshCreditsMessages();
		return;	// don't fall though
	}

	if( m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Back.IsTransitioning() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}


void ScreenCaution::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartClosing:
		if( !m_Out.IsTransitioning() )
			m_Out.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_DoneOpening:
		SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("caution") );
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenCaution::MenuStart( PlayerNumber pn )
{
	m_Out.StartTransitioning( SM_GoToNextScreen );
}

void ScreenCaution::MenuBack( PlayerNumber pn )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;
	this->ClearMessageQueue();
	m_Back.StartTransitioning( SM_GoToPrevScreen );
	SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","menu back") );
}

