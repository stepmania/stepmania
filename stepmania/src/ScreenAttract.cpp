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

#define NEXT_SCREEN						THEME->GetMetric(m_sName,"NextScreen")


ScreenAttract::ScreenAttract( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenAttract::ScreenAttract(%s)", sClassName.c_str() );

	GAMESTATE->Reset();

	// We have to do initialization in the first update because this->GetElementName() won't
	// work until the object has been fully constructed.
	m_Background.LoadFromAniDir( THEME->GetPathToB(m_sName+" background") );
	this->AddChild( &m_Background );

	m_In.Load( THEME->GetPathToB("ScreenAttract in") );
	m_In.StartTransitioning();
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB("ScreenAttract out") );
	this->AddChild( &m_Out );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sName) );

	m_soundStart.Load( THEME->GetPathToS("Common start") );

	SOUNDMAN->PlayMusic( THEME->GetPathToS(m_sName + " music") );	// DO loop.  -Chris

	GAMESTATE->m_bPlayersCanJoin = true;


	float fTimeUntilBeginFadingOut = m_Background.GetLengthSeconds() - m_Out.GetLengthSeconds();
	this->PostScreenMessage( SM_BeginFadingOut, fTimeUntilBeginFadingOut );
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
			switch( PREFSMAN->m_iCoinMode )
			{
			case COIN_PAY:
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
					break;	// don't fall through
				// fall through
			case COIN_HOME:
			case COIN_FREE:
				SOUNDMAN->StopMusic();
				/* We already played the it was a coin was inserted.  Don't play it again. */
				if( MenuI.button != MENU_BUTTON_COIN )
					SOUNDMAN->PlayOnce( THEME->GetPathToS("Common coin") );
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
			SCREENMAN->PostMessageToTopScreen( SM_BeginFadingOut, 0 );
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
