#include "global.h"
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
#include "SDL_utils.h"
#include "RageSoundManager.h"

#define NEXT_SCREEN						THEME->GetMetric(m_sClassName,"NextScreen")


ScreenAttract::ScreenAttract( CString sClassName )
{
	LOG->Trace( "ScreenAttract::ScreenAttract(%s)", sClassName.c_str() );

	GAMESTATE->Reset();

	m_sClassName = sClassName;

	// We have to do initialization in the first update because this->GetElementName() won't
	// work until the object has been fully constructed.
	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations",m_sClassName+" background") );
	this->AddChild( &m_Background );

	m_In.Load( THEME->GetPathTo("BGAnimations","ScreenAttract in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathTo("BGAnimations","ScreenAttract out") );
	this->AddChild( &m_Out );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sClassName) );

	m_soundStart.Load( THEME->GetPathTo("Sounds","Common start") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds",m_sClassName + " music") );	// DO loop.  -Chris

	GAMESTATE->m_bPlayersCanJoin = true;


	float fTimeUntilBeginFadingOut = m_Background.GetLengthSeconds() - m_Out.GetLengthSeconds();
	this->SendScreenMessage( SM_BeginFadingOut, fTimeUntilBeginFadingOut );
}


ScreenAttract::~ScreenAttract()
{
	LOG->Trace( "ScreenAttract::~ScreenAttract()" );
}


void ScreenAttract::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenAttract::Input()" );

	AttractInput( DeviceI, type, GameI, MenuI, StyleI, m_In.IsTransitioning() || m_Out.IsTransitioning() );
}

void ScreenAttract::AttractInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI, bool bTransitioning )
{
	if(type != IET_FIRST_PRESS) 
		return; // don't care

	ChangeCoinModeInput( DeviceI, type, GameI, MenuI, StyleI );

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_START:
		case MENU_BUTTON_BACK:
		case MENU_BUTTON_COIN:
			switch( PREFSMAN->m_CoinMode )
			{
			case PrefsManager::COIN_PAY:
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
					break;	// don't fall through
				// fall through
			case PrefsManager::COIN_FREE:
			case PrefsManager::COIN_HOME:
				SOUNDMAN->StopMusic();
				/* We already played the it was a coin was inserted.  Don't play it again. */
				if( MenuI.button != MENU_BUTTON_COIN )
					SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","Common coin") );
				SDL_Delay( 800 );	// do a little pause, like the arcade does
				SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
				break;
			default:
				ASSERT(0);
			}
			break;
		}
	}

	if( bTransitioning )
		return;

	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_LEFT:
		case MENU_BUTTON_RIGHT:
			SCREENMAN->SendMessageToTopScreen( SM_BeginFadingOut, 0 );
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
		if( !m_Out.IsTransitioning() )
			m_Out.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		/* XXX: Look at the def of the screen we're going to; if it has a 
		 * music theme element and it's the same as the one we're playing
		 * now, don't stop.  (However, if we're going to interrupt it 
		 * when we fade in, it's cleaner to stop it before we fade out.) */
		
		/* Don't stop the music, or else the music will start over from the 
		 * beginning for consecutive screens with the same music. -Chris */
		
		/* But if you don't stop it, for screens that have their own unique
		 * music, it will constantly loop even after the screen has gone on
		 * to the next attract screen. -- Miryokuteki */
		SOUNDMAN->PlayMusic( "" );
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}
