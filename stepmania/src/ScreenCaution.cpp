#include "stdafx.h"
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
#include "RageMusic.h"
#include "ThemeManager.h"


#define NEXT_SCREEN				THEME->GetMetric("ScreenCaution","NextScreen")


const ScreenMessage SM_GoToPrevScreen	= ScreenMessage(SM_User-6);
const ScreenMessage SM_DoneOpening		= ScreenMessage(SM_User-7);
const ScreenMessage SM_StartClosing		= ScreenMessage(SM_User-8);
const ScreenMessage SM_GoToNextScreen	= ScreenMessage(SM_User-9);


ScreenCaution::ScreenCaution()
{
	GAMESTATE->m_bPlayersCanJoin = true;

	if(!PREFSMAN->m_bShowDontDie)
	{
		this->SendScreenMessage( SM_GoToNextScreen, 0.f );
		return;
	}

	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations","caution") );
	this->AddChild( &m_Background );
	
	m_Wipe.OpenWipingRight( SM_DoneOpening );
	this->AddChild( &m_Wipe );

	m_FadeWipe.SetOpened();
	this->AddChild( &m_FadeWipe );

	this->SendScreenMessage( SM_StartClosing, 3 );

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","caution music") );
}


void ScreenCaution::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Wipe.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}


void ScreenCaution::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartClosing:
		if( !m_Wipe.IsClosing() )
			m_Wipe.CloseWipingRight( SM_GoToNextScreen );
		break;
	case SM_DoneOpening:
		SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("caution") );
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
	if( pn != PLAYER_INVALID  &&  !GAMESTATE->m_bSideIsJoined[pn] )
	{
		GAMESTATE->m_bSideIsJoined[pn] = true;
		SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu start") );
		SCREENMAN->RefreshCreditsMessages();
		return;	// don't fall though
	}

	if( !m_Wipe.IsOpening()  &&  !m_Wipe.IsClosing() )
		m_Wipe.CloseWipingRight( SM_GoToNextScreen );
}

void ScreenCaution::MenuBack( PlayerNumber pn )
{
	if(m_FadeWipe.IsClosing())
		return;
	this->ClearMessageQueue();
	m_FadeWipe.CloseWipingLeft( SM_GoToPrevScreen );
	SOUND->PlayOnceStreamed( THEME->GetPathTo("Sounds","menu back") );
}

