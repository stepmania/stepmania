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


#define SECONDS_TO_SHOW					THEME->GetMetricF(m_sMetricName,"SecondsToShow")
#define NEXT_SCREEN						THEME->GetMetric(m_sMetricName,"NextScreen")

const ScreenMessage SM_BeginFadingOut	=	ScreenMessage(SM_User+2);
const ScreenMessage SM_GoToNextScreen	=	ScreenMessage(SM_User+3);


ScreenAttract::ScreenAttract( CString sMetricName, CString sElementName )
{
	LOG->Trace( "ScreenAttract::ScreenAttract(%s, %s)", sMetricName.c_str(), sElementName.c_str() );

	GAMESTATE->Reset();

	m_sMetricName = sMetricName;
	m_sElementName = sElementName;

	// We have to do initialization in the first update because this->GetElementName() won't
	// work until the object has been fully constructed.
	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations",m_sElementName) );
	this->AddChild( &m_Background );

	m_Fade.SetClosed();
	m_Fade.OpenWipingRight( SM_None );
	this->AddChild( &m_Fade );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sElementName) );

	m_soundStart.Load( THEME->GetPathTo("Sounds","menu start") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds",m_sElementName + " music"), false );

	GAMESTATE->m_bPlayersCanJoin = true;

	this->SendScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );
}


ScreenAttract::~ScreenAttract()
{
	LOG->Trace( "ScreenAttract::~ScreenAttract()" );
}


void ScreenAttract::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenAttract::Input()" );

	if(type != IET_FIRST_PRESS) return; // don't care

	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F3 )
	{
		(int&)PREFSMAN->m_CoinMode = (PREFSMAN->m_CoinMode+1) % PrefsManager::NUM_COIN_MODES;
		ResetGame();
		CString sMessage = "Coin Mode: ";
		switch( PREFSMAN->m_CoinMode )
		{
		case PrefsManager::COIN_HOME:	sMessage += "HOME";	break;
		case PrefsManager::COIN_PAY:	sMessage += "PAY";	break;
		case PrefsManager::COIN_FREE:	sMessage += "FREE";	break;
		}
		SCREENMAN->SystemMessage( sMessage );
		return;
	}

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:
		case MENU_BUTTON_RIGHT:
			if( !m_Fade.IsOpening() && !m_Fade.IsClosing() )
				m_Fade.CloseWipingRight( SM_GoToNextScreen );
			break;
		case MENU_BUTTON_COIN:
			Screen::MenuCoin( MenuI.player );	// increment coins, play sound
			SOUNDMAN->music->StopPlaying();
			::Sleep( 200 );	// do a little pause, like the arcade does
			SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
			break;
		case MENU_BUTTON_START:
		case MENU_BUTTON_BACK:

			switch( PREFSMAN->m_CoinMode )
			{
			case PrefsManager::COIN_PAY:
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
					break;	// don't fall through
				// fall through
			case PrefsManager::COIN_FREE:
			case PrefsManager::COIN_HOME:
				SOUNDMAN->music->StopPlaying();
				SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","insert coin") );
				::Sleep( 200 );	// do a little pause, like the arcade does
				SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
				break;
			default:
				ASSERT(0);
			}
			break;
		}
	}

//	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
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
		/* XXX: Look at the def of the screen we're going to; if it has a 
		 * music theme element and it's the same as the one we're playing
		 * now, don't stop.  (However, if we're going to interrupt it 
		 * when we fade in, it's cleaner to stop it before we fade out.) */
		SOUNDMAN->PlayMusic( "" );
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}
