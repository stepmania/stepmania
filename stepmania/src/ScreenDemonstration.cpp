#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenDemonstration

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenDemonstration.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "StepMania.h"
#include "ScreenAttract.h"	// for AttractInput()


#define SECONDS_TO_SHOW			THEME->GetMetricF("ScreenDemonstration","SecondsToShow")
#define NEXT_SCREEN				THEME->GetMetric("ScreenDemonstration","NextScreen")


const ScreenMessage	SM_NotesEnded				= ScreenMessage(SM_User+101);	// MUST be same as in ScreenGameplay


bool PrepareForDemonstration()		// always return true.
{
	switch( GAMESTATE->m_CurGame )
	{
	case GAME_DANCE:	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;			break; 
	case GAME_PUMP:		GAMESTATE->m_CurStyle = STYLE_PUMP_VERSUS;			break; 
	case GAME_EZ2:		GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_VERSUS;	break; 
	case GAME_PARA:		GAMESTATE->m_CurStyle = STYLE_PARA_SINGLE;			break; 
	case GAME_DS3DDX:	GAMESTATE->m_CurStyle = STYLE_DS3DDX_SINGLE;		break;
	case GAME_BM:		GAMESTATE->m_CurStyle = STYLE_BM_SINGLE;			break;
	default:	ASSERT(0);
	}

	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;


	return true;
}

ScreenDemonstration::ScreenDemonstration() : ScreenJukebox( PrepareForDemonstration() )	// this is a hack to get some code to execute before the ScreenGameplay constructor
{
	LOG->Trace( "ScreenDemonstration::ScreenDemonstration()" );

	if( GAMESTATE->m_pCurSong == NULL )	// we didn't find a song.
	{
		this->SendScreenMessage( SM_GoToNextScreen, 0 );	// Abort demonstration.
		return;
	}


	m_Overlay.LoadFromAniDir( THEME->GetPathTo("BGAnimations","ScreenDemonstration overlay") );
	this->AddChild( &m_Overlay );

	this->MoveToTail( &m_In );
	this->MoveToTail( &m_Out );

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that drive "ready", "go", etc.

	GAMESTATE->m_bPastHereWeGo = true;

	m_DancingState = STATE_DANCING;
	this->SendScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );	
}

ScreenDemonstration::~ScreenDemonstration()
{
}

void ScreenDemonstration::Update( float fDeltaTime )
{
	ScreenGameplay::Update( fDeltaTime );
}

void ScreenDemonstration::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	//LOG->Trace( "ScreenDemonstration::Input()" );


	if( MenuI.IsValid() )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_COIN:
		case MENU_BUTTON_START:
		case MENU_BUTTON_BACK:

			if( PREFSMAN->m_CoinMode == PrefsManager::COIN_PAY )
				if( GAMESTATE->m_iCoins < PREFSMAN->m_iCoinsPerCredit )
					break;	// don't fall through
			m_soundMusic.Stop();
			break;
		}
	}

	ScreenAttract::AttractInput( DeviceI, type, GameI, MenuI, StyleI, m_In.IsTransitioning() || m_Out.IsTransitioning() );
}

void ScreenDemonstration::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_NotesEnded:
	case SM_BeginFadingOut:
		m_Out.StartTransitioning( SM_GoToNextScreen );
		return;
	case SM_GoToNextScreen:
		m_soundMusic.Stop();
		GAMESTATE->Reset();
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		return;
	}

	ScreenGameplay::HandleScreenMessage( SM );
}
