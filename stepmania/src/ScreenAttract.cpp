#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenAttract

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAttract.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageUtil.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "ThemeManager.h"
#include "SDL_Utils.h"
#include "RageSoundManager.h"


#define SECONDS_TO_SHOW					THEME->GetMetricF("Screen"+this->GetMetricName(),"SecondsToShow")
#define NEXT_SCREEN						THEME->GetMetric("Screen"+this->GetMetricName(),"NextScreen")

const ScreenMessage SM_BeginFadingOut	=	ScreenMessage(SM_User+2);
const ScreenMessage SM_GoToNextScreen	=	ScreenMessage(SM_User+3);


ScreenAttract::ScreenAttract()
{
	LOG->Trace( "ScreenAttract::ScreenAttract()" );
}


ScreenAttract::~ScreenAttract()
{
	LOG->Trace( "ScreenAttract::~ScreenAttract()" );
}


void ScreenAttract::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenAttract::Input()" );

	if(type != IET_FIRST_PRESS) return; // don't care

	if( m_Fade.IsOpening() || m_Fade.IsClosing() )
		return;

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:
		case MENU_BUTTON_RIGHT:
			m_Fade.CloseWipingRight( SM_GoToNextScreen );
			break;
		case MENU_BUTTON_START:
			m_soundMusic.Stop();
			GAMESTATE->m_bSideIsJoined[MenuI.player] = true;
			GAMESTATE->m_MasterPlayerNumber = MenuI.player;
			GAMESTATE->m_bPlayersCanJoin = false;

			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","insert coin") );
			::Sleep( 1000 );	// do a little pause, like the arcade does
			SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
			break;
		case MENU_BUTTON_BACK:
			Exit();
			break;
		}
	}

//	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenAttract::FirstUpdate()
{
	// We have to do initialization in the first update because this->GetElementName() won't
	// work until the object has been fully constructed.
	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations",this->GetElementName()) );
	this->AddChild( &m_Background );

	m_Fade.SetClosed();
	m_Fade.OpenWipingRight( SM_None );
	this->AddChild( &m_Fade );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(this->GetElementName()) );

	m_soundStart.Load( THEME->GetPathTo("Sounds","menu start") );

	m_soundMusic.Load( THEME->GetPathTo("Sounds",this->GetElementName() + " music") );
	m_soundMusic.Play();

	this->SendScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );
}

void ScreenAttract::Update( float fDelta )
{
	Screen::Update(fDelta);
}

void ScreenAttract::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		if( !m_Fade.IsClosing() )
			m_Fade.CloseWipingRight( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}
